#include "scene.hpp"

#include "utils.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/random.hpp>

namespace trac0r {

Scene::Scene() {
}

void Scene::add_triangle(std::unique_ptr<Triangle> &tri) {
    m_triangles.push_back(std::move(tri));
}

glm::vec3 Scene::intersect(glm::vec3 &ray_pos, glm::vec3 &ray_dir, int depth) const {
    if (depth == m_max_depth)
        return {0, 0, 0};

    // check all triangles for collision
    bool collided = false;
    glm::vec3 ret_color{0.f, 0.f, 0.f};
    for (const auto &tri : m_triangles) {
        float dist_to_col;
        collided =
            intersect_ray_triangle(ray_pos, ray_dir, tri->m_v1, tri->m_v2, tri->m_v3, dist_to_col);

        if (collided) {
            glm::vec3 impact_pos = ray_pos + ray_dir * dist_to_col;

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

            auto normal = tri->m_normal * -glm::sign(glm::dot(tri->m_normal, ray_dir));
            // Find new random direction for diffuse reflection
            auto new_ray_dir = normal;
            auto half_pi = glm::half_pi<float>();
            auto pi = glm::pi<float>();
            new_ray_dir = glm::rotate(normal, glm::linearRand(-half_pi, half_pi),
                                      glm::cross(normal, ray_dir));
            new_ray_dir = glm::rotate(new_ray_dir, glm::linearRand(-pi, pi), normal);
            float cos_theta = glm::dot(new_ray_dir, normal);
            glm::vec3 bdrf = 2.f * tri->m_reflectance * cos_theta;

            // Send new ray in new direction
            glm::vec3 reflected = intersect(impact_pos, new_ray_dir, depth + 1);

            ret_color = local_radiance + (bdrf * reflected);
            break;
        }
    }

    return ret_color;
}

void Scene::rebuild(const Camera &camera) {
    // Sort by distance to camera
    // TODO: Temporary until we get a proper acceleration structure
    std::sort(m_triangles.begin(), m_triangles.end(),
              [&camera](const auto &tri1, const auto &tri2) {
                  return glm::distance(camera.pos(), tri1->m_centroid) <
                         glm::distance(camera.pos(), tri2->m_centroid);
              });
}

}
