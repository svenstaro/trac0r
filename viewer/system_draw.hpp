#include "game.hpp"
#include "component_drawable.hpp"
#include "component_position.hpp"

#include "strapon/resource_manager/resource_manager.hpp"

#include "entityx/entityx.h"
#include <glm/vec2.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

class DrawSystem : public entityx::System<DrawSystem> {
  public:
    DrawSystem(Game *game) : m_game(game) {
        int w, h;
        SDL_RenderGetLogicalSize(game->renderer(), &w, &h);
        m_camera = SDL_Rect{0, 0, w, h};
        m_drawtex =
            SDL_CreateTexture(game->renderer(), SDL_PIXELTYPE_UNKNOWN, SDL_TEXTUREACCESS_TARGET,
                              game->world_size().w, game->world_size().h);
    }

    ~DrawSystem() {
        SDL_DestroyTexture(m_drawtex);
    }

    void update(entityx::EntityManager &es, entityx::EventManager &events,
                entityx::TimeDelta dt) override {

        // Change to render into rendertexture for now
        SDL_SetRenderTarget(m_game->renderer(), m_drawtex);
        SDL_SetRenderDrawColor(m_game->renderer(), 0, 100, 200, 255);
        SDL_RenderClear(m_game->renderer());

        entityx::ComponentHandle<Drawable> drawable;
        entityx::ComponentHandle<Position> position;

        for (entityx::Entity entity : es.entities_with_components(drawable, position)) {

            (void)entity;

            SDL_Rect dest;
            dest.x = position->position()[0];
            dest.y = position->position()[1];
            dest.w = drawable->width();
            dest.h = drawable->height();

            SDL_RenderCopyEx(m_game->renderer(),
                             m_game->res_manager().texture(drawable->texture_key()), NULL, &dest, 0,
                             NULL, SDL_FLIP_NONE);
        }

        auto surf = TTF_RenderText_Blended(m_game->res_manager().font("font20"), "LOL", {200, 100, 100, 150});
        auto text = SDL_CreateTextureFromSurface(m_game->renderer(), surf);
        SDL_Rect dest = {0, 0, 50, 50};
        SDL_RenderCopy(m_game->renderer(), text, NULL, &dest); 
        SDL_FreeSurface(surf);

        // Render to final window
        SDL_SetRenderTarget(m_game->renderer(), nullptr);
        SDL_RenderCopy(m_game->renderer(), m_drawtex, &m_camera, nullptr);
        SDL_RenderPresent(m_game->renderer());
    }

  private:
    Game *m_game;
    SDL_Rect m_camera;
    SDL_Texture *m_drawtex;
};
