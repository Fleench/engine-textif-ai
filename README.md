# SUMMA ARCHITECTONICA: The Text Adventure Engine

**A Dual-Target Text Adventure Architecture**

Welcome to the root of the interactive fiction engine. Herein lies the entire repository, structured according to the principles of orderly design, cross-platform utility, and robust architectural separation. 

This repository provides an exact and truthful foundation for creating text-based worlds that compile securely for both local terminals and modern web browsers.

## Repository Overview

This project is a C++ text adventure/interactive fiction engine designed to be built for both native terminal execution and WebAssembly for browser deployment. The architecture strictly separates the core engine code from the specific game content.

### Directory Structure

1. **`engine/`**: The core, framework, and I/O abstractions. This directory is **completely self-contained and decoupled**, containing zero dependencies outside its own folder. It includes the command parser, global verb registration, dynamic dispatch system, I/O interfaces, state management, and base definitions. It contains **no** game-specific narrative or demo data.
2. **`game/`**: The demo content and game implementations. This is where rooms, items, NPCs, minigames, and logic unique to the game instance are implemented.
3. **`template/`**: Clean starting templates (such as `template/engine/`) for creating new standalone interactive fiction games.
4. **`docs/`**: Technical documentation detailing the engine's subsystems and architecture.

## Recent Architectural Upgrades

The engine has been significantly expanded with robust parser, dispatch, and narrative capabilities:

- **Decoupled Engine Core**: The `engine/` directory has zero external dependencies, allowing it to be compiled or embedded independently.
- **Global Verb Registration & Dynamic Dispatch**:
  - Centralized registration via `game.registerVerb(name, defaultMsg)` with automated fallback execution when commands lack arguments or targets don't override behavior.
  - Strict contract enforcement: calls to `supports(game, name)` and `supports(game, name, func)` on game entities require the verb to already exist in `game.verbRegistry`, throwing a `std::runtime_error` immediately if unregistered.
- **Dynamic & Toggleable Help**:
  - Toggleable via `game.setHelpEnabled(bool)` and `game.isHelpEnabled()`.
  - Dynamically inspects and lists all registered verbs in `game.verbRegistry` upon typing `help` (or prints standard unrecognized command response when disabled).
- **Advanced Parser Features**: Multi-command chaining (`.` and `;`), pronoun tracking (`IT`, `HIM`, `HER`, `THEM`), and command history (`AGAIN` and `OOPS`).
- **Environmental Mechanics**: Light and darkness tracking (`isDark`, `isLightSource`, `isLit`), as well as supporter surfaces (`PUT X ON Y`).
- **Narrative & Gameplay**: Verbosity modes (`VERBOSE`, `BRIEF`, `SUPERBRIEF`), custom narrator responses, disambiguation prompts, and a comprehensive status bar tracking scoring and moves.

## Architecture

The system distinguishes the unchanging physical rules of the universe (`engine/`) from the objects placed within it (`game/`).

### Initialization
We enforce a robust **Two-Phase Initialization** pattern to eliminate circular dependencies:
1. **Creation Phase**: Instantiate entities into the master registry.
2. **Wiring Phase**: Connect the instantiated pointers (placing items in rooms, linking room exits).

### Global Verb Dispatch Lifecycle
1. Verbs are registered globally on the `Game` instance (`game.registerVerb(...)`).
2. Entities declare supported actions via `object.supports(game, verbName, handler)`.
3. The parser tokenizes user input and matches targets against the current scope.
4. If an object is found, its specific handler executes; otherwise, the registered global default fallback runs.

### The I/O Layer
To support dual-target deployment, all interaction is mediated by the `IOInterface`:
- **`NativeIO`**: Blocking terminal I/O via standard streams.
- **`WebIO`**: Asynchronous event-driven I/O for Emscripten in the browser.

### MiniGames
A core feature of the engine is the seamless transition between standard turn-based text adventure parsing and real-time interactive sequences called **MiniGames**. Control passes into real-time loops that utilize terminal clearing and framerate control, and restores smoothly upon completion.

## Building the Project

The project employs a dual-target build system utilizing `make`.

### Native Build (Terminal)
```bash
make
```
Produces an executable utilizing standard synchronous `cin`/`cout` logic.

### WebAssembly Build (Browser)
```bash
make wasm
```
Produces the `.wasm` and `.js` bindings to run the engine in a browser, relying on the `WebIO` bridge to handle input asynchronously.
