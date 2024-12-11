#!/bin/bash

if [ `uname` == "Darwin" ]; then

  brew install glfw3
  brew install sdl2
  brew install assimp
  brew install glm
  brew install premake4

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
  premake4 gmake
  cd make/*/
  make config=release soil2-static-lib
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
  sudo make install
  cd -
fi

# Controllo se la libreria IMGUI è installata
if [ ! -d "/usr/local/include/imgui/" ]; then
  cd imgui
  sudo make install
  cd -
fi
