#pragma once
#include <SDL2/SDL_mixer.h>
#include <map>
#include <memory>

class AudioManager
{
private:
    std::map<int, Mix_Chunk *> noteChunks;
    int sampleRate;
    int channels;

    Mix_Chunk *generateSineWave(float frequency, float duration, float volume = 0.5f);
    int getKeyFromFrequency(float frequency);

public:
    AudioManager();
    ~AudioManager();

    bool initialize();
    void playNote(float frequency);
    void cleanup();
};