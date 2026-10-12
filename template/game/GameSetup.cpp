#include "GameSetup.h"

void Game::init() {
    Game& game = *this;

    // Register template verbs
    game.registerVerb("eat", "You can't eat that.");
    game.registerVerb("unlock", "There's nothing to unlock.");
    game.registerVerb("lock", "There's nothing to lock.");
    game.registerVerb("examine", "You don't see that here.");

    // Define basic responses
    game.addResponse("cant_go", {"You bump into a wall.", "The path is blocked.", "You cannot go that way."});
    game.addResponse("dont_see", {"You don't see that here.", "There's no such thing here."});

    // Define Items
    this->itemDefs["brass_key"] = std::make_unique<ItemDef>("brass key", "A small, slightly tarnished brass key.");
    this->itemDefs["brass_key"]->onTake = [](Game& g, Item*) {
        g.setFlag("has_brass_key", true);
    };
    this->items["brass_key"] = std::make_unique<Item>("brass_key", this->itemDefs["brass_key"].get());
    this->items["brass_key"]->supports(*this, "examine");
    this->items["brass_key"]->supports(*this, "take");
    this->items["brass_key"]->supports(*this, "drop");

    this->itemDefs["apple"] = std::make_unique<ItemDef>("apple", "A shiny red apple.");
    this->items["apple"] = std::make_unique<Item>("apple", this->itemDefs["apple"].get());
    this->items["apple"]->supports(*this, "examine");
    this->items["apple"]->supports(*this, "take");
    this->items["apple"]->supports(*this, "drop");
    this->items["apple"]->supports(*this, "eat", [](Game& g, GameObject* obj, const Action& a) {
        g.io->print("You eat the apple. It's crisp and sweet.\n");
        g.removeInvItem("apple");
    });

    // Define a Supporter (Table)
    this->itemDefs["table"] = std::make_unique<ItemDef>("wooden table", "A sturdy oak table.");
    this->storages["table"] = std::make_unique<Storage>("table", this->itemDefs["table"].get());
    this->storages["table"]->isSupporter = true;
    this->storages["table"]->supports(*this, "examine");
    // Place the key and apple on the table
    this->storages["table"]->items.push_back(this->items["brass_key"].get());
    this->storages["table"]->items.push_back(this->items["apple"].get());

    // Define a Door
    this->doors["heavy_door"] = std::make_unique<Door>("heavy_door", "Heavy Oak Door");
    this->doors["heavy_door"]->setDescription("A thick wooden door with a sturdy brass lock.");
    this->doors["heavy_door"]->needs("has_brass_key");
    this->doors["heavy_door"]->denyMessage("The heavy oak door is securely locked.");
    this->doors["heavy_door"]->supports(*this, "examine");
    this->doors["heavy_door"]->supports(*this, "open");
    this->doors["heavy_door"]->supports(*this, "close");
    this->doors["heavy_door"]->supports(*this, "unlock");
    this->doors["heavy_door"]->supports(*this, "lock");

    // Define Rooms
    this->rooms["start_room"] = std::make_unique<Room>("start_room", "Starting Room");
    this->rooms["start_room"]->setDescription("You are in a dimly lit stone chamber. A heavy oak door leads north.");
    this->rooms["start_room"]->setShortDescription("A dimly lit chamber.");
    this->rooms["start_room"]->addItem(this->storages["table"].get());

    this->rooms["treasure_room"] = std::make_unique<Room>("treasure_room", "Treasure Room");
    this->rooms["treasure_room"]->setDescription("You've found the treasure room! It's filled with sparkling gold and jewels.");
    this->rooms["treasure_room"]->setShortDescription("The treasure room.");

    // Connect Rooms
    this->rooms["start_room"]->addExit("north", this->rooms["treasure_room"].get(), this->doors["heavy_door"].get());
    this->rooms["treasure_room"]->addExit("south", this->rooms["start_room"].get(), this->doors["heavy_door"].get());

    // Set starting state
    this->currentRoom = this->rooms["start_room"].get();
}
