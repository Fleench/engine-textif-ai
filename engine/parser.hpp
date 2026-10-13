#ifndef FICTIVE_PARSER_HPP
#define FICTIVE_PARSER_HPP
#ifndef IFTEXT_PARSER_HPP
#define IFTEXT_PARSER_HPP
#endif

#include <string>
#include <vector>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <optional>
#include <utility>
#include <map>
#include <set>
#include <deque>

namespace fictive {

// ============================================================================
// Enumerations & Grammatical Constants
// ============================================================================

/**
 * @brief Prepositions recognized by the interactive fiction parser.
 */
enum class PrepositionType {
    NONE = 0,
    IN,
    ON,
    UNDER,
    WITH,
    TO,
    FROM,
    AT,
    BEHIND,
    THROUGH,
    ABOUT,
    INTO,
    OFF,
    OUT_OF,
    UNKNOWN,

    // PascalCase aliases for interoperability
    None = NONE,
    In = IN,
    On = ON,
    Under = UNDER,
    With = WITH,
    To = TO,
    From = FROM,
    At = AT,
    Behind = BEHIND,
    Through = THROUGH,
    About = ABOUT,
    Into = INTO,
    Off = OFF,
    OutOf = OUT_OF,
    Unknown = UNKNOWN
};

/**
 * @brief Pronoun categories for anaphoric resolution.
 */
enum class PronounType {
    NONE = 0,
    IT,
    HIM,
    HER,
    THEM,

    // PascalCase aliases for interoperability
    None = NONE,
    It = IT,
    Him = HIM,
    Her = HER,
    Them = THEM
};

/**
 * @brief Convert PrepositionType to canonical string representation.
 */
inline std::string prepositionToString(PrepositionType type) {
    switch (type) {
        case PrepositionType::IN:      return "in";
        case PrepositionType::ON:      return "on";
        case PrepositionType::UNDER:   return "under";
        case PrepositionType::WITH:    return "with";
        case PrepositionType::TO:      return "to";
        case PrepositionType::FROM:    return "from";
        case PrepositionType::AT:      return "at";
        case PrepositionType::BEHIND:  return "behind";
        case PrepositionType::THROUGH: return "through";
        case PrepositionType::ABOUT:   return "about";
        case PrepositionType::INTO:    return "into";
        case PrepositionType::OFF:     return "off";
        case PrepositionType::OUT_OF:  return "out of";
        case PrepositionType::UNKNOWN: return "unknown";
        case PrepositionType::NONE:
        default:                       return "none";
    }
}

/**
 * @brief Convert PronounType to canonical string representation.
 */
inline std::string pronounToString(PronounType type) {
    switch (type) {
        case PronounType::IT:   return "it";
        case PronounType::HIM:  return "him";
        case PronounType::HER:  return "her";
        case PronounType::THEM: return "them";
        case PronounType::NONE:
        default:                return "none";
    }
}

// ============================================================================
// Abstract Syntax Tree (AST) Structures
// ============================================================================

/**
 * @brief Represents a parsed noun phrase with determiners, modifiers, and head noun.
 */
struct NounPhrase {
    std::string raw;                       ///< Original substring (e.g. "the heavy brass key")
    std::string determiner;                ///< Determiner ("the", "a", "an", "some")
    std::string article;                   ///< Alias matching determiner
    std::vector<std::string> adjectives;   ///< Descriptive adjectives in order (e.g. ["heavy", "brass"])
    std::string head;                      ///< Core head noun (e.g. "key")
    bool isPronoun = false;                ///< True if this phrase was entered as a pronoun
    PronounType pronoun = PronounType::NONE; ///< Specific pronoun classification
    bool isAll = false;                    ///< True for collective quantities ("all", "everything")

    bool empty() const noexcept {
        return raw.empty() && head.empty();
    }
};

/**
 * @brief Represents a preposition governing an object noun phrase.
 */
struct PrepositionalPhrase {
    PrepositionType type = PrepositionType::NONE; ///< Semantic preposition category
    std::string prepWord;                         ///< Preposition token as entered (e.g. "with", "into", "out of")
    NounPhrase object;                            ///< Object governed by the preposition

    bool empty() const noexcept {
        return type == PrepositionType::NONE && prepWord.empty() && object.empty();
    }
};

/**
 * @brief Structured result of parsing an interactive fiction command sentence.
 */
struct ParsedCommand {
    bool valid = false;                                 ///< True if syntactically well-formed
    bool isValid = false;                               ///< Synced alias for valid
    std::string rawInput;                               ///< Exact original input string
    std::string verb;                                   ///< Normalized canonical verb (lowercase)
    std::string rawVerb;                                ///< Exact verb token entered by player
    std::vector<NounPhrase> directObjects;              ///< Single or multi-object noun phrases
    std::vector<PrepositionalPhrase> prepositionalPhrases; ///< Prepositional phrases
    std::vector<std::string> unparsedTokens;            ///< Tokens not matched by the grammar
    std::string error;                                  ///< Diagnostic error message if invalid
    std::string errorMessage;                           ///< Synced alias for error

    // --- Convenience Query Helpers ---

    bool hasDirectObject() const noexcept {
        return !directObjects.empty();
    }

    bool hasPreposition() const noexcept {
        return !prepositionalPhrases.empty();
    }

    bool isMultiObject() const noexcept {
        return directObjects.size() > 1;
    }

    const NounPhrase* getDirectObject() const noexcept {
        return directObjects.empty() ? nullptr : &directObjects.front();
    }

    const PrepositionalPhrase* getPrepositionalPhrase() const noexcept {
        return prepositionalPhrases.empty() ? nullptr : &prepositionalPhrases.front();
    }

    bool isDirection() const noexcept {
        if (verb == "north" || verb == "south" || verb == "east" || verb == "west" ||
            verb == "northeast" || verb == "northwest" || verb == "southeast" || verb == "southwest" ||
            verb == "up" || verb == "down" || verb == "in" || verb == "out") {
            return true;
        }
        if (verb == "go" && !directObjects.empty()) {
            const std::string& h = directObjects[0].head;
            return (h == "north" || h == "south" || h == "east" || h == "west" ||
                    h == "northeast" || h == "northwest" || h == "southeast" || h == "southwest" ||
                    h == "up" || h == "down" || h == "in" || h == "out");
        }
        return false;
    }

    std::string getDirection() const {
        if (verb == "north" || verb == "south" || verb == "east" || verb == "west" ||
            verb == "northeast" || verb == "northwest" || verb == "southeast" || verb == "southwest" ||
            verb == "up" || verb == "down" || verb == "in" || verb == "out") {
            return verb;
        }
        if (verb == "go" && !directObjects.empty()) {
            return directObjects[0].head;
        }
        return "";
    }
};

// ============================================================================
// Internal Helper Utilities
// ============================================================================

namespace detail {

inline std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

inline std::string toLower(const std::string& str) {
    std::string res = str;
    for (char& c : res) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return res;
}

inline bool isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

struct TokenSpan {
    std::string text;
    std::string lower;
    bool isComma = false;
    size_t startPos = 0;
    size_t length = 0;
};

inline std::vector<TokenSpan> tokenizeClause(const std::string& clause) {
    std::vector<TokenSpan> tokens;
    const size_t n = clause.size();
    size_t i = 0;

    while (i < n) {
        if (isWhitespace(clause[i])) {
            ++i;
            continue;
        }

        if (clause[i] == ',') {
            TokenSpan t;
            t.text = ",";
            t.lower = ",";
            t.isComma = true;
            t.startPos = i;
            t.length = 1;
            tokens.push_back(t);
            ++i;
            continue;
        }

        // Punctuation like trailing periods, question marks, exclamation points
        if (clause[i] == '.' || clause[i] == '?' || clause[i] == '!' || clause[i] == ';') {
            ++i;
            continue;
        }

        // Regular word token: read until whitespace, comma, or punctuation
        size_t start = i;
        while (i < n && !isWhitespace(clause[i]) && clause[i] != ',' &&
               clause[i] != ';' && clause[i] != '.' && clause[i] != '?' && clause[i] != '!') {
            ++i;
        }

        TokenSpan t;
        t.text = clause.substr(start, i - start);
        t.lower = toLower(t.text);
        t.isComma = false;
        t.startPos = start;
        t.length = i - start;
        tokens.push_back(t);
    }

    return tokens;
}

inline bool isDeterminer(const std::string& w) {
    return (w == "the" || w == "a" || w == "an" || w == "some");
}

inline bool isPronounWord(const std::string& w, PronounType& type) {
    if (w == "it")   { type = PronounType::IT; return true; }
    if (w == "him")  { type = PronounType::HIM; return true; }
    if (w == "her")  { type = PronounType::HER; return true; }
    if (w == "them") { type = PronounType::THEM; return true; }
    type = PronounType::NONE;
    return false;
}

inline bool isDirectionWord(const std::string& w, std::string& canonical) {
    if (w == "n" || w == "north")         { canonical = "north"; return true; }
    if (w == "s" || w == "south")         { canonical = "south"; return true; }
    if (w == "e" || w == "east")          { canonical = "east"; return true; }
    if (w == "w" || w == "west")          { canonical = "west"; return true; }
    if (w == "ne" || w == "northeast")    { canonical = "northeast"; return true; }
    if (w == "nw" || w == "northwest")    { canonical = "northwest"; return true; }
    if (w == "se" || w == "southeast")    { canonical = "southeast"; return true; }
    if (w == "sw" || w == "southwest")    { canonical = "southwest"; return true; }
    if (w == "u" || w == "up")            { canonical = "up"; return true; }
    if (w == "d" || w == "down")          { canonical = "down"; return true; }
    if (w == "in" || w == "inside")       { canonical = "in"; return true; }
    if (w == "out" || w == "outside")     { canonical = "out"; return true; }
    return false;
}

inline NounPhrase makeNounPhrase(const std::vector<TokenSpan>& tokens, size_t start, size_t end) {
    NounPhrase np;
    if (start >= end || start >= tokens.size()) return np;

    // Filter out commas if any slipped in
    std::vector<TokenSpan> words;
    for (size_t i = start; i < end && i < tokens.size(); ++i) {
        if (!tokens[i].isComma) {
            words.push_back(tokens[i]);
        }
    }
    if (words.empty()) return np;

    // Reconstruct raw substring
    std::string rawStr;
    for (size_t i = 0; i < words.size(); ++i) {
        if (i > 0) rawStr += " ";
        rawStr += words[i].text;
    }
    np.raw = rawStr;

    // Check for pronouns
    PronounType pt = PronounType::NONE;
    if (words.size() == 1 && isPronounWord(words[0].lower, pt)) {
        np.isPronoun = true;
        np.pronoun = pt;
        np.head = words[0].lower;
        return np;
    }

    // Check for collective quantities ("all", "everything")
    if (words.size() == 1 && (words[0].lower == "all" || words[0].lower == "everything")) {
        np.isAll = true;
        np.head = words[0].lower;
        return np;
    }

    size_t idx = 0;

    // Check for leading "all" or "all of the"
    if (words[idx].lower == "all") {
        np.isAll = true;
        ++idx;
        if (idx < words.size() && words[idx].lower == "of") {
            ++idx;
        }
    }

    // Check for determiner
    if (idx < words.size() && isDeterminer(words[idx].lower)) {
        np.determiner = words[idx].lower;
        np.article = np.determiner;
        ++idx;
    }

    if (idx >= words.size()) {
        if (np.isAll) {
            np.head = "all";
        }
        return np;
    }

    // Head is the final token
    np.head = words.back().lower;

    // Everything between idx and the last token is an adjective modifier
    for (size_t i = idx; i + 1 < words.size(); ++i) {
        np.adjectives.push_back(words[i].lower);
    }

    return np;
}

inline std::vector<NounPhrase> parseMultiNounPhrase(const std::vector<TokenSpan>& tokens, size_t start, size_t end) {
    std::vector<NounPhrase> result;
    if (start >= end || start >= tokens.size()) return result;

    size_t segStart = start;
    for (size_t i = start; i < end; ++i) {
        bool isSeparator = false;
        if (tokens[i].isComma) {
            isSeparator = true;
        } else if (tokens[i].lower == "and") {
            // "and" acts as separator if there is content before it and after it
            if (i > segStart && i + 1 < end) {
                isSeparator = true;
            }
        }

        if (isSeparator) {
            if (i > segStart) {
                NounPhrase np = makeNounPhrase(tokens, segStart, i);
                if (!np.empty()) {
                    result.push_back(std::move(np));
                }
            }
            segStart = i + 1;
        }
    }

    if (segStart < end) {
        NounPhrase np = makeNounPhrase(tokens, segStart, end);
        if (!np.empty()) {
            result.push_back(std::move(np));
        }
    }

    return result;
}

/**
 * @brief Checks if tokens at index match a preposition (single or multi-word).
 * @return length of matched preposition in tokens (0 if not matched).
 */
inline size_t matchPreposition(const std::vector<TokenSpan>& tokens, size_t idx, PrepositionType& outType, std::string& outPrep) {
    if (idx >= tokens.size()) return 0;

    // 1. Three-word prepositions: "on top of", "in front of"
    if (idx + 2 < tokens.size()) {
        std::string w0 = tokens[idx].lower;
        std::string w1 = tokens[idx + 1].lower;
        std::string w2 = tokens[idx + 2].lower;

        if (w0 == "on" && w1 == "top" && w2 == "of") {
            outType = PrepositionType::ON;
            outPrep = "on top of";
            return 3;
        }
        if (w0 == "in" && w1 == "front" && w2 == "of") {
            outType = PrepositionType::IN;
            outPrep = "in front of";
            return 3;
        }
    }

    // 2. Two-word prepositions: "out of", "inside of", "along with", "together with"
    if (idx + 1 < tokens.size()) {
        std::string w0 = tokens[idx].lower;
        std::string w1 = tokens[idx + 1].lower;

        if (w0 == "out" && w1 == "of") {
            outType = PrepositionType::OUT_OF;
            outPrep = "out of";
            return 2;
        }
        if (w0 == "inside" && w1 == "of") {
            outType = PrepositionType::IN;
            outPrep = "inside of";
            return 2;
        }
        if (w0 == "along" && w1 == "with") {
            outType = PrepositionType::WITH;
            outPrep = "along with";
            return 2;
        }
        if (w0 == "together" && w1 == "with") {
            outType = PrepositionType::WITH;
            outPrep = "together with";
            return 2;
        }
    }

    // 3. Single-word prepositions
    const std::string& w = tokens[idx].lower;
    if (w == "in" || w == "inside") {
        outType = PrepositionType::IN;
        outPrep = w;
        return 1;
    }
    if (w == "into") {
        outType = PrepositionType::INTO;
        outPrep = "into";
        return 1;
    }
    if (w == "on" || w == "upon") {
        outType = PrepositionType::ON;
        outPrep = w;
        return 1;
    }
    if (w == "onto") {
        outType = PrepositionType::ON;
        outPrep = "onto";
        return 1;
    }
    if (w == "under" || w == "below" || w == "beneath" || w == "underneath") {
        outType = PrepositionType::UNDER;
        outPrep = w;
        return 1;
    }
    if (w == "with" || w == "using") {
        outType = PrepositionType::WITH;
        outPrep = w;
        return 1;
    }
    if (w == "to") {
        outType = PrepositionType::TO;
        outPrep = "to";
        return 1;
    }
    if (w == "from") {
        outType = PrepositionType::FROM;
        outPrep = "from";
        return 1;
    }
    if (w == "at") {
        outType = PrepositionType::AT;
        outPrep = "at";
        return 1;
    }
    if (w == "behind") {
        outType = PrepositionType::BEHIND;
        outPrep = "behind";
        return 1;
    }
    if (w == "through" || w == "thru") {
        outType = PrepositionType::THROUGH;
        outPrep = w;
        return 1;
    }
    if (w == "about") {
        outType = PrepositionType::ABOUT;
        outPrep = "about";
        return 1;
    }
    if (w == "off") {
        outType = PrepositionType::OFF;
        outPrep = "off";
        return 1;
    }

    return 0;
}

inline bool isKnownAdjective(const std::string& w) {
    static const std::set<std::string> adjectives = {
        "small", "large", "big", "little", "tiny", "huge", "giant",
        "heavy", "light", "old", "new", "ancient", "young",
        "red", "green", "blue", "yellow", "black", "white", "gray", "grey", "golden", "silver", "gold",
        "iron", "brass", "bronze", "copper", "wooden", "stone", "glass", "crystal", "leather",
        "magic", "magical", "rusty", "sharp", "dull", "shiny", "bright", "dark", "glowing",
        "hot", "cold", "warm", "frozen", "empty", "full", "open", "closed", "locked",
        "good", "bad", "evil", "holy", "sacred", "blessed"
    };
    return adjectives.count(w) > 0;
}

inline bool isDativeVerb(const std::string& v) {
    return (v == "give" || v == "hand" || v == "offer" || v == "feed" ||
            v == "pay" || v == "show" || v == "tell");
}

} // namespace detail

// ============================================================================
// Core Stateless API
// ============================================================================

/**
 * @brief Splits a multi-command sentence on periods ('.'), semicolons (';'),
 *        and case-insensitive clausal ' then ' / ' and then '.
 *
 * Quotes are respected: punctuation inside quotation marks will not split commands.
 *
 * @param input Raw player input line.
 * @return Vector of individual command clauses, trimmed of whitespace.
 */
inline std::vector<std::string> splitCommands(const std::string& input) {
    std::vector<std::string> commands;
    const std::string str = detail::trim(input);
    if (str.empty()) return commands;

    const size_t n = str.size();
    size_t lastStart = 0;
    bool inDoubleQuote = false;
    bool inSingleQuote = false;

    for (size_t i = 0; i < n; ++i) {
        char c = str[i];

        // Toggle quote states
        if (c == '"' && !inSingleQuote) {
            inDoubleQuote = !inDoubleQuote;
            continue;
        }
        if (c == '\'' && !inDoubleQuote) {
            inSingleQuote = !inSingleQuote;
            continue;
        }

        if (inDoubleQuote || inSingleQuote) {
            continue;
        }

        // Punctuation splitters: '.' and ';'
        if (c == ';' || c == '.') {
            std::string cmd = detail::trim(str.substr(lastStart, i - lastStart));
            if (!cmd.empty()) {
                commands.push_back(std::move(cmd));
            }
            lastStart = i + 1;
            continue;
        }

        // Check for clausal connectors: "then", "and then"
        // Requires word boundary before and after.
        if (i > 0 && (detail::isWhitespace(str[i - 1]) || str[i - 1] == ',')) {
            std::string remaining = str.substr(i);
            std::string remLower = detail::toLower(remaining);

            size_t matchLen = 0;
            if (remLower.rfind("and then ", 0) == 0) {
                matchLen = 9;
            } else if (remLower.rfind("and then\t", 0) == 0) {
                matchLen = 9;
            } else if (remLower.rfind("then ", 0) == 0) {
                matchLen = 5;
            } else if (remLower.rfind("then\t", 0) == 0) {
                matchLen = 5;
            }

            if (matchLen > 0) {
                // Determine slice end before any trailing comma or whitespace
                size_t sliceEnd = i;
                while (sliceEnd > lastStart && (detail::isWhitespace(str[sliceEnd - 1]) || str[sliceEnd - 1] == ',')) {
                    --sliceEnd;
                }
                std::string cmd = detail::trim(str.substr(lastStart, sliceEnd - lastStart));
                if (!cmd.empty()) {
                    commands.push_back(std::move(cmd));
                }
                i += matchLen - 1;
                lastStart = i + 1;
            }
        }
    }

    if (lastStart < n) {
        std::string cmd = detail::trim(str.substr(lastStart));
        if (!cmd.empty()) {
            commands.push_back(std::move(cmd));
        }
    }

    return commands;
}

/**
 * @brief Utility to correct a typographical error in a previously failed command.
 *
 * Replaces the word at typoWordIndex (or the last non-empty word if -1) with replacementWord.
 *
 * @param failedCommand Original command string that contained the typo.
 * @param replacementWord Desired replacement word.
 * @param typoWordIndex 0-based word index to replace, or -1 for the final word.
 * @return Repaired command string.
 */
inline std::string applyOops(const std::string& failedCommand, const std::string& replacementWord, int typoWordIndex = -1) {
    if (failedCommand.empty()) {
        return replacementWord;
    }

    // Tokenize into words with character indices
    struct WordSpan {
        size_t start = 0;
        size_t len = 0;
    };
    std::vector<WordSpan> spans;
    const size_t n = failedCommand.size();
    size_t i = 0;

    while (i < n) {
        while (i < n && detail::isWhitespace(failedCommand[i])) ++i;
        if (i >= n) break;
        size_t start = i;
        while (i < n && !detail::isWhitespace(failedCommand[i])) ++i;
        spans.push_back({start, i - start});
    }

    if (spans.empty()) {
        return replacementWord;
    }

    size_t targetIdx = spans.size() - 1;
    if (typoWordIndex >= 0 && static_cast<size_t>(typoWordIndex) < spans.size()) {
        targetIdx = static_cast<size_t>(typoWordIndex);
    }

    std::string result = failedCommand;
    result.replace(spans[targetIdx].start, spans[targetIdx].len, replacementWord);
    return result;
}

/**
 * @brief Parses a single interactive fiction command sentence into a structured AST.
 *
 * Handles intransitives, direction/movement, transitive verbs, multi-object lists,
 * prepositional phrases, dative alternation, phrasal verbs, and pronouns.
 *
 * @param sentence Input command string (single clause).
 * @return Structured ParsedCommand AST.
 */
inline ParsedCommand parse(const std::string& sentence) {
    ParsedCommand cmd;
    cmd.rawInput = sentence;

    const std::string trimmed = detail::trim(sentence);
    if (trimmed.empty()) {
        cmd.valid = false;
        cmd.isValid = false;
        cmd.error = "Empty command.";
        cmd.errorMessage = cmd.error;
        return cmd;
    }

    std::vector<detail::TokenSpan> tokens = detail::tokenizeClause(trimmed);
    if (tokens.empty()) {
        cmd.valid = false;
        cmd.isValid = false;
        cmd.error = "Empty command.";
        cmd.errorMessage = cmd.error;
        return cmd;
    }

    // ========================================================================
    // 1. Phrasal Verb & Verb Identification
    // ========================================================================

    size_t verbTokensConsumed = 0;
    std::string normVerb;
    std::string rawVerbStr;

    // Check two-word contiguous phrasal verbs at start:
    if (tokens.size() >= 2) {
        std::string w0 = tokens[0].lower;
        std::string w1 = tokens[1].lower;

        if (w0 == "pick" && w1 == "up") {
            normVerb = "take";
            rawVerbStr = tokens[0].text + " " + tokens[1].text;
            verbTokensConsumed = 2;
        } else if (w0 == "put" && w1 == "down") {
            normVerb = "drop";
            rawVerbStr = tokens[0].text + " " + tokens[1].text;
            verbTokensConsumed = 2;
        } else if (w0 == "turn" && w1 == "on") {
            normVerb = "turn on";
            rawVerbStr = tokens[0].text + " " + tokens[1].text;
            verbTokensConsumed = 2;
        } else if (w0 == "turn" && w1 == "off") {
            normVerb = "turn off";
            rawVerbStr = tokens[0].text + " " + tokens[1].text;
            verbTokensConsumed = 2;
        } else if (w0 == "switch" && w1 == "on") {
            normVerb = "turn on";
            rawVerbStr = tokens[0].text + " " + tokens[1].text;
            verbTokensConsumed = 2;
        } else if (w0 == "switch" && w1 == "off") {
            normVerb = "turn off";
            rawVerbStr = tokens[0].text + " " + tokens[1].text;
            verbTokensConsumed = 2;
        } else if (w0 == "shut" && w1 == "off") {
            normVerb = "turn off";
            rawVerbStr = tokens[0].text + " " + tokens[1].text;
            verbTokensConsumed = 2;
        } else if (w0 == "put" && w1 == "on") {
            normVerb = "wear";
            rawVerbStr = tokens[0].text + " " + tokens[1].text;
            verbTokensConsumed = 2;
        } else if (w0 == "take" && w1 == "off") {
            normVerb = "remove";
            rawVerbStr = tokens[0].text + " " + tokens[1].text;
            verbTokensConsumed = 2;
        }
    }

    // Check particle movement: verb at token 0, particle at the final token!
    // e.g. "pick lamp up", "turn flashlight on", "put sword down"
    bool particleShiftFound = false;
    size_t effectiveEndTokens = tokens.size();

    if (verbTokensConsumed == 0 && tokens.size() >= 3) {
        std::string w0 = tokens[0].lower;
        std::string wLast = tokens.back().lower;

        if (w0 == "pick" && wLast == "up") {
            normVerb = "take";
            rawVerbStr = tokens[0].text + " up";
            verbTokensConsumed = 1;
            particleShiftFound = true;
            effectiveEndTokens = tokens.size() - 1;
        } else if (w0 == "put" && wLast == "down") {
            normVerb = "drop";
            rawVerbStr = tokens[0].text + " down";
            verbTokensConsumed = 1;
            particleShiftFound = true;
            effectiveEndTokens = tokens.size() - 1;
        } else if (w0 == "turn" && wLast == "on") {
            normVerb = "turn on";
            rawVerbStr = tokens[0].text + " on";
            verbTokensConsumed = 1;
            particleShiftFound = true;
            effectiveEndTokens = tokens.size() - 1;
        } else if (w0 == "turn" && wLast == "off") {
            normVerb = "turn off";
            rawVerbStr = tokens[0].text + " off";
            verbTokensConsumed = 1;
            particleShiftFound = true;
            effectiveEndTokens = tokens.size() - 1;
        } else if (w0 == "put" && wLast == "on") {
            normVerb = "wear";
            rawVerbStr = tokens[0].text + " on";
            verbTokensConsumed = 1;
            particleShiftFound = true;
            effectiveEndTokens = tokens.size() - 1;
        } else if (w0 == "take" && wLast == "off") {
            normVerb = "remove";
            rawVerbStr = tokens[0].text + " off";
            verbTokensConsumed = 1;
            particleShiftFound = true;
            effectiveEndTokens = tokens.size() - 1;
        }
    }

    // If still no verb identified, first token is the verb
    if (verbTokensConsumed == 0) {
        rawVerbStr = tokens[0].text;
        const std::string& v = tokens[0].lower;
        verbTokensConsumed = 1;

        std::string canonicalDir;
        if (detail::isDirectionWord(v, canonicalDir)) {
            normVerb = canonicalDir;
        } else if (v == "l" || v == "look") {
            normVerb = "look";
        } else if (v == "i" || v == "inv" || v == "inventory") {
            normVerb = "inventory";
        } else if (v == "z" || v == "wait") {
            normVerb = "wait";
        } else if (v == "x" || v == "examine" || v == "inspect") {
            normVerb = "examine";
        } else if (v == "get" || v == "grab" || v == "pickup") {
            normVerb = "take";
        } else if (v == "drop" || v == "discard") {
            normVerb = "drop";
        } else if (v == "q" || v == "quit") {
            normVerb = "quit";
        } else if (v == "g" || v == "again") {
            normVerb = "again";
        } else if (v == "o" || v == "oops") {
            normVerb = "oops";
        } else {
            normVerb = v;
        }
    }

    cmd.verb = normVerb;
    cmd.rawVerb = rawVerbStr;

    // ========================================================================
    // 2. Intransitive & Directional Commands (Zero Arguments)
    // ========================================================================

    if (verbTokensConsumed >= effectiveEndTokens && !particleShiftFound) {
        // Strictly transitive verbs missing their objects
        if (normVerb == "take") {
            cmd.valid = false;
            cmd.error = "What do you want to take?";
        } else if (normVerb == "drop") {
            cmd.valid = false;
            cmd.error = "What do you want to drop?";
        } else if (normVerb == "examine") {
            cmd.valid = false;
            cmd.error = "What do you want to examine?";
        } else if (normVerb == "unlock") {
            cmd.valid = false;
            cmd.error = "What do you want to unlock?";
        } else if (normVerb == "lock") {
            cmd.valid = false;
            cmd.error = "What do you want to lock?";
        } else if (normVerb == "open") {
            cmd.valid = false;
            cmd.error = "What do you want to open?";
        } else if (normVerb == "close") {
            cmd.valid = false;
            cmd.error = "What do you want to close?";
        } else if (normVerb == "put") {
            cmd.valid = false;
            cmd.error = "What do you want to put, and where?";
        } else if (normVerb == "give") {
            cmd.valid = false;
            cmd.error = "What do you want to give, and to whom?";
        } else if (normVerb == "oops") {
            cmd.valid = false;
            cmd.error = "What word do you want to correct?";
        } else {
            cmd.valid = true;
        }

        cmd.isValid = cmd.valid;
        cmd.errorMessage = cmd.error;
        return cmd;
    }

    // ========================================================================
    // 3. Movement with Direction ("go north", "go in", "walk east")
    // ========================================================================

    if ((normVerb == "go" || normVerb == "walk" || normVerb == "run") &&
        verbTokensConsumed < effectiveEndTokens) {
        std::string dirWord = tokens[verbTokensConsumed].lower;
        std::string canonical;
        if (detail::isDirectionWord(dirWord, canonical)) {
            NounPhrase dirNp;
            dirNp.raw = tokens[verbTokensConsumed].text;
            dirNp.head = canonical;
            cmd.directObjects.push_back(dirNp);
            cmd.valid = true;
            cmd.isValid = true;
            return cmd;
        }
    }

    // ========================================================================
    // 4. Prepositional Phrase Scanning
    // ========================================================================

    struct PrepOccurrence {
        size_t tokenIndex = 0;
        size_t tokenLen = 0;
        PrepositionType type = PrepositionType::NONE;
        std::string prepWord;
    };

    std::vector<PrepOccurrence> prepositions;
    for (size_t i = verbTokensConsumed; i < effectiveEndTokens;) {
        PrepositionType pType = PrepositionType::NONE;
        std::string pWord;
        size_t pLen = detail::matchPreposition(tokens, i, pType, pWord);
        if (pLen > 0) {
            prepositions.push_back({i, pLen, pType, pWord});
            i += pLen;
        } else {
            ++i;
        }
    }

    // ========================================================================
    // 5. Argument Partitioning & AST Construction
    // ========================================================================

    if (prepositions.empty()) {
        // No prepositions present
        // Check for Dative Alternation: "give guard apple" / "give the guard the coin"
        bool handledDative = false;
        if (detail::isDativeVerb(normVerb)) {
            // Find boundary between Indirect Object and Direct Object:
            size_t totalWords = effectiveEndTokens - verbTokensConsumed;
            size_t splitIdx = 0;

            if (totalWords >= 2) {
                // Check if a second determiner marks the start of the direct object
                size_t secondDet = 0;
                for (size_t i = verbTokensConsumed + 1; i < effectiveEndTokens; ++i) {
                    if (detail::isDeterminer(tokens[i].lower)) {
                        secondDet = i;
                        break;
                    }
                }

                if (secondDet > verbTokensConsumed) {
                    splitIdx = secondDet;
                } else if (detail::isDeterminer(tokens[verbTokensConsumed].lower)) {
                    // NP1 starts with a determiner, e.g. "give the old guard red apple"
                    size_t cur = verbTokensConsumed + 1;
                    while (cur < effectiveEndTokens && detail::isKnownAdjective(tokens[cur].lower)) {
                        ++cur;
                    }
                    if (cur + 1 < effectiveEndTokens) {
                        splitIdx = cur + 1;
                    } else {
                        splitIdx = verbTokensConsumed + 1;
                    }
                } else {
                    // Neither NP1 nor NP2 has a determiner
                    PronounType pt = PronounType::NONE;
                    if (detail::isPronounWord(tokens[verbTokensConsumed].lower, pt)) {
                        // Pronoun recipient: e.g. "give him red apple" or "give them food"
                        splitIdx = verbTokensConsumed + 1;
                    } else if (totalWords == 2) {
                        // "give guard apple" -> 1 + 1
                        splitIdx = verbTokensConsumed + 1;
                    } else if (totalWords == 3) {
                        // "give guard red apple" vs "give old guard apple"
                        if (detail::isKnownAdjective(tokens[verbTokensConsumed + 1].lower)) {
                            splitIdx = verbTokensConsumed + 1;
                        } else if (detail::isKnownAdjective(tokens[verbTokensConsumed].lower)) {
                            splitIdx = verbTokensConsumed + 2;
                        } else {
                            splitIdx = verbTokensConsumed + 1;
                        }
                    } else if (totalWords == 4) {
                        // "give old guard red apple" -> 2 + 2
                        if (detail::isKnownAdjective(tokens[verbTokensConsumed + 2].lower)) {
                            splitIdx = verbTokensConsumed + 2;
                        } else if (detail::isKnownAdjective(tokens[verbTokensConsumed + 1].lower)) {
                            splitIdx = verbTokensConsumed + 1;
                        } else {
                            splitIdx = verbTokensConsumed + 2;
                        }
                    } else {
                        splitIdx = verbTokensConsumed + 1;
                    }
                }

                if (splitIdx > verbTokensConsumed && splitIdx < effectiveEndTokens) {
                    NounPhrase recipient = detail::makeNounPhrase(tokens, verbTokensConsumed, splitIdx);
                    std::vector<NounPhrase> dos = detail::parseMultiNounPhrase(tokens, splitIdx, effectiveEndTokens);

                    if (!recipient.empty() && !dos.empty()) {
                        cmd.directObjects = std::move(dos);
                        PrepositionalPhrase pp;
                        pp.type = PrepositionType::TO;
                        pp.prepWord = "to";
                        pp.object = std::move(recipient);
                        cmd.prepositionalPhrases.push_back(std::move(pp));
                        handledDative = true;
                    }
                }
            }
        }

        if (!handledDative) {
            // Standard Direct Object(s)
            cmd.directObjects = detail::parseMultiNounPhrase(tokens, verbTokensConsumed, effectiveEndTokens);
        }
    } else {
        // One or more prepositions present
        const size_t firstPrepIdx = prepositions[0].tokenIndex;

        // If there are tokens before the first preposition, they form the direct object(s)
        if (firstPrepIdx > verbTokensConsumed) {
            cmd.directObjects = detail::parseMultiNounPhrase(tokens, verbTokensConsumed, firstPrepIdx);
        }

        // Process each prepositional phrase
        for (size_t p = 0; p < prepositions.size(); ++p) {
            const auto& prep = prepositions[p];
            size_t objStart = prep.tokenIndex + prep.tokenLen;
            size_t objEnd = (p + 1 < prepositions.size()) ? prepositions[p + 1].tokenIndex : effectiveEndTokens;

            PrepositionalPhrase pp;
            pp.type = prep.type;
            pp.prepWord = prep.prepWord;

            if (objStart < objEnd) {
                pp.object = detail::makeNounPhrase(tokens, objStart, objEnd);
            } else {
                // Missing prepositional object: e.g. "put coin in"
                cmd.valid = false;
                cmd.error = "What do you want to " + normVerb + " " +
                            (cmd.directObjects.empty() ? "" : cmd.directObjects[0].raw + " ") +
                            prep.prepWord + "?";
            }

            cmd.prepositionalPhrases.push_back(std::move(pp));
        }
    }

    // Set validity
    if (cmd.error.empty()) {
        cmd.valid = true;
    } else {
        cmd.valid = false;
    }

    cmd.isValid = cmd.valid;
    cmd.errorMessage = cmd.error;
    return cmd;
}

// ============================================================================
// Stateful Parser & Command Session Helper
// ============================================================================

/**
 * @brief Stateful parser session helper.
 *
 * Manages input queueing, multi-turn history, conversational repair (AGAIN, OOPS),
 * and pronoun antecedent tracking.
 */
class Parser {
public:
    Parser() = default;

    // --- Stateless API Pass-Throughs ---

    static std::vector<std::string> split(const std::string& input) {
        return splitCommands(input);
    }

    static ParsedCommand parseSentence(const std::string& sentence) {
        return parse(sentence);
    }

    static std::string oops(const std::string& failedCommand, const std::string& replacementWord, int typoWordIndex = -1) {
        return applyOops(failedCommand, replacementWord, typoWordIndex);
    }

    // --- Command Queue Management ---

    void feedInput(const std::string& input) {
        auto cmds = splitCommands(input);
        for (auto& cmdStr : cmds) {
            m_commandQueue.push_back(std::move(cmdStr));
        }
    }

    bool hasNextCommand() const noexcept {
        return !m_commandQueue.empty();
    }

    ParsedCommand nextCommand() {
        if (m_commandQueue.empty()) {
            ParsedCommand cmd;
            cmd.valid = false;
            cmd.isValid = false;
            cmd.error = "No commands queued.";
            cmd.errorMessage = cmd.error;
            return cmd;
        }

        std::string rawClause = m_commandQueue.front();
        m_commandQueue.pop_front();

        ParsedCommand cmd = parse(rawClause);

        // Handle AGAIN / G replay
        if (cmd.valid && (cmd.verb == "again" || cmd.rawVerb == "g")) {
            if (!m_lastCommand.empty()) {
                cmd = parse(m_lastCommand);
            } else {
                cmd.valid = false;
                cmd.isValid = false;
                cmd.error = "Nothing to repeat.";
                cmd.errorMessage = cmd.error;
                return cmd;
            }
        }

        // Handle OOPS correction
        if (cmd.rawVerb == "oops" || cmd.rawVerb == "o") {
            std::string replacement;
            if (!cmd.directObjects.empty()) {
                replacement = cmd.directObjects[0].raw;
            } else if (!cmd.unparsedTokens.empty()) {
                replacement = cmd.unparsedTokens[0];
            }

            if (!m_lastFailedCommand.empty() && !replacement.empty()) {
                std::string repaired = applyOops(m_lastFailedCommand, replacement);
                cmd = parse(repaired);
            } else {
                cmd.valid = false;
                cmd.isValid = false;
                cmd.error = "No previous failed command to repair.";
                cmd.errorMessage = cmd.error;
                return cmd;
            }
        }

        // Pronoun resolution
        resolvePronouns(cmd);

        if (cmd.valid) {
            m_lastCommand = rawClause;
        } else {
            m_lastFailedCommand = rawClause;
        }

        return cmd;
    }

    void clearQueue() noexcept {
        m_commandQueue.clear();
    }

    // --- Pronoun Tracking ---

    void setPronoun(PronounType pronoun, const NounPhrase& phrase) {
        m_pronouns[pronoun] = phrase;
    }

    void setPronoun(PronounType pronoun, const std::string& rawText) {
        NounPhrase np;
        np.raw = rawText;
        np.head = rawText;
        m_pronouns[pronoun] = np;
    }

    const NounPhrase* getPronoun(PronounType pronoun) const {
        auto it = m_pronouns.find(pronoun);
        if (it != m_pronouns.end()) {
            return &it->second;
        }
        return nullptr;
    }

    // --- Reset ---

    void reset() noexcept {
        m_commandQueue.clear();
        m_pronouns.clear();
        m_lastCommand.clear();
        m_lastFailedCommand.clear();
    }

private:
    void resolvePronouns(ParsedCommand& cmd) {
        for (auto& obj : cmd.directObjects) {
            if (obj.isPronoun) {
                auto it = m_pronouns.find(obj.pronoun);
                if (it != m_pronouns.end()) {
                    obj = it->second;
                }
            }
        }
        for (auto& prep : cmd.prepositionalPhrases) {
            if (prep.object.isPronoun) {
                auto it = m_pronouns.find(prep.object.pronoun);
                if (it != m_pronouns.end()) {
                    prep.object = it->second;
                }
            }
        }
    }

    std::deque<std::string> m_commandQueue;
    std::map<PronounType, NounPhrase> m_pronouns;
    std::string m_lastCommand;
    std::string m_lastFailedCommand;
};

} // namespace fictive

// Backward compatibility alias for legacy tests and codebases
namespace iftext = fictive;

#endif // FICTIVE_PARSER_HPP
