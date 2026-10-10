#ifndef ROOM_H
#define ROOM_H

#include "GameObject.h"
#include "Item.h"
#include "Door.h"
#include "NPC.h"
#include <vector>

class Room : public GameObject {
public:
    std::string shortDescription; // BRIEF revisit text; empty = room name only
    bool visited = false;         // set on first entry
    bool isDark = false;          // F-10 darkness

    struct Exit {
        Room* target = nullptr;
        Door* door = nullptr;
    };

    std::vector<Item*> items;
    std::vector<NPC*> npcs;
    std::map<std::string, Exit> exits; // "north", "south", etc.

    Room(const std::string& n) : GameObject(n) {}
    Room(const std::string& i, const std::string& n) : GameObject(i, n) {}

    Room& addExit(const std::string& dir, Room* target, Door* door = nullptr) {
        exits[dir] = {target, door};
        return *this;
    }

    Room& setShortDescription(const std::string& sd) {
        shortDescription = sd;
        return *this;
    }

    Room& addItem(Item* item) {
        items.push_back(item);
        return *this;
    }
    
    Room& addNPC(NPC* npc) {
        npcs.push_back(npc);
        return *this;
    }
};

class ExitHandler {
private:
    std::string destination;
    std::string requiredKey;
    std::string denyMsg;
    bool locked = false;

public:
    ExitHandler& to(const std::string& dest);
    ExitHandler& needs(const std::string& key);
    ExitHandler& denyMessage(const std::string& msg);
    bool tryTraverse(class IOInterface* io, const std::string& playerKey);
};

#endif
