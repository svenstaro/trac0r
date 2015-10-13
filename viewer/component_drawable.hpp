#ifndef COMPONENT_DRAWABLE_HPP
#define COMPONENT_DRAWABLE_HPP

#include "entityx/entityx.h"

#include <SDL2/SDL.h>

struct Drawable : entityx::Component<Drawable> {
    Drawable(std::string key, float new_height, float new_width)
        : m_texture_map_key(key), m_height(new_height), m_width(new_width) {
    }

    float height() {
        return m_height;
    }

    float width() {
        return m_width;
    }

    void set_hight(float new_height) {
        m_height = new_height;
    }

    void set_width(float new_width) {
        m_width = new_width;
    }

    std::string texture_key() {
        return m_texture_map_key;
    }

  private:
    std::string m_texture_map_key;
    float m_height;
    float m_width;
};
#endif
