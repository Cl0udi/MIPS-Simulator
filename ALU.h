#ifndef __ALU_H
#define __ALU_H

#include <iostream>
#include <cstdint>
#include <iomanip>
#include <cstdlib>
#include "Debug.h"
using namespace std;

enum ALU_OP { ADD, AND, SHF_L, SHF_R, CMP_LT, MUL, DIV };

class ALU {
  private:
    uint32_t upper, lower;

  public:
    uint32_t op(ALU_OP op, uint32_t src1, uint32_t src2);
    uint32_t getUpper() const { return upper; }
    uint32_t getLower() const { return lower; }
};

#endif
