#ifndef MAIN_STATE_MAIN_HPP
#define MAIN_STATE_MAIN_HPP

#include "game.hpp"

#include "strapon/state/state.hpp"

#include "entityx/entityx.h"

class MainState : public State {
  public:
    MainState(Game *game);
    ~MainState();
    int init() override;
    void update(double dt) override;

  private:
    Game *m_game;
    entityx::EventManager m_events;
    entityx::EntityManager m_entities{m_events};
    entityx::SystemManager m_systems{m_entities, m_events};
};

#endif
