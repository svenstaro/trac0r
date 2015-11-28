#include "scene.hpp"
#include "utils.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/random.hpp>

#include <memory>

namespace trac0r {

void Scene::add_shape(Scene &scene, Shape &shape) {
    FlatStructure::add_shape(Scene::accel_struct(scene), shape);
}

IntersectionInfo Scene::intersect(const Scene &scene, const Ray &ray) {
    return FlatStructure::intersect(accel_struct(scene), ray);
}

void Scene::rebuild(Scene &scene, const Camera &camera) {
    FlatStructure::rebuild(Scene::accel_struct(scene), camera);
}

FlatStructure &Scene::accel_struct(Scene &scene) {
    return scene.m_accel_struct;
}

const FlatStructure &Scene::accel_struct(const Scene &scene) {
    return scene.m_accel_struct;
}
}
