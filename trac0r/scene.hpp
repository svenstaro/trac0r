#ifndef SCENE_HPP
#define SCENE_HPP

#include "ray.hpp"
#include "shape.hpp"
#include "camera.hpp"
#include "intersection_info.hpp"
#include "acceleration_structure.hpp"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace trac0r {

class Scene {
  public:
    Scene();
    void add_shape(Shape &shape);
    IntersectionInfo intersect(const Ray &ray) const;
    void rebuild(const Camera &camera);
    std::unique_ptr<AccelerationStructure> &accel();

  private:
    std::unique_ptr<AccelerationStructure> m_accel;
};

}

#endif /* end of include guard: SCENE_HPP*/
