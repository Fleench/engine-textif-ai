/**
 * ==============================================================================
 * FICTIVE-AI STARTER TEMPLATE
 * Blessed Fra Angelico — Sacred Aesthetics & Harmonious Illumination
 * ==============================================================================
 * Minimalist, starter game template in C++17 demonstrating:
 *   - Zero-dependency single-header engine architecture
 *   - Unified stream I/O formatting (print << styles.cyan << ...)
 *   - Interactive loop driven by (input >> line)
 *   - Syntactic parsing via parse(line) with AST queries
 *   - Development debug diagnostics (debug.run, debug <<) elided in production (-p)
 *   - Clean room navigation, inventory manipulation, and object inspection
 *
 * Compile:
 *   Dev:  g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -I. templates/main.cpp -o game
 *   Prod: g++ -std=c++17 -DPRODUCTION -O3 -I. templates/main.cpp -o game_prod
 * ==============================================================================
 */

#include <fictive-ai/parser.hpp>
#include <fictive-ai/io.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

using namespace fictive;

// ==============================================================================
// World State & Data Structures
// ==============================================================================

enum class RoomId {
    Scriptorium,
    Garden,
    Library
};

struct Room {
    std::string name;
    std::string description;
    std::map<std::string, RoomId> exits;
    std::vector<std::string> items;
};

// ==============================================================================
// Game Engine Instance
// ==============================================================================

class StarterGame {
private:
    RoomId currentRoomId{RoomId::Scriptorium};
    std::vector<std::string> inventory;
    std::map<RoomId, Room> world;
    std::map<std::string, std::string> itemDetails;
    bool running{true};

    void initializeWorld() {
        // 1. Scriptorium
        world[RoomId::Scriptorium] = Room{
            "The Scriptorium of San Marco",
            "Morning sunlight filters through arched lancet windows, illuminating flecks "
            "of lapis lazuli and gold leaf on an oak writing desk. To the north lies the "
            "cloister garden; to the east opens the vaulted library.",
            {{"north", RoomId::Garden}, {"east", RoomId::Library}},
            {"manuscript", "pigment"}
        };

        // 2. Cloister Garden
        world[RoomId::Garden] = Room{
            "The Cloister Garden",
            "A peaceful courtyard surrounded by slender stone arcades. A marble fountain "
            "whispers soft praises at the center, its cool water reflecting the azure sky. "
            "To the south, the scriptorium invites quiet contemplation.",
            {{"south", RoomId::Scriptorium}},
            {"lily", "key"}
        };

        // 3. Vaulted Library
        world[RoomId::Library] = Room{
            "The Vaulted Library",
            "Towering cedar shelves rise toward ribbed stone vaults, bearing ancient folios "
            "and illuminated commentaries. The scent of dried parchment and beeswax fills "
            "the stillness. The scriptorium lies to the west.",
            {{"west", RoomId::Scriptorium}},
            {"codex"}
        };

        // Item Lore Descriptions
        itemDetails["manuscript"] = "A vellum leaf painted with radiant vermilion initials and delicate acanthus leaves.";
        itemDetails["pigment"]    = "A glass phial filled with luminous ultramarine pigment ground from raw lapis lazuli.";
        itemDetails["lily"]       = "A pure white lily freshly gathered from the cloister garden, fragrant and untainted.";
        itemDetails["key"]        = "A heavy brass key, engraved with a small cross upon its bow.";
        itemDetails["codex"]      = "A calfskin-bound volume containing polyphonic Gregorian hymns and sacred verse.";
    }

public:
    StarterGame() {
        initializeWorld();
    }

    void showWelcome() {
        print << styles.brightCyan << styles.bold
              << "\n=======================================================\n"
              << "       FICTIVE-AI : THE CLOISTER OF SAN MARCO\n"
              << "=======================================================\n"
              << styles.reset;
        print << styles.dim
              << "A starter interactive fiction created in the Communion of Saints.\n"
              << "Type " << styles.yellow << "help" << styles.reset << styles.dim
              << " for guidance, or " << styles.yellow << "look" << styles.reset << styles.dim
              << " to inspect your surroundings.\n\n"
              << styles.reset;
        describeCurrentRoom();
    }

    void describeCurrentRoom() {
        const auto& room = world[currentRoomId];

        print << styles.bold << styles.brightWhite << "[" << room.name << "]\n" << styles.reset;
        print << room.description << "\n";

        // Display Ground Items
        if (!room.items.empty()) {
            print << styles.green << "On the ground you notice: " << styles.reset;
            for (size_t i = 0; i < room.items.size(); ++i) {
                print << styles.brightYellow << room.items[i] << styles.reset;
                if (i + 1 < room.items.size()) print << ", ";
            }
            print << "\n";
        }

        // Display Exits
        print << styles.cyan << "Exits: " << styles.reset;
        size_t exitCount = 0;
        for (const auto& [dir, _] : room.exits) {
            print << styles.brightCyan << dir << styles.reset;
            if (++exitCount < room.exits.size()) print << ", ";
        }
        print << "\n";
    }

    void handleMovement(const std::string& direction) {
        auto& room = world[currentRoomId];
        auto it = room.exits.find(direction);
        if (it != room.exits.end()) {
            currentRoomId = it->second;
            print << styles.dim << "You walk toward the " << direction << "...\n\n" << styles.reset;
            describeCurrentRoom();
        } else {
            print << styles.red << "There is no passage leading " << direction << ".\n" << styles.reset;
        }
    }

    void handleLook() {
        describeCurrentRoom();
    }

    void handleExamine(const ParsedCommand& cmd) {
        if (!cmd.hasDirectObject()) {
            print << styles.yellow << "What would you like to examine?\n" << styles.reset;
            return;
        }

        const std::string& target = cmd.getDirectObject()->head;

        // Check Inventory
        auto invIt = std::find(inventory.begin(), inventory.end(), target);
        if (invIt != inventory.end()) {
            auto loreIt = itemDetails.find(target);
            if (loreIt != itemDetails.end()) {
                print << styles.brightWhite << loreIt->second << "\n" << styles.reset;
            } else {
                print << "You see nothing unusual about the " << target << ".\n";
            }
            return;
        }

        // Check Current Room
        const auto& room = world[currentRoomId];
        auto roomIt = std::find(room.items.begin(), room.items.end(), target);
        if (roomIt != room.items.end()) {
            auto loreIt = itemDetails.find(target);
            if (loreIt != itemDetails.end()) {
                print << styles.brightWhite << loreIt->second << "\n" << styles.reset;
            } else {
                print << "You see nothing unusual about the " << target << ".\n";
            }
            return;
        }

        print << styles.dim << "You do not see any '" << target << "' here.\n" << styles.reset;
    }

    void handleTake(const ParsedCommand& cmd) {
        if (!cmd.hasDirectObject()) {
            print << styles.yellow << "What would you like to take?\n" << styles.reset;
            return;
        }

        const std::string& target = cmd.getDirectObject()->head;
        auto& room = world[currentRoomId];
        auto it = std::find(room.items.begin(), room.items.end(), target);

        if (it != room.items.end()) {
            inventory.push_back(*it);
            room.items.erase(it);
            print << styles.green << "Taken." << styles.reset
                  << " You place the " << styles.brightYellow << target << styles.reset << " into your satchel.\n";
        } else {
            print << styles.dim << "There is no '" << target << "' here to take.\n" << styles.reset;
        }
    }

    void handleDrop(const ParsedCommand& cmd) {
        if (!cmd.hasDirectObject()) {
            print << styles.yellow << "What would you like to drop?\n" << styles.reset;
            return;
        }

        const std::string& target = cmd.getDirectObject()->head;
        auto it = std::find(inventory.begin(), inventory.end(), target);

        if (it != inventory.end()) {
            world[currentRoomId].items.push_back(*it);
            inventory.erase(it);
            print << styles.green << "Dropped." << styles.reset
                  << " You set down the " << styles.brightYellow << target << styles.reset << ".\n";
        } else {
            print << styles.yellow << "You are not carrying any '" << target << "'.\n" << styles.reset;
        }
    }

    void handleInventory() {
        if (inventory.empty()) {
            print << styles.dim << "Your satchel is empty.\n" << styles.reset;
            return;
        }

        print << styles.cyan << "You are currently carrying:\n" << styles.reset;
        for (const auto& item : inventory) {
            print << "  - " << styles.brightYellow << item << styles.reset;
            auto it = itemDetails.find(item);
            if (it != itemDetails.end()) {
                print << styles.dim << " (" << it->second << ")" << styles.reset;
            }
            print << "\n";
        }
    }

    void handleHelp() {
        print << styles.brightCyan << "\n--- Available Actions ---\n" << styles.reset;
        print << "  " << styles.bold << "Movement:  " << styles.reset << "north, south, east, west (or n, s, e, w)\n";
        print << "  " << styles.bold << "Observe:   " << styles.reset << "look (l), examine <item> (x <item>)\n";
        print << "  " << styles.bold << "Interact:  " << styles.reset << "take <item>, drop <item>, inventory (i)\n";
        print << "  " << styles.bold << "Session:   " << styles.reset << "help, quit / exit\n\n";
    }

    void runLoop() {
        showWelcome();

        std::string line;

        // Interactive Player Command Loop
        while (running) {
            print << "\n" << styles.bold << styles.brightCyan << "> " << styles.reset;

            if (!(input >> line)) {
                // Stream EOF reached
                break;
            }

            // Syntactic Parse via Fictive-AI
            auto cmd = parse(line);

            // Development Debug Diagnostics (Zero-cost when compiled with -DPRODUCTION)
            debug.run([&]{
                debug << styles.gray << "[DEBUG] Raw: '" << line << "' -> Valid: "
                      << (cmd.valid ? "true" : "false") << ", Verb: '" << cmd.verb << "'";
                if (cmd.hasDirectObject()) {
                    debug << ", Head Noun: '" << cmd.getDirectObject()->head << "'";
                }
                debug << styles.reset << "\n";
            });

            if (!cmd.valid) {
                print << styles.dim << "I do not comprehend that command. Type "
                      << styles.yellow << "help" << styles.reset << styles.dim
                      << " for ideas.\n" << styles.reset;
                continue;
            }

            // Directional Movement Navigation
            if (cmd.isDirection()) {
                handleMovement(cmd.getDirection());
                continue;
            }

            if (cmd.verb == "go") {
                if (cmd.hasDirectObject()) {
                    handleMovement(cmd.getDirectObject()->head);
                } else {
                    print << styles.yellow << "Which direction do you wish to go?\n" << styles.reset;
                }
                continue;
            }

            // Canonical Command Dispatch
            if (cmd.verb == "look" || cmd.verb == "l") {
                handleLook();
            } else if (cmd.verb == "examine" || cmd.verb == "x" || cmd.verb == "read") {
                handleExamine(cmd);
            } else if (cmd.verb == "take" || cmd.verb == "get") {
                handleTake(cmd);
            } else if (cmd.verb == "drop") {
                handleDrop(cmd);
            } else if (cmd.verb == "inventory" || cmd.verb == "i") {
                handleInventory();
            } else if (cmd.verb == "help") {
                handleHelp();
            } else if (cmd.verb == "quit" || cmd.verb == "exit") {
                print << styles.brightMagenta << "\nMay peace and grace go with you on your journey. Farewell.\n" << styles.reset;
                running = false;
            } else {
                print << styles.dim << "You ponder '" << cmd.rawVerb
                      << "', but find no fitting way to do that here.\n" << styles.reset;
            }
        }
    }
};

// ==============================================================================
// Entry Point
// ==============================================================================

int main() {
    StarterGame game;
    game.runLoop();
    return 0;
}
