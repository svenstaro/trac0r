#ifndef UTILS_HPP
#define UTILS_HPP

#include <cppformat/format.h>

#include <glm/glm.hpp>

#include <SDL.h>
#include <SDL_ttf.h>

#ifdef OPENCL
#include <CL/cl.hpp>
#endif

#include <chrono>
#include <cmath>
#include <string>
#include <thread>

namespace trac0r {

inline void print_sysinfo() {
#ifdef OPENCL
    fmt::print("Rendering on OpenCL\n");
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    for (const auto &platform : platforms) {
        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

        std::string platform_name;
        std::string platform_version;
        platform.getInfo(CL_PLATFORM_NAME, &platform_name);
        platform.getInfo(CL_PLATFORM_VERSION, &platform_version);
        fmt::print("  {} ({})\n", platform_name, platform_version);
        for (size_t i = 0; i < devices.size(); i++) {
            std::string name;
            std::string version;
            std::string driver_version;
            cl_uint max_compute_units;
            cl_uint max_work_item_dimensions;
            cl_uint max_work_group_size;
            cl_ulong max_mem_alloc_size;
            size_t max_parameter_size;
            cl_uint global_mem_cacheline_size;
            cl_ulong global_mem_cache_size;
            cl_ulong global_mem_size;
            cl_ulong max_constant_buffer_size;
            cl_uint max_constant_args;
            cl_ulong local_mem_size;
            devices[i].getInfo(CL_DEVICE_NAME, &name);
            devices[i].getInfo(CL_DEVICE_VERSION, &version);
            devices[i].getInfo(CL_DRIVER_VERSION, &driver_version);
            devices[i].getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &max_compute_units);
            devices[i].getInfo(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, &max_work_item_dimensions);
            devices[i].getInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE, &max_work_group_size);
            devices[i].getInfo(CL_DEVICE_MAX_MEM_ALLOC_SIZE, &max_mem_alloc_size);
            devices[i].getInfo(CL_DEVICE_MAX_PARAMETER_SIZE, &max_parameter_size);
            devices[i].getInfo(CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, &global_mem_cacheline_size);
            devices[i].getInfo(CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, &global_mem_cache_size);
            devices[i].getInfo(CL_DEVICE_GLOBAL_MEM_SIZE, &global_mem_size);
            devices[i].getInfo(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, &max_constant_buffer_size);
            devices[i].getInfo(CL_DEVICE_MAX_CONSTANT_ARGS, &max_constant_args);
            devices[i].getInfo(CL_DEVICE_LOCAL_MEM_SIZE, &local_mem_size);
            fmt::print("    Device {}: {}, OpenCL version: {}, driver version: {}\n", i, name,
                       version, driver_version);
            fmt::print("    Compute units: {}, Max work item dim: {}, Max work group size: {}\n",
                       max_compute_units, max_work_item_dimensions, max_work_group_size);
            fmt::print("    Max mem alloc size: {} MB, Max parameter size: {} B\n",
                       max_mem_alloc_size / (1024 * 1024), max_parameter_size);
            fmt::print("    Global mem cacheline size: {} B, Global mem cache size: {} KB, Global "
                       "mem size: {} MB\n",
                       global_mem_cacheline_size, global_mem_cache_size / 1024,
                       global_mem_size / (1024 * 1024));
            fmt::print("    Max constant buffer size: {} KB, Max constant args: {}, Local mem "
                       "size: {} KB\n",
                       max_constant_buffer_size / 1024, max_constant_args, local_mem_size / 1024);
            fmt::print("\n");
        }
    }
#else
    fmt::print("Rendering on OpenMP\n");
    auto threads = std::thread::hardware_concurrency();
    fmt::print("    OpenMP ({} threads)\n", threads);
#endif
}

inline SDL_Texture *make_text(SDL_Renderer *renderer, TTF_Font *font, std::string text,
                              const SDL_Color &color) {
    auto text_surface = TTF_RenderText_Blended(font, text.c_str(), color);
    auto text_tex = SDL_CreateTextureFromSurface(renderer, text_surface);
    SDL_FreeSurface(text_surface);

    return text_tex;
}

inline void render_text(SDL_Renderer *renderer, SDL_Texture *texture, int pos_x, int pos_y) {
    int tex_width;
    int tex_height;

    SDL_QueryTexture(texture, 0, 0, &tex_width, &tex_height);
    SDL_Rect rect{pos_x, pos_y, tex_width, tex_height};
    SDL_RenderCopy(renderer, texture, 0, &rect);
}

inline uint32_t pack_color_argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    uint32_t new_color = a << 24 | r << 16 | g << 8 | b;
    return new_color;
}

inline uint32_t pack_color_argb(glm::i8vec4 color) {
    uint32_t new_color = color.a << 24 | color.r << 16 | color.g << 8 | color.b;
    return new_color;
}

inline uint32_t pack_color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    uint32_t packed_color = r << 24 | g << 16 | b << 8 | a;
    return packed_color;
}

inline uint32_t pack_color_rgba(glm::i8vec4 color) {
    uint32_t packed_color = color.r << 24 | color.g << 16 | color.b << 8 | color.a;
    return packed_color;
}

inline uint32_t pack_color_rgba(glm::vec4 color) {
    uint32_t packed_color =
        static_cast<int>(glm::round(glm::clamp(color.r, 0.f, 1.f) * 255)) << 24 |
        static_cast<int>(glm::round(glm::clamp(color.g, 0.f, 1.f) * 255)) << 16 |
        static_cast<int>(glm::round(glm::clamp(color.b, 0.f, 1.f) * 255)) << 8 |
        static_cast<int>(glm::round(glm::clamp(color.a, 0.f, 1.f) * 255));
    return packed_color;
}

inline uint32_t pack_color_argb(glm::vec4 color) {
    uint32_t packed_color =
        static_cast<int>(glm::round(glm::clamp(color.a, 0.f, 1.f) * 255)) << 24 |
        static_cast<int>(glm::round(glm::clamp(color.r, 0.f, 1.f) * 255)) << 16 |
        static_cast<int>(glm::round(glm::clamp(color.g, 0.f, 1.f) * 255)) << 8 |
        static_cast<int>(glm::round(glm::clamp(color.b, 0.f, 1.f) * 255));
    return packed_color;
}

inline glm::i8vec4 unpack_color_rgba_to_i8vec4(uint32_t packed_color_rgba) {
    glm::i8vec4 unpacked_color;
    unpacked_color.r = packed_color_rgba >> 24 & 0xFF;
    unpacked_color.g = packed_color_rgba >> 16 & 0xFF;
    unpacked_color.b = packed_color_rgba >> 8 & 0xFF;
    unpacked_color.a = packed_color_rgba & 0xFF;
    return unpacked_color;
}

inline glm::i8vec4 unpack_color_argb_to_i8vec4(uint32_t packed_color_argb) {
    glm::i8vec4 unpacked_color;
    unpacked_color.a = packed_color_argb >> 24 & 0xFF;
    unpacked_color.r = packed_color_argb >> 16 & 0xFF;
    unpacked_color.g = packed_color_argb >> 8 & 0xFF;
    unpacked_color.b = packed_color_argb & 0xFF;
    return unpacked_color;
}

inline glm::vec4 unpack_color_rgbb_to_vec4(uint32_t packed_color_rgba) {
    glm::i8vec4 unpacked_color;
    unpacked_color.r = (packed_color_rgba >> 24 & 0xFF) / 255.f;
    unpacked_color.g = (packed_color_rgba >> 16 & 0xFF) / 255.f;
    unpacked_color.b = (packed_color_rgba >> 8 & 0xFF) / 255.f;
    unpacked_color.a = (packed_color_rgba & 0xFF) / 255.f;
    return unpacked_color;
}

inline glm::vec4 unpack_color_argb_to_vec4(uint32_t packed_color_argb) {
    glm::vec4 unpacked_color;
    unpacked_color.a = (packed_color_argb >> 24 & 0xFF) / 255.f;
    unpacked_color.r = (packed_color_argb >> 16 & 0xFF) / 255.f;
    unpacked_color.g = (packed_color_argb >> 8 & 0xFF) / 255.f;
    unpacked_color.b = (packed_color_argb & 0xFF) / 255.f;
    return unpacked_color;
}
}

#endif /* end of include guard: UTILS_HPP */
