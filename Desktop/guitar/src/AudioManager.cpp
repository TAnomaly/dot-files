#include "AudioManager.h"
#include <cmath>
#include <iostream>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

AudioManager::AudioManager() : sampleRate(44100), channels(2)
{
    initialize();
}

AudioManager::~AudioManager()
{
    cleanup();
}

bool AudioManager::initialize()
{
    // SDL_mixer is already initialized in main.cpp
    return true;
}

void AudioManager::playNote(float frequency)
{
    int key = getKeyFromFrequency(frequency);

    // Check if we already have this note cached
    if (noteChunks.find(key) == noteChunks.end())
    {
        // Generate new note
        noteChunks[key] = generateSineWave(frequency, 0.8f, 0.5f); // 0.8 seconds, 50% volume
    }

    if (noteChunks[key])
    {
        // Play the note
        Mix_PlayChannel(-1, noteChunks[key], 0);
    }
}

Mix_Chunk *AudioManager::generateSineWave(float frequency, float duration, float volume)
{
    int samples = (int)(sampleRate * duration);
    int bytes = samples * channels * sizeof(Sint16);

    Sint16 *buffer = new Sint16[samples * channels];

    for (int i = 0; i < samples; i++)
    {
        // Generate sine wave with envelope (fade out)
        float time = (float)i / sampleRate;
        float envelope = 1.0f;

        // Add fade out to prevent clicking
        if (time > duration * 0.7f)
        {
            envelope = 1.0f - (time - duration * 0.7f) / (duration * 0.3f);
        }

        // Add some harmonics for more guitar-like sound
        float sample = sin(2.0f * M_PI * frequency * time) * 0.6f;    // Fundamental
        sample += sin(2.0f * M_PI * frequency * 2.0f * time) * 0.2f;  // 2nd harmonic
        sample += sin(2.0f * M_PI * frequency * 3.0f * time) * 0.1f;  // 3rd harmonic
        sample += sin(2.0f * M_PI * frequency * 4.0f * time) * 0.05f; // 4th harmonic

        sample *= volume * envelope;

        // Convert to 16-bit integer
        Sint16 sampleValue = (Sint16)(sample * 32767);

        // Stereo - same value for both channels
        buffer[i * channels] = sampleValue;     // Left
        buffer[i * channels + 1] = sampleValue; // Right
    }

    // Create Mix_Chunk
    Mix_Chunk *chunk = new Mix_Chunk;
    chunk->allocated = 1;
    chunk->abuf = (Uint8 *)buffer;
    chunk->alen = bytes;
    chunk->volume = MIX_MAX_VOLUME;

    return chunk;
}

int AudioManager::getKeyFromFrequency(float frequency)
{
    // Convert frequency to a key for caching
    // We'll round to nearest semitone
    float a4 = 440.0f;
    float noteNumber = 12 * log2(frequency / a4) + 69;
    return (int)round(noteNumber);
}

void AudioManager::cleanup()
{
    for (auto &pair : noteChunks)
    {
        if (pair.second)
        {
            delete[] pair.second->abuf;
            delete pair.second;
        }
    }
    noteChunks.clear();
}