#ifndef VIEWER_HPP
#define VIEWER_HPP

#include "trac0r/camera.hpp"
#include "trac0r/triangle.hpp"

#include <SDL.h>
#include <SDL_ttf.h>

#include <array>
#include <memory>
#include <vector>

// Right-handed coordinate system where Z is up and Y is depth (Blender-like)

class Viewer {
  public:
    ~Viewer();

    int init();
    void mainloop();
    bool is_running();
    void shutdown();

    SDL_Renderer *renderer();
    SDL_Window *window();

    // Put this stuff somewhere else
    void setup_scene(int width, int height);
    glm::vec3 intersect_scene(glm::vec3 &ray_pos, glm::vec3 &ray_dir, int depth);

  private:
    bool m_scene_changed = false;
    int m_samples_accumulated = 0;
    int m_max_samples = 2;
    int m_max_depth = 5;
    bool m_running = true;
    bool m_look_mode = false;
    int m_last_frame_time = 0;

    SDL_Renderer *m_render;
    SDL_Window *m_window;
    SDL_Texture *m_render_tex;
    TTF_Font *m_font;

    // Put this stuff somewhere else
    std::vector<std::unique_ptr<Triangle>> m_scene;
    Camera m_camera;
    std::vector<uint32_t> m_pixels;
};

#endif /* end of include guard: VIEWER_HPP */
