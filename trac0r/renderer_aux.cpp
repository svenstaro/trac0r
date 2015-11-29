#include "renderer.hpp"

#include "random.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace trac0r {
glm::vec4 Renderer::trace_pixel_color(const unsigned x, const unsigned y, const unsigned max_depth,
                                      const Camera &camera, const Scene &scene) {
    glm::vec2 rel_pos = Camera::screenspace_to_camspace(camera, x, y);
    glm::vec3 world_pos = Camera::camspace_to_worldspace(camera, rel_pos);
    glm::vec3 ray_dir = glm::normalize(world_pos - Camera::pos(camera));

    glm::vec3 ret_color{0};
    Ray next_ray{world_pos, ray_dir};
    glm::vec3 brdf{1};
    for (size_t depth = 0; depth < max_depth; depth++) {
        auto intersect_info = Scene::intersect(scene, next_ray);
        if (intersect_info.m_has_intersected) {
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
                local_radiance = intersect_info.m_material.m_emittance;
            }

            local_radiance = intersect_info.m_material.m_emittance;

            // Emitter sample
            // TODO
            // glm::vec3 illumination;

            auto normal =
                intersect_info.m_normal *
                -glm::sign(glm::dot(intersect_info.m_normal, intersect_info.m_incoming_ray.m_dir));

            // Find new random direction for diffuse reflection
            // auto new_ray_dir = normal;
            // auto half_pi = glm::half_pi<float>();
            // auto pi = glm::pi<float>();
            // new_ray_dir = glm::rotate(normal, rand_range(-half_pi, half_pi),
            //                           glm::cross(normal, intersect_info.m_incoming_ray.m_dir));
            // new_ray_dir = glm::rotate(new_ray_dir, rand_range(-pi, pi), normal);
            float u = 2.f * rand_range(0.f, 1.f);
            float v = glm::two_pi<float>() * rand_range(0.f, 1.f);
            float xx = glm::sqrt(1 - u * u) * glm::cos(v);
            float yy = glm::sqrt(1 - u * u) * glm::sin(v);
            float zz = u;
            glm::vec3 new_ray_dir = {xx, yy, zz};
            new_ray_dir = glm::normalize(new_ray_dir);
            if(glm::dot(new_ray_dir, normal) < 0) {
                new_ray_dir = -new_ray_dir;
            }

            float cos_theta = glm::dot(new_ray_dir, normal);

            ret_color += brdf * local_radiance;
            brdf *= 2.f * intersect_info.m_material.m_reflectance * cos_theta;

            // Make a new ray
            next_ray = Ray{intersect_info.m_pos, new_ray_dir};

        } else {
            break;
        }
    }

    return glm::vec4(ret_color, 1.f);
}
}
