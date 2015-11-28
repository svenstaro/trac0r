#ifndef FLAT_STRUCTURE_HPP
#define FLAT_STRUCTURE_HPP

#include "triangle.hpp"
#include "ray.hpp"
#include "intersection_info.hpp"
#include "shape.hpp"
#include "camera.hpp"

#include <glm/glm.hpp>

namespace trac0r {

class FlatStructure {
  public:
    static void add_shape(FlatStructure &flatstruct, Shape &shape);
    static std::vector<Shape> &shapes(FlatStructure &flatstruct);
    static const std::vector<Shape> &shapes(const FlatStructure &flatstruct);
    static IntersectionInfo intersect(const FlatStructure &flatstruct, const Ray &ray);
    static void rebuild(FlatStructure &flatstruct, const Camera &camera);

  private:
    std::vector<Triangle> m_triangles;
    std::vector<Shape> m_shapes;
};
}

#endif /* end of include guard: FLAT_STRUCTURE_HPP */
