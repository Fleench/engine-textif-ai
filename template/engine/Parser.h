#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>

enum class Preposition {
    NONE, UNKNOWN, IN, ON, WITH, TO, AT, UNDER, FROM
};

struct Action {
    std::string verb = "";
    std::string rawVerb = "";
    std::string rawArgs = "";
    std::vector<std::string> args;
    std::string object = "";
    Preposition prep = Preposition::NONE;
    std::string object2 = "";
    int argc = 0;
};

class Game;

class Parser {
public:
    static Action parseInput(Game& game, const std::string& input);
    static void executeAction(Game& game, const Action& action);
    static void processCommand(Game& game, const std::string& input);
};

#endif
