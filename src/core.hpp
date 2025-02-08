#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <functional>
#include <ranges>
#include <stack>

namespace core {

constexpr auto k_displayWidth = 128;
constexpr auto k_displayHeight = 64;
constexpr auto k_ramSize = 4096;

typedef std::array<std::array<bool, k_displayHeight>, k_displayWidth>
    DisplayBuffer;
typedef uint8_t Uint8;
typedef uint16_t Uint16;

class MachineState {
 private:
  std::array<Uint8, k_ramSize> ram;

  Uint16 programCounter;
  Uint16 indexRegister;
  std::array<Uint8, 16> varRegisters;

  std::stack<Uint16> stack;

  Uint8 delayTimer;

  Uint16 previousKeystate;

  bool highRes;

 public:
  DisplayBuffer displayBuffer;

  Uint8 soundTimer;

  MachineState(std::istream &romFile) noexcept;

  void tickTimer() noexcept;

  void tick(std::function<Uint16()> fn_heldKeys,
            std::function<Uint8()> fn_random) noexcept;
};

}  // namespace core
