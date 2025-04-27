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

# Compiler
COMPILER = g++

# Compiler and linker flags
ifeq ($(PLATFORM), Linux)
    INCLUDE = -I./include -I/usr/local/include -I/usr/include/freetype2
    LIBS = -lfreetype -lglad -lsoil2 `pkg-config --libs --static glfw3` -lGL -lassimp -ltiff -limgui
endif

ifeq ($(PLATFORM), Darwin)
    INCLUDE = -I./include -I/usr/local/include -I/usr/local/include/freetype2
    LIBS = -lfreetype -lglad -lsoil2 -lglfw3 -framework Cocoa -framework IOKit -framework CoreFoundation -framework OpenGL -lassimp -lSOIL2 -ltiff -limgui -rpath /usr/local/lib/
endif

# ==========================
# Targets
# ==========================

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

# Compile example program
example:
	@mkdir -p ~/bin
	$(COMPILER) -march=native -Os -std=c++17 -o ~/bin/ogl $(INCLUDE) ./src/main.cpp $(LIBS)
	@echo "Example built at ~/bin/ogl"
