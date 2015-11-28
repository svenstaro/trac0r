#ifndef AABB_HPP
#define AABB_HPP

#include <glm/glm.hpp>

#include <array>

namespace trac0r {

class AABB {
  public:
    static bool is_null(const AABB &aabb);
    static glm::vec3 min(const AABB &aabb);
    static glm::vec3 max(const AABB &aabb);
    static glm::vec3 diagonal(const AABB &aabb);
    static glm::vec3 center(const AABB &aabb);
    static std::array<glm::vec3, 8> vertices(AABB &aabb);
    static void extend(AABB &aabb, glm::vec3 &point);
    static bool overlaps(const AABB &first, const AABB &second);
    static void reset(AABB &aabb);

  private:
    glm::vec3 m_min;
    glm::vec3 m_max;
};
}

#endif /* end of include guard: AABB_HPP */
