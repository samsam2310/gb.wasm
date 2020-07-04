#pragma once
#include "io.h"

#include <stdint.h>

class Video {
 private:
  IO* io_;
  int canvasId_;
  int timing_;
  uint8_t buf_[144 * 160];

  void renderLine_(uint8_t LCDC);
  void drawFrame_(uint8_t LY);
 public:
  Video(IO* io, int canvasId);
  void runCycles(int cycles);
};