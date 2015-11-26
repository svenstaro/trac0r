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

void Scene::add_shape(Shape &shape) {
    m_accel->add_shape(shape);    
}

IntersectionInfo Scene::intersect(const Ray &ray) const {
    return m_accel->intersect(ray);
}

void Scene::rebuild(const Camera &camera) {
    m_accel->rebuild(camera);
}

std::unique_ptr<AccelerationStructure> &Scene::accel() {
    return m_accel;
}

}
