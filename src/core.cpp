#include "core.hpp"

#include "default_font.hpp"

MachineState::MachineState(const std::array<Uint8, 0x50> &font,
                           std::istream &romFile) {
  std::ranges::copy(font, &ram[0x50]);

  romFile.read((char *)&ram.at(0x200), CORE_RAM_SIZE - 0x200);

  programCounter = 0x200;
}

void MachineState::tickTimer() {
  if (delayTimer > 0) delayTimer--;
  if (soundTimer > 0) soundTimer--;
}

void MachineState::tick(std::function<Uint16()> heldKeys,
                        std::function<Uint8()> random) {
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
  switch ((instruction & 0xF000) >> 12) {
    case 0x0: {
      switch (Y) {
        case 0xE: {
          switch (N) {
            case 0x0:
              displayBuffer = {};
              break;

            case 0xE:
              programCounter = stack.top();
              stack.pop();
              break;
          }
          break;
        }
      }
      break;
    }

    case 0x1:
      programCounter = NNN;
      break;

    case 0x2:
      stack.push(programCounter);
      programCounter = NNN;
      break;

    case 0x3:
      if (VX == NN) programCounter += 2;
      break;

    case 0x4:
      if (VX != NN) programCounter += 2;
      break;

    case 0x5:
      if (VX == VY) programCounter += 2;
      break;

    case 0x9:
      if (VX != VY) programCounter += 2;
      break;

    case 0x8: {
      switch (N) {
        case 0x0:
          VX = VY;
          break;

        case 0x1:
          VX |= VY;
          VF = 0;
          break;

        case 0x2:
          VX &= VY;
          VF = 0;
          break;

        case 0x3:
          VX ^= VY;
          VF = 0;
          break;

        case 0x4: {
          uint8_t overflowFlag = (VX + VY > 0xFF) ? 1 : 0;
          VX = VX + VY;
          VF = overflowFlag;
          break;
        }

        case 0x5: {
          uint8_t carryFlag = VX >= VY ? 1 : 0;
          VX = VX - VY;
          VF = carryFlag;
          break;
        }

        case 0x7: {
          uint8_t carryFlag = VY >= VX ? 1 : 0;
          VX = VY - VX;
          VF = carryFlag;
          break;
        }

        case 0x6: {
          bool shiftedOut = VY & 0b00000001;
          VX = VY >> 1;
          VF = shiftedOut;
          break;
        }

        case 0xE: {
          bool shiftedOut = (VY & 0b10000000) >> 7;
          VX = VY << 1;
          VF = shiftedOut;
          break;
        }
      }
      break;
    }

    case 0x6:
      VX = NN;
      break;

    case 0x7:
      VX += NN;
      break;

    case 0xA:
      indexRegister = NNN;
      break;

    case 0xB:
      programCounter = NNN + varRegisters[0];
      break;

    case 0xC:
      VX = random() & NN;
      break;

    case 0xD: {
      int start_x = VX % 64;
      int y = VY % 32;
      VF = 0;

      for (int i = 0; i < N; i++) {
        int x = start_x;
        uint8_t nthSpriteRow = ram[(indexRegister + i)];
        for (int j = 7; j >= 0; j--) {
          bool bit = (nthSpriteRow & (1 << j)) >> j;
          if (bit) {
            VF |= displayBuffer[x][y];
            displayBuffer[x][y] = !displayBuffer[x][y];
          }
          x++;
          if (x > 63) break;
        }
        y++;
        if (y > 31) break;
      }
      break;
    }

    case 0xE: {
      switch (NN) {
        case 0x9E:
          if ((heldKeys() >> (VX & 0xF)) & 0b1) programCounter += 2;
          break;

        case 0xA1:
          if (!((heldKeys() >> (VX & 0xF)) & 0b1)) programCounter += 2;
          break;
      }
      break;
    }

    case 0xF: {
      switch (NN) {
        case 0x07:
          VX = delayTimer;
          break;

        case 0x15:
          delayTimer = VX;
          break;

        case 0x18:
          soundTimer = VX;
          break;

        case 0x1E:
          indexRegister += VX;
          break;

        case 0x0A: {
          uint16_t currentHeldKeys = heldKeys();

          if (currentHeldKeys < previousKeystate) {
            uint16_t keysDiff = previousKeystate - currentHeldKeys;
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
          break;
        }

        case 0x29:
          indexRegister = 0x50 + (VX & 0xF) * 5;
          break;

        case 0x33:
          ram[(indexRegister + 2)] = (VX / 1) % 10;
          ram[(indexRegister + 1)] = (VX / 10) % 10;
          ram[(indexRegister + 0)] = (VX / 100) % 10;
          break;

        case 0x55:
          for (int i = 0; i <= X; i++) ram[indexRegister++] = varRegisters[i];
          break;

        case 0x65:
          for (int i = 0; i <= X; i++) varRegisters[i] = ram[indexRegister++];
          break;
      }
      break;
    }
  }
}
