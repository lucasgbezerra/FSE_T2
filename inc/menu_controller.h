#ifndef MENU_CONTROLLER_H_
#define MENU_CONTROLLER_H_

#define NORMAL 0
#define MEAT 1
#define FRENCH_FRIES 2
#define NUGGETS 3
// Tempo em minutos
#define MEAT_TIME 10
#define FRENCH_FRIES_TIME 8
#define NUGGETS_TIME 6

// Temperatura em graus
#define MEAT_TEMPERATURE 70.0
#define FRENCH_FRIES_TEMPERATURE 60.0
#define NUGGETS_TEMPERATURE 50.0

typedef struct struct_menu
{
    const char *options[4];
    int times[4];
    float temperatures[4];
    int current;
}st_menu;

void menu_init(st_menu *menu);
int menu_controller(st_menu *menu);
void show_menu(int time, float temperature, char *option);

#endif