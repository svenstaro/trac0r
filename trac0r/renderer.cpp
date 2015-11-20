#include "renderer.hpp"

namespace trac0r {

Renderer::Renderer(const Camera &camera, const Scene &scene) :
    m_camera(camera), m_scene(scene) {}

glm::vec4 Renderer::get_color(unsigned x, unsigned y) const {
    glm::vec2 rel_pos = m_camera.screenspace_to_camspace(x, y);
    glm::vec3 world_pos = m_camera.camspace_to_worldspace(rel_pos);
    glm::vec3 ray_dir = glm::normalize(world_pos - m_camera.pos());

    glm::vec3 result_color = m_scene.intersect(world_pos, ray_dir, 0, m_max_depth);
    return glm::vec4(result_color, 1.f);
}

}
