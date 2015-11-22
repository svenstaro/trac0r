#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>

namespace trac0r {

class Camera {
  public:
    Camera();
    Camera(glm::vec3 pos, glm::vec3 dir, glm::vec3 world_up, float vertical_fov, float near_plane_dist,
           float far_plane_dist, int screen_width, int screen_height);

    glm::vec3 pos() const;
    void set_pos(glm::vec3 pos);

    glm::vec3 dir() const;
    void set_dir(glm::vec3 dir);

    glm::vec3 world_up() const;
    void set_world_up(glm::vec3 dir);

    glm::vec3 right() const;
    glm::vec3 up() const;

    float near_plane_dist() const;
    void set_near_plane_dist(float dist);

    float far_plane_dist() const;
    void set_far_plane_dist(float dist);

    float vertical_fov() const;
    void set_vertical_fov(float degrees);

    float horizontal_fov() const;

    float aspect_ratio() const;

    float canvas_width() const;
    float canvas_height() const;
    glm::vec3 canvas_center_pos() const;
    glm::vec3 canvas_dir_x() const;
    glm::vec3 canvas_dir_y() const;

    /**
     * @brief Converts absolute screen space coordinates to relative camera space positions.
     *
     * @param x Pixel coordinate. Can not be larger than m_screen_width.
     * @param y Pixel coordinate. Can not be larger than m_screen_height.
     *
     * @return Relative camera space positions. Values will be between -1.0 and 1.0.
     */
    glm::vec2 screenspace_to_camspace(int x, int y) const;

    /**
     * @brief Converts relative camera space positions to absolute screen space coordinates.
     *
     * @param coords Camera space coordinates between -1.0 and 1.0.
     *
     * @return Absolute screen space coordinates.
     */
    glm::i32vec2 camspace_to_screenspace(glm::vec2 coords) const;

    /**
     * @brief Converts camera space relative positions to world space positions on the camera's
     * canvas.
     *
     * @param rel_pos The relative position as screenspace_to_camspace(int x, int y) would return
     * it. Values need to be between -1.0 and 1.0.
     *
     * @return World space coordinates on the camera's canvas.
     */
    glm::vec3 camspace_to_worldspace(glm::vec2 rel_pos) const;

    /**
     * @brief Converts a position on the camera's canvas to a camera space relative position.
     *
     * @param world_pos The world space position.
     *
     * @return The relative position in camera space. Values are between -1.0 and 1.0.
     */
    glm::vec3 worldspace_to_camspace(glm::vec3 world_pos) const;

  private:
    glm::vec3 m_pos;
    glm::vec3 m_dir;
    glm::vec3 m_world_up;
    float m_near_plane_dist;
    float m_far_plane_dist;
    int m_screen_width;
    int m_screen_height;

    float m_vertical_fov;
    float m_horizontal_fov;
};

}

#endif /* end of include guard: CAMERA_HPP */
