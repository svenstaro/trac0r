#ifndef COLLISION_SYSTEM_CPP
#define COLLISION_SYSTEM_CPP

#include "component_position.hpp"
#include "events.hpp"
#include "game.hpp"

#include "entityx/entityx.h"

#include <glm/vec2.hpp>
#include <SDL2/SDL.h>

class CollisionSystem : public entityx::System<CollisionSystem> {
  public:
    CollisionSystem() {
    }

    void update(entityx::EntityManager &es, entityx::EventManager &events, double dt) {
        entityx::ComponentHandle<Position> first_position, second_position;
        for (entityx::Entity first_entity : es.entities_with_components(first_position)) {
            for (entityx::Entity second_entity : es.entities_with_components(second_position)) {
                if (first_entity != second_entity &&
                    SDL_HasIntersection(&first_position->rect(), &second_position->rect())) {
                    events.emit<CollisionEvent>(first_entity, second_entity);
                }
            }
        }
    }
};

#endif
