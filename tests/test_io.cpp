/**
 * @file test_io.cpp
 * @brief Comprehensive verification suite for fictive::io (io.hpp).
 * 
 * Authored in the Communion of Saints Framework.
 * St. Thomas the Apostle, Patron of Verification & QA.
 * St. Peter the Apostle, Master Builder.
 */

#include "io.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Verify backward compatibility alias
namespace test_alias {
    using namespace iftext;
    static_assert(std::is_same_v<fictive::Style, iftext::Style>, "Style alias mismatch");
    static_assert(std::is_same_v<fictive::IOStream, iftext::IOStream>, "IOStream alias mismatch");
}

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "Assertion FAILED at line " << __LINE__ << ": " << msg << std::endl; \
            std::abort(); \
        } \
    } while (0)

static int g_testCount = 0;
static int g_assertCount = 0;

#define RUN_TEST(name) \
    do { \
        std::cout << "  [RUN ] " << #name << std::endl; \
        name(); \
        g_testCount++; \
        std::cout << "  [PASS] " << #name << std::endl; \
    } while (0)

#define ASSERT_EQ(actual, expected) \
    do { \
        g_assertCount++; \
        if ((actual) != (expected)) { \
            std::cerr << "FAILED: " << #actual << " == " << #expected \
                      << "\n  Actual:   [" << (actual) << "]" \
                      << "\n  Expected: [" << (expected) << "]" \
                      << "\n  Line:     " << __LINE__ << std::endl; \
            std::abort(); \
        } \
    } while (0)

#define ASSERT_TRUE(cond) \
    do { \
        g_assertCount++; \
        if (!(cond)) { \
            std::cerr << "FAILED: " << #cond << " is true\n  Line: " << __LINE__ << std::endl; \
            std::abort(); \
        } \
    } while (0)

#define ASSERT_FALSE(cond) \
    do { \
        g_assertCount++; \
        if (cond) { \
            std::cerr << "FAILED: " << #cond << " is false\n  Line: " << __LINE__ << std::endl; \
            std::abort(); \
        } \
    } while (0)

// ============================================================================
// TEST SUITE 1: STYLES AND ANSI ESCAPES
// ============================================================================

void test_styles_catalog() {
    using namespace fictive;

    // Reset
    ASSERT_EQ(styles.reset.ansi(), "\033[0m");
    ASSERT_EQ(Styles::reset.ansi(), "\033[0m");

    // Attributes
    ASSERT_EQ(styles.bold.ansi(), "\033[1m");
    ASSERT_EQ(styles.dim.ansi(), "\033[2m");
    ASSERT_EQ(styles.italic.ansi(), "\033[3m");
    ASSERT_EQ(styles.underline.ansi(), "\033[4m");

    // Standard Colors
    ASSERT_EQ(styles.black.ansi(), "\033[30m");
    ASSERT_EQ(styles.red.ansi(), "\033[31m");
    ASSERT_EQ(styles.green.ansi(), "\033[32m");
    ASSERT_EQ(styles.yellow.ansi(), "\033[33m");
    ASSERT_EQ(styles.blue.ansi(), "\033[34m");
    ASSERT_EQ(styles.magenta.ansi(), "\033[35m");
    ASSERT_EQ(styles.cyan.ansi(), "\033[36m");
    ASSERT_EQ(styles.white.ansi(), "\033[37m");

    // Bright Colors
    ASSERT_EQ(styles.brightBlack.ansi(), "\033[90m");
    ASSERT_EQ(styles.gray.ansi(), "\033[90m");
    ASSERT_EQ(styles.brightRed.ansi(), "\033[91m");
    ASSERT_EQ(styles.brightGreen.ansi(), "\033[92m");
    ASSERT_EQ(styles.brightYellow.ansi(), "\033[93m");
    ASSERT_EQ(styles.brightBlue.ansi(), "\033[94m");
    ASSERT_EQ(styles.brightMagenta.ansi(), "\033[95m");
    ASSERT_EQ(styles.brightCyan.ansi(), "\033[96m");
    ASSERT_EQ(styles.brightWhite.ansi(), "\033[97m");

    // Style comparison
    ASSERT_TRUE(styles.red == Styles::red);
    ASSERT_TRUE(styles.red != styles.blue);
    ASSERT_EQ(styles.brightBlack, styles.gray);

    // Ostream insertion
    std::ostringstream oss;
    oss << styles.red << "danger" << styles.reset;
    ASSERT_EQ(oss.str(), "\033[31mdanger\033[0m");
}

// ============================================================================
// TEST SUITE 2: STREAM OUTPUT (io and print)
// ============================================================================

void test_stream_output() {
    using namespace fictive;

    std::stringstream in;
    std::stringstream out;
    io.redirectStreams(in, out);

    // Test operator<< with styles, strings, numbers, manipulators
    io << styles.red << "Health: " << styles.reset << 100 << " / " << 100 << std::endl;
    ASSERT_EQ(out.str(), "\033[31mHealth: \033[0m100 / 100\n");

    out.str("");
    out.clear();

    // Test print object with operator<<
    print << styles.green << "Success: " << styles.reset << 42 << "\n";
    ASSERT_EQ(out.str(), "\033[32mSuccess: \033[0m42\n");

    out.str("");
    out.clear();

    // Test io.print and print.print
    io.print("Level ", 5, ", Score: ", 999);
    ASSERT_EQ(out.str(), "Level 5, Score: 999");

    out.str("");
    out.clear();

    // Test io.println and print.println
    io.println("Quest Complete!");
    ASSERT_EQ(out.str(), "Quest Complete!\n");

    out.str("");
    out.clear();

    print.println("Inventory items: ", 3);
    ASSERT_EQ(out.str(), "Inventory items: 3\n");

    out.str("");
    out.clear();

    // Test print callable functor syntax
    print("Coordinates: (", 10, ", ", 20, ")\n");
    ASSERT_EQ(out.str(), "Coordinates: (10, 20)\n");

    out.str("");
    out.clear();

    // Test free println function
    println("Free println works: ", true);
    ASSERT_EQ(out.str(), "Free println works: 1\n");

    // Reset streams
    io.resetStreams();
}

void test_output_color_toggle() {
    using namespace fictive;

    std::stringstream in;
    std::stringstream out;
    io.redirectStreams(in, out);

    // Disable colors
    io.setColorEnabled(false);
    ASSERT_FALSE(io.isColorEnabled());

    io << styles.red << "Warning" << styles.reset << std::endl;
    ASSERT_EQ(out.str(), "Warning\n");

    // Re-enable colors
    out.str("");
    out.clear();
    io.setColorEnabled(true);
    ASSERT_TRUE(io.isColorEnabled());

    io << styles.red << "Warning" << styles.reset << std::endl;
    ASSERT_EQ(out.str(), "\033[31mWarning\033[0m\n");

    io.resetStreams();
}

// ============================================================================
// TEST SUITE 3: STREAM INPUT (io and input)
// ============================================================================

void test_stream_input_line() {
    using namespace fictive;

    std::stringstream in("take brass key\nunlock iron door with key\nlook\n");
    std::stringstream out;
    io.redirectStreams(in, out);

    std::string cmd1;
    io >> cmd1;
    ASSERT_EQ(cmd1, "take brass key");

    std::string cmd2;
    input >> cmd2;
    ASSERT_EQ(cmd2, "unlock iron door with key");

    std::string cmd3;
    input >> cmd3;
    ASSERT_EQ(cmd3, "look");

    io.resetStreams();
}

void test_stream_input_readline_prompt() {
    using namespace fictive;

    std::stringstream in("go north\neast\n");
    std::stringstream out;
    io.redirectStreams(in, out);

    // readLine with prompt on io
    std::string cmd1 = io.readLine("What next? ");
    ASSERT_EQ(cmd1, "go north");
    ASSERT_EQ(out.str(), "What next? ");

    out.str("");
    out.clear();

    // readLine with prompt on input
    std::string cmd2 = input.readLine("> ");
    ASSERT_EQ(cmd2, "east");
    ASSERT_EQ(out.str(), "> ");

    out.str("");
    out.clear();

    // Free readLine function
    std::stringstream in2("examine lamp\n");
    io.redirectStreams(in2, out);
    std::string cmd3 = readLine("Command: ");
    ASSERT_EQ(cmd3, "examine lamp");
    ASSERT_EQ(out.str(), "Command: ");

    io.resetStreams();
}

void test_stream_input_extraction() {
    using namespace fictive;

    std::stringstream in("42 3.14159 dragon\n");
    std::stringstream out;
    io.redirectStreams(in, out);

    int num = 0;
    double pi = 0.0;
    io >> num;
    ASSERT_EQ(num, 42);

    input >> pi;
    ASSERT_TRUE(pi > 3.14 && pi < 3.15);

    std::string word;
    ASSERT_TRUE(io.readWord(word));
    ASSERT_EQ(word, "dragon");

    io.resetStreams();
}

void test_stream_input_eof_and_status() {
    using namespace fictive;

    std::stringstream in("single line\n");
    std::stringstream out;
    io.redirectStreams(in, out);

    ASSERT_TRUE(static_cast<bool>(io));
    ASSERT_TRUE(static_cast<bool>(input));

    std::string line;
    io >> line;
    ASSERT_EQ(line, "single line");

    // Reading at EOF
    std::string empty;
    io >> empty;
    ASSERT_EQ(empty, "");

    io.resetStreams();
}

// ============================================================================
// TEST SUITE 4: TARGET DRIVER ARCHITECTURE & HOOKS
// ============================================================================

void test_web_driver_hooks() {
    using namespace fictive;

    auto webDriver = std::make_shared<WebDriver>();
    ASSERT_TRUE(webDriver->platform() == TargetPlatform::Web);

    std::string capturedText;
    int capturedStyleCode = -1;

    webDriver->setWriteHook([&](std::string_view text, const Style* st) {
        if (!text.empty()) {
            capturedText += text;
        }
        if (st) {
            capturedStyleCode = static_cast<int>(st->getCode());
        }
    });

    webDriver->setReadLineHook([&](std::string_view prompt) -> std::string {
        if (!prompt.empty()) {
            capturedText += prompt;
        }
        return "web simulated response";
    });

    // Attach to an IOStream instance
    IOStream webIO(webDriver);

    webIO << styles.magenta << "Hello WebAssembly" << styles.reset;
    ASSERT_EQ(capturedText, "Hello WebAssembly");
    ASSERT_EQ(capturedStyleCode, static_cast<int>(Style::Code::Reset));

    std::string inputResult = webIO.readLine("Enter web action: ");
    ASSERT_EQ(inputResult, "web simulated response");
    ASSERT_TRUE(capturedText.find("Enter web action: ") != std::string::npos);
}

void test_desktop_driver_hooks() {
    using namespace fictive;

    auto desktopDriver = std::make_shared<DesktopDriver>();
    ASSERT_TRUE(desktopDriver->platform() == TargetPlatform::Desktop);

    std::string desktopOutput;
    std::string postedIpcMessage;

    desktopDriver->setWriteHook([&](std::string_view text, const Style*) {
        desktopOutput += text;
    });

    desktopDriver->setWebviewMessageHook([&](std::string_view msg) {
        postedIpcMessage = msg;
    });

    desktopDriver->setReadLineHook([&](std::string_view) -> std::string {
        return "desktop simulated input";
    });

    IOStream desktopIO(desktopDriver);
    desktopIO << "Hello Desktop Webview";
    ASSERT_EQ(desktopOutput, "Hello Desktop Webview");

    desktopDriver->postWebviewMessage(R"({"event":"status","hp":100})");
    ASSERT_EQ(postedIpcMessage, R"({"event":"status","hp":100})");

    std::string desktopInput = desktopIO.readLine("> ");
    ASSERT_EQ(desktopInput, "desktop simulated input");
}

void test_driver_switching() {
    using namespace fictive;

    // Start with terminal driver
    IOStream stream;
    ASSERT_TRUE(stream.getDriver()->platform() == TargetPlatform::Terminal);

    // Switch to web driver
    stream.setDriver(std::make_shared<WebDriver>());
    ASSERT_TRUE(stream.getDriver()->platform() == TargetPlatform::Web);

    // Switch to desktop driver
    stream.setDriver(std::make_shared<DesktopDriver>());
    ASSERT_TRUE(stream.getDriver()->platform() == TargetPlatform::Desktop);
}

// ============================================================================
// TEST SUITE 5: DEBUG UTILITY
// ============================================================================

void test_debug_utility_development() {
    using namespace fictive;

    std::stringstream in;
    std::stringstream out;
    io.redirectStreams(in, out);

    bool lambdaExecuted = false;
    debug.run([&]() {
        lambdaExecuted = true;
    });

#if defined(PRODUCTION) || defined(NDEBUG)
    // In production, lambda must NOT run
    ASSERT_FALSE(lambdaExecuted);
    debug << "This should not appear in production" << std::endl;
    ASSERT_EQ(out.str(), "");
#else
    // In development, lambda must run
    ASSERT_TRUE(lambdaExecuted);

    debug << "Debug info: " << 42 << std::endl;
    ASSERT_EQ(out.str(), "Debug info: 42\n");

    out.str("");
    out.clear();

    // Test debug mute/unmute
    debug.setEnabled(false);
    ASSERT_FALSE(debug.isEnabled());

    bool shouldNotRun = false;
    debug.run([&]() { shouldNotRun = true; });
    ASSERT_FALSE(shouldNotRun);

    debug << "Should be muted" << std::endl;
    ASSERT_EQ(out.str(), "");

    debug.setEnabled(true);
    ASSERT_TRUE(debug.isEnabled());
#endif

    io.resetStreams();
}

// ============================================================================
// TEST SUITE 6: INTEROPERABILITY WITH PARSER.HPP
// ============================================================================

#include "parser.hpp"

void test_interoperability_with_parser() {
    // Both fictive::Parser and iftext::Parser exist and work with fictive::io
    fictive::Parser parser;
    auto cmd = fictive::parse("take ancient golden sword");
    ASSERT_TRUE(cmd.isValid);
    ASSERT_EQ(cmd.verb, "take");
    ASSERT_EQ(cmd.directObjects.size(), 1);
    ASSERT_EQ(cmd.directObjects[0].head, "sword");

    auto cmd2 = parser.parseSentence("examine lamp");
    ASSERT_TRUE(cmd2.isValid);
    ASSERT_EQ(cmd2.verb, "examine");

    std::stringstream in;
    std::stringstream out;
    fictive::io.redirectStreams(in, out);

    fictive::io << fictive::styles.cyan << "Parsed verb: " << cmd.verb << fictive::styles.reset << std::endl;
    ASSERT_EQ(out.str(), "\033[36mParsed verb: take\033[0m\n");

    fictive::io.resetStreams();
}

// ============================================================================
// MAIN ENTRY POINT
// ============================================================================

int main() {
    std::cout << "======================================================================\n";
    std::cout << "Epistola Petri: Relentless Verification of fictive::io (io.hpp)\n";
    std::cout << "St. Peter the Apostle, Master Builder & Core Developer\n";
    std::cout << "\"Upon this rock I will build my engine.\"\n";
    std::cout << "======================================================================\n\n";

    RUN_TEST(test_styles_catalog);
    RUN_TEST(test_stream_output);
    RUN_TEST(test_output_color_toggle);
    RUN_TEST(test_stream_input_line);
    RUN_TEST(test_stream_input_readline_prompt);
    RUN_TEST(test_stream_input_extraction);
    RUN_TEST(test_stream_input_eof_and_status);
    RUN_TEST(test_web_driver_hooks);
    RUN_TEST(test_desktop_driver_hooks);
    RUN_TEST(test_driver_switching);
    RUN_TEST(test_debug_utility_development);
    RUN_TEST(test_interoperability_with_parser);

    std::cout << "\n======================================================================\n";
    std::cout << "IO TEST SUITE EXECUTION SUMMARY\n";
    std::cout << "======================================================================\n";
    std::cout << "  Total Test Cases Executed : " << g_testCount << "\n";
    std::cout << "  Total Assertions Verified : " << g_assertCount << "\n";
    std::cout << "======================================================================\n";
    std::cout << "VERDICT: SOLID ROCK FOUNDATION ESTABLISHED. ALL TESTS PASSED.\n";

    return 0;
}
