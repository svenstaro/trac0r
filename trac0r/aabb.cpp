#include "aabb.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/vector_query.hpp>

#include <fmt/format.h>
#include <glm/gtx/string_cast.hpp>

namespace trac0r {

bool AABB::is_null(const AABB &aabb) {
    bool min_null = glm::isNull(aabb.m_min, glm::epsilon<decltype(aabb.m_min)::value_type>());
    bool max_null = glm::isNull(aabb.m_max, glm::epsilon<decltype(aabb.m_max)::value_type>());
    return min_null && max_null;
}

glm::vec3 AABB::min(const AABB &aabb) {
    return aabb.m_min;
}

glm::vec3 AABB::max(const AABB &aabb) {
    return aabb.m_max;
}

glm::vec3 AABB::diagonal(const AABB &aabb) {
    if (!is_null(aabb))
        return aabb.m_max - aabb.m_min;
    else
        return glm::vec3(0);
}

glm::vec3 AABB::center(const AABB &aabb) {
    if (!is_null(aabb))
        return aabb.m_min + (diagonal(aabb) * 0.5f);
    else
        return glm::vec3(0);
}

std::array<glm::vec3, 8> AABB::vertices(AABB &aabb) {
    std::array<glm::vec3, 8> result;
    result[0] = {aabb.m_min.x, aabb.m_min.y, aabb.m_min.z}; // lower left front
    result[1] = {aabb.m_max.x, aabb.m_min.y, aabb.m_min.z}; // lower right front
    result[2] = {aabb.m_min.x, aabb.m_max.y, aabb.m_min.z}; // upper left front
    result[3] = {aabb.m_max.x, aabb.m_max.y, aabb.m_min.z}; // upper right front
    result[4] = {aabb.m_min.x, aabb.m_min.y, aabb.m_max.z}; // lower left back
    result[5] = {aabb.m_max.x, aabb.m_min.y, aabb.m_max.z}; // lower right back
    result[6] = {aabb.m_min.x, aabb.m_max.y, aabb.m_max.z}; // upper left back
    result[7] = {aabb.m_max.x, aabb.m_max.y, aabb.m_max.z}; // upper right back
    return result;
}

void AABB::extend(AABB &aabb, glm::vec3 &point) {
    if (!is_null(aabb)) {
        aabb.m_min = glm::min(point, aabb.m_min);
        aabb.m_max = glm::max(point, aabb.m_max);
    } else {
        aabb.m_min = point;
        aabb.m_max = point;
    }
}

bool AABB::overlaps(const AABB &first, const AABB &second) {
    if (is_null(first) || is_null(second))
        return false;

    return first.m_max.x > second.m_min.x && first.m_min.x < second.m_max.x &&
           first.m_max.y > second.m_min.y && first.m_min.y < second.m_max.y &&
           first.m_max.z > second.m_min.z && first.m_min.z < second.m_max.z;
}

void AABB::reset(AABB &aabb) {
    aabb.m_min = glm::vec3(0);
    aabb.m_max = glm::vec3(0);
}
}
