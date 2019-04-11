#ifndef SCREEN_H
#define SCREEN_H

#include "ssd1306.h"

struct ScreenMatrix {
  uint8_t screen[DisplayWidth][MatrixRows];
  bool dirty;
};
typedef struct ScreenMatrix ScreenMatrix;
extern struct ScreenMatrix g_screen;

void screen_clear(ScreenMatrix* screen);

// 矩形の描画
// mode: 0 fill
// mode: 1 draw
void screen_draw_rect(ScreenMatrix* screen, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t mode);

// 縦か横だけの線の描画
void screen_draw_rect_line(ScreenMatrix* screen, int x1, int y1, int x2, int y2, int mode);

// 番号の描画
void screen_render_number(ScreenMatrix* screen, int x, int y, int n);

void screen_update(ScreenMatrix *dest, const ScreenMatrix *source);

#endif // SCREEN_H
