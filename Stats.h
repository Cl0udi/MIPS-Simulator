#ifndef __STATS_H
#define __STATS_H
#include <iostream>
#include "Debug.h"
using namespace std;

enum PIPESTAGE { IF1 = 0, IF2 = 1, ID = 2, EXE1 = 3, EXE2 = 4, MEM1 = 5,
                 MEM2 = 6, WB = 7, PIPESTAGES = 8 };

class Stats {
  private:
    long long cycles;
    int flushes;
    int bubbles;

    int memops;
    int branches;
    int taken;

    int resultReg[PIPESTAGES];

    // My code ****************************************************************
    int availabilityReg[PIPESTAGES];
    // My code ****************************************************************

  public:
    Stats();

    void clock();

    void flush(int count);

    void registerSrc(int r, int needed);
    void registerDest(int r, int ready);

    void countMemOp() { memops++; }
    void countBranch() { branches++; }
    void countTaken() { taken++; }

    // getters
    long long getCycles() { return cycles; }
    int getFlushes() { return flushes; }
    int getBubbles() { return bubbles; }
    int getMemOps() { return memops; }
    int getBranches() { return branches; }
    int getTaken() { return taken; }

    // Alfonso's Code:
    void emptyPipeline();
    void printPipeline();

    // My code ****************************************************************
    int raw_hazards;
    int exe1_hazards;
    int exe2_hazards;
    int mem1_hazards;
    int mem2_hazards;
    // My code ****************************************************************

  private:
    void bubble(int r);
};

#endif
