#pragma once
#include <stdint.h>

class IO {
 private:
  uint8_t* reg_;

 public:
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

  IO();
  IO(IO&& oth);
  ~IO();
  uint8_t read(uint16_t addr);
  uint8_t write(uint16_t addr, uint8_t data);
  void disableInterrupt();
  void enableInterrupt();
  uint16_t acknowledgeInterrupt();
};