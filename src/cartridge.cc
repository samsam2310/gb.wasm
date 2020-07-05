#include "cartridge.h"

#include "log.h"

Cartridge::Cartridge(uint8_t* data) {
  data_ = data;
  ram_ = nullptr;
}

Cartridge::Cartridge(Cartridge&& oth) {
  data_ = oth.data_;
  oth.data_ = nullptr;
  ram_ = oth.ram_;
  oth.ram_ = nullptr;
  ramBank_ = oth.ramBank_;
  oth.ramBank_ = nullptr;
}

Cartridge::~Cartridge() {
  delete data_;
  delete ram_;
}

uint8_t Cartridge::rom(uint16_t addr) {
  return data_[addr];
}

uint8_t& Cartridge::ram(uint16_t addr) {
  ERR << "Cart RAM! " << addr << endl;
  throw 1;
  return ramBank_[addr];
}

uint8_t Cartridge::write(uint16_t addr, uint8_t datum) {
  ERR << "Cart Write " << addr << " " << datum << endl;
  return 0;
}