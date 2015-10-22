#ifndef MESH_HPP
#define MESH_HPP

#include "triangle.hpp"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

class Mesh {
  public:
    Mesh(glm::vec3 pos, glm::vec3 orientation) : m_pos(pos), m_orientation(orientation) {
    }

    static std::unique_ptr<Mesh> make_box(glm::vec3 pos, glm::vec3 orientation, glm::vec3 size) {
        auto new_mesh = std::make_unique<Mesh>(pos, orientation);
        auto p1 = glm::vec3{pos.x - size.x / 2, pos.y - size.y / 2, pos.z - size.z / 2};
        auto p2 = glm::vec3{pos.x - size.x / 2, pos.y + size.y / 2, pos.z - size.z / 2};
        auto p3 = glm::vec3{pos.x + size.x / 2, pos.y - size.y / 2, pos.z - size.z / 2};
        auto p4 = glm::vec3{pos.x + size.x / 2, pos.y + size.y / 2, pos.z - size.z / 2};
        auto p5 = glm::vec3{pos.x - size.x / 2, pos.y - size.y / 2, pos.z + size.z / 2};
        auto p6 = glm::vec3{pos.x - size.x / 2, pos.y + size.y / 2, pos.z + size.z / 2};
        auto p7 = glm::vec3{pos.x + size.x / 2, pos.y - size.y / 2, pos.z + size.z / 2};
        auto p8 = glm::vec3{pos.x + size.x / 2, pos.y + size.y / 2, pos.z + size.z / 2};

        // front face
        auto t1 = std::make_unique<Triangle>(p1, p2, p3, glm::vec3{0.3, 0.3, 0.3},
                                             glm::vec3{0.2, 0.2, 0.2});
        auto t2 = std::make_unique<Triangle>(p1, p4, p3, glm::vec3{0.3, 0.3, 0.3},
                                             glm::vec3{0.2, 0.2, 0.2});

        // right face
        auto t3 = std::make_unique<Triangle>(p3, p4, p8, glm::vec3{0.3, 0.3, 0.3},
                                             glm::vec3{0.2, 0.2, 0.2});
        auto t4 = std::make_unique<Triangle>(p3, p7, p8, glm::vec3{0.3, 0.3, 0.3},
                                             glm::vec3{0.2, 0.2, 0.2});

        // left face
        auto t5 = std::make_unique<Triangle>(p1, p2, p6, glm::vec3{0.3, 0.3, 0.3},
                                             glm::vec3{0.2, 0.2, 0.2});
        auto t6 = std::make_unique<Triangle>(p1, p5, p6, glm::vec3{0.3, 0.3, 0.3},
                                             glm::vec3{0.2, 0.2, 0.2});

        // back face
        auto t7 = std::make_unique<Triangle>(p5, p6, p8, glm::vec3{0.3, 0.3, 0.3},
                                             glm::vec3{0.2, 0.2, 0.2});
        auto t8 = std::make_unique<Triangle>(p6, p7, p8, glm::vec3{0.3, 0.3, 0.3},
                                             glm::vec3{0.2, 0.2, 0.2});

        // top face
        auto t9 = std::make_unique<Triangle>(p5, p1, p4, glm::vec3{0.3, 0.3, 0.3},
                                             glm::vec3{0.2, 0.2, 0.2});
        auto t10 = std::make_unique<Triangle>(p5, p8, p4, glm::vec3{0.3, 0.3, 0.3},
                                              glm::vec3{0.2, 0.2, 0.2});

        // bottom face
        auto t11 = std::make_unique<Triangle>(p6, p3, p2, glm::vec3{0.3, 0.3, 0.3},
                                              glm::vec3{0.2, 0.2, 0.2});
        auto t12 = std::make_unique<Triangle>(p3, p7, p6, glm::vec3{0.3, 0.3, 0.3},
                                              glm::vec3{0.2, 0.2, 0.2});

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

        return new_mesh;
    }

    static std::unique_ptr<Mesh> make_plane(glm::vec3 pos, glm::vec3 orientation, glm::vec2 size) {
        auto new_mesh = std::make_unique<Mesh>(pos, orientation);
        auto p1 = glm::vec3{pos.x - size.x / 2, pos.y, pos.z + size.y / 2};
        auto p2 = glm::vec3{pos.x - size.x / 2, pos.y, pos.z - size.y / 2};
        auto p3 = glm::vec3{pos.x + size.x / 2, pos.y, pos.z - size.y / 2};
        auto p4 = glm::vec3{pos.x + size.x / 2, pos.y, pos.z + size.y / 2};
        auto triangle_left = std::make_unique<Triangle>(p1, p2, p3, glm::vec3{0.3, 0.3, 0.3},
                                                        glm::vec3{0.2, 0.2, 0.2});

        auto triangle_right = std::make_unique<Triangle>(p1, p4, p3, glm::vec3{0.3, 0.3, 0.3},
                                                         glm::vec3{0.2, 0.2, 0.2});

        new_mesh->add_triangle(triangle_left);
        new_mesh->add_triangle(triangle_right);

        return new_mesh;
    }

    void add_triangle(std::unique_ptr<Triangle> &triangle) {
        m_triangles.push_back(std::move(triangle));
    }

    const glm::vec3 pos() const {
        return m_pos;
    }

    const glm::vec3 orientation() const {
        return m_orientation;
    }

    std::vector<std::unique_ptr<Triangle>> &triangles() {
        return m_triangles;
    }

  private:
    glm::vec3 m_pos;
    glm::vec3 m_orientation;
    std::vector<std::unique_ptr<Triangle>> m_triangles;
};

#endif /* end of include guard: MESH_HPP */
