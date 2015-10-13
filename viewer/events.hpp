#ifndef EVENTS_HPP
#define EVENTS_HPP

#include <glm/vec2.hpp>

#include "entityx/entityx.h"

struct PlayerInstructionEvent {
    PlayerInstructionEvent(glm::vec2 direction, entityx::Entity entity)
        : m_direction(direction), m_entity(entity) {
    }

    glm::vec2 m_direction;
    entityx::Entity m_entity;
};

struct CollisionEvent {
    CollisionEvent(entityx::Entity first, entityx::Entity second)
        : m_first(first), m_second(second) {
    }

    entityx::Entity m_first;
    entityx::Entity m_second;
};

#endif
