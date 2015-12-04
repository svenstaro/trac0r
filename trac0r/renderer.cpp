#include "renderer.hpp"
#include "ray.hpp"
#include "random.hpp"

#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "cppformat/format.h"

namespace trac0r {

Renderer::Renderer(const int width, const int height, const Camera &camera, const Scene &scene)
    : m_width(width), m_height(height), m_camera(camera), m_scene(scene) {
    m_luminance.resize(width * height, glm::vec4{0});

#ifdef OPENCL
    // auto devices = boost::compute::system::devices();
    // m_compute_context = boost::compute::context(devices);
    // for (auto &device : devices) {
    //     m_compute_queues.emplace_back(boost::compute::command_queue(m_compute_context, device));
    // }
    //
    // m_program = boost::compute::program::create_with_source_file("trac0r/renderer_aux.cl",
    //                                                              m_compute_context);
    // try {
    //     m_program.build();
    //     m_kernel = boost::compute::kernel(m_program, "renderer_trace_pixel_color");
    // } catch (boost::compute::opencl_error &e) {
    //     fmt::print("{}", m_program.build_log());
    // }
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

    struct DeviceFlatStructure {
        DeviceTriangle m_triangles[100];
        cl_uint m_num_triangles;
    };

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

    // size_t image_size = m_width * m_height;

    // std::vector<boost::compute::float4_> host_output;
    // host_output.resize(image_size);

    // Init dev_output
    // boost::compute::vector<boost::compute::float4_> dev_output(image_size, m_compute_context);

    // Init dev_prng
    // DevicePRNG dev_prng;
    // auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
    // auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
    // for (auto i = 0; i < 16; i++)
    //     dev_prng.m_seed[i] = xorshift64star(ns);
    // dev_prng.m_p = 0;
    // boost::compute::buffer dev_prng_buf(m_compute_context, sizeof(dev_prng));
    // m_compute_queues[0].enqueue_write_buffer(dev_prng_buf, 0, sizeof(dev_prng), &dev_prng);

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
    // boost::compute::buffer dev_camera_buf(m_compute_context, sizeof(dev_camera));
    // m_compute_queues[0].enqueue_write_buffer(dev_camera_buf, 0, sizeof(dev_camera), &dev_camera);

    // Init dev_flatstruct
    DeviceFlatStructure dev_flatstruct;
    dev_flatstruct.m_num_triangles = 100;
    auto tri_num = 0;
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
            dev_flatstruct.m_triangles[tri_num] = dev_tri;
            tri_num++;
        }
    }
    // boost::compute::buffer dev_flatstruct_buf(m_compute_context, sizeof(dev_flatstruct));
    // m_compute_queues[0].enqueue_write_buffer(dev_flatstruct_buf, 0, sizeof(dev_flatstruct),
    //                                          &dev_flatstruct);
    //
    // m_kernel.set_arg(0, dev_output);
    // m_kernel.set_arg(1, m_width);
    // m_kernel.set_arg(2, m_max_depth);
    // m_kernel.set_arg(3, dev_prng_buf.size(), dev_prng_buf.get());
    // m_kernel.set_arg(4, sizeof(dev_camera), &dev_camera);
    // m_kernel.set_arg(5, sizeof(dev_flatstruct), &dev_flatstruct);
    // m_compute_queues[0].enqueue_nd_range_kernel(m_kernel, boost::compute::dim(0, 0),
    //                                             boost::compute::dim(m_width, m_height),
    //                                             boost::compute::dim(1, 1));
    //
    // boost::compute::copy(dev_output.begin(), dev_output.end(), host_output.begin(),
    //                      m_compute_queues[0]);
    //
    // for (auto x = 0; x < m_width; x += stride_x) {
    //     for (auto y = 0; y < m_height; y += stride_y) {
    //         auto lol = reinterpret_cast<glm::vec4 *>(&host_output[y * m_width + x]);
    //         m_luminance[y * m_width + x] += *lol;
    //     }
    // }
#else
#pragma omp parallel for collapse(2) schedule(dynamic, 1024)
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
