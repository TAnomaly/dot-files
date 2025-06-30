#include "Guitar.h"
#include "AudioManager.h"
#include <cmath>
#include <iostream>

Guitar::Guitar(SDL_Renderer *renderer, AudioManager *audioManager)
    : renderer(renderer), audioManager(audioManager)
{
    initializeStrings();
    initializeFrets();
}

void Guitar::initializeStrings()
{
    // Standard guitar tuning (from low E to high E)
    strings = {
        {0, {}, "E", 82.41f},  // Low E (6th string)
        {1, {}, "A", 110.0f},  // A (5th string)
        {2, {}, "D", 146.83f}, // D (4th string)
        {3, {}, "G", 196.0f},  // G (3rd string)
        {4, {}, "B", 246.94f}, // B (2nd string)
        {5, {}, "E", 329.63f}  // High E (1st string)
    };

    // Calculate frequencies for each fret on each string
    for (auto &guitarString : strings)
    {
        for (int fret = 0; fret <= FRET_COUNT; fret++)
        {
            float frequency = calculateFretFrequency(guitarString.baseFrequency, fret);
            guitarString.fretFrequencies.push_back(frequency);
        }
    }
}

void Guitar::initializeFrets()
{
    frets.clear();

    for (int stringIndex = 0; stringIndex < STRING_COUNT; stringIndex++)
    {
        for (int fretNumber = 0; fretNumber <= FRET_COUNT; fretNumber++)
        {
            Fret fret;
            fret.stringIndex = stringIndex;
            fret.fretNumber = fretNumber;
            fret.frequency = strings[stringIndex].fretFrequencies[fretNumber];
            fret.noteName = getNoteName(fret.frequency);

            // Calculate position
            int x = START_X + fretNumber * FRET_WIDTH;
            int y = START_Y + stringIndex * STRING_HEIGHT;

            fret.rect = {x - 25, y - 15, 50, 30};

            frets.push_back(fret);
        }
    }
}

float Guitar::calculateFretFrequency(float baseFreq, int fretNumber)
{
    // Each fret increases pitch by one semitone
    // Frequency ratio for one semitone: 2^(1/12)
    return baseFreq * std::pow(2.0f, fretNumber / 12.0f);
}

std::string Guitar::getNoteName(float frequency)
{
    // Note names in chromatic order
    std::vector<std::string> noteNames = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

    // A4 = 440 Hz is our reference (note index 9 in 4th octave)
    float a4 = 440.0f;
    float noteNumber = 12 * std::log2(frequency / a4) + 69;
    int noteIndex = ((int)noteNumber) % 12;
    if (noteIndex < 0)
        noteIndex += 12;

    int octave = ((int)noteNumber + 8) / 12;

    return noteNames[noteIndex] + std::to_string(octave);
}

void Guitar::render()
{
    // Draw background
    SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255); // Brown wood color
    SDL_Rect background = {START_X - 50, START_Y - 30, FRET_COUNT * FRET_WIDTH + 100, STRING_COUNT * STRING_HEIGHT + 60};
    SDL_RenderFillRect(renderer, &background);

    // Draw fret markers
    drawFretMarkers();

    // Draw strings
    for (int i = 0; i < STRING_COUNT; i++)
    {
        drawString(i, START_Y + i * STRING_HEIGHT);
    }

    // Draw frets
    for (const auto &fret : frets)
    {
        drawFret(fret);
    }
}

void Guitar::drawString(int stringIndex, int y)
{
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255); // Silver string color

    int thickness = 6 - stringIndex; // Thicker strings for lower notes
    if (thickness < 1)
        thickness = 1;

    for (int i = 0; i < thickness; i++)
    {
        SDL_RenderDrawLine(renderer, START_X - 40, y + i, START_X + FRET_COUNT * FRET_WIDTH + 40, y + i);
    }
}

void Guitar::drawFret(const Fret &fret)
{
    // Draw fret wire (vertical lines)
    if (fret.fretNumber > 0)
    {
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        int x = START_X + fret.fretNumber * FRET_WIDTH;
        SDL_RenderDrawLine(renderer, x, START_Y - 20, x, START_Y + STRING_COUNT * STRING_HEIGHT);
    }

    // Draw clickable area (invisible, just for hit detection)
    // We'll highlight it when hovered or clicked
}

void Guitar::drawFretMarkers()
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // Standard fret markers (3rd, 5th, 7th, 9th, 12th frets)
    std::vector<int> markerFrets = {3, 5, 7, 9, 12};

    for (int fretNum : markerFrets)
    {
        int x = START_X + fretNum * FRET_WIDTH - FRET_WIDTH / 2;
        int y = START_Y + STRING_COUNT * STRING_HEIGHT / 2;

        // Draw circle marker
        for (int w = 0; w < 8; w++)
        {
            for (int h = 0; h < 8; h++)
            {
                int dx = w - 4;
                int dy = h - 4;
                if ((dx * dx + dy * dy) <= 16)
                {
                    SDL_RenderDrawPoint(renderer, x + dx, y + dy);
                }
            }
        }

        // Double dots for 12th fret
        if (fretNum == 12)
        {
            y -= 20;
            for (int w = 0; w < 8; w++)
            {
                for (int h = 0; h < 8; h++)
                {
                    int dx = w - 4;
                    int dy = h - 4;
                    if ((dx * dx + dy * dy) <= 16)
                    {
                        SDL_RenderDrawPoint(renderer, x + dx, y + dy);
                    }
                }
            }
            y += 40;
            for (int w = 0; w < 8; w++)
            {
                for (int h = 0; h < 8; h++)
                {
                    int dx = w - 4;
                    int dy = h - 4;
                    if ((dx * dx + dy * dy) <= 16)
                    {
                        SDL_RenderDrawPoint(renderer, x + dx, y + dy);
                    }
                }
            }
        }
    }
}

void Guitar::handleClick(int x, int y)
{
    // Find which fret was clicked
    for (const auto &fret : frets)
    {
        if (x >= fret.rect.x && x <= fret.rect.x + fret.rect.w &&
            y >= fret.rect.y && y <= fret.rect.y + fret.rect.h)
        {

            std::cout << "Playing: " << fret.noteName << " (" << fret.frequency << " Hz)" << std::endl;
            audioManager->playNote(fret.frequency);

            // Visual feedback - highlight the played fret
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 128); // Yellow highlight
            SDL_RenderFillRect(renderer, &fret.rect);

            break;
        }
    }
}