#include "xorshift.h"

void xorshift_init(Xorshift* p, uint8_t x, uint8_t y, uint8_t z, uint8_t t) {
  p->x = x;
  p->y = y;
  p->z = z;
  p->t = t;
  for (int i = 0; i < 20; i++) {
    xorshift_next(p);
  }
}
uint8_t xorshift_next(Xorshift* p) {
  p->t = p->x ^ (p->x << 1);
  p->x = p->y;
  p->y = p->z;
  p->z ^= (p->z >> 3) ^ p->t ^ (p->t >> 5);
  return p->z;
}

