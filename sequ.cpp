#include "util.hpp"

unsigned R[32];
int pc = 0;

void fuckaround(inst i) {
  unsigned tmp;
  switch (i.op) {
  case LUI:
    R[i.rd] = i.imm << 12;
  break;
  case AUIPC:
    R[i.rd] = pc + (i.imm << 12);
  break;
  case JAL:
    R[i.rd] = pc + 4;
    pc += i.imm - 4;
  break;
  case JALR:
    tmp = pc + 4;
    pc = ((R[i.rs1] + i.imm) & ~1) - 4;
    R[i.rd] = tmp;
  break;
  case BEQ:
    if(R[i.rs1] == R[i.rs2]) pc += i.imm - 4;
  break;
  case BNE:
    if(R[i.rs1] != R[i.rs2]) pc += i.imm - 4;
  break;
  case BLT:
    if(static_cast<int>(R[i.rs1]) < static_cast<int>(R[i.rs2])) pc += i.imm - 4;
  break;
  case BGE:
    if(static_cast<int>(R[i.rs1]) >= static_cast<int>(R[i.rs2])) pc += i.imm - 4;
  break;
  case BLTU:
    if(R[i.rs1] < R[i.rs2]) pc += i.imm - 4;
  break;
  case BGEU:
    if(R[i.rs1] >= R[i.rs2]) pc += i.imm - 4;
  break;
  case LB:
    R[i.rd] = sext(mem(R[i.rs1] + i.imm, 1), 8);
  break;
  case LH:
    R[i.rd] = sext(mem(R[i.rs1] + i.imm, 2), 16);
  break;
  case LW:
    R[i.rd] = sext(mem(R[i.rs1] + i.imm, 4), 32);
  break;
  case LBU:
    R[i.rd] = mem(R[i.rs1] + i.imm, 1);
  break;
  case LHU:
    R[i.rd] = mem(R[i.rs1] + i.imm, 2);
  break;
  case SB:
    write(R[i.rs2], R[i.rs1] + i.imm, 1);
  break;
  case SH:
    write(R[i.rs2], R[i.rs1] + i.imm, 2);
  break;
  case SW:
    write(R[i.rs2], R[i.rs1] + i.imm, 4);
  break;
  case ADDI:
    R[i.rd] = R[i.rs1] + i.imm;
  break;
  case SLTI:
    R[i.rd] = static_cast<int>(R[i.rs1]) < static_cast<int>(i.imm);
  break;
  case SLTIU:
    R[i.rd] = static_cast<unsigned>(R[i.rs1]) < static_cast<unsigned>(i.imm);
  break;
  case XORI:
    R[i.rd] = R[i.rs1] ^ i.imm;
  break;
  case ORI:
    R[i.rd] = R[i.rs1] | i.imm;
  break;
  case ANDI:
    R[i.rd] = R[i.rs1] & i.imm;
  break;
  case SLLI:
    R[i.rd] = R[i.rs1] << i.imm;
  break;
  case SRLI:
    R[i.rd] = R[i.rs1] >> i.imm;
  break;
  case SRAI:
    R[i.rd] = sext(R[i.rs1] >> i.imm, 32 - i.imm);
  break;
  case ADD:
    R[i.rd] = R[i.rs1] + R[i.rs2];
  break;
  case SUB:
    R[i.rd] = R[i.rs1] - R[i.rs2];
  break;
  case SLL:
    R[i.rd] = R[i.rs1] << R[i.rs2];
  break;
  case SLT:
    R[i.rd] = static_cast<int>(R[i.rs1]) < static_cast<int>(R[i.rs2]);
  break;
  case SLTU:
    R[i.rd] = R[i.rs1] < R[i.rs2];
  break;
  case XOR:
    R[i.rd] = R[i.rs1] ^ R[i.rs2];
  break;
  case SRL:
    R[i.rd] = R[i.rs1] >> R[i.rs2];
  break;
  case SRA:
    R[i.rd] = sext(R[i.rs1] >> R[i.rs2], 32 - R[i.rs2]);
  break;
  case OR:
    R[i.rd] = R[i.rs1] | R[i.rs2];
  break;
  case AND:
    R[i.rd] = R[i.rs1] & R[i.rs2];
  break;
  }
  pc += 4;
}

int main() {
  input();
  unsigned clock = 0;
  while(clock < 0x10000000) {
    std::cout << std::hex << pc << std::endl;
    for(int i = 0; i<=16; ++i) std::cout << "|" << R[i];
    std::cout<< std::endl;
    ++clock;
    if(mem(pc) == 0x0ff00513) break;
    inst i = decode(mem(pc));
    if(i.op == nil) {std::cerr << std::hex << "WTF" << mem(pc) << std::endl; break;}
    fuckaround(i);
    R[0] = 0;
  }
  std::cout << (R[10] & 0xFF) << std::endl;
  return 0;
}