#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>

namespace trac0r {

class Camera {
  public:
    Camera();
    Camera(glm::vec3 pos, glm::vec3 dir, glm::vec3 world_up, float vertical_fov,
           float near_plane_dist, float far_plane_dist, int screen_width, int screen_height);

    static glm::vec3 pos(const Camera &camera);
    static void set_pos(Camera &camera, glm::vec3 pos);

    static glm::vec3 dir(const Camera &camera);
    static void set_dir(Camera &camera, glm::vec3 dir);

    static glm::vec3 world_up(const Camera &camera);
    static void set_world_up(Camera &camera, glm::vec3 dir);

    static glm::vec3 right(const Camera &camera);
    static glm::vec3 up(const Camera &camera);

    static float near_plane_dist(const Camera &camera);
    static void set_near_plane_dist(Camera &camera, float dist);

    static float far_plane_dist(const Camera &camera);
    static void set_far_plane_dist(Camera &camera, float dist);

    static int screen_width(const Camera &camera);
    static int screen_height(const Camera &camera);

    static float vertical_fov(const Camera &camera);
    static void set_vertical_fov(Camera &camera, float degrees);

    static float horizontal_fov(const Camera &camera);

    static float aspect_ratio(const Camera &camera);

    static float canvas_width(const Camera &camera);
    static float canvas_height(const Camera &camera);
    static glm::vec3 canvas_center_pos(const Camera &camera);
    static glm::vec3 canvas_dir_x(const Camera &camera);
    static glm::vec3 canvas_dir_y(const Camera &camera);

    /**
     * @brief Converts absolute screen space coordinates to relative camera space positions.
     *
     * @param x Pixel coordinate. Can not be larger than m_screen_width.
     * @param y Pixel coordinate. Can not be larger than m_screen_height.
     *
     * @return Relative camera space positions. Values will be between -1.0 and 1.0.
     */
    static glm::vec2 screenspace_to_camspace(const Camera &camera, unsigned x, unsigned y);

    /**
     * @brief Converts relative camera space positions to absolute screen space coordinates.
     *
     * @param coords Camera space coordinates between -1.0 and 1.0.
     *
     * @return Absolute screen space coordinates.
     */
    static glm::i32vec2 camspace_to_screenspace(const Camera &camera, glm::vec2 coords);

    /**
     * @brief Converts camera space relative positions to world space positions on the camera's
     * canvas.
     *
     * @param rel_pos The relative position as screenspace_to_camspace(int x, int y) would return
     * it. Values need to be between -1.0 and 1.0.
     *
     * @return World space coordinates on the camera's canvas.
     */
    static glm::vec3 camspace_to_worldspace(const Camera &camera, glm::vec2 rel_pos);

    /**
     * @brief Converts a position on the camera's canvas to a camera space relative position.
     *
     * @param world_pos The world space position.
     *
     * @return The relative position in camera space. Values are between -1.0 and 1.0.
     */
    static glm::vec2 worldspace_to_camspace(const Camera &camera, glm::vec3 world_pos);

    /**
     * @brief Converts a position in the world to a position on the camera's canvas (but still in
     * world space).
     * You then have to use worldspace_to_camspace() and then camspace_to_screenspace() to find out
     * which pixel
     * a world point falls into.
     *
     * @param world_point The world point in world space.
     *
     * @return A world space coordinate on the camera's canvas.
     */
    static glm::vec3 worldpoint_to_worldspace(const Camera &camera, glm::vec3 world_point);

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
