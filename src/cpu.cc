#include "cpu.h"
#include "log.h"

CPU::CPU(Memory* mem) : mem_(mem) {
  reg_.af = 0x01;
  reg_.f = 0xB0;
  reg_.bc = 0x0013;
  reg_.de = 0x00D8;
  reg_.hl = 0x014D;
  reg_.sp = 0xFFFE;
  reg_.pc = 0x100;
}

#define FLAG(Z, N, H, C) ((Z) << 7 | (N) << 6 | (H) << 5 | (C) << 4)
#define FLAG_Z(f) ((f >> 7) & 1)
#define FLAG_C(f) ((f >> 4) & 1)

void add(uint8_t* x, uint8_t* y, uint8_t* f) {
  uint8_t r8 = (*x & 0xF) + (*y & 0xF);
  uint16_t r16 = (uint16_t)*x + (uint16_t)*y;
  *x += *y;
  *f = FLAG(*x == 0, 0, r8 >> 4 != 0, r16 >> 8 != 0);
}

void add(uint16_t* x, uint16_t y, uint8_t* f) {
  uint16_t r16 = (*x & 0xFFF) + (y & 0xFFF);
  uint32_t r32 = (uint32_t)*x + (uint32_t)y;
  *x += y;
  *f = FLAG(FLAG_Z(*f), 0, r16 > 0xFFF, r32 > 0xFFFF);
}

void doAnd(uint8_t* x, uint8_t* y, uint8_t* f) {
  *x &= *y;
  *f = FLAG(*x == 0, 0, 1, 0);
}

void doOr(uint8_t* x, uint8_t* y, uint8_t* f) {
  *x |= *y;
  *f = FLAG(*x == 0, 0, 0, 0);
}

void doXor(uint8_t* x, uint8_t* y, uint8_t* f) {
  *x ^= *y;
  *f = FLAG(*x == 0, 0, 0, 0);
}

void cp(uint8_t x, uint8_t y, uint8_t* f) {
  *f = FLAG(x == y, 1, (x & 0xF) < (y & 0xF), x < y);
}

void inc(uint8_t* x, uint8_t* f) {
  ++(*x);
  *f = FLAG(*x == 0, 1, (*x & 0xF) == 0, (*f >> 4) & 1);
}

void dec(uint8_t* x, uint8_t* f) {
  --(*x);
  *f = FLAG(*x == 0, 1, (*x & 0xF) != 0xF, (*f >> 4) & 1);
}

void rrca(uint8_t* x, uint8_t* f) {
  *x = *x << 7 | *x >> 1;
  *f = FLAG(*x == 0, 1, 0, *x >> 7);
}

void gbSwap(uint8_t* x, uint8_t* f) {
  *x = ((*x >> 4) & 0xF) | ((*x << 4) & 0xF0);
  *f = FLAG(*x == 0, 0, 0, 0);
}

uint16_t signExtend16(uint8_t x) {
  return (uint16_t)(int16_t)(int8_t)x;
}

#define READ_NN              \
  nn = mem_->read16(reg_.pc); \
  reg_.pc += 2
#define READ_N n = mem_->read(reg_.pc++)

int CPU::executeCBInst_(uint8_t op) {
  uint8_t n;
  switch (op) {
    // ---- 5.1 ----
    case 0x37:
      gbSwap(&reg_.a, &reg_.f);
      return 8;
    case 0x30:
      gbSwap(&reg_.b, &reg_.f);
      return 8;
    case 0x31:
      gbSwap(&reg_.c, &reg_.f);
      return 8;
    case 0x32:
      gbSwap(&reg_.d, &reg_.f);
      return 8;
    case 0x33:
      gbSwap(&reg_.e, &reg_.f);
      return 8;
    case 0x34:
      gbSwap(&reg_.h, &reg_.f);
      return 8;
    case 0x35:
      gbSwap(&reg_.l, &reg_.f);
      return 8;
    case 0x36:
      n = mem_->read(reg_.hl);
      gbSwap(&n, &reg_.f);
      mem_->write(reg_.hl, n);
      return 16;

    default:
      ERR << "Uknown CB opcode: " << op << endl;
      throw 1;
  }
}

int CPU::executeSingleInstInner_() {
  uint8_t op = mem_->read(reg_.pc++);
  uint16_t nn;
  uint8_t n;
  // ERR << "OP: " << op << endl;
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
      reg_.a = mem_->read(reg_.hl);
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
      reg_.b = mem_->read(reg_.hl);
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
      reg_.c = mem_->read(reg_.hl);
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
      reg_.d = mem_->read(reg_.hl);
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
      reg_.e = mem_->read(reg_.hl);
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
      reg_.h = mem_->read(reg_.hl);
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
      reg_.l = mem_->read(reg_.hl);
      return 8;
    case 0x70:
      mem_->write(reg_.hl, reg_.b);
      return 8;
    case 0x71:
      mem_->write(reg_.hl, reg_.c);
      return 8;
    case 0x72:
      mem_->write(reg_.hl, reg_.d);
      return 8;
    case 0x73:
      mem_->write(reg_.hl, reg_.e);
      return 8;
    case 0x74:
      mem_->write(reg_.hl, reg_.h);
      return 8;
    case 0x75:
      mem_->write(reg_.hl, reg_.l);
      return 8;
    case 0x36:
      READ_N;
      mem_->write(reg_.hl, n);
      return 12;
      // ---- 1.3 ----
    case 0x0A:
      reg_.a = mem_->read(reg_.bc);
      return 8;
    case 0x1A:
      reg_.a = mem_->read(reg_.de);
      return 8;
    case 0xFA:
      READ_NN;
      reg_.a = mem_->read(nn);
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
      mem_->write(reg_.bc, reg_.a);
      return 8;
    case 0x12:
      mem_->write(reg_.de, reg_.a);
      return 8;
    case 0x77:
      mem_->write(reg_.hl, reg_.a);
      return 8;
    case 0xEA:
      READ_NN;
      mem_->write(nn, reg_.a);
      return 16;
      // ---- 1.6 ----
    case 0xE2:
      mem_->write(0xFF00 | reg_.c, reg_.a);
      return 8;
      // ---- 1.12 ----
    case 0x32:
      mem_->write(reg_.hl--, reg_.a);
      return 8;
      // ---- 1.15 ----
    case 0x2A:
      reg_.a = mem_->read(reg_.hl++);
      return 8;
      // ---- 1.19 ----
    case 0xE0:
      READ_N;
      mem_->write(0xFF00 + n, reg_.a);
      return 12;
      // ---- 1.20 ----
    case 0xF0:
      READ_N;
      reg_.a = mem_->read(0xFF00 + n);
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
      // ---- 2.6 ----
    case 0xF5:
      reg_.sp -= 2;
      mem_->write16(reg_.sp, reg_.af);
      return 16;
    case 0xC5:
      reg_.sp -= 2;
      mem_->write16(reg_.sp, reg_.bc);
      return 16;
    case 0xD5:
      reg_.sp -= 2;
      mem_->write16(reg_.sp, reg_.de);
      return 16;
    case 0xE5:
      reg_.sp -= 2;
      mem_->write16(reg_.sp, reg_.hl);
      return 16;
      // ---- 2.7 ----
    case 0xF1:
      reg_.af = mem_->read16(reg_.sp);
      reg_.sp += 2;
      return 12;
    case 0xC1:
      reg_.bc = mem_->read16(reg_.sp);
      reg_.sp += 2;
      return 12;
    case 0xD1:
      reg_.de = mem_->read16(reg_.sp);
      reg_.sp += 2;
      return 12;
    case 0xE1:
      reg_.hl = mem_->read16(reg_.sp);
      reg_.sp += 2;
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
      n = mem_->read(reg_.hl);
      add(&reg_.a, &n, &reg_.f);
      return 8;
    case 0xC6:
      READ_N;
      add(&reg_.a, &n, &reg_.f);
      return 8;
      // ---- 3.5 ----
    case 0xA7:
      doAnd(&reg_.a, &reg_.a, &reg_.f);
      return 4;
    case 0xA0:
      doAnd(&reg_.a, &reg_.b, &reg_.f);
      return 4;
    case 0xA1:
      doAnd(&reg_.a, &reg_.c, &reg_.f);
      return 4;
    case 0xA2:
      doAnd(&reg_.a, &reg_.d, &reg_.f);
      return 4;
    case 0xA3:
      doAnd(&reg_.a, &reg_.e, &reg_.f);
      return 4;
    case 0xA4:
      doAnd(&reg_.a, &reg_.h, &reg_.f);
      return 4;
    case 0xA5:
      doAnd(&reg_.a, &reg_.l, &reg_.f);
      return 4;
    case 0xA6:
      n = mem_->read(reg_.hl);
      doAnd(&reg_.a, &n, &reg_.f);
      return 8;
    case 0xE6:
      READ_N;
      doAnd(&reg_.a, &n, &reg_.f);
      return 8;
      // ---- 3.6 ----
    case 0xB7:
      doOr(&reg_.a, &reg_.a, &reg_.f);
      return 4;
    case 0xB0:
      doOr(&reg_.a, &reg_.b, &reg_.f);
      return 4;
    case 0xB1:
      doOr(&reg_.a, &reg_.c, &reg_.f);
      return 4;
    case 0xB2:
      doOr(&reg_.a, &reg_.d, &reg_.f);
      return 4;
    case 0xB3:
      doOr(&reg_.a, &reg_.e, &reg_.f);
      return 4;
    case 0xB4:
      doOr(&reg_.a, &reg_.h, &reg_.f);
      return 4;
    case 0xB5:
      doOr(&reg_.a, &reg_.l, &reg_.f);
      return 4;
    case 0xB6:
      n = mem_->read(reg_.hl);
      doOr(&reg_.a, &n, &reg_.f);
      return 8;
    case 0xF6:
      READ_N;
      doOr(&reg_.a, &n, &reg_.f);
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
      n = mem_->read(reg_.hl);
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
      cp(reg_.a, mem_->read(reg_.hl), &reg_.f);
      return 8;
    case 0xFE:
      READ_N;
      cp(reg_.a, n, &reg_.f);
      return 8;
      // ---- 3.9 ----
    case 0x3C:
      inc(&reg_.a, &reg_.f);
      return 4;
    case 0x04:
      inc(&reg_.b, &reg_.f);
      return 4;
    case 0x0C:
      inc(&reg_.c, &reg_.f);
      return 4;
    case 0x14:
      inc(&reg_.d, &reg_.f);
      return 4;
    case 0x1C:
      inc(&reg_.e, &reg_.f);
      return 4;
    case 0x24:
      inc(&reg_.h, &reg_.f);
      return 4;
    case 0x2C:
      inc(&reg_.l, &reg_.f);
      return 4;
    case 0x34:
      n = mem_->read(reg_.hl);
      inc(&n, &reg_.f);
      mem_->write(reg_.hl, n);
      return 12;
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
      n = mem_->read(reg_.hl);
      dec(&n, &reg_.f);
      mem_->write(reg_.hl, n);
      return 12;
      // ---- 4.1 ----
    case 0x09:
      add(&reg_.hl, reg_.bc, &reg_.f);
      return 8;
    case 0x19:
      add(&reg_.hl, reg_.de, &reg_.f);
      return 8;
    case 0x29:
      add(&reg_.hl, reg_.hl, &reg_.f);
      return 8;
    case 0x39:
      add(&reg_.hl, reg_.sp, &reg_.f);
      return 8;
      // ---- 4.3 ----
    case 0x03:
      ++ reg_.bc;
      return 8;
    case 0x13:
      ++ reg_.de;
      return 8;
    case 0x23:
      ++ reg_.hl;
      return 8;
    case 0x33:
      ++ reg_.sp;
      return 8;
      // ---- 4.4 ----
    case 0x0B:
      -- reg_.bc;
      return 8;
    case 0x1B:
      -- reg_.de;
      return 8;
    case 0x2B:
      -- reg_.hl;
      return 8;
    case 0x3B:
      -- reg_.sp;
      return 8;
      // ---- 5.3 ----
    case 0x2F:
      reg_.a = ~reg_.a;
      reg_.f = FLAG(FLAG_Z(reg_.f),1,1,FLAG_C(reg_.f));
      return 4;
      // ---- 5.6 ----
    case 0x00:  // NOP
      return 4;
      // ---- 5.8 ----
    case 0x10:
      READ_N;
      if (n != 0) {
        ERR << "PC: " << reg_.pc << endl;
        throw 1;
      }
      // stop_();
      return 4;
      // ---- 5.9 ----
    case 0xF3:
      mem_->io().disableInterrupt();
      return 4;
      // ---- 5.10 ----
    case 0xFB:
      mem_->io().enableInterrupt();
      return 4;
      // ---- 6.3 ----
    case 0x0F:
      rrca(&reg_.a, &reg_.f);
      return 4;
      // ---- CB ----
    case 0xCB:
      READ_N;
      return executeCBInst_(n);
      // ---- 8.2 ----
    case 0xC2:
      READ_NN;
      if (!FLAG_Z(reg_.f)) {
        reg_.pc = nn;
        return 16;
      }
      return 12;
    case 0xCA:
      READ_NN;
      if (FLAG_Z(reg_.f)) {
        reg_.pc = nn;
        return 16;
      }
      return 12;
    case 0xD2:
      READ_NN;
      if (!FLAG_C(reg_.f)) {
        reg_.pc = nn;
        return 16;
      }
      return 12;
    case 0xDA:
      READ_NN;
      if (FLAG_C(reg_.f)) {
        reg_.pc = nn;
        return 16;
      }
      return 12;
      // ---- 8.3 ----
    case 0xE9:
      reg_.pc = mem_->read(reg_.hl);
      return 4;
      // ---- 8.4 ----
    case 0x18:
      READ_N;
      reg_.pc += signExtend16(n);
      return 8;
      // ---- 8.5 ----
    case 0x20:
      READ_N;
      reg_.pc += (!FLAG_Z(reg_.f)) * signExtend16(n);
      return 8;
    case 0x28:
      READ_N;
      reg_.pc += (FLAG_Z(reg_.f)) * signExtend16(n);
      return 8;
    case 0x30:
      READ_N;
      reg_.pc += (!FLAG_C(reg_.f)) * signExtend16(n);
      return 8;
    case 0x38:
      READ_N;
      reg_.pc += (FLAG_C(reg_.f)) * signExtend16(n);
      return 8;
      // ---- 9.1 ----
    case 0xCD:
      READ_NN;
      reg_.sp -= 2;
      mem_->write16(reg_.sp, reg_.pc);
      reg_.pc = nn;
      return 12;
      // ---- 10.1 ----
    case 0xC7:
    case 0xCF:
    case 0xD7:
    case 0xDF:
    case 0xE7:
    case 0xEF:
    case 0xF7:
    case 0xFF:
      reg_.sp -= 2;
      mem_->write16(reg_.sp, reg_.pc);
      reg_.pc = op - 0xC7;
      return 16;
      // ---- 11.1 ----
    case 0xC9:
      reg_.pc = mem_->read16(reg_.sp);
      reg_.sp += 2;
      return 8;
      // ---- 11.2 ----
    case 0xC0:
      if (!FLAG_Z(reg_.f)) {
        reg_.pc = mem_->read16(reg_.sp);
        reg_.sp += 2;
        return 20;
      }
      return 8;
    case 0xC8:
      if (FLAG_Z(reg_.f)) {
        reg_.pc = mem_->read16(reg_.sp);
        reg_.sp += 2;
        return 20;
      }
      return 8;
    case 0xD0:
      if (!FLAG_C(reg_.f)) {
        reg_.pc = mem_->read16(reg_.sp);
        reg_.sp += 2;
        return 20;
      }
      return 8;
    case 0xD8:
      if (FLAG_C(reg_.f)) {
        reg_.pc = mem_->read16(reg_.sp);
        reg_.sp += 2;
        return 20;
      }
      return 8;
      // ---- 11.3 ----
    case 0xD9:
      reg_.pc = mem_->read16(reg_.sp);
      reg_.sp += 2;
      mem_->io().enableInterrupt();
      return 8;

    default:
      ERR << "Uknown opcode: " << op << endl;
      throw 1;
  }
}

int CPU::executeSingleInst() {
  uint16_t interruptAddr = mem_->io().acknowledgeInterrupt();
  if (interruptAddr != 0xFFFF) {
    ERR << "Interrupt ! " << interruptAddr << endl;
    mem_->io().disableInterrupt();
    reg_.sp -= 2;
    mem_->write16(reg_.sp, reg_.pc);
    reg_.pc = interruptAddr;
  }
  return executeSingleInstInner_();
}
