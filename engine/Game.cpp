#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "../engine/Game.h"
#include "../engine/Room.h"
#include "../engine/Item.h"
#include "../engine/Door.h"
#include "../engine/MiniGame.h"
#include "../engine/NPC.h"

#include <stdexcept>
#include <cstdio>
#include <sstream>
#include <algorithm>
#include <fstream>

class DummyCout {
    IOInterface* io;
public:
    DummyCout(IOInterface* io) : io(io) {}
    template<typename T>
    DummyCout& operator<<(const T& val) {
        std::ostringstream oss;
        oss << val;
        if (io) io->print(oss.str());
        return *this;
    }
};

class DummyCin {
    IOInterface* io;
public:
    DummyCin(IOInterface* io) : io(io) {}
    template<typename T>
    DummyCin& operator>>(T& val) {
        if (io) {
            std::istringstream iss(io->requestInput());
            iss >> val;
        }
        return *this;
    }
};

#undef cout
#undef cin
#undef endl
#define cout DummyCout(this->io)
#define cin DummyCin(this->io)
#define endl "\n"

bool Game::isLit() const {
    if (currentRoom && !currentRoom->isDark) return true;
    for (auto* itm : inventory) {
        if (itm->isLightSource && itm->isLit) return true;
    }
    if (currentRoom) {
        for (auto* itm : currentRoom->items) {
            if (itm->isLightSource && itm->isLit) return true;
            if (Storage* st = dynamic_cast<Storage*>(itm)) {
                if (st->isOpen || st->isSupporter) {
                    for (auto* sitm : st->items) {
                        if (sitm->isLightSource && sitm->isLit) return true;
                    }
                }
            }
        }
    }
    return false;
}

void Game::addResponse(const std::string& key, const std::vector<std::string>& variants) {
    responses[key] = variants;
}

std::string Game::respond(const std::string& key) {
    auto it = responses.find(key);
    if (it == responses.end() || it->second.empty()) return key; // Fallback
    
    if (it->second.size() == 1) return it->second[0];
    
    int last = -1;
    if (lastResponseIdx.count(key)) last = lastResponseIdx[key];
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    int next;
    do {
        std::uniform_int_distribution<> dist(0, it->second.size() - 1);
        next = dist(gen);
    } while (next == last && it->second.size() > 1);
    
    lastResponseIdx[key] = next;
    return it->second[next];
}


Action Game::parseInput(const std::string& input) {
    Action act;
    if (input.empty()) return act;

    std::istringstream iss(input);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) {
        std::transform(token.begin(), token.end(), token.begin(), [](unsigned char c){ return std::tolower(c); });
        if (token == "the" || token == "a" || token == "an") continue;
        
        // F-4 Pronouns
        if (token == "it" && !lastObject.empty()) token = lastObject;
        else if ((token == "him" || token == "her") && !lastNPC.empty()) token = lastNPC;
        else if (token == "them" && !lastPlural.empty()) token = lastPlural;
        else if (token == "it" || token == "him" || token == "her" || token == "them") {
            cout << "\nYou can't see any such thing.\n";
            act.verb = "unknown";
            return act;
        }
        
        tokens.push_back(token);
    }

    if (tokens.empty()) return act;

    act.rawVerb = tokens[0];
    for (size_t i = 1; i < tokens.size(); ++i) {
        if (i > 1) act.rawArgs += " ";
        act.rawArgs += tokens[i];
    }
    act.argc = tokens.size();
    

    act.verb = tokens[0];
    if (tokens[0] == "exit" && tokens.size() == 1) {
        act.object = "out";
        act.argc = 2;
    }

    size_t verbTokens = 1;
    bool known = false;
    std::string v = act.verb;
    
    if (tokens.size() > 1) {
        std::string twoWord = tokens[0] + " " + tokens[1];
        if (verbSynonyms.count(twoWord)) twoWord = verbSynonyms[twoWord];
        
        bool foundTwoWord = false;
        if (verbRegistry.count(twoWord)) foundTwoWord = true;
        if (!foundTwoWord) {
            for (auto* item : inventory) {
                if (item->verbs.count(twoWord) || (item->def && item->def->customVerbs.count(twoWord))) { foundTwoWord = true; break; }
            }
        }
        if (!foundTwoWord && currentRoom) {
            for (auto* item : currentRoom->items) {
                if (item->verbs.count(twoWord) || (item->def && item->def->customVerbs.count(twoWord))) { foundTwoWord = true; break; }
            }
        }
        if (foundTwoWord) {
            act.rawVerb = twoWord;
            act.verb = twoWord;
            verbTokens = 2;
            known = true;
            v = twoWord;
        }
    }

    if (!known) {
        if (verbSynonyms.count(v)) v = verbSynonyms[v];
        if (verbRegistry.count(v)) known = true;
        else if (v == "n" || v == "s" || v == "e" || v == "w" || v == "north" || v == "south" || v == "east" || v == "west" || v == "up" || v == "down" || v == "in" || v == "out" || v == "ne" || v == "nw" || v == "se" || v == "sw" || v == "inside" || v == "outside") known = true;
        else if (v == "g" || v == "again" || v == "oops") known = true;
        else {
            for (auto* item : inventory) {
                if (item->verbs.count(act.verb) || (item->def && item->def->customVerbs.count(act.verb))) { known = true; break; }
            }
            if (!known && currentRoom) {
                for (auto* item : currentRoom->items) {
                    if (item->verbs.count(act.verb) || (item->def && item->def->customVerbs.count(act.verb))) { known = true; break; }
                }
            }
        }
    }

    if (!known) {
        act.verb = "unknown";
        return act;
    }

    size_t prepIdx = tokens.size();
    if (act.verb != "go") {
        for (size_t i = verbTokens; i < tokens.size(); ++i) {
            if (prepMap.count(tokens[i])) {
                prepIdx = i;
                break;
            }
        }
    }

    if (prepIdx > verbTokens && tokens.size() >= verbTokens + 1) {
        act.object = tokens[verbTokens];
        for (size_t i = verbTokens + 1; i < prepIdx; ++i) {
            act.object += " " + tokens[i];
        }
        if (objectSynonyms.count(act.object)) act.object = objectSynonyms[act.object];
    }

    if (prepIdx < tokens.size()) {
        act.prep = prepMap[tokens[prepIdx]];
        if (prepIdx + 1 < tokens.size()) {
            act.object2 = tokens[prepIdx + 1];
            for (size_t i = prepIdx + 2; i < tokens.size(); ++i) {
                act.object2 += " " + tokens[i];
            }
            if (objectSynonyms.count(act.object2)) act.object2 = objectSynonyms[act.object2];
        }
    }

    return act;
}

void Game::registerDefaultVerbs() {

    verbRegistry["quit"] = { [this](Game& game, const Action& action) {

            isRunning = false;
            
    }, false, "" };

    verbRegistry["look"] = { [this](Game& game, const Action& action) {

            if (action.argc > 1 && (!action.object.empty() || action.prep == Preposition::AT)) {
                std::string target = action.object2;
                if (target.empty()) target = action.object;

                Item* found = nullptr;
                for (auto* itm : inventory) { if (itm->getMatchName() == target) found = itm; }
                if (!found && currentRoom) {
                    for (auto* itm : currentRoom->items) {
                        if (itm->getMatchName() == target) {
                            found = itm;
                            break;
                        }
                        if (Storage* st = dynamic_cast<Storage*>(itm)) {
                            if (st->isOpen || st->isSupporter) {
                                for (auto* sitm : st->items) {
                                    if (sitm->getMatchName() == target) {
                                        found = sitm;
                                        break;
                                    }
                                }
                            }
                        }
                        if (found) break;
                    }
                }
                if (found) {
                    cout << "\n" << (found->def && !found->def->description.empty() ? found->def->description : found->description) << "\n";
                    if (Storage* st = dynamic_cast<Storage*>(found)) {
                        if (st->isOpen || st->isSupporter) {
                            if (st->items.empty()) {
                                cout << (st->isSupporter ? "There is nothing on it.\n" : "It is empty.\n");
                            } else {
                                cout << (st->isSupporter ? "On it, you see:\n" : "It contains:\n");
                                for (auto* si : st->items) {
                                    cout << " - " << si->getDisplayName() << "\n";
                                }
                            }
                        } else {
                            cout << "It is closed.\n";
                        }
                    }
                } else {
                    cout << respond("dont_see");
                }
            } else if (currentRoom) {
                if (!isLit()) {
                    cout << "\nIt is pitch black. You are likely to be eaten by a grue.\n";
                } else {
                    bool wasVisited = currentRoom->visited;
                    currentRoom->visited = true;
                    cout << "\n" << currentRoom->name << "\n";
                    if (verbosityMode == VERBOSE || (verbosityMode == BRIEF && !wasVisited)) {
                        cout << "\n" << currentRoom->description << "\n";
                    } else if (verbosityMode == BRIEF && wasVisited) {
                        if (!currentRoom->shortDescription.empty()) cout << "\n" << currentRoom->shortDescription << "\n";
                    }
                    if (!currentRoom->items.empty()) {
                        cout << "\nYou see here:\n";
                        for (auto* item : currentRoom->items) {
                            cout << " - " << item->getDisplayName() << "\n";
                        }
                    }
                    if (!currentRoom->npcs.empty()) {
                        cout << "\nAlso here:\n";
                        for (auto* npc : currentRoom->npcs) {
                            cout << " - " << npc->getDisplayName() << "\n";
                        }
                    }
                    cout << "\nExits: ";
                    for (const auto& exit : currentRoom->exits) {
                        cout << exit.first << " ";
                    }
                    cout << "\n";
                }
            }
            
    }, false, "" };

    verbRegistry["inventory"] = { [this](Game& game, const Action& action) {

            if (inventory.empty()) {
                cout << "\nYou are empty-handed.\n";
            } else {
                cout << "\nYou are carrying:\n";
                std::map<std::string, int> counts;
                std::vector<std::string> order;
                for (auto* item : inventory) {
                    std::string dname = item->getDisplayName();
                    if (counts[dname] == 0) {
                        order.push_back(dname);
                    }
                    counts[dname] += item->count;
                }
                for (const auto& n : order) {
                    if (counts[n] > 1) {
                        cout << " - " << n << " (x" << counts[n] << ")\n";
                    } else {
                        cout << " - " << n << "\n";
                    }
                }
            }
            
    }, false, "" };

    verbRegistry["go"] = { [this](Game& game, const Action& action) {

            if (action.argc < 2) {
                cout << respond("go_where");
            } else {
                std::string dir = action.object;
                if (currentRoom && currentRoom->exits.count(dir)) {
                    auto exit = currentRoom->exits[dir];
                    if (exit.door) {
                        if (!exit.door->isOpen) {
                            bool canOpen = true;
                            for (const auto& flag : exit.door->requiredFlags) {
                                if (!hasFlag(flag) && !playerHasItem(flag)) {
                                    canOpen = false;
                                    break;
                                }
                            }
                            for (auto& cond : exit.door->dynamicConditions) {
                                if (!cond(*this)) {
                                    canOpen = false;
                                    break;
                                }
                            }

                            if (!canOpen) {
                                cout << "\n" << (exit.door->denyMsg.empty() ? "The door is locked." : exit.door->denyMsg) << "\n";
                                return;
                            } else {
                                cout << "\nYou open the " << exit.door->name << ".\n";
                                exit.door->isOpen = true;
                            }
                        }
                    }
                    if (currentRoom) {
                        triggerEvent("exit_" + currentRoom->id);
                    }
                    currentRoom = exit.target;
                    if (currentRoom) {
                        triggerEvent("enter_" + currentRoom->id);
                        bool wasVisited = currentRoom->visited;
                        currentRoom->visited = true;
                        if (!isLit()) {
                            cout << "\nIt is pitch black. You are likely to be eaten by a grue.\n";
                        } else {
                            cout << "\n" << currentRoom->name << "\n";
                            if (verbosityMode == VERBOSE || (verbosityMode == BRIEF && !wasVisited)) {
                                cout << "\n" << currentRoom->description << "\n";
                            } else if (verbosityMode == BRIEF && wasVisited) {
                                if (!currentRoom->shortDescription.empty()) cout << "\n" << currentRoom->shortDescription << "\n";
                            }
                        }
                    }
                } else {
                    cout << respond("cant_go");
                }
            }
            
    }, false, "" };

    verbRegistry["take"] = { [this](Game& game, const Action& action) {

            if (action.argc < 2) {
                cout << "\nTake what?\n";
            } else if (currentRoom) {
                std::string itemName = action.object;
                Item* foundItem = nullptr;
                auto it = std::find_if(currentRoom->items.begin(), currentRoom->items.end(), 
                    [&](Item* item){ return item->getMatchName() == itemName; });
                if (it != currentRoom->items.end()) {
                    foundItem = *it;
                } else {
                    if (action.prep == Preposition::FROM) {
                        std::string storageName = action.object2;
                        for (auto* itm : currentRoom->items) {
                            if (Storage* st = dynamic_cast<Storage*>(itm)) {
                                if ((st->isOpen || st->isSupporter) && st->getMatchName() == storageName) {
                                    auto sit = std::find_if(st->items.begin(), st->items.end(), [&](Item* i){ return i->getMatchName() == itemName; });
                                    if (sit != st->items.end()) {
                                        foundItem = *sit;
                                        break;
                                    }
                                }
                            }
                        }
                    } else {
                        for (auto* itm : currentRoom->items) {
                            if (Storage* st = dynamic_cast<Storage*>(itm)) {
                                if (st->isOpen || st->isSupporter) {
                                    auto sit = std::find_if(st->items.begin(), st->items.end(), [&](Item* i){ return i->getMatchName() == itemName; });
                                    if (sit != st->items.end()) {
                                        foundItem = *sit;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }

                if (foundItem) {
                    if (foundItem->def && foundItem->def->onTake) {
                        foundItem->def->onTake(*this, foundItem);
                    } else {
                        if (giveItemToPlayer(foundItem->id)) {
                            cout << "\nTaken.\n";
                        } else {
                            cout << respond("cant_take");
                        }
                    }
                } else {
                    if (action.prep == Preposition::FROM) {
                        cout << "\nYou don't see that in there.\n";
                    } else {
                        cout << respond("dont_see");
                    }
                }
            }
            
    }, true, "" };

    verbRegistry["drop"] = { [this](Game& game, const Action& action) {

            if (action.argc < 2) {
                cout << "\nDrop what?\n";
            } else if (currentRoom) {
                std::string itemName = action.object;
                auto it = std::find_if(inventory.begin(), inventory.end(),
                    [&](Item* item) { return item->getMatchName() == itemName; });
                if (it != inventory.end()) {
                    currentRoom->items.push_back(*it);
                    inventory.erase(it);
                    cout << "\nDropped.\n";
                } else {
                    cout << "\nYou don't have that.\n";
                }
            }
            
    }, true, "" };

    verbRegistry["open"] = { [this](Game& game, const Action& action) {

            if (action.argc < 2) {
                cout << "\nOpen what?\n";
            } else if (currentRoom) {
                std::string target = action.object;
                Storage* foundSt = nullptr;
                for (auto* itm : currentRoom->items) {
                    if (Storage* st = dynamic_cast<Storage*>(itm)) {
                        if (st->getMatchName() == target) foundSt = st;
                    }
                }
                if (foundSt) {
                    if (foundSt->isOpen) {
                        cout << "\nIt is already open.\n";
                    } else {
                        foundSt->isOpen = true;
                        cout << "\nYou open the " << foundSt->getDisplayName() << ".\n";
                    }
                } else {
                    Door* foundDoor = nullptr;
                    for (auto& exitPair : currentRoom->exits) {
                        if (exitPair.second.door && exitPair.second.door->getMatchName() == target) {
                            foundDoor = exitPair.second.door;
                            break;
                        }
                    }
                    if (foundDoor) {
                        if (foundDoor->isOpen) {
                            cout << "\nIt is already open.\n";
                        } else {
                            bool canOpen = true;
                            for (const auto& flag : foundDoor->requiredFlags) {
                                if (!hasFlag(flag) && !playerHasItem(flag)) {
                                    canOpen = false;
                                    break;
                                }
                            }
                            for (auto& cond : foundDoor->dynamicConditions) {
                                if (!cond(*this)) {
                                    canOpen = false;
                                    break;
                                }
                            }
                            if (canOpen) {
                                foundDoor->isOpen = true;
                                cout << "\nYou open the " << foundDoor->getDisplayName() << ".\n";
                            } else {
                                if (!foundDoor->denyMsg.empty()) {
                                    cout << "\n" << foundDoor->denyMsg << "\n";
                                } else {
                                    cout << "\nIt's locked.\n";
                                }
                            }
                        }
                    } else {
                        cout << "\nYou can't open that.\n";
                    }
                }
            }
            
    }, true, "" };

    verbRegistry["close"] = { [this](Game& game, const Action& action) {

            if (action.argc < 2) {
                cout << "\nClose what?\n";
            } else if (currentRoom) {
                std::string target = action.object;
                Storage* foundSt = nullptr;
                for (auto* itm : currentRoom->items) {
                    if (Storage* st = dynamic_cast<Storage*>(itm)) {
                        if (st->getMatchName() == target) foundSt = st;
                    }
                }
                if (foundSt) {
                    if (!foundSt->isOpen) {
                        cout << "\nIt is already closed.\n";
                    } else {
                        foundSt->isOpen = false;
                        cout << "\nYou close the " << foundSt->getDisplayName() << ".\n";
                    }
                } else {
                    Door* foundDoor = nullptr;
                    for (auto& exitPair : currentRoom->exits) {
                        if (exitPair.second.door && exitPair.second.door->getMatchName() == target) {
                            foundDoor = exitPair.second.door;
                            break;
                        }
                    }
                    if (foundDoor) {
                        if (!foundDoor->isOpen) {
                            cout << "\nIt is already closed.\n";
                        } else {
                            foundDoor->isOpen = false;
                            cout << "\nYou close the " << foundDoor->getDisplayName() << ".\n";
                        }
                    } else {
                        cout << "\nYou can't close that.\n";
                    }
                }
            }
            
    }, true, "" };

    verbRegistry["put"] = { [this](Game& game, const Action& action) {

            if (action.argc < 4) {
                cout << "\nPut what where?\n";
            } else {
                switch (action.prep) {
                    case Preposition::IN: {
                        std::string itemName = action.object;
                        std::string targetName = action.object2;
                        auto it = std::find_if(inventory.begin(), inventory.end(),
                            [&](Item* item) { return item->getMatchName() == itemName; });
                        if (it != inventory.end()) {
                            Storage* foundSt = nullptr;
                            for (auto* itm : currentRoom->items) {
                                if (Storage* st = dynamic_cast<Storage*>(itm)) {
                                    if (st->getMatchName() == targetName) foundSt = st;
                                }
                            }
                            if (foundSt) {
                                if (foundSt->isOpen) {
                                    foundSt->items.push_back(*it);
                                    inventory.erase(it);
                                    cout << "\nYou put the " << itemName << " in the " << targetName << ".\n";
                                } else {
                                    cout << "\nThe " << targetName << " is closed.\n";
                                }
                            } else {
                                cout << "\nYou don't see anywhere to put that in.\n";
                            }
                        } else {
                            cout << "\nYou don't have that.\n";
                        }
                        break;
                    }
                    case Preposition::ON: {
                        std::string itemName = action.object;
                        std::string targetName = action.object2;
                        auto it = std::find_if(inventory.begin(), inventory.end(),
                            [&](Item* item) { return item->getMatchName() == itemName; });
                        if (it != inventory.end()) {
                            Storage* foundSt = nullptr;
                            for (auto* itm : currentRoom->items) {
                                if (Storage* st = dynamic_cast<Storage*>(itm)) {
                                    if (st->getMatchName() == targetName && st->isSupporter) foundSt = st;
                                }
                            }
                            if (foundSt) {
                                foundSt->items.push_back(*it);
                                inventory.erase(it);
                                cout << "\nYou put the " << itemName << " on the " << targetName << ".\n";
                            } else {
                                cout << "\nYou don't see anywhere to put that on.\n";
                            }
                        } else {
                            cout << "\nYou don't have that.\n";
                        }
                        break;
                    }
                    default:
                        cout << "\nI don't understand how to put that there.\n";
                        break;
                }
            }
            
    }, true, "" };

    verbRegistry["save"] = { [this](Game& game, const Action& action) {

            saveGame(action.object.empty() ? "save" : action.object);
            
    }, false, "" };

    verbRegistry["load"] = { [this](Game& game, const Action& action) {

            loadGame(action.object.empty() ? "save" : action.object);
            
    }, false, "" };

    verbRegistry["use"] = { [this](Game& game, const Action& action) {
 {
            if (action.argc < 2) {
                cout << "\nUse what?\n";
            } else if (currentRoom) {
                std::string itemName = action.object;
                std::string targetName = action.object2;
                Item* foundItem = nullptr;
                auto it = std::find_if(currentRoom->items.begin(), currentRoom->items.end(), 
                    [&](Item* item){ return item->getMatchName() == itemName; });
                if (it != currentRoom->items.end()) foundItem = *it;
                
                if (!foundItem) {
                    for (auto* itm : currentRoom->items) {
                        if (Storage* st = dynamic_cast<Storage*>(itm)) {
                            if (st->isOpen) {
                                auto sit = std::find_if(st->items.begin(), st->items.end(), [&](Item* i){ return i->getMatchName() == itemName; });
                                if (sit != st->items.end()) foundItem = *sit;
                            }
                        }
                    }
                }
                if (!foundItem) {
                    auto invIt = std::find_if(inventory.begin(), inventory.end(), [&](Item* item){ return item->getMatchName() == itemName; });
                    if (invIt != inventory.end()) foundItem = *invIt;
                }

                if (foundItem && foundItem->def && foundItem->def->onUse) {
                    foundItem->def->onUse(*this, targetName);
                } else {
                    cout << "\nYou can't use that here.\n";
                }
            }
            
        }
    }, true, "" };

    verbRegistry["play"] = { [this](Game& game, const Action& action) {
 {
            if (action.argc < 2) {
                cout << "\nPlay what?\n";
            } else {
                std::string mgName = action.object;
                MiniGame* foundMg = nullptr;
                for (auto& pair : miniGames) {
                    if (pair.second->getMatchName() == mgName) {
                        foundMg = pair.second.get();
                        break;
                    }
                }
                if (foundMg) {
                    foundMg->run(*this);
                } else {
                    cout << "\nYou can't play that here.\n";
                }
            }
            
        }
    }, true, "" };

    verbRegistry["talk"] = { [this](Game& game, const Action& action) {
 {
            if (action.argc < 2) {
                cout << "\nTalk to whom?\n";
            } else if (currentRoom) {
                std::string npcName = action.object;
                if (action.prep == Preposition::TO && !action.object2.empty()) {
                    npcName = action.object2;
                }
                NPC* foundNpc = nullptr;
                for (auto* npc : currentRoom->npcs) {
                    if (npc->getMatchName() == npcName) {
                        foundNpc = npc;
                        break;
                    }
                }
                if (foundNpc) {
                    if (!foundNpc->dialogue.empty()) {
                        for (const auto& line : foundNpc->dialogue) {
                            cout << line << "\n";
                        }
                        setFlag("spoken_to_" + foundNpc->id, true);
                    } else {
                        cout << "\n" << foundNpc->getDisplayName() << " has nothing to say.\n";
                        setFlag("spoken_to_" + foundNpc->id, true);
                    }
                } else {
                    cout << "\nThere's no one by that name here.\n";
                }
            }
            
        }
    }, true, "" };

    verbRegistry["sleep"] = { [this](Game& game, const Action& action) {

            cout << "\nYou lie down and go to sleep.\n";
            
    }, false, "" };

    verbRegistry["wake"] = { [this](Game& game, const Action& action) {

            cout << "\nYou wake up feeling refreshed.\n";
            
    }, false, "" };

    verbRegistry["help"] = { [this](Game& game, const Action& action) {

            cout << "\nAvailable verbs: look, go, take, drop, open, close, use, play, talk, sleep, wake, put, inventory, save, load, quit, help.\n"
                      << "Synonyms: x, examine, read, leave, enter, get, grab, walk, move, etc.\n";
            
    }, false, "" };

    verbRegistry["verbose"] = { [this](Game& game, const Action& action) {

            verbosityMode = VERBOSE;
            cout << "\nVerbose mode on.\n";
            
    }, false, "" };

    verbRegistry["brief"] = { [this](Game& game, const Action& action) {

            verbosityMode = BRIEF;
            cout << "\nBrief mode on.\n";
            
    }, false, "" };

    verbRegistry["superbrief"] = { [this](Game& game, const Action& action) {

            verbosityMode = SUPERBRIEF;
            cout << "\nSuperbrief mode on.\n";
            
    }, false, "" };

    verbRegistry["score"] = { [this](Game& game, const Action& action) {

            cout << "\nYour score is " << score << " (" << moves << " moves).\n";
            
    }, false, "" };
}


void Game::executeAction(const Action& action) {
    bool dark = !isLit();
    std::string canonical = action.verb;
    if (verbSynonyms.count(canonical)) {
        canonical = verbSynonyms[canonical];
    }

    if (canonical.empty() || canonical == "unknown") {
        if (!dark || darknessAllowedVerbs.count(action.rawVerb)) {
            for (auto* item : inventory) {
                if (item->def && item->def->customVerbs.count(action.rawVerb)) {
                    item->def->customVerbs.at(action.rawVerb)(*this, action.rawArgs);
                    return;
                }
            }
            if (currentRoom) {
                for (auto* item : currentRoom->items) {
                    if (item->def && item->def->customVerbs.count(action.rawVerb)) {
                        item->def->customVerbs.at(action.rawVerb)(*this, action.rawArgs);
                        return;
                    }
                }
            }
        }
        cout << "\nI don't understand that command.\n";
        return;
    }

    if (dark && canonical != "quit" && canonical != "inventory" && canonical != "go" && !darknessAllowedVerbs.count(action.rawVerb)) {
        cout << "\nIt is pitch black. You are likely to be eaten by a grue.\n";
        return;
    }

    if (!action.object.empty()) {
        std::vector<GameObject*> scope;
        for (auto* item : inventory) scope.push_back(item);
        if (currentRoom) {
            for (auto* item : currentRoom->items) scope.push_back(item);
            for (auto& exit : currentRoom->exits) {
                if (exit.second.door) scope.push_back(exit.second.door);
            }
            for (auto* npc : currentRoom->npcs) {
                scope.push_back(npc);
            }
        }
        
        GameObject* target = nullptr;
        std::vector<GameObject*> matches;
        for (auto* obj : scope) {
            if (obj->getMatchName() == action.object || obj->id == action.object || (obj->getDisplayName() == action.object)) {
                matches.push_back(obj);
            }
            if (auto* st = dynamic_cast<Storage*>(obj)) {
                if (st->isOpen || st->isSupporter) {
                    for (auto* sitm : st->items) {
                        if (sitm->getMatchName() == action.object || sitm->id == action.object) {
                            matches.push_back(sitm);
                        }
                    }
                }
            }
        }
        
        if (matches.size() == 1) {
            target = matches[0];
            if (target->verbs.count(canonical)) {
                target->verbs[canonical](*this, target, action);
            } else if (verbRegistry.count(canonical) && verbRegistry[canonical].defaultFn) {
                verbRegistry[canonical].defaultFn(*this, action);
            } else {
                cout << "\nYou can't do that.\n";
            }
        } else if (matches.size() > 1) {
            pendingDisambiguationTarget = action.object;
            pendingDisambiguationMatches.clear();
            for (auto* m : matches) pendingDisambiguationMatches.push_back(m->getDisplayName());
            pendingDisambiguationAction = action;
            disambiguationSlot = "object";
            cout << "\nWhich one do you mean?\n";
        } else {
            if (verbRegistry.count(canonical) && verbRegistry[canonical].defaultFn) {
                verbRegistry[canonical].defaultFn(*this, action);
            } else {
                cout << "\nThere's nothing to " << canonical << " here.\n";
            }
        }
    } else {
        if (verbRegistry.count(canonical) && verbRegistry[canonical].needsArg) {
            cout << "\nWhat do you want to " << canonical << "?\n";
        } else if (verbRegistry.count(canonical) && verbRegistry[canonical].defaultFn) {
            verbRegistry[canonical].defaultFn(*this, action);
        } else {
            cout << "\nI don't know how to do that yet.\n";
        }
    }
    if (dark && isLit()) {
        processCommand("look");
    }
}


void Game::processCommand(const std::string& inputStr) {
    if (inDialogue) {
        try {
            int choice = std::stoi(inputStr);
            if (choice > 0 && choice <= (int)currentDialogueOptions.size()) {
                startDialogueTree(currentDialogueOptions[choice - 1].targetNode);
            } else {
                cout << "\nInvalid choice.\n";
            }
        } catch (...) {
            cout << "\nPlease enter a number.\n";
        }
        return;
    }

    std::string current_cmd = "";
    std::vector<std::string> subcommands;
    for (char c : inputStr) {
        if (c == ';' || c == '.') {
            if (!current_cmd.empty()) {
                subcommands.push_back(current_cmd);
                current_cmd = "";
            }
        } else {
            current_cmd += c;
        }
    }
    if (!current_cmd.empty()) subcommands.push_back(current_cmd);

    for (std::string cmd : subcommands) {
        if (!isRunning || isFinished()) break;

        std::string trimmed_cmd = cmd;
        trimmed_cmd.erase(0, trimmed_cmd.find_first_not_of(" \t\r\n"));

        // Clear oops state if the new command is not oops or again
        if (trimmed_cmd.find("oops ") != 0 && trimmed_cmd.find("OOPS ") != 0 && 
            trimmed_cmd != "again" && trimmed_cmd != "g" && 
            trimmed_cmd != "AGAIN" && trimmed_cmd != "G") {
            lastFailedCommand = "";
            firstUnknownWordIndex = -1;
        }

        // F-8 OOPS
        if (trimmed_cmd.find("oops ") == 0 || trimmed_cmd.find("OOPS ") == 0) {
            if (firstUnknownWordIndex >= 0 && !lastFailedCommand.empty()) {
                std::string correction = trimmed_cmd.substr(5);
                std::istringstream iss(lastFailedCommand);
                std::vector<std::string> old_tokens;
                std::string t;
                while (iss >> t) old_tokens.push_back(t);
                
                if (firstUnknownWordIndex < (int)old_tokens.size()) {
                    old_tokens[firstUnknownWordIndex] = correction;
                }
                cmd = "";
                for (size_t i = 0; i < old_tokens.size(); ++i) {
                    if (i > 0) cmd += " ";
                    cmd += old_tokens[i];
                }
                trimmed_cmd = cmd;
            } else {
                cout << "\nThere is nothing to correct.\n";
                continue;
            }
        }

        // F-5 AGAIN
        if (trimmed_cmd == "again" || trimmed_cmd == "g" || trimmed_cmd == "AGAIN" || trimmed_cmd == "G") {
            if (!lastCommand.empty()) {
                cmd = lastCommand;
            } else {
                cout << "\nNothing to repeat.\n";
                continue;
            }
        }

        Action act = parseInput(cmd);
        if (act.argc == 1) {
            std::string raw = act.rawVerb;
            if (raw == "n" || raw == "s" || raw == "e" || raw == "w" || 
                raw == "north" || raw == "south" || raw == "east" || raw == "west" || 
                raw == "up" || raw == "down" || raw == "in" || raw == "out" || 
                raw == "ne" || raw == "nw" || raw == "se" || raw == "sw" || raw == "inside" || raw == "outside") {
                act = parseInput("go " + raw);
            }
        }

        if (act.argc > 0) {
            if (act.verb == "unknown") {
                lastFailedCommand = cmd;
                firstUnknownWordIndex = 0; // Simple approximation for OOPS
                executeAction(act); // Will print "I don't understand that command."
                break; // Stop chain on failed parse
            }

            lastCommand = cmd;
            // State is intentionally NOT cleared here, so it persists for the next input loop!

            if (!pendingDisambiguationTarget.empty()) {
                // If we are waiting for disambiguation, check if the command matches one of the options
                bool found = false;
                for (const auto& opt : pendingDisambiguationMatches) {
                    if (opt.find(act.rawVerb) != std::string::npos || act.rawVerb.find(opt) != std::string::npos) {
                        if (disambiguationSlot == "object2") pendingDisambiguationAction.object2 = opt;
                        else pendingDisambiguationAction.object = opt;
                        act = pendingDisambiguationAction;
                        found = true;
                        break;
                    }
                }
                pendingDisambiguationTarget = "";
                pendingDisambiguationMatches.clear();
                if (!found) {
                    cout << "\nDisambiguation cancelled.\n";
                    continue;
                }
            } else if (act.verb != "go" && act.verb != "quit" && act.verb != "help" && act.verb != "save" && act.verb != "load" && act.verb != "inventory" && act.verb != "score" && act.verb != "unknown") {
                // Try resolving act.object
                if (!act.object.empty()) {
                    std::vector<std::string> matches;
                    if (act.verb == "talk") {
                        if (currentRoom) {
                            for (auto* npc : currentRoom->npcs) {
                                if (npc->getMatchName().find(act.object) != std::string::npos) matches.push_back(npc->getMatchName());
                            }
                        }
                    } else {
                        for (auto* itm : inventory) {
                            if (itm->getMatchName().find(act.object) != std::string::npos) matches.push_back(itm->getMatchName());
                        }
                        if (currentRoom) {
                            for (auto* itm : currentRoom->items) {
                                if (itm->getMatchName().find(act.object) != std::string::npos) matches.push_back(itm->getMatchName());
                                if (Storage* st = dynamic_cast<Storage*>(itm)) {
                                    if (st->isOpen) {
                                        for (auto* sitm : st->items) {
                                            if (sitm->getMatchName().find(act.object) != std::string::npos) matches.push_back(sitm->getMatchName());
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if (matches.size() > 1) {
                        std::vector<std::string> exacts;
                        for (auto& m : matches) if (m == act.object) exacts.push_back(m);
                        if (exacts.size() == 1) {
                            act.object = exacts[0];
                        } else {
                            cout << "\nWhich " << act.object << " do you mean, " << matches[0];
                            for (size_t i = 1; i < matches.size(); ++i) {
                                if (i == matches.size() - 1) cout << " or " << matches[i];
                                else cout << ", " << matches[i];
                            }
                            cout << "?\n";
                            pendingDisambiguationTarget = act.object;
                            pendingDisambiguationMatches = matches;
                            pendingDisambiguationAction = act;
                            disambiguationSlot = "object";
                            break; // Stop chaining
                        }
                    } else if (matches.size() == 1) {
                        act.object = matches[0];
                    } else {
                        lastFailedCommand = cmd;
                        firstUnknownWordIndex = 1;
                    }
                }

                // Resolve act.object2
                if (!act.object2.empty()) {
                    std::vector<std::string> matches;
                    for (auto* itm : inventory) {
                        if (itm->getMatchName().find(act.object2) != std::string::npos) matches.push_back(itm->getMatchName());
                    }
                    if (currentRoom) {
                        for (auto* itm : currentRoom->items) {
                            if (itm->getMatchName().find(act.object2) != std::string::npos) matches.push_back(itm->getMatchName());
                            if (Storage* st = dynamic_cast<Storage*>(itm)) {
                                if (st->isOpen) {
                                    for (auto* sitm : st->items) {
                                        if (sitm->getMatchName().find(act.object2) != std::string::npos) matches.push_back(sitm->getMatchName());
                                    }
                                }
                            }
                        }
                    }
                    if (matches.size() > 1) {
                        std::vector<std::string> exacts;
                        for (auto& m : matches) if (m == act.object2) exacts.push_back(m);
                        if (exacts.size() == 1) {
                            act.object2 = exacts[0];
                        } else {
                            cout << "\nWhich " << act.object2 << " do you mean, " << matches[0];
                            for (size_t i = 1; i < matches.size(); ++i) {
                                if (i == matches.size() - 1) cout << " or " << matches[i];
                                else cout << ", " << matches[i];
                            }
                            cout << "?\n";
                            pendingDisambiguationTarget = act.object2;
                            pendingDisambiguationMatches = matches;
                            pendingDisambiguationAction = act;
                            disambiguationSlot = "object2";
                            break; // Stop chaining
                        }
                    } else if (matches.size() == 1) {
                        act.object2 = matches[0];
                    } else {
                        lastFailedCommand = cmd;
                        firstUnknownWordIndex = act.object.empty() ? 1 : 2;
                    }
                }
            }

            if (act.verb != "unknown") {
                moves++;
            }
            executeAction(act);
            
            // F-4 Pronoun tracking
            if (!act.object.empty() && act.verb != "look" && act.verb != "help") {
                if (act.verb == "talk") lastNPC = act.object;
                else lastObject = act.object;
            }

            // Tick timers if it's an action verb
            if (act.verb != "look" && act.verb != "inventory" && 
                act.verb != "help" && act.verb != "save" && 
                act.verb != "load" && act.verb != "unknown") {
                
                std::vector<std::string> toFire;
                for (auto& pair : timers) {
                    if (!pair.second.paused) {
                        pair.second.remaining--;
                        if (pair.second.remaining <= 0) {
                            toFire.push_back(pair.first);
                        }
                    }
                }
                for (const auto& tid : toFire) {
                    cancelTimer(tid);
                    triggerEvent("timer_" + tid);
                }
            }
        }
    }
}

void Game::enterRealTimeMode() {
    if (io) io->enterRawMode();
}

void Game::exitRealTimeMode() {
    if (io) io->exitRawMode();
}

void Game::save(const std::string& filename) {
    std::ofstream out(filename);
    if (!out) {
        cout << "\nError: Could not open file for saving.\n";
        return;
    }

    auto writeLine = [&out](const std::string& s) { out << s << "\n"; };
    auto writeFlags = [&out, &writeLine](const std::map<std::string, bool>& flags) {
        out << flags.size() << "\n";
        for (const auto& f : flags) {
            writeLine(f.first);
            out << f.second << "\n";
        }
    };

    writeLine("SAVE_V6");
    writeLine(currentRoom ? currentRoom->id : "");

    out << globalFlags.size() << "\n";
    for (const auto& kv : globalFlags) {
        writeLine(kv.first);
        out << kv.second << "\n";
    }

    out << inventory.size() << "\n";
    for (auto* item : inventory) {
        writeLine(item->id);
    }

    out << items.size() << "\n";
    for (const auto& pair : items) {
        writeLine(pair.first);
        out << pair.second->count << "\n";
        out << pair.second->isLightSource << "\n" << pair.second->isLit << "\n"; // V6
        writeFlags(pair.second->flags);
        writeLine(pair.second->def ? pair.second->def->name : "");
    }

    std::vector<Room*> roomList;
    std::vector<Door*> doorList;
    std::vector<Storage*> storageList;
    std::vector<NPC*> npcList;
    for (auto& pair : rooms) roomList.push_back(pair.second.get());
    for (auto& pair : doors) doorList.push_back(pair.second.get());
    for (auto& pair : storages) storageList.push_back(pair.second.get());
    for (auto& pair : npcs) npcList.push_back(pair.second.get());

    out << roomList.size() << "\n";
    for (auto* room : roomList) {
        writeLine(room->id);
        writeFlags(room->flags);
        out << room->visited << "\n";
        out << room->isDark << "\n"; // V6
        out << room->items.size() << "\n";
        for (auto* item : room->items) {
            writeLine(item->id);
        }
    }

    out << doorList.size() << "\n";
    for (auto* door : doorList) {
        writeLine(door->id);
        writeFlags(door->flags);
        out << door->isOpen << "\n";
    }

    out << storageList.size() << "\n";
    for (auto* st : storageList) {
        writeLine(st->id);
        writeFlags(st->flags);
        out << st->isOpen << "\n" << st->isSupporter << "\n"; // V6
        out << st->items.size() << "\n";
        for (auto* item : st->items) writeLine(item->id);
    }

    out << npcList.size() << "\n";
    for (auto* npc : npcList) {
        writeLine(npc->id);
        writeFlags(npc->flags);
        out << npc->isAlive << "\n" << npc->health << "\n" << npc->cutsceneFlag << "\n";
        out << npc->inventory.size() << "\n";
        for (auto* item : npc->inventory) writeLine(item->id);
    }

    out << timers.size() << "\n";
    for (const auto& t : timers) {
        writeLine(t.first);
        out << t.second.remaining << "\n" << t.second.paused << "\n";
    }

    out << (int)verbosityMode << "\n" << score << "\n" << maxScore << "\n" << moves << "\n";

    if (out.fail()) {
        cout << "\nError: Failed to write save data to file.\n";
    } else {
        cout << "\nGame saved.\n";
    }
}

void Game::load(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) {
        cout << "\nError: Could not open file for loading.\n";
        return;
    }

    auto readLine = [&in]() {
        std::string s;
        if (!std::getline(in, s)) throw std::runtime_error("EOF");
        return s;
    };

    auto readFlags = [&in, &readLine](std::map<std::string, bool>& flags) {
        size_t fcount;
        if (!(in >> fcount)) throw std::runtime_error("EOF");
        readLine();
        for (size_t j = 0; j < fcount; ++j) {
            std::string fk = readLine();
            bool fv; if (!(in >> fv)) throw std::runtime_error("EOF");
            readLine();
            flags[fk] = fv;
        }
    };

    try {
        std::string header = readLine();
        if (header != "SAVE_V1" && header != "SAVE_V2" && header != "SAVE_V3" && header != "SAVE_V4" && header != "SAVE_V5" && header != "SAVE_V6") {
            cout << "\nError: Invalid save file format.\n";
            return;
        }

        std::string t_curRoomName = readLine();

        size_t flagCount;
        if (!(in >> flagCount)) throw std::runtime_error("EOF");
        readLine();
        std::map<std::string, bool> t_globalFlags;
        for (size_t i = 0; i < flagCount; ++i) {
            std::string key = readLine();
            bool val; if (!(in >> val)) throw std::runtime_error("EOF");
            readLine();
            t_globalFlags[key] = val;
        }

        size_t invCount;
        if (!(in >> invCount)) throw std::runtime_error("EOF");
        readLine();
        std::vector<std::string> t_inventory;
        for (size_t i = 0; i < invCount; ++i) t_inventory.push_back(readLine());

        struct TempItem { int count = 1; bool isLightSource = false; bool isLit = false; std::map<std::string, bool> flags; std::string defName; };
        std::map<std::string, TempItem> t_items;
        size_t allItemsCount;
        if (!(in >> allItemsCount)) throw std::runtime_error("EOF");
        readLine();
        for (size_t i = 0; i < allItemsCount; ++i) {
            std::string id = readLine();
            TempItem ti;
            if (header == "SAVE_V2" || header == "SAVE_V3" || header == "SAVE_V4" || header == "SAVE_V5" || header == "SAVE_V6") {
                if (!(in >> ti.count)) throw std::runtime_error("EOF");
                if (header == "SAVE_V6") {
                    if (!(in >> ti.isLightSource >> ti.isLit)) throw std::runtime_error("EOF");
                }
                readLine();
                readFlags(ti.flags);
            }
            ti.defName = readLine();
            t_items[id] = ti;
        }

        struct TempRoom { std::map<std::string, bool> flags; bool visited = false; bool isDark = false; std::vector<std::string> items; };
        std::map<std::string, TempRoom> t_rooms;
        size_t roomCount;
        if (!(in >> roomCount)) throw std::runtime_error("EOF");
        readLine();
        for (size_t i = 0; i < roomCount; ++i) {
            std::string rname = readLine();
            TempRoom tr;
            if (header == "SAVE_V3" || header == "SAVE_V4" || header == "SAVE_V5" || header == "SAVE_V6") readFlags(tr.flags);
            if (header == "SAVE_V5" || header == "SAVE_V6") {
                if (!(in >> tr.visited)) throw std::runtime_error("EOF");
                if (header == "SAVE_V6") {
                    if (!(in >> tr.isDark)) throw std::runtime_error("EOF");
                }
                readLine();
            }
            size_t ritemCount;
            if (!(in >> ritemCount)) throw std::runtime_error("EOF");
            readLine();
            for (size_t j = 0; j < ritemCount; ++j) tr.items.push_back(readLine());
            t_rooms[rname] = tr;
        }

        struct TempDoor { std::map<std::string, bool> flags; bool isOpen = false; };
        std::map<std::string, TempDoor> t_doors;
        size_t doorCount;
        if (!(in >> doorCount)) throw std::runtime_error("EOF");
        readLine();
        for (size_t i = 0; i < doorCount; ++i) {
            std::string dname = readLine();
            TempDoor td;
            if (header == "SAVE_V3" || header == "SAVE_V4") readFlags(td.flags);
            if (!(in >> td.isOpen)) throw std::runtime_error("EOF");
            readLine();
            t_doors[dname] = td;
        }

        struct TempStorage { std::map<std::string, bool> flags; bool isOpen = false; bool isSupporter = false; std::vector<std::string> items; };
        std::map<std::string, TempStorage> t_storages;
        struct TempNPC { std::map<std::string, bool> flags; bool isAlive = true; int health = 100; bool cutsceneFlag = false; std::vector<std::string> inventory; };
        std::map<std::string, TempNPC> t_npcs;
        std::map<std::string, TimerInfo> t_timers;

        if (header != "SAVE_V1") {
            size_t storageCount;
            if (!(in >> storageCount)) throw std::runtime_error("EOF");
            readLine();
            for (size_t i=0; i<storageCount; ++i) {
                std::string stname = readLine();
                TempStorage ts;
                if (header == "SAVE_V3" || header == "SAVE_V4" || header == "SAVE_V5" || header == "SAVE_V6") readFlags(ts.flags);
                if (header == "SAVE_V6") {
                    size_t stitemcount;
                    if (!(in >> ts.isOpen >> ts.isSupporter >> stitemcount)) throw std::runtime_error("EOF");
                    readLine();
                    for (size_t j=0; j<stitemcount; ++j) ts.items.push_back(readLine());
                } else {
                    size_t stitemcount;
                    if (!(in >> ts.isOpen >> stitemcount)) throw std::runtime_error("EOF");
                    readLine();
                    for (size_t j=0; j<stitemcount; ++j) ts.items.push_back(readLine());
                }
                t_storages[stname] = ts;
            }

            size_t npcCount;
            if (!(in >> npcCount)) throw std::runtime_error("EOF");
            readLine();
            for (size_t i=0; i<npcCount; ++i) {
                std::string nname = readLine();
                TempNPC tn;
                if (header == "SAVE_V3" || header == "SAVE_V4" || header == "SAVE_V5" || header == "SAVE_V6") readFlags(tn.flags);
                if (!(in >> tn.isAlive >> tn.health >> tn.cutsceneFlag)) throw std::runtime_error("EOF");
                if (header == "SAVE_V4" || header == "SAVE_V5" || header == "SAVE_V6") {
                    size_t invCount;
                    if (!(in >> invCount)) throw std::runtime_error("EOF");
                    readLine();
                    for (size_t j=0; j<invCount; ++j) tn.inventory.push_back(readLine());
                } else {
                    readLine();
                }
                t_npcs[nname] = tn;
            }

            size_t timerCount;
            if (!(in >> timerCount)) throw std::runtime_error("EOF");
            readLine();
            for (size_t i=0; i<timerCount; ++i) {
                std::string tid = readLine();
                int rem; bool ps;
                if (!(in >> rem >> ps)) throw std::runtime_error("EOF");
                readLine();
                t_timers[tid] = {rem, tid, ps};
            }
        }

        auto findObj = [&](const std::string& name) -> GameObject* {
            if (rooms.count(name)) return rooms[name].get();
            if (items.count(name)) return items[name].get();
            if (doors.count(name)) return doors[name].get();
            if (npcs.count(name)) return npcs[name].get();
            if (storages.count(name)) return storages[name].get();
            return nullptr;
        };

        if (!t_curRoomName.empty()) {
            if (auto r = dynamic_cast<Room*>(findObj(t_curRoomName))) currentRoom = r;
        }

        globalFlags = t_globalFlags;

        inventory.clear();
        for (const auto& iname : t_inventory) {
            if (auto itm = dynamic_cast<Item*>(findObj(iname))) inventory.push_back(itm);
        }

        for (auto& pair : items) pair.second->flags.clear();
        for (auto& pair : rooms) pair.second->flags.clear();
        for (auto& pair : doors) pair.second->flags.clear();
        for (auto& pair : storages) pair.second->flags.clear();
        for (auto& pair : npcs) pair.second->flags.clear();

        for (const auto& pair : t_items) {
            std::string id = pair.first;
            if (items.count(id)) {
                items[id]->count = pair.second.count;
                items[id]->isLightSource = pair.second.isLightSource;
                items[id]->isLit = pair.second.isLit;
                items[id]->flags = pair.second.flags;
                for (const auto& d : itemDefs) {
                    if (d.second->name == pair.second.defName) {
                        items[id]->def = d.second.get();
                        break;
                    }
                }
            }
        }

        for (const auto& pair : t_rooms) {
            if (auto r = dynamic_cast<Room*>(findObj(pair.first))) {
                r->flags = pair.second.flags;
                r->visited = pair.second.visited;
                r->isDark = pair.second.isDark;
                r->items.clear();
                for (const auto& iname : pair.second.items) {
                    if (auto itm = dynamic_cast<Item*>(findObj(iname))) r->items.push_back(itm);
                }
            }
        }

        for (const auto& pair : t_doors) {
            if (auto d = dynamic_cast<Door*>(findObj(pair.first))) {
                d->flags = pair.second.flags;
                d->isOpen = pair.second.isOpen;
            }
        }

        for (const auto& pair : t_storages) {
            if (auto st = dynamic_cast<Storage*>(findObj(pair.first))) {
                st->flags = pair.second.flags;
                st->isOpen = pair.second.isOpen;
                st->isSupporter = pair.second.isSupporter;
                st->items.clear();
                for (const auto& iname : pair.second.items) {
                    if (auto itm = dynamic_cast<Item*>(findObj(iname))) st->items.push_back(itm);
                }
            }
        }

        for (const auto& pair : t_npcs) {
            if (auto n = dynamic_cast<NPC*>(findObj(pair.first))) {
                n->flags = pair.second.flags;
                n->isAlive = pair.second.isAlive;
                n->health = pair.second.health;
                n->cutsceneFlag = pair.second.cutsceneFlag;
                n->inventory.clear();
                for (const auto& invId : pair.second.inventory) {
                    if (auto itm = dynamic_cast<Item*>(findObj(invId))) {
                        n->inventory.push_back(itm);
                    }
                }
            }
        }

        timers = t_timers;

        if (header == "SAVE_V5" || header == "SAVE_V6") {
            int vmode;
            if (in >> vmode >> score >> maxScore >> moves) {
                verbosityMode = (Verbosity)vmode;
            }
        }

        cout << "\nGame loaded.\n";
        processCommand("look");

    } catch (const std::exception& e) {
        cout << "\nError: Save file was corrupted or incomplete.\n";
    }
}

void Game::setFlag(const std::string& flagName, bool value) { globalFlags[flagName] = value; }
bool Game::getFlag(const std::string& flagName) const { return hasFlag(flagName); }

bool Game::giveItemToPlayer(const std::string& itemId) {
    Item* itm = nullptr;
    if (items.count(itemId)) itm = items[itemId].get();
    else if (storages.count(itemId)) itm = storages[itemId].get();
    if (itm) {
        if (!itm->unique) {
            for (auto* invItm : inventory) {
                if (invItm->def && itm->def && invItm->def->name == itm->def->name) {
                    invItm->count += itm->count;
                    for (auto& r : rooms) {
                        auto& rit = r.second->items;
                        rit.erase(std::remove(rit.begin(), rit.end(), itm), rit.end());
                    }
                    for (auto& s : storages) {
                        auto& sit = s.second->items;
                        sit.erase(std::remove(sit.begin(), sit.end(), itm), sit.end());
                    }
                    return true;
                }
            }
        }
        if (std::find(inventory.begin(), inventory.end(), itm) == inventory.end()) inventory.push_back(itm);
        for (auto& r : rooms) {
            auto& rit = r.second->items;
            rit.erase(std::remove(rit.begin(), rit.end(), itm), rit.end());
        }
        for (auto& s : storages) {
            auto& sit = s.second->items;
            sit.erase(std::remove(sit.begin(), sit.end(), itm), sit.end());
        }
        return true;
    }
    return false;
}

bool Game::removeInvItem(const std::string& itemId) {
    auto it = std::find_if(inventory.begin(), inventory.end(), [&](Item* i) { return i->id == itemId; });
    if (it != inventory.end()) { inventory.erase(it); return true; }
    return false;
}
bool Game::playerHasItem(const std::string& itemId) const {
    return std::any_of(inventory.begin(), inventory.end(), [&](Item* i) { return i->id == itemId; });
}

bool Game::spawnItem(const std::string& itemId, const std::string& roomId) {
    if (rooms.count(roomId)) {
        Item* itm = nullptr;
        if (items.count(itemId)) itm = items[itemId].get();
        else if (storages.count(itemId)) itm = storages[itemId].get();
        if (itm) {
            removeInvItem(itemId);
            for (auto& r : rooms) {
                auto& rit = r.second->items;
                rit.erase(std::remove(rit.begin(), rit.end(), itm), rit.end());
            }
            for (auto& s : storages) {
                auto& sit = s.second->items;
                sit.erase(std::remove(sit.begin(), sit.end(), itm), sit.end());
            }
            rooms[roomId]->items.push_back(itm);
            return true;
        }
    }
    return false;
}
bool Game::movePlayerTo(const std::string& roomId) {
    if (rooms.count(roomId)) {
        if (currentRoom) triggerEvent("exit_" + currentRoom->id);
        currentRoom = rooms[roomId].get();
        if (currentRoom) triggerEvent("enter_" + currentRoom->id);
        return true;
    }
    return false;
}
bool Game::moveItem(const std::string& itemId, const std::string& toRoomId) {
    if (!rooms.count(toRoomId)) return false;
    return spawnItem(itemId, toRoomId);
}

void Game::createTimer(int seconds, const std::string& callbackId) { timers[callbackId] = {seconds, callbackId, false}; }
void Game::cancelTimer(const std::string& timerId) { timers.erase(timerId); }
void Game::pauseTimer(const std::string& timerId) { if (timers.count(timerId)) timers[timerId].paused = true; }
void Game::resumeTimer(const std::string& timerId) { if (timers.count(timerId)) timers[timerId].paused = false; }

void Game::registerEventHook(const std::string& eventName, std::function<void()> hook) { eventHooks[eventName].push_back(hook); }
void Game::triggerEvent(const std::string& eventName) {
    if (eventHooks.count(eventName)) { auto hooks = eventHooks[eventName]; for (auto& h : hooks) h(); }
}
void Game::clearEventHooks(const std::string& eventName) { eventHooks.erase(eventName); }

void Game::startDialogueTree(const std::string& dialogueId) {
    activeDialogueNode = dialogueId;
    currentDialogueOptions.clear();
    inDialogue = true;
    triggerEvent("dialogue_" + dialogueId);
    if (inDialogue) {
        if (!currentDialogueOptions.empty()) {
            for (size_t i = 0; i < currentDialogueOptions.size(); ++i) cout << (i + 1) << ". " << currentDialogueOptions[i].text << "\n";
        } else endDialogue();
    }
}
void Game::addDialogueOption(const std::string& text, const std::string& targetNode) { currentDialogueOptions.push_back({text, targetNode}); }
void Game::endDialogue() { inDialogue = false; currentDialogueOptions.clear(); activeDialogueNode = ""; }
bool Game::hasSpokenTo(const std::string& npcId) { return getFlag("spoken_to_" + npcId); }

void Game::printColored(const std::string& text, int color) { (void)color; cout << text; }
void Game::printBold(const std::string& text) { cout << "\033[1m" << text << "\033[0m"; }
void Game::printItalic(const std::string& text) { cout << "\033[3m" << text << "\033[0m"; }
void Game::clearScreen() { this->io->print(" [2J [H"); }
void Game::typewriterEffect(const std::string& text, int msPerChar) { (void)msPerChar; for (char c : text) cout << c; }

void Game::playAudio(const std::string& filepath) { cout << "\n[Audio playing: " << filepath << "]\n"; }
void Game::stopAudio() { cout << "\n[Audio stopped]\n"; }
void Game::setAudioVolume(float volume) { cout << "\n[Audio volume set to " << volume << "]\n"; }
void Game::loopAudio(const std::string& filepath) { cout << "\n[Audio looping: " << filepath << "]\n"; }
void Game::shakeScreen(int intensity, int duration) { cout << "\n[Screen shaking with intensity " << intensity << " for " << duration << "ms]\n"; }
void Game::flashScreen(int color) { cout << "\n[Screen flashing color code " << color << "]\n"; }

void Game::saveGame(const std::string& slotName) { std::string fname = slotName; if (fname.find(".sav") == std::string::npos) fname += ".sav"; save(fname); }
void Game::loadGame(const std::string& slotName) { std::string fname = slotName; if (fname.find(".sav") == std::string::npos) fname += ".sav"; load(fname); }
void Game::deleteSave(const std::string& slotName) { std::string fname = slotName + ".sav"; std::remove(fname.c_str()); }

void Game::tick() {
    if (!isRunning) return;
    if (winCondition && winCondition(*this)) { isRunning = false; return; }
    
    std::string roomName = currentRoom ? currentRoom->name : "Unknown";
    std::string status = roomName + " | Score: " + std::to_string(score) + " | Moves: " + std::to_string(moves);
    this->io->setStatus(status);
    
    this->io->print("\n> ");
    std::string input = this->io->requestInput();
    if (input.empty()) return;
    processCommand(input);
}
bool Game::isFinished() const { return !isRunning; }

Game::Game(IOInterface* io) : io(io) {
    registerDefaultVerbs();
    verbSynonyms["exit"] = "quit";
    verbSynonyms["quit"] = "quit";
    
    verbSynonyms["l"] = "look";
    verbSynonyms["examine"] = "look";
    verbSynonyms["x"] = "look";
    verbSynonyms["read"] = "look";
    verbSynonyms["look"] = "look";
    
    
    
    
    verbSynonyms["walk"] = "go";
    verbSynonyms["move"] = "go";
    verbSynonyms["leave"] = "go";
    verbSynonyms["enter"] = "go";
    verbSynonyms["go"] = "go";
    
    
    
    
    verbSynonyms["get"] = "take";
    verbSynonyms["grab"] = "take";
    verbSynonyms["take"] = "take";
    
    
    verbSynonyms["drop"] = "drop";
    verbSynonyms["help"] = "help";
    verbSynonyms["open"] = "open";
    verbSynonyms["shut"] = "close";
    verbSynonyms["close"] = "close";
    
    verbSynonyms["use"] = "use";
    verbSynonyms["play"] = "play";
    verbSynonyms["speak"] = "talk";
    verbSynonyms["talk"] = "talk";
    
    verbSynonyms["sleep"] = "sleep";
    verbSynonyms["wake"] = "wake";
    verbSynonyms["place"] = "put";
    verbSynonyms["put"] = "put";
    
    verbSynonyms["i"] = "inventory";
    verbSynonyms["inventory"] = "inventory";
    
    verbSynonyms["save"] = "save";
    verbSynonyms["load"] = "load";
    verbSynonyms["verbose"] = "verbose";
    verbSynonyms["brief"] = "brief";
    verbSynonyms["superbrief"] = "superbrief";
    verbSynonyms["score"] = "score";
    verbSynonyms["g"] = "again";
    verbSynonyms["again"] = "again";
    
    verbSynonyms["oops"] = "oops";
    prepMap["in"] = Preposition::IN;
    prepMap["inside"] = Preposition::IN;
    prepMap["into"] = Preposition::IN;
    prepMap["on"] = Preposition::ON;
    prepMap["onto"] = Preposition::ON;
    prepMap["with"] = Preposition::WITH;
    prepMap["using"] = Preposition::WITH;
    prepMap["to"] = Preposition::TO;
    prepMap["at"] = Preposition::AT;
    prepMap["under"] = Preposition::UNDER;
    prepMap["from"] = Preposition::FROM;
    objectSynonyms["n"] = "north";
    objectSynonyms["s"] = "south";
    objectSynonyms["e"] = "east";
    objectSynonyms["w"] = "west";

    // F-10
    darknessAllowedVerbs.insert("inventory");
    darknessAllowedVerbs.insert("i");
    darknessAllowedVerbs.insert("quit");
    darknessAllowedVerbs.insert("exit");
    darknessAllowedVerbs.insert("turn");
    darknessAllowedVerbs.insert("turn on");
    darknessAllowedVerbs.insert("light");
    
    // F-11
    addResponse("cant_go", {"\nYou can't go that way.\n"});
    addResponse("dont_see", {"\nYou don't see that here.\n"});
    addResponse("cant_take", {"\nYou can't take that.\n"});
    addResponse("go_where", {"\nGo where?\n"});
}

bool Game::hasFlag(const std::string& flag) const {
    auto it = globalFlags.find(flag);
    return it != globalFlags.end() && it->second;
}

void Game::addScore(int points) { score += points; }
void Game::setMaxScore(int points) { maxScore = points; }
