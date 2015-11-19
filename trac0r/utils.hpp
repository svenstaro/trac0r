#ifndef UTILS_HPP
#define UTILS_HPP

#include <cppformat/format.h>

#include <glm/glm.hpp>

#include <SDL.h>
#include <SDL_ttf.h>

#include <cmath>
#include <string>

namespace trac0r {

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

// Möller-Trumbore intersection algorithm
// (see https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm)
inline bool intersect_ray_triangle(const glm::vec3 &origin, const glm::vec3 &dir, const glm::vec3 &v0,
                            glm::vec3 &v1, glm::vec3 &v2, float &dist) {
    // Calculate edges of triangle from v0.
    auto e0 = v1 - v0;
    auto e1 = v2 - v0;

    // Calculate determinant to check whether the ray is in the newly calculated plane made up from
    // e0 and e1.
    auto pvec = glm::cross(dir, e1);
    auto det = glm::dot(e0, pvec);

    // Check whether determinant is close to 0. If that is the case, the ray is in the same plane as
    // the triangle itself which means that they can't collide. This effectively disables backface
    // culling for which we would instead only check whether det < epsilon.
    auto epsilon = std::numeric_limits<float>::epsilon();
    if (det > -epsilon && det < epsilon)
        return false;

    auto inv_det = 1.f / det;

    // Calculate distance from v0 to ray origin
    auto tvec = origin - v0;

    // Calculate u parameter and test bound
    auto u = glm::dot(tvec, pvec) * inv_det;

    // Check whether the intersection lies outside of the triangle
    if (u < 0.f || u > 1.f)
        return false;

    // Prepare to test v parameter
    auto qvec = glm::cross(tvec, e0);

    // Calculate v parameter and test bound
    auto v = glm::dot(dir, qvec) * inv_det;

    // Check whether the intersection lies outside of the triangle
    if (v < 0.f || u + v > 1.f)
        return false;

    auto t = glm::dot(e1, qvec) * inv_det;

    if (t > epsilon) {
        dist = t;
        return true;
    }

    // If we end up here, there was no hit
    return false;
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
    uint32_t packed_color = int(std::round(color.r * 255)) << 24 |
                            int(std::round(color.g * 255)) << 16 |
                            int(std::round(color.b * 255)) << 8 | int(std::round(color.a * 255));
    return packed_color;
}

inline uint32_t pack_color_argb(glm::vec4 color) {
    uint32_t packed_color = int(std::round(color.a * 255)) << 24 |
                            int(std::round(color.r * 255)) << 16 |
                            int(std::round(color.g * 255)) << 8 | int(std::round(color.b * 255));
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
