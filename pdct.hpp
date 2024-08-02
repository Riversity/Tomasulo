#ifndef PRDT_HPP
#define PRDT_HPP
#include "inst.hpp"

struct PRED {
int status[256] = {};
bool predict(unsigned pc) {
  if(status[pc & 0xff] >= 2) return true;
  else return false;
}
void decrease(unsigned pc) {
  if(status[pc & 0xff] <= 0) status[pc & 0xff] = 0;
  else --status[pc & 0xff];
}
void increase(unsigned pc) {
  if(status[pc & 0xff] >= 3) status[pc & 0xff] = 3;
  else ++status[pc & 0xff];
}
} pred;
#endif
