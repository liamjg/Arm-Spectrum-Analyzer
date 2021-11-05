#include "LCD_4bit.h"
#include "delay.h"
#include <stdint.h>

uint8_t new_char[] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1f,
											 0x0,0x0,0x0,0x0,0x0,0x0,0x1f,0x1f,
											 0x0,0x0,0x0,0x0,0x0,0x1f,0x1f,0x1f,
											 0x0,0x0,0x0,0x0,0x1f,0x1f,0x1f,0x1f,
											 0x0,0x0,0x0,0x1f,0x1f,0x1f,0x1f,0x1f,
											 0x0,0x0,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
											 0x0,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,
											 0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f
};

//with 4-bit data, we need to break 
//down to two 4-bit operations, upper
//nibble and lower nibble.

//reading 4-bit nibble to 
//LCD can be done by using

static uint8_t lcd_read_status(void)
{
  uint8_t status;

  SET_LCD_DATA_DIR_IN
  SET_LCD_RS(0)
  SET_LCD_RW(1)
  Delay(1);
  SET_LCD_E(1)
  Delay(1);
  status  = GET_LCD_DATA_IN << 4;
  SET_LCD_E(0)
  Delay(1);
  SET_LCD_E(1)
  Delay(1);
  status |= GET_LCD_DATA_IN;
  SET_LCD_E(0)
  SET_LCD_DATA_DIR_OUT
  return(status);
}

void wait_while_busy(void)
{
	for( ; lcd_read_status() & LCD_BUSY_FLAG_MASK; )
		;
}

//To send information to the LCD across the 4 bit data bus,
//we use the following function. This function is called twice 
//to write a byte.
void lcd_write_4bit(uint8_t c)
{
  SET_LCD_RW(0)
  SET_LCD_E(1)
  SET_LCD_DATA_OUT(c&0x0F)
  Delay(1);
  SET_LCD_E(0)
  Delay(1);
}

//To write an instruction (i.e. command) to the LCD controller, 
//we use this function which writes the argument while keeping 
//the RS line at 0, indicating the byte is a command.
void lcd_write_cmd(uint8_t c)
{
  wait_while_busy();

  SET_LCD_RS(0)
  lcd_write_4bit(c>>4);
  lcd_write_4bit(c);
}

//To write data to the LCD controller, we use this function, 
//which writes the argument while keeping the RS line at 1, 
//indicating the byte is data.
static void lcd_write_data(uint8_t c)
{
  wait_while_busy();

  SET_LCD_RS(1)
  lcd_write_4bit(c>>4);
  lcd_write_4bit(c);
}

void lcd_putchar(char c)
{ 
  lcd_write_data(c);
}

//The MCU port pins connected to the LCD must 
//first be initialized as GPIO.
void lcd_init_port(void) {
	/* Enable clocks for peripherals        */
  ENABLE_LCD_PORT_CLOCKS                          

	/* Set Pin Mux to GPIO */
	PIN_DATA_PORT->PCR[PIN_DATA_SHIFT] = PORT_PCR_MUX(1);
	PIN_DATA_PORT->PCR[PIN_DATA_SHIFT+1] = PORT_PCR_MUX(1);
	PIN_DATA_PORT->PCR[PIN_DATA_SHIFT+2] = PORT_PCR_MUX(1);
	PIN_DATA_PORT->PCR[PIN_DATA_SHIFT+3] = PORT_PCR_MUX(1);
	PIN_E_PORT->PCR[PIN_E_SHIFT] = PORT_PCR_MUX(1);
	PIN_RW_PORT->PCR[PIN_RW_SHIFT] = PORT_PCR_MUX(1);
	PIN_RS_PORT->PCR[PIN_RS_SHIFT] = PORT_PCR_MUX(1);
}

void add_Chars(void){
	int i;
	//set cgram address 0
	lcd_write_cmd(0x40);
	Delay(10);
	for(i=0; i<= 63; i++){
		lcd_write_data(new_char[i]);
		Delay(10);
	}
	
	
}
//We use these pieces to initialize the HD44780 LCD controller 
//as directed in the datasheet
void Init_LCD(void)
{ 
	/* initialize port(s) for LCD */
	lcd_init_port();
	
  /* Set all pins for LCD as outputs */
  SET_LCD_ALL_DIR_OUT
  Delay(100);
  SET_LCD_RS(0)
	
	//0x03 allows to shift 4-bit data to LCD
	//see SET_LCD_DATA_OUT(x) when x=0x03
  lcd_write_4bit(0x03);                 
  Delay(100);
  lcd_write_4bit(0x03);
  Delay(10);
 // lcd_write_4bit(0x3);
 // lcd_write_4bit(0x2);
	//Function Set: 4-bit, 2 Line, 5x7 Dots
  lcd_write_cmd(0x28);
 // Display on Cursor off	
  lcd_write_cmd(0x0C); 
 //	Entry Mode
  lcd_write_cmd(0x06); 
	//Add custom characters
	add_Chars();
 //Set DDRAM address or coursor position on display	
  lcd_write_cmd(0x80);                 
}

void Set_Cursor(uint8_t column, uint8_t row)
{
  uint8_t address;

  address =(row * 0x40) + column;
	address |= 0x80;
  lcd_write_cmd(address);               
}

void Clear_LCD(void)
{
  lcd_write_cmd(0x01);                 
  Set_Cursor(0, 0);
}

void Print_LCD(char *string)
{
  while(*string)  {
    lcd_putchar(*string++);
  }
}
