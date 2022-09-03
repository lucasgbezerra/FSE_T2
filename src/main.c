#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

#include "modbus.h"
#include "uart.h"
#include "controller.h"
#include "control_lcd.h"
#include "bme280_controller.h"



int main()
{
    signal(SIGINT, sigintHandler);
    signal(SIGUSR1, exit_thread);
    init();
    main_controller();
    
    return 0;
}
