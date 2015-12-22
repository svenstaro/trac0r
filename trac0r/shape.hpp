#ifndef SHAPE_HPP
#define SHAPE_HPP

#include "aabb.hpp"
#include "triangle.hpp"
#include "material.hpp"

#include <memory>
#include <vector>

namespace trac0r {

class Shape {
  public:
    static const glm::vec3 pos(const Shape &shape);
    static void set_pos(Shape &shape, glm::vec3 new_pos);

    static const glm::vec3 orientation(const Shape &shape);
    static void set_orientation(Shape &shape, glm::vec3 new_orientation);

    static const glm::vec3 scale(const Shape &shape);
    static void set_scale(Shape &shape, glm::vec3 new_scale);

    static AABB &aabb(Shape &shape);
    static const AABB &aabb(const Shape &shape);

    static std::vector<Triangle> &triangles(Shape &shape);
    static const std::vector<Triangle> &triangles(const Shape &shape);

    static void add_triangle(Shape &shape, const Triangle triangle);

    static Shape make_box(glm::vec3 pos, glm::vec3 orientation, glm::vec3 size, Material material);

    static Shape make_icosphere(glm::vec3 pos, glm::vec3 orientation, float radius,
                                size_t iterations, Material material);

    static Shape make_plane(glm::vec3 pos, glm::vec3 orientation, glm::vec2 size,
                            Material material);

  protected:
    glm::vec3 m_pos;
    glm::vec3 m_orientation;
    glm::vec3 m_scale;
    AABB m_aabb;
    std::vector<Triangle> m_triangles;

  private:
    static void rebuild(Shape &shape);
};
}

#endif /* end of include guard: SHAPE_HPP */
