#ifndef INST_HPP
#define INST_HPP
#include <iostream>
#include <cassert>

enum inst_type {
  nil = -1,
  LUI, AUIPC,
  JAL, JALR, BEQ, BNE, BLT, BGE, BLTU, BGEU,
  LB, LH, LW, LBU, LHU,
  SB, SH, SW,
  ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI, 
  ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND,
  HALT
};

enum stat_type {
  None = -1, Issu, Exec, Wres, Comt
};

struct inst {
  inst_type op;
  unsigned pc;
  int imm;
  unsigned rs1, rs2, rd;
  bool pred_jp;
  inst() {}
};

template <class T, int K>
class Que {
public:
  int head, tail;
  int siz;
  T a[K];
  Que() {
    head = 0;
    tail = 0;
    siz = 0;
  }
  ~Que() = default;
  Que& operator=(const Que& rhs) = default;
  /* Que& operator=(const Que& rhs) {
    head = rhs.head;
    tail = rhs.tail;
    siz = rhs.siz;
    for(int i = 0; i < K; ++i) {
      a[i] = rhs.a[i];
    }
  } */
  T& operator[] (int i) {
    return a[i];
  }
  bool full() {
    return siz == K;
  }
  bool empty() {
    return siz == 0;
  }
  int push(T in) {
    assert(!full());
    ++siz;
    a[tail] = in;
    int tmp = tail;
    if(tail == K - 1) tail = 0;
    else ++tail;
    return tmp;
  }
  T& top() {
    assert(!empty());
    return a[head];
  }
  void pop() {
    assert(!empty());
    --siz;
    if(head == K - 1) head = 0;
    else ++head;
  }
  void clear() {
    head = 0;
    tail = 0;
    siz = 0;
  }
};

template <class T, int K>
class Lst {
public:
  int siz;
  T a[K];
  bool b[K];
  Lst() {
    siz = 0;
    for(int i = 0; i < K; ++i) b[i] = false;
  }
  ~Lst() = default;
  Lst& operator=(const Lst& rhs) = default;
  /* Que& operator=(const Que& rhs) {
    head = rhs.head;
    tail = rhs.tail;
    siz = rhs.siz;
    for(int i = 0; i < K; ++i) {
      a[i] = rhs.a[i];
    }
  } */
  T& operator[] (int i) {
    return a[i];
  }
  bool full() {
    return siz == K;
  }
  bool empty() {
    return siz == 0;
  }
  int get() {
    assert(!full());
    for(int i = 0; i < K; ++i) {
      if(!b[i]) return i;
    }
    return -1;
  }
  int find() {
    assert(!empty());
    for(int i = 0; i < K; ++i) {
      if(b[i]) return i;
    }
    return -1;
  }
  int put(T in) {
    int pos = get();
    ++siz;
    a[pos] = in;
    b[pos] = true;
    return pos;
  }
  void del(int pos) {
    if(b[pos]) --siz;
    b[pos] = false;
  }
  void clear() {
    siz = 0;
    for(int i = 0; i < K; ++i) b[i] = false;
  }
};

#endif
