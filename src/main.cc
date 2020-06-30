#include "cartridge.h"
#include "cpu.h"
#include "io.h"
#include "memory.h"

#include <fstream>
#include <iostream>
using namespace std;

int main(int argc, char* argv[]) {
  if (argc <= 1) {
    cout << argv[0] << " ROM_FILE\n";
    return 1;
  }
  ifstream romFile(argv[1], ios::binary);
  romFile.seekg(0, ios::end);
  size_t size = romFile.tellg();
  romFile.seekg(0, ios::beg);

  char* romData = new char[size];
  romFile.read(romData, size);
  romFile.close();

  CPU cpu(Memory(IO(), Cartridge(reinterpret_cast<uint8_t*>(romData))));
  while (true) {
    cpu.execute(1);
  }
}