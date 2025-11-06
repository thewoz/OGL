#!/bin/bash

if [ `uname` == "Darwin" ]; then

  brew install glfw3
  brew install sdl2
  brew install assimp
  brew install glm
  brew install premake

else

  sudo apt -y install libglfw3-dev
  sudo apt -y install libsdl2-dev
  sudo apt -y install libassimp-dev
  sudo apt -y install libglm-dev
  sudo apt -y install premake4
  sudo apt -y install libtiff-dev
  sudo apt -y install libfreetype-dev
  
fi

# Controllo se la libreria SOIL2 è installata
if [ ! -d "/usr/local/include/SOIL2/" ]; then

  git clone https://github.com/SpartanJ/SOIL2.git
  cd SOIL2
  premake5 gmake
  cd make/*/
  if [ `uname` == "Darwin" ]; then
    make soil2-static-lib config=release_x86_64
  else
    make soil2-static-lib config=release
  fi
  cd ../../lib/*
  sudo cp libsoil2.a /usr/local/lib
  cd ../../src/SOIL2/
  sudo mkdir /usr/local/include/SOIL2
  sudo cp *.h /usr/local/include/SOIL2
  cd ../../..
  rm -rf SOIL2

fi

# Controllo se la libreria GLAD è installata
if [ ! -d "/usr/local/include/glad/" ]; then
  cd glad
    make install
  cd -
fi

# Controllo se la libreria IMGUI è installata
if [ ! -d "/usr/local/include/imgui/" ]; then
  cd imgui
    make install
  cd -
fi
