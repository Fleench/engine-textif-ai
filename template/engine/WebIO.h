#ifndef WEBIO_H
#define WEBIO_H

#include "IOInterface.h"
#include <string>
#include <cstdlib>
#include <emscripten.h>

extern "C" {
    extern char* requestInputAsync();
}

class WebIO : public IOInterface {
    bool isRealtime = false;
public:
    void print(const std::string& msg) override {
        std::string outMsg = msg;
        bool clearScreen = false;
        size_t pos = 0;
        
        // Strip out \033[2J
        while ((pos = outMsg.find("\033[2J")) != std::string::npos) {
            clearScreen = true;
            outMsg.erase(pos, 4);
        }
        
        // Strip out \033[1;1H
        while ((pos = outMsg.find("\033[1;1H")) != std::string::npos) {
            clearScreen = true;
            outMsg.erase(pos, 6);
        }
        
        // Strip out \033[H (sometimes used instead of 1;1H)
        while ((pos = outMsg.find("\033[H")) != std::string::npos) {
            clearScreen = true;
            outMsg.erase(pos, 3);
        }
        
        if (clearScreen) {
            EM_ASM(
                var outputDiv = document.getElementById('output');
                if (outputDiv) {
                    outputDiv.innerHTML = "";
                } else if (typeof window.clearOutput === 'function') {
                    window.clearOutput();
                }
            );
        }
        
        if (outMsg.empty()) return;

        EM_ASM_({
            if (typeof window.printMessage === 'function') {
                window.printMessage(UTF8ToString($0));
            } else {
                console.log(UTF8ToString($0));
            }
        }, outMsg.c_str());
    }

    std::string requestInput() override {
        if (isRealtime) {
            // Non-blocking in realtime mode: pop from JS queue instantly
            char c = getCharAsync();
            if (c != 0) return std::string(1, c);
            return "";
        }
        // Blocking async for normal turn-based input
        char* result = requestInputAsync();
        if (!result) return "";
        std::string str(result);
        free(result);
        return str;
    }

    void enterRawMode() override {
        isRealtime = true;
        EM_ASM({
            if (typeof window.setRealtimeMode === 'function') {
                window.setRealtimeMode(true);
            }
        });
    }

    void exitRawMode() override {
        isRealtime = false;
        EM_ASM({
            if (typeof window.setRealtimeMode === 'function') {
                window.setRealtimeMode(false);
            }
        });
    }

    bool kbhit() override {
        return EM_ASM_INT({
            return (window.inputQueue && window.inputQueue.length > 0) ? 1 : 0;
        });
    }

    char getCharAsync() override {
        sleepFrame(10); // Automatically yield to browser to prevent tight-loop freezes
        if (!kbhit()) return 0;
        return EM_ASM_INT({
            if (window.inputQueue && window.inputQueue.length > 0) {
                var key = window.inputQueue.shift();
                if (key.length === 1) return key.charCodeAt(0);
                if (key === "ArrowUp") return 119;
                if (key === "ArrowDown") return 115;
                if (key === "ArrowLeft") return 97;
                if (key === "ArrowRight") return 100;
                if (key === "Enter") return 10;
                if (key === " " || key === "Space") return 32;
                return 0;
            }
            return 0;
        });
    }

    void enterAltScreen() override {
        print("\033[?1049h");
    }

    void exitAltScreen() override {
        print("\033[?1049l");
    }

    void sleepFrame(int ms) override {
        emscripten_sleep(ms);
    }

    void setStatus(const std::string& status) override {
        EM_ASM_({
            var el = document.getElementById('status-bar');
            if (el) { el.innerText = UTF8ToString($0); }
            else {
                if (typeof window.printMessage === 'function') {
                    window.printMessage("\n" + UTF8ToString($0));
                } else {
                    console.log("\n" + UTF8ToString($0));
                }
            }
        }, status.c_str());
    }
};

#endif
