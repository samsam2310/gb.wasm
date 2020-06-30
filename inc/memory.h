#pragma once
#include <stdint.h>

#include "cartridge.h"
#include "io.h"

class Memory {
 private:
  IO io_;
  Cartridge cart_;

  uint8_t* oam_;
  uint8_t* vram_;
  uint8_t* ram_;
  uint8_t* ram2_;

 public:
  Memory(IO&& io, Cartridge&& cart);
  Memory(Memory&& oth);
  ~Memory();
  uint8_t read(uint16_t addr);
  uint16_t read16(uint16_t addr);
  uint8_t write(uint16_t addr, uint8_t datum);
  uint16_t write16(uint16_t addr, uint16_t datum);
  IO& io() { return io_; }
};