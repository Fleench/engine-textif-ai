#ifndef IOINTERFACE_H
#define IOINTERFACE_H

#include <string>

class IOInterface {
public:
    virtual ~IOInterface() = default;
    virtual void print(const std::string& msg) = 0;
    virtual std::string requestInput() = 0;

    virtual void enterRawMode() {}
    virtual void exitRawMode() {}
    virtual bool kbhit() { return false; }
    virtual char getCharAsync() { return 0; }
    virtual void sleepFrame(int ms) { (void)ms; }
    virtual void enterAltScreen() {}
    virtual void exitAltScreen() {}
    virtual void setStatus(const std::string& status) { (void)status; print("\n" + status); }
};

class TermiosGuard {
    IOInterface* io;
public:
    TermiosGuard(IOInterface* io) : io(io) {
        if (io) io->enterRawMode();
    }
    ~TermiosGuard() {
        if (io) io->exitRawMode();
    }
};

#endif
