#include "drawcore.h"

char* draw(char* buf, char* _buf) {
  char c;
  uint i = 0;

  while ((c = _buf[i])) {
    buf[i] = c;
    i++;
  }

  return (buf += i);
}

char* drawn(char* buf, char c, uint n) {
  uint i = 0;

  while (n) {
    buf[i] = c;
    i++, n--;
  };

  return (buf += i);
}

char* draw_num(char* buf, uint num) {
  uint _i = num;
  uint j = 0;
  do {
    _i /= 10;
    j++;
  } while (_i);

  uint exp = 1;
  for (uint _ = 1; _ < j; _++) {
    exp *= 10;
  }

  j = 0;
  while (exp) {
    uint div = num / exp;
    buf[j] = ITOA(div);
    num -= exp * div;
    exp /= 10;
    j++;
  }

  buf += j;
  return buf;
}
