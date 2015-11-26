#include "renderer.hpp"
#include "ray.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/random.hpp>

#include "cppformat/format.h"
#include <glm/gtx/string_cast.hpp>

namespace trac0r {

Renderer::Renderer(const Camera &camera, const Scene &scene) : m_camera(camera), m_scene(scene) {
}

glm::vec4 Renderer::trace_pixel_color(unsigned x, unsigned y) const {
    glm::vec2 rel_pos = m_camera.screenspace_to_camspace(x, y);
    glm::vec3 world_pos = m_camera.camspace_to_worldspace(rel_pos);
    glm::vec3 ray_dir = glm::normalize(world_pos - m_camera.pos());

    glm::vec3 ret_color{0};
    Ray next_ray{world_pos, ray_dir};
    glm::vec3 brdf{1};
    for (auto depth = 0; depth < m_max_depth; depth++) {
        auto intersect_info = m_scene.intersect(next_ray);
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
            auto new_ray_dir = normal;
            auto half_pi = glm::half_pi<float>();
            auto pi = glm::pi<float>();
            new_ray_dir = glm::rotate(normal, glm::linearRand(-half_pi, half_pi),
                                      glm::cross(normal, intersect_info.m_incoming_ray.m_dir));
            new_ray_dir = glm::rotate(new_ray_dir, glm::linearRand(-pi, pi), normal);
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
