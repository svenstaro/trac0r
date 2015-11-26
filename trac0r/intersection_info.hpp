#ifndef INTERSECTION_INFO_HPP
#define INTERSECTION_INFO_HPP

#include "material.hpp"
#include "ray.hpp"

#include <glm/glm.hpp>

namespace trac0r {

struct IntersectionInfo {
    /**
     * @brief Indicates whether an intersection was found.
     */
    bool m_has_intersected = false;

    /**
     * @brief World coordinates of intersection.
     */
    glm::vec3 m_pos;

    /**
     * @brief Normal at the point of intersection.
     */
    glm::vec3 m_normal;

    /**
     * @brief Direction vector of incomig ray towards point of intersection.
     */
    Ray m_incoming_ray = Ray(glm::vec3(0), glm::vec3(0));

    /**
     * @brief Material at the point of intersection.
     */
    Material m_material;
};
}

#endif /* end of include guard: INTERSECTION_INFO_HPP */
