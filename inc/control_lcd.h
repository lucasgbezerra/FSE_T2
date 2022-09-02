#ifndef CONTROL_LCD_
#define CONTROL_LCD_

#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <stdio.h>

#define I2C_ADDR 0x27
#define LCD_CHR 1
#define LCD_CMD 0
#define LINE1 0x80
#define LINE2 0xC0
#define LCD_BACKLIGHT 0x08
#define ENABLE 0b00000100

int lcd_init(void);
void lcd_byte(int bits, int mode);
void lcd_toggle_enable(int bits);

void lcd_type_int(int i);
void lcd_type_float(float myFloat);
void lcd_go_to_loc(int line);
void clear_lcd(void);
void lcd_type_ln(const char *s);
void lcd_type_char(char val);
void write_lcd(char *first_line, char *second_line);
#endif