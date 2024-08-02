#ifndef PRDT_HPP
#define PRDT_HPP
#include "inst.hpp"

struct PRED {
int status = 0;
bool predict(inst &i) {
  /* WAIT */
  if(status >= 2) return true;
  else return false;
}
void decrease() {
  if(status <= 0) status = 0;
  else --status;
}
void increase() {
  if(status >= 3) status = 3;
  else ++status;
}
} pred;
#endif
