# Engine Core (`engine/`)

This directory houses the foundational framework for the interactive fiction engine. It strictly contains **game-agnostic** framework code and the I/O layer. **No demo content or specific game narrative resides here.**

## Purpose

The engine's responsibility is to provide the systems, scaffolding, and interfaces upon which any text adventure can be built with exact truth and reliable behavior. Its features include:

### Advanced Parsing & Disambiguation
- **Command Parser**: Understands verbs, subjects, and parameters.
- **Pronoun Tracking**: Resolves referents contextually (e.g., `IT`, `HIM`, `HER`, `THEM`).
- **Command History**: Supports `AGAIN` (repeat last command) and `OOPS` (correct a typo in the last command).
- **Multi-Command Chaining**: Players can sequence commands using periods or semicolons (e.g., `TAKE LAMP. GO NORTH`).
- **Disambiguation Prompts**: When the player's target is ambiguous, the parser prompts for clarification before executing.

### Verb Dispatch System
- **Verb Registry**: The engine utilizes a massive Part 3 Verb Dispatch Refactor. The `verbRegistry` efficiently maps input strings to game actions.
- **Dynamic Capabilities**: Items and entities define what actions they support using `supports(name)` and `supports(name, func)`. This encapsulates logic within the object itself rather than a monolithic parser.

### Core Systems & World Mechanics
- **State Management**: Tracks generic flags, rooms, items, NPCs, transitions, and inventories (`Game.h`).
- **Environmental Mechanics**:
  - **Darkness & Lighting**: Rooms and items manage illumination states via `isDark`, `isLightSource`, and `isLit`.
  - **Supporter Surfaces**: Objects can be placed on top of other objects using the `PUT X ON Y` mechanics.
- **Player Metrics**: The engine tracks the player's **Score** and **Moves**, displaying them continuously in a customizable **Status Bar**.

### Narrative & Feedback
- **Verbosity Modes**: The engine supports `VERBOSE`, `BRIEF`, and `SUPERBRIEF` room description modes.
- **Narrator Response Variety**: Allows for custom responses instead of generic default fallbacks.

## The I/O Abstraction Layer

To support both synchronous native terminal execution and asynchronous web execution (via WebAssembly), the engine uses the `IOInterface`:
- **`NativeIO.h`**: Blocking `std::cin` and `std::cout` for standard native builds.
- **`WebIO.h`**: Asynchronous I/O tailored for Emscripten, capturing inputs without blocking the browser thread.

## How it Ties Together

The engine exposes these tools so that the `game/` module can confidently define content. The engine's core bootstrapping process involves:
1. Instantiating the appropriate `IOInterface`.
2. Providing that interface to the master `Game` state object.
3. Supplying the lifecycle hooks (`init()`, `tick()`, `isFinished()`) which the game's executable loop drives.
