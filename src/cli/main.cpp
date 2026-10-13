/**
 * ==============================================================================
 * FICTIVE-AI STANDALONE COMPILER & TOOLCHAIN CLI
 * Apostle Paul, Tentmaker & Tooling Engineer
 * Communion of Saints Framework
 * ==============================================================================
 * Single standalone modern C++17 binary providing project generation,
 * tailored Makefile creation, platform compilation, and direct execution.
 */

#include "templates.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

namespace fs = std::filesystem;

namespace fictive::cli {

// ==============================================================================
// Terminal ANSI Color Styling for CLI Output
// ==============================================================================

namespace style {
    inline constexpr std::string_view RESET       = "\033[0m";
    inline constexpr std::string_view BOLD        = "\033[1m";
    inline constexpr std::string_view DIM         = "\033[2m";
    inline constexpr std::string_view RED         = "\033[31m";
    inline constexpr std::string_view GREEN       = "\033[32m";
    inline constexpr std::string_view YELLOW      = "\033[33m";
    inline constexpr std::string_view BLUE        = "\033[34m";
    inline constexpr std::string_view MAGENTA     = "\033[35m";
    inline constexpr std::string_view CYAN        = "\033[36m";
    inline constexpr std::string_view BRIGHT_CYAN = "\033[96m";
} // namespace style

// ==============================================================================
// Version & Help Reference
// ==============================================================================

void printVersion() {
    std::cout << style::BOLD << style::BRIGHT_CYAN
              << "fictive-ai version 1.0.0 (The Communion of Saints Framework)\n"
              << style::RESET;
    std::cout << style::CYAN << "Author & Architect: " << style::RESET
              << "The Blessed Virgin Mary, Queen of Heaven & Seat of Wisdom\n";
    std::cout << style::CYAN << "Master Builder:     " << style::RESET
              << "St. Peter the Apostle, Rock of the Church\n";
    std::cout << style::CYAN << "Build & Tooling:    " << style::RESET
              << "St. Paul the Apostle, Relentless Tentmaker\n";
    std::cout << style::CYAN << "Sacred Aesthetics:  " << style::RESET
              << "Blessed Fra Angelico, Master of Harmonious Illumination\n";
}

void printHelp() {
    std::cout << style::BOLD << style::BRIGHT_CYAN
              << "fictive-ai - Standalone C++17 Interactive Fiction Toolchain\n"
              << "Communion of Saints Framework\n\n"
              << style::RESET;

    std::cout << style::BOLD << "USAGE:\n" << style::RESET
              << "  fictive-ai <command> [target] [options]\n\n";

    std::cout << style::BOLD << "COMMANDS:\n" << style::RESET
              << "  " << style::GREEN << "new <project_name>" << style::RESET
              << "    Create a new game directory with complete starter templates\n"
              << "  " << style::GREEN << "init [project_name]" << style::RESET
              << "   Initialize starter project in current (or specified) directory\n"
              << "  " << style::GREEN << "makefile" << style::RESET
              << "              Generate a tailored standalone Makefile in current directory\n"
              << "  " << style::GREEN << "build [target] [opts]" << style::RESET
              << "  Compile target platform (dev or production mode)\n"
              << "  " << style::GREEN << "run [target] [opts]" << style::RESET
              << "    Compile target and immediately execute the resulting binary\n"
              << "  " << style::GREEN << "version, -v, --version" << style::RESET
              << " Display version information and Mother Mary attribution\n"
              << "  " << style::GREEN << "help, -h, --help" << style::RESET
              << "       Display this command reference\n\n";

    std::cout << style::BOLD << "BUILD TARGETS:\n" << style::RESET
              << "  " << style::YELLOW << "terminal" << style::RESET
              << "  Native interactive terminal binary (default)\n"
              << "  " << style::YELLOW << "web" << style::RESET
              << "       WebAssembly (WASM) build via Emscripten (emcc/em++)\n"
              << "  " << style::YELLOW << "desktop" << style::RESET
              << "   Desktop target binary with Webview IPC bindings\n"
              << "  " << style::YELLOW << "all" << style::RESET
              << "       Build terminal, desktop, and web targets\n\n";

    std::cout << style::BOLD << "BUILD OPTIONS:\n" << style::RESET
              << "  " << style::YELLOW << "-p, --production, --publish" << style::RESET
              << "  Enable production mode (-DPRODUCTION -DNDEBUG -O3, stripped)\n"
              << "  " << style::DIM << "(Default build mode is dev with -DDEBUG -g)\n\n"
              << style::RESET;

    std::cout << style::BOLD << "EXAMPLES:\n" << style::RESET
              << "  fictive-ai new san_marco_mystery\n"
              << "  cd san_marco_mystery\n"
              << "  fictive-ai run\n"
              << "  fictive-ai build -p\n"
              << "  fictive-ai build web\n"
              << "  fictive-ai makefile\n";
}

// ==============================================================================
// Path Discovery & Environment Inspection
// ==============================================================================

/**
 * @brief Locates the directory of the currently running fictive-ai binary.
 */
fs::path getExecutableDir() {
    try {
        // Read link on Linux
        fs::path exePath = fs::canonical("/proc/self/exe");
        return exePath.parent_path();
    } catch (...) {
        return fs::current_path();
    }
}

/**
 * @brief Checks if a directory contains valid Fictive-AI headers.
 * Resolves whether `#include <fictive-ai/parser.hpp>` works with `-I<candidate>`.
 */
bool isValidIncludePath(const fs::path& dir, fs::path& resolvedPath) {
    std::error_code ec;
    if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) {
        return false;
    }

    // Direct check: dir/fictive-ai/parser.hpp exists
    fs::path checkHeader = dir / "fictive-ai" / "parser.hpp";
    if (fs::exists(checkHeader, ec)) {
        resolvedPath = fs::canonical(dir, ec);
        return true;
    }

    // Check if dir itself is named "fictive-ai" and contains parser.hpp
    if (dir.filename() == "fictive-ai" && fs::exists(dir / "parser.hpp", ec)) {
        resolvedPath = fs::canonical(dir.parent_path(), ec);
        return true;
    }

    // Check if dir contains parser.hpp and a symlink/dir named fictive-ai pointing to .
    if (fs::exists(dir / "parser.hpp", ec) && fs::exists(dir / "fictive-ai", ec)) {
        resolvedPath = fs::canonical(dir, ec);
        return true;
    }

    return false;
}

/**
 * @brief Detects the best header include path for <fictive-ai/parser.hpp>.
 */
fs::path detectHeaderIncludePath(const fs::path& exeDir) {
    fs::path resolved;

    // 1. Environment variable override
    const char* envInc = std::getenv("FICTIVE_AI_INCLUDE");
    if (envInc && *envInc) {
        fs::path p(envInc);
        if (isValidIncludePath(p, resolved)) {
            return resolved;
        }
    }

    // 2. Relative to executable binary location
    std::vector<fs::path> exeCandidates = {
        exeDir / ".." / "engine",
        exeDir / "..",
        exeDir / ".." / "include",
        exeDir / ".." / ".." / "engine",
        exeDir / ".." / ".." / "include",
        exeDir / "engine",
        exeDir / "include"
    };

    for (const auto& cand : exeCandidates) {
        if (isValidIncludePath(cand, resolved)) {
            return resolved;
        }
    }

    // 3. Current working directory and parent paths
    fs::path cwd = fs::current_path();
    std::vector<fs::path> cwdCandidates = {
        cwd / "engine",
        cwd / "include",
        cwd / "fictive-ai",
        cwd / ".." / "engine",
        cwd / "..",
        cwd
    };

    for (const auto& cand : cwdCandidates) {
        if (isValidIncludePath(cand, resolved)) {
            return resolved;
        }
    }

    // 4. User local directory ~/.local/include
    const char* home = std::getenv("HOME");
    if (home && *home) {
        fs::path userLocal = fs::path(home) / ".local" / "include";
        if (isValidIncludePath(userLocal, resolved)) {
            return resolved;
        }
    }

    // 5. System directories
    if (isValidIncludePath("/usr/local/include", resolved)) {
        return resolved;
    }
    if (isValidIncludePath("/usr/include", resolved)) {
        return resolved;
    }

    // 6. Known repository location
    fs::path repoPath = "/var/home/glenn/Projects/cpp-if-text/AI/engine";
    if (isValidIncludePath(repoPath, resolved)) {
        return resolved;
    }

    return "";
}

/**
 * @brief Locates the templates directory on disk if available.
 */
fs::path detectTemplateDir(const fs::path& exeDir) {
    std::error_code ec;

    // 1. Environment variable
    const char* envTpl = std::getenv("FICTIVE_AI_TEMPLATES");
    if (envTpl && *envTpl && fs::exists(envTpl, ec)) {
        return fs::canonical(envTpl, ec);
    }

    // 2. Relative to executable
    std::vector<fs::path> candidates = {
        exeDir / ".." / "templates",
        exeDir / "templates",
        exeDir / ".." / ".." / "templates"
    };
    for (const auto& cand : candidates) {
        if (fs::exists(cand / "main.cpp", ec)) {
            return fs::canonical(cand, ec);
        }
    }

    // 3. User local share
    const char* home = std::getenv("HOME");
    if (home && *home) {
        fs::path userShare = fs::path(home) / ".local" / "share" / "fictive-ai" / "templates";
        if (fs::exists(userShare / "main.cpp", ec)) {
            return fs::canonical(userShare, ec);
        }
    }

    // 4. System shares
    if (fs::exists("/usr/local/share/fictive-ai/templates/main.cpp", ec)) {
        return "/usr/local/share/fictive-ai/templates";
    }
    if (fs::exists("/usr/share/fictive-ai/templates/main.cpp", ec)) {
        return "/usr/share/fictive-ai/templates";
    }

    // 5. Known repository templates
    fs::path repoTpl = "/var/home/glenn/Projects/cpp-if-text/AI/templates";
    if (fs::exists(repoTpl / "main.cpp", ec)) {
        return repoTpl;
    }

    return "";
}

/**
 * @brief Reads a template file from disk, falling back to embedded string literal.
 */
std::string getTemplateContent(const fs::path& tplDir, const std::string& filename, std::string_view embeddedFallback) {
    if (!tplDir.empty()) {
        fs::path filePath = tplDir / filename;
        std::error_code ec;
        if (fs::exists(filePath, ec)) {
            std::ifstream file(filePath, std::ios::in | std::ios::binary);
            if (file) {
                std::ostringstream ss;
                ss << file.rdbuf();
                return ss.str();
            }
        }
    }
    return std::string(embeddedFallback);
}

/**
 * @brief Generates the customized Makefile content for a project.
 */
std::string generateTailoredMakefile(const std::string& projectName, const fs::path& detectedInclude) {
    std::string content(EMBEDDED_MAKEFILE);

    // Replace {{PROJECT_NAME}}
    const std::string targetPlaceholder = "{{PROJECT_NAME}}";
    size_t pos = content.find(targetPlaceholder);
    if (pos != std::string::npos) {
        content.replace(pos, targetPlaceholder.length(), projectName);
    }

    // Build tailored include detection snippet
    std::ostringstream incRule;
    if (!detectedInclude.empty()) {
        incRule << "FICTIVE_INCLUDE ?= " << detectedInclude.string() << "\n"
                << "ifeq ($(wildcard $(FICTIVE_INCLUDE)/fictive-ai/parser.hpp),)\n"
                << "    ifeq ($(wildcard $(FICTIVE_INCLUDE)/parser.hpp),)\n"
                << "        FICTIVE_INCLUDE := $(firstword $(wildcard \\\n"
                << "            $(HOME)/.local/include \\\n"
                << "            /usr/local/include \\\n"
                << "            /usr/include \\\n"
                << "            /var/home/glenn/Projects/cpp-if-text/AI/engine \\\n"
                << "            /var/home/glenn/Projects/cpp-if-text/AI \\\n"
                << "            ../engine \\\n"
                << "            ./engine \\\n"
                << "            . \\\n"
                << "        ))\n"
                << "    endif\n"
                << "endif\n\n"
                << "ifneq ($(FICTIVE_INCLUDE),)\n"
                << "    FICTIVE_INCLUDE_FLAG := -I$(FICTIVE_INCLUDE)\n"
                << "else\n"
                << "    FICTIVE_INCLUDE_FLAG :=\n"
                << "endif";
    } else {
        incRule << "FICTIVE_INCLUDE := $(firstword $(wildcard \\\n"
                << "    $(HOME)/.local/include \\\n"
                << "    /usr/local/include \\\n"
                << "    /usr/include \\\n"
                << "    /var/home/glenn/Projects/cpp-if-text/AI/engine \\\n"
                << "    /var/home/glenn/Projects/cpp-if-text/AI \\\n"
                << "    ../engine \\\n"
                << "    ./engine \\\n"
                << "    . \\\n"
                << "))\n\n"
                << "ifneq ($(FICTIVE_INCLUDE),)\n"
                << "    FICTIVE_INCLUDE_FLAG := -I$(FICTIVE_INCLUDE)\n"
                << "else\n"
                << "    FICTIVE_INCLUDE_FLAG :=\n"
                << "endif";
    }

    const std::string incPlaceholder = "{{DETECTED_INCLUDE_RULE}}";
    pos = content.find(incPlaceholder);
    if (pos != std::string::npos) {
        content.replace(pos, incPlaceholder.length(), incRule.str());
    }

    return content;
}

/**
 * @brief Checks if a command exists in PATH.
 */
bool commandExists(const std::string& cmd) {
    std::string testCmd = "command -v " + cmd + " >/dev/null 2>&1";
    return (std::system(testCmd.c_str()) == 0);
}

/**
 * @brief Detects the host C++ compiler (CXX, g++, or clang++).
 */
std::string detectHostCompiler() {
    const char* cxx = std::getenv("CXX");
    if (cxx && *cxx) {
        return cxx;
    }
    if (commandExists("g++")) {
        return "g++";
    }
    if (commandExists("clang++")) {
        return "clang++";
    }
    return "";
}

// ==============================================================================
// Subcommand Implementations
// ==============================================================================

/**
 * @brief Handles 'fictive-ai new <project_name>' and 'fictive-ai init [project_name]'.
 */
int handleNewOrInit(bool isInit, const std::string& projectNameArg) {
    fs::path targetDir;
    std::string projectName;

    if (isInit) {
        if (projectNameArg.empty()) {
            targetDir = fs::current_path();
            projectName = targetDir.filename().string();
        } else {
            targetDir = fs::current_path() / projectNameArg;
            projectName = projectNameArg;
        }
    } else {
        if (projectNameArg.empty()) {
            std::cerr << style::RED << "Error: 'fictive-ai new' requires a project name.\n"
                      << style::RESET << "Usage: fictive-ai new <project_name>\n";
            return 1;
        }
        targetDir = fs::current_path() / projectNameArg;
        projectName = projectNameArg;
    }

    std::error_code ec;
    if (!fs::exists(targetDir, ec)) {
        if (!fs::create_directories(targetDir, ec)) {
            std::cerr << style::RED << "Error: Could not create directory '"
                      << targetDir.string() << "': " << ec.message() << "\n" << style::RESET;
            return 1;
        }
    }

    fs::path exeDir = getExecutableDir();
    fs::path tplDir = detectTemplateDir(exeDir);
    fs::path detectedInc = detectHeaderIncludePath(exeDir);

    std::string mainContent = getTemplateContent(tplDir, "main.cpp", EMBEDDED_MAIN_CPP);
    std::string styleContent = getTemplateContent(tplDir, "style.css", EMBEDDED_STYLE_CSS);
    std::string indexContent = getTemplateContent(tplDir, "index.html", EMBEDDED_INDEX_HTML);
    std::string makefileContent = generateTailoredMakefile(projectName, detectedInc);

    auto writeFile = [&](const fs::path& filePath, const std::string& data) -> bool {
        std::ofstream out(filePath, std::ios::out | std::ios::binary);
        if (!out) {
            std::cerr << style::RED << "Error: Failed to write " << filePath.string() << "\n" << style::RESET;
            return false;
        }
        out << data;
        return true;
    };

    if (!writeFile(targetDir / "main.cpp", mainContent)) return 1;
    if (!writeFile(targetDir / "style.css", styleContent)) return 1;
    if (!writeFile(targetDir / "index.html", indexContent)) return 1;
    if (!writeFile(targetDir / "Makefile", makefileContent)) return 1;

    if (isInit && projectNameArg.empty()) {
        std::cout << style::BOLD << style::GREEN
                  << "✨ Initialized Fictive-AI project in current directory successfully!\n\n"
                  << style::RESET;
        std::cout << style::BOLD << "Next steps:\n" << style::RESET;
        std::cout << "  " << style::CYAN << "fictive-ai run\n\n" << style::RESET;
    } else {
        std::cout << style::BOLD << style::GREEN
                  << "✨ Created new Fictive-AI project '" << projectName << "' successfully!\n\n"
                  << style::RESET;
        std::cout << style::BOLD << "Next steps:\n" << style::RESET;
        std::cout << "  " << style::CYAN << "cd " << projectName << "\n"
                  << "  fictive-ai run\n\n" << style::RESET;
    }

    return 0;
}

/**
 * @brief Handles 'fictive-ai makefile'.
 */
int handleMakefile() {
    fs::path cwd = fs::current_path();
    std::string projectName = cwd.filename().string();
    fs::path exeDir = getExecutableDir();
    fs::path detectedInc = detectHeaderIncludePath(exeDir);

    std::string makefileContent = generateTailoredMakefile(projectName, detectedInc);
    fs::path makefilePath = cwd / "Makefile";

    std::ofstream out(makefilePath, std::ios::out | std::ios::binary);
    if (!out) {
        std::cerr << style::RED << "Error: Could not write Makefile to " << cwd.string() << "\n" << style::RESET;
        return 1;
    }
    out << makefileContent;

    std::cout << style::BOLD << style::GREEN
              << "✨ Generated tailored Makefile in current directory.\n"
              << style::RESET;
    std::cout << style::DIM << "Targets available: all, terminal, web, desktop, clean, run, help\n" << style::RESET;
    return 0;
}

/**
 * @brief Handles 'fictive-ai build [target] [options]'.
 */
int handleBuild(const std::string& target, bool isProduction, std::string* outExecutablePath = nullptr) {
    fs::path cwd = fs::current_path();
    fs::path srcFile = cwd / "main.cpp";
    std::error_code ec;

    if (!fs::exists(srcFile, ec)) {
        if (fs::exists(cwd / "src" / "main.cpp", ec)) {
            srcFile = cwd / "src" / "main.cpp";
        } else {
            std::cerr << style::RED << "Error: No main.cpp found in current directory ("
                      << cwd.string() << ").\n" << style::RESET;
            return 1;
        }
    }

    fs::path exeDir = getExecutableDir();
    fs::path detectedInc = detectHeaderIncludePath(exeDir);
    std::string incFlag;
    if (!detectedInc.empty()) {
        incFlag = "-I" + detectedInc.string();
    }

    std::string compiler = detectHostCompiler();
    std::string projectName = cwd.filename().string();

    auto compileTerminal = [&]() -> int {
        if (compiler.empty()) {
            std::cerr << style::RED << "Error: No host C++ compiler found (g++ or clang++ required).\n" << style::RESET;
            return 1;
        }

        std::string modeStr = isProduction ? "production" : "dev";
        std::cout << style::CYAN << "[fictive-ai] Compiling terminal target (mode: " << modeStr << ")..." << style::RESET << "\n";

        std::string flags = "-std=c++17 -Wall -Wextra -Wpedantic";
        if (isProduction) {
            flags += " -DPRODUCTION -DNDEBUG -O3 -s";
        } else {
            flags += " -DDEBUG -g";
        }

        std::string outBinary = "game";
        std::string cmd = compiler + " " + flags + " " + incFlag + " " + srcFile.string() + " -o " + outBinary;

        int status = std::system(cmd.c_str());
        if (status != 0) {
            std::cerr << style::RED << "[fictive-ai] Compilation failed.\n" << style::RESET;
            return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
        }

        if (isProduction && commandExists("strip")) {
            std::string stripCmd = "strip " + outBinary + " >/dev/null 2>&1";
            std::system(stripCmd.c_str());
        }

        // Create symlink for project name if different from game
        if (projectName != "game" && !projectName.empty()) {
            fs::path linkPath = cwd / projectName;
            std::error_code lec;
            if (fs::exists(linkPath, lec)) {
                fs::remove(linkPath, lec);
            }
            fs::create_symlink(outBinary, linkPath, lec);
        }

        if (outExecutablePath) {
            *outExecutablePath = (cwd / outBinary).string();
        }

        std::cout << style::GREEN << "✨ Build complete: ./" << outBinary;
        if (projectName != "game" && !projectName.empty()) {
            std::cout << " (and ./" << projectName << ")";
        }
        std::cout << "\n" << style::RESET;
        return 0;
    };

    auto compileDesktop = [&]() -> int {
        if (compiler.empty()) {
            std::cerr << style::RED << "Error: No host C++ compiler found (g++ or clang++ required).\n" << style::RESET;
            return 1;
        }

        std::string modeStr = isProduction ? "production" : "dev";
        std::cout << style::CYAN << "[fictive-ai] Compiling desktop target (mode: " << modeStr << ")..." << style::RESET << "\n";

        std::string flags = "-std=c++17 -Wall -Wextra -Wpedantic -DFICTIVE_TARGET_DESKTOP";
        if (isProduction) {
            flags += " -DPRODUCTION -DNDEBUG -O3 -s";
        } else {
            flags += " -DDEBUG -g";
        }

        std::string outBinary = "game_desktop";
        std::string cmd = compiler + " " + flags + " " + incFlag + " " + srcFile.string() + " -o " + outBinary;

        int status = std::system(cmd.c_str());
        if (status != 0) {
            std::cerr << style::RED << "[fictive-ai] Desktop compilation failed.\n" << style::RESET;
            return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
        }

        if (isProduction && commandExists("strip")) {
            std::string stripCmd = "strip " + outBinary + " >/dev/null 2>&1";
            std::system(stripCmd.c_str());
        }

        if (outExecutablePath) {
            *outExecutablePath = (cwd / outBinary).string();
        }

        std::cout << style::GREEN << "✨ Desktop build complete: ./" << outBinary << "\n" << style::RESET;
        return 0;
    };

    auto compileWeb = [&]() -> int {
        bool hasEmcc = commandExists("em++") || commandExists("emcc");
        const char* emsdk = std::getenv("EMSDK");
        std::string emscriptenEnv;
        if (!hasEmcc && emsdk && *emsdk) {
            fs::path envScript = fs::path(emsdk) / "emsdk_env.sh";
            if (fs::exists(envScript, ec)) {
                emscriptenEnv = "source \"" + envScript.string() + "\" >/dev/null 2>&1 && ";
                hasEmcc = true;
            }
        }

        if (!hasEmcc) {
            std::cerr << style::RED << "Error: Emscripten compiler (emcc/em++) not found.\n"
                      << style::RESET
                      << "To compile WebAssembly (WASM), please install EMSDK or ensure emcc is in your PATH.\n"
                      << "Visit: https://emscripten.org/docs/getting_started/downloads.html\n";
            return 1;
        }

        std::string modeStr = isProduction ? "production" : "dev";
        std::cout << style::CYAN << "[fictive-ai] Compiling WebAssembly target (mode: " << modeStr << ")..." << style::RESET << "\n";

        std::string flags = "-std=c++17 -DFICTIVE_TARGET_WEB -s WASM=1 -s ASYNCIFY -s EXPORTED_RUNTIME_METHODS='[\"ccall\",\"cwrap\"]'";
        if (isProduction) {
            flags += " -DPRODUCTION -DNDEBUG -O3";
        } else {
            flags += " -DDEBUG -g";
        }

        std::string outJs = "engine.js";
        std::string cmd = emscriptenEnv + "em++ " + flags + " " + incFlag + " " + srcFile.string() + " -o " + outJs;

        int status = std::system(cmd.c_str());
        if (status != 0) {
            std::cerr << style::RED << "[fictive-ai] WebAssembly compilation failed.\n" << style::RESET;
            return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
        }

        if (outExecutablePath) {
            *outExecutablePath = (cwd / outJs).string();
        }

        std::cout << style::GREEN << "✨ WebAssembly build complete: engine.js\n" << style::RESET;
        return 0;
    };

    if (target == "terminal" || target.empty()) {
        return compileTerminal();
    } else if (target == "desktop") {
        return compileDesktop();
    } else if (target == "web") {
        return compileWeb();
    } else if (target == "all") {
        int ret = compileTerminal();
        if (ret != 0) return ret;
        ret = compileDesktop();
        if (ret != 0) return ret;
        if (commandExists("em++") || commandExists("emcc") || std::getenv("EMSDK")) {
            ret = compileWeb();
            if (ret != 0) return ret;
        } else {
            std::cout << style::YELLOW << "[fictive-ai] Notice: emcc/em++ not found. Skipped web target.\n" << style::RESET;
        }
        return 0;
    } else {
        std::cerr << style::RED << "Error: Unknown build target '" << target << "'.\n"
                  << style::RESET << "Supported targets: terminal, web, desktop, all\n";
        return 1;
    }
}

/**
 * @brief Executes a compiled binary with interactive terminal file descriptors.
 */
int executeBinary(const std::string& binaryPath, const std::vector<std::string>& extraArgs) {
    pid_t pid = fork();
    if (pid == 0) {
        std::vector<char*> c_args;
        c_args.push_back(const_cast<char*>(binaryPath.c_str()));
        for (const auto& arg : extraArgs) {
            c_args.push_back(const_cast<char*>(arg.c_str()));
        }
        c_args.push_back(nullptr);

        execv(binaryPath.c_str(), c_args.data());
        std::cerr << style::RED << "Failed to execute: " << binaryPath
                  << " (" << std::strerror(errno) << ")\n" << style::RESET;
        _exit(127);
    } else if (pid > 0) {
        int status = 0;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            return 128 + WTERMSIG(status);
        }
        return 1;
    } else {
        std::cerr << style::RED << "Fork failed: " << std::strerror(errno) << "\n" << style::RESET;
        return 1;
    }
}

/**
 * @brief Handles 'fictive-ai run [target] [options]'.
 */
int handleRun(const std::string& target, bool isProduction, const std::vector<std::string>& extraArgs) {
    std::string execPath;
    int buildStatus = handleBuild(target, isProduction, &execPath);
    if (buildStatus != 0) {
        return buildStatus;
    }

    if (target == "web") {
        std::cout << style::BOLD << style::GREEN
                  << "✨ WebAssembly build ready (engine.js / index.html).\n" << style::RESET;
        std::cout << "To test in your browser, start a local HTTP server:\n";
        std::cout << "  " << style::CYAN << "python3 -m http.server 8080\n" << style::RESET;
        std::cout << "Then navigate to: " << style::YELLOW << "http://localhost:8080\n" << style::RESET;
        return 0;
    }

    if (execPath.empty()) {
        execPath = (target == "desktop") ? "./game_desktop" : "./game";
    }

    return executeBinary(execPath, extraArgs);
}

} // namespace fictive::cli

// ==============================================================================
// Command-Line Argument Dispatcher
// ==============================================================================

int main(int argc, char* argv[]) {
    using namespace fictive::cli;

    if (argc < 2) {
        printHelp();
        return 0;
    }

    std::string command = argv[1];

    // Global flags
    if (command == "-v" || command == "--version" || command == "version" || command == "-V") {
        printVersion();
        return 0;
    }

    if (command == "-h" || command == "--help" || command == "help") {
        printHelp();
        return 0;
    }

    if (command == "new") {
        std::string projectName;
        if (argc >= 3) {
            projectName = argv[2];
        }
        return handleNewOrInit(false, projectName);
    }

    if (command == "init") {
        std::string projectName;
        if (argc >= 3) {
            projectName = argv[2];
        }
        return handleNewOrInit(true, projectName);
    }

    if (command == "makefile") {
        return handleMakefile();
    }

    if (command == "build" || command == "run") {
        bool isProduction = false;
        std::string target = "terminal";
        std::vector<std::string> extraArgs;

        for (int i = 2; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "-p" || arg == "--production" || arg == "--publish") {
                isProduction = true;
            } else if (arg == "terminal" || arg == "web" || arg == "desktop" || arg == "all") {
                target = arg;
            } else if (!arg.empty() && arg[0] == '-') {
                std::cerr << style::RED << "Unknown flag: " << arg << "\n" << style::RESET;
                return 1;
            } else {
                extraArgs.push_back(arg);
            }
        }

        if (command == "build") {
            return handleBuild(target, isProduction);
        } else {
            return handleRun(target, isProduction, extraArgs);
        }
    }

    std::cerr << style::RED << "Unknown command: '" << command << "'.\n" << style::RESET;
    std::cerr << "Run 'fictive-ai help' to see available commands.\n";
    return 1;
}
