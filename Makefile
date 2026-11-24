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
    LIBS = -lfreetype -lglad -lsoil2 -lglfw3 -framework Cocoa -framework IOKit -framework CoreFoundation -framework QuartzCore -framework OpenGL -lassimp -ltiff -limgui -rpath /usr/local/lib/
    # Homebrew's clang-tidy does not know where the macOS SDK is
    TIDY_SYSROOT = -isysroot $(shell xcrun --show-sdk-path)
endif

# Static analysis tools (make lint). Homebrew's LLVM keeps clang-tidy out of the
# PATH, so it is looked for there too; override with make CLANG_TIDY=... lint
CPPCHECK   ?= cppcheck
CLANG_TIDY ?= $(firstword $(shell command -v clang-tidy) $(wildcard /usr/local/opt/llvm/bin/clang-tidy /opt/homebrew/opt/llvm/bin/clang-tidy) clang-tidy)

# ==========================
# Targets
# ==========================

# All targets are actions, not files ('test' would otherwise match the test/ directory)
.PHONY: all install uninstall example example_imgui test test_asan lint

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

# The headers are listed one by one: given a directory, cppcheck only looks at
# the source files in it and would skip every .hpp. The icon font table is left out.
LINT_HEADERS = $(wildcard include/core/*.hpp include/model/*.hpp include/objects/*.hpp) \
               $(filter-out %IconsFontAwesome6.h, $(wildcard include/utils/*.hpp))

# Static analysis of the library, the example and the test: cppcheck on every
# header, then clang-tidy with the checks of .clang-tidy (through the two
# programs, since the library is header-only). Any finding is an error.
# Needs the headers installed, cppcheck and clang-tidy (brew install cppcheck llvm).
# useInitializationList is suppressed: assigning the members in the constructor
# body is the style of the library.
lint:
	$(CPPCHECK) --std=c++17 --language=c++ --enable=warning,performance,portability --quiet --error-exitcode=1 \
		-DOGL_WITHOUT_IMGUI -D_H_OGL_H_ --suppress=missingIncludeSystem --suppress=missingInclude --suppress=useInitializationList \
		$(LINT_HEADERS) src/main.cpp test/smoke.cpp
	$(CLANG_TIDY) --quiet --warnings-as-errors='*' test/smoke.cpp src/main.cpp -- -std=c++17 $(TIDY_SYSROOT) $(INCLUDE)
	@echo "LINT OK"

