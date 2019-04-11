#include "screen.h"

#include <string.h>

struct ScreenMatrix g_screen;

void screen_clear(ScreenMatrix* screen) {
  memset(screen->screen, 0, sizeof(screen->screen));
  screen->dirty = true;
}

void screen_draw_rect(ScreenMatrix* screen, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t mode) {
  uint8_t fill_masks[4] = { 0 };
  uint8_t draw_masks[4] = { 0 };

  uint8_t x2 = x + w;
  while (x < x2) {
    uint8_t n = x / 8;
    uint8_t m = x % 8;
    fill_masks[n] |= (1 << (7 - m));
    x++;
  }

  {
    uint8_t n, m;
    n = (x2 - w) / 8;
    m = (x2 - w) % 8;
    draw_masks[n] |= (1 << (7 - m));
    n = (x2 - 1) / 8;
    m = (x2 - 1) % 8;
    draw_masks[n] |= (1 << (7 - m));
  }

  if (mode == 0) {
    uint8_t y2 = y + h;
    while (y < y2) {
      for (uint8_t i = 0; i < 4; i++) {
        screen->screen[y][i] |= fill_masks[i];
      }
      y++;
    }
  } else if (mode == 1) {
    uint8_t y2 = y + h;
    while (y < y2) {
      // 最初と最後は fill_masks を使って、それ以外は draw_masks を利用する
      if (y == y2 - h || y == y2 - 1) {
        for (uint8_t i = 0; i < 4; i++) {
          screen->screen[y][i] |= fill_masks[i];
        }
      } else {
        for (uint8_t i = 0; i < 4; i++) {
          screen->screen[y][i] |= draw_masks[i];
        }
      }
      y++;
    }
  }

  screen->dirty = true;
}

void screen_draw_rect_line(ScreenMatrix* screen, int x1, int y1, int x2, int y2, int mode) {
  int x = x1 < x2 ? x1 : x2;
  int w = x1 < x2 ? (x2 - x1) : (x1 - x2);
  int y = y1 < y2 ? y1 : y2;
  int h = y1 < y2 ? (y2 - y1) : (y1 - y2);
  screen_draw_rect(screen, x, y, w + 1, h + 1, mode);
}

void _screen_render_number_one_impl(ScreenMatrix* screen, int x, int y, const int p[][2], int size) {
  const uint8_t pos[][2] = {
    { 0, 0 },
    { 2, 0 },
    { 0, 2 },
    { 2, 2 },
    { 0, 4 },
    { 2, 4 },
  };
  for (int i = 0; i < size; i++) {
    screen_draw_rect_line(screen, x + pos[p[i][0]][0], y + pos[p[i][0]][1], x + pos[p[i][1]][0], y + pos[p[i][1]][1], 0);
  }
}

// 0 1 000 111 001 111 111 101 111 111 111 111 111
//     000 101 001 001 001 101 100 100 101 101 101
// 2 3 111 101 001 111 111 111 111 111 101 111 111
//     000 101 001 100 001 001 001 101 001 101 001
// 4 5 000 111 001 111 111 001 111 111 001 111 111
void _screen_render_number_one(ScreenMatrix* screen, int x, int y, int n) {
  switch (n) {
    case -1: {
      const int ps[][2] = {
        { 2, 3 },
      };
      _screen_render_number_one_impl(screen, x, y, ps, sizeof(ps) / sizeof(ps[0]));
      break;
    }
    case 0: {
      const int ps[][2] = {
        { 0, 1 },
        { 1, 5 },
        { 5, 4 },
        { 4, 0 },
      };
      _screen_render_number_one_impl(screen, x, y, ps, sizeof(ps) / sizeof(ps[0]));
      break;
    }
    case 1: {
      const int ps[][2] = {
        { 1, 5 },
      };
      _screen_render_number_one_impl(screen, x, y, ps, sizeof(ps) / sizeof(ps[0]));
      break;
    }
    case 2: {
      const int ps[][2] = {
        { 0, 1 },
        { 1, 3 },
        { 3, 2 },
        { 2, 4 },
        { 4, 5 },
      };
      _screen_render_number_one_impl(screen, x, y, ps, sizeof(ps) / sizeof(ps[0]));
      break;
    }
    case 3: {
      const int ps[][2] = {
        { 1, 5 },
        { 0, 1 },
        { 2, 3 },
        { 4, 5 },
      };
      _screen_render_number_one_impl(screen, x, y, ps, sizeof(ps) / sizeof(ps[0]));
      break;
    }
    case 4: {
      const int ps[][2] = {
        { 0, 2 },
        { 2, 3 },
        { 1, 5 },
      };
      _screen_render_number_one_impl(screen, x, y, ps, sizeof(ps) / sizeof(ps[0]));
      break;
    }
    case 5: {
      const int ps[][2] = {
        { 1, 0 },
        { 0, 2 },
        { 2, 3 },
        { 3, 5 },
        { 5, 4 },
      };
      _screen_render_number_one_impl(screen, x, y, ps, sizeof(ps) / sizeof(ps[0]));
      break;
    }
    case 6: {
      const int ps[][2] = {
        { 1, 0 },
        { 0, 4 },
        { 4, 5 },
        { 5, 3 },
        { 3, 2 },
      };
      _screen_render_number_one_impl(screen, x, y, ps, sizeof(ps) / sizeof(ps[0]));
      break;
    }
    case 7: {
      const int ps[][2] = {
        { 2, 0 },
        { 0, 1 },
        { 1, 5 },
      };
      _screen_render_number_one_impl(screen, x, y, ps, sizeof(ps) / sizeof(ps[0]));
      break;
    }
    case 8: {
      const int ps[][2] = {
        { 0, 4 },
        { 1, 5 },
        { 0, 1 },
        { 2, 3 },
        { 4, 5 },
      };
      _screen_render_number_one_impl(screen, x, y, ps, sizeof(ps) / sizeof(ps[0]));
      break;
    }
    case 9: {
      const int ps[][2] = {
        { 3, 2 },
        { 2, 0 },
        { 0, 1 },
        { 1, 5 },
      };
      _screen_render_number_one_impl(screen, x, y, ps, sizeof(ps) / sizeof(ps[0]));
      break;
    }
  }
}

// 番号のレンダリング
void screen_render_number(ScreenMatrix* screen, int x, int y, int n) {
  if (n < 0) {
    _screen_render_number_one(screen, x, y, -1);
    x += 4;
    n = -n;
  }
  int digits = 1;
  int d = n;
  while (d >= 10) {
    digits *= 10;
    d /= 10;
  }
  while (digits != 0) {
    int v = (n / digits) % 10;
    _screen_render_number_one(screen, x, y, v);
    digits /= 10;
    x += 4;
  }
}

void screen_update(ScreenMatrix *dest, const ScreenMatrix *source) {
  if (memcmp(dest->screen, source->screen, sizeof(dest->screen))) {
    memcpy(dest->screen, source->screen, sizeof(dest->screen));
    dest->dirty = true;
  }
}
