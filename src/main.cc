#include "gameboy.h"

#include "log.h"

#include <emscripten.h>

// #include <fstream>
#include <cstdio>
// #include <iostream>

#define EXPORT EMSCRIPTEN_KEEPALIVE

extern "C" {
EXPORT Gameboy* createGameboy(uint8_t* romData, int canvasId) {
  return new Gameboy(romData, canvasId);
}

EXPORT void executeSingleFrame(Gameboy* gb) {
  gb->executeSingleFrame();
}
}

// EXPORT bool pauseGameboy(Gameboy* gb) {
//   return gb->pause();
// }

// EXPORT bool stopGameboy(Gameboy* gb) {
//   return gb->stop();
// }

// int main(int argc, char* argv[]) {
//   if (argc <= 1) {
//     cout << argv[0] << " ROM_FILE\n";
//     return 1;
//   }
//   ifstream romFile(argv[1], ios::binary);
//   romFile.seekg(0, ios::end);
//   size_t size = romFile.tellg();
//   romFile.seekg(0, ios::beg);

//   char* romData = new char[size];
//   romFile.read(romData, size);
//   romFile.close();

//   CPU cpu(Memory(IO(), Cartridge(reinterpret_cast<uint8_t*>(romData))));
//   while (true) {
//     cpu.execute(1);
//   }
// }