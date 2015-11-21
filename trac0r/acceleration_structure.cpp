#include "acceleration_structure.hpp"

namespace trac0r {

AccelerationStructure::~AccelerationStructure() {}

void AccelerationStructure::add_shape(Shape &shape) {
    m_shapes.push_back(shape);
}

std::vector<Shape> &AccelerationStructure::shapes() {
    return m_shapes;
}

}
