#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <stdexcept>
#include "GameObject.h"
#include "Game.h"

void GameObject::supports(Game& game, const std::string& name) {
    if (!game.verbRegistry.count(name)) {
        throw std::runtime_error("Verb '" + name + "' is not registered globally.");
    }
    verbs[name] = [name](Game& g, GameObject* obj, const Action& act) {
        if (g.verbRegistry.count(name) && g.verbRegistry[name].defaultFn) {
            g.verbRegistry[name].defaultFn(g, act);
        }
    };
}

void GameObject::supports(Game& game, const std::string& name, VerbFn func) {
    if (!game.verbRegistry.count(name)) {
        throw std::runtime_error("Verb '" + name + "' is not registered globally.");
    }
    verbs[name] = func;
}
