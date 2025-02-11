#include <SDL3/SDL.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <print>
#include <random>
#include <ranges>

#include "core.hpp"

#define EXTRACT_COLOUR(colour) \
  (colour >> 16) & 0xFF, (colour >> 8) & 0xFF, colour & 0xFF

class SdlHousekeeper {
  SDL_Window* p_window = nullptr;
  SDL_Renderer* p_renderer = nullptr;

 public:
  unsigned int onColour = 0x111D2B;
  unsigned int offColour = 0x8F9185;

  SdlHousekeeper() {
    SDL_SetAppMetadata("Chip8++", "1.0", "io.github.theRookieCoder.chip8pp");
    SDL_Init(SDL_INIT_VIDEO);

    SDL_CreateWindowAndRenderer("Chip8++", 1280, 640, SDL_WINDOW_RESIZABLE,
                                &p_window, &p_renderer);
    SDL_SetRenderLogicalPresentation(p_renderer, core::k_displayWidth,
                                     core::k_displayHeight,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);

    SDL_SetRenderDrawColor(p_renderer, EXTRACT_COLOUR(offColour),
                           SDL_ALPHA_OPAQUE);
    SDL_RenderClear(p_renderer);
    SDL_RenderPresent(p_renderer);
  }

  ~SdlHousekeeper() {
    SDL_DestroyRenderer(p_renderer);
    SDL_DestroyWindow(p_window);
    SDL_Quit();
  }

  void drawDisplayBuffer(const core::DisplayBuffer& displayBuffer) const {
    SDL_SetRenderDrawColor(p_renderer, EXTRACT_COLOUR(offColour),
                           SDL_ALPHA_OPAQUE);
    SDL_RenderClear(p_renderer);

    SDL_SetRenderDrawColor(p_renderer, EXTRACT_COLOUR(onColour),
                           SDL_ALPHA_OPAQUE);
    for (const auto& [x, column] : std::views::enumerate(displayBuffer))
      for (const auto& [y, pixel] : std::views::enumerate(column))
        if (pixel) SDL_RenderPoint(p_renderer, x, y);

    SDL_RenderPresent(p_renderer);
  }
};

constexpr auto k_instructionsPerFrame = 11;
constexpr Uint64 k_timePeriodNS = 1'000'000'000 / 60;

constexpr std::array<SDL_Scancode, 16> k_keymap{
    SDL_SCANCODE_X, SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3,
    SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_A,
    SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_Z, SDL_SCANCODE_C,
    SDL_SCANCODE_4, SDL_SCANCODE_R, SDL_SCANCODE_F, SDL_SCANCODE_V,
};

int main(int argc, char* p_argv[]) {
  if (argc < 2) {
    std::println(std::cerr, "Error: provide a ROM file to load");
    return 1;
  }

  std::ifstream romFile(p_argv[1], std::ios_base::binary);
  if (!romFile) {
    std::println(std::cerr, "Error: Could not read ROM file: {}",
                 strerror(errno));
    return 1;
  }

  SdlHousekeeper sdl;
  static core::MachineState machineState(romFile);
  srand(time(nullptr));

  Uint64 previousTick = 0;
  Uint16 heldKeys = 0;
  SDL_Event event{};
  bool quit = false;

  while (!quit) {
    // Process SDL events
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_EVENT_QUIT:
          quit = true;
          break;

        case SDL_EVENT_KEY_DOWN:
          for (const auto& [i, key] : std::views::enumerate(k_keymap))
            if (event.key.scancode == key) heldKeys |= 0b1 << i;
          break;

        case SDL_EVENT_KEY_UP:
          for (const auto& [i, key] : std::views::enumerate(k_keymap))
            if (event.key.scancode == key) heldKeys -= 0b1 << i;
          break;

        default:;
      }
    }

    const Uint64 delta = SDL_GetTicksNS() - previousTick;
    if (delta < k_timePeriodNS) continue;

    previousTick += k_timePeriodNS;

    machineState.tickTimer();

    for (const auto& _ : std::array<int, k_instructionsPerFrame>{})
      machineState.tick([heldKeys]() { return heldKeys; }, rand);

    sdl.drawDisplayBuffer(machineState.displayBuffer);

    SDL_DelayNS(k_timePeriodNS - 1'000'000);
  }

  return 0;
}
