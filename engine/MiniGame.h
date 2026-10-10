#ifndef MINIGAME_H
#define MINIGAME_H

#include "GameObject.h"
#include <functional>
#include <string>

class Game;

class MiniGame : public GameObject {
public:
    // Legacy blocking execution
    std::function<std::string(Game&)> gameRun;

    // Modern frame-based execution
    std::function<void(Game&)> onInit;
    std::function<bool(Game&, char)> onUpdate; // return false to exit loop
    std::function<void(Game&)> onRender;
    std::function<std::string(Game&)> onCleanup;

    bool isRealTimeMode = false;
    int fps = 30; // Target frames per second for engine-managed loop

    MiniGame(const std::string& n) : GameObject(n) {}
    MiniGame(const std::string& i, const std::string& n) : GameObject(i, n) {}

    void run(Game& game);
};

#endif
