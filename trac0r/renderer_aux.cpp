#include "renderer.hpp"

#include "random.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <iostream>

namespace trac0r {
// #pragma omp declare simd // TODO make this work
glm::vec4 Renderer::trace_camera_ray(const Ray &ray, const unsigned max_depth, const Scene &scene) {
    Ray next_ray = ray;
    glm::vec3 luminance{1};

    // Russian Roulette
    size_t depth = 0;

    // We'll run until terminated by Russian Roulette
    while (true) {
        // Russian Roulette
        float continuation_probability = 1.f - (1.f / (max_depth - depth));
        // float continuation_probability = (luminance.x + luminance.y + luminance.z) / 3.f;
        if (rand_range(0.f, 1.0f) >= continuation_probability) {
            break;
        }
        depth++;

        auto intersect_info = Scene::intersect(scene, next_ray);
        if (intersect_info.m_has_intersected) {
            // Emitter Material
            if (intersect_info.m_material.m_type == 1) {
                luminance *= intersect_info.m_material.m_color *
                             intersect_info.m_material.m_emittance / continuation_probability;
                break;
            }

            // Diffuse Material
            else if (intersect_info.m_material.m_type == 2) {
                // Find normal in correct direction
                intersect_info.m_normal =
                    intersect_info.m_normal * -glm::sign(intersect_info.m_angle_between);
                intersect_info.m_angle_between =
                    intersect_info.m_angle_between * -glm::sign(intersect_info.m_angle_between);

                // Find new random direction for diffuse reflection

                // We're using importance sampling for this since it converges much faster than
                // uniform sampling
                // See http://blog.hvidtfeldts.net/index.php/2015/01/path-tracing-3d-fractals/ and
                // http://www.rorydriscoll.com/2009/01/07/better-sampling/ and
                // https://pathtracing.wordpress.com/2011/03/03/cosine-weighted-hemisphere/
                glm::vec3 new_ray_dir =
                    oriented_cosine_weighted_hemisphere_sample(intersect_info.m_normal);
                luminance *= intersect_info.m_material.m_color;

                // For completeness, this is what it looks like with uniform sampling:
                // glm::vec3 new_ray_dir =
                // oriented_uniform_hemisphere_sample(intersect_info.m_normal);
                // float cos_theta = glm::dot(new_ray_dir, intersect_info.m_normal);
                // luminance *= 2.f * intersect_info.m_material.m_color * cos_theta;

                // Make a new ray
                next_ray = Ray{intersect_info.m_pos, new_ray_dir};
            }

            // Glass Material
            else if (intersect_info.m_material.m_type == 3) {
                // This code is mostly taken from TomCrypto's Lambda
                float n1, n2;

                if (intersect_info.m_angle_between > 0) {
                    // Ray in inside the object
                    n1 = intersect_info.m_material.m_ior;
                    n2 = 1.0003f;

                    intersect_info.m_normal = -intersect_info.m_normal;
                } else {
                    // Ray is outside the object
                    n1 = 1.0003f;
                    n2 = intersect_info.m_material.m_ior;

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
                float r1 = n1 * intersect_info.m_angle_between - n2 * cos_t;
                float r2 = n1 * intersect_info.m_angle_between + n2 * cos_t;
                float r3 = n2 * intersect_info.m_angle_between - n1 * cos_t;
                float r4 = n2 * intersect_info.m_angle_between + n1 * cos_t;
                float r = glm::pow(r1 / r2, 2) + glm::pow(r3 / r4, 2) * 0.5f;

                if (rand_range(0.f, 1.f) < r) {
                    // Reflection
                    new_ray_dir = intersect_info.m_incoming_ray.m_dir -
                                  (2.f * intersect_info.m_angle_between * intersect_info.m_normal);
                    luminance *= intersect_info.m_material.m_color;
                } else {
                    // Refraction
                    new_ray_dir = intersect_info.m_incoming_ray.m_dir * (n1 / n2) +
                                  intersect_info.m_normal *
                                      ((n1 / n2) * intersect_info.m_angle_between - cos_t);
                    luminance *= 1.f;
                }

                // Make a new ray
                next_ray = Ray{intersect_info.m_pos, new_ray_dir};
            }

            // Glossy Material
            else if (intersect_info.m_material.m_type == 4) {
                // Find normal in correct direction
                intersect_info.m_normal =
                    intersect_info.m_normal * -glm::sign(intersect_info.m_angle_between);
                intersect_info.m_angle_between =
                    intersect_info.m_angle_between * -glm::sign(intersect_info.m_angle_between);

                float real_roughness =
                    intersect_info.m_material.m_roughness * glm::half_pi<float>();

                // Find new direction for reflection
                glm::vec3 reflected_dir =
                    intersect_info.m_incoming_ray.m_dir -
                    (2.f * intersect_info.m_angle_between * intersect_info.m_normal);

                // Find new random direction on cone for glossy reflection
                glm::vec3 new_ray_dir =
                    oriented_cosine_weighted_cone_sample(reflected_dir, real_roughness);

                luminance *= intersect_info.m_material.m_color;

                // Make a new ray
                next_ray = Ray{intersect_info.m_pos, new_ray_dir};
            }
        } else {
            break;
        }
    }

    return glm::vec4(luminance, 1.f);
}

void Renderer::trace_light_ray(const Ray &ray, const unsigned max_depth, const Scene &scene,
                               const unsigned light_vertex_count, std::vector<LightVertex> &lvc) {
    Ray next_ray = ray;
    glm::vec3 return_color{0};
    glm::vec3 luminance{1};

    // Russian Roulette
    size_t depth = 0;

    // We'll run until terminated by Russian Roulette
    while (true) {
        // Russian Roulette
        float continuation_probability = 1.f - (1.f / (max_depth - depth));
        // float continuation_probability = (luminance.x + luminance.y + luminance.z) / 3.f;
        if (rand_range(0.f, 1.0f) >= continuation_probability) {
            break;
        }
        depth++;

        auto intersect_info = Scene::intersect(scene, next_ray);
        if (intersect_info.m_has_intersected) {
            // Emitter Material
            if (intersect_info.m_material.m_type == 1) {
                return_color = luminance * intersect_info.m_material.m_color *
                               intersect_info.m_material.m_emittance / continuation_probability;
                break;
            }

            // Diffuse Material
            else if (intersect_info.m_material.m_type == 2) {
                // Find normal in correct direction
                intersect_info.m_normal =
                    intersect_info.m_normal * -glm::sign(intersect_info.m_angle_between);
                intersect_info.m_angle_between =
                    intersect_info.m_angle_between * -glm::sign(intersect_info.m_angle_between);

                // Find new random direction for diffuse reflection

                // We're using importance sampling for this since it converges much faster than
                // uniform sampling
                // See http://blog.hvidtfeldts.net/index.php/2015/01/path-tracing-3d-fractals/ and
                // http://www.rorydriscoll.com/2009/01/07/better-sampling/ and
                // https://pathtracing.wordpress.com/2011/03/03/cosine-weighted-hemisphere/
                glm::vec3 new_ray_dir =
                    oriented_cosine_weighted_hemisphere_sample(intersect_info.m_normal);
                luminance *= intersect_info.m_material.m_color;

                // For completeness, this is what it looks like with uniform sampling:
                // glm::vec3 new_ray_dir =
                // oriented_uniform_hemisphere_sample(intersect_info.m_normal);
                // float cos_theta = glm::dot(new_ray_dir, intersect_info.m_normal);
                // luminance *= 2.f * intersect_info.m_material.m_color * cos_theta;

                // Make a new ray
                next_ray = Ray{intersect_info.m_pos, new_ray_dir};
            }

            // Glass Material
            else if (intersect_info.m_material.m_type == 3) {
                // This code is mostly taken from TomCrypto's Lambda
                float n1, n2;

                if (intersect_info.m_angle_between > 0) {
                    // Ray in inside the object
                    n1 = intersect_info.m_material.m_ior;
                    n2 = 1.0003f;

                    intersect_info.m_normal = -intersect_info.m_normal;
                } else {
                    // Ray is outside the object
                    n1 = 1.0003f;
                    n2 = intersect_info.m_material.m_ior;

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
                float r1 = n1 * intersect_info.m_angle_between - n2 * cos_t;
                float r2 = n1 * intersect_info.m_angle_between + n2 * cos_t;
                float r3 = n2 * intersect_info.m_angle_between - n1 * cos_t;
                float r4 = n2 * intersect_info.m_angle_between + n1 * cos_t;
                float r = glm::pow(r1 / r2, 2) + glm::pow(r3 / r4, 2) * 0.5f;

                if (rand_range(0.f, 1.f) < r) {
                    // Reflection
                    new_ray_dir = intersect_info.m_incoming_ray.m_dir -
                                  (2.f * intersect_info.m_angle_between * intersect_info.m_normal);
                    luminance *= intersect_info.m_material.m_color;
                } else {
                    // Refraction
                    new_ray_dir = intersect_info.m_incoming_ray.m_dir * (n1 / n2) +
                                  intersect_info.m_normal *
                                      ((n1 / n2) * intersect_info.m_angle_between - cos_t);
                    luminance *= 1.f;
                }

                // Make a new ray
                next_ray = Ray{intersect_info.m_pos, new_ray_dir};
            }

            // Glossy Material
            else if (intersect_info.m_material.m_type == 4) {
                // Find normal in correct direction
                intersect_info.m_normal =
                    intersect_info.m_normal * -glm::sign(intersect_info.m_angle_between);
                intersect_info.m_angle_between =
                    intersect_info.m_angle_between * -glm::sign(intersect_info.m_angle_between);

                float real_roughness =
                    intersect_info.m_material.m_roughness * glm::half_pi<float>();

                // Find new direction for reflection
                glm::vec3 reflected_dir =
                    intersect_info.m_incoming_ray.m_dir -
                    (2.f * intersect_info.m_angle_between * intersect_info.m_normal);

                // Find new random direction on cone for glossy reflection
                glm::vec3 new_ray_dir =
                    oriented_cosine_weighted_cone_sample(reflected_dir, real_roughness);

                luminance *= intersect_info.m_material.m_color;

                // Make a new ray
                next_ray = Ray{intersect_info.m_pos, new_ray_dir};
            }
        } else {
            break;
        }
    }
}
}
