#ifndef GAME_H
#define GAME_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <functional>
#include <set>
#include <random>
#include "IOInterface.h"
#include "GameObject.h"
#include "Item.h"
#include "Door.h"
#include "Room.h"
#include "NPC.h"
#include "MiniGame.h"
#include "Parser.h"
#include "SaveLoad.h"

class Game {
public:
    std::map<std::string, std::unique_ptr<Room>> rooms;
    std::map<std::string, std::unique_ptr<ItemDef>> itemDefs;
    std::map<std::string, std::unique_ptr<Item>> items;
    std::map<std::string, std::unique_ptr<Door>> doors;
    std::map<std::string, std::unique_ptr<NPC>> npcs;
    std::map<std::string, std::unique_ptr<Storage>> storages;
    std::map<std::string, std::unique_ptr<MiniGame>> miniGames;
    
    std::vector<Item*> inventory;
    Room* currentRoom = nullptr;
    std::map<std::string, bool> globalFlags;
    std::function<bool(Game&)> winCondition;
    
    struct DialogueOption {
        std::string text;
        std::string targetNode;
    };
    std::string activeDialogueNode;
    std::vector<DialogueOption> currentDialogueOptions;
    bool inDialogue = false;
    
    bool isRunning = true;

    
    struct VerbDef {
        std::function<void(Game&, const Action&)> defaultFn;
        bool needsArg = false;
        std::string helpText;
    };
    std::map<std::string, VerbDef> verbRegistry;
    std::map<std::string, std::string> verbSynonyms;
    bool helpEnabled = true;
    void setHelpEnabled(bool enabled);
    bool isHelpEnabled() const;
    void registerVerb(const std::string& name, const std::string& defaultMsg);
    void registerDefaultVerbs();
    void overrideVerbDefault(const std::string& verbName, std::function<void(Game&, const Action&)> fn);
    
    std::map<std::string, Preposition> prepMap;
    std::map<std::string, std::string> objectSynonyms;
    
    // F-10 Darkness
    std::set<std::string> darknessAllowedVerbs;
    bool isLit() const;
    
    // F-11 Narrator response variety
    std::map<std::string, std::vector<std::string>> responses;
    std::map<std::string, int> lastResponseIdx;
    void addResponse(const std::string& key, const std::vector<std::string>& variants);
    std::string respond(const std::string& key);

    IOInterface* io;
    Game(IOInterface* io);

    enum Verbosity { BRIEF, VERBOSE, SUPERBRIEF };
    Verbosity verbosityMode = BRIEF;
    int score = 0;
    int maxScore = 0;
    int moves = 0;

    void addScore(int points);
    void setMaxScore(int points);

    // F-4 Pronouns
    std::string lastObject;
    std::string lastNPC;
    std::string lastPlural;

    // F-5 AGAIN
    std::string lastCommand;

    // F-8 OOPS
    std::string lastFailedCommand;
    int firstUnknownWordIndex = -1;

    // F-6 Disambiguation
    std::string pendingDisambiguationTarget;
    std::vector<std::string> pendingDisambiguationMatches;
    Action pendingDisambiguationAction;
    std::string disambiguationSlot; // "object" or "object2"

    bool hasFlag(const std::string& flag) const;

    // 1. State and Flag Management
    void setFlag(const std::string& flagName, bool value);
    bool getFlag(const std::string& flagName) const;

    // 2. Inventory Management
    bool giveItemToPlayer(const std::string& itemId);
    bool removeInvItem(const std::string& itemId);
    bool playerHasItem(const std::string& itemId) const;

    // 3. Spatial and Entity Manipulation
    bool spawnItem(const std::string& itemId, const std::string& roomId);
    bool movePlayerTo(const std::string& roomId);
    bool moveItem(const std::string& itemId, const std::string& toRoomId);

    // 4. Temporal Mechanisms
    struct TimerInfo { int remaining; std::string callbackId; bool paused; };
    std::map<std::string, TimerInfo> timers;
    void createTimer(int seconds, const std::string& callbackId);
    void cancelTimer(const std::string& timerId);
    void pauseTimer(const std::string& timerId);
    void resumeTimer(const std::string& timerId);

    // 5. Reactive Hooks
    std::map<std::string, std::vector<std::function<void()>>> eventHooks;
    void registerEventHook(const std::string& eventName, std::function<void()> hook);
    void triggerEvent(const std::string& eventName);
    void clearEventHooks(const std::string& eventName);

    // 6. Conversational Logistics
    void startDialogueTree(const std::string& dialogueId);
    void addDialogueOption(const std::string& text, const std::string& targetNode);
    void endDialogue();
    bool hasSpokenTo(const std::string& npcId);

    // 7. Chromatic and Typographic Utilities
    void printColored(const std::string& text, int color);
    void printBold(const std::string& text);
    void printItalic(const std::string& text);
    void clearScreen();
    void typewriterEffect(const std::string& text, int msPerChar);

    // 8. Auditory and Sensory Implements
    void playAudio(const std::string& filepath);
    void stopAudio();
    void setAudioVolume(float volume);
    void loopAudio(const std::string& filepath);
    void shakeScreen(int intensity, int duration);
    void flashScreen(int color);

    // 9. State Preservation
    void saveGame(const std::string& slotName);
    void loadGame(const std::string& slotName);
    void deleteSave(const std::string& slotName);

    Action parseInput(const std::string& input);
    void executeAction(const Action& action);
    void processCommand(const std::string& input);

    void init();
    void tick();
    bool isFinished() const;

    void enterRealTimeMode();
    void exitRealTimeMode();

    void save(const std::string& filename);
    void load(const std::string& filename);
};

#endif
