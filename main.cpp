#include <cstdio>
#include <iostream>
#include "inst.hpp"
#include "util.hpp"
#include "core.hpp"

int main() {
  input();
  unsigned clock = 0u;
  while(!global_halt && clock <= 400000) {
    /* updates */
    pc_in = pc_nx;
    reg.upd();
    iq.upd();
    rob.upd();
    rs.upd();
    slb.upd();

    iq.fetch();
    rs.exe();
    slb.exe();
    rob.exe();
    ++clock;
    /*std::cout << std::hex << "->" << pc_nx << ' ' << pc_real << std::endl;
    for(int i = 0; i < 15; ++i) {
      std::cout<<"|"<<reg.reg_nx[i].val;
    }
    std::cout<<std::endl;*/
  }
  std::cout << std::dec << (reg.reg_in[10].val & 0xFF) << std::endl;
  return 0;
}
