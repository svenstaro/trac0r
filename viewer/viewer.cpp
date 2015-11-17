#include "viewer.hpp"

#include "trac0r/mesh.hpp"
#include "trac0r/utils.hpp"

#include <SDL_ttf.h>
#include <SDL_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>

#include <cppformat/format.h>

#include <algorithm>
#include <iostream>

Viewer::~Viewer() {
    TTF_CloseFont(m_font);
    TTF_Quit();
    SDL_DestroyTexture(m_render_tex);
    SDL_DestroyRenderer(m_render);
    SDL_DestroyWindow(m_window);
    IMG_Quit();
    SDL_Quit();
}

int Viewer::init() {
    fmt::print("Start init\n");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL_Init error: " << SDL_GetError() << std::endl;
        return 1;
    }

    int screen_width = 600;
    int screen_height = 400;

    m_window = SDL_CreateWindow("trac0r", 100, 100, screen_width, screen_height, SDL_WINDOW_SHOWN);
    if (m_window == nullptr) {
        std::cerr << "SDL_CreateWindow error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // m_render = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED |
    // SDL_RENDERER_PRESENTVSYNC);
    m_render = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    m_render_tex = SDL_CreateTexture(m_render, SDL_PIXELFORMAT_ARGB8888,
                                     SDL_TEXTUREACCESS_STREAMING, screen_width, screen_height);
    m_pixels.resize(screen_width * screen_height, 0);

    if (m_render == nullptr) {
        SDL_DestroyWindow(m_window);
        std::cerr << "SDL_CreateRenderer error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    if (TTF_Init() != 0) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf error: " << SDL_GetError() << std::endl;
        return 1;
    }

    auto font_path = "res/DejaVuSansMono-Bold.ttf";
    m_font = TTF_OpenFont(font_path, 14);
    if (m_font == nullptr) {
        std::cerr << "SDL_ttf could not open '" << font_path << "'" << std::endl;
        return 1;
    }

    // Setup scene
    setup_scene(screen_width, screen_height);

    fmt::print("Finish init\n");

    return 0;
}

void Viewer::setup_scene(int screen_width, int screen_height) {
    auto wall_left = Mesh::make_plane({-0.5f, 0.4f, 0}, {1, 0, 0}, {1, 1}, {0, 0, 0}, {1, 0, 0});
    auto wall_right = Mesh::make_plane({0.5f, 0.4f, 0}, {-1, 0, 0}, {1, 1}, {0, 0, 0}, {0, 1, 0});
    auto wall_back = Mesh::make_plane({0, 0.4f, 0.5}, {0, 0, 1}, {1, 1});
    auto wall_top = Mesh::make_plane({0, 0.9f, 0}, {0, 1, 0}, {1, 1});
    auto wall_bottom = Mesh::make_plane({0, -0.1f, 0}, {0, 1, 0}, {1, 1});
    auto lamp = Mesh::make_plane({0, 0.85f, -0.1}, {0, 1, 0}, {0.8, 0.8}, {3, 3, 3}, {1, 1, 1});
    auto box1 = Mesh::make_box({0.2f, 0.1f, 0}, {0.3, 0.1, 0.5}, {0.2f, 0.5f, 0.2f});
    auto box2 = Mesh::make_box({-0.2f, 0.05f, 0}, {0.3, -0.4, -0.9}, {0.3f, 0.4f, 0.3f});

    std::vector<std::unique_ptr<Mesh>> objects;
    objects.push_back(std::move(wall_left));
    objects.push_back(std::move(wall_right));
    objects.push_back(std::move(wall_top));
    objects.push_back(std::move(wall_back));
    objects.push_back(std::move(wall_bottom));
    objects.push_back(std::move(lamp));
    objects.push_back(std::move(box1));
    objects.push_back(std::move(box2));

    for (auto &object : objects) {
        for (auto &tri : object->triangles()) {
            m_scene.push_back(std::move(tri));
        }
    }

    glm::vec3 cam_pos = {0, 0.31, -1.2};
    glm::vec3 cam_dir = {0, 0, 1};
    glm::vec3 world_up = {0, 1, 0};

    m_camera = Camera(cam_pos, cam_dir, world_up, 90.f, 0.001, 100.f, screen_width, screen_height);
}

glm::vec3 Viewer::intersect_scene(glm::vec3 &ray_pos, glm::vec3 &ray_dir, int depth) {
    if (depth == m_max_depth)
        return {0, 0, 0};

    // check all triangles for collision
    bool collided = false;
    glm::vec3 ret_color{0.f, 0.f, 0.f};
    for (const auto &tri : m_scene) {
        float dist_to_col;
        collided = trac0r::intersect_ray_triangle(ray_pos, ray_dir, tri->m_v1, tri->m_v2, tri->m_v3,
                                                  dist_to_col);

        if (collided) {
            glm::vec3 impact_pos = ray_pos + ray_dir * dist_to_col;

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
                local_radiance = tri->m_emittance;
            }
            local_radiance = tri->m_emittance;

            // Emitter sample
            // TODO
            // glm::vec3 illumination;

            auto normal = tri->m_normal * -glm::sign(glm::dot(tri->m_normal, ray_dir));
            // Find new random direction for diffuse reflection
            auto new_ray_dir = normal;
            auto half_pi = glm::half_pi<float>();
            auto pi = glm::pi<float>();
            new_ray_dir = glm::rotate(normal, glm::linearRand(-half_pi, half_pi),
                                      glm::cross(normal, ray_dir));
            new_ray_dir = glm::rotate(new_ray_dir, glm::linearRand(-pi, pi), normal);
            float cos_theta = glm::dot(new_ray_dir, normal);
            glm::vec3 bdrf = 2.f * tri->m_reflectance * cos_theta;

            // Send new ray in new direction
            glm::vec3 reflected = intersect_scene(impact_pos, new_ray_dir, depth + 1);

            ret_color = local_radiance + (bdrf * reflected);
            break;
        }
    }

    return ret_color;
}

void Viewer::mainloop() {
    m_scene_changed = false;

    int current_time = SDL_GetTicks();
    double dt = (current_time - m_last_frame_time) / 1000.0;
    m_last_frame_time = current_time;
    auto fps = 1. / dt;

    // Input
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            shutdown();
        }

        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                shutdown();
            }
        }

        if (e.type == SDL_MOUSEBUTTONDOWN) {
            if (e.button.button == SDL_BUTTON_RIGHT) {
                if (m_look_mode) {
                    m_look_mode = false;
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                } else if (!m_look_mode) {
                    m_look_mode = true;
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                }
            }
        }
    }

    float cam_speed = 0.01f;

    // Left/right
    const uint8_t *keystates = SDL_GetKeyboardState(0);
    if (keystates[SDL_SCANCODE_A]) {
        m_scene_changed = true;
        m_camera.set_pos(m_camera.pos() - m_camera.right() * cam_speed);
    } else if (keystates[SDL_SCANCODE_D]) {
        m_camera.set_pos(m_camera.pos() + m_camera.right() * cam_speed);
        m_scene_changed = true;
    }

    // Up/down
    if (keystates[SDL_SCANCODE_SPACE]) {
        m_scene_changed = true;
        m_camera.set_pos(m_camera.pos() + glm::vec3{0, 1, 0} * cam_speed);
    } else if (keystates[SDL_SCANCODE_LCTRL]) {
        m_scene_changed = true;
        m_camera.set_pos(m_camera.pos() - glm::vec3{0, 1, 0} * cam_speed);
    }

    // Roll
    if (keystates[SDL_SCANCODE_Q]) {
        m_scene_changed = true;
        m_camera.set_world_up(glm::rotateZ(m_camera.up(), 0.1f));
    } else if (keystates[SDL_SCANCODE_E]) {
        m_scene_changed = true;
        m_camera.set_world_up(glm::rotateZ(m_camera.up(), -0.1f));
    }

    // Front/back
    if (keystates[SDL_SCANCODE_W]) {
        m_scene_changed = true;
        m_camera.set_pos(m_camera.pos() + m_camera.dir() * cam_speed);
    } else if (keystates[SDL_SCANCODE_S]) {
        m_scene_changed = true;
        m_camera.set_pos(m_camera.pos() - m_camera.dir() * cam_speed);
    }

    // Rendering here
    int width;
    int height;
    SDL_GetWindowSize(m_window, &width, &height);

    glm::ivec2 mouse_pos;
    if (m_look_mode) {
        // Yaw
        SDL_GetRelativeMouseState(&(mouse_pos.x), &(mouse_pos.y));
        if (mouse_pos.x != 0) {
            m_scene_changed = true;
            m_camera.set_dir(glm::rotateY(m_camera.dir(), mouse_pos.x * 0.001f));
        }

        // Pitch
        if (mouse_pos.y != 0) {
            m_scene_changed = true;
            m_camera.set_dir(glm::rotate(m_camera.dir(), mouse_pos.y * 0.001f,
                                         glm::cross(m_camera.up(), m_camera.dir())));
        }

    } else if (!m_look_mode) {
        SDL_GetMouseState(&(mouse_pos.x), &(mouse_pos.y));
    }

    if (m_scene_changed) {
        m_samples_accumulated = 0;
    }

    // Lots of debug info
    glm::vec2 mouse_rel_pos = m_camera.screenspace_to_camspace(mouse_pos.x, mouse_pos.y);
    glm::vec3 mouse_canvas_pos = m_camera.camspace_to_worldspace(mouse_rel_pos);

    auto fps_debug_info = "FPS: " + std::to_string(int(fps));
    fps_debug_info +=
        " RPS: " + std::to_string(int(fps * m_max_samples * width * height * m_max_depth));
    auto scene_changing_info = "Samples : " + std::to_string(m_samples_accumulated);
    scene_changing_info += " Scene Changing: " + std::to_string(m_scene_changed);
    auto cam_look_debug_info = "Cam Look Mode: " + std::to_string(m_look_mode);
    auto cam_pos_debug_info = "Cam Pos: " + glm::to_string(m_camera.pos());
    auto cam_dir_debug_info = "Cam Dir: " + glm::to_string(m_camera.dir());
    auto cam_up_debug_info = "Cam Up: " + glm::to_string(m_camera.up());
    auto cam_fov_debug_info =
        "Cam FOV (H/V): " + std::to_string(int(glm::degrees(m_camera.horizontal_fov()))) + "/";
    cam_fov_debug_info += std::to_string(int(glm::degrees(m_camera.vertical_fov())));
    auto cam_canvas_center_pos_info =
        "Cam Canvas Center: " + glm::to_string(m_camera.canvas_center_pos());
    auto mouse_pos_screen_info = "Mouse Pos Screen Space: " + glm::to_string(mouse_pos);
    auto mouse_pos_relative_info = "Mouse Pos Cam Space: " + glm::to_string(mouse_rel_pos);
    auto mouse_pos_canvas_info =
        "Mouse Pos Canvas World Space: " + glm::to_string(mouse_canvas_pos);

    auto fps_debug_tex = trac0r::make_text(m_render, m_font, fps_debug_info, {200, 100, 100, 200});
    auto scene_changing_tex =
        trac0r::make_text(m_render, m_font, scene_changing_info, {200, 100, 100, 200});
    auto cam_look_debug_tex =
        trac0r::make_text(m_render, m_font, cam_look_debug_info, {200, 100, 100, 200});
    auto cam_pos_debug_tex =
        trac0r::make_text(m_render, m_font, cam_pos_debug_info, {200, 100, 100, 200});
    auto cam_dir_debug_tex =
        trac0r::make_text(m_render, m_font, cam_dir_debug_info, {200, 100, 100, 200});
    auto cam_up_debug_tex =
        trac0r::make_text(m_render, m_font, cam_up_debug_info, {200, 100, 100, 200});
    auto cam_fov_debug_tex =
        trac0r::make_text(m_render, m_font, cam_fov_debug_info, {200, 100, 100, 200});
    auto cam_canvas_center_pos_tex =
        trac0r::make_text(m_render, m_font, cam_canvas_center_pos_info, {200, 100, 100, 200});
    auto mouse_pos_screen_tex =
        trac0r::make_text(m_render, m_font, mouse_pos_screen_info, {200, 100, 100, 200});
    auto mouse_pos_relative_tex =
        trac0r::make_text(m_render, m_font, mouse_pos_relative_info, {200, 100, 100, 200});
    auto mouse_pos_canvas_tex =
        trac0r::make_text(m_render, m_font, mouse_pos_canvas_info, {200, 100, 100, 200});

    // Sort by distance to camera
    std::sort(m_scene.begin(), m_scene.end(), [this](const auto &tri1, const auto &tri2) {
        return glm::distance(m_camera.pos(), tri1->m_centroid) <
               glm::distance(m_camera.pos(), tri2->m_centroid);
    });

    // For speeding up (but we'll lose quality)
    auto x_stride = 3;
    auto y_stride = 3;
    m_samples_accumulated += 1;
    for (auto x = 0; x < width; x += x_stride) {
        for (auto y = 0; y < height; y += y_stride) {
            glm::vec2 rel_pos = m_camera.screenspace_to_camspace(x, y);
            glm::vec3 world_pos = m_camera.camspace_to_worldspace(rel_pos);
            glm::vec3 ray_dir = glm::normalize(world_pos - m_camera.pos());

            glm::vec3 result_color = intersect_scene(world_pos, ray_dir, 0);
            glm::vec4 new_color = glm::vec4(result_color, 1.f);
            glm::vec4 old_color = trac0r::unpack_color_argb_to_vec4(m_pixels[y * width + x]);
            new_color = (old_color * float(m_samples_accumulated - 1) + new_color) /
                        float(m_samples_accumulated);

            // This is just for speeding up
            // We're basically drawing really bix pixels here
            uint32_t packed_color = trac0r::pack_color_argb(new_color);
            for (auto u = 0; u < x_stride; u++) {
                for (auto v = 0; v < y_stride; v++) {
                    m_pixels[(y + v) * width + (x + u)] = packed_color;
                }
            }
        }
    }

    SDL_RenderClear(m_render);
    SDL_UpdateTexture(m_render_tex, 0, m_pixels.data(), width * sizeof(uint32_t));
    SDL_RenderCopy(m_render, m_render_tex, 0, 0);

    trac0r::render_text(m_render, fps_debug_tex, 10, 10);
    trac0r::render_text(m_render, scene_changing_tex, 10, 25);
    trac0r::render_text(m_render, cam_look_debug_tex, 10, 40);
    trac0r::render_text(m_render, cam_pos_debug_tex, 10, 55);
    trac0r::render_text(m_render, cam_dir_debug_tex, 10, 70);
    trac0r::render_text(m_render, cam_up_debug_tex, 10, 85);
    trac0r::render_text(m_render, cam_fov_debug_tex, 10, 100);
    trac0r::render_text(m_render, cam_canvas_center_pos_tex, 10, 115);
    trac0r::render_text(m_render, mouse_pos_screen_tex, 10, 130);
    trac0r::render_text(m_render, mouse_pos_relative_tex, 10, 145);
    trac0r::render_text(m_render, mouse_pos_canvas_tex, 10, 160);

    SDL_RenderPresent(m_render);

    SDL_DestroyTexture(fps_debug_tex);
    SDL_DestroyTexture(scene_changing_tex);
    SDL_DestroyTexture(cam_look_debug_tex);
    SDL_DestroyTexture(cam_pos_debug_tex);
    SDL_DestroyTexture(cam_dir_debug_tex);
    SDL_DestroyTexture(cam_up_debug_tex);
    SDL_DestroyTexture(cam_fov_debug_tex);
    SDL_DestroyTexture(cam_canvas_center_pos_tex);
    SDL_DestroyTexture(mouse_pos_screen_tex);
    SDL_DestroyTexture(mouse_pos_relative_tex);
    SDL_DestroyTexture(mouse_pos_canvas_tex);
}

SDL_Renderer *Viewer::renderer() {
    return m_render;
}

SDL_Window *Viewer::window() {
    return m_window;
}

bool Viewer::is_running() {
    return m_running;
}

void Viewer::shutdown() {
    m_running = false;
}
