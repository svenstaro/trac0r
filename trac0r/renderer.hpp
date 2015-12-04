#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "camera.hpp"
#include "scene.hpp"

#ifdef OPENCL
#include <CL/cl.hpp>
#endif

namespace trac0r {

class Renderer {
  public:
    Renderer(const int width, const int height, const Camera &camera, const Scene &scene);
    static glm::vec4 trace_pixel_color(const unsigned x, const unsigned y, const unsigned max_depth,
                                       const Camera &camera, const Scene &scene);
    std::vector<glm::vec4> &render(bool screen_changed, int stride_x, int stride_y);

  private:
    int m_max_depth = 5;

    /**
     * @brief We accumulate our "photons" into here for each pixel
     */

    std::vector<glm::vec4> m_luminance;
    const int m_width;
    const int m_height;
    const Camera &m_camera;
    const Scene &m_scene;

#ifdef OPENCL
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
