#include "camera.hpp"

#include <glm/glm.hpp>

Camera::Camera() {}

Camera::Camera(glm::vec3 pos, glm::vec3 dir, glm::vec3 up, float vertical_fov_degrees,
               float near_plane_dist, float far_plane_dist, int screen_width, int screen_height)
    : m_pos(pos), m_dir(dir), m_up(up), m_near_plane_dist(near_plane_dist),
      m_far_plane_dist(far_plane_dist), m_screen_width(screen_width),
      m_screen_height(screen_height) {

    set_vertical_fov(vertical_fov_degrees);

    // m_camera.canvas_height = glm::tan(m_camera.fov) * m_camera.near_plane_dist;
    // m_camera.canvas_width = m_
}

glm::vec3 Camera::pos() {
    return m_pos;
}

void Camera::set_pos(glm::vec3 pos) {
    m_pos = pos;
}

glm::vec3 Camera::dir() {
    return m_dir;
}

void Camera::set_dir(glm::vec3 dir) {
    m_dir = dir;
}

glm::vec3 Camera::up() {
    return m_up;
}

void Camera::set_up(glm::vec3 up) {
    m_up = up;
}

float Camera::near_plane_dist() {
    return m_near_plane_dist;
}

void Camera::set_near_plane_dist(float dist) {
    m_near_plane_dist = dist;
}

float Camera::far_plane_dist() {
    return m_far_plane_dist;
}

void Camera::set_far_plane_dist(float dist) {
    m_far_plane_dist = dist;
}

float Camera::vertical_fov() {
    return m_vertical_fov;
}

void Camera::set_vertical_fov(float degrees) {
    // See https://en.wikipedia.org/wiki/Field_of_view_in_video_games
    m_vertical_fov = glm::radians(degrees);
    m_horizontal_fov = 2 * glm::atan(glm::tan(m_vertical_fov / 2) * (m_screen_width / m_screen_height));
}

float Camera::horizontal_fov() {
    return m_horizontal_fov;
}

float Camera::aspect_ratio() {
    return m_screen_width / m_screen_height;
}
