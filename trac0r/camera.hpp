#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>

class Camera {
  public:
    Camera();
    Camera(glm::vec3 pos, glm::vec3 dir, glm::vec3 up, float vertical_fov, float near_plane_dist,
           float far_plane_dist, int screen_width, int screen_height);

    glm::vec3 pos();
    void set_pos(glm::vec3 pos);

    glm::vec3 dir();
    void set_dir(glm::vec3 dir);

    glm::vec3 up();
    void set_up(glm::vec3 up);

    float near_plane_dist();
    void set_near_plane_dist(float dist);

    float far_plane_dist();
    void set_far_plane_dist(float dist);

    float vertical_fov();
    void set_vertical_fov(float degrees);

    float horizontal_fov();

    float aspect_ratio();

  private:
    glm::vec3 m_pos;
    glm::vec3 m_dir;
    glm::vec3 m_up;
    float m_near_plane_dist;
    float m_far_plane_dist;
    int m_screen_width;
    int m_screen_height;

    float m_vertical_fov;
    float m_horizontal_fov;
};

#endif /* end of include guard: CAMERA_HPP */
