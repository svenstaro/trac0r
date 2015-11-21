#ifndef RAY_HPP
#define RAY_HPP

#include <glm/glm.hpp>

struct Ray {
    Ray(const glm::vec3 &origin, const glm::vec3 &direction)
        : m_origin(origin), m_dir(direction), m_invdir(1.f / direction) {
    }

    glm::vec3 m_origin;
    glm::vec3 m_dir;
    glm::vec3 m_invdir;
};

#endif /* end of include guard: RAY_HPP */
