#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <functional>
#include <ranges>
#include <stack>

constexpr auto DISPLAY_WIDTH = 128;
constexpr auto DISPLAY_HEIGHT = 64;

typedef uint8_t Uint8;
typedef uint16_t Uint16;

#ifndef CORE_RAM_SIZE
#define CORE_RAM_SIZE 4096
#endif

class MachineState {
 private:
  std::array<Uint8, CORE_RAM_SIZE> ram;

  Uint16 programCounter;
  Uint16 indexRegister;
  std::array<Uint8, 16> varRegisters;

  std::stack<Uint16> stack;

  Uint8 delayTimer;

  Uint16 previousKeystate;

  bool highRes;

 public:
  std::array<std::array<bool, DISPLAY_HEIGHT>, DISPLAY_WIDTH> displayBuffer;

  Uint8 soundTimer;

  MachineState(std::istream &romFile);

  void tickTimer();

  void tick(std::function<Uint16()> heldKeys, std::function<Uint8()> random);
};
