#include "renderer.hpp"

#include "random.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <iostream>

namespace trac0r {
// #pragma omp declare simd // TODO make this work
glm::vec4 Renderer::trace_pixel_color(const unsigned x, const unsigned y, const unsigned max_depth,
                                      const Camera &camera, const Scene &scene) {
    glm::vec2 rel_pos = Camera::screenspace_to_camspace(camera, x, y);

    // Subpixel sampling / antialiasing
    glm::vec2 pixel_size = Camera::pixel_size(camera);
    glm::vec2 jitter = {rand_range(-pixel_size.x / 2.f, pixel_size.x / 2.f),
                        rand_range(-pixel_size.y / 2.f, pixel_size.y / 2.f)};
    rel_pos += jitter;

    glm::vec3 world_pos = Camera::camspace_to_worldspace(camera, rel_pos);
    glm::vec3 ray_dir = glm::normalize(world_pos - Camera::pos(camera));

    Ray next_ray{world_pos, ray_dir};
    glm::vec3 return_color{0};
    glm::vec3 albedo{1};

    // Russian Roulette
    size_t depth = 0;

    // We'll run until terminated by Russian Roulette
    while (true) {
        // Russian Roulette
        float continuation_probability = 1.f - (1.f / (max_depth - depth));
        // float continuation_probability = (albedo.x + albedo.y + albedo.z) / 3.f;
        if (rand_range(0.f, 1.0f) >= continuation_probability) {
            break;
        }
        depth++;

        auto intersect_info = Scene::intersect(scene, next_ray);
        if (intersect_info.m_has_intersected) {
            // Emitter Material
            if (glm::length(intersect_info.m_material.m_emittance) > 0.f) {
                return_color = albedo * intersect_info.m_material.m_emittance / continuation_probability;
                break;
            }

            // Diffuse Material
            if (glm::length(intersect_info.m_material.m_diffuse) > 0.f) {
                // Find normal in correct direction
                intersect_info.m_normal =
                    intersect_info.m_normal * -glm::sign(intersect_info.m_angle_between);
                intersect_info.m_angle_between =
                    intersect_info.m_angle_between * -glm::sign(intersect_info.m_angle_between);

                // Find new random direction for diffuse reflection
                // glm::vec3 new_ray_dir = uniform_sample_sphere();
                glm::vec3 new_ray_dir = oriented_uniform_hemisphere_sample(intersect_info.m_normal);

                // Make sphere distribution into hemisphere distribution
                float cos_theta = glm::dot(new_ray_dir, intersect_info.m_normal);

                albedo *= 2.f * intersect_info.m_material.m_diffuse * cos_theta;

                // Make a new ray
                next_ray = Ray{intersect_info.m_pos, new_ray_dir};
            }

            // Reflective Material
            if (glm::length(intersect_info.m_material.m_reflectance) > 0.f) {
                // Find normal in correct direction
                intersect_info.m_normal =
                    intersect_info.m_normal * -glm::sign(intersect_info.m_angle_between);
                intersect_info.m_angle_between =
                    intersect_info.m_angle_between * -glm::sign(intersect_info.m_angle_between);

                // Find new direction for reflection
                glm::vec3 new_ray_dir =
                    intersect_info.m_incoming_ray.m_dir -
                    (2.f * intersect_info.m_angle_between * intersect_info.m_normal);

                albedo *= intersect_info.m_material.m_reflectance;

                // Make a new ray
                next_ray = Ray{intersect_info.m_pos, new_ray_dir};
            }

            // Refractive Material TODO: Disabled for now, needs to be worked into a glass material that only
            // sometimes refracts
            if (false && intersect_info.m_material.m_refractance != 0) {
                float n1, n2;

                if (intersect_info.m_angle_between > 0) {
                    // Ray in inside the object
                    n1 = intersect_info.m_material.m_refractance;
                    n2 = 1.0003f;

                    intersect_info.m_normal = -intersect_info.m_normal;
                } else {
                    // Ray is outside the object
                    n1 = 1.0003f;
                    n2 = intersect_info.m_material.m_refractance;

                    intersect_info.m_angle_between = -intersect_info.m_angle_between;
                }

                float n = n1 / n2;

                float cos_t =
                    1.f - glm::pow(n, 2) * (1.f - glm::pow(intersect_info.m_angle_between, 2));

                glm::vec3 new_ray_dir;
                // Handle total internal reflection
                if (cos_t < 0.f) {
                    new_ray_dir = intersect_info.m_incoming_ray.m_dir -
                                  (2.f * intersect_info.m_angle_between * intersect_info.m_normal);
                    next_ray = Ray{intersect_info.m_pos, new_ray_dir};
                    break;
                }

                cos_t = glm::sqrt(cos_t);

                // Fresnel coefficients
                // float r1 = n1 * intersect_info.m_angle_between - n2 * cos_t;
                // float r2 = n1 * intersect_info.m_angle_between + n2 * cos_t;
                // float r3 = n2 * intersect_info.m_angle_between - n1 * cos_t;
                // float r4 = n2 * intersect_info.m_angle_between + n1 * cos_t;
                // float r = glm::pow(r1 / r2, 2) + glm::pow(r3 / r4, 2) * 0.5f;

                // TODO Make this into real glass and split between reflection/refraction here
                // if (rand_range(0.f, 1.f) < r) {
                //     // Reflection
                //     new_ray_dir = intersect_info.m_incoming_ray.m_dir -
                //                   (2.f * intersect_info.m_angle_between *
                //                   intersect_info.m_normal);
                //     break;
                // } else {
                // Refraction
                new_ray_dir =
                    intersect_info.m_incoming_ray.m_dir * (n1 / n2) +
                    intersect_info.m_normal * ((n1 / n2) * intersect_info.m_angle_between - cos_t);
                // }

                // Make a new ray
                next_ray = Ray{intersect_info.m_pos, new_ray_dir};
            }

        } else {
            break;
        }
    }

    return glm::vec4(return_color, 1.f);
}
}
