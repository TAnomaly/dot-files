#!/bin/bash
echo "Building Electric Guitar 3D Simulator..."

# Create build directory
mkdir -p build
cd build

# Compile with g++
g++ -std=c++17 -O2 \
    -I../src \
    -I../shaders \
    ../src/main.cpp \
    ../src/Guitar3D.cpp \
    ../src/AudioManager.cpp \
    ../src/GLBLoader.cpp \
    ../src/Camera.cpp \
    $(pkg-config --cflags --libs sdl2 SDL2_mixer glew glm) \
    -lGL -lGLU \
    -o ElectricGuitar3D

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo ""
    echo "Make sure you have:"
    echo "1. shaders/ folder with vertex.glsl and fragment.glsl"
    echo "2. gibson_les_paul_standard_guitar.glb file"
    echo ""
    echo "Run: ./ElectricGuitar3D"
else
    echo "Build failed! Make sure dependencies are installed:"
    echo "Ubuntu/Debian: sudo apt install libsdl2-dev libsdl2-mixer-dev libglew-dev libglm-dev"
    echo "Fedora: sudo dnf install SDL2-devel SDL2_mixer-devel glew-devel glm-devel"
    echo "Arch: sudo pacman -S sdl2 sdl2_mixer glew glm"
fi

cd .. 