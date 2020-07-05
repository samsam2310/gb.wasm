#pragma once
#include "io.h"

#include <stdint.h>

class IO;

class Timer {
 private:
  IO* io_;
  int divTiming_;
  int timing_;
 public:
  Timer(IO* io);
  ~Timer() {}
  void runCycles(int cycles);
};