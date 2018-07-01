/******************************
 * Alfonso de la Morena
 * CS 3339 - Spring 2018
 ******************************/
#include "CPU.h"

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

    //*************************************************************************
    stats.clock();
    //*************************************************************************

    fetch();
    decode();
    execute();
    mem();
    writeback();

    D(printRegFile());
  }
}

void CPU::fetch() {
  instr = iMem.loadWord(pc);
  pc = pc + 4;
}

// Selects the bits we want from the instruction.
uint32_t selectBits(uint32_t input, int left, int right)
{
    // Special Case #1
    // Needed because our mask cannot be more than 32 bits.
    if(left == 31 && right == 0){return input;}

    // Set a line of 1 up to the left most bit.
    uint32_t mask = (1 << (1 + left - right)) - 1;

    // Shift to the right by to exclude bits to the right of our
    // rightmost value.
    return (input >> right) & mask;
}

// Convert to 16 bit signed and return the value
int32_t unsignedToSigned(int32_t input)
{
    int16_t test = input;
    return test;
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

  opcode = selectBits(instr, 31, 26);
  rs     = selectBits(instr, 25, 21);
  rt     = selectBits(instr, 20, 16);
  rd     = selectBits(instr, 15, 11);
  shamt  = selectBits(instr, 10, 6);
  funct  = selectBits(instr,  5,  0);
  uimm   = selectBits(instr, 15,  0);
  simm   = unsignedToSigned(uimm);
  addr   = selectBits(instr, 25, 0);

  // Hint: you probably want to give all the control signals some "safe"
  // default value here, and then override their values as necessary in each
  // case statement below!
  aluOp = ADD;
  aluSrc1 = pc;
  aluSrc2 = regFile[REG_ZERO];
  opIsLoad = false;
  opIsStore = false;
  opIsMultDiv = false;
  writeDest = false;
  destReg = rd;
  storeData = 0;


  D(cout << "  " << hex << setw(8) << pc - 4 << ": ");
  switch(opcode) {
    case 0x00:
      switch(funct) {
                   // Shift left logical
        case 0x00: D(cout << "sll " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt);
                   writeDest = true; destReg = rd; stats.registerDest(rd, MEM1);
                   aluOp = SHF_L;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs, EXE1);
                   aluSrc2 = shamt;
                   //test_array[0]++;
                   break;
                   // Shift Right Arithmetic
        case 0x03: D(cout << "sra " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt);
                   writeDest = true; destReg = rd; stats.registerDest(rd, MEM1);
                   aluOp = SHF_R;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs, EXE1);
                   aluSrc2 = shamt;
                   //test_array[1]++;
                   break;
                   // Jump register
        case 0x08: D(cout << "jr " << regNames[rs]);
                   aluOp = ADD;
                   aluSrc1 = pc;
                   aluSrc2 = regFile[REG_ZERO];
                   pc = regFile[rs]; stats.registerSrc(rs, ID);
                   stats.flush(2);
                   //test_array[2]++;
                   break;
                   // Move from high
        case 0x10: D(cout << "mfhi " << regNames[rd]);
                   writeDest = true; destReg = rd; stats.registerDest(rd, MEM1);
                   aluOp = ADD;
                   aluSrc1 = hi; stats.registerSrc(REG_HILO, EXE1);
                   aluSrc2 = regFile[REG_ZERO];
                   //test_array[3]++;
                   break;
                   // Move from low
        case 0x12: D(cout << "mflo " << regNames[rd]);
                   writeDest = true; destReg = rd; stats.registerDest(rd, MEM1);
                   aluOp = ADD;
                   aluSrc1 = lo; stats.registerSrc(REG_HILO, EXE1);
                   aluSrc2 = regFile[REG_ZERO];
                   //test_array[4]++;
                   break;
                   // Multiply
        case 0x18: D(cout << "mult " << regNames[rs] << ", " << regNames[rt]);
                   opIsMultDiv = true; stats.registerDest(REG_HILO, WB);
                   aluOp = MUL;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs, EXE1);
                   aluSrc2 = regFile[rt]; stats.registerSrc(rt, EXE1);
                   //test_array[5]++;
                   break;
                   // Divide
        case 0x1a: D(cout << "div " << regNames[rs] << ", " << regNames[rt]);
                   opIsMultDiv = true; stats.registerDest(REG_HILO, WB);
                   aluOp = DIV;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs, EXE1);
                   aluSrc2 = regFile[rt]; stats.registerSrc(rt, EXE1);
                   //test_array[6]++;
                   break;
                   // Add Unsigned
        case 0x21: D(cout << "addu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                   writeDest = true; destReg = rd; stats.registerDest(rd, MEM1);
                   aluOp = ADD;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs, EXE1);
                   aluSrc2 = regFile[rt]; stats.registerSrc(rt, EXE1);
                   //test_array[7]++;
                   break;
                   //Subtract Unsigned
        case 0x23: D(cout << "subu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                   writeDest = true; destReg = rd; stats.registerDest(rd, MEM1);
                   aluOp = ADD;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs, EXE1);
                   aluSrc2 = -regFile[rt]; stats.registerSrc(rt, EXE1);
                   //test_array[8]++;
                   break;
                   // Set less than
        case 0x2a: D(cout << "slt " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                   writeDest = true; destReg = rd; stats.registerDest(rd, MEM1);
                   aluOp = CMP_LT;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs, EXE1);
                   aluSrc2 = regFile[rt]; stats.registerSrc(rt, EXE1);
                   //test_array[9]++;
                   break;
        default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
      }
      break;
               // Jump
    case 0x02: D(cout << "j " << hex << ((pc & 0xf0000000) | addr << 2)); // P1: pc + 4
               aluOp = ADD; // ALU should pass pc thru unchanged
               aluSrc1 = pc;
               aluSrc2 = regFile[REG_ZERO]; // always reads zero
               pc = (pc & 0xf0000000) | addr << 2;
               stats.flush(2);
               //test_array[10]++;
               break;
               // Jump and link
    case 0x03: D(cout << "jal " << hex << ((pc & 0xf0000000) | addr << 2)); // P1: pc + 4
               writeDest = true; destReg = REG_RA; stats.registerDest(REG_RA, EXE1);
               aluOp = ADD;
               aluSrc1 = pc;
               aluSrc2 = regFile[REG_ZERO];
               pc = (pc & 0xf0000000) | addr << 2;
               stats.flush(2);
               //test_array[11]++;
               break;
               // Branch on equal
    case 0x04: D(cout << "beq " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2));
               stats.countBranch();
               stats.registerSrc(rs, ID); stats.registerSrc(rt, ID);
               if(regFile[rs] == regFile[rt]){
                   aluOp = ADD; // ALU should pass pc thru unchanged
                   aluSrc1 = pc;
                   aluSrc2 = regFile[REG_ZERO]; // always reads zero
                   pc = pc + (simm << 2);
                   stats.countTaken();
                   stats.flush(2);
               }
               //test_array[12]++;
               break;
               // Branch on not equal
    case 0x05: D(cout << "bne " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2));
               stats.countBranch();
               stats.registerSrc(rs, ID); stats.registerSrc(rt, ID);
               if(regFile[rs] != regFile[rt]){
                   aluOp = ADD; // ALU should pass pc thru unchanged
                   aluSrc1 = pc;
                   aluSrc2 = regFile[REG_ZERO]; // always reads zero
                   pc = pc + (simm << 2);
                   stats.countTaken();
                   stats.flush(2);
               }
               //test_array[13]++;
               break;
               // Add immediate unsigned
    case 0x09: D(cout << "addiu " << regNames[rt] << ", " << regNames[rs] << ", " << dec << simm);
               writeDest = true; destReg = rt; stats.registerDest(rt, MEM1);
               aluOp = ADD;
               aluSrc1 = regFile[rs]; stats.registerSrc(rs, EXE1);
               aluSrc2 = simm;
               //test_array[14]++;
               break;
               // And immediate
    case 0x0c: D(cout << "andi " << regNames[rt] << ", " << regNames[rs] << ", " << dec << uimm);
               writeDest = true; destReg = rt; stats.registerDest(rt, MEM1);
               aluOp = AND;
               aluSrc1 = regFile[rs]; stats.registerSrc(rs, EXE1);
               aluSrc2 = uimm;
               //test_array[15]++;
               break;
               // Load upper limit
    case 0x0f: D(cout << "lui " << regNames[rt] << ", " << dec << simm);
               writeDest = true; destReg = rt; stats.registerDest(rt, MEM1);
               aluOp = ADD;
               aluSrc1 = (simm << 16);
               aluSrc2 = regFile[REG_ZERO];
               //test_array[16]++;
               break;
               // Trap??
    case 0x1a: D(cout << "trap " << hex << addr);
               switch(addr & 0xf) {
                 case 0x0: cout << endl; break;
                 case 0x1: cout << " " << (signed)regFile[rs]; stats.registerSrc(rs, EXE1);
                           break;
                 case 0x5: cout << endl << "? "; cin >> regFile[rt]; stats.registerDest(rt, MEM1);
                           break;
                 case 0xa: stop = true; break;
                 default: cerr << "unimplemented trap: pc = 0x" << hex << pc - 4 << endl;
                          //stats.flush(2);
                          stop = true;
               }
               //test_array[17]++;
               break;
               // Load word
    case 0x23: D(cout << "lw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
               stats.countMemOp();
               writeDest = true; destReg = rt; stats.registerDest(rt, WB);
               opIsLoad = true;
               aluOp = ADD;
               aluSrc1 = regFile[rs]; stats.registerSrc(rs, EXE1);
               aluSrc2 = simm;
               //test_array[18]++;
               break;
               // Store word
    case 0x2b: D(cout << "sw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
               stats.countMemOp(); stats.registerSrc(rs, EXE1);
               storeData = regFile[rt]; stats.registerSrc(rt, MEM1);
               opIsStore = true;
               aluOp = ADD;
               aluSrc1 = regFile[rs];
               aluSrc2 = simm;
               //test_array[19]++;
               break;
    default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
  }
  D(cout << endl);
}

void CPU::execute() {
  aluOut = alu.op(aluOp, aluSrc1, aluSrc2);
}

void CPU::mem() {
  if(opIsLoad)
    writeData = dMem.loadWord(aluOut);
  else
    writeData = aluOut;

  if(opIsStore)
    dMem.storeWord(storeData, aluOut);
}

void CPU::writeback() {
  if(writeDest && destReg > 0) // skip if write is to reg 0
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
       << dec << instructions << " instructions executed)" << endl;

  //*************************************************************************

  // My code will always be enclosed in asterisks to ease debugging.

  cout << endl;

  // Cycles
  cout << "Cycles: " << stats.getCycles() << endl;

  // CPI
  cout << "CPI: "    << fixed << setprecision(2)
       << (1.0 * stats.getCycles()) / instructions << endl << endl;

  // Bubbles
  cout << "Bubbles: " << stats.getBubbles() << endl;

  // Flushes
  //stats.flush(instructions);
  cout << "Flushes: " << stats.getFlushes() << endl << endl;

  // Raw hazards
  cout << "RAW hazards: " << stats.raw_hazards << " (1 per every "
       << fixed << setprecision(2)
       << ((1.00 * instructions) / stats.raw_hazards)
       << " instructions)"
       << endl;

  // EXE1 Hazards
  cout << "  On EXE1 op: "
       << stats.exe1_hazards
       << " ("
       << (stats.exe1_hazards * 100) / stats.raw_hazards
       << "%)"
       << endl;

  // EXE2 Hazards
  cout << "  On EXE2 op: "
       << stats.exe2_hazards
       << " ("
       << (stats.exe2_hazards * 100) / stats.raw_hazards
       << "%)"
       << endl;

  // MEM1 Hazards
  cout << "  On MEM1 op: "
       << stats.mem1_hazards
       << " ("
       << (stats.mem1_hazards * 100) / stats.raw_hazards
       << "%)"
       << endl;

  // MEM2 Hazards
  cout << "  On MEM2 op: "
       << stats.mem2_hazards
       << " ("
       << (stats.mem2_hazards * 100) / stats.raw_hazards
       << "%)"
       << endl;

 // For debugging
 //cout  << "Test writes: " << stats.test_writes << endl;

 //cout << "Debug: \n";
 //for(int i = 0; i < 20; i++){
    //cout << "Instruction " << i + 1 << ": " << test_array[i] << endl;
 }

  //*************************************************************************
