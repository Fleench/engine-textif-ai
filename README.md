# SUMMA ARCHITECTONICA: The Text Adventure Engine

**A Dual-Target Text Adventure Architecture**

Welcome to the root of the interactive fiction engine. Herein lies the entire repository, structured according to the principles of orderly design, cross-platform utility, and robust architectural separation. 

This repository provides an exact and truthful foundation for creating text-based worlds that compile securely for both local terminals and modern web browsers.

## Repository Overview

This project is a C++ text adventure/interactive fiction engine designed to be built for both native terminal execution and WebAssembly for browser deployment. The architecture strictly separates the core engine code from the specific game content.

### Directory Structure

1. **`engine/`**: The core, framework, and I/O abstractions. This includes the command parser, I/O interface layer, state management, and all base definitions (now enhanced with dynamic verb dispatch, pronouns, disambiguation, darkness mechanics, and scoring). It contains **no** game-specific data.
2. **`game/`**: The demo content and game implementations. This is where rooms, items, NPCs, minigames, and logic unique to the game instance are implemented. 
3. **`docs/`**: (If present) Technical documentation detailing the engine's subsystems.

## Recent Architectural Upgrades

The engine has been significantly expanded to support advanced interactive fiction features:
- **Advanced Parser Features**: Multi-command chaining (`.` and `;`), pronoun tracking (`IT`, `HIM`, `HER`, `THEM`), and `AGAIN`/`OOPS` commands.
- **Dynamic Verb Dispatch**: A massive Part 3 Verb Dispatch Refactor introducing `verbRegistry` and flexible `supports(name, func)` methodologies.
- **Environmental Mechanics**: Light and darkness tracking (`isDark`, `isLightSource`, `isLit`), as well as supporter surfaces (`PUT X ON Y`).
- **Narrative & Gameplay**: Verbosity modes (`VERBOSE`, `BRIEF`, `SUPERBRIEF`), custom narrator responses, disambiguation prompts, and a comprehensive status bar tracking scoring and moves.

## Architecture

The system distinguishes the unchanging physical rules of the universe (`engine/`) from the objects placed within it (`game/`).

### Initialization
We enforce a robust **Two-Phase Initialization** pattern to eliminate circular dependencies:
1. **Creation Phase**: Instantiate entities into the master registry.
2. **Wiring Phase**: Connect the instantiated pointers (placing items in rooms, linking room exits).

### The I/O Layer
To support dual-target deployment, all interaction is mediated by the `IOInterface`. `NativeIO` handles standard terminal I/O. `WebIO` provides a seamless abstraction for Emscripten, capturing asynchronous input cleanly.

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
