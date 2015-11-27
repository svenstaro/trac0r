#include "renderer.hpp"
#include "ray.hpp"
#include "random.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "cppformat/format.h"
#include <glm/gtx/string_cast.hpp>

namespace trac0r {

Renderer::Renderer(const int width, const int height, const Camera &camera, const Scene &scene)
    : m_width(width), m_height(height), m_camera(camera), m_scene(scene) {
    m_luminance.resize(width * height, glm::vec4{0});

#ifdef OPENCL
    auto devices = boost::compute::system::devices();
    m_compute_context = boost::compute::context(devices);
    for (auto &device : devices) {
        m_compute_queues.emplace_back(boost::compute::command_queue(m_compute_context, device));
    }
#endif
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
            new_ray_dir = glm::rotate(normal, rand_range(-half_pi, half_pi),
                                      glm::cross(normal, intersect_info.m_incoming_ray.m_dir));
            new_ray_dir = glm::rotate(new_ray_dir, rand_range(-pi, pi), normal);
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

std::vector<glm::vec4> &Renderer::render(bool scene_changed, int stride_x, int stride_y) {
#pragma omp parallel for collapse(2) schedule(dynamic, 1024)
    for (auto x = 0; x < m_width; x += stride_x) {
        for (auto y = 0; y < m_height; y += stride_y) {
            glm::vec4 new_color = trace_pixel_color(x, y);
            if (scene_changed)
                m_luminance[y * m_width + x] = new_color;
            else
                m_luminance[y * m_width + x] += new_color;
        }
    }

    return m_luminance;
}
}
