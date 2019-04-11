#include "tetris.h"

#include <string.h>

#include "timer.h"

// ブロックの情報
const int8_t blockdata[][4][2] = {
  // N
  // O
  // N
  // N
  { { 0, -1 }, { 0, 0 }, { 0, 1 }, { 0, 2 } },
  // ON
  // NN
  { { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 } },
  // NO
  //  NN
  { { -1, 0 }, { 0, 0 }, { 0, 1 }, { 1, 1 } },
  //  ON
  // NN
  { { -1, 1 }, { 0, 0 }, { 0, 1 }, { 1, 0 } },
  // N
  // ONN
  { { 0, 0 }, { 1, 0 }, { 2, 0 }, { 0, -1 } },
  // ONN
  // N
  { { 0, 0 }, { 1, 0 }, { 2, 0 }, { 0, 1 } },
  //  N
  // NON
  { { -1, 0 }, { 0, 0 }, { 1, 0 }, { 0, -1 } },
};

void tetris_init(Tetris* tetris) {
  uint16_t t = timer_read();
  xorshift_init(&tetris->xorshift, t, (t >> 8), 3, 4);

  memset(tetris->blocks, 0, sizeof(tetris->blocks));
  tetris->gamestate = 0;
  tetris->lasttime = 0;
  tetris->cur = 0;
  tetris->curx = 4;
  tetris->cury = 7;
  tetris->rot = 0;
  tetris->next = xorshift_next(&tetris->xorshift) % sizeof(blockdata) / sizeof(blockdata[0]);
  tetris->nextrot = xorshift_next(&tetris->xorshift) % 4;
  tetris->score = 0;
  tetris->counter = 0;
  tetris->debugval = sizeof(int);
  tetris->speed = 1000;
  tetris->initialized = true;
}
// -4: 左の範囲外
// -3: 右の範囲外
// -2: 上の範囲外
// -1: 下の範囲外
// 0: 何もない
// 1: 配置済みブロック
int _tetris_get_block_state(Tetris* tetris, int x, int y) {
  // 範囲外判定
  if (x < 0) {
    return -4;
  }
  if (x >= 10) {
    return -3;
  }
  if (y < 0) {
    return -2;
  }
  if (y >= 40) {
    return -1;
  }
  // 既存のブロックとの当たり判定
  if (tetris->blocks[y][x] != 0) {
    return 1;
  }
  return 0;
}

// 回転させたブロックの座標を返す
// r には回転させた座標が x1, y1, x2, y2, x3, y3, x4, y4 の順番に並ぶ
void _tetris_get_rotated_pos(int cur, int x, int y, int rot, int r[][2]) {
  const int8_t sins[] = { 0, 1, 0, -1 };
  const int8_t coss[] = { 1, 0, -1, 0 };
  for (uint8_t i = 0; i < 4; i++) {
    int p = blockdata[cur][i][0];
    int q = blockdata[cur][i][1];
    int s = sins[rot];
    int c = coss[rot];

    (*r)[0] = x + (p * c - q * s);
    (*r)[1] = y + (p * s + q * c);
    r++;
  }
}

// どれかのブロックに当たってるか外側に出てたら 0 以外
int _tetris_is_hit(Tetris* tetris, int cur, int x, int y, int rot) {
  int pos[4][2];
  _tetris_get_rotated_pos(cur, x, y, rot, pos);
  int j;
  for (j = 0; j < 4; j++) {
    int state = _tetris_get_block_state(tetris, pos[j][0], pos[j][1]);
    if (state != 0) {
      return state;
    }
  }
  return 0;
}

void tetris_rotate(Tetris* tetris, int cw) {
  if (tetris->gamestate != 0) {
    return;
  }

  int rot;
  if (cw == 1) {
    rot = (tetris->rot + 1) % 4;
  } else {
    rot = (tetris->rot + 3) % 4;
  }

  // 現在の位置と上右左に１マスずつ補正した位置を試して、
  // どことも接触してなければそれを採用する。
  // どれも接触してたら回転させない
  const int8_t ts[][2] = {
    { 0, 0 },
    { 0, -1 },
    { -1, 0 },
    { 1, 0 },
  };
  for (int i = 0; i < 4; i++) {
    int hit = _tetris_is_hit(tetris, tetris->cur, tetris->curx + ts[i][0], tetris->cury + ts[i][1], rot);
    if (hit == 0) {
      // 接触しなかった
      tetris->rot = rot;
      tetris->curx += ts[i][0];
      tetris->cury += ts[i][1];
      return;
    }
  }
}
void tetris_next_block(Tetris* tetris) {
  // 今のブロックを確定させる
  int pos[4][2];
  _tetris_get_rotated_pos(tetris->cur, tetris->curx, tetris->cury, tetris->rot, pos);
  for (int i = 0; i < 4; i++) {
    tetris->blocks[pos[i][1]][pos[i][0]] = 1;
  }

  // 列が揃ってたら削除
  int score = 1; // 0, 1, 3, 7, 15
  int lines = 0;
  int y = 39;
  int y2 = 39;
  while (y2 >= 0) {
    if (y < 0) {
      memset(tetris->blocks[y2], 0, sizeof(tetris->blocks[0]));
      y2 -= 1;
      continue;
    }

    int n = 0;
    for (int x = 0; x < 10; ++x) {
      n += tetris->blocks[y][x];
    }
    if (n == 10) {
      // 一列消えた
      score *= 2;
      lines += 1;
    } else {
      if (y != y2) {
        memcpy(tetris->blocks[y2], tetris->blocks[y], sizeof(tetris->blocks[0]));
      }
      y2 -= 1;
    }
    y -= 1;
  }

  tetris->score += score - 1;
  for (int i = 0; i < lines; i++) {
    // 0.95 倍
    tetris->speed = tetris->speed * 95 / 100;
    if (tetris->speed < 50) {
      tetris->speed = 50;
    }
  }

  // 次のブロックを出す
  tetris->cur = tetris->next;
  tetris->rot = tetris->nextrot;
  tetris->curx = 4;
  tetris->cury = 7;
  tetris->next = xorshift_next(&tetris->xorshift) % sizeof(blockdata) / sizeof(blockdata[0]);
  tetris->nextrot = xorshift_next(&tetris->xorshift) % 4;
  // 既に当たってたらゲームオーバー
  int hit = _tetris_is_hit(tetris, tetris->cur, tetris->curx, tetris->cury, tetris->rot);
  if (hit != 0) {
    tetris->gamestate = 1;
    //tetris_init(tetris);
  }
}

void tetris_move(Tetris* tetris, int dir) {
  if (tetris->gamestate != 0) {
    return;
  }

  if (dir == 0 || dir == 1) {
    int dx = dir == 0 ? -1 : 1;
    int hit = _tetris_is_hit(tetris, tetris->cur, tetris->curx + dx, tetris->cury, tetris->rot);
    if (hit == 0) {
      tetris->curx += dx;
    }
  } else if (dir == 2) {
    while (true) {
      int hit = _tetris_is_hit(tetris, tetris->cur, tetris->curx, tetris->cury + 1, tetris->rot);
      if (hit == 0) {
        tetris->cury += 1;
      } else {
        tetris_next_block(tetris);
        return;
      }
    }
  }
}

void tetris_update(Tetris* tetris) {
  if (!tetris->initialized) {
    tetris_init(tetris);
  }

  if (tetris->gamestate == 0) {
    uint16_t time = timer_read();
    if (time - tetris->lasttime < tetris->speed) {
      return;
    }
    tetris->lasttime = time;

    int hit = _tetris_is_hit(tetris, tetris->cur, tetris->curx, tetris->cury + 1, tetris->rot);
    if (hit == 0) {
      tetris->cury += 1;
    } else {
      tetris_next_block(tetris);
    }
  } else {
    uint16_t time = timer_read();
    if (time - tetris->lasttime < 50) {
      return;
    }
    tetris->lasttime = time;

    uint8_t c = tetris->counter++;
    if (c < 40) {
      for (int i = 0; i < 10; i++) {
        tetris->blocks[40 - c - 1][i] = 1;
      }
    } else if (c < 50) {
      // wait
    } else if (c < 90) {
      for (int i = 0; i < 10; i++) {
        tetris->blocks[c - 50][i] = 0;
      }
    } else if (c < 100) {
      // wait
    } else {
      // init
      tetris_init(tetris);
    }
  }
}

void tetris_render(Tetris* tetris, ScreenMatrix* matrix) {
  for (int y = 0; y < 40; y++) {
    for (int x = 0; x < 10; x++) {
      if (tetris->blocks[y][x] != 0) {
        screen_draw_rect(matrix, x * 3, y * 3, 2, 2, 0);
      }
    }
  }
  if (tetris->gamestate == 0) {
    // 動いてるブロック
    int pos[4][2];
    _tetris_get_rotated_pos(tetris->cur, tetris->curx, tetris->cury, tetris->rot, pos);
    for (uint8_t i = 0; i < 4; i++) {
      int x = pos[i][0];
      int y = pos[i][1];
      screen_draw_rect(matrix, x * 3, y * 3, 2, 2, 0);
    }

    // 次のブロック
    _tetris_get_rotated_pos(tetris->next, 4, 2, tetris->nextrot, pos);
    for (uint8_t i = 0; i < 4; i++) {
      int x = pos[i][0];
      int y = pos[i][1];
      screen_draw_rect(matrix, x * 3, y * 3, 2, 2, 0);
    }

    // 横線
    screen_draw_rect(matrix, 0, 15, 31, 1, 0);

    // ガイドの表示
    int cury = tetris->cury;
    while (true) {
      int hit = _tetris_is_hit(tetris, tetris->cur, tetris->curx, cury + 1, tetris->rot);
      if (hit == 0) {
        cury += 1;
        continue;
      }

      _tetris_get_rotated_pos(tetris->cur, tetris->curx, cury, tetris->rot, pos);
      for (uint8_t i = 0; i < 4; i++) {
        int x = pos[i][0];
        int y = pos[i][1];
        screen_draw_rect(matrix, x * 3, y * 3, 3, 3, 1);
      }
      break;
    }
  }

  screen_render_number(matrix, 0, 0, tetris->score);
}
