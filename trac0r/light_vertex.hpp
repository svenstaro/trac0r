#ifndef LIGHT_VERTEX_HPP
#define LIGHT_VERTEX_HPP

#include <glm/glm.hpp>

struct LightVertex {
    glm::vec3 m_pos{0.f};
    glm::vec3 m_luminance{0.f};
};

#endif /* end of include guard: LIGHT_VERTEX_HPP */
