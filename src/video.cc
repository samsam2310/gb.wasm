#include "video.h"

#include "canvas.h"
#include "log.h"

#include <cstring>

// const int CYCLES_PER_FRAME = 70224;
const int MOD_CYCLES[] = { 204, 456, 80, 172 };

Video::Video(IO* io, int canvasId)
    : io_(io), canvasId_(canvasId), timing_(0) {
  memset(buf_, 10, sizeof(buf_));
}

const uint8_t COLOR[] = { 240, 180, 100, 10 };

void Video::renderBackgroundLine_(uint8_t LCDC, uint8_t LY) {
  // ERR << "Video !! BGL " << endl;
  uint16_t mapBase = (((LCDC & 0x8) == 0x8) ? 0x1C00 : 0x1800);
  uint16_t dataBase = (((LCDC & 0x10) == 0x10) ? 0x0 : 0x1000);
  uint8_t SCY = io_->reg(IO::SCY);
  uint8_t SCX = io_->reg(IO::SCX);
  uint8_t BGP = io_->reg(IO::BGP);
  uint8_t y = LY + SCY; // unsigned overflow
  uint16_t ty = y / 8;
  uint8_t py = y & 0x7;
  for (uint8_t i = 0; i < 160; ++i) {
    uint8_t x = i + SCX; // unsigned overflow
    uint16_t tx = x / 8;
    uint8_t px = 7 - (x & 0x7);
    uint16_t tid = io_->vram[mapBase + ty * 32 + tx];
    uint16_t laddr;
    if (dataBase == 0) {
      laddr = dataBase + tid * 16 + py * 2;
    } else {
      ERR << "VIDOE NO IMPL" << endl;
      throw 1;
    }
    uint8_t l1 = io_->vram[laddr];
    uint8_t l2 = io_->vram[laddr + 1];
    uint8_t c1 = ((l1 >> px) & 1) | (((l2 >> px) & 1) << 1);
    uint8_t c2 = ((BGP >> (c1 << 1)) & 0x3);
    buf_[LY * 160 + i] = COLOR[c2];
  }
}

void Video::renderSpriteLine_(uint8_t LCDC, uint8_t LY) {
  // ERR << "Sprite NO IMPL" << endl;
  // throw 1;
}

void Video::renderLine_(uint8_t LCDC, uint8_t LY) {
  if ((LCDC & 0x1) == 0x1) {
    renderBackgroundLine_(LCDC, LY);
  }
  // TODO Window Line
  if ((LCDC & 0x2) == 0x2) {
    renderSpriteLine_(LCDC, LY);
  }
  // ERR << "Video !! LCDC " << io_->reg(IO::LCDC) << endl;
  // for (uint16_t i = 0; i < 160; ++i) {
  //   buf_[LY * 160 + i] = (i + LY) & 0xFF;
  // }
}

void Video::drawFrame_(uint8_t LCDC) {
  // ERR << "Video !! LCDC " << LCDC << " " << (LCDC & 0x80) << " " << ((LCDC & 0x80) != 0x80) << endl;
  if ((LCDC & 0x80) != 0x80) {
    // ERR << "Video RESET" << endl;
    // memset(buf_, 0, sizeof(buf_));
  }
  renderCanvas(canvasId_, buf_);
}

void Video::runCycles(int cycles) {
  uint8_t LCDC = io_->reg(IO::LCDC);
  if ((LCDC & 0x80) != 0x80)
    return;

  timing_ -= cycles;
  if (timing_ > 0)
    return;

  uint8_t& STAT = io_->reg(IO::STAT);
  uint8_t& LY = io_->reg(IO::LY);
  uint8_t LYC = io_->reg(IO::LYC);
  uint8_t mod = STAT & 0x3;
  bool mod_intr = false;
  bool LY_intr = false;
  switch (mod) {
    case 0:
      mod = (LY == 144 ? 1 : 2);
      mod_intr = true;
      break;
    case 1:
      ++LY;
      if (LY == 154) {
        drawFrame_(LCDC);
        LY = 0;
        mod = (LCDC & 0x80 ? 2 : 1);
      }
      LY_intr = true;
      mod_intr = (mod == 2);
      break;
    case 2:
      mod = 3;
      break;
    case 3:
      renderLine_(LCDC, LY);
      ++LY;
      mod = 0;
      mod_intr = true;
      LY_intr = true;
      break;
    default:
      throw 1;
  }
  timing_ += MOD_CYCLES[mod];
  STAT = (STAT & 0xF8) | ((LY == LYC) << 2) | mod;
  if (mod_intr && mod == 1) {
    io_->requestInterrupt(IO::VBLANK);
  }
  if (mod_intr && N_BIT(STAT, 3 + mod)) {
    io_->requestInterrupt(IO::LCDC_STATUS);
  }
  if (LY_intr && LY == LYC && N_BIT(STAT, 6)) {
    io_->requestInterrupt(IO::LCDC_STATUS);
  }
}
