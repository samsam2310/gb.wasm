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

EXPORT void executeSingleFrame(Gameboy* gb, uint8_t joypad) {
  gb->executeSingleFrame(joypad);
}

EXPORT void dump() {
  ERR.mem(0xFF80).mem(0xFF81).mem(0xFFE1) << endl;
  // ERR.mem(0xFF00) << endl;
  // ERR << "oam:" << endl;
  // for (uint16_t i = 0; i < 0xA0; i += 4) {
  //   ERR.mem(i).mem(i+1).mem(i+2).mem(i+3) << endl;

  // }
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