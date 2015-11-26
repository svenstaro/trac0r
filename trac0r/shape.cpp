#include "shape.hpp"

#include "glm/gtx/rotate_vector.hpp"

namespace trac0r {

const glm::vec3 Shape::pos() const {
    return m_pos;
}

void Shape::set_pos(glm::vec3 new_pos) {
    m_pos = new_pos;
}

const glm::vec3 Shape::orientation() const {
    return m_orientation;
}

void Shape::set_orientation(glm::vec3 new_orientation) {
    m_orientation = new_orientation;
}

const glm::vec3 Shape::scale() const {
    return m_scale;
}

void Shape::set_scale(glm::vec3 new_scale) {
    m_scale = new_scale;
}

const AABB &Shape::aabb() const {
    return m_aabb;
}

std::vector<Triangle> &Shape::triangles() {
    return m_triangles;
}

const std::vector<Triangle> &Shape::triangles() const {
    return m_triangles;
}

void Shape::add_triangle(const Triangle triangle) {
    m_triangles.push_back(triangle);
}

Shape Shape::make_box(glm::vec3 pos, glm::vec3 orientation, glm::vec3 size, Material material) {
    Shape new_shape;
    new_shape.set_pos(pos);
    new_shape.set_orientation(glm::normalize(orientation));
    new_shape.set_scale(size);

    auto p1 = glm::vec3{-0.5f, 0.5f, -0.5f};
    auto p2 = glm::vec3{-0.5f, -0.5f, -0.5f};
    auto p3 = glm::vec3{0.5f, -0.5f, -0.5f};
    auto p4 = glm::vec3{0.5f, 0.5f, -0.5f};
    auto p5 = glm::vec3{-0.5f, 0.5f, 0.5f};
    auto p6 = glm::vec3{-0.5f, -0.5f, 0.5f};
    auto p7 = glm::vec3{0.5f, -0.5f, 0.5f};
    auto p8 = glm::vec3{0.5f, 0.5f, 0.5f};

    // front face
    auto t1 = Triangle(p1, p2, p3, material);
    auto t2 = Triangle(p1, p4, p3, material);

    // right face
    auto t3 = Triangle(p3, p4, p8, material);
    auto t4 = Triangle(p3, p7, p8, material);

    // left face
    auto t5 = Triangle(p1, p2, p6, material);
    auto t6 = Triangle(p1, p5, p6, material);

    // back face
    auto t7 = Triangle(p5, p6, p8, material);
    auto t8 = Triangle(p6, p7, p8, material);

    // top face
    auto t9 = Triangle(p5, p1, p4, material);
    auto t10 = Triangle(p5, p8, p4, material);

    // bottom face
    auto t11 = Triangle(p6, p3, p2, material);
    auto t12 = Triangle(p3, p7, p6, material);

    new_shape.add_triangle(t1);
    new_shape.add_triangle(t2);
    new_shape.add_triangle(t3);
    new_shape.add_triangle(t4);
    new_shape.add_triangle(t5);
    new_shape.add_triangle(t6);
    new_shape.add_triangle(t7);
    new_shape.add_triangle(t8);
    new_shape.add_triangle(t9);
    new_shape.add_triangle(t10);
    new_shape.add_triangle(t11);
    new_shape.add_triangle(t12);

    glm::mat4 translate = glm::translate(pos);
    glm::mat4 rotation = glm::orientation(orientation, {0, 1, 0});
    glm::mat4 scale = glm::scale(size);
    glm::mat4 model = translate * rotation * scale;

    for (auto &tri : new_shape.triangles()) {
        tri.m_v1 = glm::vec3(model * glm::vec4(tri.m_v1, 1));
        tri.m_v2 = glm::vec3(model * glm::vec4(tri.m_v2, 1));
        tri.m_v3 = glm::vec3(model * glm::vec4(tri.m_v3, 1));
        tri.rebuild();
    }

    new_shape.rebuild();

    return new_shape;
}

Shape Shape::make_plane(glm::vec3 pos, glm::vec3 orientation, glm::vec2 size, Material material) {
    Shape new_shape;
    new_shape.set_pos(pos);
    new_shape.set_orientation(glm::normalize(orientation));
    new_shape.set_scale(glm::vec3(size.x, 0, size.y));

    auto p1 = glm::vec3{-0.5f, 0, 0.5f};
    auto p2 = glm::vec3{-0.5f, 0, -0.5f};
    auto p3 = glm::vec3{0.5f, 0, -0.5f};
    auto p4 = glm::vec3{0.5f, 0, 0.5f};

    auto triangle_left = Triangle(p1, p2, p3, material);
    auto triangle_right = Triangle(p1, p4, p3, material);

    new_shape.add_triangle(triangle_left);
    new_shape.add_triangle(triangle_right);

    glm::mat4 translate = glm::translate(pos);
    glm::mat4 rotation = glm::orientation(orientation, {0, 1, 0});
    glm::mat4 scale = glm::scale(new_shape.scale());
    glm::mat4 model = translate * rotation * scale;

    for (auto &tri : new_shape.triangles()) {
        tri.m_v1 = glm::vec3(model * glm::vec4(tri.m_v1, 1));
        tri.m_v2 = glm::vec3(model * glm::vec4(tri.m_v2, 1));
        tri.m_v3 = glm::vec3(model * glm::vec4(tri.m_v3, 1));
        tri.rebuild();
    }

    new_shape.rebuild();

    return new_shape;
}

void Shape::rebuild() {
    m_aabb.reset();
    for (auto &tri : m_triangles) {
        m_aabb.extend(tri.m_v1);
        m_aabb.extend(tri.m_v2);
        m_aabb.extend(tri.m_v3);
    }
}
}
