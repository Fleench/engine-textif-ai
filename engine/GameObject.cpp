#pragma GCC diagnostic ignored "-Wunused-parameter"

#include "GameObject.h"
#include "Game.h"

void GameObject::supports(const std::string& name) {
    verbs[name] = [name](Game& g, GameObject* obj, const Action& act) {
        if (g.verbRegistry.count(name) && g.verbRegistry[name].defaultFn) {
            g.verbRegistry[name].defaultFn(g, act);
        }
    };
}

void GameObject::supports(const std::string& name, VerbFn func) {
    verbs[name] = func;
}
