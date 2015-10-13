#ifndef POSITION_HPP
#define POSITION_HPP

#include "entityx/entityx.h"
#include <glm/vec2.hpp>
#include <SDL2/SDL.h>

struct Position : entityx::Component<Position> {
    Position(glm::vec2 new_position = glm::vec2(0.0f, 0.0f), int width = 0, int height = 0)
        : m_position(new_position),
          m_rect{int(new_position.x), int(new_position.y), width, height} {
    }

    glm::vec2 position() {
        return m_position;
    }

    SDL_Rect &rect() {
        return m_rect;
    }

    void set_position(glm::vec2 new_position) {
        m_position = new_position;
        m_rect.x = new_position.x;
        m_rect.y = new_position.y;
    }

  private:
    glm::vec2 m_position;
    SDL_Rect m_rect;
};

#endif
