#include "renderer.hpp"

#include "random.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace trac0r {
// #pragma omp declare simd // TODO make this work
glm::vec4 Renderer::trace_pixel_color(const unsigned x, const unsigned y, const unsigned max_depth,
                                      const Camera &camera, const Scene &scene) {
    glm::vec2 rel_pos = Camera::screenspace_to_camspace(camera, x, y);
    glm::vec3 world_pos = Camera::camspace_to_worldspace(camera, rel_pos);
    glm::vec3 ray_dir = glm::normalize(world_pos - Camera::pos(camera));

    glm::vec3 ret_color{0};
    Ray next_ray{world_pos, ray_dir};
    glm::vec3 coeff{1};
    for (size_t depth = 0; depth < max_depth; depth++) {
        auto intersect_info = Scene::intersect(scene, next_ray);
        if (intersect_info.m_has_intersected) {
            // Emitter Material
            if (glm::length(intersect_info.m_material.m_emittance) > 0.f) {
                ret_color += coeff * intersect_info.m_material.m_emittance;
                break;
            }

            // Diffuse Material
            if (glm::length(intersect_info.m_material.m_diffuse) > 0.f) {
                // Find new random direction for diffuse reflection
                // glm::vec3 new_ray_dir = uniform_sample_sphere();
                glm::vec3 new_ray_dir = oriented_hemisphere_sample(intersect_info.m_normal);

                // Make sphere distribution into hemisphere distribution
                float cos_theta = glm::dot(new_ray_dir, intersect_info.m_normal);

                // ret_color += coeff;
                coeff *= 2.f * intersect_info.m_material.m_diffuse * cos_theta;

                // Make a new ray
                next_ray = Ray{intersect_info.m_pos, new_ray_dir};
            }

            // Reflective Material
            if (glm::length(intersect_info.m_material.m_reflectance) > 0.f) {
                // Find new direction for reflection
                glm::vec3 new_ray_dir =
                    intersect_info.m_incoming_ray.m_dir -
                    (2.f * intersect_info.m_angle_between * intersect_info.m_normal);

                // ret_color += coeff;
                coeff *= 2.f * intersect_info.m_material.m_reflectance;

                // Make a new ray
                next_ray = Ray{intersect_info.m_pos, new_ray_dir};
            }

            // Refractive Material
            if (glm::length(intersect_info.m_material.m_refractance) > 0.f) {
            }

        } else {
            break;
        }
    }

    return glm::vec4(ret_color, 1.f);
}
}
