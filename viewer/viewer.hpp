#ifndef VIEWER_HPP
#define VIEWER_HPP

#include "trac0r/camera.hpp"
#include "trac0r/triangle.hpp"
#include "trac0r/renderer.hpp"
#include "trac0r/scene.hpp"

#include <SDL.h>
#include <SDL_ttf.h>

#include <array>
#include <memory>
#include <vector>

// Right-handed coordinate system where Y is up and Z is depth

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

  private:
    bool m_scene_changed = false;
    int m_samples_accumulated = 0;
    glm::vec3 m_scene_up = {0, 1, 0};
    bool m_running = true;
    bool m_look_mode = false;
    int m_last_frame_time = 0;
    bool m_debug = true;
    bool m_print_perf = true;
    int m_x_stride = 2;
    int m_y_stride = 2;
    int m_frame = 0;
#ifdef BENCHMARK
    int m_max_frames = 10; // run indefinintely
#else
    int m_max_frames = -1; // run indefinintely
#endif

    SDL_Renderer *m_render;
    SDL_Window *m_window;
    SDL_Texture *m_render_tex;
    TTF_Font *m_font;

    std::vector<uint32_t> m_pixels;

    trac0r::Camera m_camera;
    trac0r::Scene m_scene;
    std::unique_ptr<trac0r::Renderer> m_renderer;
};

#endif /* end of include guard: VIEWER_HPP */
