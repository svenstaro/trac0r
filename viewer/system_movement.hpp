#ifndef MOVEMENT_SYSTEM_CPP
#define MOVEMENT_SYSTEM_CPP

#include "events.hpp"
#include "game.hpp"
#include "component_position.hpp"

#include "entityx/entityx.h"

#include <glm/vec2.hpp>
#include <type_traits>

class MovementSystem : public entityx::System<MovementSystem>,
                       public entityx::Receiver<MovementSystem> {
  public:
    MovementSystem() {
    }

    void configure(entityx::EventManager &event_manager) override {
        event_manager.subscribe<PlayerInstructionEvent>(*this);
    }

    void update(entityx::EntityManager &es, entityx::EventManager &events,
                entityx::TimeDelta dt) override {
    }

    void receive(const PlayerInstructionEvent &player_instruction_event) {
        auto copy = player_instruction_event;
        auto position = copy.m_entity.component<Position>();
        position->set_position(position->position() + player_instruction_event.m_direction);
    }
};
#endif
