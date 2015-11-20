#ifndef SCENE_HPP
#define SCENE_HPP

#include "shape.hpp"
#include "camera.hpp"
#include "acceleration_structure.hpp"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace trac0r {

class Scene {
  public:
    Scene();
    void add_shape(std::unique_ptr<Shape> &shape);
    glm::vec3 intersect(glm::vec3 &ray_pos, glm::vec3 &ray_dir, int depth, int max_depth) const;
    void rebuild(const Camera &camera);

  private:
    std::unique_ptr<AccelerationStructure> m_accel;
};

}

#endif /* end of include guard: SCENE_HPP*/
