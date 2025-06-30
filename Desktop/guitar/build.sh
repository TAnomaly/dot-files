#!/bin/bash
echo "Building Electric Guitar Simulator..."

# Create build directory
mkdir -p build
cd build

# Compile with g++
g++ -std=c++17 -O2 \
    -I../src \
    ../src/main.cpp \
    ../src/Guitar.cpp \
    ../src/AudioManager.cpp \
    $(pkg-config --cflags --libs sdl2 SDL2_mixer) \
    -o ElectricGuitar

if [ $? -eq 0 ]; then
    echo "Build successful! Run: ./ElectricGuitar"
else
    echo "Build failed! Make sure SDL2 is installed:"
    echo "Ubuntu/Debian: sudo apt install libsdl2-dev libsdl2-mixer-dev"
    echo "Fedora: sudo dnf install SDL2-devel SDL2_mixer-devel"
    echo "Arch: sudo pacman -S sdl2 sdl2_mixer"
fi

cd .. 