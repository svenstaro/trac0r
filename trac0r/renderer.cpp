#include "renderer.hpp"
#include "ray.hpp"

#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "cppformat/format.h"

namespace trac0r {

Renderer::Renderer(const int width, const int height, const Camera &camera, const Scene &scene)
    : m_width(width), m_height(height), m_camera(camera), m_scene(scene) {
    m_luminance.resize(width * height, glm::vec4{0});

#ifdef OPENCL
    auto devices = boost::compute::system::devices();
    m_compute_context = boost::compute::context(devices);
    for (auto &device : devices) {
        m_compute_queues.emplace_back(boost::compute::command_queue(m_compute_context, device));
    }

    m_program = boost::compute::program::create_with_source_file("trac0r/renderer_aux.cl",
                                                                 m_compute_context);
    try {
        m_program.build();
        m_kernel = boost::compute::kernel(m_program, "renderer");
    } catch (boost::compute::opencl_error &e) {
        fmt::print("{}", m_program.build_log());
    }
#endif
}

std::vector<glm::vec4> &Renderer::render(bool scene_changed, int stride_x, int stride_y) {
#ifdef OPENCL
    auto image_size = m_width * m_height;
    boost::compute::default_random_engine rng(m_compute_queues[0]);

    std::vector<boost::compute::float4_> host_lum;
    host_lum.resize(image_size);

    boost::compute::vector<boost::compute::float4_> device_lum(image_size, m_compute_context);

    m_kernel.set_arg(0, device_lum); // output
    m_kernel.set_arg(1, 5);          // max_depth
    m_kernel.set_arg(2, 5);          // prng
    m_kernel.set_arg(3, 5);          // camera
    m_kernel.set_arg(4, 5);          // flatstruct
    m_compute_queues[0].enqueue_nd_range_kernel(m_kernel, 2, boost::compute::dim(0, 0),
                                                boost::compute::dim(m_width, m_height), 0);

    boost::compute::copy(device_lum.begin(), device_lum.end(), host_lum.begin(),
                         m_compute_queues[0]);

    for (auto x = 0; x < m_width; x += stride_x) {
        for (auto y = 0; y < m_height; y += stride_y) {
            auto lol = reinterpret_cast<glm::vec4 *>(&lum[y * m_width + x]);
            m_luminance[y * m_width + x] += *lol;
        }
    }
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
