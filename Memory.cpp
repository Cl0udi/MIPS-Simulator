#include "Memory.h"

Memory::Memory(int numBytes, uint32_t offset, bool isDataMem) {
  this->offset = offset;
  this->numBytes = numBytes;
  numWords = numBytes >> 2;
  
  this->isDataMem = isDataMem;
  type = isDataMem ? "data" : "inst";

  mem = new uint32_t[numWords];
  if(!mem) {
    cerr << "error: out of memory" << endl;
    exit(-1);
  }
}

void Memory::storeWord(uint32_t data, uint32_t addr) {
  if((addr & 3) != 0) {
    cerr << "unaligned " << type << " access: 0x" << hex << addr << endl;
    exit(-1);
  }

  uint32_t index = (addr - offset) >> 2;
  if(index >= numWords) {
    cerr << type << " memory access out of range: 0x" << hex << addr << endl;
    exit(-1);
  }

  D(if(isDataMem) cout << "    MEM WR: addr = 0x" << hex << addr << ", data = 0x" << data << dec << endl);
  mem[index] = data;
}

uint32_t Memory::loadWord(uint32_t addr) {
  if((addr & 3) != 0) {
    cerr << "unaligned " << type << " memory access: 0x" << hex << addr << endl;
    exit(-1);
  }

  uint32_t index = (addr - offset) >> 2;
  if(index >= numWords) {
    cerr << type << " memory access out of range: 0x" << hex << addr << endl;
    exit(-1);
  }

  D(if(isDataMem) cout << "    MEM RD: addr = 0x" << hex << addr << ", data = 0x" << mem[index] << dec << endl);
  return mem[index];

}

uint32_t Memory::swizzle(uint8_t *bytes) {
  return (bytes[0] << 24) | (bytes[1] << 16) | bytes[2] << 8 | bytes[3];
}

void Memory::initFromExe(ifstream &exeFile, int count) {
  uint8_t bytes[4];

  if(count > numWords) {
    cerr << "allocated " << type << " array not big enough for " << count << " words" << endl;
    exit(-1);
  }

  for(int i = 0; i < count; i++) {
    if(!exeFile.read((char *)&bytes, 4)) {
      cerr << "error: could not read words from file" << endl;
      exit(-1);
    }
    mem[i] = swizzle(bytes);
  }
}
