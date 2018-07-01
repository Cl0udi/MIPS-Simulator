/******************************
 * Alfonso de la Morena
 * CS 3339 - Spring 2018
 ******************************/
#include "Stats.h"

Stats::Stats() {
  cycles = PIPESTAGES - 1; // pipeline startup cost
  flushes = 0;
  bubbles = 0;

  memops = 0;
  branches = 0;
  taken = 0;

  for(int i = IF1; i < PIPESTAGES; i++) {
    resultReg[i] = -1;
    availabilityReg[i] = -1;
  }

  raw_hazards  = 0;
  exe1_hazards = 0;
  exe2_hazards = 0;
  mem1_hazards = 0;
  mem2_hazards = 0;

}

void Stats::clock() {
  cycles++;

  // advance all pipeline flops
  for(int i = WB; i > IF1; i--) {
    resultReg[i] = resultReg[i-1];
    availabilityReg[i] = availabilityReg[i-1];
  }
  // inject no-op into IF1
  resultReg[IF1] = -1;
  availabilityReg[IF1] = -1;

}

  //*************************************************************************
  // PIPESTAGE { IF1 = 0, IF2 = 1, ID = 2, EXE1 = 3, EXE2 = 4, MEM1 = 5,
  //                 MEM2 = 6, WB = 7, PIPESTAGES = 8 };

void Stats::registerSrc(int r, int needed) { // REG.READ && MEM.READ

    if(r == 0){return;}

    int bubbles = 0;
    int hazard_index = 0; // For determining when hazards occur
    int longest_wait = -1; // Latest stage in pipeline we have to wait for
    int wait = 0; // For looping and comparing to longest_wait
    bool hazard_check = false;

    // Determine the hazard index
    for(int i = EXE1; i < PIPESTAGES; i++){
        if(resultReg[i] == r){
                hazard_check = true;
                wait = availabilityReg[i] - i;
                if(wait > longest_wait){
                    longest_wait = wait;
                    hazard_index = i;
                }
        }
    }

    // Calculate bubbles
    bubbles = longest_wait - needed + ID;

    // Classify hazards
    if(hazard_check){

            raw_hazards++;

            // Determine when the hazard occurred.
            switch(hazard_index){

                case MEM2:  mem2_hazards++; break;

                case MEM1:  mem1_hazards++; break;

                case EXE2:  exe2_hazards++; break;

                case EXE1:  exe1_hazards++; break;

                default:    mem2_hazards++; break;
            }
    }

    for(int i = 0; i < bubbles; i++){bubble(r);} // Bubble pipeline
}

void Stats::registerDest(int rd, int ready) { // REG.WRITE && MEM.WRITW

    resultReg[ID] = rd; // Keeps track of pipeline
    availabilityReg[ID] = ready; // Keeps track of when data is available

}

void Stats::flush(int count) { // count == how many ops to flush
    //flushes = cycles - 7 - bubbles - count;
    for(int i = 0; i < count; i++){clock(); flushes++;}
}

void Stats::bubble(int r) {

    bubbles++;
    cycles++;

    for(int i = WB; i > EXE1; i--) {
        resultReg[i] = resultReg[i-1];
        availabilityReg[i] = availabilityReg[i-1];
    }

    resultReg[EXE1] = -1;
    availabilityReg[EXE1] = -1;

}

void Stats::printPipeline(){
    for(int i = IF1; i < PIPESTAGES; i++) {
    cout << availabilityReg[i] << " ";
    }
    cout << endl;
}

// Empty the pipeline at the end of the program.
void Stats::emptyPipeline(){

    for(int i = 2; i < PIPESTAGES; i++){
        if(resultReg[i] != -1){clock(); i = 2;}
    }
}

  //*************************************************************************
