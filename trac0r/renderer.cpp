#include "renderer.hpp"
#include "ray.hpp"
#include "random.hpp"
#include "utils.hpp"
#include "timer.hpp"

#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "cppformat/format.h"

#include <fstream>
#include <thread>

namespace trac0r {

Renderer::Renderer(const int width, const int height, const Camera &camera, const Scene &scene,
                   bool print_perf)
    : m_width(width), m_height(height), m_camera(camera), m_scene(scene), m_print_perf(print_perf) {
    m_luminance.resize(width * height, glm::vec4{0});

#ifdef OPENCL
    cl::Platform::get(&m_compute_platforms);
    for (const auto &platform : m_compute_platforms) {
        std::vector<cl::Device> devices;
        // platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
        platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
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
    // cl_int result = m_program.build();
    cl_int result = m_program.build("-cl-fast-relaxed-math");
    // cl_int result = m_program.build("-cl-nv-verbose");
    auto build_log = m_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(m_compute_devices[0]);
    fmt::print("{}\n", build_log);
    if (result != CL_SUCCESS)
        exit(1);
    m_kernel = cl::Kernel(m_program, "renderer_trace_camera_ray", &result);
    if (result != CL_SUCCESS) {
        fmt::print("{}\n", opencl_error_string(result));
        exit(1);
    }
#endif
}

std::vector<glm::vec4> &Renderer::render(bool scene_changed, int stride_x, int stride_y) {
#ifdef OPENCL
    struct DevicePRNG {
        cl_ulong m_seed[16];
        cl_ulong m_p;
    };

    struct DeviceMaterial {
        cl_uchar m_type;
        cl_float3 m_color;
        cl_float m_roughness;
        cl_float m_ior;
        cl_float m_emittance;
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

    const size_t image_size = m_width * m_height;

    Timer timer;

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
            dev_mat.m_type = tri.m_material.m_type;
            dev_mat.m_color = {{tri.m_material.m_color.r, tri.m_material.m_color.g, tri.m_material.m_color.b}};
            dev_mat.m_roughness = tri.m_material.m_roughness;
            dev_mat.m_ior = tri.m_material.m_ior;
            dev_mat.m_emittance = tri.m_material.m_emittance;
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

    if (m_print_perf)
        m_last_frame_buffer_write_time = timer.elapsed();

    m_kernel.setArg(0, dev_output_buf);
    m_kernel.setArg(1, m_width);
    m_kernel.setArg(2, m_max_camera_subpath_depth);
    m_kernel.setArg(3, dev_prng_buf);
    m_kernel.setArg(4, dev_camera_buf);
    m_kernel.setArg(5, dev_triangles_buf);
    m_kernel.setArg(6, static_cast<uint32_t>(dev_triangles.size()));
    cl::Event event;

    cl::Device device = m_compute_queues[0].getInfo<CL_QUEUE_DEVICE>();
    auto preferred_work_size_multiple =
        m_kernel.getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(device);
    auto max_work_group_size = m_kernel.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(device);
    auto local_mem_size = m_kernel.getWorkGroupInfo<CL_KERNEL_LOCAL_MEM_SIZE>(device);
    auto private_mem_size = m_kernel.getWorkGroupInfo<CL_KERNEL_PRIVATE_MEM_SIZE>(device);
    auto device_name = device.getInfo<CL_DEVICE_NAME>();
    auto local_work_size_x = preferred_work_size_multiple;
    auto local_work_size_y = 2;
    auto global_work_size = m_width * m_height;
    auto work_group_size = local_work_size_x * local_work_size_y;

    fmt::print("    Executing kernel ({}x{}/{}x{}, global work items: {}, items per work group: "
               "{}, total work groups: {}) on device {}\n",
               m_width, m_height, local_work_size_x, local_work_size_y, global_work_size,
               local_work_size_x * local_work_size_y, global_work_size / work_group_size,
               device_name);
    fmt::print(
        "    Max work group size: {}, Local mem size used by kernel: {} KB, minimum private mem "
        "size per work item: {} KB\n",
        max_work_group_size, local_mem_size / 1024, private_mem_size / 1024);
    cl_int result = m_compute_queues[0].enqueueNDRangeKernel(
        m_kernel, cl::NDRange(0, 0), cl::NDRange(m_width, m_height),
        cl::NDRange(local_work_size_x, local_work_size_y), nullptr, &event);

    if (result != CL_SUCCESS) {
        fmt::print("{}\n", opencl_error_string(result));
        exit(1);
    }

    // Wait for kernel to finish computing
    event.wait();

    if (m_print_perf)
        m_last_frame_kernel_run_time = timer.elapsed();

    // Transfer data from GPU back to CPU (TODO In later versions, just expose it to the OpenGL
    // buffer
    // and render it directly in order to get rid of this transfer)
    m_compute_queues[0].enqueueReadBuffer(dev_output_buf, CL_TRUE, 0,
                                          image_size * sizeof(cl_float4), &host_output[0]);

    // Accumulate energy
    for (uint32_t x = 0; x < m_width; x += stride_x) {
        for (uint32_t y = 0; y < m_height; y += stride_y) {
            auto lol = reinterpret_cast<glm::vec4 *>(&host_output[y * m_width + x]);
            if (scene_changed)
                m_luminance[y * m_width + x] = *lol;
            else
                m_luminance[y * m_width + x] += *lol;
        }
    }

    if (m_print_perf)
        m_last_frame_buffer_read_time = timer.elapsed();
#else
    Timer timer;

// TODO Make OpenMP simd option work
#pragma omp parallel for collapse(2) schedule(dynamic, 1024)
    // Reverse path tracing part: Trace a ray through every camera pixel
    for (auto x = 0; x < m_width; x += stride_x) {
        for (auto y = 0; y < m_height; y += stride_y) {
            Ray ray = Camera::pixel_to_ray(m_camera, x, y);
            glm::vec4 new_color = trace_camera_ray(ray, m_max_camera_subpath_depth, m_scene);
            if (scene_changed)
                m_luminance[y * m_width + x] = new_color;
            else
                m_luminance[y * m_width + x] += new_color;
        }
    }

    if (m_print_perf)
        fmt::print("    {:<15} {:>10.3f} ms\n", "Path tracing", timer.elapsed());
#endif

    return m_luminance;
}

void Renderer::print_sysinfo() const {
#ifdef OPENCL
    fmt::print("Rendering on OpenCL\n");
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    for (const auto &platform : platforms) {
        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);

        if (devices.size() > 0) {
            auto platform_name = platform.getInfo<CL_PLATFORM_NAME>();
            auto platform_version = platform.getInfo<CL_PLATFORM_VERSION>();
            fmt::print("  {} ({})\n", platform_name, platform_version);
            for (size_t i = 0; i < devices.size(); i++) {
                auto device_name = devices[i].getInfo<CL_DEVICE_NAME>();
                auto device_version = devices[i].getInfo<CL_DEVICE_VERSION>();
                auto driver_version = devices[i].getInfo<CL_DRIVER_VERSION>();
                auto max_compute_units = devices[i].getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
                auto max_work_item_dimensions =
                    devices[i].getInfo<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>();
                auto max_work_item_sizes = devices[i].getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();
                auto max_work_group_size = devices[i].getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
                auto max_mem_alloc_size = devices[i].getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>();
                auto max_parameter_size = devices[i].getInfo<CL_DEVICE_MAX_PARAMETER_SIZE>();
                auto global_mem_cacheline_size =
                    devices[i].getInfo<CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE>();
                auto global_mem_cache_size = devices[i].getInfo<CL_DEVICE_GLOBAL_MEM_CACHE_SIZE>();
                auto global_mem_size = devices[i].getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>();
                auto max_constant_buffer_size =
                    devices[i].getInfo<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE>();
                auto max_constant_args = devices[i].getInfo<CL_DEVICE_MAX_CONSTANT_ARGS>();
                auto local_mem_size = devices[i].getInfo<CL_DEVICE_LOCAL_MEM_SIZE>();
                auto preferred_work_size_multiple =
                    m_kernel.getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(devices[i]);
                fmt::print("    Device {}: {}, OpenCL version: {}, driver version: {}\n", i,
                           device_name, device_version, driver_version);
                fmt::print("    Compute units: {}, Max work item dim: {}, Max work item sizes: "
                           "{}/{}/{}, Max work group size: {}\n",
                           max_compute_units, max_work_item_dimensions, max_work_item_sizes[0],
                           max_work_item_sizes[1], max_work_item_sizes[2], max_work_group_size);
                fmt::print("    Max mem alloc size: {} MB, Max parameter size: {} B\n",
                           max_mem_alloc_size / (1024 * 1024), max_parameter_size);
                fmt::print("    Global mem cacheline size: {} B, Global mem cache size: {} KB, Global "
                           "mem size: {} MB\n",
                           global_mem_cacheline_size, global_mem_cache_size / 1024,
                           global_mem_size / (1024 * 1024));
                fmt::print("    Max constant buffer size: {} KB, Max constant args: {}, Local mem "
                           "size: {} KB\n",
                           max_constant_buffer_size / 1024, max_constant_args, local_mem_size / 1024);
                fmt::print("    Preferred work group size multiple: {}\n",
                           preferred_work_size_multiple);
                fmt::print("\n");
            }
        }
    }
#else
    fmt::print("Rendering on OpenMP\n");
    auto threads = std::thread::hardware_concurrency();
    fmt::print("    OpenMP ({} threads)\n", threads);
#endif
}

void Renderer::print_last_frame_timings() const {
#ifdef OPENCL
    fmt::print("      {:<15} {:>12.3f} ms\n", "Buffer write to device",
               m_last_frame_buffer_write_time);
    fmt::print("      {:<15} {:>20.3f} ms\n", "Kernel run time", m_last_frame_kernel_run_time);
    fmt::print("      {:<15} {:>15.3f} ms\n", "Buffer read to host", m_last_frame_buffer_read_time);
#endif
}
}
