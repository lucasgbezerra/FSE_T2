#include "control_lcd.h"

int fd = -1;

void lcd_type_float(float myFloat)
{
    char buffer[20];
    sprintf(buffer, "%4.2f", myFloat);
    lcd_type_ln(buffer);
}

void lcd_type_int(int i)
{
    char array1[20];
    sprintf(array1, "%d", i);
    lcd_type_ln(array1);
}

void clear_lcd(void)
{
    lcd_byte(0x01, LCD_CMD);
    lcd_byte(0x02, LCD_CMD);
}

void lcd_go_to_loc(int line)
{
    lcd_byte(line, LCD_CMD);
}

void lcd_type_char(char val)
{
    lcd_byte(val, LCD_CHR);
}

void lcd_type_ln(const char *s)
{
    while (*s)
        lcd_byte(*(s++), LCD_CHR);
}

void lcd_byte(int bits, int mode)
{

    // Send byte to data pins
    //  bits = the data
    //  mode = 1 for data, 0 for command
    int bits_high;
    int bits_low;
    // uses the two half byte writes to LCD
    bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT;
    bits_low = mode | ((bits << 4) & 0xF0) | LCD_BACKLIGHT;

    // High bits
    wiringPiI2CReadReg8(fd, bits_high);
    lcd_toggle_enable(bits_high);

    // Low bits
    wiringPiI2CReadReg8(fd, bits_low);
    lcd_toggle_enable(bits_low);
}

void lcd_toggle_enable(int bits)
{
    // Toggle enable pin on LCD display
    delayMicroseconds(500);
    wiringPiI2CReadReg8(fd, (bits | ENABLE));
    delayMicroseconds(500);
    wiringPiI2CReadReg8(fd, (bits & ~ENABLE));
    delayMicroseconds(500);
}

int lcd_init()
{
    if (wiringPiSetup() == -1)
        return -1;

    fd = wiringPiI2CSetup(I2C_ADDR);

    lcd_byte(0x33, LCD_CMD);
    lcd_byte(0x32, LCD_CMD);
    lcd_byte(0x06, LCD_CMD);
    lcd_byte(0x0C, LCD_CMD);
    lcd_byte(0x28, LCD_CMD);
    lcd_byte(0x01, LCD_CMD);
    delayMicroseconds(500);

    return fd;
}

void write_lcd(char *first_line, char *second_line)
{
    clear_lcd();
    lcd_go_to_loc(LINE1);
    lcd_type_ln(first_line);
    lcd_go_to_loc(LINE2);
    lcd_type_ln(second_line);
}
