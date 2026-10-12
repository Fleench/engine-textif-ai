#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <stdexcept>
#include <algorithm>

#include "../engine/Game.h"
#include "../engine/GameObject.h"
#include "../engine/Item.h"
#include "../engine/Room.h"
#include "../engine/IOInterface.h"
#include "../game/game.h"

// St. Thomas's rigorous In-Memory Test IO
class CapturingIO : public IOInterface {
public:
    std::string buffer;
    void print(const std::string& msg) override {
        buffer += msg;
    }
    std::string requestInput() override {
        return "";
    }
    void clear() {
        buffer.clear();
    }
    bool contains(const std::string& needle) const {
        return buffer.find(needle) != std::string::npos;
    }
};

int total_assertions = 0;
int passed_assertions = 0;

#define SKEPTICAL_ASSERT(cond, msg) do { \
    total_assertions++; \
    if (!(cond)) { \
        std::cerr << "[-] WOUND DETECTED (Assertion Failed): " << msg << " (" << #cond << ") at line " << __LINE__ << std::endl; \
        std::exit(1); \
    } else { \
        passed_assertions++; \
    } \
} while (0)

void test_dynamic_help() {
    std::cout << "[St. Thomas] Examining Feature 1: Dynamic Help Command..." << std::endl;
    CapturingIO io;
    Game game(&io);
    auto room = std::make_unique<Room>("Test Chamber");
    room->isDark = false;
    game.currentRoom = room.get();
    game.rooms["Test Chamber"] = std::move(room);

    // Initial registered verbs include default verbs
    io.clear();
    game.processCommand("help");

    SKEPTICAL_ASSERT(io.contains("Available verbs: "), "Help output must start with 'Available verbs: '");
    SKEPTICAL_ASSERT(io.contains("look"), "Help output must list 'look'");
    SKEPTICAL_ASSERT(io.contains("go"), "Help output must list 'go'");
    SKEPTICAL_ASSERT(io.contains("inventory"), "Help output must list 'inventory'");
    SKEPTICAL_ASSERT(io.contains("take"), "Help output must list 'take'");
    SKEPTICAL_ASSERT(io.contains("drop"), "Help output must list 'drop'");
    SKEPTICAL_ASSERT(io.contains("quit"), "Help output must list 'quit'");

    // Verify verbs are formatted properly and ended with a period
    SKEPTICAL_ASSERT(io.buffer.find(".\n") != std::string::npos, "Help output must terminate with a period and newline");

    // Dynamic addition of new verbs
    game.registerVerb("resurrect", "You bring the target back to life.");
    io.clear();
    game.processCommand("help");
    SKEPTICAL_ASSERT(io.contains("resurrect"), "Newly registered verb 'resurrect' must immediately appear in help output");

    // Dynamic addition of second new verb
    game.registerVerb("transmute", "You transmute base metals into gold.");
    io.clear();
    game.processCommand("help");
    SKEPTICAL_ASSERT(io.contains("transmute"), "Newly registered verb 'transmute' must immediately appear in help output");

    // Verify verb help text / descriptions are NOT printed in dynamic help
    SKEPTICAL_ASSERT(!io.contains("You bring the target back to life"), "Verb help text must NOT be printed in the help output");
    SKEPTICAL_ASSERT(!io.contains("You transmute base metals into gold"), "Verb help text must NOT be printed in the help output");

    std::cout << "  [+] Dynamic help outputs all verbs cleanly and dynamically without help text." << std::endl;
}

void test_toggleable_help() {
    std::cout << "[St. Thomas] Examining Feature 2: Toggleable Help Command..." << std::endl;
    CapturingIO io;
    Game game(&io);
    auto room = std::make_unique<Room>("Test Chamber");
    room->isDark = false;
    game.currentRoom = room.get();
    game.rooms["Test Chamber"] = std::move(room);

    // 1. Initially enabled
    SKEPTICAL_ASSERT(game.isHelpEnabled() == true, "helpEnabled should be true by default");

    io.clear();
    game.processCommand("help");
    SKEPTICAL_ASSERT(io.contains("Available verbs:"), "Help should display verbs when enabled");

    // 2. Disable help
    game.setHelpEnabled(false);
    SKEPTICAL_ASSERT(game.isHelpEnabled() == false, "helpEnabled should be false after setHelpEnabled(false)");

    io.clear();
    game.processCommand("help");
    SKEPTICAL_ASSERT(io.contains("I don't understand that command."), "Disabled help must print 'I don't understand that command.'");
    SKEPTICAL_ASSERT(!io.contains("Available verbs:"), "Disabled help must NOT print available verbs list");

    // 3. Re-enable help
    game.setHelpEnabled(true);
    SKEPTICAL_ASSERT(game.isHelpEnabled() == true, "helpEnabled should be true after setHelpEnabled(true)");

    io.clear();
    game.processCommand("help");
    SKEPTICAL_ASSERT(io.contains("Available verbs:"), "Re-enabled help should display available verbs list again");
    SKEPTICAL_ASSERT(!io.contains("I don't understand that command."), "Re-enabled help must not print unknown command");

    std::cout << "  [+] Help command toggle successfully toggles between dynamic listing and 'I don't understand that command.'" << std::endl;
}

void test_global_verb_registration_and_object_binding() {
    std::cout << "[St. Thomas] Examining Feature 3: Global Verb Registration & Object Binding..." << std::endl;
    CapturingIO io;
    Game game(&io);

    auto room = std::make_unique<Room>("Laboratory");
    game.currentRoom = room.get();
    game.rooms["Laboratory"] = std::move(room);

    // Subcase 3.1: Verb registered via registerVerb("call", "no phone nearby") run with no args/target
    game.registerVerb("call", "no phone nearby");
    io.clear();
    game.processCommand("call");
    SKEPTICAL_ASSERT(io.contains("no phone nearby"), "'call' with no args must print 'no phone nearby'");

    // Register another verb to be sure
    game.registerVerb("whisper", "Nobody hears your quiet voice.");
    io.clear();
    game.processCommand("whisper");
    SKEPTICAL_ASSERT(io.contains("Nobody hears your quiet voice."), "'whisper' with no args must print registered default message");

    // Subcase 3.2: Object binds to unregistered verb using supports(game, "unregistered_verb") -> throws std::runtime_error
    GameObject obj1("strange_artifact");
    bool threw1 = false;
    std::string exMsg1 = "";
    try {
        obj1.supports(game, "unregistered_verb");
    } catch (const std::runtime_error& e) {
        threw1 = true;
        exMsg1 = e.what();
    } catch (...) {
        SKEPTICAL_ASSERT(false, "Threw unexpected exception type, expected std::runtime_error");
    }
    SKEPTICAL_ASSERT(threw1, "obj.supports(game, 'unregistered_verb') MUST throw std::runtime_error immediately");
    SKEPTICAL_ASSERT(exMsg1.find("unregistered_verb") != std::string::npos, "Exception message must mention verb name");
    SKEPTICAL_ASSERT(exMsg1.find("not registered globally") != std::string::npos, "Exception message must state verb is not registered globally");

    // Subcase 3.3: Object binds to unregistered verb with custom lambda handler -> throws std::runtime_error
    bool threw2 = false;
    std::string exMsg2 = "";
    try {
        obj1.supports(game, "levitate", [](Game&, GameObject*, const Action&) {});
    } catch (const std::runtime_error& e) {
        threw2 = true;
        exMsg2 = e.what();
    } catch (...) {
        SKEPTICAL_ASSERT(false, "Threw unexpected exception type for 2-arg supports, expected std::runtime_error");
    }
    SKEPTICAL_ASSERT(threw2, "obj.supports(game, 'levitate', lambda) MUST throw std::runtime_error immediately");
    SKEPTICAL_ASSERT(exMsg2.find("levitate") != std::string::npos, "Exception message must mention verb 'levitate'");

    // Subcase 3.4: Object binds to a registered verb with custom handler, targeted by user command
    game.registerVerb("zap", "You don't have anything to zap with.");
    auto wandDef = std::make_unique<ItemDef>("wand", "A sparkling wand.");
    auto wand = std::make_unique<Item>("wand", wandDef.get());
    
    bool customHandlerRan = false;
    wand->supports(game, "zap", [&](Game& g, GameObject* obj, const Action& act) {
        customHandlerRan = true;
        g.io->print("Sparks shoot out from the wand!\n");
    });

    game.currentRoom->items.push_back(wand.get());
    game.items["wand"] = std::move(wand);
    game.itemDefs["wand"] = std::move(wandDef);

    io.clear();
    game.processCommand("zap wand");
    SKEPTICAL_ASSERT(customHandlerRan == true, "Custom object handler MUST run when targeted object supports verb");
    SKEPTICAL_ASSERT(io.contains("Sparks shoot out from the wand!"), "Output should contain custom handler output");

    // Subcase 3.5: Object in inventory targeted
    auto communicatorDef = std::make_unique<ItemDef>("radio", "A portable radio.");
    auto communicator = std::make_unique<Item>("radio", communicatorDef.get());
    bool radioCalled = false;
    communicator->supports(game, "call", [&](Game& g, GameObject*, const Action&) {
        radioCalled = true;
        g.io->print("Static crackles on the radio frequency.\n");
    });
    game.inventory.push_back(communicator.get());
    game.items["radio"] = std::move(communicator);
    game.itemDefs["radio"] = std::move(communicatorDef);

    io.clear();
    game.processCommand("call radio");
    SKEPTICAL_ASSERT(radioCalled == true, "Targeted item in inventory must trigger custom handler for 'call'");
    SKEPTICAL_ASSERT(io.contains("Static crackles on the radio frequency."), "Inventory item custom handler output confirmed");

    // Subcase 3.6: Object binds with default delegate (1-arg supports)
    game.registerVerb("scan", "Scanner emits a generic beep.");
    auto scannerDef = std::make_unique<ItemDef>("scanner", "A handheld device.");
    auto scanner = std::make_unique<Item>("scanner", scannerDef.get());
    scanner->supports(game, "scan"); // Uses 1-arg overload, delegates to defaultFn
    game.inventory.push_back(scanner.get());
    game.items["scanner"] = std::move(scanner);
    game.itemDefs["scanner"] = std::move(scannerDef);

    io.clear();
    game.processCommand("scan scanner");
    SKEPTICAL_ASSERT(io.contains("Scanner emits a generic beep."), "1-arg supports must delegate to defaultFn of registered verb");

    std::cout << "  [+] Global verb registration, unregistered rejection, and custom handler dispatch verified." << std::endl;
}

void test_full_mall_game_integration() {
    std::cout << "[St. Thomas] Examining Feature 4: Integration with Full Game Setup..." << std::endl;
    CapturingIO io;
    Game game(&io);
    game.init(); // Run the complete game.cpp initialization

    // Check registered verbs
    SKEPTICAL_ASSERT(game.verbRegistry.count("call") > 0, "'call' must be registered in game.init()");
    SKEPTICAL_ASSERT(game.verbRegistry.count("turn on") > 0, "'turn on' must be registered in game.init()");
    SKEPTICAL_ASSERT(game.verbRegistry.count("turn off") > 0, "'turn off' must be registered in game.init()");

    // Check help dynamic listing includes all mall verbs
    io.clear();
    game.processCommand("help");
    SKEPTICAL_ASSERT(io.contains("call"), "Help must contain 'call'");
    SKEPTICAL_ASSERT(io.contains("turn on"), "Help must contain 'turn on'");
    SKEPTICAL_ASSERT(io.contains("turn off"), "Help must contain 'turn off'");

    // Check phone binding
    // Move player to Central Atrium where phone is
    game.currentRoom = game.rooms["Central Atrium"].get();
    
    // Call phone while in the room
    io.clear();
    game.processCommand("call phone");
    SKEPTICAL_ASSERT(io.contains("Calling phone... no answer."), "Targeted phone must invoke phone's custom handler");

    // Move player away from phone (e.g. to Food Court - Burgers)
    game.currentRoom = game.rooms["Food Court - Burgers"].get();
    // Ensure phone is NOT in inventory
    auto it = std::find_if(game.inventory.begin(), game.inventory.end(), [](Item* i){ return i->id == "phone"; });
    if (it != game.inventory.end()) game.inventory.erase(it);

    io.clear();
    game.processCommand("call");
    SKEPTICAL_ASSERT(io.contains("no phone nearby"), "'call' when no phone nearby must output 'no phone nearby'");

    // Toggle help in mall game
    game.setHelpEnabled(false);
    io.clear();
    game.processCommand("help");
    SKEPTICAL_ASSERT(io.contains("I don't understand that command."), "Mall game with help disabled prints unknown command");

    game.setHelpEnabled(true);
    io.clear();
    game.processCommand("help");
    SKEPTICAL_ASSERT(io.contains("Available verbs:"), "Mall game re-enables help properly");

    std::cout << "  [+] Mall game integration passed all checks." << std::endl;
}

int main() {
    std::cout << "=======================================================" << std::endl;
    std::cout << " St. Thomas the Apostle - Parser Feature Verification " << std::endl;
    std::cout << " \"Unless I see the successful test output, I will not believe.\" " << std::endl;
    std::cout << "=======================================================" << std::endl;

    test_dynamic_help();
    test_toggleable_help();
    test_global_verb_registration_and_object_binding();
    test_full_mall_game_integration();

    std::cout << "=======================================================" << std::endl;
    std::cout << " [VERIFIED] All " << passed_assertions << " / " << total_assertions 
              << " skeptical assertions PASSED without error!" << std::endl;
    std::cout << "=======================================================" << std::endl;
    return 0;
}
