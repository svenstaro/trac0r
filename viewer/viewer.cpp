#include "viewer.hpp"

#include "trac0r/shape.hpp"
#include "trac0r/utils.hpp"
#include "trac0r/flat_structure.hpp"
#include "trac0r/filtering.hpp"

#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL2_gfxPrimitives.h>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <cppformat/format.h>

#ifdef OPENCL
#include <CL/cl.hpp>
#endif

#include <algorithm>
#include <iostream>
#include <memory>

using Camera = trac0r::Camera;
using Scene = trac0r::Scene;
using FlatStructure = trac0r::FlatStructure;
using AABB = trac0r::AABB;
using Shape = trac0r::Shape;

Viewer::~Viewer() {
    TTF_CloseFont(m_font);
    TTF_Quit();
    SDL_DestroyTexture(m_render_tex);
    SDL_DestroyRenderer(m_render);
    SDL_DestroyWindow(m_window);
    IMG_Quit();
    SDL_Quit();
}

int Viewer::init(int argc, char *argv[]) {
    fmt::print("Start init\n");

    for (auto i = 0; i < argc; i++) {
        std::string argv_str(argv[i]);
        if (argv_str == "-b1") {
            m_benchmark_mode = 1;
            m_max_frames = 500;
        } else if (argv_str == "-b2") {
            m_benchmark_mode = 2;
            m_max_frames = 250;
        } else if (argv_str == "-b3") {
            m_benchmark_mode = 3;
            m_max_frames = 125;
        } else if (argv_str == "-b4") {
            m_benchmark_mode = 4;
            m_max_frames = 50;
        } else if (argv_str == "-b5") {
            m_benchmark_mode = 5;
            m_max_frames = 25;
        }
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL_Init error: " << SDL_GetError() << std::endl;
        return 1;
    }

    m_window =
        SDL_CreateWindow("trac0r", 100, 100, m_screen_width, m_screen_height, SDL_WINDOW_SHOWN);
    if (m_window == nullptr) {
        std::cerr << "SDL_CreateWindow error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // m_render = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED |
    // SDL_RENDERER_PRESENTVSYNC);
    m_render = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    m_render_tex = SDL_CreateTexture(m_render, SDL_PIXELFORMAT_ARGB8888,
                                     SDL_TEXTUREACCESS_STREAMING, m_screen_width, m_screen_height);
    m_pixels.resize(m_screen_width * m_screen_height, 0);

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
    setup_scene();
    m_renderer = std::make_unique<trac0r::Renderer>(m_screen_width, m_screen_height, m_camera,
                                                    m_scene, m_print_perf);
    m_renderer->print_sysinfo();

    fmt::print("Finish init\n");

    return 0;
}

void Viewer::setup_scene() {
    trac0r::Material emissive{1, {1.f, 0.93f, 0.85f}, 0.f, 1.f, 15.f};
    trac0r::Material default_material{2, {0.740063, 0.742313, 0.733934}};
    trac0r::Material diffuse_red{2, {0.366046, 0.0371827, 0.0416385}};
    trac0r::Material diffuse_green{2, {0.162928, 0.408903, 0.0833759}};
    trac0r::Material glass{3, {0.5f, 0.5f, 0.9f}, 0.0f, 1.51714f};
    trac0r::Material glossy{4, {1.f, 1.f, 1.f}, 0.09f};
    auto wall_left = trac0r::Shape::make_plane({-0.5f, 0.4f, 0}, {0, 0, -glm::half_pi<float>()},
                                               {1, 1}, diffuse_red);
    auto wall_right = trac0r::Shape::make_plane({0.5f, 0.4f, 0}, {0, 0, glm::half_pi<float>()},
                                                {1, 1}, diffuse_green);
    auto wall_back = trac0r::Shape::make_plane({0, 0.4f, 0.5}, {-glm::half_pi<float>(), 0, 0},
                                               {1, 1}, default_material);
    auto wall_top =
        trac0r::Shape::make_plane({0, 0.9f, 0}, {glm::pi<float>(), 0, 0}, {1, 1}, default_material);
    auto wall_bottom =
        trac0r::Shape::make_plane({0, -0.1f, 0}, {0, 0, 0}, {1, 1}, default_material);
    auto lamp = trac0r::Shape::make_plane({0, 0.85f, -0.1}, {0, 0, 0}, {0.4, 0.4}, emissive);
    auto box1 = trac0r::Shape::make_box({0.3f, 0.1f, 0.1f}, {0, 0.6f, 0}, {0.2f, 0.5f, 0.2f},
                                        default_material);
    auto box2 =
        trac0r::Shape::make_box({-0.2f, 0.15f, 0.1f}, {0, -0.5f, 0}, {0.3f, 0.6f, 0.3f}, glossy);
    if (m_benchmark_mode > 0) {
        auto sphere1 = trac0r::Shape::make_icosphere({0.f, 0.1f, -0.3f}, {0, 0, 0}, 0.15f, m_benchmark_mode - 1,
                                                     default_material);
        Scene::add_shape(m_scene, sphere1);
    } else {
        auto sphere1 =
            trac0r::Shape::make_icosphere({0.f, 0.1f, -0.3f}, {0, 0, 0}, 0.15f, 2, glass);
        auto sphere2 =
            trac0r::Shape::make_icosphere({0.3f, 0.45f, 0.1f}, {0, 0, 0}, 0.15f, 2, glossy);
        Scene::add_shape(m_scene, sphere1);
        Scene::add_shape(m_scene, sphere2);
    }

    Scene::add_shape(m_scene, wall_left);
    Scene::add_shape(m_scene, wall_right);
    Scene::add_shape(m_scene, wall_back);
    Scene::add_shape(m_scene, wall_top);
    Scene::add_shape(m_scene, wall_bottom);
    Scene::add_shape(m_scene, lamp);
    Scene::add_shape(m_scene, box1);
    Scene::add_shape(m_scene, box2);

    glm::vec3 cam_pos = {0, 0.31, -1.2};
    glm::vec3 cam_dir = {0, 0, 1};
    glm::vec3 world_up = {0, 1, 0};

    m_camera =
        Camera(cam_pos, cam_dir, world_up, 90.f, 0.001, 100.f, m_screen_width, m_screen_height);
}

void Viewer::mainloop() {
    Timer timer;
    Timer total;

    if (m_print_perf)
        fmt::print("Rendering frame {}\n", m_frame);
    m_scene_changed = false;
    m_frame++;

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
            if (e.key.keysym.sym == SDLK_b) {
                m_debug = !m_debug;
            }
            if (e.key.keysym.sym == SDLK_n) {
                m_print_perf = !m_print_perf;
            }
            if (e.key.keysym.sym == SDLK_1) {
                m_stride_x = 1;
                m_stride_y = 1;
                m_scene_changed = true;
            }
            if (e.key.keysym.sym == SDLK_2) {
                m_stride_x = 2;
                m_stride_y = 2;
                m_scene_changed = true;
            }
            if (e.key.keysym.sym == SDLK_3) {
                m_stride_x = 4;
                m_stride_y = 4;
                m_scene_changed = true;
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

    float cam_speed = .8f * dt;

    // Left/right
    const uint8_t *keystates = SDL_GetKeyboardState(0);
    if (keystates[SDL_SCANCODE_A]) {
        m_scene_changed = true;
        Camera::set_pos(m_camera, Camera::pos(m_camera) - Camera::right(m_camera) * cam_speed);
    } else if (keystates[SDL_SCANCODE_D]) {
        Camera::set_pos(m_camera, Camera::pos(m_camera) + Camera::right(m_camera) * cam_speed);
        m_scene_changed = true;
    }

    // Up/down
    if (keystates[SDL_SCANCODE_SPACE]) {
        m_scene_changed = true;
        Camera::set_pos(m_camera, Camera::pos(m_camera) + glm::vec3{0, 1, 0} * cam_speed);
    } else if (keystates[SDL_SCANCODE_LCTRL]) {
        m_scene_changed = true;
        Camera::set_pos(m_camera, Camera::pos(m_camera) - glm::vec3{0, 1, 0} * cam_speed);
    }

    // Roll
    if (keystates[SDL_SCANCODE_Q]) {
        m_scene_changed = true;
        Camera::set_world_up(m_camera, glm::rotateZ(Camera::up(m_camera), 0.1f));
    } else if (keystates[SDL_SCANCODE_E]) {
        m_scene_changed = true;
        Camera::set_world_up(m_camera, glm::rotateZ(Camera::up(m_camera), -0.1f));
    }

    // Front/back
    if (keystates[SDL_SCANCODE_W]) {
        m_scene_changed = true;
        Camera::set_pos(m_camera, Camera::pos(m_camera) + Camera::dir(m_camera) * cam_speed);
    } else if (keystates[SDL_SCANCODE_S]) {
        m_scene_changed = true;
        Camera::set_pos(m_camera, Camera::pos(m_camera) - Camera::dir(m_camera) * cam_speed);
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
            Camera::set_dir(m_camera, glm::rotateY(Camera::dir(m_camera), mouse_pos.x * 0.001f));
        }

        // Pitch
        if (mouse_pos.y != 0) {
            m_scene_changed = true;
            Camera::set_dir(m_camera,
                            glm::rotate(Camera::dir(m_camera), mouse_pos.y * 0.001f,
                                        glm::cross(Camera::up(m_camera), Camera::dir(m_camera))));
        }

    } else if (!m_look_mode) {
        SDL_GetMouseState(&(mouse_pos.x), &(mouse_pos.y));
    }

    if (m_scene_changed) {
        m_samples_accumulated = 0;
    }

    if (m_print_perf)
        fmt::print("    {:<15} {:>10.3f} ms\n", "Input handling", timer.elapsed());

    Scene::rebuild(m_scene);

    if (m_print_perf)
        fmt::print("    {:<15} {:=10.3f} ms\n", "Scene rebuild", timer.elapsed());

    const auto luminance = m_renderer->render(m_scene_changed, m_stride_x, m_stride_y);
    if (m_print_perf) {
        m_renderer->print_last_frame_timings();
    }

    m_samples_accumulated += 1;

    timer.reset();

// This striding is just for speeding up
// We're basically drawing really big pixels here
#pragma omp parallel for collapse(2) schedule(dynamic, 1024)
    for (auto x = 0; x < width; x += m_stride_x) {
        for (auto y = 0; y < height; y += m_stride_y) {
            glm::vec4 color = luminance[y * width + x] / static_cast<float>(m_samples_accumulated);
            for (auto u = 0; u < m_stride_x; u++) {
                for (auto v = 0; v < m_stride_y; v++) {
                    m_pixels[(y + v) * width + (x + u)] = trac0r::pack_color_argb(color);
                }
            }
        }
    }

    if (m_print_perf)
        fmt::print("    {:<15} {:>10.3f} ms\n", "Pixel transfer", timer.elapsed());

    // std::vector<uint32_t> m_pixels_filtered;
    // m_pixels_filtered.resize(m_screen_width * m_screen_height, 0);
    // trac0r::box_filter(m_pixels, width, height, m_pixels_filtered);
    // m_pixels = m_pixels_filtered;
    //
    // if (m_print_perf)
    //     fmt::print("    {:<15} {:>10.3f} ms\n", "Image filtering", timer.elapsed());

    SDL_RenderClear(m_render);
    SDL_UpdateTexture(m_render_tex, 0, m_pixels.data(), width * sizeof(uint32_t));
    SDL_RenderCopy(m_render, m_render_tex, 0, 0);

    if (m_debug) {
        // Lots of debug info
        glm::vec2 mouse_rel_pos =
            Camera::screenspace_to_camspace(m_camera, mouse_pos.x, mouse_pos.y);
        glm::vec3 mouse_canvas_pos = Camera::camspace_to_worldspace(m_camera, mouse_rel_pos);

        auto fps_debug_info = "FPS: " + std::to_string(int(fps));
        auto scene_changing_info = "Samples : " + std::to_string(m_samples_accumulated);
        scene_changing_info += " Scene Changing: " + std::to_string(m_scene_changed);
        auto cam_look_debug_info = "Cam Look Mode: " + std::to_string(m_look_mode);
        auto cam_pos_debug_info = "Cam Pos: " + glm::to_string(Camera::pos(m_camera));
        auto cam_dir_debug_info = "Cam Dir: " + glm::to_string(Camera::dir(m_camera));
        auto cam_up_debug_info = "Cam Up: " + glm::to_string(Camera::up(m_camera));
        auto cam_fov_debug_info =
            "Cam FOV (H/V): " +
            std::to_string(int(glm::degrees(Camera::horizontal_fov(m_camera)))) + "/";
        cam_fov_debug_info += std::to_string(int(glm::degrees(Camera::vertical_fov(m_camera))));
        auto cam_canvas_center_pos_info =
            "Cam Canvas Center: " + glm::to_string(Camera::canvas_center_pos(m_camera));
        auto mouse_pos_screen_info = "Mouse Pos Screen Space: " + glm::to_string(mouse_pos);
        auto mouse_pos_relative_info = "Mouse Pos Cam Space: " + glm::to_string(mouse_rel_pos);
        auto mouse_pos_canvas_info =
            "Mouse Pos Canvas World Space: " + glm::to_string(mouse_canvas_pos);

        auto fps_debug_tex =
            trac0r::make_text(m_render, m_font, fps_debug_info, {200, 100, 100, 200});
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

        // Let's draw some debug to the display (such as AABBs)
        if (m_debug) {
            auto &accel_struct = Scene::accel_struct(m_scene);
            for (auto &shape : FlatStructure::shapes(accel_struct)) {
                auto &aabb = Shape::aabb(shape);
                const auto &verts = AABB::vertices(aabb);
                std::array<glm::i8vec2, 12> pairs;
                pairs[0] = {0, 1};
                pairs[1] = {1, 3};
                pairs[2] = {2, 3};
                pairs[3] = {0, 2};
                pairs[4] = {4, 5};
                pairs[5] = {5, 7};
                pairs[6] = {6, 7};
                pairs[7] = {4, 6};
                pairs[8] = {0, 4};
                pairs[9] = {1, 5};
                pairs[10] = {2, 6};
                pairs[11] = {3, 7};
                for (auto pair : pairs) {
                    auto ws1 = Camera::worldpoint_to_worldspace(m_camera, verts[pair[0]]);
                    auto ss1 = glm::i32vec2(0);
                    if (ws1 != glm::vec3(0)) {
                        auto cs1 = Camera::worldspace_to_camspace(m_camera, ws1);
                        ss1 = Camera::camspace_to_screenspace(m_camera, cs1);
                    }
                    auto ws2 = Camera::worldpoint_to_worldspace(m_camera, verts[pair[1]]);
                    auto ss2 = glm::i32vec2(0);
                    if (ws2 != glm::vec3(0)) {
                        auto cs2 = Camera::worldspace_to_camspace(m_camera, ws2);
                        ss2 = Camera::camspace_to_screenspace(m_camera, cs2);
                    }
                    if (ss1 != glm::i32vec2(0) && ss2 != glm::i32vec2(0)) {
                        aalineRGBA(m_render, ss1.x, ss1.y, ss2.x, ss2.y, 255, 255, 0, 200);
                    }
                }
            }
        }

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

    SDL_RenderPresent(m_render);

    if (m_print_perf) {
        fmt::print("    {:<15} {:>10.3f} ms\n", "Rendering", timer.elapsed());
        fmt::print("    {:<15} {:>10.3f} ms\n", "=> Budget", 1000.f / 60.f - total.peek());
        fmt::print("    {:<15} {:>10.3f} ms\n\n", "=> Total", total.peek());
    }

    m_frame_total += total.elapsed();
    if (m_benchmark_mode > 0 && m_max_frames != 0 && m_frame > m_max_frames) {
        fmt::print("Benchmark results:\n");
        fmt::print("    {:<15} {:>10}\n", "Frames rendered", m_max_frames);
        fmt::print("    {:<15} {:>10.3f} ms\n", "Total runtime", m_frame_total);
        fmt::print("    {:<15} {:>10.3f} ms\n", "Avg. frame", m_frame_total / m_max_frames);
        fmt::print("    {:<15} {:>10.3f} FPS\n", "Avg. FPS", 1.f / ((m_frame_total / 1000.f) / m_max_frames));
        shutdown();
    }
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
