#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "viewer.hpp"

void mainloop(void *args) {
    auto viewer = static_cast<Viewer *>(args);
    viewer->mainloop();
}

int main(int argc, char *argv[]) {
    Viewer viewer;

    if (viewer.init(argc, argv) != 0)
        return 1;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(mainloop, &viewer, 0, true);
#else
    while (viewer.is_running()) {
        mainloop(&viewer);
    }
#endif

    return 0;
}
