#ifndef SAVELOAD_H
#define SAVELOAD_H

#include <string>

class Game;

class SaveLoad {
public:
    static void save(Game& game, const std::string& filename);
    static void load(Game& game, const std::string& filename);
    static void saveGame(Game& game, const std::string& slotName);
    static void loadGame(Game& game, const std::string& slotName);
    static void deleteSave(Game& game, const std::string& slotName);
};

#endif
