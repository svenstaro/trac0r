#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <glm/glm.hpp>

namespace trac0r {

struct Material {
    glm::vec3 m_diffuse;
    glm::vec3 m_reflectance;
    glm::vec3 m_refractance;
    glm::vec3 m_emittance;
};
}

#endif /* end of include guard: MATERIAL_HPP */
