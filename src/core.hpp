#pragma once

#include <array>
#include <cstdint>
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
  std::array<Uint8, k_ramSize> ram{};

  Uint16 programCounter = 0x200;
  Uint16 indexRegister = 0;
  std::array<Uint8, 16> varRegisters{};

  std::stack<Uint16> stack{};

  Uint8 delayTimer = 0;

  Uint16 previousKeystate = 0;

  bool highRes = false;

 public:
  DisplayBuffer displayBuffer{};

  Uint8 soundTimer = 0;

  explicit MachineState(std::istream& romFile) noexcept;

  void tickTimer() noexcept;

  void tick(const std::function<Uint16()>& fn_heldKeys,
            const std::function<Uint8()>& fn_random) noexcept;
};
}  // namespace core
