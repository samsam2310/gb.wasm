#pragma once
#include <stdint.h>

class Cartridge {
 private:
  uint8_t* data_;
  uint8_t* ram_;
  uint8_t* ramBank_;

 public:
  Cartridge(uint8_t* data);
  Cartridge(Cartridge&& oth);
  ~Cartridge();
  uint8_t& ram(uint16_t addr);
  uint8_t rom(uint16_t addr);
  uint8_t write(uint16_t addr, uint8_t datum);
};