#pragma once
#include <stdint.h>

#include "cartridge.h"
#include "io.h"

class IO;

class Memory {
 private:
  Cartridge cart_;
  IO* io_;

  uint8_t* ram_;
  uint8_t* highRam_;

 public:
  Memory(Cartridge&& cart, IO* io);
  Memory(Memory&& oth);
  ~Memory();
  uint8_t read(uint16_t addr);
  uint16_t read16(uint16_t addr);
  uint8_t write(uint16_t addr, uint8_t datum);
  uint16_t write16(uint16_t addr, uint16_t datum);
  IO& io() { return *io_; }
};