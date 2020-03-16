INCLUDE_INSTALL_PATH=/usr/local/include
LIBRARY_INSTALL_PATH=/usr/local/lib

LIBRARY_NAME=ogl

PLATFORM := $(shell uname)

ifeq "$(PLATFORM)" "Linux"
	INCLUDE=-I./include `pkg-config --cflags opencv4` 
	LIBS=`pkg-config --libs opencv4` -lglad -lsoil2 `pkg-config --libs --static glfw3` -lGL -lassimp -ltiff
	COMPILER=g++
endif

ifeq "$(PLATFORM)" "Darwin"
	INCLUDE=-I./include `pkg-config --cflags opencv4` 
	LIBS=`pkg-config --libs opencv4` -lglad `pkg-config --libs --static glfw3` -framework OpenGL -lassimp -lSOIL2 -ltiff
	COMPILER=g++	
endif


all: install


setup:


development:
	@rm -rf $(INCLUDE_INSTALL_PATH)/$(LIBRARY_NAME)
	@ln -s $(shell pwd)/include $(INCLUDE_INSTALL_PATH)/$(LIBRARY_NAME)
#	@cp mpl.pc /usr/share/pkgconfig/
#	@cp mpl.pc /usr/local/lib/pkgconfig/


install:
	@mkdir -p $(INCLUDE_INSTALL_PATH)/$(LIBRARY_NAME)/
	@cp -r ./include/* $(INCLUDE_INSTALL_PATH)/$(LIBRARY_NAME)/
#	@cp mpl.pc /usr/share/pkgconfig/
#	@cp mpl.pc /usr/local/lib/pkgconfig/


uninstall:
	@rm -rf $(INCLUDE_INSTALL_PATH)/$(LIBRARY_NAME)


clean:
	@rm -rf ./bin


test:
	@mkdir ./bin
	$(COMPILER) -march=native -Os -std=c++17 -o ./bin/ogl $(INCLUDE) ./src/main.cpp $(LIBS)

