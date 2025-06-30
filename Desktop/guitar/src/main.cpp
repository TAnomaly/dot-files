#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <GL/glew.h>
#include <iostream>
#include <memory>
#include "Guitar3D.h"
#include "AudioManager.h"

const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 800;

int main(int argc, char *argv[])
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        std::cerr << "SDL_mixer init failed: " << Mix_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    // Allocate mixing channels
    Mix_AllocateChannels(16); // 16 channel for simultaneous sounds

    std::cout << "SDL_mixer initialized successfully" << std::endl;
    std::cout << "Audio format: " << MIX_DEFAULT_FORMAT << std::endl;
    std::cout << "Audio channels: 2" << std::endl;
    std::cout << "Audio frequency: 44100" << std::endl;
    std::cout << "Mixing channels: " << Mix_AllocateChannels(-1) << std::endl;

    // Create window with OpenGL context
    SDL_Window *window = SDL_CreateWindow(
        "Electric Guitar 3D Simulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (!window)
    {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        Mix_CloseAudio();
        SDL_Quit();
        return -1;
    }

    // Create OpenGL context
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext)
    {
        std::cerr << "OpenGL context creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        Mix_CloseAudio();
        SDL_Quit();
        return -1;
    }

    // Enable VSync
    SDL_GL_SetSwapInterval(1);

    // Create audio manager and 3D guitar
    auto audioManager = std::make_unique<AudioManager>();
    auto guitar3D = std::make_unique<Guitar3D>(WINDOW_WIDTH, WINDOW_HEIGHT, audioManager.get());

    // Initialize guitar
    if (!guitar3D->initialize())
    {
        std::cerr << "Failed to initialize Guitar3D" << std::endl;
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        Mix_CloseAudio();
        SDL_Quit();
        return -1;
    }

    // Main loop
    bool running = true;
    SDL_Event event;
    bool mouseDown = false;
    int lastMouseX = 0, lastMouseY = 0;

    std::cout << "3D Guitar Simulator ready!" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "- Left click: Play guitar notes" << std::endl;
    std::cout << "- Right click + drag: Rotate camera" << std::endl;
    std::cout << "- Mouse wheel: Zoom in/out" << std::endl;

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = false;
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    int windowWidth, windowHeight;
                    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
                    guitar3D->handleClick(event.button.x, event.button.y, windowWidth, windowHeight);
                }
                else if (event.button.button == SDL_BUTTON_RIGHT)
                {
                    mouseDown = true;
                    lastMouseX = event.button.x;
                    lastMouseY = event.button.y;
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_RIGHT)
                {
                    mouseDown = false;
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                }
                break;

            case SDL_MOUSEMOTION:
                if (mouseDown)
                {
                    guitar3D->handleMouseMotion(event.motion.xrel, event.motion.yrel);
                }
                break;

            case SDL_MOUSEWHEEL:
                guitar3D->handleMouseWheel(event.wheel.y);
                break;

            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    int width = event.window.data1;
                    int height = event.window.data2;
                    guitar3D->resize(width, height);
                }
                break;
            }
        }

        // Render
        guitar3D->render();

        // Swap buffers
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    SDL_Quit();

    return 0;
}