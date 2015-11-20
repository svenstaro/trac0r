#include "acceleration_structure.hpp"

namespace trac0r {

AccelerationStructure::~AccelerationStructure() {}

void AccelerationStructure::add_shape(std::unique_ptr<Shape> &&shape) {
    m_shapes.push_back(std::move(shape));
}

std::vector<std::unique_ptr<Shape>> &AccelerationStructure::shapes() {
    return m_shapes;
}

}
