#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "game.hpp"

void mainloop(void *args) {
    auto game = static_cast<Game *>(args);
    game->mainloop();
}

int main() {
    Game game;

    if (game.init() != 0)
        return 1;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(mainloop, &game, 0, true);
#else
    while (game.is_running()) {
        mainloop(&game);
    }
#endif

    return 0;
}
