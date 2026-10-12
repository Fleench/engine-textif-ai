# Engine Core (`engine/`)

This directory houses the foundational framework for the interactive fiction engine. It strictly contains **game-agnostic** framework code and the I/O layer. **No demo content or specific game narrative resides here.**

## Architectural Decoupling & Modularity

The `engine/` directory is strictly **self-contained and decoupled**:
- It contains **zero external dependencies** outside its own directory.
- It does not reference or include anything from `game/` or any application-level module.
- It can be copied, referenced, or integrated as a standalone library/submodule in any project or template (`template/engine/`).
- Game implementations depend downward on `engine/`, preserving clean unidirectional dependency flow.

---

## Subsystems and Capabilities

### 1. Global Verb Registration & Dispatch Architecture

The engine employs a centralized verb registration and dynamic dispatch system:

#### Global Verb Registration (`registerVerb`)
New verbs are registered directly onto the `Game` instance via:
```cpp
void registerVerb(const std::string& name, const std::string& defaultMsg);
```
- **Registration**: Associates the verb `name` with a `VerbDef` containing its fallback implementation (`defaultFn`) and registers a canonical identity in `verbSynonyms`.
- **Fallback Execution**: When a player inputs a registered verb with no arguments (e.g. `JUMP`), or targets an object that does not define specific handling for that verb, or when no target object is matched in scope, the engine falls back to executing the registered `defaultFn` (printing `defaultMsg`).
- **Standard Defaults**: Built-in verbs (e.g., `look`, `take`, `drop`, `inventory`, `go`, `open`, `close`, `put`, `talk`, `quit`, `help`, `score`) are registered during engine initialization via `registerDefaultVerbs()`. Custom fallback behaviors can be provided via `overrideVerbDefault(...)`.

#### Strict `supports(game, name, ...)` Requirement
Game entities derived from `GameObject` (such as `Item`, `Door`, `NPC`) attach custom actions using the two-argument or three-argument `supports` methods:
```cpp
void supports(Game& game, const std::string& name);
void supports(Game& game, const std::string& name, VerbFn func);
```
- **Strict Registration Check**: Verifies that `game.verbRegistry.count(name)` is non-zero.
- **Fail-Fast Error**: If a verb is bound to an object before it has been registered globally in `game.verbRegistry`, a `std::runtime_error` is immediately thrown:
  ```cpp
  throw std::runtime_error("Verb '" + name + "' is not registered globally.");
  ```
  This guarantees that phantom verbs, misspellings, or unmapped commands cannot silently contaminate game entity behavior.

---

### 2. Dynamic & Toggleable Help System

The engine provides an introspective, dynamic help system that reflects registered commands:

- **Toggle State**: Controlled via `game.setHelpEnabled(bool)` and queried with `bool game.isHelpEnabled() const`.
- **Dynamic Verb Enumeration**: When enabled (default is `true`), issuing `help` dynamically iterates across all keys registered in `game.verbRegistry` and prints an alphabetically ordered, comma-separated list of available verbs:
  ```text
  Available verbs: ask, attack, brief, close, drop, examine, go, help, inventory, look, open, put, quit, score, superbrief, take, talk, verbose.
  ```
- **Disabling Help**: When `game.setHelpEnabled(false)` is invoked, the `help` command is suppressed. Entering `help` prints the standard unrecognized command response (`"I don't understand that command."`), ideal for puzzle sections or titles that prefer not to expose all verb capabilities upfront.

---

### 3. Advanced Parsing & Disambiguation

- **Tokenization & Normalization**: Strips articles (`the`, `a`, `an`), handles casing, and parses prepositional phrases (`PUT X IN Y`, `LOOK AT X`, `TAKE X FROM Y`).
- **Multi-Word Verbs & Synonyms**: Recognizes multi-token verbs (e.g., `pick up`, `look at`) and maps verb synonyms (`examine` $\to$ `look`, `get` $\to$ `take`).
- **Pronoun Tracking**: Contextually tracks referents:
  - `IT` $\to$ resolves to the last referenced object.
  - `HIM` / `HER` $\to$ resolves to the last referenced NPC.
  - `THEM` $\to$ resolves to the last referenced plural collection.
- **Command History**:
  - `AGAIN` (or `g`): Repeats the previous action.
  - `OOPS`: Corrects the first unknown token of the last rejected command without retyping the entire sentence.
- **Multi-Command Chaining**: Sequences multiple commands split by periods or semicolons (e.g., `TAKE LANTERN. GO NORTH; LOOK`).
- **Disambiguation Prompts**: When input matches multiple objects in scope, prompts the player (`"Which one do you mean?"`) and awaits resolution.

---

### 4. Core World Mechanics

- **State Management**: Tracks rooms, items, definitions, doors, NPCs, storages, global flags, and win conditions (`Game.h`).
- **Environmental Mechanics**:
  - **Darkness & Illumination**: Tracks darkness via `room->isDark` and light sources via `item->isLightSource` and `item->isLit`. Restricts actions in unlit rooms with grue warnings, unless executing darkness-exempt verbs.
  - **Supporter Surfaces & Containers**: `Storage` items function as openable containers or open supporter surfaces (`PUT X ON Y`), updating containment and light propagation accordingly.
- **Player Metrics & Status**:
  - Tracks player `score`, `maxScore`, and total `moves`.
  - Accessible via the `score` verb or customizable status display hooks.

---

### 5. Narrative & Feedback Control

- **Verbosity Modes**: Selectable modes for room navigation:
  - `VERBOSE`: Always print full room descriptions upon entry.
  - `BRIEF`: Print full room description on first visit, name only on subsequent visits.
  - `SUPERBRIEF`: Always print room name only upon entry.
- **Narrator Response Variety**: `addResponse(key, variants)` allows multi-variant pool responses for standard failures (e.g., `"dont_see"`, `"cant_take"`), preventing repetitive narration.

---

### 6. The I/O Abstraction Layer

All user interaction is mediated by `IOInterface` to support seamless multi-target deployment:
- **`NativeIO.h`**: Blocking synchronous standard I/O (`std::cin`/`std::cout`) for native desktop/terminal binaries.
- **`WebIO.h`**: Asynchronous input/output designed for Emscripten WebAssembly, integrating with browser runtimes without locking UI threads.

---

## Bootstrapping & Lifecycle

1. **Instantiate I/O**: Construct the target `IOInterface` implementation (`NativeIO` or `WebIO`).
2. **Instantiate Game**: Initialize the master `Game(&io)` instance.
3. **Register Verbs**: Built-in verbs are registered (`registerDefaultVerbs()`), followed by any custom game verbs via `registerVerb(...)`.
4. **Populate Entities**: Construct entities and bind actions via `supports(game, name, ...)`.
5. **Game Loop**: Drive turns via `game.tick()`, checking `game.isFinished()` or `game.isRunning`.
