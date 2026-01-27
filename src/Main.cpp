#include <SDL.h>
#include <SDL_image.h>
#include <iostream>

// Screen dimensions
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int argc, char* argv[])
{
    // -----------------------
    // Initialize SDL subsystems
    // -----------------------
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    {
        std::cerr << "IMG_Init Error: " << IMG_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    // -----------------------
    // Create window and renderer
    // -----------------------
    SDL_Window* window = SDL_CreateWindow(
        "SDL2 Game Loop",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN);

    if (!window)
    {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // -----------------------
    // Main game loop
    // -----------------------
    bool running = true;
    SDL_Event event;
    const int FPS = 60;
    const int frameDelay = 1000 / FPS;

    while (running)
    {
        Uint32 frameStart = SDL_GetTicks();

        // --- Event handling ---
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    running = false;
                }
            }
        }

        // --- Rendering ---
        SDL_SetRenderDrawColor(renderer, 30, 30, 60, 255); // dark blue background
        SDL_RenderClear(renderer);

        // Example: draw a simple white square for the player
        SDL_Rect playerRect;
        playerRect.x = 50;
        playerRect.y = 50;
        playerRect.w = 50;
        playerRect.h = 50;
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &playerRect);

        SDL_RenderPresent(renderer);

        // --- Frame limiter ---
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime)
        {
            SDL_Delay(frameDelay - frameTime);
        }
    }

    // -----------------------
    // Cleanup
    // -----------------------
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
