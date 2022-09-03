#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>


#include "modbus.h"
#include "uart.h"
#include "controller.h"
#include "control_lcd.h"
#include "bme280_controller.h"

// pthread_t thread_commands;
// void teste(){
//     float temperature;
//     float tr;
//     int error = read_mensage(SOLICITA_TEMP_INT, &temperature);
//     if (error != FAIL)
//     {
//         printf("TI :%2f\n", temperature);
//     }
//     else
//     {
//         printf("TI :%d\n", error);
//     }
//     sleep(1);
//     write_mensage(ENVIA_SINAL_REF, &tr);
// }

void sigintHandler(int sig_num)
{

    // signal(SIGINT, sigintHandler);
    // pthread_kill(thread_commands, SIGUSR1);
    stop();
    exit(0);
}

int main()
{
    signal(SIGINT, sigintHandler);
    // signal(SIGUSR1, exit_thread);
    init();
    // pthread_create(&thread_commands, NULL, read_commands, NULL);
    main_controller();

    return 0;
}
