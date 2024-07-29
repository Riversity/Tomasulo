#ifndef UTIL_HPP
#define UTIL_HPP
#include <iostream>
#include <iomanip>
#include "inst.hpp"

unsigned char M[0x200000];

void input() {
  std::cin >> std::hex;
  unsigned pos = 0;
  unsigned val;
  while(std::cin) {
    char c = std::cin.get();
    while(isspace(c)) c = std::cin.get();
    if(c == '@') {
      std::cin >> pos;
    }
    else if(isalnum(c)) {
      std::cin.unget();
      std::cin >> val; M[pos] = val;
      std::cin >> val; M[pos + 1] = val;
      std::cin >> val; M[pos + 2] = val;
      std::cin >> val; M[pos + 3] = val;
      pos += 4;
    }
  }
}

unsigned mem(unsigned pos, unsigned len = 4) {
  // extracting a word from bytes
  unsigned ans = 0;
  for(int i = 0; i < len; ++i) {
    ans |= (M[pos + i]) << (i << 3);
  }
  return ans;
}

void write(unsigned val, unsigned pos, unsigned len = 4) {
  for(int i = 0; i < len; ++i) {
    M[pos + i] = val & 0xFF;
    val >>= 8;
  }
}

int sext(unsigned c, int bit) {
   if (bit == 32) return c;
   return (c >> (bit - 1) & 1) ? (c | (0xFFFFFFFF >> bit << bit)) : c;
}

inst decode(unsigned co) {
  inst i;
  unsigned op = co & 0x7F;
  i.rd = co >> 7 & 0x1F;
  i.rs1 = co >> 15 & 0x1F;
  i.rs2 = co >> 20 & 0x1F;
  if(op == 0b011'0111u) {
    i.op = LUI;
    i.imm = sext(co >> 12, 20);
  }
  else if (op == 0b001'0111u) {
    i.op = AUIPC;
    i.imm = sext(co >> 12, 20);
  }
  else if (op == 0b110'1111u) {
    i.op = JAL;
    i.imm = (co >> 31 & 1) << 20 | (co >> 21 & 0x3FF) << 1 | (co >> 20 & 1) << 11 | (co >> 12 & 0xFF) << 12;
    i.imm = sext(i.imm, 21);
  }
  else if (op == 0b110'0111u) {
    i.op = JALR;
    i.imm = sext(co >> 20, 12);
  }
  else if (op == 0b110'0011u) {
    i.imm = (co >> 8 & 0xF) << 1 | (co >> 25 & 0x3F) << 5 | (co >> 7 & 1) << 11 | (co >> 31 & 1) << 12;
    i.imm = sext(i.imm, 13);
    switch (co >> 12 & 7) {
      case 0b000:
        i.op = BEQ;
      break;
      case 0b001:
        i.op = BNE;
      break;
      case 0b100:
        i.op = BLT;
      break;
      case 0b101:
        i.op = BGE;
      break;
      case 0b110:
        i.op = BLTU;
      break;
      case 0b111:
        i.op = BGEU;
      break;
    }
  }
  else if (op == 0b000'0011u) {
    i.imm = sext(co >> 20, 12);
    switch (co >> 12 & 7) {
    case 0b000:
      i.op = LB;
    break;
    case 0b001:
      i.op = LH;
    break;
    case 0b010:
      i.op = LW;
    break;
    case 0b100:
      i.op = LBU;
    break;
    case 0b101:
      i.op = LHU;
    break;
    }
  }
  else if (op == 0b010'0011u) {
    i.imm = (co >> 20 & 0xFE0) | (co >> 7 & 0x1F);
    i.imm = sext(i.imm, 12);
    switch (co >> 12 & 7) {
    case 0b000:
      i.op = SB;
    break;
    case 0b001:
      i.op = SH;
    break;
    case 0b010:
      i.op = SW;
    break;
    }
  }
  else if (op == 0b001'0011u) {
    i.imm = sext(co >> 20, 12);
    i.shamt = i.rs2;
    switch (co >> 12 & 7) {
    case 0b000:
      i.op = ADDI;
    break;
    case 0b010:
      i.op = SLTI;
    break;
    case 0b011:
      i.op = SLTIU;
    break;
    case 0b100:
      i.op = XORI;
    break;
    case 0b110:
      i.op = ORI;
    break;
    case 0b111:
      i.op = ANDI;
    break;
    case 0b001:
      i.op = SLLI;
    break;
    case 0b101:
      i.op = (co >> 30 & 1) ? SRAI : SRLI;
    break;
    }
  }
  else if (op == 0b011'0011u) {
    switch (co >> 12 & 7) {
    case 0b000:
      i.op = (co >> 30 & 1) ? SUB : ADD;
    break;
    case 0b001:
      i.op = SLL;
    break;
    case 0b010:
      i.op = SLT;
    break;
    case 0b011:
      i.op = SLTU;
    break;
    case 0b100:
      i.op = XOR;
    break;
    case 0b101:
      i.op = (co >> 30 & 1) ? SRA : SRL;
    break;
    case 0b110:
      i.op = OR;
    break;
    case 0b111:
      i.op = AND;
    break;
    }
  }
  else {
    i.op = nil;
  }
  return i;
}


#endif
