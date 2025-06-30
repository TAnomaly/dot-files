@echo off
echo Building Electric Guitar 3D Simulator...

:: Clean previous build
if exist "build" (
    echo Cleaning build directory...
    rmdir /s /q build
)

:: Create build directory
mkdir build
cd build

:: Compile with g++ (MSYS2/MinGW)
:: Need OpenGL, GLEW, GLM libraries
echo Installing required packages if missing...
pacman -S --noconfirm mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_mixer mingw-w64-x86_64-glew mingw-w64-x86_64-glm

echo Compiling...
g++ -std=c++17 -O2 ^
    -I../src ^
    -I../shaders ^
    ../src/main.cpp ^
    ../src/Guitar3D.cpp ^
    ../src/AudioManager.cpp ^
    ../src/GLBLoader.cpp ^
    ../src/Camera.cpp ^
    -lmingw32 -lSDL2main -lSDL2 -lSDL2_mixer ^
    -lglew32 -lopengl32 -lglu32 ^
    -mconsole ^
    -o ElectricGuitar3D.exe

if %errorlevel% equ 0 (
    echo Build successful! 
    echo.
    echo Make sure you have:
    echo 1. shaders/ folder with vertex.glsl and fragment.glsl
    echo 2. gibson_les_paul_standard_guitar.glb file
    echo.
    echo Run: ElectricGuitar3D.exe
) else (
    echo Build failed! 
    echo Make sure all dependencies are installed:
    echo pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_mixer
    echo pacman -S mingw-w64-x86_64-glew mingw-w64-x86_64-glm
)

cd ..
pause 