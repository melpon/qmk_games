#include <stdbool.h>

#include "xorshift.h"
#include "screen.h"

struct Tetris {
  uint8_t blocks[40][10];
  uint8_t cur;
  uint8_t curx;
  uint8_t cury;
  uint8_t rot;
  uint8_t next;
  uint8_t nextrot;
  uint8_t gamestate;
  uint16_t lasttime;
  uint16_t score;
  uint8_t counter;
  uint16_t speed;
  int debugval;
  Xorshift xorshift;
  bool initialized;
};
typedef struct Tetris Tetris;

extern void tetris_init(Tetris* tetris);
extern void tetris_update(Tetris* tetris);
extern void tetris_render(Tetris* tetris, ScreenMatrix* matrix);

// dir:
//   0: 左
//   1: 右
//   2: 下
extern void tetris_move(Tetris* tetris, int dir);

// cw:
//   0: 反時計回り
//   1: 時計回り
extern void tetris_rotate(Tetris* tetris, int cw);
