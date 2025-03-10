INCLUDE_INSTALL_PATH=/usr/local/include
LIBRARY_INSTALL_PATH=/usr/local/lib

LIBRARY_NAME=ogl

PLATFORM := $(shell uname)

ifeq "$(PLATFORM)" "Linux"
	INCLUDE=-I./include -I/usr/local/include -I/usr/local/include/opencv4 -I/usr/include/freetype2
	LIBS=`pkg-config --libs opencv4` -lfreetype -lglad -lsoil2 `pkg-config --libs --static glfw3` -lGL -lassimp -ltiff -limgui
	COMPILER=g++
endif

ifeq "$(PLATFORM)" "Darwin"
	INCLUDE=-I./include -I/usr/local/include -I/usr/local/include/opencv4 -I/usr/local/include/freetype2
	LIBS=`pkg-config --libs opencv4` -lfreetype -lglad -lsoil2 -lglfw3 -framework Cocoa -framework IOKit -framework CoreFoundation -framework OpenGL -lassimp -lSOIL2 -ltiff -limgui -rpath /usr/local/lib/
	COMPILER=g++
endif


all: install


install:
	@rm -rf $(INCLUDE_INSTALL_PATH)/$(LIBRARY_NAME)
	@ln -s $(shell pwd)/include $(INCLUDE_INSTALL_PATH)/$(LIBRARY_NAME)

uninstall:
	@rm -rf $(INCLUDE_INSTALL_PATH)/$(LIBRARY_NAME)

example:
	@mkdir -p ~/bin
	$(COMPILER) -march=native -Os -std=c++17 -o ~/bin/ogl $(INCLUDE) ./src/main.cpp $(LIBS)

