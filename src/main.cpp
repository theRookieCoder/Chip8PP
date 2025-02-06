#include <SDL3/SDL.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <print>
#include <random>
#include <ranges>

#include "core.hpp"
#include "default_font.hpp"

#define OFF_COLOUR_R 0x8f
#define OFF_COLOUR_G 0x91
#define OFF_COLOUR_B 0x85

#define ON_COLOUR_R 0x11
#define ON_COLOUR_G 0x1d
#define ON_COLOUR_B 0x2b

class SdlHousekeeper {
  SDL_Window* window;
  SDL_Renderer* renderer;

 public:
  SdlHousekeeper() {
    SDL_SetAppMetadata("Chip8++", "1.0", "io.github.theRookieCoder.chip8pp");
    SDL_Init(SDL_INIT_VIDEO);

    SDL_CreateWindowAndRenderer("Chip8++", 1280, 640, SDL_WINDOW_RESIZABLE,
                                &window, &renderer);
    SDL_SetRenderLogicalPresentation(renderer, DISPLAY_WIDTH, DISPLAY_HEIGHT,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);

    SDL_SetRenderDrawColor(renderer, OFF_COLOUR_R, OFF_COLOUR_G, OFF_COLOUR_B,
                           SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
  }

  ~SdlHousekeeper() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
  }

  void drawDisplayBuffer(std::array<std::array<bool, DISPLAY_HEIGHT>,
                                    DISPLAY_WIDTH>& displayBuffer) {
    SDL_SetRenderDrawColor(renderer, OFF_COLOUR_R, OFF_COLOUR_G, OFF_COLOUR_B,
                           SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, ON_COLOUR_R, ON_COLOUR_G, ON_COLOUR_B,
                           SDL_ALPHA_OPAQUE);
    for (auto const [x, column] : std::views::enumerate(displayBuffer))
      for (auto const [y, pixel] : std::views::enumerate(column))
        if (pixel) SDL_RenderPoint(renderer, x, y);

    SDL_RenderPresent(renderer);
  }
};

constexpr auto kInstructionsPerFrame = 11;
constexpr std::array<SDL_Scancode, 16> kKeymap = {
    SDL_SCANCODE_X, SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3,
    SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_A,
    SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_Z, SDL_SCANCODE_C,
    SDL_SCANCODE_4, SDL_SCANCODE_R, SDL_SCANCODE_F, SDL_SCANCODE_V,
};

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::println("Error: provide a ROM file to load");
    return 1;
  }

  std::ifstream romFile(argv[1]);
  if (!romFile) {
    std::println("Error: Could not read ROM file");
    return 1;
  }

  SdlHousekeeper sdl;
  static auto machineState = MachineState(romFile);

  constexpr Uint64 time_period = 1'000'000'000 / 60;
  Uint64 previousTick = 0;

  SDL_Event event{};
  bool quit = false;
  Uint16 heldKeys = 0;
  srand(time(NULL));

  while (!quit) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_EVENT_QUIT:
          quit = true;
          break;

        case SDL_EVENT_KEY_DOWN:
          for (const auto [i, key] : std::views::enumerate(kKeymap))
            if (event.key.scancode == key) heldKeys |= 0b1 << i;
          break;

        case SDL_EVENT_KEY_UP:
          for (const auto [i, key] : std::views::enumerate(kKeymap))
            if (event.key.scancode == key) heldKeys -= 0b1 << i;
          break;
      }
    }

    const Uint64 delta = SDL_GetTicksNS() - previousTick;
    if (delta < time_period)
      continue;
    else
      previousTick += time_period;

    machineState.tickTimer();

    for (const auto _ : std::array<int, 11>())
      machineState.tick([heldKeys]() { return heldKeys; },
                        []() { return rand(); });

    sdl.drawDisplayBuffer(machineState.displayBuffer);

    SDL_DelayNS(time_period - 1'000'000);
  }

  return 0;
}
