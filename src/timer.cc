#include "timer.h"

Timer::Timer(IO* io): io_(io), divTiming_(0), timing_(0) {}

const int CYCLE_PER_SECOND = 4194304;
const int DIV_TIMING = CYCLE_PER_SECOND / 16384;
const int TIMING[] = {
  CYCLE_PER_SECOND / 4096,
  CYCLE_PER_SECOND / 262144,
  CYCLE_PER_SECOND / 65536,
  CYCLE_PER_SECOND / 16384,
};

void Timer::runCycles(int cycles) {
  divTiming_ -= cycles;
  if (divTiming_ < 0) {
    divTiming_ += DIV_TIMING;
    io_->reg(IO::DIV) += 1;
  }

  uint8_t tac = io_->reg(IO::TAC);
  if ((tac & 0x4) != 0x4) {
    timing_ = 0;
    return;
  }
  timing_ -= cycles;
  if (timing_ > 0)
    return;

  timing_ += TIMING[tac & 0x3];
  uint8_t& tima = io_->reg(IO::TIMA);
  if (++ tima != 0)
    return;
  tima = io_->reg(IO::TMA);
  io_->requestInterrupt(IO::IRQ_TIMER);
}