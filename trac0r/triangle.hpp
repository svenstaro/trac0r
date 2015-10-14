#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include <glm/glm.hpp>
#include <glm/gtx/normal.hpp>

struct Triangle {
    Triangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 color, glm::vec3 reflectance) : m_v1(v1), m_v2(v2), m_v3(v3), m_color(color), m_reflectance(reflectance) {
        m_normal = glm::triangleNormal(v1, v2, v3);
        
        m_centroid.x = v1.x + v2.x + v3.x;
        m_centroid.y = v1.y + v2.y + v3.y;
        m_centroid.z = v1.z + v2.z + v3.z;
        m_centroid /= 3;
    }
    glm::vec3 m_v1;
    glm::vec3 m_v2;
    glm::vec3 m_v3;
    glm::vec3 m_color;
    glm::vec3 m_reflectance;
    glm::vec3 m_normal;
    glm::vec3 m_centroid;
};

#endif /* end of include guard: TRIANGLE_HPP */
