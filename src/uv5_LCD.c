/*----------------------------------------------------------------------------
This project code demonstrates how to interface a ARM Cortex M0+ processor with
a LCD driver IC controller
*----------------------------------------------------------------------------*/
#include <MKL25Z4.H>
#include "gpio_defs.h"
#include "LCD_4bit.h"

/* create a delay function */
void Delay(volatile unsigned int time_del) {
	while (time_del--) {
		;
	}
}

/*------------------------------------------------------------------------------
	Example for LCD interface
	*------------------------------------------------------------------------------*/
void LCD_Example(void) {
	Init_LCD();
	Clear_LCD();
	Set_Cursor(0,0);
	lcd_printchar(0);
	lcd_printchar(1);
	lcd_printchar(2);
	lcd_printchar(3);
	lcd_printchar(4);
	lcd_printchar(5);
	lcd_printchar(6);
	lcd_printchar(7);
}

/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {


	LCD_Example();
	
}
