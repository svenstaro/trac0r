#include "scene.hpp"
#include "flat_structure.hpp"
#include "utils.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/random.hpp>

#include <memory>

namespace trac0r {

Scene::Scene() {
    m_accel = std::make_unique<FlatStructure>();
}

void Scene::add_shape(std::unique_ptr<Shape> &shape) {
    m_accel->add_shape(std::move(shape));    
}

glm::vec3 Scene::intersect(glm::vec3 &ray_pos, glm::vec3 &ray_dir, int depth, int max_depth) const {
    return m_accel->intersect(ray_pos, ray_dir, depth, max_depth);
}

void Scene::rebuild(const Camera &camera) {
    m_accel->rebuild(camera);
}

}
