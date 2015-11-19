#ifndef SCENE_HPP
#define SCENE_HPP

#include "triangle.hpp"
#include "camera.hpp" // TODO: Temp till bvh

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace trac0r {

class Scene {
  public:
    Scene();
    void add_triangle(std::unique_ptr<Triangle> &tri);
    glm::vec3 intersect(glm::vec3 &ray_pos, glm::vec3 &ray_dir, int depth) const;
    void rebuild(const Camera &camera);

  private:
    int m_max_depth = 5;
    std::vector<std::unique_ptr<Triangle>> m_triangles;
};

}

#endif /* end of include guard: SCENE_HPP*/
