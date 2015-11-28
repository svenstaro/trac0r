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

    const char source2[] =
        BOOST_COMPUTE_STRINGIZE_SOURCE(__kernel void lolol(__global float4 * input) {
            const uint i = get_global_id(0);
            input[i].x = 1.f;
            input[i].y = .5f;
            input[i].z = 0.f;
            input[i].w = 1.f;
        });

    m_prog2 = boost::compute::program::create_with_source(source2, m_compute_context);
    try {
        m_prog2.build();
        m_kernel2 = boost::compute::kernel(m_prog2, "lolol");
    } catch (boost::compute::opencl_error &e) {
        fmt::print("{}", m_prog2.build_log());
    }
#endif
}

std::vector<glm::vec4> &Renderer::render(bool scene_changed, int stride_x, int stride_y) {
#ifdef OPENCL
    auto n = m_width * m_height;
    boost::compute::default_random_engine rng(m_compute_queues[0]);
    std::vector<boost::compute::float4_> lum;
    lum.resize(n);
    boost::compute::vector<boost::compute::float4_> dev_lum(n, m_compute_context);
    //
    // BOOST_COMPUTE_FUNCTION(boost::compute::float4_, lol, (const boost::compute::float4_ omg), {
    //     float4 was;
    //     was.x = 1.f;
    //     was.y = 1.f;
    //     was.z = 1.f;
    //     was.w = 1.f;
    //     return was;
    // });

    m_kernel2.set_arg(0, dev_lum);
    m_compute_queues[0].enqueue_1d_range_kernel(m_kernel2, 0, n, 0);

    // boost::compute::transform(dev_lum.begin(), dev_lum.end(), dev_lum.begin(), lol,
    // m_compute_queues[0]);

    boost::compute::copy(dev_lum.begin(), dev_lum.end(), lum.begin(), m_compute_queues[0]);

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
            glm::vec4 new_color =
                trace_pixel_color(x, y, m_max_depth, m_camera, m_scene);
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
