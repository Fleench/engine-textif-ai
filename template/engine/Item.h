#ifndef ITEM_H
#define ITEM_H

#include "GameObject.h"
#include <functional>
#include <vector>
#include <algorithm>

class Game;

class ItemDef {
public:
    std::string name;
    std::string description;
    std::function<void(Game&, const std::string&)> onUse;
    std::function<void(Game&, class Item*)> onTake;
    std::map<std::string, std::function<void(Game&, const std::string&)>> customVerbs;

    ItemDef() = default;
    ItemDef(const std::string& n, const std::string& d) : name(n), description(d) {}
};

class Item : public GameObject {
public:
    const ItemDef* def = nullptr;
    int count = 1;
    bool unique = true;
    bool isLightSource = false; // F-10
    bool isLit = false;         // F-10

    Item(const std::string& id, const ItemDef* def)
        : GameObject(id, ""), def(def) {}
    
    std::string getDisplayName() const {
        if (def && !def->name.empty()) return def->name;
        if (!name.empty()) return name;
        return id;
    }

    std::string getMatchName() const {
        std::string s = getDisplayName();
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
        return s;
    }
};

class Storage : public Item {
public:
    std::vector<Item*> items;
    bool isOpen = false;
    bool isSupporter = false; // F-9
    Storage(const std::string& id, const ItemDef* def) : Item(id, def) {}
};

#endif
