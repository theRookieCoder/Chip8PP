#pragma warning(disable : 4365)

#include "core.hpp"
using namespace core;

#include <algorithm>
#include <istream>

#include "default_font.hpp"


#define IOTA(n) std::views::iota(0, n)

MachineState::MachineState(std::istream& romFile) noexcept {
  std::ranges::copy(k_defaultFont, &ram[0x50]);
  std::ranges::copy(k_defaultBigFont, &ram[0xA0]);

  romFile.read(reinterpret_cast<char*>(&ram.at(0x200)), k_ramSize - 0x200);
}


void MachineState::tickTimer() noexcept {
  if (delayTimer > 0) delayTimer--;
  if (soundTimer > 0) soundTimer--;
}


void MachineState::tick(const std::function<Uint16()>& fn_heldKeys,
                        const std::function<Uint8()>& fn_random) noexcept {
  /* FETCH */
  const Uint16 instruction =
      (ram[programCounter] << 8) + ram[programCounter + 1];
  programCounter += 2;


  /* DECODE */
  const auto x = (instruction & 0x0F00) >> 8;
  const auto y = (instruction & 0x00F0) >> 4;
  const auto n = instruction & 0x000F;
  const auto nn = instruction & 0x00FF;
  const auto nnn = instruction & 0x0FFF;

  auto& vx = varRegisters[x];
  const auto vy = varRegisters[y];
  auto& vf = varRegisters[0xF];


  /* EXECUTE */

  // 00E0  Clear display
  if (instruction == 0x00E0) {
    // std::ranges::fill(displayBuffer.begin(), displayBuffer.end(),
    // DisplayRow{});
    displayBuffer = {};
  }

  // 00EE  Return from subroutine
  else if (instruction == 0x00EE) {
    programCounter = stack.top();
    stack.pop();
  }

  // 1NNN  Jump to address NNN
  else if ((instruction & 0xF000) == 0x1000) {
    programCounter = nnn;
  }

  // 2NNN  Goto subroutine at NNN
  else if ((instruction & 0xF000) == 0x2000) {
    stack.push(programCounter);
    programCounter = nnn;
  }

  // 3xNN  Skip if Vx == NN
  else if ((instruction & 0xF000) == 0x3000) {
    if (vx == nn) programCounter += 2;
  }

  // 4xNN  Skip if Vx != NN
  else if ((instruction & 0xF000) == 0x4000) {
    if (vx != nn) programCounter += 2;
  }

  // 5xy0  Skip if Vx == Vy
  else if ((instruction & 0xF000) == 0x5000) {
    if (vx == vy) programCounter += 2;
  }

  // 9xy0  Skip if Vx != Vy
  else if ((instruction & 0xF000) == 0x9000) {
    if (vx != vy) programCounter += 2;
  }

  // 8xy0  Set Vx to Vy
  else if ((instruction & 0xF00F) == 0x8000) {
    vx = vy;
  }

  // 8xy1  Set Vx to (Vx OR Vy)
  else if ((instruction & 0xF00F) == 0x8001) {
    vx |= vy;
  }

  // 8xy2  Set Vx to (Vx AND Vy)
  else if ((instruction & 0xF00F) == 0x8002) {
    vx &= vy;
  }

  // 8xy3  Set Vx to (Vx XOR Vy)
  else if ((instruction & 0xF00F) == 0x8003) {
    vx ^= vy;
  }

  // 8xy4  Set Vx to (Vx + Vy) with overflow on VF
  else if ((instruction & 0xF00F) == 0x8004) {
    const auto overflowFlag = (vx + vy > 0xFF) ? 1 : 0;
    vx += vy;
    vf = overflowFlag;
  }

  // 8xy5  Set Vx to (Vx - Vy) with carry on VF
  else if ((instruction & 0xF00F) == 0x8005) {
    const auto carryFlag = vx >= vy ? 1 : 0;
    vx -= vy;
    vf = carryFlag;
  }

  // 8xy7  Set Vx to (Vy - Vx) with carry on VF
  else if ((instruction & 0xF00F) == 0x8007) {
    const auto carryFlag = vy >= vx ? 1 : 0;
    vx = vy - vx;
    vf = carryFlag;
  }

  // 8xy6  Set Vx to (Vy >> 1) with shifted-out bit on VF
  else if ((instruction & 0xF00F) == 0x8006) {
    const auto shiftedOut = vy & 0b00000001;
    vx = vx >> 1;
    vf = shiftedOut;
  }

  // 8xyE  Set Vx to (Vy << 1) with shifted-out bit on VF
  else if ((instruction & 0xF00F) == 0x800E) {
    const auto shiftedOut = (vy & 0b10000000) >> 7;
    vx = vx << 1;
    vf = shiftedOut;
  }

  // 6xNN  Set Vx to NN
  else if ((instruction & 0xF000) == 0x6000) {
    vx = nn;
  }

  // 7xNN  Add NN to Vx
  else if ((instruction & 0xF000) == 0x7000) {
    vx += nn;
  }

  // ANNN  Set I to NNN
  else if ((instruction & 0xF000) == 0xA000) {
    indexRegister = nnn;
  }

  // BNNN  Set PC to (NNN + VX)
  else if ((instruction & 0xF000) == 0xB000) {
    programCounter = nnn + vx;
  }

  // FxNN  Set Vx to a random number ANDed with NN
  else if ((instruction & 0xF000) == 0xC000) {
    vx = fn_random() & nn;
  }

  // DxyN  Draw
  else if ((instruction & 0xF000) == 0xD000) {
    if (highRes) {
      // Wrap the initial sprite coordinates
      const int start_x = vx % k_displayWidth;
      const int start_y = vy % k_displayHeight;

      const bool sprite16 = (n == 0);
      const int height = sprite16 ? 16 : n;
      vf = 0;  // Clear the collision flag

      for (const auto& i : IOTA(height)) {
        // Stop drawing once the sprite reaches the bottom of the screen
        if (start_y + i >= k_displayHeight) {
          // Add the number of rows not drawn to the collision flag
          vf += height - i;
          break;
        }

        // Fetch the sprite from RAM
        // Pad the last 8 bits with zeroes if drawing an 8-pixel wide sprite
        const Uint16 spriteRow = sprite16 ? (ram[indexRegister + i * 2] << 8) +
                                                ram[indexRegister + i * 2 + 1]
                                          : ram[indexRegister + i] << 8;
        // Keep track of collision for this row
        bool collision = false;

        for (const auto& j : IOTA(16)) {
          // Stop drawing once the sprite reaches the right end of the screen
          if (start_x + j >= k_displayWidth) break;

          // Extract the pixel from the sprite row
          const bool pixel = ((spriteRow >> (15 - j)) & 0b1) == 1;

          if (pixel) {
            // Check if the pixel was already on
            if (displayBuffer[start_x + j][start_y + i]) collision = true;

            // Toggle the pixel
            displayBuffer[start_x + j][start_y + i] =
                !displayBuffer[start_x + j][start_y + i];
          }
        }

        // Increment the collision flag if the row collided
        if (collision) vf += 1;
      }

    } else {
      const int start_x = vx % (k_displayWidth / 2);
      const int start_y = vy % (k_displayHeight / 2);
      vf = 0;  // Clear the collision flag

      for (const auto& i : IOTA(n)) {
        // Stop drawing once the sprite reaches the bottom of the screen
        if (2 * (start_y + i) >= k_displayHeight) break;

        const Uint8 spriteRow = ram[indexRegister + i];

        for (const auto& j : IOTA(8)) {
          // Stop drawing once the sprite reaches the right end of the screen
          if (2 * (start_x + j) >= k_displayWidth) break;

          // Extract the pixel from the sprite row
          const bool pixel = ((spriteRow >> (7 - j)) & 0b1) == 1;

          if (pixel) {
            const auto x_ = 2 * (start_x + j);
            const auto y_ = 2 * (start_y + i);

            // Check if the pixel was already on
            // If so, set the collision flag
            if (displayBuffer[x_][y_]) vf |= 1;

            // Toggle 2x2 pixels
            displayBuffer[x_ + 0][y_ + 0] = !displayBuffer[x_ + 0][y_ + 0];
            displayBuffer[x_ + 0][y_ + 1] = !displayBuffer[x_ + 0][y_ + 1];
            displayBuffer[x_ + 1][y_ + 0] = !displayBuffer[x_ + 1][y_ + 0];
            displayBuffer[x_ + 1][y_ + 1] = !displayBuffer[x_ + 1][y_ + 1];
          }
        }
      }
    }
  }

  // Ex9E  Skip if key (Vx & 0xF) is held
  else if ((instruction & 0xF0FF) == 0xE09E) {
    if ((fn_heldKeys() >> (vx & 0xF)) & 0b1) programCounter += 2;
  }

  // ExA1  Skip if key (Vx & 0xF) is not held
  else if ((instruction & 0xF0FF) == 0xE0A1) {
    if (!((fn_heldKeys() >> (vx & 0xF)) & 0b1)) programCounter += 2;
  }

  // Fx07  Set Vx to the value in the delay timer
  else if ((instruction & 0xF0FF) == 0xF007) {
    vx = delayTimer;
  }

  // Fx15  Set delay timer to the value in Vx
  else if ((instruction & 0xF0FF) == 0xF015) {
    delayTimer = vx;
  }

  // Fx18  Set sound timer to the value in Vx
  else if ((instruction & 0xF0FF) == 0xF018) {
    soundTimer = vx;
  }

  // Fx1E  Add Vx to I
  else if ((instruction & 0xF0FF) == 0xF01E) {
    indexRegister += vx;
  }

  // Fx0A  Wait for key (Vx & 0xF) to be released
  else if ((instruction & 0xF0FF) == 0xF00A) {
    const auto currentHeldKeys = fn_heldKeys();

    // If a key was released,
    // the current bitflags must be smaller than the previous bitflags
    if (currentHeldKeys < previousKeystate) {
      const auto keysDiff = previousKeystate - currentHeldKeys;
      for (const auto& i : IOTA(16))
        if (keysDiff >> i & 0b1) {
          vx = i;
          break;
        }
      previousKeystate = 0;
    } else {
      previousKeystate = currentHeldKeys;
      programCounter -= 2;
    }
  }

  // Fx29  Set I to the font character (Vx & 0xF)
  else if ((instruction & 0xF0FF) == 0xF029) {
    indexRegister = 0x50 + (vx & 0xF) * 5;
  }

  // Fx33  Store Vx as binary-coded-decimal in RAM at I
  else if ((instruction & 0xF0FF) == 0xF033) {
    ram[indexRegister + 2] = (vx / 1) % 10;
    ram[indexRegister + 1] = (vx / 10) % 10;
    ram[indexRegister + 0] = (vx / 100) % 10;
  }

  // Fx55  Store variables V0 to Vx in RAM at I
  else if ((instruction & 0xF0FF) == 0xF055) {
    for (const auto& i : IOTA(x + 1)) ram[indexRegister + i] = varRegisters[i];
  }

  // Fx65  Set variables V0 to Vx from RAM at I
  else if ((instruction & 0xF0FF) == 0xF065) {
    for (const auto& i : IOTA(x + 1)) varRegisters[i] = ram[indexRegister + i];
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
    const auto pixels = highRes ? 4 : 8;

    std::move_backward(displayBuffer.begin(), displayBuffer.end() - pixels,
                       displayBuffer.end());
    std::fill_n(displayBuffer.begin(), pixels, DisplayRow{});
  }

  // 00FC  Scroll display 4 pixels left
  else if (instruction == 0x00FC) {
    const auto pixels = highRes ? 4 : 8;

    std::move(displayBuffer.begin() + pixels, displayBuffer.end(),
              displayBuffer.begin());
    std::fill_n(displayBuffer.end() - pixels, pixels, DisplayRow{});
  }

  // 00Cn  Scroll n pixels down
  else if ((instruction & 0xFFF0) == 0x00C0) {
    const auto pixels = highRes ? n : n * 2;

    for (auto& column : displayBuffer) {
      std::move_backward(column.begin(), column.end() - pixels, column.end());
      std::fill_n(column.begin(), pixels, false);
    }
  }

  // Fx30  Set I to the big font character (Vx & 0xF)
  else if ((instruction & 0xF0FF) == 0xF030) {
    indexRegister = 0x0A0 + (vx & 0xF) * 10;
  }

  else {
    // TODO: Illegal instruction
  }
}
