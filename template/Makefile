SHELL := /bin/bash
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wno-unused-parameter
TARGET_NATIVE = a.out

EMXX = em++
EMFLAGS = -s WASM=1 -O3 -s ASYNCIFY -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "stringToUTF8", "lengthBytesUTF8"]' -s EXPORTED_FUNCTIONS='["_malloc", "_free", "_main"]' -std=c++20
TARGET_WASM = engine.js

SRCS = engine/main.cpp engine/GameObject.cpp engine/Item.cpp engine/Door.cpp engine/Room.cpp engine/NPC.cpp engine/MiniGame.cpp engine/Game.cpp engine/WebIO.cpp game/GameSetup.cpp

.PHONY: all build wasm clean run debug

all: build

debug: $(SRCS)
	@echo "Building native debug binary..."
	$(CXX) $(CXXFLAGS) -g -O0 -fsanitize=address,undefined -o $(TARGET_NATIVE) $(SRCS)

build: $(TARGET_NATIVE)

$(TARGET_NATIVE): $(SRCS)
	@echo "Building native binary..."
	$(CXX) $(CXXFLAGS) -o $(TARGET_NATIVE) $(SRCS)

wasm: $(TARGET_WASM)

$(TARGET_WASM): $(SRCS)
	@echo "Building WebAssembly..."
	@if [ -n "$$EMSDK" ]; then \
		source "$$EMSDK/emsdk_env.sh" >/dev/null 2>&1 && $(EMXX) $(SRCS) -o $(TARGET_WASM) $(EMFLAGS); \
	elif command -v emcc >/dev/null 2>&1; then \
		$(EMXX) $(SRCS) -o $(TARGET_WASM) $(EMFLAGS); \
	else \
		echo "emsdk not found: set EMSDK or add emcc to PATH" >&2; \
		exit 1; \
	fi

run: build
	@echo "Running local game..."
	./$(TARGET_NATIVE)

clean:
	@echo "Cleaning up..."
	rm -f $(TARGET_NATIVE) engine.wasm engine.js
