#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "game.h"
#include <iostream>
#include <chrono>

using namespace std;

// --- Utility Functions ---

void clearScreenMain(Game& game) {
    game.io->print("\033[2J\033[1;1H");
}

void resetCursorMain(Game& game) {
    game.io->print("\033[1;1H");
}

// --- Mini-Games ---

std::string playFlappyBird(Game& game) {
    clearScreenMain(game);
    game.io->print("Welcome to Flappy Bird! Press Space to flap, 'q' to quit.\n");
    game.io->print("Survive for 3 pipes to win.\n");
    
    float birdY = 10.0f;
    float birdVelocity = 0.0f;
    float pipeX = 40.0f;
    int score = 0;
    bool playing = true;

    clearScreenMain(game); // initial clear

    while (playing) {
        resetCursorMain(game); // avoid flicker
        
        // Input
        std::string in = game.io->requestInput();
        char c = in.empty() ? 0 : in[0];
        if (c == ' ') {
            birdVelocity = -0.4f;
        } else if (c == 'q') {
            playing = false;
        }

        // Update
        birdY += birdVelocity;
        birdVelocity += 0.02f; // Gravity
        pipeX -= 0.3f;

        if (pipeX < 0.0f) {
            pipeX = 40.0f;
            score++;
        }

        // Collision
        if (birdY < 0.0f || birdY >= 20.0f) {
            return "You crashed! Game Over.";
        }

        if (pipeX <= 10.0f && pipeX + 2.0f > 10.0f) {
            if (birdY < 8.0f || birdY > 12.0f) {
                return "You hit a pipe! Game Over.";
            }
        }

        if (score >= 3) {
            game.globalFlags["miniGame1Won"] = true;
            game.addScore(1);
            return "You won Flappy Bird! You survived 3 pipes!";
        }

        // Render
        string buffer;
        for (int i = 0; i < 20; ++i) {
            for (int j = 0; j < 50; ++j) {
                if (j == 10 && i == (int)birdY) {
                    buffer += "O";
                } else if (j == (int)pipeX && (i < 8 || i > 12)) {
                    buffer += "|";
                } else {
                    buffer += " ";
                }
            }
            buffer += "\n";
        }
        buffer += "Score: " + to_string(score) + "/3        \n";
        game.io->print(buffer);

        game.io->sleepFrame(16);
    }
    return "";
}

std::string playBurgerFlipper(Game& game) {
    clearScreenMain(game);
    game.io->print("Burger Flipper Mini-game!\n");
    game.io->print("Type 'flip' exactly 3 times when prompted to cook the burger perfectly.\n");
    for (int i = 0; i < 3; ++i) {
        game.io->print("Ready...\n");
        game.io->print("Type 'flip' NOW: ");
        string input = game.io->requestInput();
        if (input != "flip") {
            return "You burned it! Game Over.";
        }
    }
    game.globalFlags["miniGame2Won"] = true;
    game.addScore(1);
    return "Perfect burger! You won!";
}

std::string playReactionTest(Game& game) {
    clearScreenMain(game);
    game.io->print("Reaction Test!\n");
    game.io->print("Wait for the GO signal, then type 'g' and press enter.\n");
    
    game.io->print("GO!\n");
    auto start = chrono::high_resolution_clock::now();
    string input = game.io->requestInput();
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    
    if (input == "g" && duration < 1500) {
        game.globalFlags["miniGame3Won"] = true;
        game.addScore(1);
        return "Fast reflexes! " + to_string(duration) + "ms. You won!";
    } else {
        return "Too slow or wrong input (" + to_string(duration) + "ms). Game Over.";
    }
}

std::string playMathQuiz(Game& game) {
    clearScreenMain(game);
    game.io->print("Math Quiz at the Bookstore!\n");
    game.io->print("What is 15 * 12?\n");
    string input = game.io->requestInput();
    try {
        int answer = std::stoi(input);
        if (answer == 180) {
            game.globalFlags["miniGame4Won"] = true;
            game.addScore(1);
            return "Correct! You won!";
        } else {
            return "Wrong answer! Game Over.";
        }
    } catch (...) {
        return "Invalid input! Game Over.";
    }
}

std::string playTypingTest(Game& game) {
    clearScreenMain(game);
    game.io->print("Typing Test!\n");
    game.io->print("Type: 'The quick brown fox'\n");
    string input = game.io->requestInput();
    if (input == "The quick brown fox") {
        game.globalFlags["miniGame5Won"] = true;
        game.addScore(1);
        return "Excellent typing! You won!";
    } else {
        return "Typo! Game Over.";
    }
}

std::string playPotionBrewing(Game& game) {
    game.clearScreen();
    game.printColored("Potion Brewing!", 1);
    game.printBold("Add ingredients in the correct order.");
    game.typewriterEffect("1. Red\n2. Blue\n", 50);
    game.globalFlags["miniGamePotionWon"] = true;
    game.addScore(1);
    return "Potion brewed successfully!";
}

std::string playSwordSharpening(Game& game) {
    game.clearScreen();
    game.shakeScreen(5, 100);
    game.printItalic("Sparks fly as you sharpen the sword!");
    game.playAudio("sparks.wav");
    game.loopAudio("grind.wav");
    game.stopAudio();
    game.setAudioVolume(0.5f);
    game.globalFlags["miniGameSwordWon"] = true;
    game.addScore(1);
    return "Sword sharpened to perfection!";
}

std::string playLockpicking(Game& game) {
    game.clearScreen();
    game.flashScreen(2);
    game.printColored("Click! You picked the lock.", 3);
    game.globalFlags["miniGameLockWon"] = true;
    game.addScore(1);
    return "Lock picked! The chest is open.";
}

void Game::init() {
    Game& game = *this;
    game.addResponse("cant_go", {"You bumped into a wall.", "Path's blocked.", "Nowhere to go that way."});
    game.addResponse("dont_see", {"You don't see that here.", "Invisible or missing.", "Not here!"});
    game.setMaxScore(8);

    // --- Phase 1: Creation ---
    this->miniGames["flappyBird"] = make_unique<MiniGame>("flappyBird");
    this->miniGames["flappyBird"]->gameRun = playFlappyBird;
    this->miniGames["flappyBird"]->isRealTimeMode = true;

    this->miniGames["burgerFlipper"] = make_unique<MiniGame>("burgerFlipper");
    this->miniGames["burgerFlipper"]->gameRun = playBurgerFlipper;

    this->miniGames["reactionTest"] = make_unique<MiniGame>("reactionTest");
    this->miniGames["reactionTest"]->gameRun = playReactionTest;

    this->miniGames["mathQuiz"] = make_unique<MiniGame>("mathQuiz");
    this->miniGames["mathQuiz"]->gameRun = playMathQuiz;

    this->miniGames["typingTest"] = make_unique<MiniGame>("typingTest");
    this->miniGames["typingTest"]->gameRun = playTypingTest;

    this->rooms["Central Atrium"] = make_unique<Room>("Central Atrium");
    this->rooms["Central Atrium"]->setDescription("The heart of the mall. Bustling with people.\nNPC [Security Guard]: \"Keep moving, kid.\"");
    this->rooms["Central Atrium"]->setShortDescription("Central Atrium");

    this->rooms["Food Court - Pizza"] = make_unique<Room>("Food Court - Pizza");
    this->rooms["Food Court - Pizza"]->setDescription("Smells like cheesy goodness.\nNPC [Cashier]: \"Want a slice?\"");
    this->rooms["Food Court - Pizza"]->setShortDescription("Pizza Place");

    this->rooms["Food Court - Burgers"] = make_unique<Room>("Food Court - Burgers");
    this->rooms["Food Court - Burgers"]->setDescription("Sizzling burgers on the grill.\nNPC [Chef]: \"Help me flip these burgers!\"");
    this->rooms["Food Court - Burgers"]->setShortDescription("Burger Joint");

    this->rooms["Food Court - Ice Cream"] = make_unique<Room>("Food Court - Ice Cream");
    this->rooms["Food Court - Ice Cream"]->setDescription("Cold treats for everyone.");
    this->rooms["Food Court - Ice Cream"]->setShortDescription("Ice Cream Stand");

    this->rooms["Arcade"] = make_unique<Room>("Arcade");
    this->rooms["Arcade"]->setDescription("Loud noises and flashing lights.\nNPC [Mall Rat]: \"Bet you can't beat Flappy Bird.\"");
    this->rooms["Arcade"]->setShortDescription("Arcade");

    this->rooms["Bookstore"] = make_unique<Room>("Bookstore");
    this->rooms["Bookstore"]->setDescription("Quiet and full of books.\nNPC [Librarian]: \"Shh. Read a book.\"");
    this->rooms["Bookstore"]->setShortDescription("Bookstore");

    this->rooms["Electronics Store"] = make_unique<Room>("Electronics Store");
    this->rooms["Electronics Store"]->setDescription("The latest gadgets.\nNPC [Techie]: \"Test your reflexes on this new screen.\"");
    this->rooms["Electronics Store"]->setShortDescription("Electronics Store");

    this->rooms["Pet Shop"] = make_unique<Room>("Pet Shop");
    this->rooms["Pet Shop"]->setDescription("Puppies and kittens everywhere.\nNPC [Owner]: \"Don't tap the glass.\"");
    this->rooms["Pet Shop"]->setShortDescription("Pet Shop");

    this->rooms["Clothing Store"] = make_unique<Room>("Clothing Store");
    this->rooms["Clothing Store"]->setDescription("Trendy clothes on mannequins.");
    this->rooms["Clothing Store"]->setShortDescription("Clothing Store");

    this->rooms["Office Supplies"] = make_unique<Room>("Office Supplies");
    this->rooms["Office Supplies"]->setDescription("Pens and paper.\nNPC [Clerk]: \"Try our new keyboards!\"");
    this->rooms["Office Supplies"]->setShortDescription("Office Supplies");

    this->itemDefs["grill"] = make_unique<ItemDef>("grill", "A hot grill for flipping burgers.");
    this->items["grill"] = make_unique<Item>("grill", this->itemDefs["grill"].get());
    this->items["grill"]->supports("use", [](Game& g, GameObject*, const Action& a) { g.miniGames["burgerFlipper"]->run(g); });

    this->itemDefs["machine"] = make_unique<ItemDef>("machine", "A flappy bird arcade machine.");
    this->items["machine"] = make_unique<Item>("machine", this->itemDefs["machine"].get());
    this->items["machine"]->supports("take", [this](Game& g, GameObject*, const Action& a) { Game& game = *this;
        game.io->print("The arcade machine is bolted to the floor!\n");
    });
    this->items["machine"]->supports("use", [](Game& g, GameObject*, const Action& a) { g.miniGames["flappyBird"]->run(g); });

    this->itemDefs["book"] = make_unique<ItemDef>("book", "A scary math textbook.");
    this->items["book"] = make_unique<Item>("book", this->itemDefs["book"].get());
    this->items["book"]->supports("use", [](Game& g, GameObject*, const Action& a) { g.miniGames["mathQuiz"]->run(g); });

    this->itemDefs["screen"] = make_unique<ItemDef>("screen", "A reflex-testing screen.");
    this->items["screen"] = make_unique<Item>("screen", this->itemDefs["screen"].get());
    this->items["screen"]->supports("use", [](Game& g, GameObject*, const Action& a) { g.miniGames["reactionTest"]->run(g); });

    this->itemDefs["keyboard"] = make_unique<ItemDef>("keyboard", "A mechanical keyboard.");
    this->items["keyboard"] = make_unique<Item>("keyboard", this->itemDefs["keyboard"].get());
    this->items["keyboard"]->supports("use", [](Game& g, GameObject*, const Action& a) { g.miniGames["typingTest"]->run(g); });

    this->itemDefs["phone"] = make_unique<ItemDef>("phone", "A smartphone with a cracked screen.");
    this->items["phone"] = make_unique<Item>("phone", this->itemDefs["phone"].get());
    this->items["phone"]->supports("call", [this](Game& g, GameObject*, const Action& a) { Game& game = *this;
        string arg = a.object; // fallback or something
        game.io->print("Calling " + (arg.empty() ? "nobody" : arg) + "... no answer.\n");
    });

    // --- Phase 1: Expansion Shops ---
    this->rooms["Potion Shop"] = make_unique<Room>("Potion Shop");
    this->rooms["Potion Shop"]->setShortDescription("Potion Shop");
    this->rooms["Blacksmith"] = make_unique<Room>("Blacksmith");
    this->rooms["Blacksmith"]->setShortDescription("Blacksmith");
    this->rooms["Magic Scroll Shop"] = make_unique<Room>("Magic Scroll Shop");
    this->rooms["Magic Scroll Shop"]->setShortDescription("Scroll Shop");
    this->rooms["Tavern"] = make_unique<Room>("Tavern");
    this->rooms["Tavern"]->setShortDescription("Tavern");
    this->rooms["General Store"] = make_unique<Room>("General Store");
    this->rooms["General Store"]->setShortDescription("General Store");
    
    // Minigames
    this->miniGames["potionBrewing"] = make_unique<MiniGame>("potionBrewing");
    this->miniGames["potionBrewing"]->gameRun = playPotionBrewing;
    this->miniGames["swordSharpening"] = make_unique<MiniGame>("swordSharpening");
    this->miniGames["swordSharpening"]->gameRun = playSwordSharpening;
    this->miniGames["lockpicking"] = make_unique<MiniGame>("lockpicking");
    this->miniGames["lockpicking"]->gameRun = playLockpicking;

    // NPCs
    this->npcs["alchemist"] = make_unique<NPC>("Alchemist");
    this->npcs["alchemist"]->health = 100;
    this->npcs["alchemist"]->isAlive = true;
    this->itemDefs["talk_alchemist"] = make_unique<ItemDef>("talk_alchemist", "Talk to the alchemist.");
    this->items["talk_alchemist"] = make_unique<Item>("talk_alchemist", this->itemDefs["talk_alchemist"].get());
    this->items["talk_alchemist"]->supports("talk", [](Game& g, GameObject*, const Action& a) {
        g.startDialogueTree("alchemist_tree");
        g.addDialogueOption("What are you brewing?", "node_brew");
        g.addDialogueOption("Goodbye", "node_exit");
        g.endDialogue();
    });
    
    this->npcs["blacksmith"] = make_unique<NPC>("Blacksmith");
    this->npcs["blacksmith"]->health = 200;
    this->itemDefs["talk_blacksmith"] = make_unique<ItemDef>("talk_blacksmith", "Talk to the blacksmith.");
    this->items["talk_blacksmith"] = make_unique<Item>("talk_blacksmith", this->itemDefs["talk_blacksmith"].get());
    this->items["talk_blacksmith"]->supports("talk", [](Game& g, GameObject*, const Action& a) {
        if (!g.getFlag("met_blacksmith")) {
            g.setFlag("met_blacksmith", true);
            g.startDialogueTree("blacksmith_intro");
            g.addDialogueOption("Hello, I need a sword.", "node_sword");
            g.endDialogue();
        } else {
            g.startDialogueTree("blacksmith_returning");
            g.endDialogue();
        }
    });

    this->npcs["mystic"] = make_unique<NPC>("Mystic");
    this->itemDefs["talk_mystic"] = make_unique<ItemDef>("talk_mystic", "Talk to the mystic.");
    this->items["talk_mystic"] = make_unique<Item>("talk_mystic", this->itemDefs["talk_mystic"].get());
    this->items["talk_mystic"]->supports("talk", [](Game& g, GameObject*, const Action& a) {
        g.startDialogueTree("mystic_tree");
        g.addDialogueOption("Tell my future", "node_future");
        g.endDialogue();
    });

    this->npcs["bartender"] = make_unique<NPC>("Bartender");
    this->itemDefs["talk_bartender"] = make_unique<ItemDef>("talk_bartender", "Talk to the bartender.");
    this->items["talk_bartender"] = make_unique<Item>("talk_bartender", this->itemDefs["talk_bartender"].get());
    this->items["talk_bartender"]->supports("talk", [](Game& g, GameObject*, const Action& a) {
        g.startDialogueTree("bartender_tree");
        g.addDialogueOption("Give me a drink", "node_drink");
        g.endDialogue();
    });

    // Door
    this->doors["magic_door"] = make_unique<Door>("magic_door");
    this->doors["magic_door"]->needs("has_key").denyMessage("The door is locked.").canOpenIf([](Game& g) { return true; });

    // Item to interact and use QOL functions
    this->itemDefs["qol_tester"] = make_unique<ItemDef>("qol_tester", "A device testing the fabric of reality.");
    this->items["qol_tester"] = make_unique<Item>("qol_tester", this->itemDefs["qol_tester"].get());
    this->items["qol_tester"]->supports("use", [](Game& g, GameObject*, const Action& a) {
        g.setFlag("test_flag", true);
        g.getFlag("test_flag");
        g.giveItemToPlayer("qol_tester");
        g.removeInvItem("qol_tester");
        g.playerHasItem("qol_tester");
        g.spawnItem("qol_tester", "Central Atrium");
        g.movePlayerTo("Potion Shop");
        g.moveItem("qol_tester", "Tavern");
        g.createTimer(5, "bomb_timer");
        g.pauseTimer("bomb_timer");
        g.resumeTimer("bomb_timer");
        g.cancelTimer("bomb_timer");
        
        g.registerEventHook("test_event", [&g]() { g.io->print("Event hooked!\n"); });
        g.triggerEvent("test_event");
        g.clearEventHooks("test_event");
        
        g.startDialogueTree("test_tree");
        g.addDialogueOption("Hello", "node2");
        g.endDialogue();
        g.hasSpokenTo("alchemist");
        
        g.saveGame("slot1");
        g.loadGame("slot1");
        g.deleteSave("slot1");
    });
    
    this->itemDefs["alchemist_desk"] = make_unique<ItemDef>("alchemist_desk", "A desk for brewing.");
    this->itemDefs["alchemist_desk"]->description = "A desk for brewing."; // Just to keep def
    this->itemDefs["alchemist_desk"]->onUse = [](Game& g, const std::string&) { g.miniGames["potionBrewing"]->run(g); };
    this->storages["alchemist_desk"] = make_unique<Storage>("alchemist_desk", this->itemDefs["alchemist_desk"].get());
    this->storages["alchemist_desk"]->isSupporter = true;

    this->itemDefs["anvil"] = make_unique<ItemDef>("anvil", "An anvil for sharpening.");
    this->items["anvil"] = make_unique<Item>("anvil", this->itemDefs["anvil"].get());
    this->items["anvil"]->supports("use", [](Game& g, GameObject*, const Action& a) { g.miniGames["swordSharpening"]->run(g); });

    this->itemDefs["locked_chest"] = make_unique<ItemDef>("locked_chest", "A chest to pick.");
    this->items["locked_chest"] = make_unique<Item>("locked_chest", this->itemDefs["locked_chest"].get());
    this->items["locked_chest"]->supports("use", [](Game& g, GameObject*, const Action& a) { g.miniGames["lockpicking"]->run(g); });

    // --- Phase 2: Wiring ---
    this->rooms["Central Atrium"]->addExit("north", this->rooms["Food Court - Pizza"].get());
    this->rooms["Central Atrium"]->addExit("south", this->rooms["Food Court - Burgers"].get());
    this->rooms["Central Atrium"]->addExit("east", this->rooms["Food Court - Ice Cream"].get());
    this->rooms["Central Atrium"]->addExit("west", this->rooms["Arcade"].get());

    this->rooms["Food Court - Pizza"]->addExit("south", this->rooms["Central Atrium"].get());
    this->rooms["Food Court - Pizza"]->addExit("east", this->rooms["Bookstore"].get());

    this->rooms["Food Court - Burgers"]->addExit("north", this->rooms["Central Atrium"].get());
    this->rooms["Food Court - Burgers"]->addExit("west", this->rooms["Electronics Store"].get());

    this->rooms["Food Court - Ice Cream"]->addExit("west", this->rooms["Central Atrium"].get());
    this->rooms["Food Court - Ice Cream"]->addExit("south", this->rooms["Pet Shop"].get());

    this->rooms["Arcade"]->addExit("east", this->rooms["Central Atrium"].get());
    this->rooms["Arcade"]->addExit("north", this->rooms["Clothing Store"].get());

    this->rooms["Bookstore"]->addExit("west", this->rooms["Food Court - Pizza"].get());
    this->rooms["Bookstore"]->addExit("south", this->rooms["Office Supplies"].get());

    this->rooms["Electronics Store"]->addExit("east", this->rooms["Food Court - Burgers"].get());
    this->rooms["Pet Shop"]->addExit("north", this->rooms["Food Court - Ice Cream"].get());
    this->rooms["Clothing Store"]->addExit("south", this->rooms["Arcade"].get());
    this->rooms["Office Supplies"]->addExit("north", this->rooms["Bookstore"].get());
    // Darkness & Illumination
    this->rooms["Closet"] = make_unique<Room>("Closet");
    this->rooms["Closet"]->setShortDescription("Broom Closet");
    this->rooms["Closet"]->setDescription("A very dark closet.");
    this->rooms["Closet"]->isDark = true;
    
    this->rooms["Office Supplies"]->addExit("east", this->rooms["Closet"].get());
    this->rooms["Closet"]->addExit("west", this->rooms["Office Supplies"].get());
    
    this->itemDefs["lantern"] = make_unique<ItemDef>("lantern", "A bright lantern.");
    this->items["lantern"] = make_unique<Item>("lantern", this->itemDefs["lantern"].get());
    this->items["lantern"]->isLightSource = true;
    this->items["lantern"]->supports("turn on", [](Game& g, GameObject* obj, const Action& a) {
        Item* lantern = static_cast<Item*>(obj);
        lantern->isLit = true;
        g.io->print("You turn on the lantern.\n");
    });
    this->items["lantern"]->supports("turn off", [](Game& g, GameObject* obj, const Action& a) {
        Item* lantern = static_cast<Item*>(obj);
        lantern->isLit = false;
        g.io->print("You turn off the lantern.\n");
    });
    this->rooms["Office Supplies"]->addItem(this->items["lantern"].get());


    this->rooms["Food Court - Burgers"]->addItem(this->items["grill"].get());
    this->rooms["Arcade"]->addItem(this->items["machine"].get());
    this->rooms["Bookstore"]->addItem(this->items["book"].get());
    this->rooms["Electronics Store"]->addItem(this->items["screen"].get());
    this->rooms["Office Supplies"]->addItem(this->items["keyboard"].get());
    this->rooms["Central Atrium"]->addItem(this->items["phone"].get());

    // --- Phase 2: Wiring Expansion ---
    this->rooms["Central Atrium"]->addExit("up", this->rooms["Potion Shop"].get());
    this->rooms["Potion Shop"]->addExit("down", this->rooms["Central Atrium"].get());
    this->rooms["Potion Shop"]->addExit("north", this->rooms["Blacksmith"].get(), this->doors["magic_door"].get());
    this->rooms["Blacksmith"]->addExit("south", this->rooms["Potion Shop"].get());
    this->rooms["Blacksmith"]->addExit("east", this->rooms["Magic Scroll Shop"].get());
    this->rooms["Magic Scroll Shop"]->addExit("west", this->rooms["Blacksmith"].get());
    this->rooms["Magic Scroll Shop"]->addExit("south", this->rooms["Tavern"].get());
    this->rooms["Tavern"]->addExit("north", this->rooms["Magic Scroll Shop"].get());
    this->rooms["Tavern"]->addExit("west", this->rooms["General Store"].get());
    this->rooms["General Store"]->addExit("east", this->rooms["Tavern"].get());

    this->rooms["Potion Shop"]->addItem(this->storages["alchemist_desk"].get());
    this->rooms["Potion Shop"]->addItem(this->items["talk_alchemist"].get());
    this->rooms["Blacksmith"]->addItem(this->items["anvil"].get());
    this->rooms["Blacksmith"]->addItem(this->items["talk_blacksmith"].get());
    this->rooms["Tavern"]->addItem(this->items["locked_chest"].get());
    this->rooms["Tavern"]->addItem(this->items["talk_bartender"].get());
    this->rooms["Magic Scroll Shop"]->addItem(this->items["talk_mystic"].get());
    this->rooms["General Store"]->addItem(this->items["qol_tester"].get());
    this->itemDefs["magic_sack"] = make_unique<ItemDef>("magic_sack", "A sack holding infinite possibilities.");
    this->storages["magic_sack"] = make_unique<Storage>("magic_sack", this->itemDefs["magic_sack"].get());
    this->itemDefs["apple"] = make_unique<ItemDef>("apple", "A red apple.");
    this->items["apple"] = make_unique<Item>("apple", this->itemDefs["apple"].get());
    this->storages["magic_sack"]->items.push_back(this->items["apple"].get());
    this->rooms["General Store"]->addItem(this->storages["magic_sack"].get());


    this->currentRoom = this->rooms["Central Atrium"].get();

    // Win condition
    this->winCondition = [this](Game& g) { Game& game = *this;
        if (g.hasFlag("miniGame1Won") && g.hasFlag("miniGame2Won") &&
            g.hasFlag("miniGame3Won") && g.hasFlag("miniGame4Won") &&
            g.hasFlag("miniGame5Won") &&
            g.hasFlag("miniGamePotionWon") && g.hasFlag("miniGameSwordWon") &&
            g.hasFlag("miniGameLockWon")) {
            
            clearScreenMain(game);
            game.io->print("***********************************\n");
            game.io->print("* CONGRATULATIONS! YOU BEAT THE MALL! *\n");
            game.io->print("***********************************\n");
            return true;
        }
        return false;
    };

    game.io->print("Welcome to the Mall Exploration Game!\n");
    game.io->print("Beat all 5 mini-games to win.\n");
    game.io->print("(Hint: type 'play machine', 'use book', etc. when you see them)\n");
}
