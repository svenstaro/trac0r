#include "camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/constants.hpp>

#include <glm/gtx/string_cast.hpp>
#include <cppformat/format.h>

namespace trac0r {

Camera::Camera() {
}

Camera::Camera(glm::vec3 pos, glm::vec3 dir, glm::vec3 world_up, float vertical_fov_degrees,
               float near_plane_dist, float far_plane_dist, int screen_width, int screen_height)
    : m_pos(pos), m_dir(dir), m_world_up(world_up), m_near_plane_dist(near_plane_dist),
      m_far_plane_dist(far_plane_dist), m_screen_width(screen_width),
      m_screen_height(screen_height) {

    set_vertical_fov(*this, vertical_fov_degrees);
}

glm::vec3 Camera::pos(const Camera &camera) {
    return camera.m_pos;
}

void Camera::set_pos(Camera &camera, glm::vec3 pos) {
    camera.m_pos = pos;
}

glm::vec3 Camera::dir(const Camera &camera) {
    return camera.m_dir;
}

void Camera::set_dir(Camera &camera, glm::vec3 dir) {
    camera.m_dir = dir;
}

glm::vec3 Camera::world_up(const Camera &camera) {
    return camera.m_world_up;
}

void Camera::set_world_up(Camera &camera, glm::vec3 world_up) {
    camera.m_world_up = world_up;
}

glm::vec3 Camera::right(const Camera &camera) {
    return glm::normalize(glm::cross(camera.m_world_up, camera.m_dir));
    ;
}

glm::vec3 Camera::up(const Camera &camera) {
    return glm::cross(camera.m_dir, right(camera));
}

float Camera::near_plane_dist(const Camera &camera) {
    return camera.m_near_plane_dist;
}

void Camera::set_near_plane_dist(Camera &camera, float dist) {
    camera.m_near_plane_dist = dist;
}

float Camera::far_plane_dist(const Camera &camera) {
    return camera.m_far_plane_dist;
}

void Camera::set_far_plane_dist(Camera &camera, float dist) {
    camera.m_far_plane_dist = dist;
}

int Camera::screen_width(const Camera &camera) {
    return camera.m_screen_width;
}

int Camera::screen_height(const Camera &camera) {
    return camera.m_screen_height;
}

float Camera::vertical_fov(const Camera &camera) {
    return camera.m_vertical_fov;
}

void Camera::set_vertical_fov(Camera &camera, float degrees) {
    // See https://en.wikipedia.org/wiki/Field_of_view_in_video_games
    camera.m_vertical_fov = glm::radians(degrees);
    camera.m_horizontal_fov =
        2 * glm::atan(glm::tan(vertical_fov(camera) / 2) *
                      ((float)screen_width(camera) / (float)screen_height(camera)));
}

float Camera::horizontal_fov(const Camera &camera) {
    return camera.m_horizontal_fov;
}

float Camera::aspect_ratio(const Camera &camera) {
    return screen_width(camera) / screen_height(camera);
}

float Camera::canvas_width(const Camera &camera) {
    return 2 * glm::tan(horizontal_fov(camera) / 2) * near_plane_dist(camera);
}

float Camera::canvas_height(const Camera &camera) {
    return 2 * glm::tan(vertical_fov(camera) / 2) * near_plane_dist(camera);
}

glm::vec3 Camera::canvas_center_pos(const Camera &camera) {
    return pos(camera) + dir(camera) * near_plane_dist(camera);
}

glm::vec3 Camera::canvas_dir_x(const Camera &camera) {
    return glm::normalize(glm::cross(dir(camera), up(camera))) * (canvas_width(camera) / 2);
}

glm::vec3 Camera::canvas_dir_y(const Camera &camera) {
    return glm::normalize(up(camera)) * (canvas_height(camera) / 2);
}

glm::vec2 Camera::screenspace_to_camspace(const Camera &camera, unsigned x, unsigned y) {
    auto rel_x = -(x - screen_width(camera) / 2.f) / screen_width(camera);
    auto rel_y = -(y - screen_height(camera) / 2.f) / screen_height(camera);
    return {rel_x, rel_y};
}

glm::i32vec2 Camera::camspace_to_screenspace(const Camera &camera, glm::vec2 coords) {
    int screen_x =
        glm::round(0.5f * (screen_width(camera) - 2.f * screen_width(camera) * coords.x));
    int screen_y =
        glm::round(0.5f * (screen_height(camera) - 2.f * screen_height(camera) * coords.y));
    return {screen_x, screen_y};
}

glm::vec3 Camera::camspace_to_worldspace(const Camera &camera, glm::vec2 rel_pos) {
    auto worldspace = canvas_center_pos(camera) + (rel_pos.x * canvas_dir_x(camera)) +
                      (rel_pos.y * canvas_dir_y(camera));
    return worldspace;
}

glm::vec2 Camera::worldspace_to_camspace(const Camera &camera, glm::vec3 world_pos_on_canvas) {
    auto canvas_center_to_point = world_pos_on_canvas - canvas_center_pos(camera);

    // Manually calculate angle between the positive y-axis on the canvas and the world point
    auto ay = canvas_center_to_point;
    auto by = up(camera);
    auto cy = glm::cross(ay, by);
    auto angle1y = glm::atan(glm::dot(ay, by), glm::length(cy));

    // Manually calculate angle between the positive x-axis on the canvas and the world point
    auto ax = canvas_center_to_point;
    auto bx = right(camera);
    auto cx = glm::cross(ax, bx);
    auto angle1x = glm::atan(glm::length(cx), glm::dot(ax, bx));

    auto angle2x = glm::pi<float>() - (glm::half_pi<float>() + angle1x);
    auto length = glm::length(canvas_center_to_point);
    auto x = glm::sin(angle2x) * length;
    auto y = glm::sin(angle1y) * length;
    auto rel_x = -(2 * x) / canvas_width(camera);
    auto rel_y = -(2 * y) / canvas_height(camera);
    return {rel_x, -rel_y};
}

glm::vec3 Camera::worldpoint_to_worldspace(const Camera &camera, glm::vec3 world_point) {
    // auto far_right_on_canvas = camspace_to_worldspace({1.f, 0.f});
    // auto far_top_on_canvas = camspace_to_worldspace({0.f, 1.f});
    // auto max_distance_x = glm::length(far_right_on_canvas - pos());
    // auto max_distance_y = glm::length(far_top_on_canvas - pos());
    auto ray_to_cam = pos(camera) - world_point;
    float dist = 0;
    bool collided = glm::intersectRayPlane(world_point, glm::normalize(ray_to_cam),
                                           canvas_center_pos(camera), dir(camera), dist);
    if (collided) {
        return world_point + glm::normalize(ray_to_cam) * dist;
    } else {
        return glm::vec3(0);
    }
}
}
