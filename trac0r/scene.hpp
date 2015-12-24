#ifndef SCENE_HPP
#define SCENE_HPP

#include "ray.hpp"
#include "shape.hpp"
#include "camera.hpp"
#include "intersection_info.hpp"
#include "flat_structure.hpp"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace trac0r {

class Scene {
  public:
    static void add_shape(Scene &scene, Shape &shape);
    static IntersectionInfo intersect(const Scene &scene, const Ray &ray);
    static void rebuild(Scene &scene);
    static const FlatStructure &accel_struct(const Scene &scene);
    static FlatStructure &accel_struct(Scene &scene);

  private:
    FlatStructure m_accel_struct;
};
}

#endif /* end of include guard: SCENE_HPP*/
