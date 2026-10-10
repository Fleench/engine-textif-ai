#ifndef DOOR_H
#define DOOR_H

#include "GameObject.h"
#include <vector>
#include <functional>

class Game;

class Door : public GameObject {
public:
    bool isOpen = false;
    std::vector<std::string> requiredFlags;
    std::string denyMsg;
    
    // Dynamic conditions
    std::vector<std::function<bool(Game&)>> dynamicConditions;

    Door(const std::string& n) : GameObject(n) {}
    Door(const std::string& i, const std::string& n) : GameObject(i, n) {}

    Door& needs(const std::string& flag) {
        requiredFlags.push_back(flag);
        return *this;
    }

    Door& denyMessage(const std::string& msg) {
        denyMsg = msg;
        return *this;
    }

    Door& canOpenIf(std::function<bool(Game&)> cond) {
        dynamicConditions.push_back(cond);
        return *this;
    }
};

#endif
