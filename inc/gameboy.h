#pragma once
#include "cpu.h"
#include "io.h"
#include "memory.h"
#include "timer.h"
#include "video.h"

#include <stdint.h>

class Gameboy {
 private:
  IO io_;
  Memory mem_;
  Video video_;
  Timer timer_;
  CPU cpu_;
  bool isRunning_;
  int timing_;

 public:
  Gameboy(uint8_t* romData, int canvasId);
  bool run();
  // bool pause();
  // bool stop();
  void executeSingleFrame(uint8_t joypad);
};