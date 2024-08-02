#ifndef CORE_HPP
#define CORE_HPP
#include "util.hpp"
#include "pdct.hpp"

bool global_halt = false;

unsigned pc_real = 0;
unsigned pc_in, pc_nx = 0;

const int MAX_R = 32;

void clearall();
void cancel_stuck();
void set_rob(int Q, int proc, int which, int val);
bool get_rob(int Q, int& val);

struct Register {
struct RegisterNode {
  unsigned val = 0;
  bool busy = false;
  int Q = -1; // No. in ROB
};

RegisterNode reg_in[MAX_R]; // 0 old, 1 new
RegisterNode reg_nx[MAX_R];

void upd() {
  for(int i = 0; i < MAX_R; ++i) {
    reg_in[i] = reg_nx[i];
  }
  reg_in[0] = {0, false, -1};
  reg_nx[0] = {0, false, -1};
}
} reg;

struct ALU {
unsigned calc(inst_type op, unsigned vj, unsigned vk, unsigned aa) { // vj = rs1 | pc, vk = rs2, a = imm | shamt
  unsigned res;
  switch (op) {
  case LUI:
    res = aa << 12;
  break;
  case AUIPC:
    res = vj + (aa << 12);
  break;
  case JALR:
    res = ((vj + aa) & ~1);
  break;
  case BEQ:
    res = (vj == vk);
  break;
  case BNE:
    res = (vj != vk);
  break;
  case BLT:
    res = (static_cast<int>(vj) < static_cast<int>(vk));
  break;
  case BGE:
    res = (static_cast<int>(vj) >= static_cast<int>(vk));
  break;
  case BLTU:
    res = (vj < vk);
  break;
  case BGEU:
    res = (vj >= vk);
  break;
  case LB:
    res = vj + aa;
  break;
  case LH:
    res = vj + aa;
  break;
  case LW:
    res = vj + aa;
  break;
  case LBU:
    res = vj + aa;
  break;
  case LHU:
    res = vj + aa;
  break;
  case SB:
    res = vj + aa;
  break;
  case SH:
    res = vj + aa;
  break;
  case SW:
    res = vj + aa;
  break;
  case ADDI:
    res = vj + aa;
  break;
  case SLTI:
    res = static_cast<int>(vj) < static_cast<int>(aa);
  break;
  case SLTIU:
    res = static_cast<unsigned>(vj) < static_cast<unsigned>(aa);
  break;
  case XORI:
    res = vj ^ aa;
  break;
  case ORI:
    res = vj | aa;
  break;
  case ANDI:
    res = vj & aa;
  break;
  case SLLI:
    res = vj << aa;
  break;
  case SRLI:
    res = vj >> aa;
  break;
  case SRAI:
    res = sext(vj >> aa, 32 - aa);
  break;
  case ADD:
    res = vj + vk;
  break;
  case SUB:
    res = vj - vk;
  break;
  case SLL:
    res = vj << vk;
  break;
  case SLT:
    res = static_cast<int>(vj) < static_cast<int>(vk);
  break;
  case SLTU:
    res = vj < vk;
  break;
  case XOR:
    res = vj ^ vk;
  break;
  case SRL:
    res = vj >> vk;
  break;
  case SRA:
    res = sext(vj >> vk, 32 - vk);
  break;
  case OR:
    res = vj | vk;
  break;
  case AND:
    res = vj & vk;
  break;
  default:
    assert(false);
  break;
  }
  return res;
}
} alu;

struct RSNode {
  bool busy;
  inst_type op;
  unsigned Vj, Vk;
  int Qj = -1, Qk = -1; // -1 ready
  unsigned A;
  int Qdest;
};

struct RS {
Lst<RSNode, 32> RS_in;
Lst<RSNode, 32> RS_nx;

void clear() {
  RS_in.clear();
  RS_nx.clear();
}
void upd() {
  RS_in = RS_nx;
}
void exe() {
  if(RS_in.empty()) return;
  for(int i = 0; i < 32; ++i) {
    if(!RS_in.b[i]) continue;
    // std::cout<<std::hex<<"IN RS "<<i<<" "<<RS_in[i].op<<std::endl;

    RSNode node = RS_in[i];
    // std::cout<<"QJQK "<<node.Qj << node.Qk << std::endl;
    /*experimental*/
    if(node.Qj != -1) {
      int tmp;
      if(get_rob(node.Qj, tmp)) {
        RS_nx[i].Qj = -1;
        RS_nx[i].Vj = tmp;
      }
    }
    if(node.Qk != -1) {
      int tmp;
      if(get_rob(node.Qk, tmp)) {
        RS_nx[i].Qk = -1;
        RS_nx[i].Vk = tmp;
      }
    }
    if(node.Qj == -1 && node.Qk == -1) {
      if(node.op == JAL) {
        set_rob(node.Qdest, 2, 0, 0);
        RS_nx.del(i);
        continue;
      }
      if(node.op == HALT) {
        set_rob(node.Qdest, 2, 0, 0);
        RS_nx.del(i);
        return;
      }
      unsigned res = alu.calc(node.op, node.Vj, node.Vk, node.A);
      RS_nx[i].busy = false;
      //if(LB <= node.op && node.op <= LHU) {
      //  set_rob(node.Qdest, 0, 2, res);
      //}
      //else if((SB <= node.op && node.op <= SW) || (node.op == JALR)) {}
      if(node.op == JALR) {
        set_rob(node.Qdest, 2, 2, res);
        // std::cout<<"RES="<<res<<std::endl;
      }
      else if(!(LB <= node.op && node.op <= SW)) {
        set_rob(node.Qdest, 2, 1, res);
      }
      RS_nx.del(i);
      break;
    }
  }
}
} rs;

struct SLB {
  Que<RSNode, 32> slb_in;
  Que<RSNode, 32> slb_nx;
  bool stuck = false;
  void upd() {
    slb_in = slb_nx;
  }
  void clear() {
    slb_in.clear();
    slb_nx.clear();
    stuck = false;
  }
  void exe() {
    if(!stuck && !slb_in.empty()) {
      int i = slb_in.head;
      RSNode node = slb_in.top();
      // std::cout <<"SLQJQK" << std::hex << node.Qj << " " << node.Qk << std::endl;
      if(node.Qj != -1) {
        int tmp;
        if(get_rob(node.Qj, tmp)) {
          slb_nx[i].Qj = -1;
          slb_nx[i].Vj = tmp;
        }
      }
      if(node.Qk != -1) {
        int tmp;
        if(get_rob(node.Qk, tmp)) {
          slb_nx[i].Qk = -1;
          slb_nx[i].Vk = tmp;
        }
      }
      if(node.Qj == -1 && node.Qk == -1) {
        int pos = node.Vj + node.A;
        switch(node.op) {
        case LB:
          set_rob(node.Qdest, 2, 1, sext(mem(pos, 1), 8));
        break;
        case LH:
          set_rob(node.Qdest, 2, 1, sext(mem(pos, 2), 16));
        break;
        case LW:
          set_rob(node.Qdest, 2, 1, sext(mem(pos, 4), 32));
        break;
        case LBU:
          set_rob(node.Qdest, 2, 1, mem(pos, 1));
        break;
        case LHU:
          set_rob(node.Qdest, 2, 1, mem(pos, 2));
        break;
        case SB:
          // write(node.Vk, pos, 1);
          stuck = true;
          set_rob(node.Qdest, 2, 2, pos);
          set_rob(node.Qdest, 2, 1, node.Vk);
        break;
        case SH:
          // write(node.Vk, pos, 2);
          stuck = true;
          set_rob(node.Qdest, 2, 2, pos);
          set_rob(node.Qdest, 2, 1, node.Vk);
        break;
        case SW:
          // write(node.Vk, pos, 4);
          stuck = true;
          set_rob(node.Qdest, 2, 2, pos);
          set_rob(node.Qdest, 2, 1, node.Vk);
        break;
        default:
          assert(false);
        break;
        }
        slb_nx.pop();
      }
    }
  }
} slb;

struct RoBNode {
  bool busy = false;
  stat_type stat = None;
  inst in;
  int dest = -1;
  int rs = -1, rt = -1; // -1 not have
  unsigned val = -1; // value to write in dest
  unsigned pos = -1; // calculated pos
  int exec_process = 0;
  int R;
  RoBNode() = default;
  RoBNode(inst i) {
    in = i;
    if((LUI <= i.op && i.op <= JALR) || (LB <= i.op && i.op <= LHU) || (ADDI <= i.op && i.op <= AND)) dest = i.rd;
    if(JALR <= i.op && i.op <= AND) rs = i.rs1;
    if((BEQ <= i.op && i.op <= BGEU) || (SB <= i.op && i.op <= SW) || (ADD <= i.op && i.op <= AND)) rt = i.rs2;
  }
};
struct RoB {
Que<RoBNode, 64> RoB_in;
Que<RoBNode, 64> RoB_nx;
void upd() {
  RoB_in = RoB_nx;
}
void clear() {
  RoB_in.clear();
  RoB_nx.clear();
}
void exe() {
  if(!RoB_in.empty() && RoB_in.top().stat >= Wres) commit(RoB_in.head);
  if(!RoB_in.empty()) {
    int i = RoB_in.head;
    stat_type st = RoB_in[i].stat;
    if(st == None) issue(i);
    else if(st == Issu) execute(i);
    else if(st == Exec) write_result(i);
    for(int i = (RoB_in.head + 1) % 64; i != RoB_in.tail; i = (i + 1) % 64) {
      stat_type st = RoB_in[i].stat;
      if(st == None) issue(i);
      else if(st == Issu) execute(i);
      else if(st == Exec) write_result(i);
      // std::cout<<std::hex<<"IN ROB "<< i << " " <<RoB_in[i].in.pc<<" "<<st<<std::endl;
    }
  }
}
void commit(int i) {
  RoBNode node = RoB_in[i];
  /*if(node.dest != -1) {
    reg.reg_nx[node.dest].busy = false;
    reg.reg_nx[node.dest].val = node.val;
  }*/
  if(node.in.op == HALT) {
    global_halt = true;
    return;
  }
  pc_real = node.in.pc + 4;
  if(BEQ <= node.in.op && node.in.op <= BGEU) {
    if(node.in.pred_jp != node.val) { // pred fail
      pc_real = pc_nx = (node.val) ? (node.in.pc + node.in.imm) : (node.in.pc + 4);
      if(node.val) {
        pred.increase();
      }
      else {
        pred.decrease();
      }
      clearall();
      return;
    }
  } else if(node.in.op == JAL) {
    pc_real = node.in.pc + node.in.imm;
    reg.reg_nx[node.dest].busy = false;
    reg.reg_nx[node.dest].val = node.val;
  } else if(node.in.op == JALR) {
    pc_real = pc_nx = node.pos;
    reg.reg_nx[node.dest].busy = false;
    reg.reg_nx[node.dest].val = node.val;
    cancel_stuck();
  } else if(node.in.op == SB) {
    write(node.val, node.pos, 1);
    slb.stuck = false;
  } else if(node.in.op == SH) {
    write(node.val, node.pos, 2);
    slb.stuck = false;
  } else if(node.in.op == SW) {
    write(node.val, node.pos, 4);
    slb.stuck = false;
  } else {
    reg.reg_nx[node.dest].val = node.val;
  }
  if(reg.reg_nx[node.dest].Q == i) {
    reg.reg_nx[node.dest].busy = false;
  }
  RoB_nx[i].busy = false;
  RoB_nx[i].stat = Comt;
  RoB_nx.pop();
}

void issue(int i) {
  RoBNode node = RoB_in[i];
  inst_type op = node.in.op;
  if(LB <= op && op <= SW) {
    if(!slb.slb_nx.full()) {
      /*basically same as below*/
      RSNode out = {true, node.in.op, 0, 0, -1, -1, 0, -1};
      if(node.rs != -1) {
        if(reg.reg_nx[node.rs].busy) {
          int Qh = reg.reg_nx[node.rs].Q;
          if(RoB_nx[Qh].stat == Wres) {
            out.Vj = RoB_nx[Qh].val;
          }
          else {
            out.Qj = Qh;
          }
        }
        else {
          out.Vj = reg.reg_nx[node.rs].val;
        }
      }
      if(node.rt != -1) {
        if(reg.reg_nx[node.rt].busy) {
          int Qh = reg.reg_nx[node.rt].Q;
          if(RoB_nx[Qh].stat == Wres) {
            out.Vk = RoB_nx[Qh].val;
          }
          else {
            out.Qk = Qh;
          }
        }
        else {
          out.Vk = reg.reg_nx[node.rt].val;
        }
      }
      out.A = node.in.imm;
      out.Qdest = i;

      RoB_nx[i].R = slb.slb_nx.push(out);

      if(node.dest != -1) {
        reg.reg_nx[node.dest].busy = true;
        reg.reg_nx[node.dest].Q = i;
      }
      RoB_nx[i].busy = true;
      RoB_nx[i].stat = Issu;
    }
  }
  else if(!rs.RS_nx.full()) { // to RS
    RoB_nx[i].R = rs.RS_nx.put({true, node.in.op, 0, 0, -1, -1, 0, -1});
    int Rh = RoB_nx[i].R;
    RSNode& out = rs.RS_nx[Rh];
    if(node.rs != -1) {
      if(reg.reg_nx[node.rs].busy) { // caution, nx?
        int Qh = reg.reg_nx[node.rs].Q;
        if(RoB_nx[Qh].stat == Wres) {
          out.Vj = RoB_nx[Qh].val;
        }
        else {
          out.Qj = Qh;
        }
      }
      else {
        out.Vj = reg.reg_nx[node.rs].val;
      }
    }
    if(node.rt != -1) {
      if(reg.reg_nx[node.rt].busy) {
        int Qh = reg.reg_nx[node.rt].Q;
        if(RoB_nx[Qh].stat == Wres) {
          out.Vk = RoB_nx[Qh].val;
        }
        else {
          out.Qk = Qh;
        }
      }
      else {
        out.Vk = reg.reg_nx[node.rt].val;
      }
    }

    if(op == AUIPC) {
      out.Vj = node.in.pc;
    }
    if(op == JAL || op == JALR) {
      RoB_nx[i].val = node.in.pc + 4;
    }
    // if(op == JALR) {
    // std::cout<<std::hex<<out.Vj<<" "<<out.Vk<<" "<<out.Qj<<" "<<out.Qk <<" "<<out.A<<std::endl;
    // }

    out.A = node.in.imm;
    out.Qdest = i;
    if(node.dest != -1) {
      reg.reg_nx[node.dest].busy = true;
      reg.reg_nx[node.dest].Q = i;
    }
    RoB_nx[i].busy = true;
    RoB_nx[i].stat = Issu;
  }
}

void execute(int i) {
  RoBNode node = RoB_in[i];
  // inst_type op = node.in.op;
  if(node.exec_process == 0) {
    // std::cout<<std::hex<<"!"<<node.in.pc<<std::endl;
    return;
  } // RS or SLB will do the work
  /*if(node.exec_process == 1) {
    if(op == LB) RoB_nx[i].val = sext(mem(node.pos, 1), 8);
    else if(op == LH) RoB_nx[i].val = sext(mem(node.pos, 2), 16);
    else if(op == LW) RoB_nx[i].val = sext(mem(node.pos, 4), 32);
    else if(op == LBU) RoB_nx[i].val = mem(node.pos, 1);
    else if(op == LHU) RoB_nx[i].val = mem(node.pos, 2);
    ++RoB_nx[i].exec_process;
  }*/
  if(node.exec_process == 2) {
    RoB_nx[i].stat = Exec;
  }
}

void write_result(int i) {
  RoBNode node = RoB_in[i];
  // std::cout<<std::hex<<"?!"<<node.in.pc<<std::endl;
  if(node.dest != -1) {
    for(int j = 0; j < 32; ++j) {
      if(!rs.RS_nx.b[j]) continue;
      if(rs.RS_nx[j].Qj == i) {
        rs.RS_nx[j].Qj = -1;
        rs.RS_nx[j].Vj = node.val;
      }
      if(rs.RS_nx[j].Qk == i) {
        rs.RS_nx[j].Qk = -1;
        rs.RS_nx[j].Vk = node.val;
      }
    }
    if(!slb.slb_nx.empty()) {
      int j = slb.slb_nx.head;
      if(slb.slb_nx[j].Qj == i) {
        slb.slb_nx[j].Qj = -1;
        slb.slb_nx[j].Vj = node.val;
      }
      if(slb.slb_nx[j].Qk == i) {
        slb.slb_nx[j].Qk = -1;
        slb.slb_nx[j].Vk = node.val;
      }
      for(int j = (slb.slb_nx.head + 1) % 32; j != slb.slb_nx.tail; j = (j + 1) % 32) {
        if(slb.slb_nx[j].Qj == i) {
          slb.slb_nx[j].Qj = -1;
          slb.slb_nx[j].Vj = node.val;
        }
        if(slb.slb_nx[j].Qk == i) {
          slb.slb_nx[j].Qk = -1;
          slb.slb_nx[j].Vk = node.val;
        }
      }
    }
  }
  RoB_nx[i].stat = Wres;
}

} rob;

struct IQ {
bool stuck_in = false, stuck_nx = false;
Que<inst, 64> IQQ;

void upd() {
  stuck_in = stuck_nx;
}
void clear() {
  stuck_in = stuck_nx = false;
  IQQ.clear();
}
unsigned get_next_pc(inst& i) {
  if(BEQ <= i.op && i.op <= BGEU) {
    i.pred_jp = pred.predict(i);
    if(i.pred_jp) return pc_in + i.imm;
    else return pc_in + 4;
  }
  else if(i.op == JAL) {
    return pc_in + i.imm;
  }
  else if((i.op == JALR) || (i.op == HALT)) {
    stuck_nx = true;
    return pc_in;
  }
  else return pc_in + 4;
}

void fetch() {
  if(!stuck_in && !IQQ.full()) {
    unsigned code = mem(pc_in);
    inst i = decode(code, pc_in);
    pc_nx = get_next_pc(i);
    IQQ.push(i);
    // if(i.op == JALR || i.op == HALT) stuck = true;
  }
  // send to RoB
  if(!IQQ.empty()) {
    inst now = IQQ.top();
    if(!rob.RoB_nx.full()) {
      rob.RoB_nx.push(RoBNode(now));
      IQQ.pop();
    }
  }
}
} iq;

void clearall() {
  iq.clear();
  rob.clear();
  rs.clear();
  slb.clear();
  for(int i = 0; i < 32; ++i) {
    reg.reg_nx[i].busy = false;
    reg.reg_nx[i].Q = -1;
  }
}

void cancel_stuck() {
  iq.stuck_nx = false;
}

void set_rob(int Q, int proc, int which, int res) {
  rob.RoB_nx[Q].exec_process = proc;
  if(which == 1) {
    rob.RoB_nx[Q].val = res;
  }
  else if(which == 2) {
    rob.RoB_nx[Q].pos = res;
  }
}

bool get_rob(int Q, int& val) {
  if(rob.RoB_in[Q].stat >= Wres) {
    return true;
    val = rob.RoB_in[Q].val;
  }
  return false;
}

#endif
