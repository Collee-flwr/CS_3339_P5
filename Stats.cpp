/******************************
 * Submitted by: enter your first and last name and net ID
 * CS 3339 - Spring 2020, Texas State University
 * Project 3 Pipelining
 * Copyright 2020, all rights reserved
 * Updated by Lee B. Hinkle based on prior work by Martin Burtscher and Molly O'Neil
 ******************************/
 
#include "Stats.h"

Stats::Stats() {
  cycles = PIPESTAGES - 1; // pipeline startup cost
  flushes = 0;
  bubbles = 0;
  stalls = 0;

  memops = 0;
  branches = 0;
  taken = 0;
 
  for(int i = IF1; i < PIPESTAGES; i++) {
    resultReg[i] = -1;
    resultStage[i] = 0;
    hazardTracking[i] = 0;
  }
}

void Stats::clock() {
  cycles++;

  // advance all pipeline flip-flops
  for(int i = WB; i > IF1; i--) {
    resultReg[i] = resultReg[i-1];
    resultStage[i] = resultStage[i-1];
  }
  // inject a no-op in IF1
  resultReg[IF1] = -1;
  resultStage[IF1] = 0;
}

void Stats::registerSrc(int r, PIPESTAGE needed) {

  if (r == 0) return;
  else{
        for (int i = EXE1; i < WB; i++){
           if (resultReg[i] == r){ 
                hazardTracking[i]++;
	     int distValid = resultStage[i] - (i - 1);
             int distNeeded =(int)(needed - ID) + 1;
             for (int bubbles = 0; bubbles < (distValid - distNeeded); ++bubbles)
                 bubble();
             break;
           }
        }
  }
}
  


void Stats::registerDest(int r, PIPESTAGE valid) {
 resultReg[ID] = r;
 resultStage[ID] = valid; 
}

void Stats::flush(int count) { // count == how many ops to flush 
  for (int i = 0; i < count; i++){
      flushes++;
      clock();       
  }
}

void Stats::bubble() {
  bubbles++;
  cycles++;
  for (int i = WB; i > EXE1; i--){
    resultReg[i] = resultReg[i-1];
    resultStage[i] = resultStage[i-1];
  }
  resultReg[EXE1] = -1;
  resultStage[EXE1] = 0; 
}

void Stats::showPipe() {
  // this method is to assist testing and debug, please do not delete or edit
  // you are welcome to use it but remove any debug outputs before you submit
  cout << "              IF1  IF2 *ID* EXE1 EXE2 MEM1 MEM2 WB         #C      #B      #F" << endl; 
  cout << "  resultReg ";
  for(int i = 0; i < PIPESTAGES; i++) {
    cout << "  " << dec << setw(2) << resultReg[i] << " ";
  }
  cout << "   " << setw(7) << cycles << " " << setw(7) << bubbles << " " << setw(7) << flushes;
  cout << endl;
}
