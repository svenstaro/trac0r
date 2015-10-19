#include "viewer.hpp"

#include "trac0r/utils.hpp"

#include <SDL_ttf.h>
#include <SDL_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/rotate_vector.hpp>

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
    std::cout << "Start init" << std::endl;

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

    std::cout << "Finish init" << std::endl;

    return 0;
}

void Viewer::setup_scene(int screen_width, int screen_height) {
    auto triangle1 = std::make_unique<Triangle>(
        glm::vec3{0.f, 0.f, -0.89f}, glm::vec3{0.07f, 0.f, -0.89f},
        glm::vec3{0.07f, -0.04f, -0.89f}, glm::vec3{0.3, 0.3, 0.3}, glm::vec3{0.2, 0.2, 0.2});
    auto triangle2 = std::make_unique<Triangle>(
        glm::vec3{-0.05f, 0.f, -0.6f}, glm::vec3{-0.005f, 0.f, -0.6f},
        glm::vec3{-0.05f, -0.04f, -0.6f}, glm::vec3{0.9, 0.3, 0.3}, glm::vec3{0.2, 0.2, 0.4});
    m_scene.push_back(std::move(triangle1));
    m_scene.push_back(std::move(triangle2));
    // for (auto i = 0; i < 2; i++) {
    //     auto triangle =
    //         std::make_unique<Triangle>(glm::ballRand(5.f), glm::ballRand(5.f),
    //         glm::ballRand(5.f),
    //                                    glm::vec3{0.8, 0.3, 0.3}, glm::vec3{0.5, 0.5, 0.5});
    //     m_scene.push_back(std::move(triangle));
    // }

    glm::vec3 cam_pos = {0, 0, -1};
    glm::vec3 cam_dir = {0, 0, 1};
    glm::vec3 cam_up = {0, 1, 0};

    m_camera = Camera(cam_pos, cam_dir, cam_up, 45.f, 0.001, 100.f, screen_width, screen_height);
}

glm::vec4 Viewer::intersect_scene(glm::vec3 &ray_pos, glm::vec3 &ray_dir, int depth) {
    if (depth == m_max_depth)
        return {0, 0, 0, 1.f};

    // check all triangles for collision
    bool collided = false;
    glm::vec4 ret_color{0.f, 0.f, 0.f, 1.f};
    for (const auto &tri : m_scene) {
        glm::vec3 bary_pos;
        collided =
            glm::intersectRayTriangle(ray_pos, ray_dir, tri->m_v1, tri->m_v2, tri->m_v3, bary_pos);

        if (collided) {
            // glm::vec3 new_ray_pos = ray_pos + ray_dir * bary_pos.z;
            // auto new_ray_dir = tri->m_normal;
            // new_ray_dir = glm::rotateX(tri->m_normal, glm::linearRand(-90.f, 90.f));
            // new_ray_dir = glm::rotateZ(tri->m_normal, glm::linearRand(-90.f, 90.f));
            //
            // float cos_theta = glm::dot(new_ray_dir, tri->m_normal);
            // glm::vec3 brdf = 2.f * tri->m_reflectance * cos_theta;
            // glm::vec3 reflected = intersect_scene(new_ray_pos, new_ray_dir, depth + 1);
            //
            // ret_color = tri->m_emittance + (brdf * reflected);
            ret_color = {0.7f, 0.0f, 0.7f, 1.f};
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

    glm::vec3 cam_velocity{0};
    const uint8_t *keystates = SDL_GetKeyboardState(0);
    if (keystates[SDL_SCANCODE_A]) {
        m_scene_changed = true;
        cam_velocity.x += -0.01f;
    } else if (keystates[SDL_SCANCODE_D]) {
        m_scene_changed = true;
        cam_velocity.x += 0.01f;
    }

    if (keystates[SDL_SCANCODE_SPACE]) {
        m_scene_changed = true;
        cam_velocity.y += 0.01f;
    } else if (keystates[SDL_SCANCODE_LCTRL]) {
        m_scene_changed = true;
        cam_velocity.y += -0.01f;
    }

    if (keystates[SDL_SCANCODE_Q]) {
        m_scene_changed = true;
        m_camera.set_up(glm::rotateZ(m_camera.up(), 0.1f));
    } else if (keystates[SDL_SCANCODE_E]) {
        m_scene_changed = true;
        m_camera.set_up(glm::rotateZ(m_camera.up(), -0.1f));
    }

    if (keystates[SDL_SCANCODE_W]) {
        m_scene_changed = true;
        cam_velocity.z += 0.01f;
    } else if (keystates[SDL_SCANCODE_S]) {
        m_scene_changed = true;
        cam_velocity.z += -0.01f;
    }

    m_camera.set_pos(m_camera.pos() += cam_velocity);

    // Rendering here
    int width;
    int height;
    SDL_GetWindowSize(m_window, &width, &height);

    glm::ivec2 mouse_pos;
    if (m_look_mode) {
        SDL_GetRelativeMouseState(&(mouse_pos.x), &(mouse_pos.y));
        if (mouse_pos.x != 0) {
            m_scene_changed = true;
            m_camera.set_dir(glm::rotateY(m_camera.dir(), mouse_pos.x * 0.001f));
        }

        if (mouse_pos.y != 0) {
            m_scene_changed = true;
            m_camera.set_dir(glm::rotateX(m_camera.dir(), mouse_pos.y * 0.001f));
            m_camera.set_up(glm::rotateX(m_camera.up(), mouse_pos.y * 0.001f));
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
    fps_debug_info += " RPS: " + std::to_string(int(fps * m_max_samples * width * height * m_max_depth));
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
    auto scene_changing_tex = trac0r::make_text(m_render, m_font, scene_changing_info, {200, 100, 100, 200});
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

    m_samples_accumulated += 1;
    // for (auto sample_cnt = 0; sample_cnt < m_max_samples; sample_cnt++) {
        for (auto x = 0; x < width; x++) {
            for (auto y = 0; y < height; y++) {
                glm::vec2 rel_pos = m_camera.screenspace_to_camspace(x, y);
                glm::vec3 world_pos = m_camera.camspace_to_worldspace(rel_pos);
                glm::vec3 ray_dir = glm::normalize(world_pos - m_camera.pos());

                glm::vec4 result_color = intersect_scene(world_pos, ray_dir, 0);
                // glm::vec4 old_color = trac0r::unpack_color_argb_to_vec4(m_pixels[y * width + x]);

                // new_color = (old_color * float(m_samples_accumulated - 1) + new_color) / float(m_samples_accumulated);
                // m_pixels[y * width + x] = trac0r::pack_color_argb(new_color);
                m_pixels[y * width + x] = trac0r::pack_color_argb(result_color);
            }
        }
    // }

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
