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
#define FLAG_Z(f) (((f) >> 7) & 1)
#define FLAG_N(f) (((f) >> 6) & 1)
#define FLAG_H(f) (((f) >> 5) & 1)
#define FLAG_C(f) (((f) >> 4) & 1)
#define N_BIT(x, n) (((x) >> (n)) & 1)

void add(uint8_t* x, uint8_t* y, uint8_t* f) {
  uint8_t r8 = (*x & 0xF) + (*y & 0xF);
  uint16_t r16 = (uint16_t)*x + (uint16_t)*y;
  *x += *y;
  *f = FLAG(*x == 0, 0, r8 >> 4 != 0, r16 >> 8 != 0);
}

void adc(uint8_t* x, uint8_t y, uint8_t* f) {
  uint8_t r8 = (*x & 0xF) + (y & 0xF) + FLAG_C(*f);
  uint16_t r16 = (uint16_t)*x + (uint16_t)y;
  *x += y + FLAG_C(*f);
  *f = FLAG(*x == 0, 0, r8 >> 4 != 0, r16 >> 8 != 0);
}

void sub(uint8_t* x, uint8_t y, uint8_t* f) {
  *f = FLAG(*x == y, 1, (*x & 0xF) < (y & 0xF), *x < y);
  *x -= y;
}

void sbc(uint8_t* x, uint8_t y, uint8_t* f) {
  *f = FLAG(*x == y, 1, (*x & 0xF) < (y & 0xF) + FLAG_C(*f), *x < y + FLAG_C(*f));
  *x -= y + FLAG_C(*f);
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

void rlc(uint8_t* x, uint8_t* f) {
  *x = (*x << 1) | (*x >> 7);
  *f = FLAG(*x == 0, 0, 0, *x & 1);
}

void rl(uint8_t* x, uint8_t* f) {
  uint8_t c = (*x >> 7);
  *x = (*x << 1) | FLAG_C(*f);
  *f = FLAG(*x == 0, 0, 0, c);
}

void rrc(uint8_t* x, uint8_t* f) {
  *f = FLAG(*x == 0, 0, 0, *x & 1);
  *x = (*x >> 1) | (*x << 7);
}

void rr(uint8_t* x, uint8_t* f) {
  uint8_t c = (*x & 1);
  *x = (*x >> 1) | (FLAG_C(*f) << 7);
  *f = FLAG(*x == 0, 0, 0, c);
}

void sla(uint8_t* x, uint8_t* f) {
  uint8_t c = N_BIT(*x, 7);
  *x <<= 1;
  *f = FLAG(*x == 0, 0, 0, c);
}

void sra(uint8_t* x, uint8_t* f) {
  uint8_t c = (*x & 1);
  *x = (*x & 0x80) | (*x >> 1);
  *f = FLAG(*x == 0, 0, 0, c);
}

void srl(uint8_t* x, uint8_t* f) {
  uint8_t c = (*x & 1);
  *x >>= 1;
  *f = FLAG(*x == 0, 0, 0, c);
}

void gbSwap(uint8_t* x, uint8_t* f) {
  *x = ((*x >> 4) & 0xF) | ((*x << 4) & 0xF0);
  *f = FLAG(*x == 0, 0, 0, 0);
}

void bit(uint8_t x, uint8_t b, uint8_t* f) {
  *f = FLAG(((x >> b) & 1) == 0, 0, 1, FLAG_C(*f));
}

void daa(uint8_t* x, uint8_t* f) {
  int t = 0;
  uint8_t c = 0;
  uint8_t h = 0;
  if(FLAG_H(*f) || ((*x & 0xF) > 9)) {
    ++t;
  }
  if(FLAG_C(*f) || (*x > 0x99)) {
    t += 2;
    c = 1;
  }
  if (FLAG_N(*f) && !FLAG_H(*f));
  else if (FLAG_N(*f) && FLAG_H(*f)) {
    h = (((*x & 0x0F)) < 6);
  } else {
    h = ((*x & 0x0F) >= 0x0A);
  }
  switch(t) {
    case 1:
      *x += (FLAG_N(*f)) ? 0xFA : 0x06; // -6:6
      break;
    case 2:
      *x += (FLAG_N(*f)) ? 0xA0 : 0x60; // -0x60:0x60
      break;
    case 3:
      *x += (FLAG_N(*f)) ? 0x9A : 0x66; // -0x66:0x66
      break;
  }
  *f = FLAG(*x == 0, FLAG_N(*f), h, c);
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
      // ---- 6.5 ----
    case 0x07:
      rlc(&reg_.a, &reg_.f);
      return 8;
    case 0x00:
      rlc(&reg_.b, &reg_.f);
      return 8;
    case 0x01:
      rlc(&reg_.c, &reg_.f);
      return 8;
    case 0x02:
      rlc(&reg_.d, &reg_.f);
      return 8;
    case 0x03:
      rlc(&reg_.e, &reg_.f);
      return 8;
    case 0x04:
      rlc(&reg_.h, &reg_.f);
      return 8;
    case 0x05:
      rlc(&reg_.l, &reg_.f);
      return 8;
    case 0x06:
      n = mem_->read(reg_.hl);
      rlc(&n, &reg_.f);
      mem_->write(reg_.hl, n);
      return 16;
      // ---- 6.6 ----
    case 0x17:
      rl(&reg_.a, &reg_.f);
      return 8;
    case 0x10:
      rl(&reg_.b, &reg_.f);
      return 8;
    case 0x11:
      rl(&reg_.c, &reg_.f);
      return 8;
    case 0x12:
      rl(&reg_.d, &reg_.f);
      return 8;
    case 0x13:
      rl(&reg_.e, &reg_.f);
      return 8;
    case 0x14:
      rl(&reg_.h, &reg_.f);
      return 8;
    case 0x15:
      rl(&reg_.l, &reg_.f);
      return 8;
    case 0x16:
      n = mem_->read(reg_.hl);
      rl(&n, &reg_.f);
      mem_->write(reg_.hl, n);
      return 16;
      // ---- 6.7 ----
    case 0x0F:
      rrc(&reg_.a, &reg_.f);
      return 8;
    case 0x08:
      rrc(&reg_.b, &reg_.f);
      return 8;
    case 0x09:
      rrc(&reg_.c, &reg_.f);
      return 8;
    case 0x0A:
      rrc(&reg_.d, &reg_.f);
      return 8;
    case 0x0B:
      rrc(&reg_.e, &reg_.f);
      return 8;
    case 0x0C:
      rrc(&reg_.h, &reg_.f);
      return 8;
    case 0x0D:
      rrc(&reg_.l, &reg_.f);
      return 8;
    case 0x0E:
      n = mem_->read(reg_.hl);
      rrc(&n, &reg_.f);
      mem_->write(reg_.hl, n);
      return 16;
      // ---- 6.8 ----
    case 0x1F:
      rr(&reg_.a, &reg_.f);
      return 8;
    case 0x18:
      rr(&reg_.b, &reg_.f);
      return 8;
    case 0x19:
      rr(&reg_.c, &reg_.f);
      return 8;
    case 0x1A:
      rr(&reg_.d, &reg_.f);
      return 8;
    case 0x1B:
      rr(&reg_.e, &reg_.f);
      return 8;
    case 0x1C:
      rr(&reg_.h, &reg_.f);
      return 8;
    case 0x1D:
      rr(&reg_.l, &reg_.f);
      return 8;
    case 0x1E:
      n = mem_->read(reg_.hl);
      rr(&n, &reg_.f);
      mem_->write(reg_.hl, n);
      return 16;
      // ---- 6.9 ----
    case 0x27:
      sla(&reg_.a, &reg_.f);
      return 8;
    case 0x20:
      sla(&reg_.b, &reg_.f);
      return 8;
    case 0x21:
      sla(&reg_.c, &reg_.f);
      return 8;
    case 0x22:
      sla(&reg_.d, &reg_.f);
      return 8;
    case 0x23:
      sla(&reg_.e, &reg_.f);
      return 8;
    case 0x24:
      sla(&reg_.h, &reg_.f);
      return 8;
    case 0x25:
      sla(&reg_.l, &reg_.f);
      return 8;
    case 0x26:
      n = mem_->read(reg_.hl);
      sla(&n, &reg_.f);
      mem_->write(reg_.hl, n);
      return 16;
      // ---- 6.10 ----
    case 0x2F:
      sra(&reg_.a, &reg_.f);
      return 8;
    case 0x28:
      sra(&reg_.b, &reg_.f);
      return 8;
    case 0x29:
      sra(&reg_.c, &reg_.f);
      return 8;
    case 0x2A:
      sra(&reg_.d, &reg_.f);
      return 8;
    case 0x2B:
      sra(&reg_.e, &reg_.f);
      return 8;
    case 0x2C:
      sra(&reg_.h, &reg_.f);
      return 8;
    case 0x2D:
      sra(&reg_.l, &reg_.f);
      return 8;
    case 0x2E:
      n = mem_->read(reg_.hl);
      sra(&n, &reg_.f);
      mem_->write(reg_.hl, n);
      return 16;
      // ---- 6.11 ----
    case 0x3F:
      srl(&reg_.a, &reg_.f);
      return 8;
    case 0x38:
      srl(&reg_.b, &reg_.f);
      return 8;
    case 0x39:
      srl(&reg_.c, &reg_.f);
      return 8;
    case 0x3A:
      srl(&reg_.d, &reg_.f);
      return 8;
    case 0x3B:
      srl(&reg_.e, &reg_.f);
      return 8;
    case 0x3C:
      srl(&reg_.h, &reg_.f);
      return 8;
    case 0x3D:
      srl(&reg_.l, &reg_.f);
      return 8;
    case 0x3E:
      n = mem_->read(reg_.hl);
      srl(&n, &reg_.f);
      mem_->write(reg_.hl, n);
      return 16;
      // BIT
    case 0x40:
    case 0x48:
    case 0x50:
    case 0x58:
    case 0x60:
    case 0x68:
    case 0x70:
    case 0x78:
      bit(reg_.b, (op >> 3) & 0x3, &reg_.f);
      return 8;
    case 0x41:
    case 0x49:
    case 0x51:
    case 0x59:
    case 0x61:
    case 0x69:
    case 0x71:
    case 0x79:
      bit(reg_.c, (op >> 3) & 0x3, &reg_.f);
      return 8;
    case 0x42:
    case 0x4A:
    case 0x52:
    case 0x5A:
    case 0x62:
    case 0x6A:
    case 0x72:
    case 0x7A:
      bit(reg_.d, (op >> 3) & 0x3, &reg_.f);
      return 8;
    case 0x43:
    case 0x4B:
    case 0x53:
    case 0x5B:
    case 0x63:
    case 0x6B:
    case 0x73:
    case 0x7B:
      bit(reg_.e, (op >> 3) & 0x3, &reg_.f);
      return 8;
    case 0x44:
    case 0x4C:
    case 0x54:
    case 0x5C:
    case 0x64:
    case 0x6C:
    case 0x74:
    case 0x7C:
      bit(reg_.h, (op >> 3) & 0x3, &reg_.f);
      return 8;
    case 0x45:
    case 0x4D:
    case 0x55:
    case 0x5D:
    case 0x65:
    case 0x6D:
    case 0x75:
    case 0x7D:
      bit(reg_.l, (op >> 3) & 0x3, &reg_.f);
      return 8;
    case 0x46:
    case 0x4E:
    case 0x56:
    case 0x5E:
    case 0x66:
    case 0x6E:
    case 0x76:
    case 0x7E:
      bit(mem_->read(reg_.hl), (op >> 3) & 0x3, &reg_.f);
      return 12;
    case 0x47:
    case 0x4F:
    case 0x57:
    case 0x5F:
    case 0x67:
    case 0x6F:
    case 0x77:
    case 0x7F:
      bit(reg_.a, (op >> 3) & 0x3, &reg_.f);
      return 8;
      // RESET
    case 0x80:
    case 0x88:
    case 0x90:
    case 0x98:
    case 0xA0:
    case 0xA8:
    case 0xB0:
    case 0xB8:
      reg_.b &= ~(1 << ((op >> 3) & 0x3));
      return 8;
    case 0x81:
    case 0x89:
    case 0x91:
    case 0x99:
    case 0xA1:
    case 0xA9:
    case 0xB1:
    case 0xB9:
      reg_.c &= ~(1 << ((op >> 3) & 0x3));
      return 8;
    case 0x82:
    case 0x8A:
    case 0x92:
    case 0x9A:
    case 0xA2:
    case 0xAA:
    case 0xB2:
    case 0xBA:
      reg_.d &= ~(1 << ((op >> 3) & 0x3));
      return 8;
    case 0x83:
    case 0x8B:
    case 0x93:
    case 0x9B:
    case 0xA3:
    case 0xAB:
    case 0xB3:
    case 0xBB:
      reg_.e &= ~(1 << ((op >> 3) & 0x3));
      return 8;
    case 0x84:
    case 0x8C:
    case 0x94:
    case 0x9C:
    case 0xA4:
    case 0xAC:
    case 0xB4:
    case 0xBC:
      reg_.h &= ~(1 << ((op >> 3) & 0x3));
      return 8;
    case 0x85:
    case 0x8D:
    case 0x95:
    case 0x9D:
    case 0xA5:
    case 0xAD:
    case 0xB5:
    case 0xBD:
      reg_.h &= ~(1 << ((op >> 3) & 0x3));
      return 8;
    case 0x86:
    case 0x8E:
    case 0x96:
    case 0x9E:
    case 0xA6:
    case 0xAE:
    case 0xB6:
    case 0xBE:
      n = mem_->read(reg_.hl);
      n &= ~(1 << ((op >> 3) & 0x3));
      mem_->write(reg_.hl, n);
      return 8;
    case 0x87:
    case 0x8F:
    case 0x97:
    case 0x9F:
    case 0xA7:
    case 0xAF:
    case 0xB7:
    case 0xBF:
      reg_.a &= ~(1 << ((op >> 3) & 0x3));
      return 8;
      // SET
    case 0xC0:
    case 0xC8:
    case 0xD0:
    case 0xD8:
    case 0xE0:
    case 0xE8:
    case 0xF0:
    case 0xF8:
      reg_.b |= (1 << ((op >> 3) & 0x3));
      return 8;
    case 0xC1:
    case 0xC9:
    case 0xD1:
    case 0xD9:
    case 0xE1:
    case 0xE9:
    case 0xF1:
    case 0xF9:
      reg_.c |= (1 << ((op >> 3) & 0x3));
      return 8;
    case 0xC2:
    case 0xCA:
    case 0xD2:
    case 0xDA:
    case 0xE2:
    case 0xEA:
    case 0xF2:
    case 0xFA:
      reg_.d |= (1 << ((op >> 3) & 0x3));
      return 8;
    case 0xC3:
    case 0xCB:
    case 0xD3:
    case 0xDB:
    case 0xE3:
    case 0xEB:
    case 0xF3:
    case 0xFB:
      reg_.e |= (1 << ((op >> 3) & 0x3));
      return 8;
    case 0xC4:
    case 0xCC:
    case 0xD4:
    case 0xDC:
    case 0xE4:
    case 0xEC:
    case 0xF4:
    case 0xFC:
      reg_.h |= (1 << ((op >> 3) & 0x3));
      return 8;
    case 0xC5:
    case 0xCD:
    case 0xD5:
    case 0xDD:
    case 0xE5:
    case 0xED:
    case 0xF5:
    case 0xFD:
      reg_.l |= (1 << ((op >> 3) & 0x3));
      return 8;
    case 0xC6:
    case 0xCE:
    case 0xD6:
    case 0xDE:
    case 0xE6:
    case 0xEE:
    case 0xF6:
    case 0xFE:
      n = mem_->read(reg_.hl);
      n |= (1 << ((op >> 3) & 0x3));
      mem_->write(reg_.hl, n);
      return 8;
    case 0xC7:
    case 0xCF:
    case 0xD7:
    case 0xDF:
    case 0xE7:
    case 0xEF:
    case 0xF7:
    case 0xFF:
      reg_.a |= (1 << ((op >> 3) & 0x3));
      return 8;

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
      // ---- 1.5 ----
    case 0xF2:
      reg_.a = mem_->read(0xFF00 | reg_.c);
      return 8;
      // ---- 1.6 ----
    case 0xE2:
      mem_->write(0xFF00 | reg_.c, reg_.a);
      return 8;
      // ---- 1.9 ---- (1.7 1.8)
    case 0x3A:
      reg_.a = mem_->read(reg_.hl--);
      return 8;
      // ---- 1.12 ---- (1.10 1.11)
    case 0x32:
      mem_->write(reg_.hl--, reg_.a);
      return 8;
      // ---- 1.15 ---- (1.13 1.14)
    case 0x2A:
      reg_.a = mem_->read(reg_.hl++);
      return 8;
      // ---- 1.18 ---- (1.16 1.17)
    case 0x22:
      mem_->write(reg_.hl++, reg_.a);
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
      // ---- 2.2 ----
    case 0xF9:
      reg_.sp = reg_.hl;
      return 8;
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
      // ---- 6.2 ----
    case 0x8F:
      adc(&reg_.a, reg_.a, &reg_.f);
      return 4;
    case 0x88:
      adc(&reg_.a, reg_.b, &reg_.f);
      return 4;
    case 0x89:
      adc(&reg_.a, reg_.c, &reg_.f);
      return 4;
    case 0x8A:
      adc(&reg_.a, reg_.d, &reg_.f);
      return 4;
    case 0x8B:
      adc(&reg_.a, reg_.e, &reg_.f);
      return 4;
    case 0x8C:
      adc(&reg_.a, reg_.h, &reg_.f);
      return 4;
    case 0x8D:
      adc(&reg_.a, reg_.l, &reg_.f);
      return 4;
    case 0x8E:
      n = mem_->read(reg_.hl);
      adc(&reg_.a, n, &reg_.f);
      return 8;
    case 0xCE:
      READ_N;
      adc(&reg_.a, n, &reg_.f);
      return 8;
      // ---- 3.3 ----
    case 0x97:
      sub(&reg_.a, reg_.a, &reg_.f);
      return 4;
    case 0x90:
      sub(&reg_.a, reg_.b, &reg_.f);
      return 4;
    case 0x91:
      sub(&reg_.a, reg_.c, &reg_.f);
      return 4;
    case 0x92:
      sub(&reg_.a, reg_.d, &reg_.f);
      return 4;
    case 0x93:
      sub(&reg_.a, reg_.e, &reg_.f);
      return 4;
    case 0x94:
      sub(&reg_.a, reg_.h, &reg_.f);
      return 4;
    case 0x95:
      sub(&reg_.a, reg_.l, &reg_.f);
      return 4;
    case 0x96:
      n = mem_->read(reg_.hl);
      sub(&reg_.a, n, &reg_.f);
      return 8;
    case 0xD6:
      READ_N;
      sub(&reg_.a, n, &reg_.f);
      return 8;
      // ---- 3.4 ----
    case 0x9F:
      sbc(&reg_.a, reg_.a, &reg_.f);
      return 4;
    case 0x98:
      sbc(&reg_.a, reg_.b, &reg_.f);
      return 4;
    case 0x99:
      sbc(&reg_.a, reg_.c, &reg_.f);
      return 4;
    case 0x9A:
      sbc(&reg_.a, reg_.d, &reg_.f);
      return 4;
    case 0x9B:
      sbc(&reg_.a, reg_.e, &reg_.f);
      return 4;
    case 0x9C:
      sbc(&reg_.a, reg_.h, &reg_.f);
      return 4;
    case 0x9D:
      sbc(&reg_.a, reg_.l, &reg_.f);
      return 4;
    case 0x9E:
      n = mem_->read(reg_.hl);
      sbc(&reg_.a, n, &reg_.f);
      return 8;
    case 0xDE:
      READ_N;
      sbc(&reg_.a, n, &reg_.f);
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
      // ---- 5.2 ----
    case 0x27:
      daa(&reg_.a, &reg_.f);
      return 4;
      // ---- 5.3 ----
    case 0x2F:
      reg_.a = ~reg_.a;
      reg_.f = FLAG(FLAG_Z(reg_.f),1,1,FLAG_C(reg_.f));
      return 4;
      // ---- 5.4 ----
    case 0x3F:
      reg_.f = FLAG(FLAG_Z(reg_.f), 0, 0, !FLAG_C(reg_.f));
      return 4;
      // ---- 5.5 ----
    case 0x37:
      reg_.f = FLAG(FLAG_Z(reg_.f), 0, 0, 1);
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
      // ---- 6.1 ----
    case 0x07:
      rlc(&reg_.a, &reg_.f);
      return 4;
      // ---- 6.1 ----
    case 0x17:
      rl(&reg_.a, &reg_.f);
      return 4;
      // ---- 6.3 ----
    case 0x0F:
      rrc(&reg_.a, &reg_.f);
      return 4;
      // ---- 6.4 ----
    case 0x1F:
      rr(&reg_.a, &reg_.f);
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
      // reg_.pc = mem_->read(reg_.hl); ?!?!
      reg_.pc = reg_.hl;
      return 4;
      // ---- 8.4 ----
    case 0x18:
      READ_N;
      reg_.pc += signExtend16(n);
      return 8;
      // ---- 8.5 ----
    case 0x20:
      READ_N;
      nn = (!FLAG_Z(reg_.f));
      reg_.pc += nn * signExtend16(n);
      return 8 + nn * 4;
    case 0x28:
      READ_N;
      nn = (FLAG_Z(reg_.f));
      reg_.pc += nn * signExtend16(n);
      return 8 + nn * 4;
    case 0x30:
      READ_N;
      nn = (!FLAG_C(reg_.f));
      reg_.pc += nn * signExtend16(n);
      return 8 + nn * 4;
    case 0x38:
      READ_N;
      nn = (FLAG_C(reg_.f));
      reg_.pc += nn * signExtend16(n);
      return 8 + nn * 4;
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
      // ERR << "RETI" << endl;
      return 8;

    default:
      ERR << "Uknown opcode: " << op << endl;
      throw 1;
  }
}

int CPU::executeSingleInst() {
  uint16_t interruptAddr = mem_->io().acknowledgeInterrupt();
  if (interruptAddr != 0xFFFF) {
    // ERR << "Interrupt ! " << interruptAddr << endl;
    mem_->io().disableInterrupt();
    reg_.sp -= 2;
    mem_->write16(reg_.sp, reg_.pc);
    reg_.pc = interruptAddr;
  }
  return executeSingleInstInner_();
}
