#ifndef UTILS_HPP
#define UTILS_HPP

#include <SDL.h>
#include <SDL_ttf.h>

#include <string>

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
}

#endif /* end of include guard: UTILS_HPP */
