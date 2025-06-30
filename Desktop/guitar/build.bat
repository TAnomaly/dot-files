@echo off
echo Building Electric Guitar Simulator...

:: Create build directory
if not exist "build" mkdir build
cd build

:: Compile with g++ (MSYS2/MinGW)
g++ -std=c++17 -O2 ^
    -I../src ^
    ../src/main.cpp ^
    ../src/Guitar.cpp ^
    ../src/AudioManager.cpp ^
    -lSDL2 -lSDL2_mixer -lSDL2main ^
    -o ElectricGuitar.exe

if %errorlevel% equ 0 (
    echo Build successful! Run: ElectricGuitar.exe
) else (
    echo Build failed! Make sure SDL2 is installed.
    echo Install with: pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_mixer
)

cd ..
pause 