#ifndef SYSTEM_CONTROLS_HPP
#define SYSTEM_CONTROLS_HPP

#include "component_position.hpp"
#include "events.hpp"

#include <glm/vec2.hpp>
#include <glm/glm.hpp>

class ControlSystem : public entityx::System<ControlSystem> {
  public:
    void update(entityx::EntityManager &es, entityx::EventManager &events, double dt) {
        entityx::ComponentHandle<Position> position;

        for (entityx::Entity entity : es.entities_with_components(position)) {
            float x = 0.0f;
            float y = 0.0f;

            const Uint8 *state = SDL_GetKeyboardState(NULL);

            if (state[SDL_SCANCODE_W])
                y -= 1.0f;
            if (state[SDL_SCANCODE_A])
                x -= 1.0f;
            if (state[SDL_SCANCODE_S])
                y += 1.0f;
            if (state[SDL_SCANCODE_D])
                x += 1.0f;

            if (x != 0.0f || y != 0.0f) {
                glm::vec2 direction(x, y);
                direction = glm::normalize(direction);
                events.emit<PlayerInstructionEvent>(direction, entity);
            }
        }
    }
};

#endif
