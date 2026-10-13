#include "parser.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <type_traits>
#include <functional>

// ============================================================================
// St. Thomas the Apostle Test Harness
// "Unless I see the test output, I will not believe."
// ============================================================================

namespace st_thomas {

size_t g_assertionsCount = 0;
size_t g_assertionsPassed = 0;
size_t g_assertionsFailed = 0;

size_t g_testsCount = 0;
size_t g_testsPassed = 0;
size_t g_testsFailed = 0;

bool g_currentTestFailed = false;
std::string g_currentTestName;

inline void beginTest(const std::string& name) {
    g_currentTestName = name;
    g_currentTestFailed = false;
    ++g_testsCount;
}

inline void endTest() {
    if (g_currentTestFailed) {
        ++g_testsFailed;
        std::cout << "  [FAIL] " << g_currentTestName << "\n";
    } else {
        ++g_testsPassed;
        std::cout << "  [PASS] " << g_currentTestName << "\n";
    }
}

template <typename T, typename U>
inline bool areEqual(const T& a, const U& b) {
    if constexpr (std::is_integral_v<T> && std::is_integral_v<U>) {
        if constexpr (std::is_signed_v<T> == std::is_signed_v<U>) {
            return a == b;
        } else if constexpr (std::is_signed_v<T>) {
            return a >= 0 && static_cast<std::make_unsigned_t<T>>(a) == static_cast<std::make_unsigned_t<U>>(b);
        } else {
            return b >= 0 && static_cast<std::make_unsigned_t<T>>(a) == static_cast<std::make_unsigned_t<U>>(b);
        }
    } else {
        return a == b;
    }
}

template <typename T, typename U>
inline void assertEqual(const T& actual, const U& expected, const char* exprActual, const char* exprExpected, const char* file, int line) {
    ++g_assertionsCount;
    if (areEqual(actual, expected)) {
        ++g_assertionsPassed;
    } else {
        ++g_assertionsFailed;
        g_currentTestFailed = true;
        std::cerr << "    FAILED ASSERTION at " << file << ":" << line << "\n"
                  << "      Condition: " << exprActual << " == " << exprExpected << "\n"
                  << "      Actual:    \"" << actual << "\"\n"
                  << "      Expected:  \"" << expected << "\"\n";
    }
}

template <typename T, typename U>
inline void assertNotEqual(const T& actual, const U& expected, const char* exprActual, const char* exprExpected, const char* file, int line) {
    ++g_assertionsCount;
    if (!areEqual(actual, expected)) {
        ++g_assertionsPassed;
    } else {
        ++g_assertionsFailed;
        g_currentTestFailed = true;
        std::cerr << "    FAILED ASSERTION at " << file << ":" << line << "\n"
                  << "      Condition: " << exprActual << " != " << exprExpected << "\n"
                  << "      Value:     \"" << actual << "\"\n";
    }
}

inline void assertTrue(bool condition, const char* expr, const char* file, int line) {
    ++g_assertionsCount;
    if (condition) {
        ++g_assertionsPassed;
    } else {
        ++g_assertionsFailed;
        g_currentTestFailed = true;
        std::cerr << "    FAILED ASSERTION at " << file << ":" << line << "\n"
                  << "      Expected TRUE: " << expr << "\n";
    }
}

inline void assertFalse(bool condition, const char* expr, const char* file, int line) {
    ++g_assertionsCount;
    if (!condition) {
        ++g_assertionsPassed;
    } else {
        ++g_assertionsFailed;
        g_currentTestFailed = true;
        std::cerr << "    FAILED ASSERTION at " << file << ":" << line << "\n"
                  << "      Expected FALSE: " << expr << "\n";
    }
}

inline void runTest(const std::string& name, const std::function<void()>& body) {
    beginTest(name);
    try {
        body();
    } catch (const std::exception& e) {
        g_currentTestFailed = true;
        std::cerr << "    UNCAUGHT EXCEPTION in [" << name << "]: " << e.what() << "\n";
    } catch (...) {
        g_currentTestFailed = true;
        std::cerr << "    UNKNOWN EXCEPTION in [" << name << "]\n";
    }
    endTest();
}

} // namespace st_thomas

#define ASSERT_TRUE(cond) st_thomas::assertTrue((cond), #cond, __FILE__, __LINE__)
#define ASSERT_FALSE(cond) st_thomas::assertFalse((cond), #cond, __FILE__, __LINE__)
#define ASSERT_EQ(actual, expected) st_thomas::assertEqual((actual), (expected), #actual, #expected, __FILE__, __LINE__)
#define ASSERT_NE(actual, expected) st_thomas::assertNotEqual((actual), (expected), #actual, #expected, __FILE__, __LINE__)

using namespace iftext;

// ============================================================================
// Test Suite Implementations
// ============================================================================

void runSuite1_IntransitiveVerbs() {
    std::cout << "\n--- SUITE 1: Intransitive Verbs & Standard Commands ---\n";

    st_thomas::runTest("Intransitive look commands ('look', 'l')", []() {
        auto c1 = parse("look");
        ASSERT_TRUE(c1.valid);
        ASSERT_TRUE(c1.isValid);
        ASSERT_EQ(c1.verb, "look");
        ASSERT_EQ(c1.rawVerb, "look");
        ASSERT_FALSE(c1.hasDirectObject());
        ASSERT_FALSE(c1.hasPreposition());
        ASSERT_TRUE(c1.directObjects.empty());
        ASSERT_TRUE(c1.prepositionalPhrases.empty());
        ASSERT_EQ(c1.error, "");

        auto c2 = parse("l");
        ASSERT_TRUE(c2.valid);
        ASSERT_EQ(c2.verb, "look");
        ASSERT_EQ(c2.rawVerb, "l");
        ASSERT_FALSE(c2.hasDirectObject());
    });

    st_thomas::runTest("Intransitive inventory commands ('inventory', 'inv', 'i')", []() {
        auto c1 = parse("inventory");
        ASSERT_TRUE(c1.valid);
        ASSERT_EQ(c1.verb, "inventory");
        ASSERT_EQ(c1.rawVerb, "inventory");
        ASSERT_FALSE(c1.hasDirectObject());

        auto c2 = parse("inv");
        ASSERT_TRUE(c2.valid);
        ASSERT_EQ(c2.verb, "inventory");
        ASSERT_EQ(c2.rawVerb, "inv");

        auto c3 = parse("i");
        ASSERT_TRUE(c3.valid);
        ASSERT_EQ(c3.verb, "inventory");
        ASSERT_EQ(c3.rawVerb, "i");
    });

    st_thomas::runTest("Intransitive wait commands ('wait', 'z')", []() {
        auto c1 = parse("wait");
        ASSERT_TRUE(c1.valid);
        ASSERT_EQ(c1.verb, "wait");
        ASSERT_EQ(c1.rawVerb, "wait");

        auto c2 = parse("z");
        ASSERT_TRUE(c2.valid);
        ASSERT_EQ(c2.verb, "wait");
        ASSERT_EQ(c2.rawVerb, "z");
    });

    st_thomas::runTest("Intransitive session commands ('help', 'quit', 'q', 'again', 'g', 'score', 'save', 'restore')", []() {
        auto ch = parse("help");
        ASSERT_TRUE(ch.valid);
        ASSERT_EQ(ch.verb, "help");

        auto cq1 = parse("quit");
        ASSERT_TRUE(cq1.valid);
        ASSERT_EQ(cq1.verb, "quit");

        auto cq2 = parse("q");
        ASSERT_TRUE(cq2.valid);
        ASSERT_EQ(cq2.verb, "quit");

        auto ca1 = parse("again");
        ASSERT_TRUE(ca1.valid);
        ASSERT_EQ(ca1.verb, "again");

        auto ca2 = parse("g");
        ASSERT_TRUE(ca2.valid);
        ASSERT_EQ(ca2.verb, "again");

        auto cs = parse("score");
        ASSERT_TRUE(cs.valid);
        ASSERT_EQ(cs.verb, "score");

        auto csv = parse("save");
        ASSERT_TRUE(csv.valid);
        ASSERT_EQ(csv.verb, "save");

        auto cr = parse("restore");
        ASSERT_TRUE(cr.valid);
        ASSERT_EQ(cr.verb, "restore");
    });

    st_thomas::runTest("Negative: Transitive verbs with missing direct objects", []() {
        auto c_take = parse("take");
        ASSERT_FALSE(c_take.valid);
        ASSERT_FALSE(c_take.isValid);
        ASSERT_EQ(c_take.error, "What do you want to take?");
        ASSERT_EQ(c_take.errorMessage, c_take.error);

        auto c_get = parse("get");
        ASSERT_FALSE(c_get.valid);
        ASSERT_EQ(c_get.error, "What do you want to take?");

        auto c_grab = parse("grab");
        ASSERT_FALSE(c_grab.valid);
        ASSERT_EQ(c_grab.error, "What do you want to take?");

        auto c_pickup = parse("pickup");
        ASSERT_FALSE(c_pickup.valid);
        ASSERT_EQ(c_pickup.error, "What do you want to take?");

        auto c_drop = parse("drop");
        ASSERT_FALSE(c_drop.valid);
        ASSERT_EQ(c_drop.error, "What do you want to drop?");

        auto c_discard = parse("discard");
        ASSERT_FALSE(c_discard.valid);
        ASSERT_EQ(c_discard.error, "What do you want to drop?");

        auto c_examine = parse("examine");
        ASSERT_FALSE(c_examine.valid);
        ASSERT_EQ(c_examine.error, "What do you want to examine?");

        auto c_x = parse("x");
        ASSERT_FALSE(c_x.valid);
        ASSERT_EQ(c_x.error, "What do you want to examine?");

        auto c_inspect = parse("inspect");
        ASSERT_FALSE(c_inspect.valid);
        ASSERT_EQ(c_inspect.error, "What do you want to examine?");

        auto c_unlock = parse("unlock");
        ASSERT_FALSE(c_unlock.valid);
        ASSERT_EQ(c_unlock.error, "What do you want to unlock?");

        auto c_lock = parse("lock");
        ASSERT_FALSE(c_lock.valid);
        ASSERT_EQ(c_lock.error, "What do you want to lock?");

        auto c_open = parse("open");
        ASSERT_FALSE(c_open.valid);
        ASSERT_EQ(c_open.error, "What do you want to open?");

        auto c_close = parse("close");
        ASSERT_FALSE(c_close.valid);
        ASSERT_EQ(c_close.error, "What do you want to close?");

        auto c_put = parse("put");
        ASSERT_FALSE(c_put.valid);
        ASSERT_EQ(c_put.error, "What do you want to put, and where?");

        auto c_give = parse("give");
        ASSERT_FALSE(c_give.valid);
        ASSERT_EQ(c_give.error, "What do you want to give, and to whom?");

        auto c_oops = parse("oops");
        ASSERT_FALSE(c_oops.valid);
        ASSERT_EQ(c_oops.error, "What word do you want to correct?");

        auto c_o = parse("o");
        ASSERT_FALSE(c_o.valid);
        ASSERT_EQ(c_o.error, "What word do you want to correct?");
    });
}

void runSuite2_Directions() {
    std::cout << "\n--- SUITE 2: Cardinal & Ordinal Directions ---\n";

    st_thomas::runTest("Cardinal directions abbreviation ('n', 's', 'e', 'w')", []() {
        struct TestCase { std::string in; std::string canon; };
        std::vector<TestCase> cases = {
            {"n", "north"}, {"s", "south"}, {"e", "east"}, {"w", "west"}
        };
        for (const auto& tc : cases) {
            auto cmd = parse(tc.in);
            ASSERT_TRUE(cmd.valid);
            ASSERT_EQ(cmd.verb, tc.canon);
            ASSERT_EQ(cmd.rawVerb, tc.in);
            ASSERT_TRUE(cmd.isDirection());
            ASSERT_EQ(cmd.getDirection(), tc.canon);
            ASSERT_TRUE(cmd.directObjects.empty());
        }
    });

    st_thomas::runTest("Cardinal directions full names ('north', 'south', 'east', 'west')", []() {
        std::vector<std::string> dirs = {"north", "south", "east", "west"};
        for (const auto& d : dirs) {
            auto cmd = parse(d);
            ASSERT_TRUE(cmd.valid);
            ASSERT_EQ(cmd.verb, d);
            ASSERT_EQ(cmd.rawVerb, d);
            ASSERT_TRUE(cmd.isDirection());
            ASSERT_EQ(cmd.getDirection(), d);
        }
    });

    st_thomas::runTest("Ordinal directions abbreviation ('ne', 'nw', 'se', 'sw')", []() {
        struct TestCase { std::string in; std::string canon; };
        std::vector<TestCase> cases = {
            {"ne", "northeast"}, {"nw", "northwest"}, {"se", "southeast"}, {"sw", "southwest"}
        };
        for (const auto& tc : cases) {
            auto cmd = parse(tc.in);
            ASSERT_TRUE(cmd.valid);
            ASSERT_EQ(cmd.verb, tc.canon);
            ASSERT_EQ(cmd.rawVerb, tc.in);
            ASSERT_TRUE(cmd.isDirection());
            ASSERT_EQ(cmd.getDirection(), tc.canon);
        }
    });

    st_thomas::runTest("Ordinal directions full names ('northeast', 'northwest', 'southeast', 'southwest')", []() {
        std::vector<std::string> dirs = {"northeast", "northwest", "southeast", "southwest"};
        for (const auto& d : dirs) {
            auto cmd = parse(d);
            ASSERT_TRUE(cmd.valid);
            ASSERT_EQ(cmd.verb, d);
            ASSERT_TRUE(cmd.isDirection());
            ASSERT_EQ(cmd.getDirection(), d);
        }
    });

    st_thomas::runTest("Vertical and Interior directions ('u', 'd', 'up', 'down', 'in', 'inside', 'out', 'outside')", []() {
        struct TestCase { std::string in; std::string canon; };
        std::vector<TestCase> cases = {
            {"u", "up"}, {"up", "up"},
            {"d", "down"}, {"down", "down"},
            {"in", "in"}, {"inside", "in"},
            {"out", "out"}, {"outside", "out"}
        };
        for (const auto& tc : cases) {
            auto cmd = parse(tc.in);
            ASSERT_TRUE(cmd.valid);
            ASSERT_EQ(cmd.verb, tc.canon);
            ASSERT_TRUE(cmd.isDirection());
            ASSERT_EQ(cmd.getDirection(), tc.canon);
        }
    });

    st_thomas::runTest("Movement verbs with directions ('go north', 'go s', 'go ne', 'go in', 'go up')", []() {
        struct TestCase { std::string in; std::string canon; };
        std::vector<TestCase> cases = {
            {"go north", "north"}, {"go s", "south"},
            {"go east", "east"},   {"go w", "west"},
            {"go ne", "northeast"},{"go nw", "northwest"},
            {"go se", "southeast"},{"go sw", "southwest"},
            {"go up", "up"},       {"go d", "down"},
            {"go in", "in"},       {"go inside", "in"},
            {"go out", "out"},     {"go outside", "out"}
        };
        for (const auto& tc : cases) {
            auto cmd = parse(tc.in);
            ASSERT_TRUE(cmd.valid);
            ASSERT_EQ(cmd.verb, "go");
            ASSERT_TRUE(cmd.isDirection());
            ASSERT_EQ(cmd.getDirection(), tc.canon);
            ASSERT_TRUE(cmd.hasDirectObject());
            ASSERT_EQ(cmd.directObjects[0].head, tc.canon);
        }
    });

    st_thomas::runTest("Alternative movement verbs ('walk south', 'run east')", []() {
        auto cmd_walk = parse("walk south");
        ASSERT_TRUE(cmd_walk.valid);
        ASSERT_EQ(cmd_walk.verb, "walk");
        ASSERT_TRUE(cmd_walk.hasDirectObject());
        ASSERT_EQ(cmd_walk.directObjects[0].head, "south");

        auto cmd_run = parse("run east");
        ASSERT_TRUE(cmd_run.valid);
        ASSERT_EQ(cmd_run.verb, "run");
        ASSERT_TRUE(cmd_run.hasDirectObject());
        ASSERT_EQ(cmd_run.directObjects[0].head, "east");
    });

    st_thomas::runTest("Non-directional commands do not trigger isDirection()", []() {
        auto c1 = parse("look north");
        ASSERT_TRUE(c1.valid);
        ASSERT_EQ(c1.verb, "look");
        ASSERT_FALSE(c1.isDirection());
        ASSERT_EQ(c1.getDirection(), "");

        auto c2 = parse("examine chest");
        ASSERT_TRUE(c2.valid);
        ASSERT_FALSE(c2.isDirection());

        auto c3 = parse("go castle"); // "castle" is not a direction word
        ASSERT_TRUE(c3.valid);
        ASSERT_EQ(c3.verb, "go");
        ASSERT_FALSE(c3.isDirection()); // isDirection returns false because castle is not a direction
        ASSERT_EQ(c3.getDirection(), "castle"); // getDirection returns directObjects[0].head for go

        auto c4 = parse("take sword");
        ASSERT_TRUE(c4.valid);
        ASSERT_FALSE(c4.isDirection());
        ASSERT_EQ(c4.getDirection(), "");
    });
}

void runSuite3_TransitiveAndNounPhrases() {
    std::cout << "\n--- SUITE 3: Transitive Actions & Noun Phrase Extraction ---\n";

    st_thomas::runTest("Bare noun extraction ('take lamp')", []() {
        auto cmd = parse("take lamp");
        ASSERT_TRUE(cmd.valid);
        ASSERT_EQ(cmd.verb, "take");
        ASSERT_EQ(cmd.rawVerb, "take");
        ASSERT_TRUE(cmd.hasDirectObject());
        ASSERT_FALSE(cmd.isMultiObject());
        ASSERT_EQ(cmd.directObjects.size(), 1);

        const auto& np = cmd.directObjects[0];
        ASSERT_EQ(np.raw, "lamp");
        ASSERT_EQ(np.head, "lamp");
        ASSERT_EQ(np.determiner, "");
        ASSERT_EQ(np.article, "");
        ASSERT_TRUE(np.adjectives.empty());
        ASSERT_FALSE(np.isPronoun);
        ASSERT_FALSE(np.isAll);
        ASSERT_FALSE(np.empty());

        ASSERT_NE(cmd.getDirectObject(), nullptr);
        ASSERT_EQ(cmd.getDirectObject()->head, "lamp");
    });

    st_thomas::runTest("Determiners ('the', 'a', 'an', 'some')", []() {
        auto c1 = parse("take the lamp");
        ASSERT_TRUE(c1.valid);
        ASSERT_EQ(c1.directObjects[0].determiner, "the");
        ASSERT_EQ(c1.directObjects[0].article, "the");
        ASSERT_EQ(c1.directObjects[0].head, "lamp");
        ASSERT_EQ(c1.directObjects[0].raw, "the lamp");

        auto c2 = parse("read a book");
        ASSERT_TRUE(c2.valid);
        ASSERT_EQ(c2.verb, "read");
        ASSERT_EQ(c2.directObjects[0].determiner, "a");
        ASSERT_EQ(c2.directObjects[0].head, "book");

        auto c3 = parse("eat an apple");
        ASSERT_TRUE(c3.valid);
        ASSERT_EQ(c3.verb, "eat");
        ASSERT_EQ(c3.directObjects[0].determiner, "an");
        ASSERT_EQ(c3.directObjects[0].head, "apple");

        auto c4 = parse("drink some water");
        ASSERT_TRUE(c4.valid);
        ASSERT_EQ(c4.verb, "drink");
        ASSERT_EQ(c4.directObjects[0].determiner, "some");
        ASSERT_EQ(c4.directObjects[0].head, "water");
    });

    st_thomas::runTest("Multi-adjective noun phrase ('examine the ancient brass key')", []() {
        auto cmd = parse("examine the ancient brass key");
        ASSERT_TRUE(cmd.valid);
        ASSERT_EQ(cmd.verb, "examine");
        ASSERT_TRUE(cmd.hasDirectObject());
        ASSERT_EQ(cmd.directObjects.size(), 1);

        const auto& np = cmd.directObjects[0];
        ASSERT_EQ(np.raw, "the ancient brass key");
        ASSERT_EQ(np.determiner, "the");
        ASSERT_EQ(np.head, "key");
        ASSERT_EQ(np.adjectives.size(), 2);
        ASSERT_EQ(np.adjectives[0], "ancient");
        ASSERT_EQ(np.adjectives[1], "brass");
        ASSERT_FALSE(np.isPronoun);
        ASSERT_FALSE(np.isAll);
    });

    st_thomas::runTest("Adjectives without determiners ('take heavy iron sword')", []() {
        auto cmd = parse("take heavy iron sword");
        ASSERT_TRUE(cmd.valid);
        ASSERT_EQ(cmd.verb, "take");
        ASSERT_EQ(cmd.directObjects.size(), 1);

        const auto& np = cmd.directObjects[0];
        ASSERT_EQ(np.raw, "heavy iron sword");
        ASSERT_EQ(np.determiner, "");
        ASSERT_EQ(np.head, "sword");
        ASSERT_EQ(np.adjectives.size(), 2);
        ASSERT_EQ(np.adjectives[0], "heavy");
        ASSERT_EQ(np.adjectives[1], "iron");
    });

    st_thomas::runTest("Triple adjectives ('touch glowing magical crystal orb')", []() {
        auto cmd = parse("touch glowing magical crystal orb");
        ASSERT_TRUE(cmd.valid);
        ASSERT_EQ(cmd.verb, "touch");
        const auto& np = cmd.directObjects[0];
        ASSERT_EQ(np.head, "orb");
        ASSERT_EQ(np.adjectives.size(), 3);
        ASSERT_EQ(np.adjectives[0], "glowing");
        ASSERT_EQ(np.adjectives[1], "magical");
        ASSERT_EQ(np.adjectives[2], "crystal");
    });

    st_thomas::runTest("Synonym verbs mapping to canonical ('get', 'grab', 'pickup' -> 'take')", []() {
        auto c1 = parse("get golden chalice");
        ASSERT_TRUE(c1.valid);
        ASSERT_EQ(c1.verb, "take");
        ASSERT_EQ(c1.rawVerb, "get");
        ASSERT_EQ(c1.directObjects[0].head, "chalice");

        auto c2 = parse("grab rusty dagger");
        ASSERT_TRUE(c2.valid);
        ASSERT_EQ(c2.verb, "take");
        ASSERT_EQ(c2.rawVerb, "grab");
        ASSERT_EQ(c2.directObjects[0].head, "dagger");

        auto c3 = parse("pickup wooden staff");
        ASSERT_TRUE(c3.valid);
        ASSERT_EQ(c3.verb, "take");
        ASSERT_EQ(c3.rawVerb, "pickup");
        ASSERT_EQ(c3.directObjects[0].head, "staff");
    });

    st_thomas::runTest("Synonym verbs mapping to canonical ('x', 'inspect' -> 'examine')", []() {
        auto c1 = parse("x mirror");
        ASSERT_TRUE(c1.valid);
        ASSERT_EQ(c1.verb, "examine");
        ASSERT_EQ(c1.rawVerb, "x");
        ASSERT_EQ(c1.directObjects[0].head, "mirror");

        auto c2 = parse("inspect tapestry");
        ASSERT_TRUE(c2.valid);
        ASSERT_EQ(c2.verb, "examine");
        ASSERT_EQ(c2.rawVerb, "inspect");
        ASSERT_EQ(c2.directObjects[0].head, "tapestry");
    });

    st_thomas::runTest("Synonym verbs mapping to canonical ('discard' -> 'drop')", []() {
        auto c1 = parse("discard skull");
        ASSERT_TRUE(c1.valid);
        ASSERT_EQ(c1.verb, "drop");
        ASSERT_EQ(c1.rawVerb, "discard");
        ASSERT_EQ(c1.directObjects[0].head, "skull");
    });
}

void runSuite4_PrepositionalPhrases() {
    std::cout << "\n--- SUITE 4: Prepositional Phrases ---\n";

    st_thomas::runTest("Single-word prepositions without direct object", []() {
        auto c1 = parse("look in chest");
        ASSERT_TRUE(c1.valid);
        ASSERT_EQ(c1.verb, "look");
        ASSERT_FALSE(c1.hasDirectObject());
        ASSERT_TRUE(c1.hasPreposition());
        ASSERT_EQ(c1.prepositionalPhrases.size(), 1);
        ASSERT_TRUE(c1.prepositionalPhrases[0].type == PrepositionType::IN);
        ASSERT_EQ(c1.prepositionalPhrases[0].prepWord, "in");
        ASSERT_EQ(c1.prepositionalPhrases[0].object.head, "chest");

        auto c2 = parse("look under bed");
        ASSERT_TRUE(c2.valid);
        ASSERT_TRUE(c2.prepositionalPhrases[0].type == PrepositionType::UNDER);
        ASSERT_EQ(c2.prepositionalPhrases[0].prepWord, "under");
        ASSERT_EQ(c2.prepositionalPhrases[0].object.head, "bed");

        auto c3 = parse("climb through window");
        ASSERT_TRUE(c3.valid);
        ASSERT_EQ(c3.verb, "climb");
        ASSERT_TRUE(c3.prepositionalPhrases[0].type == PrepositionType::THROUGH);
        ASSERT_EQ(c3.prepositionalPhrases[0].prepWord, "through");
        ASSERT_EQ(c3.prepositionalPhrases[0].object.head, "window");

        auto c4 = parse("crawl thru tunnel");
        ASSERT_TRUE(c4.valid);
        ASSERT_TRUE(c4.prepositionalPhrases[0].type == PrepositionType::THROUGH);
        ASSERT_EQ(c4.prepositionalPhrases[0].prepWord, "thru");
        ASSERT_EQ(c4.prepositionalPhrases[0].object.head, "tunnel");

        auto c5 = parse("jump into portal");
        ASSERT_TRUE(c5.valid);
        ASSERT_TRUE(c5.prepositionalPhrases[0].type == PrepositionType::INTO);
        ASSERT_EQ(c5.prepositionalPhrases[0].prepWord, "into");
        ASSERT_EQ(c5.prepositionalPhrases[0].object.head, "portal");

        auto c6 = parse("stand behind column");
        ASSERT_TRUE(c6.valid);
        ASSERT_TRUE(c6.prepositionalPhrases[0].type == PrepositionType::BEHIND);
        ASSERT_EQ(c6.prepositionalPhrases[0].prepWord, "behind");
        ASSERT_EQ(c6.prepositionalPhrases[0].object.head, "column");

        auto c7 = parse("smile at merchant");
        ASSERT_TRUE(c7.valid);
        ASSERT_TRUE(c7.prepositionalPhrases[0].type == PrepositionType::AT);
        ASSERT_EQ(c7.prepositionalPhrases[0].prepWord, "at");
        ASSERT_EQ(c7.prepositionalPhrases[0].object.head, "merchant");

        auto c8 = parse("jump off cliff");
        ASSERT_TRUE(c8.valid);
        ASSERT_TRUE(c8.prepositionalPhrases[0].type == PrepositionType::OFF);
        ASSERT_EQ(c8.prepositionalPhrases[0].prepWord, "off");
        ASSERT_EQ(c8.prepositionalPhrases[0].object.head, "cliff");

        auto c9 = parse("run to temple");
        ASSERT_TRUE(c9.valid);
        ASSERT_TRUE(c9.prepositionalPhrases[0].type == PrepositionType::TO);
        ASSERT_EQ(c9.prepositionalPhrases[0].prepWord, "to");
        ASSERT_EQ(c9.prepositionalPhrases[0].object.head, "temple");

        auto c10 = parse("escape from dungeon");
        ASSERT_TRUE(c10.valid);
        ASSERT_TRUE(c10.prepositionalPhrases[0].type == PrepositionType::FROM);
        ASSERT_EQ(c10.prepositionalPhrases[0].prepWord, "from");
        ASSERT_EQ(c10.prepositionalPhrases[0].object.head, "dungeon");

        auto c11 = parse("talk about quest");
        ASSERT_TRUE(c11.valid);
        ASSERT_TRUE(c11.prepositionalPhrases[0].type == PrepositionType::ABOUT);
        ASSERT_EQ(c11.prepositionalPhrases[0].prepWord, "about");
        ASSERT_EQ(c11.prepositionalPhrases[0].object.head, "quest");

        auto c12 = parse("hide beneath blanket");
        ASSERT_TRUE(c12.valid);
        ASSERT_TRUE(c12.prepositionalPhrases[0].type == PrepositionType::UNDER);
        ASSERT_EQ(c12.prepositionalPhrases[0].prepWord, "beneath");
        ASSERT_EQ(c12.prepositionalPhrases[0].object.head, "blanket");

        auto c13 = parse("rest underneath tree");
        ASSERT_TRUE(c13.valid);
        ASSERT_TRUE(c13.prepositionalPhrases[0].type == PrepositionType::UNDER);
        ASSERT_EQ(c13.prepositionalPhrases[0].prepWord, "underneath");
        ASSERT_EQ(c13.prepositionalPhrases[0].object.head, "tree");

        auto c14 = parse("kneel upon altar");
        ASSERT_TRUE(c14.valid);
        ASSERT_TRUE(c14.prepositionalPhrases[0].type == PrepositionType::ON);
        ASSERT_EQ(c14.prepositionalPhrases[0].prepWord, "upon");
        ASSERT_EQ(c14.prepositionalPhrases[0].object.head, "altar");
    });

    st_thomas::runTest("Multi-word prepositions ('on top of', 'in front of', 'out of', 'inside of', 'along with', 'together with')", []() {
        auto c1 = parse("put box on top of table");
        ASSERT_TRUE(c1.valid);
        ASSERT_EQ(c1.verb, "put");
        ASSERT_TRUE(c1.hasDirectObject());
        ASSERT_EQ(c1.directObjects[0].head, "box");
        ASSERT_TRUE(c1.hasPreposition());
        ASSERT_TRUE(c1.prepositionalPhrases[0].type == PrepositionType::ON);
        ASSERT_EQ(c1.prepositionalPhrases[0].prepWord, "on top of");
        ASSERT_EQ(c1.prepositionalPhrases[0].object.head, "table");

        auto c2 = parse("stand in front of statue");
        ASSERT_TRUE(c2.valid);
        ASSERT_EQ(c2.verb, "stand");
        ASSERT_FALSE(c2.hasDirectObject());
        ASSERT_TRUE(c2.prepositionalPhrases[0].type == PrepositionType::IN);
        ASSERT_EQ(c2.prepositionalPhrases[0].prepWord, "in front of");
        ASSERT_EQ(c2.prepositionalPhrases[0].object.head, "statue");

        auto c3 = parse("pull rabbit out of hat");
        ASSERT_TRUE(c3.valid);
        ASSERT_EQ(c3.verb, "pull");
        ASSERT_EQ(c3.directObjects[0].head, "rabbit");
        ASSERT_TRUE(c3.prepositionalPhrases[0].type == PrepositionType::OUT_OF);
        ASSERT_EQ(c3.prepositionalPhrases[0].prepWord, "out of");
        ASSERT_EQ(c3.prepositionalPhrases[0].object.head, "hat");

        auto c4 = parse("climb inside of barrel");
        ASSERT_TRUE(c4.valid);
        ASSERT_TRUE(c4.prepositionalPhrases[0].type == PrepositionType::IN);
        ASSERT_EQ(c4.prepositionalPhrases[0].prepWord, "inside of");
        ASSERT_EQ(c4.prepositionalPhrases[0].object.head, "barrel");

        auto c5 = parse("bring dog along with cat");
        ASSERT_TRUE(c5.valid);
        ASSERT_EQ(c5.directObjects[0].head, "dog");
        ASSERT_TRUE(c5.prepositionalPhrases[0].type == PrepositionType::WITH);
        ASSERT_EQ(c5.prepositionalPhrases[0].prepWord, "along with");
        ASSERT_EQ(c5.prepositionalPhrases[0].object.head, "cat");

        auto c6 = parse("eat bread together with cheese");
        ASSERT_TRUE(c6.valid);
        ASSERT_EQ(c6.directObjects[0].head, "bread");
        ASSERT_TRUE(c6.prepositionalPhrases[0].type == PrepositionType::WITH);
        ASSERT_EQ(c6.prepositionalPhrases[0].prepWord, "together with");
        ASSERT_EQ(c6.prepositionalPhrases[0].object.head, "cheese");
    });

    st_thomas::runTest("Negative: Incomplete prepositional phrases missing their object", []() {
        auto c1 = parse("put coin in");
        ASSERT_FALSE(c1.valid);
        ASSERT_EQ(c1.error, "What do you want to put coin in?");

        auto c_into = parse("put box into");
        ASSERT_FALSE(c_into.valid);
        ASSERT_EQ(c_into.error, "What do you want to put box into?");

        auto c2 = parse("look in");
        ASSERT_FALSE(c2.valid);
        ASSERT_EQ(c2.error, "What do you want to look in?");

        auto c3 = parse("look under");
        ASSERT_FALSE(c3.valid);
        ASSERT_EQ(c3.error, "What do you want to look under?");

        auto c4 = parse("climb through");
        ASSERT_FALSE(c4.valid);
        ASSERT_EQ(c4.error, "What do you want to climb through?");
    });
}

void runSuite5_TwoObjectAndDative() {
    std::cout << "\n--- SUITE 5: Two-Object / Dative / Instrument Constructions ---\n";

    st_thomas::runTest("Explicit instrument constructions with 'with' or 'using'", []() {
        auto c1 = parse("unlock door with silver key");
        ASSERT_TRUE(c1.valid);
        ASSERT_EQ(c1.verb, "unlock");
        ASSERT_EQ(c1.directObjects.size(), 1);
        ASSERT_EQ(c1.directObjects[0].head, "door");
        ASSERT_EQ(c1.prepositionalPhrases.size(), 1);
        ASSERT_TRUE(c1.prepositionalPhrases[0].type == PrepositionType::WITH);
        ASSERT_EQ(c1.prepositionalPhrases[0].prepWord, "with");
        ASSERT_EQ(c1.prepositionalPhrases[0].object.head, "key");
        ASSERT_EQ(c1.prepositionalPhrases[0].object.adjectives.size(), 1);
        ASSERT_EQ(c1.prepositionalPhrases[0].object.adjectives[0], "silver");

        auto c2 = parse("pry lid using iron crowbar");
        ASSERT_TRUE(c2.valid);
        ASSERT_EQ(c2.verb, "pry");
        ASSERT_EQ(c2.directObjects[0].head, "lid");
        ASSERT_TRUE(c2.prepositionalPhrases[0].type == PrepositionType::WITH);
        ASSERT_EQ(c2.prepositionalPhrases[0].prepWord, "using");
        ASSERT_EQ(c2.prepositionalPhrases[0].object.head, "crowbar");
        ASSERT_EQ(c2.prepositionalPhrases[0].object.adjectives[0], "iron");
    });

    st_thomas::runTest("Explicit preposition recipient ('give red apple to old guard')", []() {
        auto cmd = parse("give red apple to old guard");
        ASSERT_TRUE(cmd.valid);
        ASSERT_EQ(cmd.verb, "give");
        ASSERT_EQ(cmd.directObjects.size(), 1);
        ASSERT_EQ(cmd.directObjects[0].head, "apple");
        ASSERT_EQ(cmd.directObjects[0].adjectives.size(), 1);
        ASSERT_EQ(cmd.directObjects[0].adjectives[0], "red");

        ASSERT_EQ(cmd.prepositionalPhrases.size(), 1);
        ASSERT_TRUE(cmd.prepositionalPhrases[0].type == PrepositionType::TO);
        ASSERT_EQ(cmd.prepositionalPhrases[0].prepWord, "to");
        ASSERT_EQ(cmd.prepositionalPhrases[0].object.head, "guard");
        ASSERT_EQ(cmd.prepositionalPhrases[0].object.adjectives.size(), 1);
        ASSERT_EQ(cmd.prepositionalPhrases[0].object.adjectives[0], "old");
    });

    st_thomas::runTest("Dative alternation: 'give guard apple'", []() {
        auto cmd = parse("give guard apple");
        ASSERT_TRUE(cmd.valid);
        ASSERT_EQ(cmd.verb, "give");
        // Canonical mapping: apple is direct object, guard is recipient governed by TO
        ASSERT_EQ(cmd.directObjects.size(), 1);
        ASSERT_EQ(cmd.directObjects[0].head, "apple");
        ASSERT_EQ(cmd.prepositionalPhrases.size(), 1);
        ASSERT_TRUE(cmd.prepositionalPhrases[0].type == PrepositionType::TO);
        ASSERT_EQ(cmd.prepositionalPhrases[0].prepWord, "to");
        ASSERT_EQ(cmd.prepositionalPhrases[0].object.head, "guard");
    });

    st_thomas::runTest("Dative alternation with determiners: 'give the guard the coin'", []() {
        auto cmd = parse("give the guard the coin");
        ASSERT_TRUE(cmd.valid);
        ASSERT_EQ(cmd.verb, "give");
        ASSERT_EQ(cmd.directObjects.size(), 1);
        ASSERT_EQ(cmd.directObjects[0].head, "coin");
        ASSERT_EQ(cmd.directObjects[0].determiner, "the");
        ASSERT_EQ(cmd.prepositionalPhrases.size(), 1);
        ASSERT_TRUE(cmd.prepositionalPhrases[0].type == PrepositionType::TO);
        ASSERT_EQ(cmd.prepositionalPhrases[0].object.head, "guard");
        ASSERT_EQ(cmd.prepositionalPhrases[0].object.determiner, "the");
    });

    st_thomas::runTest("Dative alternation with adjectives: 'give the old guard red apple'", []() {
        auto cmd = parse("give the old guard red apple");
        ASSERT_TRUE(cmd.valid);
        ASSERT_EQ(cmd.verb, "give");
        ASSERT_EQ(cmd.directObjects.size(), 1);
        ASSERT_EQ(cmd.directObjects[0].head, "apple");
        ASSERT_EQ(cmd.directObjects[0].adjectives.size(), 1);
        ASSERT_EQ(cmd.directObjects[0].adjectives[0], "red");

        ASSERT_EQ(cmd.prepositionalPhrases.size(), 1);
        ASSERT_TRUE(cmd.prepositionalPhrases[0].type == PrepositionType::TO);
        ASSERT_EQ(cmd.prepositionalPhrases[0].object.head, "guard");
        ASSERT_EQ(cmd.prepositionalPhrases[0].object.determiner, "the");
        ASSERT_EQ(cmd.prepositionalPhrases[0].object.adjectives.size(), 1);
        ASSERT_EQ(cmd.prepositionalPhrases[0].object.adjectives[0], "old");
    });

    st_thomas::runTest("Dative alternation with pronoun recipient: 'give him food'", []() {
        auto cmd = parse("give him food");
        ASSERT_TRUE(cmd.valid);
        ASSERT_EQ(cmd.verb, "give");
        ASSERT_EQ(cmd.directObjects.size(), 1);
        ASSERT_EQ(cmd.directObjects[0].head, "food");

        ASSERT_EQ(cmd.prepositionalPhrases.size(), 1);
        ASSERT_TRUE(cmd.prepositionalPhrases[0].type == PrepositionType::TO);
        ASSERT_TRUE(cmd.prepositionalPhrases[0].object.isPronoun);
        ASSERT_TRUE(cmd.prepositionalPhrases[0].object.pronoun == PronounType::HIM);
        ASSERT_EQ(cmd.prepositionalPhrases[0].object.head, "him");
    });

    st_thomas::runTest("Other dative verbs ('hand', 'feed', 'show', 'tell', 'pay')", []() {
        auto c_hand = parse("hand merchant gold");
        ASSERT_TRUE(c_hand.valid);
        ASSERT_EQ(c_hand.verb, "hand");
        ASSERT_EQ(c_hand.directObjects[0].head, "gold");
        ASSERT_EQ(c_hand.prepositionalPhrases[0].object.head, "merchant");

        auto c_feed = parse("feed dog meat");
        ASSERT_TRUE(c_feed.valid);
        ASSERT_EQ(c_feed.verb, "feed");
        ASSERT_EQ(c_feed.directObjects[0].head, "meat");
        ASSERT_EQ(c_feed.prepositionalPhrases[0].object.head, "dog");

        auto c_show = parse("show king map");
        ASSERT_TRUE(c_show.valid);
        ASSERT_EQ(c_show.verb, "show");
        ASSERT_EQ(c_show.directObjects[0].head, "map");
        ASSERT_EQ(c_show.prepositionalPhrases[0].object.head, "king");

        auto c_tell = parse("tell guide secret");
        ASSERT_TRUE(c_tell.valid);
        ASSERT_EQ(c_tell.verb, "tell");
        ASSERT_EQ(c_tell.directObjects[0].head, "secret");
        ASSERT_EQ(c_tell.prepositionalPhrases[0].object.head, "guide");

        auto c_pay = parse("pay sailor coin");
        ASSERT_TRUE(c_pay.valid);
        ASSERT_EQ(c_pay.verb, "pay");
        ASSERT_EQ(c_pay.directObjects[0].head, "coin");
        ASSERT_EQ(c_pay.prepositionalPhrases[0].object.head, "sailor");
    });
}

void runSuite6_PhrasalVerbsAndParticleMovement() {
    std::cout << "\n--- SUITE 6: Phrasal Verbs & Particle Movement ---\n";

    st_thomas::runTest("Contiguous vs Split: 'pick up lamp' vs 'pick lamp up'", []() {
        auto c1 = parse("pick up lamp");
        ASSERT_TRUE(c1.valid);
        ASSERT_EQ(c1.verb, "take");
        ASSERT_EQ(c1.rawVerb, "pick up");
        ASSERT_EQ(c1.directObjects.size(), 1);
        ASSERT_EQ(c1.directObjects[0].head, "lamp");

        auto c2 = parse("pick lamp up");
        ASSERT_TRUE(c2.valid);
        ASSERT_EQ(c2.verb, "take");
        ASSERT_EQ(c2.rawVerb, "pick up");
        ASSERT_EQ(c2.directObjects.size(), 1);
        ASSERT_EQ(c2.directObjects[0].head, "lamp");

        // Both representations produce identical canonical interpretation!
        ASSERT_EQ(c1.verb, c2.verb);
        ASSERT_EQ(c1.directObjects[0].head, c2.directObjects[0].head);
    });

    st_thomas::runTest("Contiguous vs Split with modifiers: 'pick up the brass lamp' vs 'pick the brass lamp up'", []() {
        auto c1 = parse("pick up the brass lamp");
        ASSERT_TRUE(c1.valid);
        ASSERT_EQ(c1.verb, "take");
        ASSERT_EQ(c1.directObjects[0].determiner, "the");
        ASSERT_EQ(c1.directObjects[0].adjectives[0], "brass");
        ASSERT_EQ(c1.directObjects[0].head, "lamp");

        auto c2 = parse("pick the brass lamp up");
        ASSERT_TRUE(c2.valid);
        ASSERT_EQ(c2.verb, "take");
        ASSERT_EQ(c2.directObjects[0].determiner, "the");
        ASSERT_EQ(c2.directObjects[0].adjectives[0], "brass");
        ASSERT_EQ(c2.directObjects[0].head, "lamp");
    });

    st_thomas::runTest("Contiguous vs Split: 'turn on flashlight' vs 'turn flashlight on'", []() {
        auto c1 = parse("turn on flashlight");
        ASSERT_TRUE(c1.valid);
        ASSERT_EQ(c1.verb, "turn on");
        ASSERT_EQ(c1.rawVerb, "turn on");
        ASSERT_EQ(c1.directObjects[0].head, "flashlight");

        auto c2 = parse("turn flashlight on");
        ASSERT_TRUE(c2.valid);
        ASSERT_EQ(c2.verb, "turn on");
        ASSERT_EQ(c2.rawVerb, "turn on");
        ASSERT_EQ(c2.directObjects[0].head, "flashlight");
    });

    st_thomas::runTest("Contiguous vs Split: 'turn off radio' vs 'turn radio off'", []() {
        auto c1 = parse("turn off radio");
        ASSERT_TRUE(c1.valid);
        ASSERT_EQ(c1.verb, "turn off");
        ASSERT_EQ(c1.directObjects[0].head, "radio");

        auto c2 = parse("turn radio off");
        ASSERT_TRUE(c2.valid);
        ASSERT_EQ(c2.verb, "turn off");
        ASSERT_EQ(c2.directObjects[0].head, "radio");
    });

    st_thomas::runTest("Contiguous vs Split: 'put down sword' vs 'put sword down'", []() {
        auto c1 = parse("put down sword");
        ASSERT_TRUE(c1.valid);
        ASSERT_EQ(c1.verb, "drop");
        ASSERT_EQ(c1.rawVerb, "put down");
        ASSERT_EQ(c1.directObjects[0].head, "sword");

        auto c2 = parse("put sword down");
        ASSERT_TRUE(c2.valid);
        ASSERT_EQ(c2.verb, "drop");
        ASSERT_EQ(c2.rawVerb, "put down");
        ASSERT_EQ(c2.directObjects[0].head, "sword");
    });

    st_thomas::runTest("Contiguous vs Split: 'put on cloak' vs 'put cloak on'", []() {
        auto c1 = parse("put on cloak");
        ASSERT_TRUE(c1.valid);
        ASSERT_EQ(c1.verb, "wear");
        ASSERT_EQ(c1.directObjects[0].head, "cloak");

        auto c2 = parse("put cloak on");
        ASSERT_TRUE(c2.valid);
        ASSERT_EQ(c2.verb, "wear");
        ASSERT_EQ(c2.directObjects[0].head, "cloak");
    });

    st_thomas::runTest("Contiguous vs Split: 'take off boots' vs 'take boots off'", []() {
        auto c1 = parse("take off boots");
        ASSERT_TRUE(c1.valid);
        ASSERT_EQ(c1.verb, "remove");
        ASSERT_EQ(c1.directObjects[0].head, "boots");

        auto c2 = parse("take boots off");
        ASSERT_TRUE(c2.valid);
        ASSERT_EQ(c2.verb, "remove");
        ASSERT_EQ(c2.directObjects[0].head, "boots");
    });

    st_thomas::runTest("Additional phrasal verbs ('switch on', 'switch off', 'shut off')", []() {
        auto c1 = parse("switch on lantern");
        ASSERT_TRUE(c1.valid);
        ASSERT_EQ(c1.verb, "turn on");
        ASSERT_EQ(c1.directObjects[0].head, "lantern");

        auto c2 = parse("switch off lantern");
        ASSERT_TRUE(c2.valid);
        ASSERT_EQ(c2.verb, "turn off");
        ASSERT_EQ(c2.directObjects[0].head, "lantern");

        auto c3 = parse("shut off generator");
        ASSERT_TRUE(c3.valid);
        ASSERT_EQ(c3.verb, "turn off");
        ASSERT_EQ(c3.directObjects[0].head, "generator");
    });

    st_thomas::runTest("Negative: Phrasal verbs with missing objects", []() {
        auto c1 = parse("pick up");
        ASSERT_FALSE(c1.valid);
        ASSERT_EQ(c1.error, "What do you want to take?");

        auto c2 = parse("put down");
        ASSERT_FALSE(c2.valid);
        ASSERT_EQ(c2.error, "What do you want to drop?");
    });
}

void runSuite7_MultiObjectLists() {
    std::cout << "\n--- SUITE 7: Multi-Object Lists & Quantifiers ---\n";

    st_thomas::runTest("Comma and conjunction lists ('take sword, shield and dagger')", []() {
        auto cmd = parse("take sword, shield and dagger");
        ASSERT_TRUE(cmd.valid);
        ASSERT_EQ(cmd.verb, "take");
        ASSERT_TRUE(cmd.isMultiObject());
        ASSERT_EQ(cmd.directObjects.size(), 3);
        ASSERT_EQ(cmd.directObjects[0].head, "sword");
        ASSERT_EQ(cmd.directObjects[1].head, "shield");
        ASSERT_EQ(cmd.directObjects[2].head, "dagger");
    });

    st_thomas::runTest("Oxford comma list ('take sword, shield, and dagger')", []() {
        auto cmd = parse("take sword, shield, and dagger");
        ASSERT_TRUE(cmd.valid);
        ASSERT_TRUE(cmd.isMultiObject());
        ASSERT_EQ(cmd.directObjects.size(), 3);
        ASSERT_EQ(cmd.directObjects[0].head, "sword");
        ASSERT_EQ(cmd.directObjects[1].head, "shield");
        ASSERT_EQ(cmd.directObjects[2].head, "dagger");
    });

    st_thomas::runTest("Simple two-object conjunction ('take sword and shield')", []() {
        auto cmd = parse("take sword and shield");
        ASSERT_TRUE(cmd.valid);
        ASSERT_TRUE(cmd.isMultiObject());
        ASSERT_EQ(cmd.directObjects.size(), 2);
        ASSERT_EQ(cmd.directObjects[0].head, "sword");
        ASSERT_EQ(cmd.directObjects[1].head, "shield");
    });

    st_thomas::runTest("Four items with adjectives ('take heavy sword, red apple, golden key and silver coin')", []() {
        auto cmd = parse("take heavy sword, red apple, golden key and silver coin");
        ASSERT_TRUE(cmd.valid);
        ASSERT_TRUE(cmd.isMultiObject());
        ASSERT_EQ(cmd.directObjects.size(), 4);

        ASSERT_EQ(cmd.directObjects[0].head, "sword");
        ASSERT_EQ(cmd.directObjects[0].adjectives[0], "heavy");

        ASSERT_EQ(cmd.directObjects[1].head, "apple");
        ASSERT_EQ(cmd.directObjects[1].adjectives[0], "red");

        ASSERT_EQ(cmd.directObjects[2].head, "key");
        ASSERT_EQ(cmd.directObjects[2].adjectives[0], "golden");

        ASSERT_EQ(cmd.directObjects[3].head, "coin");
        ASSERT_EQ(cmd.directObjects[3].adjectives[0], "silver");
    });

    st_thomas::runTest("Collective quantifier 'all' ('drop all')", []() {
        auto cmd = parse("drop all");
        ASSERT_TRUE(cmd.valid);
        ASSERT_EQ(cmd.verb, "drop");
        ASSERT_EQ(cmd.directObjects.size(), 1);
        ASSERT_TRUE(cmd.directObjects[0].isAll);
        ASSERT_EQ(cmd.directObjects[0].head, "all");
    });

    st_thomas::runTest("Collective quantifier 'everything' ('take everything')", []() {
        auto cmd = parse("take everything");
        ASSERT_TRUE(cmd.valid);
        ASSERT_EQ(cmd.verb, "take");
        ASSERT_EQ(cmd.directObjects.size(), 1);
        ASSERT_TRUE(cmd.directObjects[0].isAll);
        ASSERT_EQ(cmd.directObjects[0].head, "everything");
    });

    st_thomas::runTest("Collective quantifier 'all of the keys' and 'all keys'", []() {
        auto c1 = parse("take all of the keys");
        ASSERT_TRUE(c1.valid);
        ASSERT_EQ(c1.directObjects.size(), 1);
        ASSERT_TRUE(c1.directObjects[0].isAll);
        ASSERT_EQ(c1.directObjects[0].head, "keys");
        ASSERT_EQ(c1.directObjects[0].determiner, "the");

        auto c2 = parse("drop all coins");
        ASSERT_TRUE(c2.valid);
        ASSERT_EQ(c2.directObjects.size(), 1);
        ASSERT_TRUE(c2.directObjects[0].isAll);
        ASSERT_EQ(c2.directObjects[0].head, "coins");
    });
}

void runSuite8_PronounTaggingAndResolution() {
    std::cout << "\n--- SUITE 8: Pronoun Tagging & Anaphoric Resolution ---\n";

    st_thomas::runTest("Stateless pronoun recognition ('it', 'him', 'her', 'them')", []() {
        auto c_it = parse("take it");
        ASSERT_TRUE(c_it.valid);
        ASSERT_EQ(c_it.verb, "take");
        ASSERT_TRUE(c_it.directObjects[0].isPronoun);
        ASSERT_TRUE(c_it.directObjects[0].pronoun == PronounType::IT);
        ASSERT_EQ(c_it.directObjects[0].head, "it");

        auto c_him = parse("examine him");
        ASSERT_TRUE(c_him.valid);
        ASSERT_TRUE(c_him.directObjects[0].isPronoun);
        ASSERT_TRUE(c_him.directObjects[0].pronoun == PronounType::HIM);
        ASSERT_EQ(c_him.directObjects[0].head, "him");

        auto c_her = parse("examine her");
        ASSERT_TRUE(c_her.valid);
        ASSERT_TRUE(c_her.directObjects[0].isPronoun);
        ASSERT_TRUE(c_her.directObjects[0].pronoun == PronounType::HER);
        ASSERT_EQ(c_her.directObjects[0].head, "her");

        auto c_them = parse("drop them");
        ASSERT_TRUE(c_them.valid);
        ASSERT_TRUE(c_them.directObjects[0].isPronoun);
        ASSERT_TRUE(c_them.directObjects[0].pronoun == PronounType::THEM);
        ASSERT_EQ(c_them.directObjects[0].head, "them");
    });

    st_thomas::runTest("Pronouns in complex structures: 'ask him about secret', 'give them food'", []() {
        auto c1 = parse("ask him about secret");
        ASSERT_TRUE(c1.valid);
        ASSERT_EQ(c1.verb, "ask");
        ASSERT_TRUE(c1.directObjects[0].isPronoun);
        ASSERT_TRUE(c1.directObjects[0].pronoun == PronounType::HIM);
        ASSERT_TRUE(c1.prepositionalPhrases[0].type == PrepositionType::ABOUT);
        ASSERT_EQ(c1.prepositionalPhrases[0].object.head, "secret");

        auto c2 = parse("give them food");
        ASSERT_TRUE(c2.valid);
        ASSERT_EQ(c2.verb, "give");
        ASSERT_EQ(c2.directObjects[0].head, "food");
        ASSERT_TRUE(c2.prepositionalPhrases[0].type == PrepositionType::TO);
        ASSERT_TRUE(c2.prepositionalPhrases[0].object.isPronoun);
        ASSERT_TRUE(c2.prepositionalPhrases[0].object.pronoun == PronounType::THEM);
    });

    st_thomas::runTest("Enum to string conversion helpers", []() {
        ASSERT_EQ(pronounToString(PronounType::NONE), "none");
        ASSERT_EQ(pronounToString(PronounType::IT), "it");
        ASSERT_EQ(pronounToString(PronounType::HIM), "him");
        ASSERT_EQ(pronounToString(PronounType::HER), "her");
        ASSERT_EQ(pronounToString(PronounType::THEM), "them");

        ASSERT_EQ(prepositionToString(PrepositionType::NONE), "none");
        ASSERT_EQ(prepositionToString(PrepositionType::IN), "in");
        ASSERT_EQ(prepositionToString(PrepositionType::ON), "on");
        ASSERT_EQ(prepositionToString(PrepositionType::UNDER), "under");
        ASSERT_EQ(prepositionToString(PrepositionType::WITH), "with");
        ASSERT_EQ(prepositionToString(PrepositionType::TO), "to");
        ASSERT_EQ(prepositionToString(PrepositionType::FROM), "from");
        ASSERT_EQ(prepositionToString(PrepositionType::AT), "at");
        ASSERT_EQ(prepositionToString(PrepositionType::BEHIND), "behind");
        ASSERT_EQ(prepositionToString(PrepositionType::THROUGH), "through");
        ASSERT_EQ(prepositionToString(PrepositionType::ABOUT), "about");
        ASSERT_EQ(prepositionToString(PrepositionType::INTO), "into");
        ASSERT_EQ(prepositionToString(PrepositionType::OFF), "off");
        ASSERT_EQ(prepositionToString(PrepositionType::OUT_OF), "out of");
        ASSERT_EQ(prepositionToString(PrepositionType::UNKNOWN), "unknown");
    });

    st_thomas::runTest("Stateful Parser session pronoun resolution", []() {
        Parser session;
        ASSERT_EQ(session.getPronoun(PronounType::IT), nullptr);

        session.setPronoun(PronounType::IT, "golden key");
        ASSERT_NE(session.getPronoun(PronounType::IT), nullptr);
        ASSERT_EQ(session.getPronoun(PronounType::IT)->head, "golden key");

        session.feedInput("examine it");
        auto cmd = session.nextCommand();
        ASSERT_TRUE(cmd.valid);
        ASSERT_EQ(cmd.verb, "examine");
        ASSERT_EQ(cmd.directObjects.size(), 1);
        ASSERT_EQ(cmd.directObjects[0].head, "golden key");
        ASSERT_EQ(cmd.directObjects[0].raw, "golden key");
        ASSERT_FALSE(cmd.directObjects[0].isPronoun);

        // Preposition antecedent resolution:
        session.setPronoun(PronounType::HER, "queen");
        session.feedInput("give apple to her");
        auto c_prep = session.nextCommand();
        ASSERT_TRUE(c_prep.valid);
        ASSERT_EQ(c_prep.prepositionalPhrases[0].object.head, "queen");

        // Rich NounPhrase antecedent registration:
        NounPhrase dragonNp;
        dragonNp.raw = "the ancient red dragon";
        dragonNp.determiner = "the";
        dragonNp.adjectives = {"ancient", "red"};
        dragonNp.head = "dragon";
        session.setPronoun(PronounType::HIM, dragonNp);

        session.feedInput("attack him");
        auto c_dragon = session.nextCommand();
        ASSERT_TRUE(c_dragon.valid);
        ASSERT_EQ(c_dragon.directObjects[0].head, "dragon");
        ASSERT_EQ(c_dragon.directObjects[0].determiner, "the");
        ASSERT_EQ(c_dragon.directObjects[0].adjectives.size(), 2);
    });
}

void runSuite9_CommandSplitting() {
    std::cout << "\n--- SUITE 9: Command Splitting via splitCommands ---\n";

    st_thomas::runTest("Semicolon splitting ('north; take key; unlock door')", []() {
        auto cmds = splitCommands("north; take key; unlock door");
        ASSERT_EQ(cmds.size(), 3);
        ASSERT_EQ(cmds[0], "north");
        ASSERT_EQ(cmds[1], "take key");
        ASSERT_EQ(cmds[2], "unlock door");
    });

    st_thomas::runTest("Period splitting ('look. inventory. wait.')", []() {
        auto cmds = splitCommands("look. inventory. wait.");
        ASSERT_EQ(cmds.size(), 3);
        ASSERT_EQ(cmds[0], "look");
        ASSERT_EQ(cmds[1], "inventory");
        ASSERT_EQ(cmds[2], "wait");
    });

    st_thomas::runTest("Clausal 'then' splitting ('open door then go north')", []() {
        auto cmds = splitCommands("open door then go north");
        ASSERT_EQ(cmds.size(), 2);
        ASSERT_EQ(cmds[0], "open door");
        ASSERT_EQ(cmds[1], "go north");
    });

    st_thomas::runTest("Clausal 'and then' splitting ('unlock door and then open door')", []() {
        auto cmds = splitCommands("unlock door and then open door");
        ASSERT_EQ(cmds.size(), 2);
        ASSERT_EQ(cmds[0], "unlock door");
        ASSERT_EQ(cmds[1], "open door");
    });

    st_thomas::runTest("Mixed delimiters and connectors", []() {
        auto cmds = splitCommands("take lamp; turn lamp on and then go north. look");
        ASSERT_EQ(cmds.size(), 4);
        ASSERT_EQ(cmds[0], "take lamp");
        ASSERT_EQ(cmds[1], "turn lamp on");
        ASSERT_EQ(cmds[2], "go north");
        ASSERT_EQ(cmds[3], "look");
    });

    st_thomas::runTest("Case insensitivity in connectors ('THEN', 'AND THEN')", []() {
        auto cmds = splitCommands("take lamp THEN look AND THEN wait");
        ASSERT_EQ(cmds.size(), 3);
        ASSERT_EQ(cmds[0], "take lamp");
        ASSERT_EQ(cmds[1], "look");
        ASSERT_EQ(cmds[2], "wait");
    });

    st_thomas::runTest("Quoted strings preserve internal delimiters", []() {
        auto cmds1 = splitCommands("say \"take lamp. then go north\" then west");
        ASSERT_EQ(cmds1.size(), 2);
        ASSERT_EQ(cmds1[0], "say \"take lamp. then go north\"");
        ASSERT_EQ(cmds1[1], "west");

        auto cmds2 = splitCommands("say 'open door; wait.' then east");
        ASSERT_EQ(cmds2.size(), 2);
        ASSERT_EQ(cmds2[0], "say 'open door; wait.'");
        ASSERT_EQ(cmds2[1], "east");
    });

    st_thomas::runTest("Stateful input queueing in Parser", []() {
        Parser session;
        ASSERT_FALSE(session.hasNextCommand());

        session.feedInput("north; take key; open door");
        ASSERT_TRUE(session.hasNextCommand());

        auto c1 = session.nextCommand();
        ASSERT_EQ(c1.verb, "north");
        ASSERT_TRUE(session.hasNextCommand());

        auto c2 = session.nextCommand();
        ASSERT_EQ(c2.verb, "take");
        ASSERT_TRUE(session.hasNextCommand());

        auto c3 = session.nextCommand();
        ASSERT_EQ(c3.verb, "open");
        ASSERT_FALSE(session.hasNextCommand());

        session.feedInput("look");
        ASSERT_TRUE(session.hasNextCommand());
        session.clearQueue();
        ASSERT_FALSE(session.hasNextCommand());
    });
}

void runSuite10_TypoCorrectionAndOops() {
    std::cout << "\n--- SUITE 10: Typo Correction via applyOops & Replay ---\n";

    st_thomas::runTest("Stateless applyOops replacing last word by default", []() {
        std::string repaired = applyOops("take lmap", "lamp");
        ASSERT_EQ(repaired, "take lamp");

        std::string rep2 = Parser::oops("examine the bbrass key", "brass", 2);
        ASSERT_EQ(rep2, "examine the brass key");
    });

    st_thomas::runTest("Stateless applyOops replacing specific word index", []() {
        std::string rep1 = applyOops("taek sword", "take", 0);
        ASSERT_EQ(rep1, "take sword");

        std::string rep2 = applyOops("give gurad apple", "guard", 1);
        ASSERT_EQ(rep2, "give guard apple");
    });

    st_thomas::runTest("Stateless applyOops boundary inputs", []() {
        ASSERT_EQ(applyOops("", "look"), "look");
        ASSERT_EQ(applyOops("   \t ", "look"), "look");
        ASSERT_EQ(applyOops("take key", "sword", 99), "take sword"); // Out of bounds falls back to last word
        ASSERT_EQ(applyOops("tkae", "take"), "take");
    });

    st_thomas::runTest("Stateful interactive OOPS repair in Parser", []() {
        Parser session;

        // Command fails due to missing prepositional object
        session.feedInput("put coin in");
        auto c_fail = session.nextCommand();
        ASSERT_FALSE(c_fail.valid);
        ASSERT_EQ(c_fail.error, "What do you want to put coin in?");

        // Repair via OOPS command:
        // "put coin in" -> default index -1 replaces "in" with "slot" -> "put coin slot"
        session.feedInput("oops slot");
        auto c_repaired = session.nextCommand();
        ASSERT_TRUE(c_repaired.valid);
        ASSERT_EQ(c_repaired.verb, "put");
        ASSERT_EQ(c_repaired.directObjects.size(), 1);
        ASSERT_EQ(c_repaired.directObjects[0].head, "slot");
    });

    st_thomas::runTest("Stateful OOPS without previous failed command", []() {
        Parser session;
        session.feedInput("oops sword");
        auto c = session.nextCommand();
        ASSERT_FALSE(c.valid);
        ASSERT_EQ(c.error, "No previous failed command to repair.");
    });

    st_thomas::runTest("Stateful command repetition with AGAIN / 'g'", []() {
        Parser session;
        session.feedInput("take lamp");
        auto c1 = session.nextCommand();
        ASSERT_TRUE(c1.valid);
        ASSERT_EQ(c1.verb, "take");

        session.feedInput("again");
        auto c2 = session.nextCommand();
        ASSERT_TRUE(c2.valid);
        ASSERT_EQ(c2.verb, "take");

        // Repeat with 'g':
        session.feedInput("drop sword");
        auto c3 = session.nextCommand();
        ASSERT_EQ(c3.verb, "drop");

        session.feedInput("g");
        auto c4 = session.nextCommand();
        ASSERT_EQ(c4.verb, "drop");

        // Fresh session with no previous command:
        session.reset();
        session.feedInput("again");
        auto c_none = session.nextCommand();
        ASSERT_FALSE(c_none.valid);
        ASSERT_EQ(c_none.error, "Nothing to repeat.");
    });
}

void runSuite11_BoundaryAndAdversarial() {
    std::cout << "\n--- SUITE 11: Boundary, Adversarial & Edge Conditions ---\n";

    st_thomas::runTest("Empty string and pure whitespace inputs", []() {
        auto c1 = parse("");
        ASSERT_FALSE(c1.valid);
        ASSERT_EQ(c1.error, "Empty command.");

        auto c2 = parse("   ");
        ASSERT_FALSE(c2.valid);
        ASSERT_EQ(c2.error, "Empty command.");

        auto c3 = parse("\t\r\n  \t");
        ASSERT_FALSE(c3.valid);
        ASSERT_EQ(c3.error, "Empty command.");
    });

    st_thomas::runTest("Punctuation-only strings", []() {
        auto c1 = parse("...");
        ASSERT_FALSE(c1.valid);
        ASSERT_EQ(c1.error, "Empty command.");

        auto c2 = parse("???");
        ASSERT_FALSE(c2.valid);
        ASSERT_EQ(c2.error, "Empty command.");

        auto c3 = parse("!!!");
        ASSERT_FALSE(c3.valid);
        ASSERT_EQ(c3.error, "Empty command.");

        auto c4 = parse(";;;");
        ASSERT_FALSE(c4.valid);
        ASSERT_EQ(c4.error, "Empty command.");

        auto c5 = parse(". ; ! ?");
        ASSERT_FALSE(c5.valid);
        ASSERT_EQ(c5.error, "Empty command.");
    });

    st_thomas::runTest("Valid commands with trailing punctuation", []() {
        auto c1 = parse("look.");
        ASSERT_TRUE(c1.valid);
        ASSERT_EQ(c1.verb, "look");

        auto c2 = parse("look!");
        ASSERT_TRUE(c2.valid);
        ASSERT_EQ(c2.verb, "look");

        auto c3 = parse("look?");
        ASSERT_TRUE(c3.valid);
        ASSERT_EQ(c3.verb, "look");

        auto c4 = parse("take lamp...");
        ASSERT_TRUE(c4.valid);
        ASSERT_EQ(c4.verb, "take");
        ASSERT_EQ(c4.directObjects[0].head, "lamp");
    });

    st_thomas::runTest("Excessive leading, trailing, and internal whitespace", []() {
        auto cmd = parse("    take     the     ancient     brass     key    ");
        ASSERT_TRUE(cmd.valid);
        ASSERT_EQ(cmd.verb, "take");
        ASSERT_EQ(cmd.directObjects.size(), 1);
        ASSERT_EQ(cmd.directObjects[0].determiner, "the");
        ASSERT_EQ(cmd.directObjects[0].adjectives.size(), 2);
        ASSERT_EQ(cmd.directObjects[0].adjectives[0], "ancient");
        ASSERT_EQ(cmd.directObjects[0].adjectives[1], "brass");
        ASSERT_EQ(cmd.directObjects[0].head, "key");
    });

    st_thomas::runTest("Strange and mixed capitalization", []() {
        auto c1 = parse("tAkE tHe BrAsS kEy");
        ASSERT_TRUE(c1.valid);
        ASSERT_EQ(c1.verb, "take");
        ASSERT_EQ(c1.rawVerb, "tAkE");
        ASSERT_EQ(c1.directObjects[0].head, "key");

        auto c2 = parse("NORTH");
        ASSERT_TRUE(c2.valid);
        ASSERT_EQ(c2.verb, "north");
        ASSERT_TRUE(c2.isDirection());

        auto c3 = parse("ExAmInE");
        ASSERT_FALSE(c3.valid);
        ASSERT_EQ(c3.error, "What do you want to examine?");
    });

    st_thomas::runTest("Fantasy and unknown words handled gracefully", []() {
        auto c1 = parse("frotz goblin");
        ASSERT_TRUE(c1.valid);
        ASSERT_EQ(c1.verb, "frotz");
        ASSERT_EQ(c1.directObjects[0].head, "goblin");

        auto c2 = parse("xyzzy");
        ASSERT_TRUE(c2.valid);
        ASSERT_EQ(c2.verb, "xyzzy");
    });

    st_thomas::runTest("Boundary stress test with long chained input", []() {
        std::string longInput = "examine the ";
        for (int i = 0; i < 50; ++i) {
            longInput += "ancient brass ";
        }
        longInput += "key";

        auto cmd = parse(longInput);
        ASSERT_TRUE(cmd.valid);
        ASSERT_EQ(cmd.verb, "examine");
        ASSERT_EQ(cmd.directObjects[0].head, "key");
        ASSERT_EQ(cmd.directObjects[0].determiner, "the");
        ASSERT_EQ(cmd.directObjects[0].adjectives.size(), 100);
    });

    st_thomas::runTest("Data structure default values and empty checks", []() {
        NounPhrase npEmpty;
        ASSERT_TRUE(npEmpty.empty());

        NounPhrase npPopulated;
        npPopulated.head = "ring";
        ASSERT_FALSE(npPopulated.empty());

        PrepositionalPhrase ppEmpty;
        ASSERT_TRUE(ppEmpty.empty());

        PrepositionalPhrase ppPopulated;
        ppPopulated.type = PrepositionType::IN;
        ppPopulated.prepWord = "in";
        ASSERT_FALSE(ppPopulated.empty());

        ParsedCommand cmdEmpty;
        ASSERT_FALSE(cmdEmpty.hasDirectObject());
        ASSERT_FALSE(cmdEmpty.hasPreposition());
        ASSERT_FALSE(cmdEmpty.isMultiObject());
        ASSERT_EQ(cmdEmpty.getDirectObject(), nullptr);
        ASSERT_EQ(cmdEmpty.getPrepositionalPhrase(), nullptr);
        ASSERT_FALSE(cmdEmpty.isDirection());
        ASSERT_EQ(cmdEmpty.getDirection(), "");
    });
}

// ============================================================================
// Main Execution Entrypoint
// ============================================================================

int main() {
    std::cout << "======================================================================\n";
    std::cout << "Epistola Thomae: Relentless Verification of iftext::Parser\n";
    std::cout << "St. Thomas the Apostle, Patron of Testing and QA\n";
    std::cout << "\"Unless I see the test output, I will not believe.\"\n";
    std::cout << "======================================================================\n";

    runSuite1_IntransitiveVerbs();
    runSuite2_Directions();
    runSuite3_TransitiveAndNounPhrases();
    runSuite4_PrepositionalPhrases();
    runSuite5_TwoObjectAndDative();
    runSuite6_PhrasalVerbsAndParticleMovement();
    runSuite7_MultiObjectLists();
    runSuite8_PronounTaggingAndResolution();
    runSuite9_CommandSplitting();
    runSuite10_TypoCorrectionAndOops();
    runSuite11_BoundaryAndAdversarial();

    std::cout << "\n======================================================================\n";
    std::cout << "TEST SUITE EXECUTION SUMMARY\n";
    std::cout << "======================================================================\n";
    std::cout << "  Total Test Cases Executed : " << st_thomas::g_testsCount << "\n";
    std::cout << "  Test Cases Passed         : " << st_thomas::g_testsPassed << "\n";
    std::cout << "  Test Cases Failed         : " << st_thomas::g_testsFailed << "\n";
    std::cout << "  --------------------------------------------------------------------\n";
    std::cout << "  Total Assertions Verified : " << st_thomas::g_assertionsCount << "\n";
    std::cout << "  Assertions Passed         : " << st_thomas::g_assertionsPassed << "\n";
    std::cout << "  Assertions Failed         : " << st_thomas::g_assertionsFailed << "\n";
    std::cout << "======================================================================\n";

    if (st_thomas::g_testsFailed == 0 && st_thomas::g_assertionsFailed == 0) {
        std::cout << "VERDICT: EMPIRICAL PROOF ESTABLISHED. ALL TESTS PASSED IN PURITY.\n\n";
        return 0;
    } else {
        std::cerr << "VERDICT: WOUNDS EXPOSED! TEST FAILURES DETECTED.\n\n";
        return 1;
    }
}
