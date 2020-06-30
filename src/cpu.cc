#include "cpu.h"
#include "log.h"

#include <utility>

CPU::CPU(Memory&& mem) : mem_(std::move(mem)) {
  reg_.af = 0x01;
  reg_.f = 0xB0;
  reg_.bc = 0x0013;
  reg_.de = 0x00D8;
  reg_.hl = 0x014D;
  reg_.sp = 0xFFFE;
  reg_.pc = 0x100;
}

#define FLAG(Z, N, H, C) ((Z) << 7 | (N) << 6 | (H) << 5 | (C) << 4)

void add(uint8_t* x, uint8_t* y, uint8_t* f) {
  uint8_t r8 = (*x & 0xF) + (*y & 0xF);
  uint16_t r16 = (uint16_t)*x + (uint16_t)*y;
  *x += *y;
  *f = FLAG(*x == 0, 0, r8 >> 4 != 0, r16 >> 8 != 0);
}

void doXor(uint8_t* x, uint8_t* y, uint8_t* f) {
  *x ^= *y;
  *f = FLAG(*x == 0, 0, 0, 0);
}

void cp(uint8_t x, uint8_t y, uint8_t* f) {
  *f = FLAG(x == y, 1, (x & 0xF) < (y & 0xF), x < y);
}

void dec(uint8_t* x, uint8_t* f) {
  --(*x);
  *f = FLAG(*x == 0, 1, (*x & 0xF) != 0xF, (*f >> 4) & 1);
}

void rrca(uint8_t* x, uint8_t* f) {
  *x = *x << 7 | *x >> 1;
  *f = FLAG(*x == 0, 1, 0, *x >> 7);
}

uint16_t signExtend16(uint8_t x) {
  return (uint16_t)(int16_t)(int8_t)x;
}

#define READ_NN              \
  nn = mem_.read16(reg_.pc); \
  reg_.pc += 2
#define READ_N n = mem_.read(reg_.pc++)

int CPU::execSingleInst_() {
  uint8_t op = mem_.read(reg_.pc++);
  uint16_t nn;
  uint8_t n;
  ERR << "OP: " << (uint16_t)op << "\n";
  switch (op) {
    case 0xC3:  // JP
      READ_NN;
      reg_.pc = nn;
      return 12;
      // ---- 1.1 ----
    case 0x06:
      READ_N;
      reg_.b = n;
      return 8;
    case 0x0E:
      READ_N;
      reg_.c = n;
      return 8;
    case 0x16:
      READ_N;
      reg_.d = n;
      return 8;
    case 0x1E:
      READ_N;
      reg_.e = n;
      return 8;
    case 0x26:
      READ_N;
      reg_.h = n;
      return 8;
    case 0x2E:
      READ_N;
      reg_.l = n;
      return 8;
      // ---- 1.2 ----
    case 0x7F:
      reg_.a = reg_.a;
      return 4;
    case 0x78:
      reg_.a = reg_.b;
      return 4;
    case 0x79:
      reg_.a = reg_.c;
      return 4;
    case 0x7A:
      reg_.a = reg_.d;
      return 4;
    case 0x7B:
      reg_.a = reg_.e;
      return 4;
    case 0x7C:
      reg_.a = reg_.h;
      return 4;
    case 0x7D:
      reg_.a = reg_.l;
      return 4;
    case 0x7E:
      reg_.a = mem_.read(reg_.hl);
      return 8;
    case 0x40:
      reg_.b = reg_.b;
      return 4;
    case 0x41:
      reg_.b = reg_.c;
      return 4;
    case 0x42:
      reg_.b = reg_.d;
      return 4;
    case 0x43:
      reg_.b = reg_.e;
      return 4;
    case 0x44:
      reg_.b = reg_.h;
      return 4;
    case 0x45:
      reg_.b = reg_.l;
      return 4;
    case 0x46:
      reg_.b = mem_.read(reg_.hl);
      return 8;
    case 0x48:
      reg_.c = reg_.b;
      return 4;
    case 0x49:
      reg_.c = reg_.c;
      return 4;
    case 0x4A:
      reg_.c = reg_.d;
      return 4;
    case 0x4B:
      reg_.c = reg_.e;
      return 4;
    case 0x4C:
      reg_.c = reg_.h;
      return 4;
    case 0x4D:
      reg_.c = reg_.l;
      return 4;
    case 0x4E:
      reg_.c = mem_.read(reg_.hl);
      return 8;
    case 0x50:
      reg_.d = reg_.b;
      return 4;
    case 0x51:
      reg_.d = reg_.c;
      return 4;
    case 0x52:
      reg_.d = reg_.d;
      return 4;
    case 0x53:
      reg_.d = reg_.e;
      return 4;
    case 0x54:
      reg_.d = reg_.h;
      return 4;
    case 0x55:
      reg_.d = reg_.l;
      return 4;
    case 0x56:
      reg_.d = mem_.read(reg_.hl);
      return 8;
    case 0x58:
      reg_.e = reg_.b;
      return 4;
    case 0x59:
      reg_.e = reg_.c;
      return 4;
    case 0x5A:
      reg_.e = reg_.d;
      return 4;
    case 0x5B:
      reg_.e = reg_.e;
      return 4;
    case 0x5C:
      reg_.e = reg_.h;
      return 4;
    case 0x5D:
      reg_.e = reg_.l;
      return 4;
    case 0x5E:
      reg_.e = mem_.read(reg_.hl);
      return 8;
    case 0x60:
      reg_.h = reg_.b;
      return 4;
    case 0x61:
      reg_.h = reg_.c;
      return 4;
    case 0x62:
      reg_.h = reg_.d;
      return 4;
    case 0x63:
      reg_.h = reg_.e;
      return 4;
    case 0x64:
      reg_.h = reg_.h;
      return 4;
    case 0x65:
      reg_.h = reg_.l;
      return 4;
    case 0x66:
      reg_.h = mem_.read(reg_.hl);
      return 8;
    case 0x68:
      reg_.l = reg_.b;
      return 4;
    case 0x69:
      reg_.l = reg_.c;
      return 4;
    case 0x6A:
      reg_.l = reg_.d;
      return 4;
    case 0x6B:
      reg_.l = reg_.e;
      return 4;
    case 0x6C:
      reg_.l = reg_.h;
      return 4;
    case 0x6D:
      reg_.l = reg_.l;
      return 4;
    case 0x6E:
      reg_.l = mem_.read(reg_.hl);
      return 8;
    case 0x70:
      mem_.write(reg_.hl, reg_.b);
      return 8;
    case 0x71:
      mem_.write(reg_.hl, reg_.c);
      return 8;
    case 0x72:
      mem_.write(reg_.hl, reg_.d);
      return 8;
    case 0x73:
      mem_.write(reg_.hl, reg_.e);
      return 8;
    case 0x74:
      mem_.write(reg_.hl, reg_.h);
      return 8;
    case 0x75:
      mem_.write(reg_.hl, reg_.l);
      return 8;
    case 0x36:
      READ_N;
      mem_.write(reg_.hl, n);
      return 12;
      // ---- 1.3 ----
    case 0x0A:
      reg_.a = mem_.read(reg_.bc);
      return 8;
    case 0x1A:
      reg_.a = mem_.read(reg_.de);
      return 8;
    case 0xFA:
      READ_NN;
      reg_.a = mem_.read(nn);
      return 16;
    case 0x3E:
      READ_N;
      reg_.a = n;
      return 8;
      // ---- 1.4 ----
    case 0x47:
      reg_.b = reg_.a;
      return 4;
    case 0x4F:
      reg_.c = reg_.a;
      return 4;
    case 0x57:
      reg_.d = reg_.a;
      return 4;
    case 0x5F:
      reg_.e = reg_.a;
      return 4;
    case 0x67:
      reg_.h = reg_.a;
      return 4;
    case 0x6F:
      reg_.l = reg_.a;
      return 4;
    case 0x02:
      mem_.write(reg_.bc, reg_.a);
      return 8;
    case 0x12:
      mem_.write(reg_.de, reg_.a);
      return 8;
    case 0x77:
      mem_.write(reg_.hl, reg_.a);
      return 8;
    case 0xEA:
      READ_NN;
      mem_.write(nn, reg_.a);
      return 16;
      // ---- 1.12 ----
    case 0x32:
      mem_.write(reg_.hl--, reg_.a);
      return 8;
      // ---- 1.19 ----
    case 0xE0:
      READ_N;
      mem_.write(0xFF00 + n, reg_.a);
      return 12;
      // ---- 1.20 ----
    case 0xF0:
      READ_N;
      reg_.a = mem_.read(0xFF00 + n);
      return 12;
      // ---- 2.1 ----
    case 0x01:
      READ_NN;
      reg_.bc = nn;
      return 12;
    case 0x11:
      READ_NN;
      reg_.de = nn;
      return 12;
    case 0x21:
      READ_NN;
      reg_.hl = nn;
      return 12;
    case 0x31:
      READ_NN;
      reg_.sp = nn;
      return 12;
      // ---- 3.1 ----
    case 0x87:
      add(&reg_.a, &reg_.a, &reg_.f);
      return 4;
    case 0x80:
      add(&reg_.a, &reg_.b, &reg_.f);
      return 4;
    case 0x81:
      add(&reg_.a, &reg_.c, &reg_.f);
      return 4;
    case 0x82:
      add(&reg_.a, &reg_.d, &reg_.f);
      return 4;
    case 0x83:
      add(&reg_.a, &reg_.e, &reg_.f);
      return 4;
    case 0x84:
      add(&reg_.a, &reg_.h, &reg_.f);
      return 4;
    case 0x85:
      add(&reg_.a, &reg_.l, &reg_.f);
      return 4;
    case 0x86:
      n = mem_.read(reg_.hl);
      add(&reg_.a, &n, &reg_.f);
      return 8;
    case 0xC6:
      READ_N;
      add(&reg_.a, &n, &reg_.f);
      return 8;
      // ---- 3.7 ----
    case 0xAF:
      doXor(&reg_.a, &reg_.a, &reg_.f);
      return 4;
    case 0xA8:
      doXor(&reg_.a, &reg_.b, &reg_.f);
      return 4;
    case 0xA9:
      doXor(&reg_.a, &reg_.c, &reg_.f);
      return 4;
    case 0xAA:
      doXor(&reg_.a, &reg_.d, &reg_.f);
      return 4;
    case 0xAB:
      doXor(&reg_.a, &reg_.e, &reg_.f);
      return 4;
    case 0xAC:
      doXor(&reg_.a, &reg_.h, &reg_.f);
      return 4;
    case 0xAD:
      doXor(&reg_.a, &reg_.l, &reg_.f);
      return 4;
    case 0xAE:
      n = mem_.read(reg_.hl);
      doXor(&reg_.a, &n, &reg_.f);
      return 8;
    case 0xEE:
      READ_N;
      doXor(&reg_.a, &n, &reg_.f);
      return 8;
      // ---- 3.8 ----
    case 0xBF:
      cp(reg_.a, reg_.a, &reg_.f);
      return 4;
    case 0xB8:
      cp(reg_.a, reg_.b, &reg_.f);
      return 4;
    case 0xB9:
      cp(reg_.a, reg_.c, &reg_.f);
      return 4;
    case 0xBA:
      cp(reg_.a, reg_.d, &reg_.f);
      return 4;
    case 0xBB:
      cp(reg_.a, reg_.e, &reg_.f);
      return 4;
    case 0xBC:
      cp(reg_.a, reg_.h, &reg_.f);
      return 4;
    case 0xBD:
      cp(reg_.a, reg_.l, &reg_.f);
      return 4;
    case 0xBE:
      cp(reg_.a, mem_.read(reg_.hl), &reg_.f);
      return 8;
    case 0xFE:
      READ_N;
      cp(reg_.a, n, &reg_.f);
      return 8;
      // ---- 3.10 ----
    case 0x3D:
      dec(&reg_.a, &reg_.f);
      return 4;
    case 0x05:
      dec(&reg_.b, &reg_.f);
      return 4;
    case 0x0D:
      dec(&reg_.c, &reg_.f);
      return 4;
    case 0x15:
      dec(&reg_.d, &reg_.f);
      return 4;
    case 0x1D:
      dec(&reg_.e, &reg_.f);
      return 4;
    case 0x25:
      dec(&reg_.h, &reg_.f);
      return 4;
    case 0x2D:
      dec(&reg_.l, &reg_.f);
      return 4;
    case 0x35:
      n = mem_.read(reg_.hl);
      dec(&n, &reg_.f);
      mem_.write(reg_.hl, n);
      return 12;
      // ---- 5.6 ----
    case 0x00:  // NOP
      return 4;
      // ---- 5.8 ----
    case 0x10:
      READ_N;
      if (n != 0) {
        ERR << "PC: " << reg_.pc << "\n";
        throw 1;
      }
      // stop_();
      return 4;
      // ---- 5.9 ----
    case 0xF3:
      mem_.io().disableInterrupt();
      return 4;
      // ---- 5.10 ----
    case 0xFB:
      mem_.io().enableInterrupt();
      return 4;
      // ---- 6.3 ----
    case 0x0F:
      rrca(&reg_.a, &reg_.f);
      return 4;
      // ---- 8.5 ----
    case 0x20:
      READ_N;
      reg_.pc += (uint16_t)((~reg_.f >> 7) & 1) * signExtend16(n);
      return 8;
    case 0x28:
      READ_N;
      reg_.pc += (uint16_t)((reg_.f >> 7) & 1) * signExtend16(n);
      return 8;
    case 0x30:
      READ_N;
      reg_.pc += (uint16_t)((~reg_.f >> 4) & 1) * signExtend16(n);
      return 8;
    case 0x38:
      READ_N;
      reg_.pc += (uint16_t)((reg_.f >> 4) & 1) * signExtend16(n);
      return 8;

    default:
      ERR << "Uknown opcode: " << (uint16_t)op << "\n";
      throw 1;
  }
}

int CPU::execute(int cycle) {
  while (cycle > 0) {
    uint16_t interruptAddr = mem_.io().acknowledgeInterrupt();
    if (interruptAddr != 0xFFFF) {
      mem_.io().disableInterrupt();
      reg_.sp -= 2;
      mem_.write16(reg_.sp, reg_.pc);
      reg_.pc = interruptAddr;
    }
    cycle -= execSingleInst_();
  }
  return cycle;
}
