#!/bin/bash
#
# Installa le dipendenze esterne di OGL: i pacchetti di sistema (brew su macOS,
# apt su Linux), poi SOIL2, GLAD e ImGui se non ci sono gia'.
#
# Uso: ./install.sh            (da qualunque cartella)
#      PREFIX=/opt/x ./install.sh   per installare SOIL2, GLAD e ImGui altrove
#
# A schermo esce una riga per passo; l'output completo dei comandi va in
# install.log, accanto a questo script. Un passo che fallisce NON ferma gli
# altri: alla fine c'e' l'elenco di quelli falliti e lo script esce con errore.
# (Prima c'era set -e: al primo pacchetto mancante lo script si fermava con il
# solo output di apt, senza dire niente, e SOIL2, GLAD e ImGui non venivano
# nemmeno provati.)
#

set -uo pipefail

cd "$(dirname "$0")" || exit 1
HERE=$PWD
LOG="$HERE/install.log"
: > "$LOG"

PREFIX=${PREFIX:-/usr/local}

FAILED=()

#------------------------------------------------------------------------------
# step <nome> <comando...> - esegue il comando con l'output nel log e scrive
# una riga sola: ok o FAILED
#------------------------------------------------------------------------------
step() {
  local name=$1; shift
  printf '  %-30s ' "$name"
  echo "===== $name: $*" >> "$LOG"
  if "$@" >> "$LOG" 2>&1; then
    echo "ok"
    return 0
  fi
  echo "FAILED"
  FAILED+=("$name")
  return 1
}

skip() { printf '  %-30s %s\n' "$1" "already installed"; }

#------------------------------------------------------------------------------
# permessi: se PREFIX non e' scrivibile (su Linux /usr/local di norma non lo
# e') o servono i pacchetti di sistema di Linux, serve sudo. La password si
# chiede qui, una volta, a schermo: dopo i comandi sono silenziosi e una
# richiesta nascosta nel log bloccherebbe lo script senza che si veda
#------------------------------------------------------------------------------
SUDO=""
mkdir -p "$PREFIX/include" "$PREFIX/lib" 2>/dev/null
if [ "$(uname)" != "Darwin" ] || [ ! -w "$PREFIX/include" ] || [ ! -w "$PREFIX/lib" ]; then
  SUDO=sudo
  echo "Some steps need administrator rights:"
  if ! sudo -v; then
    echo "ERROR [install.sh]: sudo failed, nothing has been installed" >&2
    exit 1
  fi
  $SUDO mkdir -p "$PREFIX/include" "$PREFIX/lib"
fi

# git non deve mai fermarsi a chiedere credenziali con l'output nascosto
export GIT_TERMINAL_PROMPT=0

#------------------------------------------------------------------------------
# pacchetti di sistema
#------------------------------------------------------------------------------

# apt_any <pacchetto...> - installa il primo disponibile: alcuni pacchetti
# hanno cambiato nome fra le versioni di Ubuntu. Non interattivo: una domanda
# di apt (tzdata, servizi da riavviare) con l'output nel log bloccherebbe tutto
apt_any() {
  local p
  for p in "$@"; do
    sudo DEBIAN_FRONTEND=noninteractive apt-get -y install "$p" && return 0
  done
  return 1
}

echo "System packages:"

if [ "$(uname)" == "Darwin" ]; then

  if ! command -v brew >/dev/null 2>&1; then
    echo "  brew not found: install Homebrew first (https://brew.sh)"
    FAILED+=("homebrew")
  else
    step "glfw"     brew install glfw
    step "sdl2"     brew install sdl2
    step "assimp"   brew install assimp
    step "glm"      brew install glm
    step "premake"  brew install premake
    # richiesti dalla libreria: snapshot TIFF e rendering del testo
    step "libtiff"  brew install libtiff
    step "freetype" brew install freetype
  fi

else

  # un update fallito (rete, repository irraggiungibile, apt bloccato da un
  # altro processo) si segnala ma non impedisce di provare i pacchetti
  step "apt update" sudo DEBIAN_FRONTEND=noninteractive apt-get update
  step "glfw"       apt_any libglfw3-dev
  step "sdl2"       apt_any libsdl2-dev
  step "assimp"     apt_any libassimp-dev
  step "glm"        apt_any libglm-dev
  step "libtiff"    apt_any libtiff-dev libtiff5-dev
  step "freetype"   apt_any libfreetype-dev libfreetype6-dev

fi

#------------------------------------------------------------------------------
# SOIL2, da sorgente con premake
#------------------------------------------------------------------------------
install_soil2() {

  local premake
  if   command -v premake5 >/dev/null 2>&1; then premake=premake5
  elif command -v premake4 >/dev/null 2>&1; then premake=premake4
  else
    echo "premake5/premake4 not found: SOIL2 needs premake to build"
    return 1
  fi

  local tmp
  tmp=$(mktemp -d) || return 1

  local config=release
  [ "$(uname)" == "Darwin" ] && config=release_x86_64

  # ogni comando parte solo se il precedente e' riuscito. NB: non set -e:
  # questa funzione gira dentro l'if di step(), e li' bash ignora set -e anche
  # in una subshell, quindi dopo un clone fallito si andava avanti e si
  # creava una cartella SOIL2 vuota in PREFIX
  (
    cd "$tmp" &&
    git clone --depth 1 https://github.com/SpartanJ/SOIL2.git &&
    cd SOIL2 &&
    "$premake" gmake &&
    cd make/*/ &&
    make soil2-static-lib config="$config" &&
    cd ../../lib/* &&
    $SUDO cp libsoil2.a "$PREFIX/lib" &&
    cd ../../src/SOIL2/ &&
    $SUDO mkdir -p "$PREFIX/include/SOIL2" &&
    $SUDO cp ./*.h "$PREFIX/include/SOIL2"
  )
  local rc=$?

  rm -rf "$tmp"

  return $rc

}

echo "Libraries:"

# premake serve solo per SOIL2: su Linux si prova qui, e solo se manca
if [ "$(uname)" != "Darwin" ] && [ ! -d "$PREFIX/include/SOIL2/" ] &&
   ! command -v premake5 >/dev/null 2>&1 && ! command -v premake4 >/dev/null 2>&1; then
  # premake4 non esiste piu' nei repo delle Ubuntu recenti
  step "premake" apt_any premake5 premake4
fi

if [ -d "$PREFIX/include/SOIL2/" ] && [ -f "$PREFIX/lib/libsoil2.a" ]; then
  skip "SOIL2"
else
  step "SOIL2" install_soil2
fi

#------------------------------------------------------------------------------
# GLAD e ImGui, dalle cartelle qui accanto. I Makefile copiano in PREFIX, che
# su Linux di norma richiede sudo
#------------------------------------------------------------------------------
if [ -d "$PREFIX/include/glad/" ] && [ -f "$PREFIX/lib/libglad.a" ]; then
  skip "GLAD"
else
  step "GLAD" $SUDO make -C "$HERE/glad" install INCLUDE_INSTALL_PATH="$PREFIX/include" LIBRARY_INSTALL_PATH="$PREFIX/lib"
fi

if [ -d "$PREFIX/include/imgui/" ] && [ -f "$PREFIX/lib/libimgui.a" ]; then
  skip "ImGui"
else
  step "ImGui" $SUDO make -C "$HERE/imgui" install INCLUDE_INSTALL_PATH="$PREFIX/include" LIBRARY_INSTALL_PATH="$PREFIX/lib"
fi

#------------------------------------------------------------------------------
# riepilogo
#------------------------------------------------------------------------------
echo

if [ ${#FAILED[@]} -eq 0 ]; then
  echo "Done, everything is installed (details in $LOG)"
  exit 0
fi

echo "ERROR [install.sh]: these steps failed: ${FAILED[*]}" >&2
echo "  full output in $LOG, the last lines are:" >&2
tail -n 25 "$LOG" | sed 's/^/    /' >&2
exit 1
