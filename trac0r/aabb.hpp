#ifndef AABB_HPP
#define AABB_HPP

#include <glm/glm.hpp>

#include <array>

namespace trac0r {

class AABB {
  public:
    bool is_null() const;
    glm::vec3 min() const;
    glm::vec3 max() const;
    glm::vec3 diagonal() const;
    glm::vec3 center() const;
    std::array<glm::vec3, 8> vertices() const;
    void extend(glm::vec3 &point);
    bool overlaps(const AABB &other) const;
    void reset();

  private:
    glm::vec3 m_min;
    glm::vec3 m_max;
};
}

#endif /* end of include guard: AABB_HPP */
