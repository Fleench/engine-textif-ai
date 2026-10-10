#ifndef NPC_H
#define NPC_H

#include "GameObject.h"
#include "Item.h"

enum class Pronoun { IT, HE, SHE, THEY };

class NPC : public GameObject {
public:
    std::vector<Item*> inventory;
    bool isAlive = true;
    int health = 100;
    bool cutsceneFlag = false;
    std::vector<std::string> dialogue;
    Pronoun pronoun = Pronoun::IT; // F-4: used for HIM/HER/IT/THEM resolution

    NPC(const std::string& n) : GameObject(n) {}
    NPC(const std::string& i, const std::string& n) : GameObject(i, n) {}
};

#endif
