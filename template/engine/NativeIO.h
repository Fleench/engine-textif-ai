#ifndef NATIVEIO_H
#define NATIVEIO_H

#include "IOInterface.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <thread>
#include <chrono>
#include <fcntl.h>

class NativeIO : public IOInterface {
private:
    struct termios orig_termios;
    bool raw_mode_active = false;
public:
    void print(const std::string& msg) override {
        std::cout << msg;
    }
    
    std::string requestInput() override {
        if (raw_mode_active) {
            char c = getCharAsync();
            if (c != 0) return std::string(1, c);
            return "";
        }
        std::string input;
        if (!std::getline(std::cin, input)) {
            if (std::cin.eof()) {
                return "quit";
            }
            std::cin.clear();
        }
        return input;
    }

    void enterRawMode() override {
        if (!raw_mode_active) {
            if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
                return;
            }
            struct termios raw = orig_termios;
            raw.c_lflag &= ~(ECHO | ICANON);
            raw.c_cc[VMIN] = 0;
            raw.c_cc[VTIME] = 0;
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
            
            int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
            fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
            
            raw_mode_active = true;
        }
    }

    void exitRawMode() override {
        if (raw_mode_active) {
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
            int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
            fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
            raw_mode_active = false;
        }
    }

    bool kbhit() override {
        struct timeval tv = { 0L, 0L };
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
    }

    char getCharAsync() override {
        if (kbhit()) {
            char c;
            if (read(STDIN_FILENO, &c, 1) > 0) {
                return c;
            }
        }
        return 0;
    }

    void enterAltScreen() override {
        std::cout << "\033[?1049h" << std::flush;
    }

    void exitAltScreen() override {
        std::cout << "\033[?1049l" << std::flush;
    }

    void sleepFrame(int ms) override {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
    
    ~NativeIO() {
        exitRawMode();
    }
};


#endif
