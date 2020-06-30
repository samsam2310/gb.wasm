#include "io.h"

#include "log.h"

IO::IO() {
  reg_ = new uint8_t[0x100];
  enableInterrupt();
}

IO::IO(IO&& oth) {
  reg_ = oth.reg_;
  oth.reg_ = nullptr;
}

IO::~IO() {
  delete reg_;
}

uint8_t IO::read(uint16_t addr) {
  uint8_t reg = addr & 0xFF;
  switch (reg) {
    case IF:
      return reg_[IME] & reg_[IF];
    case P1:
    case SB:
    case SC:
    case DIV:
    case TIMA:
    case TMA:
    case TAC:
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
    case DMA:
    case BGP:
    case OBP0:
    case OBP1:
    case WY:
    case WX:
    case IE:
      return reg_[reg];

    default:
      ERR << "Read undefined IO register: " << addr << "\n";
      throw 1;
  }
}

uint8_t IO::write(uint16_t addr, uint8_t datum) {
  uint8_t reg = addr & 0xFF;
  switch (reg) {
    case P1:
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
    case LCDC:
    case STAT:
    case SCY:
    case SCX:
    case LY:
    case LYC:
    case DMA:
    case BGP:
    case OBP0:
    case OBP1:
    case WY:
    case WX:
    case IE:
      return reg_[reg] = datum;

    default:
      ERR << "Write undefined IO register: " << addr << ' ' << (uint16_t)datum
          << "\n";
      throw 1;
  }
}

void IO::disableInterrupt() {
  reg_[IME] = 0;
}

void IO::enableInterrupt() {
  reg_[IME] = 0xFF;
}

uint16_t IO::acknowledgeInterrupt() {
  uint8_t r = reg_[IME] & reg_[IF] & reg_[IE];
  for (int i = 0; i < 5; ++i) {
    if ((1 << i) & r) {
      reg_[IF] &= (~(1 << i));
      return 0x40 + 8 * i;
    }
  }
  return 0xFFFF;
}
