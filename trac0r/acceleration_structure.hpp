#ifndef ACCELERATION_STRUCTURE_HPP
#define ACCELERATION_STRUCTURE_HPP

#include "shape.hpp"
#include "camera.hpp"
#include "ray.hpp"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace trac0r {

class AccelerationStructure {
public:
    virtual ~AccelerationStructure() = 0;
    void add_shape(std::unique_ptr<Shape> &&shape);
    std::vector<std::unique_ptr<Shape>> &shapes();
    virtual glm::vec3 intersect(const Ray &ray, int depth, int max_depth) const = 0;
    virtual void rebuild(const Camera &camera) = 0;

protected:
    std::vector<std::unique_ptr<Shape>> m_shapes;
};

}

#endif /* end of include guard: ACCELERATION_STRUCTURE_HPP */
