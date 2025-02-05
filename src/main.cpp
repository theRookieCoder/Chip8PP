#include <SDL3/SDL.h>

#include <chrono>
#include <iostream>

#include "core.hpp"
#include "default_font.hpp"

#define OFF_COLOUR_R 0x8f
#define OFF_COLOUR_G 0x91
#define OFF_COLOUR_B 0x85

#define ON_COLOUR_R 0x11
#define ON_COLOUR_G 0x1d
#define ON_COLOUR_B 0x2b

int main(int argc, char* argv[]) {
  SDL_SetAppMetadata("Chip8++", "1.0", "io.github.theRookieCoder.chip8pp");
  SDL_Init(SDL_INIT_VIDEO);

  static SDL_Window* window = NULL;
  static SDL_Renderer* renderer = NULL;

  SDL_CreateWindowAndRenderer("Chip8++", 1280, 640, 0, &window, &renderer);
  SDL_SetRenderLogicalPresentation(renderer, DISPLAY_WIDTH, DISPLAY_HEIGHT,
                                   SDL_LOGICAL_PRESENTATION_LETTERBOX);

  SDL_SetRenderDrawColor(renderer, OFF_COLOUR_R, OFF_COLOUR_G, OFF_COLOUR_B,
                         SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);

  auto machineState = MachineState(kDefaultFont);
  Uint64 previousTick = 0;

  SDL_Event event;
  bool quit = false;

  while (!quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) quit = true;
    }

    SDL_SetRenderDrawColor(renderer, OFF_COLOUR_R, OFF_COLOUR_G, OFF_COLOUR_B,
                           SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    SDL_RenderPresent(renderer);
  }

  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
