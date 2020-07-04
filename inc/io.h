#pragma once
#include "memory.h"

#include <stdint.h>

#define N_BIT(x, n) (((x) >> (n)) & 1)

class Memory;

class IO {
 private:
  uint8_t reg_[0x100];
  Memory* mem_;

  uint8_t doDMA_(uint8_t arg);

 public:
  uint8_t oam[0xA0];
  uint8_t vram[0x2000];

  enum REG {
    P1 = 0x00,
    SB = 0x01,
    SC,
    DIV = 0x04,
    TIMA = 0x05,
    TMA,
    TAC,
    IF = 0x0F,
    NR10 = 0x10,
    NR11,
    NR12,
    NR13,
    NR14,
    NR21 = 0x16,
    NR22,
    NR23,
    NR24,
    NR30 = 0x1A,
    NR31,
    NR32,
    NR33,
    NR34,
    NR41 = 0x20,
    NR42,
    NR43,
    NR44,
    NR50,
    NR51,
    NR52,
    LCDC = 0x40,
    STAT,
    SCY,
    SCX,
    LY,
    LYC,
    DMA,
    BGP,
    OBP0,
    OBP1,
    WY,
    WX,
    IME = 0xFE,
    IE = 0xFF
  };
  enum INTERRUPT {
    VBLANK = 0,
    LCDC_STATUS,
    TIMER_OVERFLOW,
    SERIAL_TRANSFER,
    HIGH_TO_LOW_P10_P13,
  };

  IO(Memory* mem);
  ~IO();
  uint8_t read(uint16_t addr);
  uint8_t write(uint16_t addr, uint8_t data);
  uint8_t& reg(REG name) {
    return reg_[name];
  }
  void disableInterrupt();
  void enableInterrupt();
  void requestInterrupt(INTERRUPT inter);
  uint16_t acknowledgeInterrupt();
};