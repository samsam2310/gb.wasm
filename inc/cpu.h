#pragma once
#include <stdint.h>

#include "memory.h"

class CPU {
 private:
  struct Register {
    union {
      struct {
        uint8_t f;
        uint8_t a;
      };
      uint16_t af;
    };
    union {
      struct {
        uint8_t c;
        uint8_t b;
      };
      uint16_t bc;
    };
    union {
      struct {
        uint8_t e;
        uint8_t d;
      };
      uint16_t de;
    };
    union {
      struct {
        uint8_t l;
        uint8_t h;
      };
      uint16_t hl;
    };

    uint16_t sp;
    uint16_t pc;
  } reg_;

  Memory* mem_;

  int executeCBInst_(uint8_t op);
  int executeSingleInstInner_();
  int stop_();

 public:
  CPU(Memory* mem);
  int executeSingleInst();
};