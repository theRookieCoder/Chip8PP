#include "core.hpp"

#include "default_font.hpp"

MachineState::MachineState(const std::array<uint8_t, 0x50> &font) {
  std::ranges::copy(font, &ram[0x50]);
}

void MachineState::tickTimer() {
  if (delayTimer > 0) delayTimer--;
  if (soundTimer > 0) soundTimer--;
}
