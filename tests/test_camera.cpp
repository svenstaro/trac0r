#include "trac0r/camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <cppformat/format.h>

#include <iostream>

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    glm::vec3 cam_pos = {0, 0.31, -1.2};
    glm::vec3 cam_dir = {0, 0, 1};
    cam_dir = glm::rotateY(cam_dir, 0.6f);
    cam_dir = glm::rotateX(cam_dir, 0.6f);
    glm::vec3 world_up = {0, 1, 0};

    auto camera = trac0r::Camera(cam_pos, cam_dir, world_up, 90.f, 0.001, 100.f, 800, 600);

    fmt::print("Canvas Center Pos {}\n", glm::to_string(camera.canvas_center_pos()));
    fmt::print("Canvas Canvas Width/Height {}/{}\n", camera.canvas_width(), camera.canvas_height());
    fmt::print("Canvas Dir X {}\n", glm::to_string(camera.canvas_dir_x()));
    fmt::print("Canvas Dir Y {}\n\n", glm::to_string(camera.canvas_dir_y()));

    glm::i32vec2 ss1 {600, 100};

    auto cs1 = camera.screenspace_to_camspace(ss1.x, ss1.y);
    auto ws1 = camera.camspace_to_worldspace(cs1);
    fmt::print("Converting screen space to cam space:                  ");
    fmt::print("{} -> {}\n", glm::to_string(ss1), glm::to_string(cs1));
    fmt::print("Converting cam space to world space on canvas:         ");
    fmt::print("{} -> {}\n\n", glm::to_string(cs1), glm::to_string(ws1));

    auto cs2 = camera.worldspace_to_camspace(ws1);
    auto ss2 = camera.camspace_to_screenspace(cs2);
    fmt::print("Converting world space on canvas BACK to cam space:    ");
    fmt::print("{} -> {}\n", glm::to_string(ws1), glm::to_string(cs2));
    fmt::print("Converting cam space BACK to screen space:             ");
    fmt::print("{} -> {}\n", glm::to_string(cs2), glm::to_string(ss2));

    return 0;
}
