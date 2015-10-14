#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>

struct Camera {
    glm::vec3 pos;
    glm::vec3 dir;
    glm::vec3 up;
    float fov;
};

#endif /* end of include guard: CAMERA_HPP */
