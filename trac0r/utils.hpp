#ifndef UTILS_HPP
#define UTILS_HPP

#include <glm/glm.hpp>

#include <SDL.h>
#include <SDL_ttf.h>

#include <cmath>
#include <string>
#include <iostream>

namespace trac0r {

SDL_Texture *make_text(SDL_Renderer *renderer, TTF_Font *font, std::string text,
                       const SDL_Color &color) {
    auto text_surface = TTF_RenderText_Blended(font, text.c_str(), color);
    auto text_tex = SDL_CreateTextureFromSurface(renderer, text_surface);
    SDL_FreeSurface(text_surface);

    return text_tex;
}

void render_text(SDL_Renderer *renderer, SDL_Texture *texture, int pos_x, int pos_y) {
    int tex_width;
    int tex_height;

    SDL_QueryTexture(texture, 0, 0, &tex_width, &tex_height);
    SDL_Rect rect{pos_x, pos_y, tex_width, tex_height};
    SDL_RenderCopy(renderer, texture, 0, &rect);
}

uint32_t pack_color_argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    uint32_t new_color = a << 24 | r << 16 | g << 8 | b;
    return new_color;
}

uint32_t pack_color_argb(glm::i8vec4 color) {
    uint32_t new_color = color.a << 24 | color.r << 16 | color.g << 8 | color.b;
    return new_color;
}

uint32_t pack_color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    uint32_t packed_color = r << 24 | g << 16 | b << 8 | a;
    return packed_color;
}

uint32_t pack_color_rgba(glm::i8vec4 color) {
    uint32_t packed_color = color.r << 24 | color.g << 16 | color.b << 8 | color.a;
    return packed_color;
}

uint32_t pack_color_rgba(glm::vec4 color) {
    uint32_t packed_color = int(std::round(color.r * 255)) << 24 |
                            int(std::round(color.g * 255)) << 16 |
                            int(std::round(color.b * 255)) << 8 | int(std::round(color.a * 255));
    return packed_color;
}

uint32_t pack_color_argb(glm::vec4 color) {
    uint32_t packed_color = int(std::round(color.a * 255)) << 24 |
                            int(std::round(color.r * 255)) << 16 |
                            int(std::round(color.g * 255)) << 8 | int(std::round(color.b * 255));
    return packed_color;
}

glm::i8vec4 unpack_color_rgba_to_i8vec4(uint32_t packed_color_rgba) {
    glm::i8vec4 unpacked_color;
    unpacked_color.r = packed_color_rgba >> 24 & 0xFF;
    unpacked_color.g = packed_color_rgba >> 16 & 0xFF;
    unpacked_color.b = packed_color_rgba >> 8 & 0xFF;
    unpacked_color.a = packed_color_rgba & 0xFF;
    return unpacked_color;
}

glm::i8vec4 unpack_color_argb_to_i8vec4(uint32_t packed_color_argb) {
    glm::i8vec4 unpacked_color;
    unpacked_color.a = packed_color_argb >> 24 & 0xFF;
    unpacked_color.r = packed_color_argb >> 16 & 0xFF;
    unpacked_color.g = packed_color_argb >> 8 & 0xFF;
    unpacked_color.b = packed_color_argb & 0xFF;
    return unpacked_color;
}

glm::vec4 unpack_color_rgbb_to_vec4(uint32_t packed_color_rgba) {
    glm::i8vec4 unpacked_color;
    unpacked_color.r = (packed_color_rgba >> 24 & 0xFF) / 255.f;
    unpacked_color.g = (packed_color_rgba >> 16 & 0xFF) / 255.f;
    unpacked_color.b = (packed_color_rgba >> 8 & 0xFF) / 255.f;
    unpacked_color.a = (packed_color_rgba & 0xFF) / 255.f;
    return unpacked_color;
}

glm::vec4 unpack_color_argb_to_vec4(uint32_t packed_color_argb) {
    glm::i8vec4 unpacked_color;
    unpacked_color.a = (packed_color_argb >> 24 & 0xFF) / 255.f;
    unpacked_color.r = (packed_color_argb >> 16 & 0xFF) / 255.f;
    unpacked_color.g = (packed_color_argb >> 8 & 0xFF) / 255.f;
    unpacked_color.b = (packed_color_argb & 0xFF) / 255.f;
    return unpacked_color;
}
}

#endif /* end of include guard: UTILS_HPP */
