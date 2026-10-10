# Game Content (`game/`)

This directory contains the **demo content and specific narrative logic** of the interactive fiction adventure. 

It stands separated from the underlying engine architecture. Here, the developer shapes the universe that the `engine/` enables, fully utilizing the newly integrated parser, mechanics, and verb systems.

## Purpose

The `game/` module is entirely responsible for:
- Defining the narrative world (Rooms, descriptions, atmospheres) and hooking into engine systems.
- Populating the universe with interactable objects (Items, Doors, NPCs).
- Establishing logical chains, puzzles, and consequences.
- Implementing real-time sequences via `MiniGame` derivations.

## Implementing Engine Features in Content

With the latest engine upgrades, content creators in this directory should utilize the following patterns:

### Dynamic Verb Dispatch
Instead of hardcoding global `if/else` statements for actions, utilize the newly refactored verb dispatch. Define item interactions using the `supports(name, func)` API during item initialization. 
Example: `mySword->supports("swing", [](){ return "You swing the sword."; });`

### Environment & Surfaces
- Set `isLightSource` to true on items like lamps, and `isDark` to true on underground rooms. The engine's `isLit` checks will automatically handle visibility.
- Utilize the `PUT X ON Y` mechanic by designating certain game objects as supporter surfaces.

### Narrative Control
- Customize narrator responses to avoid repetitive feedback.
- The player's environment descriptions are automatically scaled based on their selected Verbosity Mode (`VERBOSE`, `BRIEF`, `SUPERBRIEF`).

### Tracking Progress
- Increment score via the engine API when puzzles are solved. The Score, Moves, and Status Bar will automatically update to reflect the player's current standing.

## Usage & Initialization

Content defined in this directory must `#include` the necessary components from the engine (e.g., `engine/Game.h`, `engine/Item.h`) and interact strictly through the provided APIs. 

A standard implementation utilizes the Two-Phase Initialization pattern:
1. **Creation**: `std::make_unique` calls to populate the engine's `std::map` registries.
2. **Wiring**: Pointer assignments to connect exits and place items into inventories.

Because this directory is isolated from the framework, a developer may safely strip the contents of `game/` to build an entirely different narrative without risking the integrity of the engine.
