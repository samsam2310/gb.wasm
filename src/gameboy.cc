#include "gameboy.h"

#include "cartridge.h"
#include "memory.h"
#include "log.h"

#include <emscripten.h>

const int CYCLE_PER_SECOND = 4194304;
const int FPS = 64;
const int CYCLE_PER_FRAME = CYCLE_PER_SECOND / FPS;

Gameboy::Gameboy(uint8_t* romData, int canvasId)
    : io_(&mem_, &video_), mem_(Cartridge(romData), &io_), video_(&io_, canvasId), cpu_(&mem_), isRunning_(false), timing_(0) {
  ERR.set(&cpu_);
  ERR.set(&mem_);
}

void Gameboy::executeSingleFrame() {
  timing_ += CYCLE_PER_FRAME;
  while (timing_ > 0) {
    int cycle = cpu_.executeSingleInst();
    video_.runCycles(cycle);
    timing_ -= cycle;
  }
  ERR.pc().mem(0xFFE1) << " " << io_.reg(IO::LCDC) << endl;
}