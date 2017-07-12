#ifndef UTILS_HPP
#define UTILS_HPP

#include <fmt/format.h>

#include <glm/glm.hpp>
#include <glm/gtx/log_base.hpp>

#include <SDL.h>
#include <SDL_ttf.h>

#ifdef OPENCL
#include <CL/cl.hpp>
#endif

#include <chrono>
#include <cmath>
#include <string>
#include <vector>

namespace trac0r {

/**
 * @brief Calculate Mean Square Error of two frames
 *
 * @param frame1 The first frame to compare against
 * @param frame2 The second frame to compare against the first one
 *
 * @return Mean square error as a scalar
 */
inline float mse(std::vector<glm::vec4> frame1, std::vector<glm::vec4> frame2) {
    glm::vec4 error{0.f};
    for (size_t n = 0; n < frame1.size(); n++) {
        glm::vec4 difference = frame1[n] - frame2[n];
        error += difference * difference;
    }

    // We'll use only RGB channels because A is always 1.f and thusly not meaningful
    float mean = (error.r + error.g + error.b) / 3.f;
    return mean / frame1.size();
}

/**
 * @brief Peak signal-to-noise ratio
 *
 * @param frame1 The first frame to compare against
 * @param frame2 The second frame to compare against the first one
 *
 * @return
 */
inline float psnr(std::vector<glm::vec4> frame1, std::vector<glm::vec4> frame2) {
    return 10.f * glm::log(1.f / mse(frame1, frame2), 10.f);
}

/**
 * @brief Returns a vector orthogonal to a given vector in 3D space.
 *
 * @param v The vector to find an orthogonal vector for
 *
 * @return A vector orthogonal to the given vector
 */
inline glm::vec3 ortho(glm::vec3 v) {
    // Awesome branchless function for finding an orthogonal vector in 3D space by
    // http://lolengine.net/blog/2013/09/21/picking-orthogonal-vector-combing-coconuts
    //
    // Their "boring" branching is commented here for completeness:
    // return glm::abs(v.x) > glm::abs(v.z) ? glm::vec3(-v.y, v.x, 0.0) : glm::vec3(0.0, -v.z, v.y);

    float k = glm::fract(glm::abs(v.x) + 0.5f);
    return glm::vec3(-v.y, v.x - k * v.z, k * v.y);
}

inline glm::vec3 get_middle_point(glm::vec3 v1, glm::vec3 v2) {
    return (v1 - v2) / 2.f + v2;
}

#ifdef OPENCL
inline std::string opencl_error_string(cl_int error) {
    switch (error) {
    // run-time and JIT compiler errors
    case 0:
        return "CL_SUCCESS";
    case -1:
        return "CL_DEVICE_NOT_FOUND";
    case -2:
        return "CL_DEVICE_NOT_AVAILABLE";
    case -3:
        return "CL_COMPILER_NOT_AVAILABLE";
    case -4:
        return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
    case -5:
        return "CL_OUT_OF_RESOURCES";
    case -6:
        return "CL_OUT_OF_HOST_MEMORY";
    case -7:
        return "CL_PROFILING_INFO_NOT_AVAILABLE";
    case -8:
        return "CL_MEM_COPY_OVERLAP";
    case -9:
        return "CL_IMAGE_FORMAT_MISMATCH";
    case -10:
        return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
    case -11:
        return "CL_BUILD_PROGRAM_FAILURE";
    case -12:
        return "CL_MAP_FAILURE";
    case -13:
        return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
    case -14:
        return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
    case -15:
        return "CL_COMPILE_PROGRAM_FAILURE";
    case -16:
        return "CL_LINKER_NOT_AVAILABLE";
    case -17:
        return "CL_LINK_PROGRAM_FAILURE";
    case -18:
        return "CL_DEVICE_PARTITION_FAILED";
    case -19:
        return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

    // compile-time errors
    case -30:
        return "CL_INVALID_VALUE";
    case -31:
        return "CL_INVALID_DEVICE_TYPE";
    case -32:
        return "CL_INVALID_PLATFORM";
    case -33:
        return "CL_INVALID_DEVICE";
    case -34:
        return "CL_INVALID_CONTEXT";
    case -35:
        return "CL_INVALID_QUEUE_PROPERTIES";
    case -36:
        return "CL_INVALID_COMMAND_QUEUE";
    case -37:
        return "CL_INVALID_HOST_PTR";
    case -38:
        return "CL_INVALID_MEM_OBJECT";
    case -39:
        return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
    case -40:
        return "CL_INVALID_IMAGE_SIZE";
    case -41:
        return "CL_INVALID_SAMPLER";
    case -42:
        return "CL_INVALID_BINARY";
    case -43:
        return "CL_INVALID_BUILD_OPTIONS";
    case -44:
        return "CL_INVALID_PROGRAM";
    case -45:
        return "CL_INVALID_PROGRAM_EXECUTABLE";
    case -46:
        return "CL_INVALID_KERNEL_NAME";
    case -47:
        return "CL_INVALID_KERNEL_DEFINITION";
    case -48:
        return "CL_INVALID_KERNEL";
    case -49:
        return "CL_INVALID_ARG_INDEX";
    case -50:
        return "CL_INVALID_ARG_VALUE";
    case -51:
        return "CL_INVALID_ARG_SIZE";
    case -52:
        return "CL_INVALID_KERNEL_ARGS";
    case -53:
        return "CL_INVALID_WORK_DIMENSION";
    case -54:
        return "CL_INVALID_WORK_GROUP_SIZE";
    case -55:
        return "CL_INVALID_WORK_ITEM_SIZE";
    case -56:
        return "CL_INVALID_GLOBAL_OFFSET";
    case -57:
        return "CL_INVALID_EVENT_WAIT_LIST";
    case -58:
        return "CL_INVALID_EVENT";
    case -59:
        return "CL_INVALID_OPERATION";
    case -60:
        return "CL_INVALID_GL_OBJECT";
    case -61:
        return "CL_INVALID_BUFFER_SIZE";
    case -62:
        return "CL_INVALID_MIP_LEVEL";
    case -63:
        return "CL_INVALID_GLOBAL_WORK_SIZE";
    case -64:
        return "CL_INVALID_PROPERTY";
    case -65:
        return "CL_INVALID_IMAGE_DESCRIPTOR";
    case -66:
        return "CL_INVALID_COMPILER_OPTIONS";
    case -67:
        return "CL_INVALID_LINKER_OPTIONS";
    case -68:
        return "CL_INVALID_DEVICE_PARTITION_COUNT";

    // extension errors
    case -1000:
        return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
    case -1001:
        return "CL_PLATFORM_NOT_FOUND_KHR";
    case -1002:
        return "CL_INVALID_D3D10_DEVICE_KHR";
    case -1003:
        return "CL_INVALID_D3D10_RESOURCE_KHR";
    case -1004:
        return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
    case -1005:
        return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
    default:
        return "Unknown OpenCL error";
    }
}
#endif

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
