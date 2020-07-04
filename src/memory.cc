#include "memory.h"
#include "log.h"

Memory::Memory(Cartridge&& cart, IO* io) : cart_(std::move(cart)), io_(io) {
  ram_ = new uint8_t[0x2000];
  highRam_ = new uint8_t[0x7F];
}

Memory::~Memory() {
  delete ram_;
  delete highRam_;
}

uint8_t Memory::read(uint16_t addr) {
  if (addr < 0x8000)
    return cart_.rom(addr);
  if (addr < 0xA000)
    return io_->vram[addr - 0x8000];
  if (addr < 0xC000)
    return cart_.ram(addr - 0xA000);
  if (addr < 0xE000)
    return ram_[addr - 0xC000];
  if (addr < 0xFE00)
    return ram_[addr - 0xE000];
  if (addr < 0xFEA0)
    return io_->oam[addr - 0xFE00];
  if (addr < 0xFF00) {
    ERR << "Read UIO Addr " << addr << endl;
    return 0;
  }
  if (addr < 0xFF4C)
    return io_->read(addr);
  if (addr < 0xFF80) {
    ERR << "Read UIO Addr " << addr << endl;
    return 0;
  }
  if (addr < 0xFFFF) {
    // if (addr == 0xFF80) return 0;
    return highRam_[addr - 0xFF80];
  }
  if (addr == 0xFFFF)
    return io_->read(0xFFFF);
  ERR << "Read address out of bound: " << addr << endl;
  return 0;
}

uint16_t Memory::read16(uint16_t addr) {
  return read(addr) | static_cast<uint16_t>(read(addr + 1)) << 8;
}

uint8_t Memory::write(uint16_t addr, uint8_t datum) {
  if (addr < 0x8000)
    return cart_.write(addr, datum);
  if (addr < 0xA000)
    return io_->vram[addr - 0x8000] = datum;
  if (addr < 0xC000)
    return cart_.ram(addr - 0xA000) = datum;
  if (addr < 0xE000)
    return ram_[addr - 0xC000] = datum;
  if (addr < 0xFE00)
    return ram_[addr - 0xE000] = datum;
  if (addr < 0xFEA0)
    return io_->oam[addr - 0xFE00] = datum;
  if (addr < 0xFF00) {
    // ERR << "Wirte UIO Addr " << addr << " " << datum << endl;
    return 0;
  }
  if (addr < 0xFF4C)
    return io_->write(addr, datum);
  if (addr < 0xFF80) {
    // ERR << "Write UIO Addr " << addr << " " << datum << endl;
    return 0;
  }
  if (addr < 0xFFFF) {
    return highRam_[addr - 0xFF80] = datum;
  }
  if (addr == 0xFFFF)
    return io_->write(0xFFFF, datum);
  ERR << "Write address out of bound: " << addr << endl;
  return 0;
}

uint16_t Memory::write16(uint16_t addr, uint16_t datum) {
  write(addr, datum & 0xFF);
  write(addr + 1, (datum >> 8) & 0xFF);
  return datum;
}