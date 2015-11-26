#ifndef FLAT_STRUCTURE_HPP
#define FLAT_STRUCTURE_HPP

#include "acceleration_structure.hpp"
#include "triangle.hpp"
#include "ray.hpp"
#include "intersection_info.hpp"

#include <glm/glm.hpp>

namespace trac0r {

class FlatStructure final : public AccelerationStructure {
  public:
    ~FlatStructure() override;
    IntersectionInfo intersect(const Ray &ray) const override;
    void rebuild(const Camera &camera) override;

  private:
    std::vector<Triangle> m_triangles;
};
}

#endif /* end of include guard: FLAT_STRUCTURE_HPP */
