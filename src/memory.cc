#include "memory.h"
#include "log.h"

#include <utility>

Memory::Memory(IO&& io, Cartridge&& cart)
    : io_(std::move(io)), cart_(std::move(cart)) {
  oam_ = new uint8_t[0xA0];
  vram_ = new uint8_t[0x2000];
  ram_ = new uint8_t[0x2000];
  ram2_ = new uint8_t[0x7F];
}

Memory::Memory(Memory&& oth)
    : io_(std::move(oth.io_)), cart_(std::move(oth.cart_)) {
  oam_ = oth.oam_;
  oth.oam_ = nullptr;
  vram_ = oth.vram_;
  oth.vram_ = nullptr;
  ram_ = oth.ram_;
  oth.ram_ = nullptr;
  ram2_ = oth.ram2_;
  oth.ram2_ = nullptr;
}

Memory::~Memory() {
  delete oam_;
  delete vram_;
  delete ram_;
  delete ram2_;
}

uint8_t Memory::read(uint16_t addr) {
  if (addr < 0x8000)
    return cart_.rom(addr);
  if (addr < 0xA000)
    return vram_[addr - 0x8000];
  if (addr < 0xC000)
    return cart_.ram(addr - 0xA000);
  if (addr < 0xE000)
    return ram_[addr - 0xC000];
  if (addr < 0xFE00)
    return ram_[addr - 0xE000];
  if (addr < 0xFEA0)
    return oam_[addr - 0xFE00];
  if (addr < 0xFF80)
    return io_.read(addr);
  if (addr < 0xFFFF)
    return ram2_[addr - 0xFF80];
  if (addr == 0xFFFF)
    return io_.read(0xFFFF);
  ERR << "Read address out of bound: " << addr << "\n";
  return 0;
}

uint16_t Memory::read16(uint16_t addr) {
  return read(addr) | static_cast<uint16_t>(read(addr + 1)) << 8;
}

uint8_t Memory::write(uint16_t addr, uint8_t datum) {
  if (addr < 0x8000)
    return cart_.write(addr, datum);
  if (addr < 0xA000)
    return vram_[addr - 0x8000] = datum;
  if (addr < 0xC000)
    return cart_.ram(addr - 0xA000) = datum;
  if (addr < 0xE000)
    return ram_[addr - 0xC000] = datum;
  if (addr < 0xFE00)
    return ram_[addr - 0xE000] = datum;
  if (addr < 0xFEA0)
    return oam_[addr - 0xFE00] = datum;
  if (addr < 0xFF80)
    return io_.write(addr, datum);
  if (addr < 0xFFFF)
    return ram2_[addr - 0xFF80] = datum;
  if (addr == 0xFFFF)
    return io_.write(0xFFFF, datum);
  ERR << "Write address out of bound: " << addr << "\n";
  return 0;
}

uint16_t Memory::write16(uint16_t addr, uint16_t datum) {
  write(addr, datum & 0xFF);
  write(addr + 1, (datum >> 8) & 0xFF);
  return datum;
}