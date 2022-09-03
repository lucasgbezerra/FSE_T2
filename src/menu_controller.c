#include "menu_controller.h"
#include "control_lcd.h"


void menu_init(st_menu *menu)
{
    menu->options[0] = "NORMAL";
    menu->options[1] = "CARNE";
    menu->options[2] = "BATATA FRITA";
    menu->options[3] = "NUGGETS";

    menu->temperatures[0] = NORMAL;
    menu->temperatures[1] = MEAT_TEMPERATURE;
    menu->temperatures[2] = FRENCH_FRIES_TEMPERATURE;
    menu->temperatures[3] = NUGGETS_TEMPERATURE;

    menu->times[0] = NORMAL;
    menu->times[1] = MEAT_TIME;
    menu->times[2] = FRENCH_FRIES_TIME;
    menu->times[3] = NUGGETS_TIME;

    menu->current = 0;
}

int menu_controller(st_menu *menu)
{
    if (menu->current == NUGGETS)
    {
        menu->current = NORMAL;
    }
    else
    {
        menu->current++;
    }
    show_menu(menu->times[menu->current], menu->temperatures[menu->current], menu->options[menu->current]);
    return menu->current;
}

void show_menu(int time, float temperature, char * option)
{
    char first_line[16];
    char second_line[16];

    sprintf(second_line, "TIME:%d TR:%.1f", time, temperature);
    sprintf(first_line, option);
    write_lcd(first_line, second_line);
}