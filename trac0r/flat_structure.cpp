#include "flat_structure.hpp"
#include "intersections.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/random.hpp>

#include <algorithm>
#include <memory>

namespace trac0r {

FlatStructure::~FlatStructure() {
}

glm::vec3 FlatStructure::intersect(const Ray &ray, int depth, int max_depth) const {
    if (depth == max_depth)
        return {0, 0, 0};

    // check all triangles for collision
    bool collided = false;
    glm::vec3 ret_color{0.f, 0.f, 0.f};
    // TODO: We get like 2x better performance here if we loop over a flat structure of triangles
    // instead of looping over all shapes and for each shape over all triangles
    for (const auto &shape : m_shapes) {
        if (intersect_ray_aabb(ray, shape->aabb())) {
            for (const auto &tri : shape->triangles()) {
                float dist_to_col;
                collided = intersect_ray_triangle(ray, tri, dist_to_col);

                if (collided) {
                    glm::vec3 impact_pos = ray.m_origin + ray.m_dir * dist_to_col;

                    // Get the local radiance only on first bounce
                    glm::vec3 local_radiance(0);
                    if (depth == 0) {
                        // auto ray = ray_pos - impact_pos;
                        // float dist2 = glm::dot(ray, ray);
                        // auto cos_area = glm::dot(-ray_dir, tri->m_normal) * tri->m_area;
                        // auto solid_angle = cos_area / glm::max(dist2, 1e-6f);
                        //
                        // if (cos_area > 0.0)
                        //     local_radiance = tri->m_emittance * solid_angle;
                        local_radiance = tri->m_emittance;
                    }
                    local_radiance = tri->m_emittance;

                    // Emitter sample
                    // TODO
                    // glm::vec3 illumination;

                    auto normal = tri->m_normal * -glm::sign(glm::dot(tri->m_normal, ray.m_dir));
                    // Find new random direction for diffuse reflection
                    auto new_ray_dir = normal;
                    auto half_pi = glm::half_pi<float>();
                    auto pi = glm::pi<float>();
                    new_ray_dir = glm::rotate(normal, glm::linearRand(-half_pi, half_pi),
                                              glm::cross(normal, ray.m_dir));
                    new_ray_dir = glm::rotate(new_ray_dir, glm::linearRand(-pi, pi), normal);
                    float cos_theta = glm::dot(new_ray_dir, normal);
                    glm::vec3 bdrf = 2.f * tri->m_reflectance * cos_theta;

                    // Send new ray in new direction
                    Ray new_ray(impact_pos, new_ray_dir);
                    glm::vec3 reflected = intersect(new_ray, depth + 1, max_depth);

                    ret_color = local_radiance + (bdrf * reflected);
                    break;
                }
            }
        }
    }

    return ret_color;
}

void FlatStructure::rebuild(const Camera &camera) {
    //     m_triangles.clear();
    //     for (auto &shape : m_shapes) {
    //         for (auto &tri : shape->triangles()) {
    //             m_triangles.push_back(*tri);
    //         }
    //     }
    //     // Sort by distance to camera
    //     // This is obviously broken but it works well enough for now
    //         std::sort(m_triangles.begin(), m_triangles.end(),
    //                   [&camera](const auto &tri1, const auto &tri2) {
    //                       return glm::distance(camera.pos(), tri1.m_centroid) <
    //                              glm::distance(camera.pos(), tri2.m_centroid);
    //                   });
}
}
