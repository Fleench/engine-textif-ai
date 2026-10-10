#include "../game/game.h"
#include <cstdlib>

#ifdef __EMSCRIPTEN__
#include "WebIO.h"
#include <emscripten.h>
#else
#include "NativeIO.h"
#endif

Game* game = nullptr;

void main_loop() {
    if (game) {
        game->tick();
        if (game->isFinished()) {
#ifdef __EMSCRIPTEN__
            emscripten_cancel_main_loop();
#endif
        }
    }
}

int main() {
    IOInterface* io;
#ifdef __EMSCRIPTEN__
    io = new WebIO();
#else
    io = new NativeIO();
#endif

    game = new Game(io);
    game->init();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, 1);
#else
    while (!game->isFinished()) {
        main_loop();
    }
#endif

    return 0;
}
