#include "video.h"

#include "canvas.h"
#include "log.h"

#include <cstring>

// const int CYCLES_PER_FRAME = 70224;
const int MOD_CYCLES[] = { 204, 456, 80, 172 };

Video::Video(IO* io, int canvasId)
    : io_(io), canvasId_(canvasId), timing_(0) {
  memset(buf_, 200, sizeof(buf_));
  io_->reg(IO::LCDC) = 0x0;
  io_->reg(IO::STAT) = 0x1;
  io_->reg(IO::LY) = 0x0;
  io_->requestInterrupt(IO::VBLANK);
}

void Video::renderLine_(uint8_t LY) {
  for (uint16_t i = 0; i < 160; ++i) {
    buf_[LY * 160 + i] = (i + LY) & 0xFF;
  }
}

void Video::drawFrame_(uint8_t LCDC) {
  if (!(LCDC & 0x80)) {
    memset(buf_, 200, sizeof(buf_));
  }
  renderCanvas(canvasId_, buf_);
}

void Video::runCycles(int cycles) {
  timing_ -= cycles;
  if (timing_ > 0)
    return;

  uint8_t LCDC = io_->reg(IO::LCDC);
  uint8_t& STAT = io_->reg(IO::STAT);
  uint8_t& LY = io_->reg(IO::LY);
  uint8_t mod = STAT & 0x3;
  switch (mod) {
    case 0:
      mod = (LY == 144 ? 1 : 2);
      break;
    case 1:
      ++LY;
      if (LY == 154) {
        drawFrame_(LCDC);
        LY = 0;
        mod = (LCDC & 0x80 ? 2 : 1);
      }
      break;
    case 2:
      mod = 3;
      break;
    case 3:
      renderLine_(LY);
      ++LY;
      mod = 0;
      break;
    default:
      throw 1;
  }
  timing_ += MOD_CYCLES[mod];
  STAT = (STAT & 0xFC) | mod;
}
