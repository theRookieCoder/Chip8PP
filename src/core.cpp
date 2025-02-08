#pragma warning(disable : 4365)

#include "core.hpp"
using namespace core;

#include "default_font.hpp"

MachineState::MachineState(std::istream &romFile) noexcept {
  std::ranges::copy(k_defaultFont, &ram[0x50]);
  std::ranges::copy(k_defaultBigFont, &ram[0xA0]);

  romFile.read((char *)&ram.at(0x200), k_ramSize - 0x200);

  programCounter = 0x200;
}

void MachineState::tickTimer() noexcept {
  if (delayTimer > 0) delayTimer--;
  if (soundTimer > 0) soundTimer--;
}

void MachineState::tick(std::function<Uint16()> heldKeys,
                        std::function<Uint8()> random) noexcept {
  /* FETCH */
  const Uint16 instruction =
      (ram[programCounter] << 8) + ram[programCounter + 1];
  programCounter += 2;


  /* DECODE */
#define X ((instruction & 0x0F00) >> 8)
#define Y ((instruction & 0x00F0) >> 4)
#define N (instruction & 0x000F)
#define NN (instruction & 0x00FF)
#define NNN (instruction & 0x0FFF)

#define VX varRegisters[X]
#define VY varRegisters[Y]
#define VF varRegisters[0xF]


  /* EXECUTE */

  // 00E0  Clear display
  if (instruction == 0x00E0) {
    displayBuffer = {};
  }

  // 00EE  Return from subroutine
  else if (instruction == 0x00EE) {
    programCounter = stack.top();
    stack.pop();
  }

  // 1NNN  Jump to address NNN
  else if ((instruction & 0xF000) == 0x1000) {
    programCounter = NNN;
  }

  // 2NNN  Goto subroutine at NNN
  else if ((instruction & 0xF000) == 0x2000) {
    stack.push(programCounter);
    programCounter = NNN;
  }

  // 3xNN  Skip if Vx == NN
  else if ((instruction & 0xF000) == 0x3000) {
    if (VX == NN) programCounter += 2;
  }

  // 4xNN  Skip if Vx != NN
  else if ((instruction & 0xF000) == 0x4000) {
    if (VX != NN) programCounter += 2;
  }

  // 5xy0  Skip if Vx == Vy
  else if ((instruction & 0xF000) == 0x5000) {
    if (VX == VY) programCounter += 2;
  }

  // 9xy0  Skip if Vx != Vy
  else if ((instruction & 0xF000) == 0x9000) {
    if (VX != VY) programCounter += 2;
  }

  // 8xy0  Set Vx to Vy
  else if ((instruction & 0xF00F) == 0x8000) {
    VX = VY;
  }

  // 8xy1  Set Vx to (Vx OR Vy)
  else if ((instruction & 0xF00F) == 0x8001) {
    VX |= VY;
  }

  // 8xy2  Set Vx to (Vx AND Vy)
  else if ((instruction & 0xF00F) == 0x8002) {
    VX &= VY;
  }

  // 8xy3  Set Vx to (Vx XOR Vy)
  else if ((instruction & 0xF00F) == 0x8003) {
    VX ^= VY;
  }

  // 8xy4  Set Vx to (Vx + Vy) with overflow on VF
  else if ((instruction & 0xF00F) == 0x8004) {
    Uint8 overflowFlag = (VX + VY > 0xFF) ? 1 : 0;
    VX = VX + VY;

    VF = overflowFlag;
  }

  // 8xy5  Set Vx to (Vx - Vy) with carry on VF
  else if ((instruction & 0xF00F) == 0x8005) {
    Uint8 carryFlag = VX >= VY ? 1 : 0;
    VX = VX - VY;
    VF = carryFlag;
  }

  // 8xy7  Set Vx to (Vy - Vx) with carry on VF
  else if ((instruction & 0xF00F) == 0x8007) {
    Uint8 carryFlag = VY >= VX ? 1 : 0;
    VX = VY - VX;
    VF = carryFlag;
  }

  // 8xy6  Set Vx to (Vy >> 1) with shifted-out bit on VF
  else if ((instruction & 0xF00F) == 0x8006) {
    bool shiftedOut = VY & 0b00000001;
    VX = VX >> 1;
    VF = shiftedOut;
  }

  // 8xyE  Set Vx to (Vy << 1) with shifted-out bit on VF
  else if ((instruction & 0xF00F) == 0x800E) {
    bool shiftedOut = (VY & 0b10000000) >> 7;
    VX = VX << 1;
    VF = shiftedOut;
  }

  // 6xNN  Set Vx to NN
  else if ((instruction & 0xF000) == 0x6000) {
    VX = NN;
  }

  // 7xNN  Add NN to Vx
  else if ((instruction & 0xF000) == 0x7000) {
    VX += NN;
  }

  // ANNN  Set I to NNN
  else if ((instruction & 0xF000) == 0xA000) {
    indexRegister = NNN;
  }

  // BNNN  Set PC to (NNN + V0)
  else if ((instruction & 0xF000) == 0xB000) {
    programCounter = NNN + varRegisters[0];
  }

  // FxNN  Set Vx to a random number ANDed with NN
  else if ((instruction & 0xF000) == 0xC000) {
    VX = random() & NN;
  }

  // DxyN  Draw
  else if ((instruction & 0xF000) == 0xD000) {
    if (highRes) {
      const int x = VX % k_displayWidth;
      const int y = VY % k_displayHeight;

      const bool sprite16 = N == 0;
      const int n = sprite16 ? 16 : N;
      VF = 0;

      for (const auto i : std::ranges::views::iota(0, n)) {
        if (y + i >= k_displayHeight) {
          VF += (n - i);
          break;
        }

        const Uint16 spriteRow = sprite16 ? (ram[indexRegister + i * 2] << 8) +
                                                ram[indexRegister + i * 2 + 1]
                                          : ram[indexRegister + i] << 8;
        bool collision = false;

        for (const auto j : std::ranges::views::iota(0, 16)) {
          if (x + j >= k_displayWidth) break;

          const bool pixel = ((spriteRow >> (15 - j)) & 0b1) == 1;

          if (pixel) {
            if (displayBuffer[x + j][y + i]) collision = true;

            displayBuffer[x + j][y + i] = !displayBuffer[x + j][y + i];
          }
        }

        if (collision) VF += 1;
      }


    } else {
      const int x = VX % (k_displayWidth / 2);
      const int y = VY % (k_displayHeight / 2);
      VF = 0;

      for (const auto i : std::ranges::views::iota(0, N)) {
        if (2 * (y + i) >= k_displayHeight) break;

        const Uint8 spriteRow = ram[indexRegister + i];

        for (const auto j : std::ranges::views::iota(0, 8)) {
          if (2 * (x + j) >= k_displayWidth) break;

          if (((spriteRow >> (7 - j)) & 0b1) == 1) {
            if (displayBuffer[2 * (x + j)][2 * (y + i)]) VF = 1;

            displayBuffer[2 * (x + j) + 0][2 * (y + i) + 0] =
                !displayBuffer[2 * (x + j) + 0][2 * (y + i) + 0];
            displayBuffer[2 * (x + j) + 0][2 * (y + i) + 1] =
                !displayBuffer[2 * (x + j) + 0][2 * (y + i) + 1];
            displayBuffer[2 * (x + j) + 1][2 * (y + i) + 0] =
                !displayBuffer[2 * (x + j) + 1][2 * (y + i) + 0];
            displayBuffer[2 * (x + j) + 1][2 * (y + i) + 1] =
                !displayBuffer[2 * (x + j) + 1][2 * (y + i) + 1];
          }
        }
      }
    }
  }

  // Ex9E  Skip if key (Vx & 0xF) is held
  else if ((instruction & 0xF0FF) == 0xE09E) {
    if ((heldKeys() >> (VX & 0xF)) & 0b1) programCounter += 2;
  }

  // ExA1  Skip if key (Vx & 0xF) is not held
  else if ((instruction & 0xF0FF) == 0xE0A1) {
    if (!((heldKeys() >> (VX & 0xF)) & 0b1)) programCounter += 2;
  }

  // Fx07  Set Vx to the value in the delay timer
  else if ((instruction & 0xF0FF) == 0xF007) {
    VX = delayTimer;
  }

  // Fx15  Set delay timer to the value in Vx
  else if ((instruction & 0xF0FF) == 0xF015) {
    delayTimer = VX;
  }

  // Fx18  Set sound timer to the value in Vx
  else if ((instruction & 0xF0FF) == 0xF018) {
    soundTimer = VX;
  }

  // Fx1E  Add Vx to I
  else if ((instruction & 0xF0FF) == 0xF01E) {
    indexRegister += VX;
  }

  // Fx0A  Wait for key (Vx & 0xF) to be released
  else if ((instruction & 0xF0FF) == 0xF00A) {
    Uint16 currentHeldKeys = heldKeys();

    if (currentHeldKeys < previousKeystate) {
      Uint16 keysDiff = previousKeystate - currentHeldKeys;
      for (int i = 0; i < 16; i++)
        if (keysDiff >> i & 0b1) {
          VX = i;
          break;
        };
      previousKeystate = 0;
    } else {
      previousKeystate = currentHeldKeys;
      programCounter -= 2;
    }
  }

  // Fx29  Set I to the font character (Vx & 0xF)
  else if ((instruction & 0xF0FF) == 0xF029) {
    indexRegister = 0x50 + (VX & 0xF) * 5;
  }

  // Fx33  Store Vx as binary-coded-decimal in RAM at I
  else if ((instruction & 0xF0FF) == 0xF033) {
    ram[(indexRegister + 2)] = (VX / 1) % 10;
    ram[(indexRegister + 1)] = (VX / 10) % 10;
    ram[(indexRegister + 0)] = (VX / 100) % 10;
  }

  // Fx55  Store variables V0 to Vx in RAM at I
  else if ((instruction & 0xF0FF) == 0xF055) {
    for (int i = 0; i <= X; i++) ram[indexRegister + i] = varRegisters[i];
  }

  // Fx65  Set variables V0 to Vx from RAM at I
  else if ((instruction & 0xF0FF) == 0xF065) {
    for (int i = 0; i <= X; i++) varRegisters[i] = ram[indexRegister + i];
  }


  /* SUPERCHIP INSTRUCTIONS */

  // 00FD  Halt the emulator
  else if (instruction == 0x00FD) {
    // TODO: Exit/halt the emulator
  }

  // 00FE  Reset high resolution mode
  else if (instruction == 0x00FE) {
    highRes = false;
  }

  // 00FF  Set high resolution mode
  else if (instruction == 0x00FF) {
    highRes = true;
  }

  // Fx75  Store V0 to Vx in persistent storage
  else if ((instruction & 0xF0FF) == 0xF075) {
    // TODO: Persistent storage
  }

  // Fx85  Retrieve V0 to Vx from persistent storage
  else if ((instruction & 0xF0FF) == 0xF085) {
    // TODO: Retrieve from persistent storage
  }

  // 00FB  Scroll display 4 pixels right
  else if (instruction == 0x00FB) {
    const auto n = highRes ? 4 : 8;

    std::move_backward(displayBuffer.begin(), displayBuffer.end() - n,
                       displayBuffer.end());
    std::fill_n(displayBuffer.begin(), n, std::array<bool, k_displayHeight>());
  }

  // 00FC  Scroll display 4 pixels left
  else if (instruction == 0x00FC) {
    const auto n = highRes ? 4 : 8;

    std::move(displayBuffer.begin() + n, displayBuffer.end(),
              displayBuffer.begin());
    std::fill(displayBuffer.end() - n, displayBuffer.end(),
              std::array<bool, k_displayHeight>());
  }

  // 00Cn  Scroll n pixels down
  else if ((instruction & 0xFFF0) == 0x00C0) {
    const auto n = highRes ? N : N * 2;

    for (auto column : displayBuffer) {
      std::move_backward(column.begin(), column.end() - n, column.end());
      std::fill_n(column.begin(), n, false);
    }
  }

  // Fx30  Set I to the big font character (Vx & 0xF)
  else if ((instruction & 0xF0FF) == 0xF030) {
    indexRegister = 0x0A0 + (VX & 0xF) * 10;
  }

  else {
    // TODO: Illegal instruction
  }
}
