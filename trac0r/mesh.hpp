#ifndef MESH_HPP
#define MESH_HPP

#include "triangle.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <memory>
#include <vector>

class Mesh {
  public:
    Mesh(glm::vec3 pos) : m_pos(pos) {
    }

    static std::unique_ptr<Mesh> make_box(glm::vec3 pos, glm::vec3 orientation, glm::vec3 size,
                                          glm::vec3 reflectance = {0, 0, 0},
                                          glm::vec3 emittance = {0.3f, 0.3f, 0.3f}) {
        auto new_mesh = std::make_unique<Mesh>(pos);
        new_mesh->m_pos = pos;
        new_mesh->m_orientation = glm::normalize(orientation);
        new_mesh->m_scale = size;

        auto p1 = glm::vec3{-0.5f, 0.5f, -0.5f};
        auto p2 = glm::vec3{-0.5f, -0.5f, -0.5f};
        auto p3 = glm::vec3{0.5f, -0.5f, -0.5f};
        auto p4 = glm::vec3{0.5f, 0.5f, -0.5f};
        auto p5 = glm::vec3{-0.5f, 0.5f, 0.5f};
        auto p6 = glm::vec3{-0.5f, -0.5f, 0.5f};
        auto p7 = glm::vec3{0.5f, -0.5f, 0.5f};
        auto p8 = glm::vec3{0.5f, 0.5f, 0.5f};

        // front face
        auto t1 = std::make_unique<Triangle>(p1, p2, p3, reflectance, emittance);
        auto t2 = std::make_unique<Triangle>(p1, p4, p3, reflectance, emittance);

        // right face
        auto t3 = std::make_unique<Triangle>(p3, p4, p8, reflectance, emittance);
        auto t4 = std::make_unique<Triangle>(p3, p7, p8, reflectance, emittance);

        // left face
        auto t5 = std::make_unique<Triangle>(p1, p2, p6, reflectance, emittance);
        auto t6 = std::make_unique<Triangle>(p1, p5, p6, reflectance, emittance);

        // back face
        auto t7 = std::make_unique<Triangle>(p5, p6, p8, reflectance, emittance);
        auto t8 = std::make_unique<Triangle>(p6, p7, p8, reflectance, emittance);

        // top face
        auto t9 = std::make_unique<Triangle>(p5, p1, p4, reflectance, emittance);
        auto t10 = std::make_unique<Triangle>(p5, p8, p4, reflectance, emittance);

        // bottom face
        auto t11 = std::make_unique<Triangle>(p6, p3, p2, reflectance, emittance);
        auto t12 = std::make_unique<Triangle>(p3, p7, p6, reflectance, emittance);

        new_mesh->add_triangle(t1);
        new_mesh->add_triangle(t2);
        new_mesh->add_triangle(t3);
        new_mesh->add_triangle(t4);
        new_mesh->add_triangle(t5);
        new_mesh->add_triangle(t6);
        new_mesh->add_triangle(t7);
        new_mesh->add_triangle(t8);
        new_mesh->add_triangle(t9);
        new_mesh->add_triangle(t10);
        new_mesh->add_triangle(t11);
        new_mesh->add_triangle(t12);

        glm::mat4 translate = glm::translate(pos);
        glm::mat4 rotation = glm::orientation(orientation, {0, 1, 0});
        glm::mat4 scale = glm::scale(size);
        glm::mat4 model = translate * rotation * scale;

        for (auto &tri : new_mesh->m_triangles) {
            tri->m_v1 = glm::vec3(model * glm::vec4(tri->m_v1, 1));
            tri->m_v2 = glm::vec3(model * glm::vec4(tri->m_v2, 1));
            tri->m_v3 = glm::vec3(model * glm::vec4(tri->m_v3, 1));
            tri->update();
        }

        return new_mesh;
    }

    static std::unique_ptr<Mesh> make_plane(glm::vec3 pos, glm::vec3 orientation, glm::vec2 size,
                                            glm::vec3 reflectance = {0, 0, 0},
                                            glm::vec3 emittance = {0.2, 0.2, 0.2}) {
        auto new_mesh = std::make_unique<Mesh>(pos);
        new_mesh->m_pos = pos;
        new_mesh->m_orientation = glm::normalize(orientation);
        new_mesh->m_scale = glm::vec3(size.x, 0, size.y);

        auto p1 = glm::vec3{-0.5f, 0, 0.5f};
        auto p2 = glm::vec3{-0.5f, 0, -0.5f};
        auto p3 = glm::vec3{0.5f, 0, -0.5f};
        auto p4 = glm::vec3{0.5f, 0, 0.5f};

        auto triangle_left = std::make_unique<Triangle>(p1, p2, p3, reflectance, emittance);
        auto triangle_right = std::make_unique<Triangle>(p1, p4, p3, reflectance, emittance);

        new_mesh->add_triangle(triangle_left);
        new_mesh->add_triangle(triangle_right);

        glm::mat4 translate = glm::translate(pos);
        glm::mat4 rotation = glm::orientation(orientation, {0, 1, 0});
        glm::mat4 scale = glm::scale(new_mesh->m_scale);
        glm::mat4 model = translate * rotation * scale;

        for (auto &tri : new_mesh->m_triangles) {
            tri->m_v1 = glm::vec3(model * glm::vec4(tri->m_v1, 1));
            tri->m_v2 = glm::vec3(model * glm::vec4(tri->m_v2, 1));
            tri->m_v3 = glm::vec3(model * glm::vec4(tri->m_v3, 1));
            tri->update();
        }

        return new_mesh;
    }

    void add_triangle(std::unique_ptr<Triangle> &triangle) {
        m_triangles.push_back(std::move(triangle));
    }

    const glm::vec3 pos() const {
        return m_pos;
    }

    std::vector<std::unique_ptr<Triangle>> &triangles() {
        return m_triangles;
    }

  private:
    glm::vec3 m_pos;
    glm::vec3 m_orientation;
    glm::vec3 m_scale;
    std::vector<std::unique_ptr<Triangle>> m_triangles;
};

#endif /* end of include guard: MESH_HPP */
