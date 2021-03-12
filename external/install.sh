if [ `uname` == "Darwin" ]; then

  brew install glfw3
  brew install sdl2
  brew install assimp
  brew install glm

else

  apt-get install libglfw3-dev
  apt-get install libsdl2-dev
  apt-get install libassimp-dev
  apt-get install libglm-dev
  apt-get install premake4

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
  cp *.h /usr/local/include/SOIL2
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
