#include "shape.hpp"

#include "glm/gtx/rotate_vector.hpp"

#include "utils.hpp"

namespace trac0r {

const glm::vec3 Shape::pos(const Shape &shape) {
    return shape.m_pos;
}

void Shape::set_pos(Shape &shape, glm::vec3 new_pos) {
    shape.m_pos = new_pos;
}

const glm::vec3 Shape::orientation(const Shape &shape) {
    return shape.m_orientation;
}

void Shape::set_orientation(Shape &shape, glm::vec3 new_orientation) {
    shape.m_orientation = new_orientation;
}

const glm::vec3 Shape::scale(const Shape &shape) {
    return shape.m_scale;
}

void Shape::set_scale(Shape &shape, glm::vec3 new_scale) {
    shape.m_scale = new_scale;
}

AABB &Shape::aabb(Shape &shape) {
    return shape.m_aabb;
}

const AABB &Shape::aabb(const Shape &shape) {
    return shape.m_aabb;
}

std::vector<Triangle> &Shape::triangles(Shape &shape) {
    return shape.m_triangles;
}

const std::vector<Triangle> &Shape::triangles(const Shape &shape) {
    return shape.m_triangles;
}

void Shape::add_triangle(Shape &shape, const Triangle triangle) {
    shape.m_triangles.push_back(triangle);
}

Shape Shape::make_box(glm::vec3 pos, glm::vec3 orientation, glm::vec3 size, Material material) {
    Shape new_shape;
    Shape::set_pos(new_shape, pos);
    Shape::set_orientation(new_shape, orientation);
    Shape::set_scale(new_shape, size);

    auto p1 = glm::vec3{-0.5f, 0.5f, -0.5f};
    auto p2 = glm::vec3{-0.5f, -0.5f, -0.5f};
    auto p3 = glm::vec3{0.5f, -0.5f, -0.5f};
    auto p4 = glm::vec3{0.5f, 0.5f, -0.5f};
    auto p5 = glm::vec3{-0.5f, 0.5f, 0.5f};
    auto p6 = glm::vec3{-0.5f, -0.5f, 0.5f};
    auto p7 = glm::vec3{0.5f, -0.5f, 0.5f};
    auto p8 = glm::vec3{0.5f, 0.5f, 0.5f};

    // front face
    auto t1 = Triangle(p2, p1, p3, material);
    auto t2 = Triangle(p1, p4, p3, material);

    // right face
    auto t3 = Triangle(p4, p3, p8, material);
    auto t4 = Triangle(p3, p7, p8, material);

    // left face
    auto t5 = Triangle(p1, p2, p6, material);
    auto t6 = Triangle(p5, p1, p6, material);

    // back face
    auto t7 = Triangle(p5, p6, p8, material);
    auto t8 = Triangle(p6, p7, p8, material);

    // top face
    auto t9 = Triangle(p5, p1, p4, material);
    auto t10 = Triangle(p8, p4, p5, material);

    // bottom face
    auto t11 = Triangle(p2, p3, p6, material);
    auto t12 = Triangle(p3, p7, p6, material);

    Shape::add_triangle(new_shape, t1);
    Shape::add_triangle(new_shape, t2);
    Shape::add_triangle(new_shape, t3);
    Shape::add_triangle(new_shape, t4);
    Shape::add_triangle(new_shape, t5);
    Shape::add_triangle(new_shape, t6);
    Shape::add_triangle(new_shape, t7);
    Shape::add_triangle(new_shape, t8);
    Shape::add_triangle(new_shape, t9);
    Shape::add_triangle(new_shape, t10);
    Shape::add_triangle(new_shape, t11);
    Shape::add_triangle(new_shape, t12);

    glm::mat4 translation = glm::translate(glm::mat4(1.f), pos);
    glm::mat4 rotation = glm::orientation(orientation, {0, 1, 0});
    glm::mat4 scale = glm::scale(glm::mat4(1.f), size);
    glm::mat4 model = translation * rotation * scale;

    for (auto &tri : triangles(new_shape)) {
        tri.m_v1 = glm::vec3(model * glm::vec4(tri.m_v1, 1));
        tri.m_v2 = glm::vec3(model * glm::vec4(tri.m_v2, 1));
        tri.m_v3 = glm::vec3(model * glm::vec4(tri.m_v3, 1));
        tri.rebuild();
    }

    rebuild(new_shape);

    return new_shape;
}

Shape Shape::make_icosphere(glm::vec3 pos, glm::vec3 orientation, float radius, size_t iterations,
                            Material material) {
    Shape new_shape;
    Shape::set_pos(new_shape, pos);
    Shape::set_orientation(new_shape, glm::normalize(orientation));
    Shape::set_scale(new_shape, glm::vec3(radius, radius, radius));

    float t = 0.5 + glm::sqrt(5) / 2.f;

    triangles(new_shape).push_back(Triangle{{-1, t, 0}, {-t, 0, 1}, {0, 1, t}, material});
    triangles(new_shape).push_back(Triangle{{-1, t, 0}, {0, 1, t}, {1, t, 0}, material});
    triangles(new_shape).push_back(Triangle{{-1, t, 0}, {1, t, 0}, {0, 1, -t}, material});
    triangles(new_shape).push_back(Triangle{{-1, t, 0}, {0, 1, -t}, {-t, 0, -1}, material});
    triangles(new_shape).push_back(Triangle{{-1, t, 0}, {-t, 0, -1}, {-t, 0, 1}, material});

    triangles(new_shape).push_back(Triangle{{1, t, 0}, {0, 1, t}, {t, 0, 1}, material});
    triangles(new_shape).push_back(Triangle{{0, 1, t}, {-t, 0, 1}, {0, -1, t}, material});
    triangles(new_shape).push_back(Triangle{{-t, 0, 1}, {-t, 0, -1}, {-1, -t, 0}, material});
    triangles(new_shape).push_back(Triangle{{-t, 0, -1}, {0, 1, -t}, {0, -1, -t}, material});
    triangles(new_shape).push_back(Triangle{{0, 1, -t}, {1, t, 0}, {t, 0, -1}, material});

    triangles(new_shape).push_back(Triangle{{1, -t, 0}, {t, 0, 1}, {0, -1, t}, material});
    triangles(new_shape).push_back(Triangle{{1, -t, 0}, {0, -1, t}, {-1, -t, 0}, material});
    triangles(new_shape).push_back(Triangle{{1, -t, 0}, {-1, -t, 0}, {0, -1, -t}, material});
    triangles(new_shape).push_back(Triangle{{1, -t, 0}, {0, -1, -t}, {t, 0, -1}, material});
    triangles(new_shape).push_back(Triangle{{1, -t, 0}, {t, 0, -1}, {t, 0, 1}, material});

    triangles(new_shape).push_back(Triangle{{0, -1, t}, {t, 0, 1}, {0, 1, t}, material});
    triangles(new_shape).push_back(Triangle{{-1, -t, 0}, {0, -1, t}, {-t, 0, 1}, material});
    triangles(new_shape).push_back(Triangle{{0, -1, -t}, {-1, -t, 0}, {-t, 0, -1}, material});
    triangles(new_shape).push_back(Triangle{{t, 0, -1}, {0, -1, -t}, {0, 1, -t}, material});
    triangles(new_shape).push_back(Triangle{{t, 0, 1}, {t, 0, -1}, {1, t, 0}, material});
    rebuild(new_shape);

    for (size_t i = 0; i < iterations; i++) {
        std::vector<Triangle> new_triangles;
        for (const auto &tri : triangles(new_shape)) {
            glm::vec3 a = glm::normalize(get_middle_point(tri.m_v1, tri.m_v2));
            glm::vec3 b = glm::normalize(get_middle_point(tri.m_v2, tri.m_v3));
            glm::vec3 c = glm::normalize(get_middle_point(tri.m_v3, tri.m_v1));

            new_triangles.push_back(Triangle{glm::normalize(tri.m_v1), a, c, material});
            new_triangles.push_back(Triangle{glm::normalize(tri.m_v2), b, a, material});
            new_triangles.push_back(Triangle{glm::normalize(tri.m_v3), c, b, material});
            new_triangles.push_back(Triangle{a, b, c, material});
        }

        new_shape.m_triangles = new_triangles;
    }

    glm::mat4 translation = glm::translate(pos);
    glm::mat4 rotation = glm::orientation(orientation, {0, 1, 0});
    glm::mat4 scale = glm::scale(Shape::scale(new_shape));
    glm::mat4 model = translation * rotation * scale;

    for (auto &tri : triangles(new_shape)) {
        tri.m_v1 = glm::vec3(model * glm::vec4(tri.m_v1, 1));
        tri.m_v2 = glm::vec3(model * glm::vec4(tri.m_v2, 1));
        tri.m_v3 = glm::vec3(model * glm::vec4(tri.m_v3, 1));
        tri.rebuild();
    }

    rebuild(new_shape);

    return new_shape;
}

Shape Shape::make_plane(glm::vec3 pos, glm::vec3 orientation, glm::vec2 size, Material material) {
    Shape new_shape;
    Shape::set_pos(new_shape, pos);
    Shape::set_orientation(new_shape, glm::normalize(orientation));
    Shape::set_scale(new_shape, glm::vec3(size.x, 0, size.y));

    auto p1 = glm::vec3{-0.5f, 0, 0.5f};
    auto p2 = glm::vec3{-0.5f, 0, -0.5f};
    auto p3 = glm::vec3{0.5f, 0, -0.5f};
    auto p4 = glm::vec3{0.5f, 0, 0.5f};

    auto triangle_left = Triangle(p2, p1, p3, material);
    auto triangle_right = Triangle(p1, p4, p3, material);

    Shape::add_triangle(new_shape, triangle_left);
    Shape::add_triangle(new_shape, triangle_right);

    glm::mat4 translation = glm::translate(pos);
    glm::mat4 rotation = glm::orientation(orientation, {0, 1, 0});
    glm::mat4 scale = glm::scale(Shape::scale(new_shape));
    glm::mat4 model = translation * rotation * scale;

    for (auto &tri : triangles(new_shape)) {
        tri.m_v1 = glm::vec3(model * glm::vec4(tri.m_v1, 1));
        tri.m_v2 = glm::vec3(model * glm::vec4(tri.m_v2, 1));
        tri.m_v3 = glm::vec3(model * glm::vec4(tri.m_v3, 1));
        tri.rebuild();
    }

    rebuild(new_shape);

    return new_shape;
}

void Shape::rebuild(Shape &shape) {
    auto &aabb = Shape::aabb(shape);
    AABB::reset(aabb);
    for (auto &tri : Shape::triangles(shape)) {
        AABB::extend(aabb, tri.m_v1);
        AABB::extend(aabb, tri.m_v2);
        AABB::extend(aabb, tri.m_v3);
    }
}
}
