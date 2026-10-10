#include "Room.h"
#include "IOInterface.h"
#include <cctype>

ExitHandler& ExitHandler::to(const std::string& dest) { destination = dest; return *this; }
ExitHandler& ExitHandler::needs(const std::string& key) { requiredKey = key; locked = true; return *this; }
ExitHandler& ExitHandler::denyMessage(const std::string& msg) { denyMsg = msg; return *this; }
bool ExitHandler::tryTraverse(IOInterface* io, const std::string& playerKey) {
    auto toLower = [](std::string s) { for (auto& c: s) c = std::tolower((unsigned char)c); return s; };
    if (locked && toLower(playerKey) != toLower(requiredKey)) {
        io->print(denyMsg + "\n");
        return false;
    }
    io->print("You go to " + destination + ".\n");
    return true;
}
