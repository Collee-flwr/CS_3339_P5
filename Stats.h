#ifndef __STATS_H
#define __STATS_H
#include <iostream>
#include <iomanip>
#include "Debug.h"
using namespace std;

enum PIPESTAGE { IF1 = 0, IF2 = 1, ID = 2, EXE1 = 3, EXE2 = 4, MEM1 = 5, 
                 MEM2 = 6, WB = 7, PIPESTAGES = 8 };

class Stats {
  private:
    long long cycles;
    int flushes;
    int bubbles;
    int stalls;
    int memops;
    int branches;
    int taken;

    int resultReg[PIPESTAGES];
    int resultStage[PIPESTAGES];
    int hazardTracking[PIPESTAGES];

  public:
    Stats();

    void clock();

    void flush(int count);

    void registerSrc(int r, PIPESTAGE needed);
    void registerDest(int r, PIPESTAGE valid);

    void countMemOp() { memops++; }
    void countBranch() { branches++; }
    void countTaken() { taken++; }
	
    void showPipe();
    int setStalls(int s){stalls += s; cycles += s;}
    // getters
    long long getCycles() { return cycles; }
    int getFlushes() { return flushes; }
    int getBubbles() { return bubbles; }
    int getMemOps() { return memops; }
    int getBranches() { return branches; }
    int getTaken() { return taken; }
    int getStalls(){ return stalls;}
    
    int getExe1Hazards(){ return hazardTracking[EXE1]; }
    int getExe2Hazards(){ return hazardTracking[EXE2]; }
    int getMem1Hazards(){ return hazardTracking[MEM1]; }
    int getMem2Hazards(){ return hazardTracking[MEM2]; }
    int getTotalHazards(){ return hazardTracking[EXE1] + hazardTracking[EXE2] + hazardTracking[MEM1] + hazardTracking[MEM2]; }

  private:
    void bubble();
};

#endif
