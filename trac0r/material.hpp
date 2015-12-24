#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <glm/glm.hpp>

namespace trac0r {

struct Material {
    /**
     * @brief Used to determine the material type used. Refer to the table below:
     *        m_type = 1: Emissive
     *        m_type = 2: Diffuse
     *        m_type = 3: Glass
     *        m_type = 4: Glossy
     */
    uint8_t m_type = 1;

    /**
     * @brief Used as the basic color for most materials.
     */
    glm::vec3 m_color = {0.9f, 0.5f, 0.1f};

    /**
     * @brief Used to determine the ratio between reflection and diffusion for some materials. Value
     * must be between 0.0 and 1.0.
     */
    float m_roughness = 0.f;

    /**
     * @brief Index of refraction (IOR) is used to determine how strongly light is bent inside a
     * glass
     * material.
     */
    float m_ior = 1.f;

    /**
     * @brief Luminous emittance provides the strength of emissive material. Values can be larger
     * than 1.0.
     */
    float m_emittance = 0.f;
};
}

#endif /* end of include guard: MATERIAL_HPP */
