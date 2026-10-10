#include "MiniGame.h"
#include "Game.h"
#include "IOInterface.h"

void MiniGame::run(Game& game) {
    game.io->enterAltScreen();
    std::string result;
    
    if (isRealTimeMode) {
        TermiosGuard guard(game.io); // enters raw/realtime mode, exits on scope end
        
        if (onUpdate) {
            if (onInit) onInit(game);
            bool playing = true;
            while (playing) {
                char c = game.io->getCharAsync();
                playing = onUpdate(game, c);
                if (onRender) onRender(game);
                game.io->sleepFrame(1000 / (fps > 0 ? fps : 30));
            }
            if (onCleanup) result = onCleanup(game);
        } else if (gameRun) {
            result = gameRun(game);
        }
    } else {
        if (gameRun) {
            result = gameRun(game);
        }
    }
    
    game.io->exitAltScreen();
    if (!result.empty()) {
        game.io->print("\n" + result + "\n");
    }
}
