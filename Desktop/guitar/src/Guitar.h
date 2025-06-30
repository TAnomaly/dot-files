#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <string>

class AudioManager;

struct GuitarString
{
    int stringNumber;
    std::vector<float> fretFrequencies;
    std::string name;
    float baseFrequency;
};

struct Fret
{
    int stringIndex;
    int fretNumber;
    SDL_Rect rect;
    float frequency;
    std::string noteName;
};

class Guitar
{
private:
    SDL_Renderer *renderer;
    AudioManager *audioManager;
    std::vector<GuitarString> strings;
    std::vector<Fret> frets;

    const int FRET_COUNT = 12;
    const int STRING_COUNT = 6;
    const int FRET_WIDTH = 80;
    const int STRING_HEIGHT = 50;
    const int START_X = 100;
    const int START_Y = 50;

    void initializeStrings();
    void initializeFrets();
    float calculateFretFrequency(float baseFreq, int fretNumber);
    std::string getNoteName(float frequency);
    void drawString(int stringIndex, int y);
    void drawFret(const Fret &fret);
    void drawFretMarkers();

public:
    Guitar(SDL_Renderer *renderer, AudioManager *audioManager);
    void render();
    void handleClick(int x, int y);
};