/******************************
 * Submitted by: enter your first and last name and net ID
 * CS 3339 - Spring 2020, Texas State University
 * Project 5 Data Cache
 * Copyright 2020, all rights reserved
 * Updated by Lee B. Hinkle based on prior work by Martin Burtscher and Molly O'Neil
 ******************************/
#include <iostream>
#include <cstdlib>
#include <iomanip>
#include "CacheStats.h"
using namespace std;

CacheStats::CacheStats() {
  cout << "Cache Config: ";
  if(!CACHE_EN) {
    cout << "cache disabled" << endl;
  } else {
    cout << (SETS * WAYS * BLOCKSIZE) << " B (";
    cout << BLOCKSIZE << " bytes/block, " << SETS << " sets, " << WAYS << " ways)" << endl;
    cout << "  Latencies: Lookup = " << LOOKUP_LATENCY << " cycles, ";
    cout << "Read = " << READ_LATENCY << " cycles, ";
    cout << "Write = " << WRITE_LATENCY << " cycles" << endl;
  }

  loads = 0;
  stores = 0;
  load_misses = 0;
  store_misses = 0;
  writebacks = 0;

  /* TODO: your code to initialize your datastructures here */

}

int CacheStats::access(uint32_t addr, ACCESS_TYPE type) {
  if(!CACHE_EN) { // cache is disabled
    return (type == LOAD) ? READ_LATENCY : WRITE_LATENCY;
  }

  if (type == LOAD) loads++;
  else stores++;

  addr = addr >> 3;
  addr = addr >> 2;
  uint32_t index = addr & 0x7;
  uint32_t tag = addr >> 3;
  int penalty = 0;

  for (int i = 0; i < WAYS; i++){
      if (dataCache[index][i].validBit && (dataCache[index][i].tag == tag))
      {
            if (type == STORE)
                dataCache[index][i].dirtyBit = true;

            return penalty;
      }
  }


   int oldestBlock = roundRobin[index] % 4;
   if(dataCache[index][oldestBlock].dirtyBit)
   {
       writebacks++;
       penalty += WRITE_LATENCY;
   }

   penalty += READ_LATENCY;

   if (type == LOAD)
   {
       load_misses++;
       dataCache[index][oldestBlock].dirtyBit = false;
   }
   else{
      store_misses++;
      dataCache[index][oldestBlock].dirtyBit = true;
   }

   dataCache[index][oldestBlock].tag = tag;
   dataCache[index][oldestBlock].validBit = true;
   roundRobin[index]++;


   return penalty;

}
void CacheStats::printFinalStats() {
  /* TODO: your code here "drain" the cache of writebacks */
  for (int i = 0; i < SETS;i++)
    for (int j = 0; j < WAYS; j++)
       if(dataCache[i][j].dirtyBit == true)
           writebacks++;

  int accesses = loads + stores;
  int misses = load_misses + store_misses;
  cout << "Accesses: " << accesses << endl;
  cout << "  Loads: " << loads << endl;
  cout << "  Stores: " << stores << endl;
  cout << "Misses: " << misses << endl;
  cout << "  Load misses: " << load_misses << endl;
  cout << "  Store misses: " << store_misses << endl;
  cout << "Writebacks: " << writebacks << endl;
  cout << "Hit Ratio: " << fixed << setprecision(1) << 100.0 * (accesses - misses) / accesses;
  cout << "%" << endl;
}
