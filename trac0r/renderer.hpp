#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "camera.hpp"
#include "scene.hpp"

namespace trac0r {

class Renderer {
  public:
    Renderer(const Camera &camera, const Scene &scene);
    glm::vec4 get_color(unsigned x, unsigned y) const;

  private:
    const Camera &m_camera;
    const Scene &m_scene;
};

}

#endif /* end of include guard: RENDERER_HPP */
