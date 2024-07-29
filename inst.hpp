#ifndef INST_HPP
#define INST_HPP
#include <iostream>

enum inst_type {
  nil = -1,
  LUI, AUIPC,
  JAL, JALR, BEQ, BNE, BLT, BGE, BLTU, BGEU,
  LB, LH, LW, LBU, LHU,
  SB, SH, SW,
  ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI, 
  ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND
};

struct inst {
  inst_type op;
  int imm;
  unsigned rs1, rs2, rd, shamt;
  bool pred_jp, real_jp;
  inst() {
    pred_jp = true;
    real_jp = false;
  }
};

#endif
