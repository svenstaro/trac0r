#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "camera.hpp"
#include "scene.hpp"

#ifdef OPENCL
#include <boost/compute/core.hpp>
#endif

namespace trac0r {

class Renderer {
  public:
    Renderer(const int width, const int height, const Camera &camera, const Scene &scene);
    glm::vec4 trace_pixel_color(unsigned x, unsigned y) const;
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
    boost::compute::context m_compute_context;
    std::vector<boost::compute::command_queue> m_compute_queues;
#endif
};

}

#endif /* end of include guard: RENDERER_HPP */
