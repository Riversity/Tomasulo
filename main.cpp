#include <cstdio>
#include <iostream>
#include "inst.hpp"
#include "util.hpp"
#include "core.hpp"

int main() {
  // unsigned pc_prev = 0;
  input();
  unsigned clock = 0u;
  while(!global_halt) {
    /* updates */
    pc_in = pc_nx;
    reg.upd();
    iq.upd();
    rob.upd();
    rs.upd();
    slb.upd();

    slb.exe();
    rob.exe();
    iq.fetch();
    rs.exe();
    ++clock;
    /*if(pc_real != pc_prev) {
      std::cout << std::hex << pc_real << std::endl;
      for(int i = 0; i < 16; ++i) {
        std::cout<<"|"<<reg.reg_nx[i].val;
      }
      std::cout<<std::endl;
      pc_prev = pc_real;
    }*/
  }
  std::cout << std::dec << (reg.reg_in[10].val & 0xFF) << std::endl;
  return 0;
}
