#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "camera.hpp"
#include "scene.hpp"
#include "light_vertex.hpp"

#ifdef OPENCL
#include <CL/cl.hpp>
#endif

#include <chrono>
#include <memory>

namespace trac0r {

class Renderer {
  public:
    Renderer(const int width, const int height, const Camera &camera, const Scene &scene,
             bool print_perf);
    static glm::vec4 trace_camera_ray(const Ray &ray, const unsigned max_depth, const Scene &scene);
    static void trace_light_ray(const Ray &ray, const unsigned max_depth, const Scene &scene,
                                const Triangle &light_triangle, const unsigned light_path_index,
                                std::vector<LightVertex> &lvc);
    std::vector<glm::vec4> &render(bool screen_changed, int stride_x, int stride_y);
    void print_sysinfo() const;
    void print_last_frame_timings() const;

  private:
    int m_max_depth = 6;

    /**
     * @brief We accumulate our "photons" into here for each pixel
     */
    std::vector<glm::vec4> m_luminance;

    /**
     * @brief LVC for light traces
     */
    std::vector<LightVertex> m_lvc;

    const int m_width;
    const int m_height;
    const Camera &m_camera;
    const Scene &m_scene;
    bool m_print_perf = false;

#ifdef OPENCL
    double m_last_frame_buffer_write_time;
    double m_last_frame_kernel_run_time;
    double m_last_frame_buffer_read_time;
    std::vector<cl::Platform> m_compute_platforms;
    std::vector<cl::Device> m_compute_devices;
    cl::Context m_compute_context;
    std::vector<cl::CommandQueue> m_compute_queues;
    cl::Program m_program;
    cl::Kernel m_kernel;
#endif
};
}

#endif /* end of include guard: RENDERER_HPP */
