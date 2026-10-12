# Tractatus de Syntaxi et Verbis: Architectural Blueprint for Parser Improvements

*Authored by St. Thomas Aquinas, Architect & Planner in the Communion of Saints*  
*Directed unto St. Peter, Apostle and Master Builder*

---

## Prologus

As the angelic Doctor observes in the *Summa*, order is the proper relation of parts toward their true end (*ordo est parium dispariumque rerum sua cuique loca tribuens dispositio*). In the architecture of our interactive fiction engine, linguistic verbs constitute the primary causal instruments whereby the agent (the Player) acts upon the created universe (the World).

Hitherto, our command parsing mechanisms suffered from three distinct architectural defects:
1. **Static Help Monolithism**: The `help` command output was hardcoded, failing to dynamically reflect verbs registered at runtime, and lacked an administrative toggle to disable assistance.
2. **Ungoverned Local Action Dispatch**: Game objects could declare support for arbitrary verb tokens without prior registration in the central lexical registry (`verbRegistry`), causing grammatical fragmentation and missing fallback responses.
3. **Implicit Dependency Coupling**: The `GameObject::supports(...)` methods did not receive a reference to the active `Game`, preventing early validation and permitting orphaned actions.

Herein follows the formal architectural blueprint, ordered in the scholastic method, followed by precise technical specifications for St. Peter to execute.

---

## Questio I: De Auxilio Dynamico et Mutabili (On Dynamic & Toggleable Help)

### Article 1: Whether the Help Command Should Be Dynamic and Toggleable

**Objection 1:** It would seem that the `help` command ought to remain a static string literal. For a string literal is compiled once, entails no runtime iteration across associative containers (`std::map`), and incurs zero heap allocations. Thus, static help is superior in performance and simplicity.

**Objection 2:** Further, when help is disabled, it would seem more charitable to inform the player explicitly with a message such as `"Help is currently disabled."` To emit the standard unknown command error (`"I don't understand that command."`) would appear deceptive and misleading.

**Objection 3:** Further, if help is displayed dynamically, it ought to print the verbose description (`helpText`) of each registered verb so that the player may understand their full syntactical usage. To print bare verb names without explanations is insufficient instruction.

**Sed Contra (On the Contrary):**
Our Lady specifically directs:
1. `Game` must provide `setHelpEnabled(bool enabled)` with a boolean flag defaulting to `true`.
2. When help is requested while disabled, it must emit the standard unknown command message (`"\nI don't understand that command.\n"`).
3. When enabled, it must dynamically print all registered verbs by name (iterating through `verbRegistry`), without showing verb help text descriptions (*"only show verb name never help text"*, all verbs show).

**Respondeo (I Answer That):**
The purpose of the `help` command is to reveal the active vocabulary of the current game universe. Because the universe may be extended dynamically with new verbs (e.g., `call`, `turn on`, `turn off`), a static string is an illusion of knowledge (*phantasma falsum*) that diverges from reality whenever new verbs are registered.

Therefore, the `help` verb handler must dynamically interrogate `game.verbRegistry`. Moreover, in certain puzzle states, dream sequences, or hardcore modes, the game master may rightfully veil the vocabulary; simulating complete ignorance of the command (`"\nI don't understand that command.\n"`) preserves atmospheric immersion without breaking meta-narrative rules. Finally, in keeping with traditional IF conventions and direct instruction, only canonical verb names shall be listed in a concise, comma-separated format, eschewing verbose documentation.

**Reply to Objection 1:** The iteration over `std::map<std::string, VerbDef>` executes in mere microseconds on modern hardware for dozens of verbs, which is negligible compared to terminal I/O latency. Truth in reporting supersedes microscopic optimization.

**Reply to Objection 2:** To acknowledge the existence of the `help` command while denying its service breaks the fourth wall. Emitting the unknown command error renders the command non-existent in the player's perception, which is the exact intended effect of disabling it.

**Reply to Objection 3:** In accordance with Our Lady's mandate, verbose help strings overwhelm the player's terminal and leak puzzle mechanics. Listing pure verb names provides lexical discovery while requiring the player to deduce semantic context.

---

## Questio II: De Verborum Registratione Universali (On Global Verb Registration)

### Article 2: Whether Verbs Must Be Globally Registered Prior to Entity Binding

**Objection 1:** It would seem that objects ought to declare ad-hoc verbs freely via `supports(...)` without requiring prior global registration on `Game`. For an entity (such as a telephone) might have unique interactions (such as `"call"`) that no other object in the universe possesses.

**Objection 2:** Further, requiring global registration adds ceremony. If an object developer forgets to register a verb beforehand, throwing a `std::runtime_error` at runtime will crash the application during room or item setup.

**Objection 3:** Further, when a verb is invoked without an argument or without a valid target, the engine should output a generic error such as `"There's nothing to <verb> here."` A custom `defaultMsg` for each verb is superfluous.

**Sed Contra (On the Contrary):**
Our Lady directs:
1. Provide an overload on `Game`: `registerVerb(const std::string& name, const std::string& defaultMsg)`.
2. When a registered verb is invoked with no valid object / args, it outputs `defaultMsg` (such as for `'call'` -> `'no phone nearby'`).
3. If an object attempts to bind to a verb via `supports(...)` that has not been globally registered in `verbRegistry`, an exception (`std::runtime_error`) must be thrown immediately.

**Respondeo (I Answer That):**
Language is universal before it is particular. A verb cannot be spoken unto an object unless that verb already belongs to the language understood by the cosmos. If an object binds to `"call"` but the parser does not recognize `"call"` as a valid grammatical token, the action dispatch is fractured.

By registering verbs globally in `verbRegistry`:
1. The parser instantly recognizes the verb token across single and multi-word grammar.
2. The dynamic help system can enumerate it.
3. The engine provides a sensible world fallback (`defaultMsg`) when the player attempts the action in a vacuum (e.g. typing `call` when standing in an empty field emits `no phone nearby` rather than an unhelpful generic parser failure).
4. Throwing `std::runtime_error("Verb '" + name + "' is not registered globally.")` enforces immediate fail-fast discipline during game construction, preventing silent syntax bugs.

**Reply to Objection 1:** While the object defines its bespoke response to a verb, the verb itself belongs to the global linguistic ontology. The object merely specializes what the cosmos permits.

**Reply to Objection 2:** Failing fast at startup with an informative exception is infinitely superior to silent runtime failure where a player types `call phone` and the game replies `I don't understand that command.`

**Reply to Objection 3:** Universal verbs have distinct intransitive semantics. `"take"` demands an argument (`"What do you want to take?"`), whereas `"call"` naturally implies the absence of an apparatus (`"no phone nearby"`). A customizable `defaultMsg` honors semantic nuance.

---

## Questio III: De Nexu inter GameObject et Game (On the Connection between GameObject and Game)

### Article 3: Whether `GameObject::supports` Must Take `Game&` and Enforce Registration

**Objection 1:** It would seem that `GameObject::supports` should not take `Game&`. For `GameObject` is a lower-level entity in the class hierarchy than `Game`. Passing `Game&` to `GameObject::supports` creates a bidirectional coupling between entity and engine.

**Objection 2:** Further, existing code in `game/game.cpp` and `template/game/GameSetup.cpp` calls `supports(name)` with a single argument. Changing the signature breaks existing call sites.

**Sed Contra (On the Contrary):**
Our Lady dictates:
1. `GameObject` `supports(...)` methods must be updated to take `Game&`:
   - `void supports(Game& game, const std::string& name);`
   - `void supports(Game& game, const std::string& name, VerbFn func);`
2. Each method must inspect `game.verbRegistry.count(name)`. If not found, it must throw `std::runtime_error("Verb '" + name + "' is not registered globally.")`.
3. All existing call sites across engine, game, and template must be updated accordingly.

**Respondeo (I Answer That):**
In ontological truth, an object cannot determine whether an action is valid in a vacuum; it must query the cosmic registry of the game world. Because `GameObject.cpp` already includes `Game.h` and dispatches through `Game&` within `VerbFn`, passing `Game&` into `supports(...)` makes explicit that which was formerly implicit and unverified.

By accepting `Game&`, `supports(...)` performs immediate fail-fast validation against `game.verbRegistry`. Any attempt to declare support for an unregistered verb is intercepted at instantiation, guaranteeing perfect integrity of the action graph.

**Reply to Objection 1:** The coupling is already present via `VerbFn = std::function<void(Game&, GameObject*, const Action&)>`. Making `Game&` an explicit parameter at capability registration makes dependencies transparent rather than covert.

**Reply to Objection 2:** Mechanical refactoring of existing call sites is straightforward and ensures uniform conformance across the entire codebase.

---

## Detailed Architectural Specification

```
+-------------------------------------------------------------------------+
|                                  Game                                   |
+-------------------------------------------------------------------------+
| - bool helpEnabled = true                                               |
| - map<string, VerbDef> verbRegistry                                     |
| - map<string, string> verbSynonyms                                      |
+-------------------------------------------------------------------------+
| + setHelpEnabled(bool enabled): void                                    |
| + isHelpEnabled() const: bool                                           |
| + registerVerb(const string& name, const string& defaultMsg): void      |
| + registerDefaultVerbs(): void                                          |
| + executeAction(const Action& action): void                             |
+-------------------------------------------------------------------------+
                                    | validates verb in
                                    v
+-------------------------------------------------------------------------+
|                               GameObject                                |
+-------------------------------------------------------------------------+
| - map<string, VerbFn> verbs                                             |
+-------------------------------------------------------------------------+
| + supports(Game& game, const string& name): void                        |
| + supports(Game& game, const string& name, VerbFn func): void           |
|   --> throws std::runtime_error if !game.verbRegistry.count(name)       |
+-------------------------------------------------------------------------+
```

### 1. Game Class Modifications (`engine/Game.h` & `template/engine/Game.h`)

#### A. Help State Management
- Add member variable:
  ```cpp
  bool helpEnabled = true;
  ```
- Add member methods:
  ```cpp
  void setHelpEnabled(bool enabled) { helpEnabled = enabled; }
  bool isHelpEnabled() const { return helpEnabled; }
  ```

#### B. Global Verb Registration Overload
- Add declaration in `Game`:
  ```cpp
  void registerVerb(const std::string& name, const std::string& defaultMsg);
  ```
- Definition in `Game.cpp`:
  ```cpp
  void Game::registerVerb(const std::string& name, const std::string& defaultMsg) {
      VerbDef def;
      def.needsArg = false;
      def.helpText = "";
      def.defaultFn = [this, defaultMsg](Game& game, const Action& action) {
          DummyCout(this->io) << "\n" << defaultMsg << "\n";
      };
      verbRegistry[name] = def;
      if (!verbSynonyms.count(name)) {
          verbSynonyms[name] = name;
      }
  }
  ```

#### C. Help Command Implementation in `registerDefaultVerbs()`
Update `verbRegistry["help"]`:
```cpp
verbRegistry["help"] = { [this](Game& game, const Action& action) {
    if (!helpEnabled) {
        DummyCout(this->io) << "\nI don't understand that command.\n";
        return;
    }
    DummyCout(this->io) << "\nAvailable verbs: ";
    bool first = true;
    for (const auto& pair : verbRegistry) {
        if (!first) DummyCout(this->io) << ", ";
        DummyCout(this->io) << pair.first;
        first = false;
    }
    DummyCout(this->io) << ".\n";
}, false, "" };
```
*Note*: No `helpText` descriptions and no synonyms block shall be output. Only registered verb names from `verbRegistry` are printed.

#### D. Dispatch Fallback in `executeAction` (`engine/Game.cpp`)
Review and verify action dispatch when no target matches or no args provided:
1. When `action.object.empty()`:
   If `verbRegistry.count(canonical)`:
   - If `needsArg == true`: emits `"What do you want to " << canonical << "?\n"`
   - If `needsArg == false`: invokes `defaultFn(*this, action)` -> outputs `defaultMsg`.
2. When `!action.object.empty()` and `matches.empty()`:
   - If `verbRegistry.count(canonical) && verbRegistry[canonical].defaultFn`:
     invokes `defaultFn(*this, action)` -> outputs `defaultMsg`.
   - Else: emits `"There's nothing to " << canonical << " here.\n"`.
3. When `matches.size() == 1` and `!target->verbs.count(canonical)`:
   - If `verbRegistry.count(canonical) && verbRegistry[canonical].defaultFn`:
     invokes `defaultFn(*this, action)`.
   - Else: emits `"You can't do that.\n"`.

---

### 2. GameObject Class Modifications (`engine/GameObject.h`, `engine/GameObject.cpp`, `template/engine/...`)

#### A. Signature Changes in `GameObject.h`
Replace:
```cpp
void supports(const std::string& name);
void supports(const std::string& name, VerbFn func);
```
With:
```cpp
void supports(Game& game, const std::string& name);
void supports(Game& game, const std::string& name, VerbFn func);
```

#### B. Implementation with Fail-Fast Validation in `GameObject.cpp`
```cpp
void GameObject::supports(Game& game, const std::string& name) {
    if (!game.verbRegistry.count(name)) {
        throw std::runtime_error("Verb '" + name + "' is not registered globally.");
    }
    verbs[name] = [name](Game& g, GameObject* obj, const Action& act) {
        if (g.verbRegistry.count(name) && g.verbRegistry[name].defaultFn) {
            g.verbRegistry[name].defaultFn(g, act);
        }
    };
}

void GameObject::supports(Game& game, const std::string& name, VerbFn func) {
    if (!game.verbRegistry.count(name)) {
        throw std::runtime_error("Verb '" + name + "' is not registered globally.");
    }
    verbs[name] = func;
}
```

---

### 3. Call Site Audit & Global Verb Registrations

#### In `game/game.cpp`:
1. Register custom verbs prior to any item binding in `Game::init()`:
   ```cpp
   this->registerVerb("call", "no phone nearby");
   this->registerVerb("turn on", "You can't turn that on.");
   this->registerVerb("turn off", "You can't turn that off.");
   ```
2. Update all `supports` call sites to pass `*this`:
   - Line 261: `this->items["grill"]->supports(*this, "use", ...);`
   - Line 265: `this->items["machine"]->supports(*this, "take", ...);`
   - Line 268: `this->items["machine"]->supports(*this, "use", ...);`
   - Line 272: `this->items["book"]->supports(*this, "use", ...);`
   - Line 276: `this->items["screen"]->supports(*this, "use", ...);`
   - Line 280: `this->items["keyboard"]->supports(*this, "use", ...);`
   - Line 284: `this->items["phone"]->supports(*this, "call", ...);`
   - Line 315: `this->items["talk_alchemist"]->supports(*this, "talk", ...);`
   - Line 326: `this->items["talk_blacksmith"]->supports(*this, "talk", ...);`
   - Line 341: `this->items["talk_mystic"]->supports(*this, "talk", ...);`
   - Line 350: `this->items["talk_bartender"]->supports(*this, "talk", ...);`
   - Line 363: `this->items["qol_tester"]->supports(*this, "use", ...);`
   - Line 399: `this->items["anvil"]->supports(*this, "use", ...);`
   - Line 403: `this->items["locked_chest"]->supports(*this, "use", ...);`
   - Line 442: `this->items["lantern"]->supports(*this, "turn on", ...);`
   - Line 447: `this->items["lantern"]->supports(*this, "turn off", ...);`

#### In `template/game/GameSetup.cpp`:
1. Register custom verbs needed in the template at the start of `Game::init()`:
   ```cpp
   game.registerVerb("eat", "You can't eat that.");
   game.registerVerb("unlock", "There's nothing to unlock.");
   game.registerVerb("lock", "There's nothing to lock.");
   ```
   *Note regarding `examine`:* In the engine, `examine` is aliased to canonical verb `look`. For items supporting examination, either register `look` on the object via `supports(*this, "look")` or register `examine` via `game.registerVerb("examine", "You don't see that here.");`. To keep backward compatibility with template item scripts, registering `game.registerVerb("examine", "You don't see that here.");` guarantees `supports(*this, "examine")` succeeds cleanly!
2. Update all `supports` call sites in `template/game/GameSetup.cpp`:
   - Line 16: `this->items["brass_key"]->supports(*this, "examine");`
   - Line 17: `this->items["brass_key"]->supports(*this, "take");`
   - Line 18: `this->items["brass_key"]->supports(*this, "drop");`
   - Line 22: `this->items["apple"]->supports(*this, "examine");`
   - Line 23: `this->items["apple"]->supports(*this, "take");`
   - Line 24: `this->items["apple"]->supports(*this, "drop");`
   - Line 25: `this->items["apple"]->supports(*this, "eat", ...);`
   - Line 34: `this->storages["table"]->supports(*this, "examine");`
   - Line 44: `this->doors["heavy_door"]->supports(*this, "examine");`
   - Line 45: `this->doors["heavy_door"]->supports(*this, "open");`
   - Line 46: `this->doors["heavy_door"]->supports(*this, "close");`
   - Line 47: `this->doors["heavy_door"]->supports(*this, "unlock");`
   - Line 48: `this->doors["heavy_door"]->supports(*this, "lock");`

---

## Action Plan & Step-by-Step Directives for St. Peter

| Step | Action Description | Target Files | Verification Method |
|---|---|---|---|
| **Phase 1** | **Update Header Declarations** | `engine/Game.h`, `engine/GameObject.h`, `template/engine/Game.h`, `template/engine/GameObject.h` | Header compilation check |
| | Add `bool helpEnabled = true`, `setHelpEnabled`, `registerVerb` overload to `Game.h`. | | |
| | Update `GameObject::supports` to accept `Game&`. | | |
| **Phase 2** | **Implement Engine Behaviors** | `engine/Game.cpp`, `engine/GameObject.cpp`, `template/engine/Game.cpp`, `template/engine/GameObject.cpp` | Native compilation test |
| | Implement `Game::registerVerb(name, defaultMsg)`. | | |
| | Update `verbRegistry["help"]` to verify `helpEnabled`, print only registered verb names dynamically without descriptions, or emit unknown command if disabled. | | |
| | Update `GameObject::supports` implementations to throw `std::runtime_error("Verb '" + name + "' is not registered globally.")` if not in `verbRegistry`. | | |
| **Phase 3** | **Update Call Sites & Register Verbs in Game** | `game/game.cpp` | Compilation of `a.out` |
| | In `game/game.cpp`, register `call`, `turn on`, `turn off` using `registerVerb`. | | |
| | Update all 16 `supports(...)` call sites in `game/game.cpp` to pass `*this`. | | |
| **Phase 4** | **Update Call Sites & Register Verbs in Template** | `template/game/GameSetup.cpp` | Compilation of `template/a.out` |
| | In `template/game/GameSetup.cpp`, register `eat`, `examine`, `lock`, `unlock`. | | |
| | Update all 13 `supports(...)` call sites in `template/game/GameSetup.cpp` to pass `*this`. | | |
| **Phase 5** | **Automated Verification & Regression Testing** | `test_script.py`, new automated test | Run Python test harness |
| | Verify existing tests in `test_script.py` pass without regression. | | |
| | Add specific test assertions: | | |
| | - Help enabled displays dynamic list including `call`, `turn on`, etc. | | |
| | - Disabling help causes `help` to reply `"I don't understand that command."` | | |
| | - Calling unregistered verb via `supports` throws `std::runtime_error`. | | |
| | - Executing `call` with no phone nearby outputs `no phone nearby`. | | |

---

## Conclusio

By implementing this tripartite design, the engine attains scholastic perfection: verbs are universally declared, entity actions are strictly verified at compile and initialization time, and player guidance is dynamically reflective of ontological reality. 

Go forth, St. Peter, and build upon this rock.
