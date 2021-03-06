#pragma once
#include "cpu.h"
#include "memory.h"

#include <stdint.h>

#include <string>

extern "C" {
  extern void printAsciiBuffer(const char* buf);
}

const struct JSStreamEndl {} endl;

struct JSStream {
  std::string s;
  CPU* cpu_;
  Memory* mem_;

  JSStream(): s() {}

  void set(CPU* cpu) {
    cpu_ = cpu;
  }

  void set(Memory* mem) {
    mem_ = mem;
  }

  JSStream& pc() {
    return (*this << "PC(" << cpu_->reg_.pc << ") ");
  }

  JSStream& mem(uint16_t addr) {
    return (*this << "MEM(" << addr << " " << mem_->read(addr) << ") ");
  }

  JSStream& operator<<(const std::string& x) {
    s += x;
    return *this;
  };

  JSStream& operator<<(const uint8_t& x) {
    return *this << (uint16_t)x;
  }

  JSStream& operator<<(const uint16_t& x) {
    s+= "0x";
    for (int i = 3; i >= 0; --i) {
      int t = ((x >> (i * 4)) & 0xF);
      if (t < 10) {
        s += '0' + t;
      } else {
        s += 'A' + t - 10;
      }
    }
    return *this;
  };

  JSStream& operator<<(const int32_t& x) {
    s += std::to_string(x);
    return *this;
  };

  JSStream& operator<<(const JSStreamEndl&){
    printAsciiBuffer(s.c_str());
    s = "";
    return *this;
  }
};

extern JSStream ERR;
