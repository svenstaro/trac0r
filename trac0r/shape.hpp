#ifndef SHAPE_HPP
#define SHAPE_HPP

#include "aabb.hpp"
#include "triangle.hpp"

#include <memory>
#include <vector>

namespace trac0r {

class Shape {
  public:
    const glm::vec3 pos() const;
    void set_pos(glm::vec3 new_pos);

    const glm::vec3 orientation() const;
    void set_orientation(glm::vec3 new_orientation);

    const glm::vec3 scale() const;
    void set_scale(glm::vec3 new_scale);

    const AABB &aabb() const;

    std::vector<Triangle> &triangles();
    void add_triangle(const Triangle triangle);

    static std::unique_ptr<Shape> make_box(glm::vec3 pos, glm::vec3 orientation, glm::vec3 size,
                                           glm::vec3 reflectance = {0, 0, 0},
                                           glm::vec3 emittance = {0.3f, 0.3f, 0.3f});

    static std::unique_ptr<Shape> make_plane(glm::vec3 pos, glm::vec3 orientation, glm::vec2 size,
                                             glm::vec3 reflectance = {0, 0, 0},
                                             glm::vec3 emittance = {0.2f, 0.2f, 0.2f});

  protected:
    glm::vec3 m_pos;
    glm::vec3 m_orientation;
    glm::vec3 m_scale;
    AABB m_aabb;
    std::vector<Triangle> m_triangles;
    
  private:
    void rebuild();
};
}

#endif /* end of include guard: SHAPE_HPP */
