/******************************
 * Submitted by: Colin Campbell c_c953
 * CS 3339 - Spring 2020, Texas State University
 * Project 2 Emulator
 * Copyright 2020, all rights reserved
 * Updated by Lee B. Hinkle based on prior work by Martin Burtscher and Molly O'Neil
 ******************************/

#include "CPU.h"
#include "Stats.h"
#include "CacheStats.h"
Stats stats;
CacheStats cacheStats;
const string CPU::regNames[] = {"$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
                                "$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
                                "$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
                                "$t8","$t9","$k0","$k1","$gp","$sp","$fp","$ra"};

CPU::CPU(uint32_t pc, Memory &iMem, Memory &dMem) : pc(pc), iMem(iMem), dMem(dMem) {
  for(int i = 0; i < NREGS; i++) {
    regFile[i] = 0;
  }
  hi = 0;
  lo = 0;
  regFile[28] = 0x10008000; // gp
  regFile[29] = 0x10000000 + dMem.getSize(); // sp

  instructions = 0;
  stop = false;
 }

void CPU::run() {
  while(!stop) {
    instructions++;

    fetch();
    decode();
    execute();
    mem();
    writeback();
    stats.clock();
    D(printRegFile());
  }
}

void CPU::fetch() {
  instr = iMem.loadWord(pc);
  pc = pc + 4;
}

/////////////////////////////////////////
// ALL YOUR CHANGES GO IN THIS FUNCTION
/////////////////////////////////////////
void CPU::decode() {
  uint32_t opcode;      // opcode field
  uint32_t rs, rt, rd;  // register specifiers
  uint32_t shamt;       // shift amount (R-type)
  uint32_t funct;       // funct field (R-type)
  uint32_t uimm;        // unsigned version of immediate (I-type)
  int32_t simm;         // signed version of immediate (I-type)
  uint32_t addr;        // jump address offset field (J-type)

  opcode = ( instr >> 26 );
  rs = ( instr >> 21 )  & 0x1f;
  rt = ( instr >> 16 ) & 0x1f;
  rd = ( instr >> 11 ) & 0x1f;
  shamt = ( instr >> 6 ) & 0x1f;
  funct = instr & 0x3f;
  uimm = instr & 0xffff;
  simm = ( (signed)instr << 16 ) >> 16;
  addr = instr & 0x3ffffff;

  // Hint: you probably want to give all the control signals some "safe"
  // default value here, and then override their values as necessary in each
  // case statement below!
  opIsLoad = false;
  opIsStore = false;
  opIsMultDiv = false;
  writeDest = false;
  destReg = REG_ZERO;
  storeData = regFile[REG_ZERO];
  aluSrc1 = regFile[REG_ZERO];
  aluSrc2 = regFile[REG_ZERO];

  D(cout << "  " << hex << setw(8) << pc - 4 << ": ");
  switch(opcode) {
    case 0x00:
      switch(funct) {
        case 0x00: D(cout << "sll " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt);
                   writeDest = true; destReg = rd; stats.registerDest(rd, MEM1);
                   aluOp = SHF_L;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs, EXE1);
                   aluSrc2 = shamt;
                   break; // use prototype above, not the greensheet
        case 0x03: D(cout << "sra " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt);
                   writeDest = true; destReg = rd; stats.registerDest(rd, MEM1);
                   aluOp = SHF_R;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs, EXE1);
                   aluSrc2 = shamt;
                   break; // use prototype above, not the greensheet
        case 0x08: D(cout << "jr " << regNames[rs]);
                   pc = regFile[rs]; stats.registerSrc(rs,ID); stats.flush(2);
                   break;
        case 0x10: D(cout << "mfhi " << regNames[rd]);
                   writeDest = true; destReg = rd; stats.registerDest(rd, MEM1);
                   aluOp = ADD;
                   aluSrc1 = hi; stats.registerSrc(REG_HILO, EXE1);
                   aluSrc2 = regFile[REG_ZERO];
                   break;
        case 0x12: D(cout << "mflo " << regNames[rd]);
                   writeDest = true; destReg = rd; stats.registerDest(rd, MEM1);
                   aluOp = ADD;
                   aluSrc1 = lo; stats.registerSrc(REG_HILO, EXE1);
                   aluSrc2 = regFile[REG_ZERO];
                   break;
        case 0x18: D(cout << "mult " << regNames[rs] << ", " << regNames[rt]);
                   opIsMultDiv = true; stats.registerDest(REG_HILO, WB);
                   aluOp = MUL;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs, EXE1);
                   aluSrc2 = regFile[rt]; stats.registerSrc(rt, EXE1);
                   break;
        case 0x1a: D(cout << "div " << regNames[rs] << ", " << regNames[rt]);
                   opIsMultDiv = true; stats.registerDest(REG_HILO, WB);
                   aluOp = DIV;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs, EXE1);
                   aluSrc2 = regFile[rt]; stats.registerSrc(rt, EXE1);
                   break;
        case 0x21: D(cout << "addu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                   writeDest = true; destReg = rd; stats.registerDest(rd, MEM1);
                   aluOp = ADD;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs, EXE1);
                   aluSrc2 = regFile[rt]; stats.registerSrc(rt, EXE1);
                   break;
        case 0x23: D(cout << "subu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                   writeDest = true; destReg = rd; stats.registerDest(rd, MEM1);
                   aluOp = ADD;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs, EXE1);
                   aluSrc2 = -regFile[rt]; stats.registerSrc(rt, EXE1);
                   break; //hint: subtract is the same as adding a negative
        case 0x2a: D(cout << "slt " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                   writeDest = true; destReg = rd; stats.registerDest(rd, MEM1);
                   aluOp = CMP_LT;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs, EXE1);
                   aluSrc2 = regFile[rt]; stats.registerSrc(rt, EXE1);
                   break;
        default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
      }
      break;
   case 0x02: D(cout << "j " << hex << ((pc & 0xf0000000) | addr << 2)); // P1: pc + 4
              pc = (pc & 0xf0000000) | addr << 2; stats.flush(2);
              break;
   case 0x03: D(cout << "jal " << hex << ((pc & 0xf0000000) | addr << 2)); // P1: pc + 4
              writeDest = true; destReg = REG_RA; stats.registerDest(REG_RA, EXE1); // writes PC+4 to $ra
              aluOp = ADD; // ALU should pass pc thru unchanged
              aluSrc1 = pc;
              aluSrc2 = regFile[REG_ZERO]; // always reads zero
              pc = (pc & 0xf0000000) | addr << 2; stats.flush(2);
              break;
   case 0x04: D(cout << "beq " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2));
              stats.countBranch(); stats.registerSrc(rs,ID); stats.registerSrc(rt,ID);
              if (regFile[rs] == regFile[rt]){
              pc = pc + (simm << 2);
              stats.countTaken();
              stats.flush(2);
              }
              break;  // read the handout carefully, update PC directly here as in jal example
   case 0x05: D(cout << "bne " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2));
              stats.countBranch(); stats.registerSrc(rs,ID); stats.registerSrc(rt,ID);
              if (regFile[rs] != regFile[rt]){
              pc = pc + (simm << 2);
              stats.countTaken();
              stats.flush(2);
              }
              break;  // same comment as beq
   case 0x09: D(cout << "addiu " << regNames[rt] << ", " << regNames[rs] << ", " << dec << simm);
              writeDest = true; destReg = rt; stats.registerDest(rt, MEM1);
              aluOp = ADD;
              aluSrc1 = regFile[rs]; stats.registerSrc(rs, EXE1);
              aluSrc2 = simm;
              break;
   case 0x0c: D(cout << "andi " << regNames[rt] << ", " << regNames[rs] << ", " << dec << uimm);
              writeDest = true; destReg = rt; stats.registerDest(rt, MEM1);      // write flag true, destination set to register rt
              aluOp = AND;                                                 // set aluOp to And instruction
              aluSrc1 = regFile[rs]; stats.registerSrc(rs, EXE1);                // operand #1 = value in register rs, check pipeline for dependencies
              aluSrc2 = uimm;                                              // operand #2 = unsigned immediate value
              break;
   case 0x0f: D(cout << "lui " << regNames[rt] << ", " << dec << simm);
               writeDest = true; destReg = rt; stats.registerDest(rt, MEM1);
               aluOp = SHF_L;
               aluSrc1 = simm;
               aluSrc2 = 16;
               break; //use the ALU to perform necessary op, you may set aluSrc2 = xx directly
   case 0x1a: D(cout << "trap " << hex << addr);
              switch(addr & 0xf) {
                case 0x0: cout << endl; break;
                case 0x1: cout << " " << (signed)regFile[rs];
                          stats.registerSrc(rs, EXE1);                          //check for dependencies as a source
                          break;
                case 0x5: cout << endl << "? "; cin >> regFile[rt];
                          stats.registerDest(rt, MEM1);                         //check for dependencies as a destination
                          break;
                case 0xa: stop = true; break;
                default: cerr << "unimplemented trap: pc = 0x" << hex << pc - 4 << endl;
                          stop = true;
              }
              break;
    case 0x23: D(cout << "lw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
              writeDest = true; destReg = rt; stats.registerDest(rt, WB);     // write flag true, destingation set to register rt
              opIsLoad = true;                                            // load flag set true, load into rt
              aluOp = ADD;                                                // set aluOp to Add instruction
              aluSrc1 = regFile[rs]; stats.registerSrc(rs, EXE1);               // register value that we're loading into rt,
                                                                          // check pipeline for dependencies
              aluSrc2 = simm;                                             // offset to calculate
              stats.countMemOp();                                         // increment the number of memory instructions
              break;  // do not interact with memory here - setup control signals for mem()

    case 0x2b: D(cout << "sw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
              storeData = regFile[rt];                  // storing data from register rs into rt
              opIsStore = true;                         // store flag set to true
              aluOp = ADD;
              aluSrc1 = regFile[rs];
              stats.registerSrc(rs, EXE1);                    // check pipeline for dependencies
              stats.registerSrc(rt, MEM1);                    // check pipeline for dependencies
              stats.countMemOp();                       // increment the number of memory instructions
              aluSrc2 = simm;
              break;  // same comment as lw

    default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;

  }
  D(cout << endl);
}

void CPU::execute() {
  aluOut = alu.op(aluOp, aluSrc1, aluSrc2);
}

void CPU::mem() {
  if(opIsLoad){
    writeData = dMem.loadWord(aluOut);
    int stalls = cacheStats.access(aluOut, LOAD);
    stats.setStalls(stalls);  
}
  else
    writeData = aluOut;

  if(opIsStore){
    dMem.storeWord(storeData, aluOut);
    int stalls = cacheStats.access(aluOut, STORE);
    stats.setStalls(stalls);   
  }
}

void CPU::writeback() {
  if(writeDest && destReg > 0) // skip when write is to zero_register
    regFile[destReg] = writeData;

  if(opIsMultDiv) {
    hi = alu.getUpper();
    lo = alu.getLower();
  }
}

void CPU::printRegFile() {
  cout << hex;
  for(int i = 0; i < NREGS; i++) {
    cout << "    " << regNames[i];
    if(i > 0) cout << "  ";
    cout << ": " << setfill('0') << setw(8) << regFile[i];
    if( i == (NREGS - 1) || (i + 1) % 4 == 0 )
      cout << endl;
  }
  cout << "    hi   : " << setfill('0') << setw(8) << hi;
  cout << "    lo   : " << setfill('0') << setw(8) << lo;
  cout << dec << endl;
}

void CPU::printFinalStats() {
 cout << "Program finished at pc = 0x" << hex << pc << "  ("
      << dec << instructions << " instructions executed)" << endl << endl;

  cout << "Cycles: " << stats.getCycles() << endl
       << "CPI: " << fixed << setprecision(2) << (float)stats.getCycles()/instructions << endl << endl

       << "Bubbles: " << stats.getBubbles() << endl
       << "Flushes: " << stats.getFlushes() << endl
       << "Stalls: " << stats.getStalls() << endl << endl;

       cacheStats.printFinalStats();
 
}
