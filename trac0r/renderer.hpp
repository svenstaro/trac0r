#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "camera.hpp"
#include "scene.hpp"

#ifdef OPENCL
#include <boost/compute/core.hpp>
#include <boost/compute/function.hpp>
#include <boost/compute/system.hpp>
#include <boost/compute/algorithm/for_each.hpp>
#include <boost/compute/container/vector.hpp>
#include <boost/compute/iterator/buffer_iterator.hpp>
#include <boost/compute/random/default_random_engine.hpp>
#include <boost/compute/types/fundamental.hpp>
#include <boost/compute/types/struct.hpp>
#include <boost/compute/utility/source.hpp>
#include <boost/compute/utility/dim.hpp>
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
    boost::compute::context m_compute_context;
    std::vector<boost::compute::command_queue> m_compute_queues;
    boost::compute::program m_program;
    boost::compute::kernel m_kernel;
#endif
};
}

#endif /* end of include guard: RENDERER_HPP */
