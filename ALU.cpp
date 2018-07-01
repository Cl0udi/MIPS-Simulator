#include "ALU.h"

uint32_t ALU::op(ALU_OP op, uint32_t src1, uint32_t src2) {
  switch(op) {
    case ADD   : return (signed)src1 + (signed)src2;
    case AND   : return src1 & src2;
    case SHF_L : return src1 << src2;
    case SHF_R : return (signed)src1 >> src2;
    case CMP_LT: return ((signed)src1 < (signed)src2) ? 1 : 0;
    case MUL   : {
                   uint64_t wide = (uint64_t)src1 * (uint64_t)src2;
                   lower = wide & 0xffffffff;
                   upper = wide >> 32;
                 }
                 break;
    case DIV   : if(src2 == 0) {
                   cerr << "division by zero!" << endl;
                   exit(-1);
                 }
                 lower = src1 / src2;
                 upper = src1 % src2;
                 break;
    default: cerr << "unimplemented ALU operation: op = " << dec << op << endl;
  }
  return 0;
}
