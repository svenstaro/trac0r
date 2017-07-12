#include "trac0r/camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <fmt/format.h>

#include <iostream>

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    glm::vec3 cam_pos = {0, 0.31, -1.2};
    glm::vec3 cam_dir = {0, 0, 1};
    cam_dir = glm::rotateY(cam_dir, 0.6f);
    cam_dir = glm::rotateX(cam_dir, 0.6f);
    glm::vec3 world_up = {0, 1, 0};

    using Camera = trac0r::Camera;
    Camera camera(cam_pos, cam_dir, world_up, 90.f, 0.001, 100.f, 800, 600);

    fmt::print("Canvas Center Pos {}\n", glm::to_string(Camera::canvas_center_pos(camera)));
    fmt::print("Canvas Canvas Width/Height {}/{}\n", Camera::canvas_width(camera), camera.canvas_height(camera));
    fmt::print("Canvas Dir X {}\n", glm::to_string(Camera::canvas_dir_x(camera)));
    fmt::print("Canvas Dir Y {}\n\n", glm::to_string(Camera::canvas_dir_y(camera)));

    glm::i32vec2 ss1 {600, 100};

    auto cs1 = Camera::screenspace_to_camspace(camera, ss1.x, ss1.y);
    auto ws1 = Camera::camspace_to_worldspace(camera, cs1);
    fmt::print("Converting screen space to cam space:                  ");
    fmt::print("{} -> {}\n", glm::to_string(ss1), glm::to_string(cs1));
    fmt::print("Converting cam space to world space on canvas:         ");
    fmt::print("{} -> {}\n\n", glm::to_string(cs1), glm::to_string(ws1));

    auto cs2 = Camera::worldspace_to_camspace(camera, ws1);
    auto ss2 = Camera::camspace_to_screenspace(camera, cs2);
    fmt::print("Converting world space on canvas BACK to cam space:    ");
    fmt::print("{} -> {}\n", glm::to_string(ws1), glm::to_string(cs2));
    fmt::print("Converting cam space BACK to screen space:             ");
    fmt::print("{} -> {}\n", glm::to_string(cs2), glm::to_string(ss2));

    return 0;
}
