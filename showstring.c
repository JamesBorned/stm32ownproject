#include <string.h>
#include <stdio.h>
#include <stm32f10x.h>
#include <utils.h>
#include <spi.h>

#define LCD_W 128
#define LCD_H 8

uint8_t LCD_Buf[LCD_H][LCD_W];

/*`````````````````````````````````````````````````````````````````````````````````````````````````
*   Font definition -- Char cell: 5 x 8 pixels -- PROPORTIONAL spacing.
*   Char cell: 5 x 8 pixels (8 bytes/char); each data byte is a row of 5 pixels.
*
*   Table format: 8 bytes per character --
*          byte[0] bit 7 is descender flag, if true, shift char bitmap down 1 row;
*          byte[0] bits 2:0 specify width of character (1..5 pixels);
*          byte[1:7] (7 bytes) contain the character bitmap image
*
*   Note:  Most chars start on the top row and are 5 dots wide.
*          Upper-case letters are all 5 dots wide (same as mono font), except 'I'.
*          Digits and lower-case letters are mostly 4 dots wide, with a few exceptions,
*          e.g. '1', 'i', 'j', 'l', etc.
*          Descenders use the 8th row in the bitmap cell.
*          Special cases, e.g. 'j', '[', '{', etc, are handled by LCD_PutChar#().
*
*/
#define FONT_WIDTH 5
const uint8_t font[] = {
  0x0, 0x0, 0x0, 0x0, 0x0,       // Ascii 0
  0x7C, 0xDA, 0xF2, 0xDA, 0x7C,  //ASC(01)
  0x7C, 0xD6, 0xF2, 0xD6, 0x7C,  //ASC(02)
  0x38, 0x7C, 0x3E, 0x7C, 0x38, 
  0x18, 0x3C, 0x7E, 0x3C, 0x18, 
  0x38, 0xEA, 0xBE, 0xEA, 0x38, 
  0x38, 0x7A, 0xFE, 0x7A, 0x38, 
  0x0, 0x18, 0x3C, 0x18, 0x0, 
  0xFF, 0xE7, 0xC3, 0xE7, 0xFF, 
  0x0, 0x18, 0x24, 0x18, 0x0, 
  0xFF, 0xE7, 0xDB, 0xE7, 0xFF, 
  0xC, 0x12, 0x5C, 0x60, 0x70, 
  0x64, 0x94, 0x9E, 0x94, 0x64, 
  0x2, 0xFE, 0xA0, 0xA0, 0xE0, 
  0x2, 0xFE, 0xA0, 0xA4, 0xFC, 
  0x5A, 0x3C, 0xE7, 0x3C, 0x5A, 
  0xFE, 0x7C, 0x38, 0x38, 0x10, 
  0x10, 0x38, 0x38, 0x7C, 0xFE, 
  0x28, 0x44, 0xFE, 0x44, 0x28, 
  0xFA, 0xFA, 0x0, 0xFA, 0xFA, 
  0x60, 0x90, 0xFE, 0x80, 0xFE, 
  0x0, 0x66, 0x91, 0xA9, 0x56, 
  0x6, 0x6, 0x6, 0x6, 0x6,
  0x29, 0x45, 0xFF, 0x45, 0x29, 
  0x10, 0x20, 0x7E, 0x20, 0x10, 
  0x8, 0x4, 0x7E, 0x4, 0x8, 
  0x10, 0x10, 0x54, 0x38, 0x10, 
  0x10, 0x38, 0x54, 0x10, 0x10, 
  0x78, 0x8, 0x8, 0x8, 0x8, 
  0x30, 0x78, 0x30, 0x78, 0x30, 
  0xC, 0x1C, 0x7C, 0x1C, 0xC, 
  0x60, 0x70, 0x7C, 0x70, 0x60, 
  0x0, 0x0, 0x0, 0x0, 0x0, 
  0x0, 0x0, 0xFA, 0x0, 0x0, 
  0x0, 0xE0, 0x0, 0xE0, 0x0, 
  0x28, 0xFE, 0x28, 0xFE, 0x28, 
  0x24, 0x54, 0xFE, 0x54, 0x48, 
  0xC4, 0xC8, 0x10, 0x26, 0x46, 
  0x6C, 0x92, 0x6A, 0x4, 0xA, 
  0x0, 0x10, 0xE0, 0xC0, 0x0, 
  0x0, 0x38, 0x44, 0x82, 0x0, 
  0x0, 0x82, 0x44, 0x38, 0x0, 
  0x54, 0x38, 0xFE, 0x38, 0x54, 
  0x10, 0x10, 0x7C, 0x10, 0x10, 
  0x0, 0x1, 0xE, 0xC, 0x0, 
  0x10, 0x10, 0x10, 0x10, 0x10, 
  0x0, 0x0, 0x6, 0x6, 0x0, 
  0x4, 0x8, 0x10, 0x20, 0x40, 
  0x7C, 0x8A, 0x92, 0xA2, 0x7C, 
  0x0, 0x42, 0xFE, 0x2, 0x0, 
  0x4E, 0x92, 0x92, 0x92, 0x62, 
  0x84, 0x82, 0x92, 0xB2, 0xCC, 
  0x18, 0x28, 0x48, 0xFE, 0x8, 
  0xE4, 0xA2, 0xA2, 0xA2, 0x9C, 
  0x3C, 0x52, 0x92, 0x92, 0x8C, 
  0x82, 0x84, 0x88, 0x90, 0xE0, 
  0x6C, 0x92, 0x92, 0x92, 0x6C, 
  0x62, 0x92, 0x92, 0x94, 0x78, 
  0x0, 0x0, 0x28, 0x0, 0x0, 
  0x0, 0x2, 0x2C, 0x0, 0x0, 
  0x0, 0x10, 0x28, 0x44, 0x82, 
  0x28, 0x28, 0x28, 0x28, 0x28, 
  0x0, 0x82, 0x44, 0x28, 0x10, 
  0x40, 0x80, 0x9A, 0x90, 0x60, 
  0x7C, 0x82, 0xBA, 0x9A, 0x72, 
  0x3E, 0x48, 0x88, 0x48, 0x3E, 
  0xFE, 0x92, 0x92, 0x92, 0x6C, 
  0x7C, 0x82, 0x82, 0x82, 0x44, 
  0xFE, 0x82, 0x82, 0x82, 0x7C, 
  0xFE, 0x92, 0x92, 0x92, 0x82, 
  0xFE, 0x90, 0x90, 0x90, 0x80, 
  0x7C, 0x82, 0x82, 0x8A, 0xCE, 
  0xFE, 0x10, 0x10, 0x10, 0xFE, 
  0x0, 0x82, 0xFE, 0x82, 0x0, 
  0x4, 0x2, 0x82, 0xFC, 0x80, 
  0xFE, 0x10, 0x28, 0x44, 0x82, 
  0xFE, 0x2, 0x2, 0x2, 0x2, 
  0xFE, 0x40, 0x38, 0x40, 0xFE, 
  0xFE, 0x20, 0x10, 0x8, 0xFE, 
  0x7C, 0x82, 0x82, 0x82, 0x7C, 
  0xFE, 0x90, 0x90, 0x90, 0x60, 
  0x7C, 0x82, 0x8A, 0x84, 0x7A, 
  0xFE, 0x90, 0x98, 0x94, 0x62, 
  0x64, 0x92, 0x92, 0x92, 0x4C, 
  0xC0, 0x80, 0xFE, 0x80, 0xC0, 
  0xFC, 0x2, 0x2, 0x2, 0xFC, 
  0xF8, 0x4, 0x2, 0x4, 0xF8, 
  0xFC, 0x2, 0x1C, 0x2, 0xFC, 
  0xC6, 0x28, 0x10, 0x28, 0xC6, 
  0xC0, 0x20, 0x1E, 0x20, 0xC0, 
  0x86, 0x9A, 0x92, 0xB2, 0xC2, 
  0x0, 0xFE, 0x82, 0x82, 0x82, 
  0x40, 0x20, 0x10, 0x8, 0x4, 
  0x0, 0x82, 0x82, 0x82, 0xFE, 
  0x20, 0x40, 0x80, 0x40, 0x20, 
  0x2, 0x2, 0x2, 0x2, 0x2, 
  0x0, 0xC0, 0xE0, 0x10, 0x0, 
  0x4, 0x2A, 0x2A, 0x1E, 0x2, 
  0xFE, 0x14, 0x22, 0x22, 0x1C, 
  0x1C, 0x22, 0x22, 0x22, 0x14, 
  0x1C, 0x22, 0x22, 0x14, 0xFE, 
  0x1C, 0x2A, 0x2A, 0x2A, 0x18, 
  0x0, 0x10, 0x7E, 0x90, 0x40, 
  0x18, 0x25, 0x25, 0x39, 0x1E, 
  0xFE, 0x10, 0x20, 0x20, 0x1E, 
  0x0, 0x22, 0xBE, 0x2, 0x0, 
  0x4, 0x2, 0x2, 0xBC, 0x0, 
  0xFE, 0x8, 0x14, 0x22, 0x0, 
  0x0, 0x82, 0xFE, 0x2, 0x0, 
  0x3E, 0x20, 0x1E, 0x20, 0x1E, 
  0x3E, 0x10, 0x20, 0x20, 0x1E, 
  0x1C, 0x22, 0x22, 0x22, 0x1C, 
  0x3F, 0x18, 0x24, 0x24, 0x18, 
  0x18, 0x24, 0x24, 0x18, 0x3F, 
  0x3E, 0x10, 0x20, 0x20, 0x10, 
  0x12, 0x2A, 0x2A, 0x2A, 0x24, 
  0x20, 0x20, 0xFC, 0x22, 0x24, 
  0x3C, 0x2, 0x2, 0x4, 0x3E, 
  0x38, 0x4, 0x2, 0x4, 0x38, 
  0x3C, 0x2, 0xC, 0x2, 0x3C, 
  0x22, 0x14, 0x8, 0x14, 0x22, 
  0x32, 0x9, 0x9, 0x9, 0x3E, 
  0x22, 0x26, 0x2A, 0x32, 0x22, 
  0x0, 0x10, 0x6C, 0x82, 0x0, 
  0x0, 0x0, 0xEE, 0x0, 0x0, 
  0x0, 0x82, 0x6C, 0x10, 0x0, 
  0x40, 0x80, 0x40, 0x20, 0x40, 
  0x3C, 0x64, 0xC4, 0x64, 0x3C, 
  0x78, 0x85, 0x85, 0x86, 0x48, 
  0x5C, 0x2, 0x2, 0x4, 0x5E, 
  0x1C, 0x2A, 0x2A, 0xAA, 0x9A, 
  0x84, 0xAA, 0xAA, 0x9E, 0x82, 
  0x84, 0x2A, 0x2A, 0x1E, 0x82, 
  0x84, 0xAA, 0x2A, 0x1E, 0x2, 
  0x4, 0x2A, 0xAA, 0x9E, 0x2, 
  0x30, 0x78, 0x4A, 0x4E, 0x48, 
  0x9C, 0xAA, 0xAA, 0xAA, 0x9A, 
  0x9C, 0x2A, 0x2A, 0x2A, 0x9A, 
  0x9C, 0xAA, 0x2A, 0x2A, 0x1A, 
  0x0, 0x0, 0xA2, 0x3E, 0x82, 
  0x0, 0x40, 0xA2, 0xBE, 0x42, 
  0x0, 0x80, 0xA2, 0x3E, 0x2, 
  0xF, 0x94, 0x24, 0x94, 0xF, 
  0xF, 0x14, 0xA4, 0x14, 0xF, 
  0x3E, 0x2A, 0xAA, 0xA2, 0x0, 
  0x4, 0x2A, 0x2A, 0x3E, 0x2A,
  0x3E, 0x50, 0x90, 0xFE, 0x92, 
  0x4C, 0x92, 0x92, 0x92, 0x4C, 
  0x4C, 0x12, 0x12, 0x12, 0x4C, 
  0x4C, 0x52, 0x12, 0x12, 0xC, 
  0x5C, 0x82, 0x82, 0x84, 0x5E, 
  0x5C, 0x42, 0x2, 0x4, 0x1E, 
  0x0, 0xB9, 0x5, 0x5, 0xBE, 
  0x9C, 0x22, 0x22, 0x22, 0x9C, 
  0xBC, 0x2, 0x2, 0x2, 0xBC, 
  0x3C, 0x24, 0xFF, 0x24, 0x24, 
  0x12, 0x7E, 0x92, 0xC2, 0x66, 
  0xD4, 0xF4, 0x3F, 0xF4, 0xD4, 
  0xFF, 0x90, 0x94, 0x6F, 0x4, 
  0x3, 0x11, 0x7E, 0x90, 0xC0, 
  0x4, 0x2A, 0x2A, 0x9E, 0x82, 
  0x0, 0x0, 0x22, 0xBE, 0x82, 
  0xC, 0x12, 0x12, 0x52, 0x4C, 
  0x1C, 0x2, 0x2, 0x44, 0x5E, 
  0x0, 0x5E, 0x50, 0x50, 0x4E, 
  0xBE, 0xB0, 0x98, 0x8C, 0xBE, 
  0x64, 0x94, 0x94, 0xF4, 0x14, 
  0x64, 0x94, 0x94, 0x94, 0x64, 
  0xC, 0x12, 0xB2, 0x2, 0x4, 
  0x1C, 0x10, 0x10, 0x10, 0x10, 
  0x10, 0x10, 0x10, 0x10, 0x1C, 
  0xF4, 0x8, 0x13, 0x35, 0x5D, 
  0xF4, 0x8, 0x14, 0x2C, 0x5F, 
  0x0, 0x0, 0xDE, 0x0, 0x0, 
  0x10, 0x28, 0x54, 0x28, 0x44, 
  0x44, 0x28, 0x54, 0x28, 0x10, 
  0x55, 0x0, 0xAA, 0x0, 0x55, 
  0x55, 0xAA, 0x55, 0xAA, 0x55, 
  0xAA, 0x55, 0xAA, 0x55, 0xAA,
  0x0, 0x0, 0x0, 0xFF, 0x0, 
  0x8, 0x8, 0x8, 0xFF, 0x0, 
  0x28, 0x28, 0x28, 0xFF, 0x0, 
  0x8, 0x8, 0xFF, 0x0, 0xFF, 
  0x8, 0x8, 0xF, 0x8, 0xF, 
  0x28, 0x28, 0x28, 0x3F, 0x0, 
  0x28, 0x28, 0xEF, 0x0, 0xFF, 
  0x0, 0x0, 0xFF, 0x0, 0xFF, 
  0x28, 0x28, 0x2F, 0x20, 0x3F, 
  0x28, 0x28, 0xE8, 0x8, 0xF8, 
  0x8, 0x8, 0xF8, 0x8, 0xF8, 
  0x28, 0x28, 0x28, 0xF8, 0x0, 
  0x8, 0x8, 0x8, 0xF, 0x0, 
  0x0, 0x0, 0x0, 0xF8, 0x8,
  0x8, 0x8, 0x8, 0xF8, 0x8,
  0x8, 0x8, 0x8, 0xF, 0x8,
  0x0, 0x0, 0x0, 0xFF, 0x8,
  0x8, 0x8, 0x8, 0x8, 0x8,
  0x8, 0x8, 0x8, 0xFF, 0x8,
  0x0, 0x0, 0x0, 0xFF, 0x28,
  0x0, 0x0, 0xFF, 0x0, 0xFF,
  0x0, 0x0, 0xF8, 0x8, 0xE8,
  0x0, 0x0, 0x3F, 0x20, 0x2F,
  0x28, 0x28, 0xE8, 0x8, 0xE8,
  0x28, 0x28, 0x2F, 0x20, 0x2F,
  0x0, 0x0, 0xFF, 0x0, 0xEF,
  0x28, 0x28, 0x28, 0x28, 0x28,
  0x28, 0x28, 0xEF, 0x0, 0xEF,
  0x28, 0x28, 0x28, 0xE8, 0x28,
  0x8, 0x8, 0xF8, 0x8, 0xF8,
  0x28, 0x28, 0x28, 0x2F, 0x28,
  0x8, 0x8, 0xF, 0x8, 0xF,
  0x0, 0x0, 0xF8, 0x8, 0xF8,
  0x0, 0x0, 0x0, 0xF8, 0x28,
  0x0, 0x0, 0x0, 0x3F, 0x28,
  0x0, 0x0, 0xF, 0x8, 0xF,
  0x8, 0x8, 0xFF, 0x8, 0xFF,
  0x28, 0x28, 0x28, 0xFF, 0x28,
  0x8, 0x8, 0x8, 0xF8, 0x0,
  0x0, 0x0, 0x0, 0xF, 0x8,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xF, 0xF, 0xF, 0xF, 0xF,
  0xFF, 0xFF, 0xFF, 0x0, 0x0, 
  0x0, 0x0, 0x0, 0xFF, 0xFF,
  0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
  0x1C, 0x22, 0x22, 0x1C, 0x22, 
  0x3E, 0x54, 0x54, 0x7C, 0x28, 
  0x7E, 0x40, 0x40, 0x60, 0x60, 
  0x40, 0x7E, 0x40, 0x7E, 0x40, 
  0xC6, 0xAA, 0x92, 0x82, 0xC6, 
  0x1C, 0x22, 0x22, 0x3C, 0x20, 
  0x2, 0x7E, 0x4, 0x78, 0x4, 
  0x60, 0x40, 0x7E, 0x40, 0x40, 
  0x99, 0xA5, 0xE7, 0xA5, 0x99, 
  0x38, 0x54, 0x92, 0x54, 0x38, 
  0x32, 0x4E, 0x80, 0x4E, 0x32, 
  0xC, 0x52, 0xB2, 0xB2, 0xC, 
  0xC, 0x12, 0x1E, 0x12, 0xC, 
  0x3D, 0x46, 0x5A, 0x62, 0xBC, 
  0x7C, 0x92, 0x92, 0x92, 0x0, 
  0x7E, 0x80, 0x80, 0x80, 0x7E, 
  0x54, 0x54, 0x54, 0x54, 0x54, 
  0x22, 0x22, 0xFA, 0x22, 0x22, 
  0x2, 0x8A, 0x52, 0x22, 0x2, 
  0x2, 0x22, 0x52, 0x8A, 0x2, 
  0x0, 0x0, 0xFF, 0x80, 0xC0, 
  0x7, 0x1, 0xFF, 0x0, 0x0, 
  0x10, 0x10, 0xD6, 0xD6, 0x10,
  0x6C, 0x48, 0x6C, 0x24, 0x6C, 
  0x60, 0xF0, 0x90, 0xF0, 0x60, 
  0x0, 0x0, 0x18, 0x18, 0x0, 
  0x0, 0x0, 0x8, 0x8, 0x0, 
  0xC, 0x2, 0xFF, 0x80, 0x80, 
  0x0, 0xF8, 0x80, 0x80, 0x78, 
  0x0, 0x98, 0xB8, 0xE8, 0x48, 
  0x0, 0x3C, 0x3C, 0x3C, 0x3C,};

void cmd(uint8_t data){
    GPIOA->ODR &= ~GPIO_ODR_ODR3; // A0=0 --a command is being sent
    GPIOA->ODR &= ~GPIO_ODR_ODR4; // CS=0
    SPI1_Write(data);
    //delay(500);
    while (SPI1->SR & SPI_SR_BSY);
    GPIOA->ODR |= GPIO_ODR_ODR4; // CS=1
}

void dat(uint8_t data){
    GPIOA->ODR |= GPIO_ODR_ODR3; // A0=1 --data is being sent
    GPIOA->ODR &= ~GPIO_ODR_ODR4; // CS=0
    //delay(1000);
    SPI1_Write(data);
    while (SPI1->SR & SPI_SR_BSY); //delay(500); // Important!
    GPIOA->ODR |= GPIO_ODR_ODR4; // CS=1
}

//отправит сразу несколько байт
void dat_bulk(uint8_t *data, size_t count){
    GPIOA->ODR |= GPIO_ODR_ODR3;
    GPIOA->ODR &= ~GPIO_ODR_ODR4;
    for (int i=0; i<count; i++) {
       SPI1_Write(data[i]);
    }
    while (SPI1->SR & SPI_SR_BSY);
    GPIOA->ODR |= GPIO_ODR_ODR4; // CS=1
}

void lcdbufPutPixel(uint8_t x, uint8_t y) {
  uint8_t page_address = (x >> 3) % 8;
  y = y % 128;    
  uint8_t column = LCD_Buf[page_address][y];
  column |= (1 << (x % 8));
  LCD_Buf[page_address][y] = column;
}

void lcdbufBlitScreen() {
  cmd(0x40 | 0x00); // Set start line address (Lines 0x00...0x3F)
  for(int k=0; k<=7; k++) { // Clear DRAM
    cmd(0xB0 | k); // Set Page 0 (Pages 0x00...0x0F)
    dat_bulk(LCD_Buf[k], 128);
    cmd(0xEE); // End writing to the page, return the page address back
  }
}

//Обновляет только часть буфера lcdbuf [x .. x+h]x[y .. y+w]
void lcdbufBlitRect(uint8_t x, uint8_t y, uint8_t h, uint8_t w) {
  uint8_t start_addr = (x / 8) % 8;
  uint8_t end_addr = MAX(((x + h) / 8), 7);
  w = MIN(w, 127 - y);
  for (uint8_t page=start_addr; page<=end_addr; page++) {
    cmd(0xB0 | (page & 0x0F));
    cmd(0b00010000 | (y >> 4) & 0x0F);
		cmd(0b00000000 | y & 0x0F);
    dat_bulk(&LCD_Buf[page][y], w + 1);
    cmd(0xEE);
  }
}

void DrawPixel(uint8_t x, uint8_t y){ // x in [0..63], y in [0..127]
  uint8_t page_address = (x >> 3) & 0b00000111; // %8 guarantees page_address in [0..7]
  cmd(0xB0 | page_address); // Set Page page_address (Pages 0x00...0x0F)
  cmd(y & 0b00001111); // Set column address LSB
  cmd(0b00010000 | (y >> 4));
  uint8_t column = LCD_Buf[page_address][y];
  //Set pixel value in LCD_buf
  column |= (1 << (x % 8));
  dat(column);
  LCD_Buf[page_address][y] = column;
}

void DrawChess(){
  uint8_t i,j,k; // i=row number, j=col number, k=cycle inside a square
  uint8_t val = 0x00; // Color 0x00=White strip, 0xFF=black strip
  for(i=0; i<=7; i++){
    cmd(0xB0 | i); // Set Page i (Pages 0x00...0x0F)
    val = ~ val;
    for(j=0; j<=15; j++){
      for(k=0; k<=7; k++) dat(val);
      val = ~val;
    }
    cmd(0xEE);
  }
}

void lcdbufDrawLine(int x0, int y0, int x1, int y1) {
	int delta_x = ABS(x1 - x0);
	int delta_y = ABS(y1 - y0);
	int error = 0;
	
	if (delta_x >= delta_y) {
		//x-direction
		if (x0 > x1) {
			SWAP(x0, x1, int);
			SWAP(y0, y1, int);
		}
		int dir_y = SIGN(y1 - y0);
		int delta_err = delta_y + 1;
		int y = y0;
		for (int x=x0; x<=x1; x++) {
			lcdbufPutPixel(x, y);
			error += delta_err;
			if (error >= delta_x + 1) {
				y += dir_y;
				error -= (delta_x + 1);
			}
		}
	} else { //y-direction
		if (y0 > y1) {
			SWAP(x0, x1, int);
			SWAP(y0, y1, int);
		}
		int dir_x = SIGN(x1 - x0);
		int delta_err = delta_x + 1;
		int x = x0;
		for (int y=y0; y<=y1; y++) {
			lcdbufPutPixel(x, y);
			error += delta_err;
			if (error >= delta_y + 1) {
				x += dir_x;
				error -= (delta_y + 1);
			}
		}
	}
}

void PutChar(char x){
  uint8_t *pData = (uint8_t *) &font[x * 5];
  for(int i=0; i<5; i++ ){ dat( bitrev(*pData) ); pData++; }
  dat(0x00); // Spacing
}

void PutString(const char pChar[], int strlen, uint8_t page, uint8_t column){
  uint8_t pagenum = page;
  uint8_t colnum = column;
  cmd(0xB0 | pagenum); // Set Page p (Pages 0x00...0x0F)
  cmd(0x10 | (column >> 4)); // Set column address MSB (0x00...0x0F)
  cmd(column & 0x0F); // Set column address LSB (0x00...0x0F)
  
  for(int i=0; i<strlen; i++){
    if(colnum + FONT_WIDTH > LCD_W){
      pagenum++;
      colnum = 0;
      cmd(0xEE); // End of line
      cmd(0xB0 | (pagenum % 8)); // Set Page p (Pages 0x00...0x0F)
    }
      PutChar(pChar[i]);
      colnum += (FONT_WIDTH + 1);
  }
  

}