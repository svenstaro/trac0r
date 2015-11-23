#include "camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/fast_square_root.hpp>

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

    set_vertical_fov(vertical_fov_degrees);
}

glm::vec3 Camera::pos() const {
    return m_pos;
}

void Camera::set_pos(glm::vec3 pos) {
    m_pos = pos;
}

glm::vec3 Camera::dir() const {
    return m_dir;
}

void Camera::set_dir(glm::vec3 dir) {
    m_dir = dir;
}

glm::vec3 Camera::world_up() const {
    return m_world_up;
}

void Camera::set_world_up(glm::vec3 world_up) {
    m_world_up = world_up;
}

glm::vec3 Camera::right() const {
    return glm::normalize(glm::cross(m_world_up, m_dir));
    ;
}

glm::vec3 Camera::up() const {
    return glm::cross(m_dir, right());
}

float Camera::near_plane_dist() const {
    return m_near_plane_dist;
}

void Camera::set_near_plane_dist(float dist) {
    m_near_plane_dist = dist;
}

float Camera::far_plane_dist() const {
    return m_far_plane_dist;
}

void Camera::set_far_plane_dist(float dist) {
    m_far_plane_dist = dist;
}

float Camera::vertical_fov() const {
    return m_vertical_fov;
}

void Camera::set_vertical_fov(float degrees) {
    // See https://en.wikipedia.org/wiki/Field_of_view_in_video_games
    m_vertical_fov = glm::radians(degrees);
    m_horizontal_fov = 2 * glm::atan(glm::tan(m_vertical_fov / 2) *
                                     ((float)m_screen_width / (float)m_screen_height));
}

float Camera::horizontal_fov() const {
    return m_horizontal_fov;
}

float Camera::aspect_ratio() const {
    return m_screen_width / m_screen_height;
}

float Camera::canvas_width() const {
    return 2 * glm::tan(m_horizontal_fov / 2) * m_near_plane_dist;
}

float Camera::canvas_height() const {
    return 2 * glm::tan(m_vertical_fov / 2) * m_near_plane_dist;
}

glm::vec3 Camera::canvas_center_pos() const {
    return m_pos + m_dir * m_near_plane_dist;
}

glm::vec3 Camera::canvas_dir_x() const {
    return glm::normalize(glm::cross(m_dir, up())) * (canvas_width() / 2);
}

glm::vec3 Camera::canvas_dir_y() const {
    return glm::normalize(up()) * (canvas_height() / 2);
}

glm::vec2 Camera::screenspace_to_camspace(int x, int y) const {
    auto rel_x = -(x - m_screen_width / 2.f) / m_screen_width;
    auto rel_y = -(y - m_screen_height / 2.f) / m_screen_height;
    return {rel_x, rel_y};
}

glm::i32vec2 Camera::camspace_to_screenspace(glm::vec2 coords) const {
    int screen_x = glm::round(0.5f * (m_screen_width - 2.f * m_screen_width * coords.x));
    int screen_y = glm::round(0.5f * (m_screen_height - 2.f * m_screen_height * coords.y));
    return {screen_x, screen_y};
}

glm::vec3 Camera::camspace_to_worldspace(glm::vec2 rel_pos) const {
    auto worldspace =
        canvas_center_pos() + (rel_pos.x * canvas_dir_x()) + (rel_pos.y * canvas_dir_y());
    return worldspace;
}

glm::vec2 Camera::worldspace_to_camspace(glm::vec3 world_pos_on_canvas) const {
    auto canvas_center_to_point = world_pos_on_canvas - canvas_center_pos();

    // Manually calculate angle between the positive y-axis on the canvas and the world point
    auto ay = canvas_center_to_point;
    auto by = up();
    auto cy = glm::cross(ay, by);
    auto angle1y = glm::atan(glm::dot(ay, by), glm::length(cy));

    // Manually calculate angle between the positive x-axis on the canvas and the world point
    auto ax = canvas_center_to_point;
    auto bx = right();
    auto cx = glm::cross(ax, bx);
    auto angle1x = glm::atan(glm::length(cx), glm::dot(ax, bx));

    auto angle2x = glm::pi<float>() - (glm::half_pi<float>() + angle1x);
    auto length = glm::length(canvas_center_to_point);
    auto x = glm::sin(angle2x) * length;
    auto y = glm::sin(angle1y) * length;
    auto rel_x = -(2 * x) / canvas_width();
    auto rel_y = -(2 * y) / canvas_height();
    return {rel_x, -rel_y};
}

glm::vec3 Camera::worldpoint_to_worldspace(glm::vec3 world_point) const {
    // auto far_right_on_canvas = camspace_to_worldspace({1.f, 0.f});
    // auto far_top_on_canvas = camspace_to_worldspace({0.f, 1.f});
    // auto max_distance_x = glm::length(far_right_on_canvas - pos());
    // auto max_distance_y = glm::length(far_top_on_canvas - pos());
    auto ray_to_cam = pos() - world_point;
    float dist = 0;
    bool collided = glm::intersectRayPlane(world_point, glm::normalize(ray_to_cam), canvas_center_pos(), dir(), dist);
    if (collided) {
        return world_point + glm::normalize(ray_to_cam) * dist;
    } else {
        return glm::vec3(0);
    }
}
}
