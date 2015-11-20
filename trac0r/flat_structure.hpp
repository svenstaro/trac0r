#ifndef FLAT_STRUCTURE_HPP
#define FLAT_STRUCTURE_HPP

#include "acceleration_structure.hpp"
#include "triangle.hpp"

#include <glm/glm.hpp>

namespace trac0r {

class FlatStructure final : public AccelerationStructure {
  public:
    ~FlatStructure() override;
    glm::vec3 intersect(glm::vec3 &ray_pos, glm::vec3 &ray_dir, int depth,
                                int max_depth) const override;
    void rebuild(const Camera &camera) override;

  private:
    std::vector<std::unique_ptr<Triangle>> m_triangles;
};
}

#endif /* end of include guard: FLAT_STRUCTURE_HPP */
