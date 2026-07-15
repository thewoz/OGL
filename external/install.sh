#!/bin/bash

# Ogni passo deve riuscire: senza set -e un'installazione fallita veniva
# ignorata in silenzio e gli step successivi proseguivano comunque.
set -euo pipefail

if [ "$(uname)" == "Darwin" ]; then

  brew install glfw3
  brew install sdl2
  brew install assimp
  brew install glm
  brew install premake
  # Richiesti dalla libreria (snapshot TIFF e rendering del testo): prima
  # venivano installati solo nel ramo Linux.
  brew install libtiff
  brew install freetype

else

  sudo apt -y install libglfw3-dev
  sudo apt -y install libsdl2-dev
  sudo apt -y install libassimp-dev
  sudo apt -y install libglm-dev
  # premake4 non esiste più nei repo delle Ubuntu recenti: se manca non
  # bloccare lo script (set -e), ci pensa il blocco di detection più sotto.
  sudo apt -y install premake4 || echo "WARNING [install.sh]: premake4 not available from apt, will look for premake5/premake4 in PATH"
  sudo apt -y install libtiff-dev
  sudo apt -y install libfreetype-dev

fi

# Controllo se la libreria SOIL2 è installata
if [ ! -d "/usr/local/include/SOIL2/" ]; then

  # SOIL2 usa premake: preferisci premake5, ripiega su premake4 (pacchetto apt),
  # altrimenti fermati con un errore chiaro invece di fallire a metà build.
  if command -v premake5 >/dev/null 2>&1; then
    PREMAKE=premake5
  elif command -v premake4 >/dev/null 2>&1; then
    PREMAKE=premake4
  else
    echo "ERROR [install.sh]: premake5/premake4 not found; install premake first" >&2
    exit 1
  fi

  git clone https://github.com/SpartanJ/SOIL2.git
  cd SOIL2
  "$PREMAKE" gmake
  cd make/*/
  if [ "$(uname)" == "Darwin" ]; then
    make soil2-static-lib config=release_x86_64
  else
    make soil2-static-lib config=release
  fi
  cd ../../lib/*
  sudo cp libsoil2.a /usr/local/lib
  cd ../../src/SOIL2/
  sudo mkdir -p /usr/local/include/SOIL2
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
