#include "io.h"

#include "log.h"

#include <cstring>

IO::IO(Memory* mem, Video* video): mem_(mem), video_(video) {
  memset(reg_, 0, sizeof(reg_));
  enableInterrupt();
  reg_[P1] = 0xF;
  reg_[LCDC] = 0x91;
  reg_[BGP] = 0xFC;
  reg_[OBP0] = 0xFF;
  reg_[OBP1] = 0xFF;
}

IO::~IO() {}

void IO::disableInterrupt() {
  // ERR << "IO DL" << endl;
  reg_[IME] = 0;
}
void IO::enableInterrupt() {
  // ERR << "IO EL" << endl;
  reg_[IME] = 0xFF;
}
void IO::requestInterrupt(INTERRUPT inter) {
  // ERR << "IO R inter: " << inter << endl;
  reg_[IF] |= (1 << inter);
}

uint8_t IO::read(uint16_t addr) {
  uint8_t reg = addr & 0xFF;
  switch (reg) {
    case IF:
      return reg_[IME] & reg_[IF];
    case P1:
    case SB:
    case SC:
    // case DIV:
    // case TIMA:
    // case TMA:
    // case TAC:
    case NR10:
    case NR11:
    case NR12:
    case NR13:
    case NR14:
    case NR21:
    case NR22:
    case NR23:
    case NR24:
    case NR30:
    case NR31:
    case NR32:
    case NR33:
    case NR34:
    case NR41:
    case NR42:
    case NR43:
    case NR44:
    case NR50:
    case NR51:
    case NR52:
    case LCDC:
    case STAT:
    case SCY:
    case SCX:
    case LY:
    case LYC:
    case BGP:
    case OBP0:
    case OBP1:
    case WY:
    case WX:
    case IE:
      return reg_[reg];

    default:
      // Wave Pattern RAM
      if (reg >= 0x30 && reg <= 0x3F)
        return reg_[reg];
      ERR << "Read undefined IO register: " << addr << endl;
      throw 1;
  }
}

uint8_t IO::write(uint16_t addr, uint8_t datum) {
  uint8_t reg = addr & 0xFF;
  // ERR << "IO Write " << reg << " " << datum << endl;
  switch (reg) {
    case DMA:
      return doDMA_(datum);
    case STAT:
      ERR << "STAT write " << datum << endl;
      return reg_[reg] = (reg_[reg] & 0x7) | (datum & 0xF8);
    case LCDC:
      ERR.pc() << "LCDC write " << datum << " STAT " << reg_[STAT] << endl;
      if ((datum & 0x80) != 0x80) {
        reg_[LY] = 0;
        reg_[STAT] = reg_[STAT] & 0xF8;
        video_->resetTimer();
      }
      return reg_[reg] = datum;
    case P1:
      return 0;
      return reg_[reg] = (reg_[reg] & 0xF) | (datum & 0xF0);

    case SB:
    case SC:
    case DIV:
    case TIMA:
    case TMA:
    case TAC:
    case IF:
    case NR10:
    case NR11:
    case NR12:
    case NR13:
    case NR14:
    case NR21:
    case NR22:
    case NR23:
    case NR24:
    case NR30:
    case NR31:
    case NR32:
    case NR33:
    case NR34:
    case NR41:
    case NR42:
    case NR43:
    case NR44:
    case NR50:
    case NR51:
    case NR52:
    // case LCDC:
    case SCY:
    case SCX:
    // case LY:
    // case LYC:
    case BGP:
    case OBP0:
    case OBP1:
    case WY:
    case WX:
    case IE:
      return reg_[reg] = datum;

    default:
      // Wave Pattern RAM
      if (reg >= 0x30 && reg <= 0x3F)
        return reg_[reg] = datum;

      ERR << "Write undefined IO register: " << addr << " " << (uint16_t)datum
          << endl;
      throw 1;
  }
}

uint16_t IO::acknowledgeInterrupt() {
  uint8_t r = reg_[IME] & reg_[IF] & reg_[IE];
  for (int i = 0; i < 5; ++i) {
    if ((1 << i) & r) {
      reg_[IF] &= (uint8_t)(~(1u << i));
      return 0x40 + 8 * i;
    }
  }
  return 0xFFFF;
}

uint8_t IO::doDMA_(uint8_t arg) {
  uint16_t base = (uint16_t)(arg) << 8;
  for (uint16_t i = 0; i < 0xA0; ++i) {
    oam[i] = mem_->read(base | i);
  }
  return 0;
}
