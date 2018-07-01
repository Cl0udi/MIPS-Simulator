#ifndef __CPU_H
#define __CPU_H

#include <iostream>
#include <cstdint>
#include <string>
#include <iomanip>
#include <cstdlib>
#include "Memory.h"
#include "ALU.h"
#include "Debug.h"
#include "Stats.h"
using namespace std;

class CPU {
  private:
    static const int NREGS = 32;
    static const string regNames[];

    static const int REG_ZERO = 0;
    static const int REG_RA = 31;
    static const int REG_HILO = 32;

    uint32_t pc;
    uint32_t instr;

    // Register file
    uint32_t regFile[NREGS];
    uint32_t hi, lo;

    ALU alu;

    Memory &iMem;
    Memory &dMem;

    //*************************************************************************
    Stats stats;
    int test_array[20] ={0};
    enum PIPESTAGE { IF1 = 0, IF2 = 1, ID = 2, EXE1 = 3, EXE2 = 4, MEM1 = 5,
                 MEM2 = 6, WB = 7, PIPESTAGES = 8 };
    //*************************************************************************

    int instructions;
    bool stop;

    // Control signals
    bool opIsLoad, opIsStore, opIsMultDiv;
    ALU_OP aluOp;
    bool writeDest;
    int destReg;
    uint32_t aluSrc1, aluSrc2;
    uint32_t storeData;

    // Wires between stages
    uint32_t aluOut;
    uint32_t writeData;

  public:
    CPU(uint32_t pc, Memory &iMem, Memory &dMem);

    void run();
    void printFinalStats();

  private:
    void fetch();
    void decode();
    void execute();
    void mem();
    void writeback();

    void printRegFile();
};

#endif
