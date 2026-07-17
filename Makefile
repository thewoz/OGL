# ==========================
# OGL - OpenGL Lightweight Engine
# Makefile
# ==========================

# Installation paths
INCLUDE_INSTALL_PATH = /usr/local/include
LIBRARY_INSTALL_PATH = /usr/local/lib

# Library name
LIBRARY_NAME = ogl

# Detect platform
PLATFORM := $(shell uname)

# Compiler (overridable: make COMPILER=clang++ example)
COMPILER ?= g++

# Warning flags: the library is expected to build warning-free with these
WARNINGS = -Wall -Wextra

# Compiler and linker flags
ifeq ($(PLATFORM), Linux)
    INCLUDE = -I./include -I/usr/local/include -I/usr/include/freetype2
    LIBS = -lfreetype -lglad -lsoil2 `pkg-config --libs --static glfw3` -lGL -lassimp -ltiff -limgui
endif

ifeq ($(PLATFORM), Darwin)
    INCLUDE = -I./include -I/usr/local/include -I/usr/local/include/freetype2
    LIBS = -lfreetype -lglad -lsoil2 -lglfw3 -framework Cocoa -framework IOKit -framework CoreFoundation -framework OpenGL -lassimp -ltiff -limgui -rpath /usr/local/lib/
endif

# ==========================
# Targets
# ==========================

# All targets are actions, not files ('test' would otherwise match the test/ directory)
.PHONY: all install uninstall example example_imgui test test_asan

all: install

# Install headers as a symbolic link
install:
	@echo "Installing OGL headers to $(INCLUDE_INSTALL_PATH)/$(LIBRARY_NAME)..."
	@rm -rf $(INCLUDE_INSTALL_PATH)/$(LIBRARY_NAME)
	@ln -s $(shell pwd)/include $(INCLUDE_INSTALL_PATH)/$(LIBRARY_NAME)
	@echo "Done."

# Remove installed headers
uninstall:
	@echo "Uninstalling OGL headers..."
	@rm -rf $(INCLUDE_INSTALL_PATH)/$(LIBRARY_NAME)
	@echo "Done."

# Compile example program (no ImGui)
example:
	@mkdir -p ~/bin
	$(COMPILER) $(WARNINGS) -march=native -Os -std=c++17 -DOGL_WITHOUT_IMGUI -o ~/bin/ogl $(INCLUDE) ./src/main.cpp $(LIBS)
	@echo "Example built at ~/bin/ogl"

# Compile ImGui integration example
example_imgui:
	@mkdir -p ~/bin
	$(COMPILER) $(WARNINGS) -march=native -Os -std=c++17 -o ~/bin/ogl_imgui $(INCLUDE) ./src/main.cpp $(LIBS)
	@echo "ImGui example built at ~/bin/ogl_imgui"

# Build and run the offscreen smoke test (non-interactive; needs the headers installed)
test:
	@mkdir -p ~/bin
	$(COMPILER) $(WARNINGS) -march=native -Os -std=c++17 -o ~/bin/ogl_smoke $(INCLUDE) ./test/smoke.cpp $(LIBS)
	~/bin/ogl_smoke

# Same smoke test under AddressSanitizer + UndefinedBehaviorSanitizer.
# detect_leaks=0: LeakSanitizer is not supported on macOS, and the process-lifetime
# caches (textures, glyph atlas) are by-design reachable at exit anyway.
test_asan:
	@mkdir -p ~/bin
	$(COMPILER) $(WARNINGS) -g -O1 -std=c++17 -fsanitize=address,undefined -fno-omit-frame-pointer -o ~/bin/ogl_smoke_asan $(INCLUDE) ./test/smoke.cpp $(LIBS)
	ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=print_stacktrace=1 ~/bin/ogl_smoke_asan
