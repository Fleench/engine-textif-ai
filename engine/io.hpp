#ifndef FICTIVE_IO_HPP
#define FICTIVE_IO_HPP

#ifndef IFTEXT_IO_HPP
#define IFTEXT_IO_HPP
#endif

/**
 * @file io.hpp
 * @brief Fictive-AI Unified I/O, Terminal Driver, Styling, and Debug Engine.
 * 
 * Author: Mother Mary (Our Lady Orchestrator) / Communion of Saints Framework
 * Master Builder: St. Peter the Apostle, Master Builder and Core Developer
 * 
 * Single header-only C++17 library. Zero external dependencies.
 * Built upon solid rock.
 */

#include <iostream>
#include <string>
#include <string_view>
#include <sstream>
#include <functional>
#include <memory>
#include <utility>
#include <type_traits>

namespace fictive {

// ============================================================================
// 1. STYLES & ANSI ESCAPE SEQUENCES
// ============================================================================

/**
 * @brief Represents an individual styling code or ANSI escape sequence.
 */
struct Style {
    enum class Code : int {
        Reset = 0,
        Bold = 1,
        Dim = 2,
        Italic = 3,
        Underline = 4,

        Black = 30,
        Red = 31,
        Green = 32,
        Yellow = 33,
        Blue = 34,
        Magenta = 35,
        Cyan = 36,
        White = 37,

        BrightBlack = 90,
        BrightRed = 91,
        BrightGreen = 92,
        BrightYellow = 93,
        BrightBlue = 94,
        BrightMagenta = 95,
        BrightCyan = 96,
        BrightWhite = 97
    };

    std::string_view ansiCode;
    Code code;
    std::string_view name;

    constexpr Style() noexcept : ansiCode(""), code(Code::Reset), name("reset") {}
    constexpr Style(std::string_view ansi, Code c, std::string_view n) noexcept
        : ansiCode(ansi), code(c), name(n) {}

    constexpr std::string_view ansi() const noexcept { return ansiCode; }
    constexpr std::string_view str() const noexcept { return ansiCode; }
    constexpr Code getCode() const noexcept { return code; }
    constexpr std::string_view getName() const noexcept { return name; }

    constexpr bool operator==(const Style& other) const noexcept { return code == other.code; }
    constexpr bool operator!=(const Style& other) const noexcept { return code != other.code; }
};

/**
 * @brief Stream insertion operator for Style into standard std::ostream.
 */
inline std::ostream& operator<<(std::ostream& os, const Style& s) {
    return os << s.ansi();
}

/**
 * @brief Catalog of standard terminal and interface styles.
 * Supports both instance access (styles.red) and static access (Styles::red).
 */
struct StylesCatalog {
    // Reset
    static constexpr Style reset{"\033[0m", Style::Code::Reset, "reset"};

    // Text Attributes
    static constexpr Style bold{"\033[1m", Style::Code::Bold, "bold"};
    static constexpr Style dim{"\033[2m", Style::Code::Dim, "dim"};
    static constexpr Style italic{"\033[3m", Style::Code::Italic, "italic"};
    static constexpr Style underline{"\033[4m", Style::Code::Underline, "underline"};

    // Standard Colors
    static constexpr Style black{"\033[30m", Style::Code::Black, "black"};
    static constexpr Style red{"\033[31m", Style::Code::Red, "red"};
    static constexpr Style green{"\033[32m", Style::Code::Green, "green"};
    static constexpr Style yellow{"\033[33m", Style::Code::Yellow, "yellow"};
    static constexpr Style blue{"\033[34m", Style::Code::Blue, "blue"};
    static constexpr Style magenta{"\033[35m", Style::Code::Magenta, "magenta"};
    static constexpr Style cyan{"\033[36m", Style::Code::Cyan, "cyan"};
    static constexpr Style white{"\033[37m", Style::Code::White, "white"};

    // Bright Colors
    static constexpr Style brightBlack{"\033[90m", Style::Code::BrightBlack, "brightBlack"};
    static constexpr Style gray{"\033[90m", Style::Code::BrightBlack, "gray"};
    static constexpr Style brightRed{"\033[91m", Style::Code::BrightRed, "brightRed"};
    static constexpr Style brightGreen{"\033[92m", Style::Code::BrightGreen, "brightGreen"};
    static constexpr Style brightYellow{"\033[93m", Style::Code::BrightYellow, "brightYellow"};
    static constexpr Style brightBlue{"\033[94m", Style::Code::BrightBlue, "brightBlue"};
    static constexpr Style brightMagenta{"\033[95m", Style::Code::BrightMagenta, "brightMagenta"};
    static constexpr Style brightCyan{"\033[96m", Style::Code::BrightCyan, "brightCyan"};
    static constexpr Style brightWhite{"\033[97m", Style::Code::BrightWhite, "brightWhite"};
};

using Styles = StylesCatalog;
inline constexpr StylesCatalog styles{};

// ============================================================================
// 2. TARGET DRIVER ARCHITECTURE
// ============================================================================

enum class TargetPlatform {
    Terminal,
    Web,
    Desktop
};

/**
 * @brief Abstract I/O Driver Interface.
 */
class IODriver {
public:
    virtual ~IODriver() = default;

    virtual TargetPlatform platform() const noexcept = 0;
    virtual void write(std::string_view text) = 0;
    virtual void writeStyle(const Style& style) = 0;
    virtual void flush() = 0;
    virtual std::string readLine(std::string_view prompt) = 0;
    virtual bool read(std::string& out) = 0;

    virtual bool isColorEnabled() const noexcept = 0;
    virtual void setColorEnabled(bool enabled) noexcept = 0;

    virtual void setStreams(std::istream* in, std::ostream* out) = 0;
    virtual std::istream* getInputStream() noexcept = 0;
    virtual std::ostream* getOutputStream() noexcept = 0;

    virtual bool good() const noexcept {
        auto* in = const_cast<IODriver*>(this)->getInputStream();
        return in ? in->good() : true;
    }
    virtual bool eof() const noexcept {
        auto* in = const_cast<IODriver*>(this)->getInputStream();
        return in ? in->eof() : false;
    }
    virtual bool fail() const noexcept {
        auto* in = const_cast<IODriver*>(this)->getInputStream();
        return in ? in->fail() : false;
    }
};

/**
 * @brief Terminal Driver (Standard ANSI escape sequences, std::cout, std::cin, std::getline).
 */
class TerminalDriver : public IODriver {
private:
    std::istream* m_in;
    std::ostream* m_out;
    bool m_colorEnabled;

public:
    explicit TerminalDriver(std::istream* in = &std::cin, std::ostream* out = &std::cout, bool color = true)
        : m_in(in), m_out(out), m_colorEnabled(color) {}

    TargetPlatform platform() const noexcept override {
        return TargetPlatform::Terminal;
    }

    void write(std::string_view text) override {
        if (m_out) {
            m_out->write(text.data(), static_cast<std::streamsize>(text.size()));
        }
    }

    void writeStyle(const Style& style) override {
        if (m_out && m_colorEnabled) {
            auto ansi = style.ansi();
            if (!ansi.empty()) {
                m_out->write(ansi.data(), static_cast<std::streamsize>(ansi.size()));
            }
        }
    }

    void flush() override {
        if (m_out) {
            m_out->flush();
        }
    }

    std::string readLine(std::string_view prompt) override {
        if (!prompt.empty()) {
            write(prompt);
            flush();
        }
        std::string line;
        if (m_in && std::getline(*m_in, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            return line;
        }
        return "";
    }

    bool read(std::string& out) override {
        if (m_in && (*m_in >> out)) {
            return true;
        }
        return false;
    }

    bool isColorEnabled() const noexcept override { return m_colorEnabled; }
    void setColorEnabled(bool enabled) noexcept override { m_colorEnabled = enabled; }

    void setStreams(std::istream* in, std::ostream* out) override {
        m_in = in;
        m_out = out;
    }

    std::istream* getInputStream() noexcept override { return m_in; }
    std::ostream* getOutputStream() noexcept override { return m_out; }
};

/**
 * @brief Web Driver (Hooks for WebAssembly/Emscripten and JavaScript runtime).
 */
class WebDriver : public IODriver {
public:
    using WriteHook = std::function<void(std::string_view text, const Style* style)>;
    using ReadLineHook = std::function<std::string(std::string_view prompt)>;

private:
    std::istream* m_in;
    std::ostream* m_out;
    bool m_colorEnabled;
    WriteHook m_writeHook;
    ReadLineHook m_readHook;

public:
    explicit WebDriver(std::istream* in = &std::cin, std::ostream* out = &std::cout, bool color = true)
        : m_in(in), m_out(out), m_colorEnabled(color), m_writeHook(nullptr), m_readHook(nullptr) {}

    TargetPlatform platform() const noexcept override {
        return TargetPlatform::Web;
    }

    void setWriteHook(WriteHook hook) { m_writeHook = std::move(hook); }
    void setReadLineHook(ReadLineHook hook) { m_readHook = std::move(hook); }

    void write(std::string_view text) override {
        if (m_writeHook) {
            m_writeHook(text, nullptr);
        } else if (m_out) {
            m_out->write(text.data(), static_cast<std::streamsize>(text.size()));
        }
    }

    void writeStyle(const Style& style) override {
        if (m_writeHook) {
            m_writeHook("", &style);
        } else if (m_out && m_colorEnabled) {
            auto ansi = style.ansi();
            if (!ansi.empty()) {
                m_out->write(ansi.data(), static_cast<std::streamsize>(ansi.size()));
            }
        }
    }

    void flush() override {
        if (m_out) {
            m_out->flush();
        }
    }

    std::string readLine(std::string_view prompt) override {
        if (m_readHook) {
            return m_readHook(prompt);
        }
        if (!prompt.empty()) {
            write(prompt);
            flush();
        }
        std::string line;
        if (m_in && std::getline(*m_in, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            return line;
        }
        return "";
    }

    bool read(std::string& out) override {
        if (m_readHook) {
            out = m_readHook("");
            return !out.empty();
        }
        if (m_in && (*m_in >> out)) {
            return true;
        }
        return false;
    }

    bool isColorEnabled() const noexcept override { return m_colorEnabled; }
    void setColorEnabled(bool enabled) noexcept override { m_colorEnabled = enabled; }

    void setStreams(std::istream* in, std::ostream* out) override {
        m_in = in;
        m_out = out;
    }

    std::istream* getInputStream() noexcept override { return m_in; }
    std::ostream* getOutputStream() noexcept override { return m_out; }
};

/**
 * @brief Desktop Driver (Hooks for Desktop Webview window, IPC events, and native bridges).
 */
class DesktopDriver : public IODriver {
public:
    using WriteHook = std::function<void(std::string_view text, const Style* style)>;
    using ReadLineHook = std::function<std::string(std::string_view prompt)>;
    using MessageHook = std::function<void(std::string_view message)>;

private:
    std::istream* m_in;
    std::ostream* m_out;
    bool m_colorEnabled;
    WriteHook m_writeHook;
    ReadLineHook m_readHook;
    MessageHook m_messageHook;

public:
    explicit DesktopDriver(std::istream* in = &std::cin, std::ostream* out = &std::cout, bool color = true)
        : m_in(in), m_out(out), m_colorEnabled(color),
          m_writeHook(nullptr), m_readHook(nullptr), m_messageHook(nullptr) {}

    TargetPlatform platform() const noexcept override {
        return TargetPlatform::Desktop;
    }

    void setWriteHook(WriteHook hook) { m_writeHook = std::move(hook); }
    void setReadLineHook(ReadLineHook hook) { m_readHook = std::move(hook); }
    void setWebviewMessageHook(MessageHook hook) { m_messageHook = std::move(hook); }

    void postWebviewMessage(std::string_view message) {
        if (m_messageHook) {
            m_messageHook(message);
        }
    }

    void write(std::string_view text) override {
        if (m_writeHook) {
            m_writeHook(text, nullptr);
        } else if (m_out) {
            m_out->write(text.data(), static_cast<std::streamsize>(text.size()));
        }
    }

    void writeStyle(const Style& style) override {
        if (m_writeHook) {
            m_writeHook("", &style);
        } else if (m_out && m_colorEnabled) {
            auto ansi = style.ansi();
            if (!ansi.empty()) {
                m_out->write(ansi.data(), static_cast<std::streamsize>(ansi.size()));
            }
        }
    }

    void flush() override {
        if (m_out) {
            m_out->flush();
        }
    }

    std::string readLine(std::string_view prompt) override {
        if (m_readHook) {
            return m_readHook(prompt);
        }
        if (!prompt.empty()) {
            write(prompt);
            flush();
        }
        std::string line;
        if (m_in && std::getline(*m_in, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            return line;
        }
        return "";
    }

    bool read(std::string& out) override {
        if (m_readHook) {
            out = m_readHook("");
            return !out.empty();
        }
        if (m_in && (*m_in >> out)) {
            return true;
        }
        return false;
    }

    bool isColorEnabled() const noexcept override { return m_colorEnabled; }
    void setColorEnabled(bool enabled) noexcept override { m_colorEnabled = enabled; }

    void setStreams(std::istream* in, std::ostream* out) override {
        m_in = in;
        m_out = out;
    }

    std::istream* getInputStream() noexcept override { return m_in; }
    std::ostream* getOutputStream() noexcept override { return m_out; }
};

// Target driver selection based on preprocessor configuration
#if defined(FICTIVE_TARGET_WEB)
using ActiveTargetDriver = WebDriver;
#elif defined(FICTIVE_TARGET_DESKTOP)
using ActiveTargetDriver = DesktopDriver;
#else
using ActiveTargetDriver = TerminalDriver;
#endif

// ============================================================================
// 3. UNIFIED IO STREAM OBJECT (io)
// ============================================================================

/**
 * @brief Unified Stream supporting both input ('>>') and output ('<<') operations.
 */
class IOStream {
private:
    std::shared_ptr<IODriver> m_driver;

public:
    IOStream() : m_driver(std::make_shared<ActiveTargetDriver>()) {}
    explicit IOStream(std::shared_ptr<IODriver> driver) : m_driver(std::move(driver)) {}

    void setDriver(std::shared_ptr<IODriver> driver) noexcept {
        m_driver = std::move(driver);
    }

    std::shared_ptr<IODriver> getDriver() const noexcept {
        return m_driver;
    }

    void redirectStreams(std::istream& in, std::ostream& out) {
        if (m_driver) {
            m_driver->setStreams(&in, &out);
        }
    }

    void resetStreams() {
        if (m_driver) {
            m_driver->setStreams(&std::cin, &std::cout);
        }
    }

    void setColorEnabled(bool enabled) noexcept {
        if (m_driver) {
            m_driver->setColorEnabled(enabled);
        }
    }

    bool isColorEnabled() const noexcept {
        return m_driver ? m_driver->isColorEnabled() : false;
    }

    void flush() {
        if (m_driver) {
            m_driver->flush();
        }
    }

    // --- Output Operations (operator<<) ---

    IOStream& operator<<(const Style& style) {
        if (m_driver) {
            m_driver->writeStyle(style);
        }
        return *this;
    }

    IOStream& operator<<(std::string_view sv) {
        if (m_driver) {
            m_driver->write(sv);
        }
        return *this;
    }

    IOStream& operator<<(const std::string& str) {
        if (m_driver) {
            m_driver->write(str);
        }
        return *this;
    }

    IOStream& operator<<(const char* str) {
        if (m_driver && str) {
            m_driver->write(str);
        }
        return *this;
    }

    IOStream& operator<<(char ch) {
        if (m_driver) {
            char buf[2] = {ch, '\0'};
            m_driver->write(std::string_view(buf, 1));
        }
        return *this;
    }

    // Support standard manipulators (e.g., std::endl, std::flush)
    IOStream& operator<<(std::ostream& (*manip)(std::ostream&)) {
        if (manip == static_cast<std::ostream& (*)(std::ostream&)>(std::endl)) {
            if (m_driver) {
                m_driver->write("\n");
                m_driver->flush();
            }
        } else if (manip == static_cast<std::ostream& (*)(std::ostream&)>(std::flush)) {
            if (m_driver) {
                m_driver->flush();
            }
        } else {
            if (m_driver && m_driver->getOutputStream()) {
                manip(*m_driver->getOutputStream());
            }
        }
        return *this;
    }

    // Generic fallback for any type printable to std::ostream
    template <typename T>
    IOStream& operator<<(const T& val) {
        if constexpr (std::is_same_v<std::decay_t<T>, Style>) {
            return *this << static_cast<const Style&>(val);
        } else {
            if (m_driver) {
                if (auto* out = m_driver->getOutputStream()) {
                    *out << val;
                } else {
                    std::ostringstream oss;
                    oss << val;
                    m_driver->write(oss.str());
                }
            }
            return *this;
        }
    }

    // --- Input Operations (operator>>) ---

    IOStream& operator>>(std::string& str) {
        str = readLine("");
        return *this;
    }

    template <typename T>
    IOStream& operator>>(T& val) {
        if (m_driver) {
            if (auto* in = m_driver->getInputStream()) {
                *in >> val;
            }
        }
        return *this;
    }

    // --- Helper Methods ---

    std::string readLine(std::string_view prompt = "") {
        return m_driver ? m_driver->readLine(prompt) : std::string{};
    }

    bool readLine(std::string& line, std::string_view prompt = "") {
        if (!m_driver) return false;
        line = m_driver->readLine(prompt);
        return !m_driver->fail();
    }

    bool readWord(std::string& word) {
        return m_driver ? m_driver->read(word) : false;
    }

    template <typename... Args>
    void print(Args&&... args) {
        if constexpr (sizeof...(args) > 0) {
            ((*this << std::forward<Args>(args)), ...);
        }
    }

    template <typename... Args>
    void println(Args&&... args) {
        if constexpr (sizeof...(args) > 0) {
            ((*this << std::forward<Args>(args)), ...);
        }
        *this << "\n";
        flush();
    }

    template <typename... Args>
    void operator()(Args&&... args) {
        print(std::forward<Args>(args)...);
    }

    explicit operator bool() const noexcept {
        return m_driver ? (!m_driver->fail() && !m_driver->eof()) : false;
    }

    bool good() const noexcept { return m_driver ? m_driver->good() : false; }
    bool eof() const noexcept { return m_driver ? m_driver->eof() : false; }
    bool fail() const noexcept { return m_driver ? m_driver->fail() : false; }
};

// Global unified IO instance
inline IOStream io;

// ============================================================================
// 4. DEDICATED STREAM FACETS (print and input)
// ============================================================================

/**
 * @brief Dedicated Output Stream Object supporting '<<', 'print(...)', and 'println(...)'.
 */
class OutputStream {
private:
    IOStream* m_io;

public:
    explicit OutputStream(IOStream* ioInstance = &io) noexcept : m_io(ioInstance) {}

    void setIO(IOStream* ioInstance) noexcept { m_io = ioInstance; }
    IOStream& getIO() const noexcept { return m_io ? *m_io : io; }

    template <typename T>
    OutputStream& operator<<(const T& val) {
        getIO() << val;
        return *this;
    }

    OutputStream& operator<<(std::ostream& (*manip)(std::ostream&)) {
        getIO() << manip;
        return *this;
    }

    template <typename... Args>
    void print(Args&&... args) {
        getIO().print(std::forward<Args>(args)...);
    }

    template <typename... Args>
    void println(Args&&... args) {
        getIO().println(std::forward<Args>(args)...);
    }

    template <typename... Args>
    void operator()(Args&&... args) {
        getIO().print(std::forward<Args>(args)...);
    }

    void flush() {
        getIO().flush();
    }
};

/**
 * @brief Dedicated Input Stream Object supporting '>>' and 'readLine(...)'.
 */
class InputStream {
private:
    IOStream* m_io;

public:
    explicit InputStream(IOStream* ioInstance = &io) noexcept : m_io(ioInstance) {}

    void setIO(IOStream* ioInstance) noexcept { m_io = ioInstance; }
    IOStream& getIO() const noexcept { return m_io ? *m_io : io; }

    template <typename T>
    InputStream& operator>>(T& val) {
        getIO() >> val;
        return *this;
    }

    std::string readLine(std::string_view prompt = "") {
        return getIO().readLine(prompt);
    }

    bool readLine(std::string& line, std::string_view prompt = "") {
        return getIO().readLine(line, prompt);
    }

    bool readWord(std::string& word) {
        return getIO().readWord(word);
    }

    explicit operator bool() const noexcept {
        return static_cast<bool>(getIO());
    }

    bool good() const noexcept { return getIO().good(); }
    bool eof() const noexcept { return getIO().eof(); }
    bool fail() const noexcept { return getIO().fail(); }
};

// Global dedicated stream facet objects
inline OutputStream print{&io};
inline InputStream input{&io};

// ============================================================================
// 5. FREE HELPER FUNCTIONS
// ============================================================================

/**
 * @brief Reads a line of input with an optional prompt.
 */
inline std::string readLine(std::string_view prompt = "") {
    return io.readLine(prompt);
}

/**
 * @brief Prints formatted arguments followed by a newline and flush.
 */
template <typename... Args>
inline void println(Args&&... args) {
    io.println(std::forward<Args>(args)...);
}

// ============================================================================
// 6. DEBUG UTILITY (Development vs Zero-Cost Production)
// ============================================================================

#if defined(PRODUCTION) || defined(NDEBUG)

/**
 * @brief Production Debug Stream: Completely zero-cost constexpr no-op.
 * Elided entirely by the compiler with zero runtime instructions.
 */
class DebugStream {
public:
    constexpr DebugStream() noexcept = default;

    template <typename Func>
    constexpr void run(Func&&) const noexcept {
        // Completely elided at compile-time
    }

    template <typename T>
    constexpr const DebugStream& operator<<(const T&) const noexcept {
        return *this;
    }

    constexpr const DebugStream& operator<<(std::ostream& (*)(std::ostream&)) const noexcept {
        return *this;
    }

    template <typename... Args>
    constexpr void print(Args&&...) const noexcept {}

    template <typename... Args>
    constexpr void println(Args&&...) const noexcept {}

    constexpr void setEnabled(bool) const noexcept {}
    constexpr bool isEnabled() const noexcept { return false; }
};

inline constexpr DebugStream debug{};

#else // Development Mode (neither PRODUCTION nor NDEBUG is defined)

/**
 * @brief Development Debug Stream: Executes lambdas and emits debug traces to io.
 */
class DebugStream {
private:
    IOStream* m_io;
    bool m_enabled;

public:
    explicit DebugStream(IOStream* ioInstance = &io) noexcept
        : m_io(ioInstance), m_enabled(true) {}

    void setEnabled(bool enabled) noexcept { m_enabled = enabled; }
    bool isEnabled() const noexcept { return m_enabled; }

    IOStream& getIO() const noexcept { return m_io ? *m_io : io; }

    template <typename Func>
    void run(Func&& fn) const {
        if (m_enabled) {
            fn();
        }
    }

    template <typename T>
    const DebugStream& operator<<(const T& val) const {
        if (m_enabled) {
            getIO() << val;
        }
        return *this;
    }

    const DebugStream& operator<<(std::ostream& (*manip)(std::ostream&)) const {
        if (m_enabled) {
            getIO() << manip;
        }
        return *this;
    }

    template <typename... Args>
    void print(Args&&... args) const {
        if (m_enabled) {
            getIO().print(std::forward<Args>(args)...);
        }
    }

    template <typename... Args>
    void println(Args&&... args) const {
        if (m_enabled) {
            getIO().println(std::forward<Args>(args)...);
        }
    }
};

inline DebugStream debug{&io};

#endif // PRODUCTION || NDEBUG

} // namespace fictive

// Backward compatibility alias
namespace iftext = fictive;

#endif // FICTIVE_IO_HPP
