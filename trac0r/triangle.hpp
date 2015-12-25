#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include "material.hpp"
#include "random.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/normal.hpp>

namespace trac0r {

struct Triangle {
    Triangle() {
    }

    Triangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, Material material)
        : m_v1(v1), m_v2(v2), m_v3(v3), m_material(material) {
        rebuild();
    }

    // rebuild cached geometry data
    void rebuild() {
        m_normal = glm::triangleNormal(m_v1, m_v2, m_v3);

        m_centroid.x = m_v1.x + m_v2.x + m_v3.x;
        m_centroid.y = m_v1.y + m_v2.y + m_v3.y;
        m_centroid.z = m_v1.z + m_v2.z + m_v3.z;
        m_centroid /= 3;

        // Heron's formula
        auto a = glm::length(m_v1 - m_v2);
        auto b = glm::length(m_v2 - m_v3);
        auto c = glm::length(m_v3 - m_v1);
        auto s = (a + b + c) / 2.f;
        m_area = glm::sqrt(s * (s - a) * (s - b) * (s - c));
    }

    static inline glm::vec3 random_point(const Triangle &triangle) {
        return triangle.m_v1 + rand_range(0.f, 1.f) * (triangle.m_v2 - triangle.m_v1) +
               rand_range(0.f, 1.f) * (triangle.m_v3 - triangle.m_v1);
    }

    glm::vec3 m_v1;
    glm::vec3 m_v2;
    glm::vec3 m_v3;
    Material m_material;
    glm::vec3 m_normal;
    glm::vec3 m_centroid;
    float m_area;
};
}

#endif /* end of include guard: TRIANGLE_HPP */
