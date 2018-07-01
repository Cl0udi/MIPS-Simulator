#include <iostream>
#include <fstream>
#include <cstdint>
#include <iomanip>
#include "CPU.h"
#include "Memory.h"
#include "Debug.h"
using namespace std;

const int MEMSIZE = 1 << 20; // 2^20

int main(int argc, char *argv[]) {
  int count, start;
  ifstream exeFile;
  uint8_t bytes[4];

  cout << "CS 3339 MIPS Simulator" << endl;
  if(argc != 2) {
    cerr << "usage: " << argv[0] << " mips_executable" << endl;
    return -1;
  }

  // open executable
  exeFile.open(argv[1], ios::binary | ios::in);
  if(!exeFile) {
    cerr << "error: could not open executable file " << argv[1] << endl;
    return -1;
  }

  // BE->LE swap: Executable files are stored 0A0B0C0D => addr 00,01,02,03
  //              Read into bytes[] as b[0],b[1],b[2],b[3] = 0A,0B,0C,0D
  //              Need to swizzle bytes back into little-endian
  
  // read # of words in file
  if(!exeFile.read((char *)&bytes, 4)) {
    cerr << "error: could not read count from file " << argv[1] << endl;
    return -1;
  }
  count = Memory::swizzle(bytes);

  // read start address from file
  if(!exeFile.read((char *)&bytes, 4)) {
    cerr << "error: could not read start addr from file " << argv[1] << endl;
    return -1;
  }
  start = Memory::swizzle(bytes);

  // Memories
  Memory instMem(count << 4, 0x400000, false); // 4 bytes per inst
  Memory dataMem(MEMSIZE, 0x10000000, true);

  // CPU
  CPU cpu(start, instMem, dataMem);

  // initialize the instruction memory
  instMem.initFromExe(exeFile, count);
  exeFile.close();

  cout << "Running: " << argv[1] << endl << endl;
  cpu.run();

  // Finish-up stats
  cout << endl;
  cpu.printFinalStats();

  return 0;
}
