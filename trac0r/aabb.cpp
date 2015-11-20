#include "aabb.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/vector_query.hpp>

#include <cppformat/format.h>
#include <glm/gtx/string_cast.hpp>

namespace trac0r {

bool AABB::is_null() const {
    bool min_null = glm::isNull(m_min, glm::epsilon<decltype(m_min)::value_type>());
    bool max_null = glm::isNull(m_max, glm::epsilon<decltype(m_max)::value_type>());
    return min_null && max_null;
}

glm::vec3 AABB::min() const {
    return m_min;
}

glm::vec3 AABB::max() const {
    return m_max;
}

glm::vec3 AABB::diagonal() const {
    if (!is_null())
        return m_max - m_min;
    else
        return glm::vec3(0);
}

glm::vec3 AABB::center() const {
    if (!is_null())
        return m_min + (diagonal() * 0.5f);
    else
        return glm::vec3(0);
}

void AABB::extend(glm::vec3 &point) {
    if (!is_null()) {
        m_min = glm::min(point, m_min);
        m_max = glm::max(point, m_max);
    } else {
        m_min = point;
        m_max = point;
    }
}

bool AABB::overlaps(const AABB &other) const {
    if (is_null() || other.is_null())
        return false;

    return m_max.x > other.min().x && m_min.x < other.max().x && m_max.y > other.min().y &&
           m_min.y < other.max().y && m_max.z > other.min().z && m_min.z < other.max().z;
}

void AABB::reset() {
    m_min = glm::vec3(0);
    m_max = glm::vec3(0);
}
}
