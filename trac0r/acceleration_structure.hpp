#ifndef ACCELERATION_STRUCTURE_HPP
#define ACCELERATION_STRUCTURE_HPP

#include "shape.hpp"
#include "camera.hpp"
#include "ray.hpp"
#include "intersection_info.hpp"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace trac0r {

class AccelerationStructure {
public:
    virtual ~AccelerationStructure() = 0;
    void add_shape(Shape &shape);
    std::vector<Shape> &shapes();
    virtual IntersectionInfo intersect(const Ray &ray) const = 0;
    virtual void rebuild(const Camera &camera) = 0;

protected:
    std::vector<Shape> m_shapes;
};

}

#endif /* end of include guard: ACCELERATION_STRUCTURE_HPP */
