INCLUDE_INSTALL_PATH=/usr/local/include
LIBRARY_INSTALL_PATH=/usr/local/lib

PLATFORM := $(shell uname)

COMPILER=gcc

all: install

install:
	@cp -r ./include/* $(INCLUDE_INSTALL_PATH)
	$(COMPILER) -march=native -Os -c -I/usr/local/include ./src/glad.c -o glad.o
	ar rcs $(LIBRARY_INSTALL_PATH)/libglad.a glad.o
	@rm glad.o