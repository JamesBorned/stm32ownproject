#include <string.h>
#include <stdio.h>
#include <stm32f10x.h>
#include <showstring.h>
#include <utils.h>
#include <spi.h>

#define LCD_W 128
#define LCD_H 8

uint8_t LCD_Buf[LCD_H][LCD_W];

int main(void) {
	// Enable clock for GPIOC
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
	// Enable PC13 push-pull mode
	GPIOC->CRH &= ~GPIO_CRH_CNF13; //clear cnf bits
	GPIOC->CRH |= GPIO_CRH_MODE13_0; //Max speed = 10Mhz

  SPI1_Init();
  GPIOA->ODR &= ~GPIO_ODR_ODR4; // CS=0
  GPIOA->ODR &= ~GPIO_ODR_ODR2; // RESET=0
  delay(10000); // Wait for the power stabilized
  GPIOA->ODR |= GPIO_ODR_ODR2; // RESET=1
  delay(1000);

  cmd(0xA2); //LCD Drive set 1/9 bias
  cmd(0xA0); // RAM Address SEG Output normal
  cmd(0xC8); // Common outout mode selection
  cmd(0x28 | 0x07); // Power control mode
  cmd(0b00100000 | 0x05); // Voltage regulator
  cmd(0xA6); // Normal color, A7 = inverse color
  cmd(0xAF); // Display on
  cmd(0x40 | 0x00); // Set start line address (Lines 0x00...0x3F)
  memset(LCD_Buf, 0, sizeof(int8_t) * 8 * 128);
  for(int k=0; k<=7; k++){ // Clear DRAM
    cmd(0xB0 | k); // Set Page 0 (Pages 0x00...0x0F)
    for(int i=0; i<=127; i++){
        dat(LCD_Buf[k][i]);
    }
    cmd(0xEE); // End writing to the page, return the page address back
  }

  // Setting the page address:
  //cmd(0xB0 | 0x00); // Set Page 0 (Pages 0x00...0x0F)
  //
  // Setting the line address:
  //cmd(0x10 | 0x00); // Set column address MSB (0x00...0x0F)
  //cmd(0x00); // Set column address LSB (0x00...0x0F)

  /*for(int i=0; i<64; i++)
    for(int j=0; j<128; j++)
      DrawPixel(i,j);
  */

  //DrawChess();
  const char* s = "Triangle";
  PutString(s, 10, 0, 15);
  
  //delay(10000000);
  //cmd(0x40 | 0x01); // Set start line address (Lines 0x00...0x3F)
  
    while (1) { // LED blinking
	    GPIOC->ODR |= (1U<<13U); //U -- unsigned suffix (to avoid syntax warnings in IDE)
		  delay(1000000);
	    GPIOC->ODR &= ~(1U<<13U);
	    delay(1000000);
    }
}