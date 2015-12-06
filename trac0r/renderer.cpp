#include "renderer.hpp"
#include "ray.hpp"
#include "random.hpp"

#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "cppformat/format.h"

#include <fstream>

namespace trac0r {

Renderer::Renderer(const int width, const int height, const Camera &camera, const Scene &scene)
    : m_width(width), m_height(height), m_camera(camera), m_scene(scene) {
    m_luminance.resize(width * height, glm::vec4{0});

#ifdef OPENCL
    cl::Platform::get(&m_compute_platforms);
    for (const auto &platform : m_compute_platforms) {
        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
        for (auto &device : devices) {
            m_compute_devices.push_back(device);
        }
    }

    m_compute_context = cl::Context(m_compute_devices);
    for (auto &device : m_compute_devices) {
        m_compute_queues.emplace_back(cl::CommandQueue(m_compute_context, device));
    }

    std::ifstream source_file("trac0r/renderer_aux.cl");
    std::string source_content((std::istreambuf_iterator<char>(source_file)),
                               (std::istreambuf_iterator<char>()));
    m_program = cl::Program(m_compute_context, source_content);
    cl_int result = m_program.build();
    // cl_int result = m_program.build("-cl-fast-relaxed-math");
    // cl_int result = m_program.build("-cl-nv-verbose");
    if (result != CL_SUCCESS) {
        auto build_log = m_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(m_compute_devices[0]);
        fmt::print("{}", build_log);
        exit(1);
    }
    m_kernel = cl::Kernel(m_program, "renderer_trace_pixel_color");
#endif
}

std::vector<glm::vec4> &Renderer::render(bool scene_changed, int stride_x, int stride_y) {
#ifdef OPENCL
    struct DevicePRNG {
        cl_ulong m_seed[16];
        cl_ulong m_p;
    };

    struct DeviceMaterial {
        cl_float3 m_reflectance;
        cl_float3 m_emittance;
    };

    struct DeviceTriangle {
        cl_float3 m_v1;
        cl_float3 m_v2;
        cl_float3 m_v3;
        DeviceMaterial m_material;
        cl_float3 m_normal;
        cl_float3 m_centroid;
        cl_float m_area;
    };

    // struct DeviceFlatStructure {
    //     DeviceTriangle *m_triangles;
    //     cl_uint m_num_triangles;
    // };

    struct DeviceCamera {
        cl_float3 m_pos;
        cl_float3 m_dir;
        cl_float3 m_world_up;
        cl_float3 m_right;
        cl_float3 m_up;
        cl_float m_canvas_width;
        cl_float m_canvas_height;
        cl_float3 m_canvas_center_pos;
        cl_float3 m_canvas_dir_x;
        cl_float3 m_canvas_dir_y;
        cl_float m_near_plane_dist;
        cl_float m_far_plane_dist;
        cl_int m_screen_width;
        cl_int m_screen_height;
        cl_float m_vertical_fov;
        cl_float m_horizontal_fov;
    };

    size_t image_size = m_width * m_height;

    std::vector<cl_float4> host_output;
    host_output.resize(image_size);

    // Init dev_output
    cl::Buffer dev_output_buf(m_compute_context, CL_MEM_WRITE_ONLY, image_size * sizeof(cl_float4));

    // Init dev_prng
    DevicePRNG dev_prng;
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
    for (auto i = 0; i < 16; i++)
        dev_prng.m_seed[i] = xorshift64star(ns);
    dev_prng.m_p = 0;
    cl::Buffer dev_prng_buf(m_compute_context, CL_MEM_READ_WRITE, sizeof(dev_prng));
    m_compute_queues[0].enqueueWriteBuffer(dev_prng_buf, CL_TRUE, 0, sizeof(dev_prng), &dev_prng);

    // Init dev_camera
    DeviceCamera dev_camera;
    dev_camera.m_pos = {
        {Camera::pos(m_camera).x, Camera::pos(m_camera).y, Camera::pos(m_camera).z}};
    dev_camera.m_dir = {
        {Camera::dir(m_camera).x, Camera::dir(m_camera).y, Camera::dir(m_camera).z}};
    dev_camera.m_world_up = {
        {Camera::world_up(m_camera).x, Camera::world_up(m_camera).y, Camera::world_up(m_camera).z}};
    dev_camera.m_right = {
        {Camera::right(m_camera).x, Camera::right(m_camera).y, Camera::right(m_camera).z}};
    dev_camera.m_up = {{Camera::up(m_camera).x, Camera::up(m_camera).y, Camera::up(m_camera).z}};
    dev_camera.m_canvas_width = Camera::canvas_width(m_camera);
    dev_camera.m_canvas_height = Camera::canvas_height(m_camera);
    dev_camera.m_canvas_center_pos = {{Camera::canvas_center_pos(m_camera).x,
                                       Camera::canvas_center_pos(m_camera).y,
                                       Camera::canvas_center_pos(m_camera).z}};
    dev_camera.m_canvas_dir_x = {{Camera::canvas_dir_x(m_camera).x,
                                  Camera::canvas_dir_x(m_camera).y,
                                  Camera::canvas_dir_x(m_camera).z}};
    dev_camera.m_canvas_dir_y = {{Camera::canvas_dir_y(m_camera).x,
                                  Camera::canvas_dir_y(m_camera).y,
                                  Camera::canvas_dir_y(m_camera).z}};
    dev_camera.m_near_plane_dist = Camera::near_plane_dist(m_camera);
    dev_camera.m_far_plane_dist = Camera::far_plane_dist(m_camera);
    dev_camera.m_screen_width = Camera::screen_width(m_camera);
    dev_camera.m_screen_height = Camera::screen_height(m_camera);
    dev_camera.m_vertical_fov = Camera::vertical_fov(m_camera);
    dev_camera.m_horizontal_fov = Camera::horizontal_fov(m_camera);
    cl::Buffer dev_camera_buf(m_compute_context, CL_MEM_READ_ONLY, sizeof(dev_camera));
    m_compute_queues[0].enqueueWriteBuffer(dev_camera_buf, CL_TRUE, 0, sizeof(dev_camera),
                                           &dev_camera);

    // Init dev_flatstruct
    // DeviceFlatStructure dev_flatstruct;
    std::vector<DeviceTriangle> dev_triangles;
    auto &accel_struct = Scene::accel_struct(m_scene);
    for (auto &shape : FlatStructure::shapes(accel_struct)) {
        for (auto &tri : Shape::triangles(shape)) {
            DeviceMaterial dev_mat;
            dev_mat.m_reflectance = {{tri.m_material.m_reflectance.r,
                                      tri.m_material.m_reflectance.g,
                                      tri.m_material.m_reflectance.b}};
            dev_mat.m_emittance = {{tri.m_material.m_emittance.r, tri.m_material.m_emittance.g,
                                    tri.m_material.m_emittance.b}};
            DeviceTriangle dev_tri;
            dev_tri.m_v1 = {{tri.m_v1.x, tri.m_v1.y, tri.m_v1.z}};
            dev_tri.m_v2 = {{tri.m_v2.x, tri.m_v2.y, tri.m_v2.z}};
            dev_tri.m_v3 = {{tri.m_v3.x, tri.m_v3.y, tri.m_v3.z}};
            dev_tri.m_material = dev_mat;
            dev_tri.m_normal = {{tri.m_normal.x, tri.m_normal.y, tri.m_normal.z}};
            dev_tri.m_centroid = {{tri.m_centroid.x, tri.m_centroid.y, tri.m_centroid.z}};
            dev_tri.m_area = tri.m_area;
            dev_triangles.push_back(dev_tri);
        }
    }

    cl::Buffer dev_triangles_buf(m_compute_context, CL_MEM_READ_ONLY,
                                 sizeof(DeviceTriangle) * dev_triangles.size());
    m_compute_queues[0].enqueueWriteBuffer(dev_triangles_buf, CL_TRUE, 0,
                                           sizeof(DeviceTriangle) * dev_triangles.size(),
                                           &dev_triangles[0]);

    m_kernel.setArg(0, dev_output_buf);
    m_kernel.setArg(1, m_width);
    m_kernel.setArg(2, m_max_depth);
    m_kernel.setArg(3, dev_prng_buf);
    m_kernel.setArg(4, dev_camera_buf);
    m_kernel.setArg(5, dev_triangles_buf);
    m_kernel.setArg(6, static_cast<unsigned>(dev_triangles.size()));
    cl::Event event;

    m_compute_queues[0].enqueueNDRangeKernel(m_kernel, cl::NDRange(0, 0),
                                             cl::NDRange(m_width, m_height), cl::NDRange(1, 1));

    event.wait();
    m_compute_queues[0].enqueueReadBuffer(dev_output_buf, CL_TRUE, 0,
                                          image_size * sizeof(cl_float4), &host_output[0]);

    for (auto x = 0; x < m_width; x += stride_x) {
        for (auto y = 0; y < m_height; y += stride_y) {
            auto lol = reinterpret_cast<glm::vec4 *>(&host_output[y * m_width + x]);
            if (scene_changed)
                m_luminance[y * m_width + x] = *lol;
            else
                m_luminance[y * m_width + x] += *lol;
        }
    }
#else
#pragma omp parallel for simd collapse(2) schedule(dynamic, 1024)
    for (auto x = 0; x < m_width; x += stride_x) {
        for (auto y = 0; y < m_height; y += stride_y) {
            glm::vec4 new_color = trace_pixel_color(x, y, m_max_depth, m_camera, m_scene);
            if (scene_changed)
                m_luminance[y * m_width + x] = new_color;
            else
                m_luminance[y * m_width + x] += new_color;
        }
    }
#endif

    return m_luminance;
}
}
