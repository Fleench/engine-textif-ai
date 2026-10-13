#pragma once
#include <string_view>

/**
 * ==============================================================================
 * FICTIVE-AI EMBEDDED TEMPLATES
 * Apostle Paul, Tentmaker & Tooling Engineer
 * Communion of Saints Framework
 * ==============================================================================
 * Self-contained embedded assets ensuring fictive-ai operates autonomously
 * even in air-gapped environments without templates installed on disk.
 */

namespace fictive::cli {

inline constexpr std::string_view EMBEDDED_MAIN_CPP = R"FICTIVE_RAW(/**
 * ==============================================================================
 * FICTIVE-AI STARTER TEMPLATE
 * Blessed Fra Angelico — Sacred Aesthetics & Harmonious Illumination
 * ==============================================================================
 * Minimalist, starter game template in C++17 demonstrating:
 *   - Zero-dependency single-header engine architecture
 *   - Unified stream I/O formatting (print << styles.cyan << ...)
 *   - Interactive loop driven by (input >> line)
 *   - Syntactic parsing via parse(line) with AST queries
 *   - Development debug diagnostics (debug.run, debug <<) elided in production (-p)
 *   - Clean room navigation, inventory manipulation, and object inspection
 *
 * Compile:
 *   Dev:  g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -I. templates/main.cpp -o game
 *   Prod: g++ -std=c++17 -DPRODUCTION -O3 -I. templates/main.cpp -o game_prod
 * ==============================================================================
 */

#include <fictive-ai/parser.hpp>
#include <fictive-ai/io.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

using namespace fictive;

// ==============================================================================
// World State & Data Structures
// ==============================================================================

enum class RoomId {
    Scriptorium,
    Garden,
    Library
};

struct Room {
    std::string name;
    std::string description;
    std::map<std::string, RoomId> exits;
    std::vector<std::string> items;
};

// ==============================================================================
// Game Engine Instance
// ==============================================================================

class StarterGame {
private:
    RoomId currentRoomId{RoomId::Scriptorium};
    std::vector<std::string> inventory;
    std::map<RoomId, Room> world;
    std::map<std::string, std::string> itemDetails;
    bool running{true};

    void initializeWorld() {
        // 1. Scriptorium
        world[RoomId::Scriptorium] = Room{
            "The Scriptorium of San Marco",
            "Morning sunlight filters through arched lancet windows, illuminating flecks "
            "of lapis lazuli and gold leaf on an oak writing desk. To the north lies the "
            "cloister garden; to the east opens the vaulted library.",
            {{"north", RoomId::Garden}, {"east", RoomId::Library}},
            {"manuscript", "pigment"}
        };

        // 2. Cloister Garden
        world[RoomId::Garden] = Room{
            "The Cloister Garden",
            "A peaceful courtyard surrounded by slender stone arcades. A marble fountain "
            "whispers soft praises at the center, its cool water reflecting the azure sky. "
            "To the south, the scriptorium invites quiet contemplation.",
            {{"south", RoomId::Scriptorium}},
            {"lily", "key"}
        };

        // 3. Vaulted Library
        world[RoomId::Library] = Room{
            "The Vaulted Library",
            "Towering cedar shelves rise toward ribbed stone vaults, bearing ancient folios "
            "and illuminated commentaries. The scent of dried parchment and beeswax fills "
            "the stillness. The scriptorium lies to the west.",
            {{"west", RoomId::Scriptorium}},
            {"codex"}
        };

        // Item Lore Descriptions
        itemDetails["manuscript"] = "A vellum leaf painted with radiant vermilion initials and delicate acanthus leaves.";
        itemDetails["pigment"]    = "A glass phial filled with luminous ultramarine pigment ground from raw lapis lazuli.";
        itemDetails["lily"]       = "A pure white lily freshly gathered from the cloister garden, fragrant and untainted.";
        itemDetails["key"]        = "A heavy brass key, engraved with a small cross upon its bow.";
        itemDetails["codex"]      = "A calfskin-bound volume containing polyphonic Gregorian hymns and sacred verse.";
    }

public:
    StarterGame() {
        initializeWorld();
    }

    void showWelcome() {
        print << styles.brightCyan << styles.bold
              << "\n=======================================================\n"
              << "       FICTIVE-AI : THE CLOISTER OF SAN MARCO\n"
              << "=======================================================\n"
              << styles.reset;
        print << styles.dim
              << "A starter interactive fiction created in the Communion of Saints.\n"
              << "Type " << styles.yellow << "help" << styles.reset << styles.dim
              << " for guidance, or " << styles.yellow << "look" << styles.reset << styles.dim
              << " to inspect your surroundings.\n\n"
              << styles.reset;
        describeCurrentRoom();
    }

    void describeCurrentRoom() {
        const auto& room = world[currentRoomId];

        print << styles.bold << styles.brightWhite << "[" << room.name << "]\n" << styles.reset;
        print << room.description << "\n";

        // Display Ground Items
        if (!room.items.empty()) {
            print << styles.green << "On the ground you notice: " << styles.reset;
            for (size_t i = 0; i < room.items.size(); ++i) {
                print << styles.brightYellow << room.items[i] << styles.reset;
                if (i + 1 < room.items.size()) print << ", ";
            }
            print << "\n";
        }

        // Display Exits
        print << styles.cyan << "Exits: " << styles.reset;
        size_t exitCount = 0;
        for (const auto& [dir, _] : room.exits) {
            print << styles.brightCyan << dir << styles.reset;
            if (++exitCount < room.exits.size()) print << ", ";
        }
        print << "\n";
    }

    void handleMovement(const std::string& direction) {
        auto& room = world[currentRoomId];
        auto it = room.exits.find(direction);
        if (it != room.exits.end()) {
            currentRoomId = it->second;
            print << styles.dim << "You walk toward the " << direction << "...\n\n" << styles.reset;
            describeCurrentRoom();
        } else {
            print << styles.red << "There is no passage leading " << direction << ".\n" << styles.reset;
        }
    }

    void handleLook() {
        describeCurrentRoom();
    }

    void handleExamine(const ParsedCommand& cmd) {
        if (!cmd.hasDirectObject()) {
            print << styles.yellow << "What would you like to examine?\n" << styles.reset;
            return;
        }

        const std::string& target = cmd.getDirectObject()->head;

        // Check Inventory
        auto invIt = std::find(inventory.begin(), inventory.end(), target);
        if (invIt != inventory.end()) {
            auto loreIt = itemDetails.find(target);
            if (loreIt != itemDetails.end()) {
                print << styles.brightWhite << loreIt->second << "\n" << styles.reset;
            } else {
                print << "You see nothing unusual about the " << target << ".\n";
            }
            return;
        }

        // Check Current Room
        const auto& room = world[currentRoomId];
        auto roomIt = std::find(room.items.begin(), room.items.end(), target);
        if (roomIt != room.items.end()) {
            auto loreIt = itemDetails.find(target);
            if (loreIt != itemDetails.end()) {
                print << styles.brightWhite << loreIt->second << "\n" << styles.reset;
            } else {
                print << "You see nothing unusual about the " << target << ".\n";
            }
            return;
        }

        print << styles.dim << "You do not see any '" << target << "' here.\n" << styles.reset;
    }

    void handleTake(const ParsedCommand& cmd) {
        if (!cmd.hasDirectObject()) {
            print << styles.yellow << "What would you like to take?\n" << styles.reset;
            return;
        }

        const std::string& target = cmd.getDirectObject()->head;
        auto& room = world[currentRoomId];
        auto it = std::find(room.items.begin(), room.items.end(), target);

        if (it != room.items.end()) {
            inventory.push_back(*it);
            room.items.erase(it);
            print << styles.green << "Taken." << styles.reset
                  << " You place the " << styles.brightYellow << target << styles.reset << " into your satchel.\n";
        } else {
            print << styles.dim << "There is no '" << target << "' here to take.\n" << styles.reset;
        }
    }

    void handleDrop(const ParsedCommand& cmd) {
        if (!cmd.hasDirectObject()) {
            print << styles.yellow << "What would you like to drop?\n" << styles.reset;
            return;
        }

        const std::string& target = cmd.getDirectObject()->head;
        auto it = std::find(inventory.begin(), inventory.end(), target);

        if (it != inventory.end()) {
            world[currentRoomId].items.push_back(*it);
            inventory.erase(it);
            print << styles.green << "Dropped." << styles.reset
                  << " You set down the " << styles.brightYellow << target << styles.reset << ".\n";
        } else {
            print << styles.yellow << "You are not carrying any '" << target << "'.\n" << styles.reset;
        }
    }

    void handleInventory() {
        if (inventory.empty()) {
            print << styles.dim << "Your satchel is empty.\n" << styles.reset;
            return;
        }

        print << styles.cyan << "You are currently carrying:\n" << styles.reset;
        for (const auto& item : inventory) {
            print << "  - " << styles.brightYellow << item << styles.reset;
            auto it = itemDetails.find(item);
            if (it != itemDetails.end()) {
                print << styles.dim << " (" << it->second << ")" << styles.reset;
            }
            print << "\n";
        }
    }

    void handleHelp() {
        print << styles.brightCyan << "\n--- Available Actions ---\n" << styles.reset;
        print << "  " << styles.bold << "Movement:  " << styles.reset << "north, south, east, west (or n, s, e, w)\n";
        print << "  " << styles.bold << "Observe:   " << styles.reset << "look (l), examine <item> (x <item>)\n";
        print << "  " << styles.bold << "Interact:  " << styles.reset << "take <item>, drop <item>, inventory (i)\n";
        print << "  " << styles.bold << "Session:   " << styles.reset << "help, quit / exit\n\n";
    }

    void runLoop() {
        showWelcome();

        std::string line;

        // Interactive Player Command Loop
        while (running) {
            print << "\n" << styles.bold << styles.brightCyan << "> " << styles.reset;

            if (!(input >> line)) {
                // Stream EOF reached
                break;
            }

            // Syntactic Parse via Fictive-AI
            auto cmd = parse(line);

            // Development Debug Diagnostics (Zero-cost when compiled with -DPRODUCTION)
            debug.run([&]{
                debug << styles.gray << "[DEBUG] Raw: '" << line << "' -> Valid: "
                      << (cmd.valid ? "true" : "false") << ", Verb: '" << cmd.verb << "'";
                if (cmd.hasDirectObject()) {
                    debug << ", Head Noun: '" << cmd.getDirectObject()->head << "'";
                }
                debug << styles.reset << "\n";
            });

            if (!cmd.valid) {
                print << styles.dim << "I do not comprehend that command. Type "
                      << styles.yellow << "help" << styles.reset << styles.dim
                      << " for ideas.\n" << styles.reset;
                continue;
            }

            // Directional Movement Navigation
            if (cmd.isDirection()) {
                handleMovement(cmd.getDirection());
                continue;
            }

            if (cmd.verb == "go") {
                if (cmd.hasDirectObject()) {
                    handleMovement(cmd.getDirectObject()->head);
                } else {
                    print << styles.yellow << "Which direction do you wish to go?\n" << styles.reset;
                }
                continue;
            }

            // Canonical Command Dispatch
            if (cmd.verb == "look" || cmd.verb == "l") {
                handleLook();
            } else if (cmd.verb == "examine" || cmd.verb == "x" || cmd.verb == "read") {
                handleExamine(cmd);
            } else if (cmd.verb == "take" || cmd.verb == "get") {
                handleTake(cmd);
            } else if (cmd.verb == "drop") {
                handleDrop(cmd);
            } else if (cmd.verb == "inventory" || cmd.verb == "i") {
                handleInventory();
            } else if (cmd.verb == "help") {
                handleHelp();
            } else if (cmd.verb == "quit" || cmd.verb == "exit") {
                print << styles.brightMagenta << "\nMay peace and grace go with you on your journey. Farewell.\n" << styles.reset;
                running = false;
            } else {
                print << styles.dim << "You ponder '" << cmd.rawVerb
                      << "', but find no fitting way to do that here.\n" << styles.reset;
            }
        }
    }
};

// ==============================================================================
// Entry Point
// ==============================================================================

int main() {
    StarterGame game;
    game.runLoop();
    return 0;
}
)FICTIVE_RAW";

inline constexpr std::string_view EMBEDDED_STYLE_CSS = R"FICTIVE_RAW(/**
 * ==============================================================================
 * FICTIVE-AI UNIVERSAL TERMINAL THEME
 * Blessed Fra Angelico — Sacred Aesthetics & Harmonious Illumination
 * ==============================================================================
 * Universal CSS stylesheet for WebAssembly (WASM) and Desktop Webview windows.
 * Built for contemplation, clarity, retro CRT elegance, and responsive beauty.
 */

/* ------------------------------------------------------------------------------
 * 1. DESIGN TOKENS & CSS CUSTOM PROPERTIES
 * ------------------------------------------------------------------------------ */
:root {
    color-scheme: dark;

    /* Core Canvas & Surface Geometry */
    --bg-color: #0b0e14;
    --bg-surface: #111622;
    --bg-surface-elevated: #161d2d;
    --border-color: #1e293b;
    --border-subtle: #172033;
    --selection-bg: rgba(56, 189, 248, 0.28);
    --selection-text: #ffffff;

    /* Primary Typography & Radiant Prompt */
    --text-color: #e2e8f0;
    --text-muted: #8492a6;
    --text-dimmed: #526075;
    --prompt-color: #38bdf8;
    --prompt-glow: rgba(56, 189, 248, 0.45);
    --cursor-color: #38bdf8;

    /* Standard ANSI Palette */
    --color-black: #181c24;
    --color-red: #ef4444;
    --color-green: #22c55e;
    --color-yellow: #eab308;
    --color-blue: #3b82f6;
    --color-magenta: #a855f7;
    --color-cyan: #06b6d4;
    --color-white: #f8fafc;

    /* High-Vibrance Bright ANSI Palette */
    --color-bright-black: #64748b;
    --color-gray: #94a3b8;
    --color-bright-red: #f87171;
    --color-bright-green: #4ade80;
    --color-bright-yellow: #fde047;
    --color-bright-blue: #60a5fa;
    --color-bright-magenta: #d8b4fe;
    --color-bright-cyan: #67e8f9;
    --color-bright-white: #ffffff;

    /* Atmospheric & Diagnostic Accents */
    --accent-gold: #fbbf24;
    --accent-emerald: #34d399;
    --accent-rose: #fb7185;
    --debug-color: #a78bfa;
    --debug-bg: rgba(167, 139, 250, 0.12);
    --echo-color: #94a3b8;

    /* Typography Specifications */
    --font-family: "JetBrains Mono", "Fira Code", "Cascadia Code", "SF Mono",
                   "Roboto Mono", "Menlo", "Consolas", "DejaVu Sans Mono", monospace;
    --font-size: 16px;
    --line-height: 1.6;

    /* Terminal Metrics */
    --terminal-max-width: 960px;
    --terminal-padding-x: clamp(14px, 3.5vw, 36px);
    --terminal-padding-y: clamp(16px, 2.8vw, 28px);
    --header-height: 44px;
    --status-dot-size: 8px;

    /* CRT Atmospheric Filter Parameters */
    --crt-scanline-color: rgba(0, 0, 0, 0.32);
    --crt-scanline-size: 3px;
    --crt-phosphor-glow: 0 0 7px rgba(56, 189, 248, 0.35);
    --crt-vignette: radial-gradient(circle at center, transparent 65%, rgba(0, 0, 0, 0.65) 100%);
}

/* ------------------------------------------------------------------------------
 * 2. GLOBAL RESET & BASE CANVAS
 * ------------------------------------------------------------------------------ */
*, *::before, *::after {
    box-sizing: border-box;
    margin: 0;
    padding: 0;
}

html, body {
    width: 100%;
    height: 100%;
    margin: 0;
    padding: 0;
    background-color: var(--bg-color);
    color: var(--text-color);
    font-family: var(--font-family);
    font-size: var(--font-size);
    line-height: var(--line-height);
    -webkit-font-smoothing: antialiased;
    -moz-osx-font-smoothing: grayscale;
    text-rendering: optimizeLegibility;
    overflow: hidden;
    user-select: text;
}

::selection {
    background-color: var(--selection-bg);
    color: var(--selection-text);
}

/* ------------------------------------------------------------------------------
 * 3. APPLICATION SHELL & LAYOUT STRUCTURE
 * ------------------------------------------------------------------------------ */
#app-shell {
    display: flex;
    flex-direction: column;
    width: 100%;
    height: 100%;
    height: 100dvh;
    background-color: var(--bg-color);
    position: relative;
    overflow: hidden;
}

/* Top App / Webview Header Bar */
#header-bar {
    display: flex;
    align-items: center;
    justify-content: space-between;
    height: var(--header-height);
    min-height: var(--header-height);
    padding: 0 var(--terminal-padding-x);
    background-color: var(--bg-surface);
    border-bottom: 1px solid var(--border-color);
    z-index: 10;
    user-select: none;
}

.brand-section {
    display: flex;
    align-items: center;
    gap: 10px;
}

.brand-title {
    font-size: 0.88rem;
    font-weight: 700;
    letter-spacing: 0.08em;
    text-transform: uppercase;
    color: var(--text-color);
}

.brand-badge {
    font-size: 0.72rem;
    font-weight: 600;
    padding: 2px 7px;
    border-radius: 4px;
    background: var(--bg-surface-elevated);
    border: 1px solid var(--border-color);
    color: var(--prompt-color);
}

.header-status {
    display: flex;
    align-items: center;
    gap: 8px;
    font-size: 0.8rem;
    color: var(--text-muted);
}

.status-dot {
    width: var(--status-dot-size);
    height: var(--status-dot-size);
    border-radius: 50%;
    background-color: var(--color-bright-green);
    box-shadow: 0 0 6px var(--color-bright-green);
    transition: background-color 0.3s ease, box-shadow 0.3s ease;
}

.status-dot.connecting {
    background-color: var(--color-bright-yellow);
    box-shadow: 0 0 6px var(--color-bright-yellow);
}

.status-dot.error {
    background-color: var(--color-bright-red);
    box-shadow: 0 0 6px var(--color-bright-red);
}

.header-controls {
    display: flex;
    align-items: center;
    gap: 8px;
}

.btn-ctrl {
    background: var(--bg-surface-elevated);
    border: 1px solid var(--border-color);
    color: var(--text-muted);
    font-family: inherit;
    font-size: 0.75rem;
    padding: 4px 10px;
    border-radius: 4px;
    cursor: pointer;
    transition: all 0.2s ease;
    user-select: none;
}

.btn-ctrl:hover {
    color: var(--text-color);
    border-color: var(--prompt-color);
}

.btn-ctrl.active {
    color: var(--prompt-color);
    border-color: var(--prompt-color);
    background: rgba(56, 189, 248, 0.12);
}

/* ------------------------------------------------------------------------------
 * 4. TERMINAL CONTAINER & TEXT SCROLLPORT
 * ------------------------------------------------------------------------------ */
#terminal-scrollport {
    flex: 1 1 auto;
    min-height: 0;
    overflow-y: auto;
    overflow-x: hidden;
    scroll-behavior: smooth;
    padding: var(--terminal-padding-y) var(--terminal-padding-x);
    position: relative;
    cursor: text;
}

#terminal-viewport {
    width: min(100%, var(--terminal-max-width));
    margin: 0 auto;
    display: flex;
    flex-direction: column;
    min-height: 100%;
}

/* Output Buffer Element */
#output-buffer {
    flex: 1 0 auto;
    display: block;
    white-space: pre-wrap;
    word-break: break-word;
    overflow-wrap: anywhere;
    font-size: var(--font-size);
    line-height: var(--line-height);
    margin-bottom: 8px;
}

/* Output Buffer Child Lines */
.terminal-line {
    display: block;
    min-height: 1.25em;
}

.echo-command {
    color: var(--echo-color);
    opacity: 0.9;
    margin: 4px 0 8px 0;
}

.echo-command .prompt-symbol {
    color: var(--prompt-color);
    font-weight: 700;
    margin-right: 8px;
}

/* ------------------------------------------------------------------------------
 * 5. COMMAND INPUT ROW & INTERACTIVE PROMPT
 * ------------------------------------------------------------------------------ */
#input-container {
    flex: 0 0 auto;
    margin-top: 4px;
    padding-bottom: 12px;
}

#command-form {
    display: flex;
    align-items: baseline;
    width: 100%;
    gap: 10px;
    font-size: var(--font-size);
    line-height: var(--line-height);
}

.prompt-label {
    flex: 0 0 auto;
    color: var(--prompt-color);
    font-weight: 700;
    text-shadow: 0 0 8px var(--prompt-glow);
    user-select: none;
    font-size: 1.1em;
}

.input-wrapper {
    flex: 1 1 auto;
    position: relative;
    display: flex;
    align-items: center;
    min-width: 0;
}

#command-input {
    width: 100%;
    background: transparent;
    border: none;
    outline: none;
    color: var(--text-color);
    font-family: inherit;
    font-size: var(--font-size);
    line-height: var(--line-height);
    caret-color: var(--cursor-color);
    padding: 0;
    margin: 0;
}

#command-input:disabled {
    color: var(--text-dimmed);
    cursor: wait;
}

/* Optional Custom Blinking Cursor */
.terminal-cursor {
    display: inline-block;
    width: 9px;
    height: 1.25em;
    background-color: var(--cursor-color);
    box-shadow: 0 0 8px var(--prompt-glow);
    vertical-align: text-bottom;
    margin-left: 2px;
    animation: cursor-blink 1.05s infinite;
}

@keyframes cursor-blink {
    0%, 49% {
        opacity: 1;
        visibility: visible;
    }
    50%, 100% {
        opacity: 0;
        visibility: hidden;
    }
}

/* ------------------------------------------------------------------------------
 * 6. STREAM MANIPULATOR STYLE CLASSES (io.hpp Compatibility)
 * ------------------------------------------------------------------------------ */
/* Text Attributes */
.style-reset {
    color: var(--text-color);
    font-weight: normal;
    font-style: normal;
    text-decoration: none;
    opacity: 1;
}

.style-bold {
    font-weight: 700;
}

.style-dim {
    opacity: 0.65;
}

.style-italic {
    font-style: italic;
}

.style-underline {
    text-decoration: underline;
    text-underline-offset: 3px;
}

/* Standard ANSI Colors */
.style-black   { color: var(--color-black); }
.style-red     { color: var(--color-red); }
.style-green   { color: var(--color-green); }
.style-yellow  { color: var(--color-yellow); }
.style-blue    { color: var(--color-blue); }
.style-magenta { color: var(--color-magenta); }
.style-cyan    { color: var(--color-cyan); }
.style-white   { color: var(--color-white); }

/* High-Vibrance Bright ANSI Colors (both camelCase and kebab-case) */
.style-brightBlack,
.style-bright-black { color: var(--color-bright-black); }

.style-gray,
.style-grey        { color: var(--color-gray); }

.style-brightRed,
.style-bright-red     { color: var(--color-bright-red); }

.style-brightGreen,
.style-bright-green   { color: var(--color-bright-green); }

.style-brightYellow,
.style-bright-yellow  { color: var(--color-bright-yellow); }

.style-brightBlue,
.style-bright-blue    { color: var(--color-bright-blue); }

.style-brightMagenta,
.style-bright-magenta { color: var(--color-bright-magenta); }

.style-brightCyan,
.style-bright-cyan    { color: var(--color-bright-cyan); }

.style-brightWhite,
.style-bright-white   { color: var(--color-bright-white); }

/* Dev & Diagnostic Streams */
.style-debug {
    color: var(--debug-color);
    background-color: var(--debug-bg);
    padding: 1px 4px;
    border-radius: 3px;
}

/* ------------------------------------------------------------------------------
 * 7. RETRO CRT ATMOSPHERIC EFFECTS (Toggle via .crt class)
 * ------------------------------------------------------------------------------ */
.crt-overlay {
    display: none;
    pointer-events: none;
    position: absolute;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    z-index: 50;
}

body.crt .crt-overlay {
    display: block;
    background:
        linear-gradient(var(--crt-scanline-color) 50%, transparent 50%),
        var(--crt-vignette);
    background-size: 100% var(--crt-scanline-size), 100% 100%;
    opacity: 0.85;
}

body.crt #output-buffer,
body.crt .prompt-label,
body.crt #command-input {
    text-shadow: var(--crt-phosphor-glow);
}

body.crt #terminal-scrollport {
    animation: crt-flicker 0.12s infinite;
}

@keyframes crt-flicker {
    0%   { opacity: 0.985; }
    50%  { opacity: 1.0; }
    100% { opacity: 0.99; }
}

/* ------------------------------------------------------------------------------
 * 8. CUSTOM SCROLLBAR ILLUMINATION
 * ------------------------------------------------------------------------------ */
#terminal-scrollport {
    scrollbar-width: thin;
    scrollbar-color: var(--border-color) transparent;
}

#terminal-scrollport::-webkit-scrollbar {
    width: 8px;
    height: 8px;
}

#terminal-scrollport::-webkit-scrollbar-track {
    background: transparent;
}

#terminal-scrollport::-webkit-scrollbar-thumb {
    background: var(--border-color);
    border-radius: 4px;
    border: 2px solid var(--bg-color);
}

#terminal-scrollport::-webkit-scrollbar-thumb:hover {
    background: var(--text-dimmed);
}

/* ------------------------------------------------------------------------------
 * 9. RESPONSIVE DESIGN & MOBILE OPTIMIZATION
 * ------------------------------------------------------------------------------ */
@media (max-width: 640px) {
    :root {
        --font-size: 14.5px;
        --line-height: 1.5;
        --header-height: 40px;
    }

    .brand-title {
        font-size: 0.8rem;
    }

    .btn-ctrl {
        padding: 3px 8px;
        font-size: 0.7rem;
    }

    #input-container {
        padding-bottom: 8px;
    }
}

@media (max-width: 420px) {
    .brand-badge {
        display: none;
    }
}

/* ------------------------------------------------------------------------------
 * 10. SYSTEM NOTICES, SPINNERS & ALERTS
 * ------------------------------------------------------------------------------ */
.system-notice {
    padding: 10px 14px;
    margin-bottom: 14px;
    border-radius: 6px;
    background: var(--bg-surface-elevated);
    border-left: 4px solid var(--prompt-color);
    font-size: 0.92rem;
    color: var(--text-muted);
}

.system-error {
    background: rgba(239, 68, 68, 0.12);
    border-left-color: var(--color-bright-red);
    color: var(--color-bright-red);
}

.spinner {
    display: inline-block;
    width: 14px;
    height: 14px;
    border: 2px solid var(--border-color);
    border-top-color: var(--prompt-color);
    border-radius: 50%;
    animation: spin 0.8s linear infinite;
    vertical-align: middle;
    margin-right: 8px;
}

@keyframes spin {
    to { transform: rotate(360deg); }
}
)FICTIVE_RAW";

inline constexpr std::string_view EMBEDDED_INDEX_HTML = R"FICTIVE_RAW(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <meta name="description" content="Fictive-AI Interactive Fiction Terminal & Engine Shell">
    <meta name="theme-color" content="#0b0e14">
    <title>Fictive-AI Terminal</title>
    <link rel="stylesheet" href="style.css">
</head>
<body class="crt">
    <!-- Atmospheric CRT Overlay -->
    <div class="crt-overlay" aria-hidden="true"></div>

    <!-- Application Shell -->
    <div id="app-shell">
        <!-- Top Navigation & Status Bar -->
        <header id="header-bar" role="banner">
            <div class="brand-section">
                <span class="brand-title">Fictive-AI</span>
                <span class="brand-badge">Pure C++17</span>
            </div>

            <div class="header-status">
                <span id="status-indicator" class="status-dot connecting" aria-hidden="true"></span>
                <span id="status-text">Initializing engine...</span>
            </div>

            <div class="header-controls">
                <button type="button" id="btn-toggle-crt" class="btn-ctrl active" title="Toggle retro CRT scanline effect (Ctrl+Shift+C)">
                    CRT: ON
                </button>
                <button type="button" id="btn-clear-screen" class="btn-ctrl" title="Clear terminal screen (Ctrl+L)">
                    Clear
                </button>
            </div>
        </header>

        <!-- Terminal Scrollport & Output Buffer -->
        <main id="terminal-scrollport" role="main" aria-label="Terminal Output Scrollport">
            <div id="terminal-viewport">
                <!-- System Loading Notice -->
                <div id="system-loading" class="system-notice">
                    <span class="spinner"></span>
                    <span id="loading-message">Awakening Fictive-AI engine...</span>
                </div>

                <!-- Error Container -->
                <div id="system-error" class="system-notice system-error" style="display: none;" role="alert"></div>

                <!-- Output Buffer -->
                <div id="output-buffer" role="log" aria-live="polite" aria-atomic="false"></div>

                <!-- Command Input Row -->
                <div id="input-container">
                    <form id="command-form" autocomplete="off" spellcheck="false">
                        <label id="prompt-label" class="prompt-label" for="command-input">&gt;</label>
                        <div class="input-wrapper">
                            <input
                                type="text"
                                id="command-input"
                                name="command"
                                autocomplete="off"
                                autocapitalize="off"
                                spellcheck="false"
                                placeholder="Type a command or 'help'..."
                                disabled
                            >
                        </div>
                    </form>
                </div>
            </div>
        </main>
    </div>

    <!-- ----------------------------------------------------------------------
         FICTIVE-AI UNIVERSAL JS BRIDGE & TERMINAL DRIVER
         ---------------------------------------------------------------------- -->
    <script>
        (function() {
            "use strict";

            // DOM References
            const scrollport = document.getElementById("terminal-scrollport");
            const outputBuffer = document.getElementById("output-buffer");
            const commandForm = document.getElementById("command-form");
            const commandInput = document.getElementById("command-input");
            const promptLabel = document.getElementById("prompt-label");
            const statusIndicator = document.getElementById("status-indicator");
            const statusText = document.getElementById("status-text");
            const loadingNotice = document.getElementById("system-loading");
            const errorNotice = document.getElementById("system-error");
            const btnToggleCrt = document.getElementById("btn-toggle-crt");
            const btnClearScreen = document.getElementById("btn-clear-screen");

            // Terminal State
            let commandHistory = [];
            let historyIndex = -1;
            let currentInputDraft = "";
            let pendingInputResolver = null;
            let isEngineReady = false;

            window.inputQueue = [];

            // ------------------------------------------------------------------
            // ANSI ESCAPE SEQUENCE DECODER & HTML SANITIZER
            // ------------------------------------------------------------------
            const ANSI_STYLES_MAP = {
                1: "style-bold",
                2: "style-dim",
                3: "style-italic",
                4: "style-underline",
                30: "style-black",
                31: "style-red",
                32: "style-green",
                33: "style-yellow",
                34: "style-blue",
                35: "style-magenta",
                36: "style-cyan",
                37: "style-white",
                90: "style-bright-black",
                91: "style-bright-red",
                92: "style-bright-green",
                93: "style-bright-yellow",
                94: "style-bright-blue",
                95: "style-bright-magenta",
                96: "style-bright-cyan",
                97: "style-bright-white"
            };

            function escapeHtml(str) {
                return str
                    .replace(/&/g, "&amp;")
                    .replace(/</g, "&lt;")
                    .replace(/>/g, "&gt;")
                    .replace(/"/g, "&quot;")
                    .replace(/'/g, "&#039;");
            }

            function ansiToHtml(rawText) {
                if (!rawText) return "";

                // Strip terminal clear / cursor position sequences
                let cleaned = rawText
                    .replace(/\x1b\[\?1049[hl]/g, "")
                    .replace(/\x1b\[2J/g, "")
                    .replace(/\x1b\[\d*(;\d+)*H/g, "");

                const parts = cleaned.split(/(\x1b\[[0-9;]*m)/g);
                let html = "";
                let activeClasses = new Set();
                let openSpansCount = 0;

                for (let i = 0; i < parts.length; i++) {
                    const part = parts[i];
                    if (!part) continue;

                    const match = part.match(/^\x1b\[([0-9;]*)m$/);
                    if (match) {
                        const codesStr = match[1];
                        if (!codesStr || codesStr === "0") {
                            // Reset: close open spans
                            while (openSpansCount > 0) {
                                html += "</span>";
                                openSpansCount--;
                            }
                            activeClasses.clear();
                        } else {
                            const codes = codesStr.split(";").map(Number);
                            for (let c = 0; c < codes.length; c++) {
                                const code = codes[c];
                                if (code === 0) {
                                    while (openSpansCount > 0) {
                                        html += "</span>";
                                        openSpansCount--;
                                    }
                                    activeClasses.clear();
                                } else if (ANSI_STYLES_MAP[code]) {
                                    const cls = ANSI_STYLES_MAP[code];
                                    html += `<span class="${cls}">`;
                                    openSpansCount++;
                                    activeClasses.add(cls);
                                }
                            }
                        }
                    } else {
                        html += escapeHtml(part);
                    }
                }

                // Close remaining unclosed spans
                while (openSpansCount > 0) {
                    html += "</span>";
                    openSpansCount--;
                }

                return html;
            }

            // ------------------------------------------------------------------
            // SCROLLPORT & BUFFER OPERATIONS
            // ------------------------------------------------------------------
            function scrollToBottom() {
                requestAnimationFrame(() => {
                    scrollport.scrollTop = scrollport.scrollHeight;
                });
            }

            function appendTerminalOutput(rawText) {
                if (rawText === undefined || rawText === null) return;
                const converted = ansiToHtml(String(rawText));
                if (!converted) return;

                const temp = document.createElement("div");
                temp.innerHTML = converted;

                while (temp.firstChild) {
                    outputBuffer.appendChild(temp.firstChild);
                }

                scrollToBottom();
            }

            function clearScreen() {
                outputBuffer.innerHTML = "";
                scrollToBottom();
            }

            function setEngineStatus(state, message) {
                statusIndicator.className = "status-dot " + (state || "ready");
                statusText.textContent = message || (state === "ready" ? "Ready" : state);
            }

            function displayError(message) {
                loadingNotice.style.display = "none";
                errorNotice.style.display = "block";
                errorNotice.innerHTML = `<strong>Runtime Notice:</strong> ${escapeHtml(message)}`;
                setEngineStatus("error", "Engine failed");
            }

            // ------------------------------------------------------------------
            // INPUT & HISTORY MANAGEMENT
            // ------------------------------------------------------------------
            function echoUserCommand(cmd) {
                const echoDiv = document.createElement("div");
                echoDiv.className = "terminal-line echo-command";
                echoDiv.innerHTML = `<span class="prompt-symbol">&gt;</span>${escapeHtml(cmd)}`;
                outputBuffer.appendChild(echoDiv);
                scrollToBottom();
            }

            function handleCommandSubmit(cmd) {
                cmd = cmd.trim();

                // Echo to terminal
                echoUserCommand(cmd);

                // Command history management
                if (cmd.length > 0) {
                    commandHistory.push(cmd);
                    historyIndex = commandHistory.length;
                    currentInputDraft = "";
                }

                // Built-in terminal commands
                if (cmd.toLowerCase() === "clear" || cmd.toLowerCase() === "cls") {
                    clearScreen();
                    resolveOrQueueInput("");
                    return;
                }

                if (cmd.toLowerCase() === "quit" || cmd.toLowerCase() === "exit") {
                    appendTerminalOutput("\nExiting session...\n");
                }

                resolveOrQueueInput(cmd);
            }

            function resolveOrQueueInput(cmd) {
                if (typeof pendingInputResolver === "function") {
                    const resolver = pendingInputResolver;
                    pendingInputResolver = null;
                    resolver(cmd);
                } else {
                    window.inputQueue.push(cmd);
                }

                // Send to Desktop Webview IPC if available
                postToDesktopBridge(cmd);
            }

            // ------------------------------------------------------------------
            // DESKTOP WEBVIEW IPC BRIDGE
            // ------------------------------------------------------------------
            function postToDesktopBridge(message) {
                try {
                    // Windows / WebView2
                    if (window.chrome && window.chrome.webview && typeof window.chrome.webview.postMessage === "function") {
                        window.chrome.webview.postMessage(message);
                        return true;
                    }
                    // macOS / iOS / Linux WebKit
                    if (window.webkit && window.webkit.messageHandlers && window.webkit.messageHandlers.fictive) {
                        window.webkit.messageHandlers.fictive.postMessage(message);
                        return true;
                    }
                    // Generic desktopBridge interface
                    if (window.desktopBridge && typeof window.desktopBridge.postMessage === "function") {
                        window.desktopBridge.postMessage(message);
                        return true;
                    }
                    // Python pywebview
                    if (window.pywebview && window.pywebview.api && typeof window.pywebview.api.on_command === "function") {
                        window.pywebview.api.on_command(message);
                        return true;
                    }
                } catch (err) {
                    console.warn("Desktop IPC postMessage warning:", err);
                }
                return false;
            }

            // Listen for incoming Desktop IPC messages
            window.addEventListener("message", function(event) {
                if (event.data !== undefined && event.data !== null) {
                    if (typeof event.data === "string") {
                        appendTerminalOutput(event.data);
                    } else if (event.data.text) {
                        appendTerminalOutput(event.data.text);
                    }
                }
            });

            if (window.chrome && window.chrome.webview && typeof window.chrome.webview.addEventListener === "function") {
                window.chrome.webview.addEventListener("message", function(event) {
                    if (event.data !== undefined) {
                        appendTerminalOutput(event.data);
                    }
                });
            }

            // Global bridge hooks for native C++ host
            window.fictive_print = function(text) {
                appendTerminalOutput(text);
            };

            window.fictive_clear = function() {
                clearScreen();
            };

            window.fictive_readLine = function() {
                return new Promise(function(resolve) {
                    if (window.inputQueue.length > 0) {
                        resolve(window.inputQueue.shift());
                    } else {
                        pendingInputResolver = resolve;
                    }
                });
            };

            // Compatibility hook for input resolver property
            Object.defineProperty(window, "resolveInput", {
                get() { return pendingInputResolver; },
                set(resolver) {
                    pendingInputResolver = resolver;
                    if (window.inputQueue.length > 0 && typeof pendingInputResolver === "function") {
                        const r = pendingInputResolver;
                        pendingInputResolver = null;
                        r(window.inputQueue.shift());
                    }
                }
            });

            // ------------------------------------------------------------------
            // KEYBOARD & USER INTERACTION LISTENERS
            // ------------------------------------------------------------------
            commandForm.addEventListener("submit", function(e) {
                e.preventDefault();
                const cmd = commandInput.value;
                commandInput.value = "";
                handleCommandSubmit(cmd);
            });

            commandInput.addEventListener("keydown", function(e) {
                if (e.key === "ArrowUp") {
                    e.preventDefault();
                    if (commandHistory.length === 0) return;
                    if (historyIndex === commandHistory.length) {
                        currentInputDraft = commandInput.value;
                    }
                    if (historyIndex > 0) {
                        historyIndex--;
                        commandInput.value = commandHistory[historyIndex];
                    }
                } else if (e.key === "ArrowDown") {
                    e.preventDefault();
                    if (historyIndex < commandHistory.length - 1) {
                        historyIndex++;
                        commandInput.value = commandHistory[historyIndex];
                    } else {
                        historyIndex = commandHistory.length;
                        commandInput.value = currentInputDraft;
                    }
                }
            });

            // Global keyboard shortcuts (Ctrl+L, Ctrl+Shift+C)
            document.addEventListener("keydown", function(e) {
                // Clear screen: Ctrl+L
                if (e.ctrlKey && e.key.toLowerCase() === "l" && !e.shiftKey && !e.altKey) {
                    e.preventDefault();
                    clearScreen();
                    return;
                }

                // Toggle CRT: Ctrl+Shift+C
                if (e.ctrlKey && e.shiftKey && e.key.toLowerCase() === "c") {
                    e.preventDefault();
                    toggleCrt();
                    return;
                }

                // If typing and input is not focused (and not selecting text), autofocus input
                if (!e.ctrlKey && !e.metaKey && !e.altKey && e.key.length === 1) {
                    if (document.activeElement !== commandInput && window.getSelection().toString() === "") {
                        commandInput.focus();
                    }
                }
            });

            // Focus retention: clicking anywhere on terminal refocuses input
            scrollport.addEventListener("click", function() {
                if (window.getSelection().toString() === "") {
                    commandInput.focus();
                }
            });

            // ------------------------------------------------------------------
            // CRT & UI CONTROLS
            // ------------------------------------------------------------------
            function toggleCrt() {
                const isCrt = document.body.classList.toggle("crt");
                btnToggleCrt.classList.toggle("active", isCrt);
                btnToggleCrt.textContent = isCrt ? "CRT: ON" : "CRT: OFF";
                try {
                    localStorage.setItem("fictive_crt", isCrt ? "1" : "0");
                } catch (_) {}
            }

            btnToggleCrt.addEventListener("click", toggleCrt);
            btnClearScreen.addEventListener("click", clearScreen);

            // Restore CRT preference
            try {
                const savedCrt = localStorage.getItem("fictive_crt");
                if (savedCrt === "0") {
                    document.body.classList.remove("crt");
                    btnToggleCrt.classList.remove("active");
                    btnToggleCrt.textContent = "CRT: OFF";
                }
            } catch (_) {}

            // ------------------------------------------------------------------
            // EMSCRIPTEN / WEBASSEMBLY RUNTIME INTEGRATION
            // ------------------------------------------------------------------
            window.Module = {
                locateFile: function(path) {
                    return path;
                },
                print: function(text) {
                    appendTerminalOutput(String(text) + "\n");
                },
                printErr: function(text) {
                    console.warn("[Fictive-AI WASM]", text);
                },
                onRuntimeInitialized: function() {
                    isEngineReady = true;
                    loadingNotice.style.display = "none";
                    commandInput.disabled = false;
                    commandInput.focus();
                    setEngineStatus("ready", "Ready");
                    scrollToBottom();
                },
                onAbort: function(what) {
                    displayError("WebAssembly execution aborted: " + what);
                }
            };

            // Standalone Fallback / Desktop Webview Ready Initializer
            window.addEventListener("DOMContentLoaded", function() {
                // If not loading WASM via engine.js or if running inside Webview, enable input
                setTimeout(function() {
                    if (!isEngineReady && loadingNotice.style.display !== "none") {
                        loadingNotice.style.display = "none";
                        commandInput.disabled = false;
                        commandInput.focus();
                        setEngineStatus("ready", "Ready");
                    }
                }, 1200);
            });

        })();
    </script>

    <!-- Optional WebAssembly Engine Script (Loaded asynchronously if present) -->
    <script async src="engine.js" onerror="/* Optional in standalone desktop webview mode */"></script>
</body>
</html>
)FICTIVE_RAW";

inline constexpr std::string_view EMBEDDED_MAKEFILE = R"FICTIVE_RAW(# ==============================================================================
# Fictive-AI Tailored Game Makefile
# Generated by fictive-ai (Communion of Saints Framework)
# ==============================================================================

SHELL := /bin/bash

# Project configuration
TARGET ?= {{PROJECT_NAME}}
SRCS ?= main.cpp

# Compiler discovery
CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic

# Build mode flags: dev (default) vs production (-p / --production / PRODUCTION=1)
ifeq ($(PRODUCTION),1)
    CXXFLAGS += -DPRODUCTION -DNDEBUG -O3 -s
else
    CXXFLAGS += -DDEBUG -g
endif

# Target platform flags
ifdef TARGET_DESKTOP
    CXXFLAGS += -DFICTIVE_TARGET_DESKTOP
endif

# Fictive-AI header include path discovery
{{DETECTED_INCLUDE_RULE}}

INCLUDES ?= $(FICTIVE_INCLUDE_FLAG)

# WebAssembly (Emscripten) configuration
EMXX ?= em++
EMFLAGS ?= -DFICTIVE_TARGET_WEB -s WASM=1 -s ASYNCIFY -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' -std=c++17
TARGET_WEB ?= engine.js

# Output binary paths
TARGET_NATIVE ?= $(TARGET)
TARGET_DESKTOP ?= $(TARGET)_desktop

.PHONY: all terminal web desktop clean run help

# Target 'all': default builds terminal target
all: terminal

# Target 'terminal': compiles native interactive terminal binary
terminal: $(TARGET_NATIVE)

$(TARGET_NATIVE): $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRCS) -o $(TARGET_NATIVE)
	@echo "Build successful: $(TARGET_NATIVE)"

# Target 'desktop': compiles native desktop-target binary
desktop: $(TARGET_DESKTOP)

$(TARGET_DESKTOP): $(SRCS)
	$(CXX) $(CXXFLAGS) -DFICTIVE_TARGET_DESKTOP $(INCLUDES) $(SRCS) -o $(TARGET_DESKTOP)
	@echo "Desktop build successful: $(TARGET_DESKTOP)"

# Target 'web': compiles WebAssembly module using Emscripten
web: $(TARGET_WEB)

$(TARGET_WEB): $(SRCS)
	@if command -v $(EMXX) >/dev/null 2>&1; then \
		echo "Compiling WebAssembly with $(EMXX)..."; \
		$(EMXX) $(CXXFLAGS) $(INCLUDES) $(EMFLAGS) $(SRCS) -o $(TARGET_WEB); \
		echo "WebAssembly build successful: $(TARGET_WEB)"; \
	elif [ -n "$$EMSDK" ] && [ -f "$$EMSDK/emsdk_env.sh" ]; then \
		echo "Sourcing EMSDK environment..."; \
		source "$$EMSDK/emsdk_env.sh" >/dev/null 2>&1 && $(EMXX) $(CXXFLAGS) $(INCLUDES) $(EMFLAGS) $(SRCS) -o $(TARGET_WEB); \
		echo "WebAssembly build successful: $(TARGET_WEB)"; \
	else \
		echo "Error: Emscripten compiler ($(EMXX)) not found. Install EMSDK to compile web target." >&2; \
		exit 1; \
	fi

# Target 'clean': removes generated binaries and web assets
clean:
	rm -f $(TARGET_NATIVE) $(TARGET_DESKTOP) $(TARGET_WEB) engine.wasm game *.o

# Target 'run': executes terminal binary
run: terminal
	./$(TARGET_NATIVE)

# Target 'help': displays targets
help:
	@echo "Available targets:"
	@echo "  make terminal    - Build native terminal game (dev mode)"
	@echo "  make terminal PRODUCTION=1 - Build optimized production binary"
	@echo "  make desktop     - Build desktop platform binary"
	@echo "  make web         - Build WebAssembly module (requires emcc/em++)"
	@echo "  make clean       - Remove built binaries"
	@echo "  make run         - Build and run terminal game"
)FICTIVE_RAW";

} // namespace fictive::cli
