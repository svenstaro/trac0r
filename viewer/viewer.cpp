#include "viewer.hpp"

#include <SDL_ttf.h>
#include <SDL_image.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <algorithm>
#include <iostream>

Viewer::~Viewer() {
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

    int width = 600;
    int height = 400;

    m_window = SDL_CreateWindow("trac0r", 100, 100, width, height, SDL_WINDOW_SHOWN);
    if (m_window == nullptr) {
        std::cerr << "SDL_CreateWindow error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // m_render = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    m_render = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    m_render_tex = SDL_CreateTexture(m_render, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    m_pixels.resize(width*height, 0);

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
    
    // Setup scene
    //SDL_SetRelativeMouseMode(SDL_TRUE);
    setup_scene();

    std::cout << "Finish init" << std::endl;

    return 0;
}

void Viewer::setup_scene() {
    auto triangle = std::make_unique<Triangle>(glm::vec3{0.f, 5.f, 0.f}, glm::vec3{5.f, 5.f, 0.f}, glm::vec3{5.f, 5.f, 5.f}, glm::vec3{0.8, 0.3, 0.3}, glm::vec3{0.5, 0.5, 0.5});
    m_scene.push_back(std::move(triangle));
    // for (auto i = 0; i < 2; i++) {
    //     auto triangle = std::make_unique<Triangle>(glm::ballRand(5.f), glm::ballRand(5.f), glm::ballRand(5.f), glm::vec3{0.8, 0.3, 0.3}, glm::vec3{0.5, 0.5, 0.5});
    //     m_scene.push_back(std::move(triangle));
    // }

    m_camera.pos = {0, 0, 0};
    m_camera.dir = {0, 1, 0};
    m_camera.up = {0, 0, 1};
    m_camera.fov = 45.f;
    m_camera.near_plane_dist = 0.1f;
    m_camera.far_plane_dist = 100.f;
}

glm::vec3 Viewer::intersect_scene(glm::vec3 &ray_pos, glm::vec3 &ray_dir, int depth) {
    const auto MAX_DEPTH = 5;

    if (depth == MAX_DEPTH)
        return {0, 0, 0};

    // check all triangles for collision
    bool collided = false;
    glm::vec3 ret_color;
    for (const auto& tri : m_scene) {
        glm::vec3 bary_pos;
        collided = glm::intersectRayTriangle(ray_pos, ray_dir, tri->m_v1, tri->m_v2, tri->m_v3, bary_pos);

        if (collided) {
            glm::vec3 new_ray_pos = ray_pos + ray_dir * bary_pos.z;
            auto new_ray_dir = tri->m_normal;
            new_ray_dir = glm::rotateX(tri->m_normal, glm::linearRand(-90.f, 90.f));
            new_ray_dir = glm::rotateY(tri->m_normal, glm::linearRand(-90.f, 90.f));

            float cos_theta = glm::dot(new_ray_dir, tri->m_normal);
            glm::vec3 brdf = 2.f * tri->m_reflectance * cos_theta;
            glm::vec3 reflected = intersect_scene(new_ray_pos, new_ray_dir, depth + 1);

            ret_color = tri->m_color + (brdf * reflected);
            break;
        }
    }

    if (collided) {
        return ret_color;
    }
    else {
        return {0, 0, 0};
    }
}

void Viewer::mainloop() {
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
    }

    // Rendering here
    int width;
    int height;
    SDL_GetWindowSize(m_window, &width, &height);
    auto aspect_ratio = (float)width/(float)height;

    int mouse_x;
    int mouse_y;
    SDL_GetRelativeMouseState(&mouse_x, &mouse_y);

    glm::mat4 projection_matrix = glm::perspective(m_camera.fov, aspect_ratio, m_camera.near_plane_dist, m_camera.far_plane_dist);
    glm::mat4 view_matrix = glm::lookAt(m_camera.pos, m_camera.pos + m_camera.dir, m_camera.up);

    // Sort by distance to camera
    std::sort(m_scene.begin(), m_scene.end(), [this](const auto &tri1, const auto &tri2){return glm::distance(m_camera.pos, tri1->m_centroid) < glm::distance(m_camera.pos, tri2->m_centroid);});
    for (auto sample_cnt = 0; sample_cnt < 2; sample_cnt++) {
        for (auto x = 0; x < width; x++) {
            for (auto y = 0; y < height; y++) {
                glm::vec3 win_coords{x, y, 0};
                glm::mat4 model{1.0f};
                glm::vec4 viewport{0, 0, width, height};
                glm::vec3 object_coords = glm::unProject(win_coords, model * view_matrix, projection_matrix, viewport);
                glm::vec3 color = intersect_scene(object_coords, m_camera.dir, 0);
                // std::cout << "r: " << color.r << " g: " << color.g << " b: " << color.b << std::endl;
                m_pixels[y * width + x] = 0xff << 24 | int(color.r * 255) << 16 | int(color.g * 255) << 8 | int(color.b * 255);
            }
        }
    }
    
    SDL_SetRenderDrawColor(m_render, glm::linearRand(0, 255), glm::linearRand(0, 255), glm::linearRand(0, 255), 255);
    SDL_RenderClear(m_render);
    SDL_UpdateTexture(m_render_tex, 0, m_pixels.data(), width * sizeof(uint32_t));
    SDL_RenderCopy(m_render, m_render_tex, 0, 0);

    SDL_RenderPresent(m_render);

    int current_time = SDL_GetTicks();
    double dt = (current_time - m_last_frame_time) / 1000.0;
    (void)dt;
    m_last_frame_time = current_time;

    std::cout << "FPS: " << 1./dt << std::endl;
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
