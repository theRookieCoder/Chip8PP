#include <algorithm>
#include <array>
#include <cstdint>

constexpr auto DISPLAY_WIDTH = 64;
constexpr auto DISPLAY_HEIGHT = 32;

#ifndef CORE_RAM_SIZE
#define CORE_RAM_SIZE 4096
#endif

class MachineState {
 private:
  std::array<uint8_t, CORE_RAM_SIZE> ram;

  uint16_t programCounter;
  uint16_t indexRegister;
  std::array<uint8_t, 16> varRegisters;

  std::array<uint16_t, 12> stack;

  uint8_t delayTimer;

  uint16_t previousKeystate;

 public:
  std::array<std::array<bool, DISPLAY_HEIGHT>, DISPLAY_WIDTH> displayBuffer;

  uint8_t soundTimer;

  MachineState(const std::array<uint8_t, 0x50> &font) {}

  void tickTimer();
};
