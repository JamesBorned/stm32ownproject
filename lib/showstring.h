#pragma once
#include <stdint.h>
#include <stddef.h>

#define LCD_W 128
#define LCD_H 8
static uint8_t LCD_Buf[LCD_H][LCD_W];

void cmd(uint8_t data);
void dat(uint8_t data);
void dat_bulk(uint8_t *data, size_t count);
void lcdbufPutPixel(uint8_t x, uint8_t y);
void lcdbufBlitScreen();
void lcdbufBlitRect(uint8_t x, uint8_t y, uint8_t h, uint8_t w);
void DrawPixel(uint8_t x, uint8_t y);
void DrawChess();
void lcdbufDrawLine(int x0, int y0, int x1, int y1);
void PutChar(char x);
void PutString(const char pChar[], int strlen, uint8_t page, uint8_t column);
