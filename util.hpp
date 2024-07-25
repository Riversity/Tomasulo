#ifndef UTIL_HPP
#define UTIL_HPP
#include "core.hpp"


void execute(unsigned inst) {
  unsigned op = inst % 128u;
  if(op == 0b011'0111u) {
    // lui
  }
  else if (op == 0b001'0111u) {
    // auipc
  }
  else if (op == 0b110'1111u) {
    // jal
  }
  else if (op == 0b110'0111u) {
    
  }
}

#endif
