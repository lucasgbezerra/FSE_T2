#include <stdio.h>
#include <time.h>
#include <softPwm.h>
#include <wiringPi.h>

#include "modbus.h"
#include "controller.h"
#include "control_lcd.h"
#include "bme280_controller.h"
#include "uart.h"
#include "pid.h"

#define TRUE (1 == 1)
#define FALSE (!TRUE)

// Temperaturas
float ref_temperature = 0;
float internal_temperature;

int is_running = TRUE;
int is_on = FALSE;
int is_start = FALSE;
int init_countdown = FALSE;
int timer = 0;
time_t start_time;

void main_controller()
{
    while (is_running)
    {
        read_commands(); // 600 ms
        if (is_on)
        {
            request_temperatures(); // 1,2s
            show_lcd();
            timer_controller();
            temperature_controller();
        }
    }
    stop();
}

void read_commands()
{
    int command = 0;

    int error = read_mensage(LE_COMANDO, &command);
    if (error != FAIL && command != 0)
    {
        printf("Comando: %d\n", command);
        command_handle(command);
    }
}
// void clear_buffer()
// {
//     int tmp;
//     read_mensage(LE_COMANDO, &tmp);
//     usleep(100000);
// }

void request_temperatures()
{

    // clear_buffer();
    if (read_mensage(SOLICITA_TEMP_INT, &internal_temperature) == FAIL)
    {
        printf("Falhou INT\n");
        stop();
    }
    // printf("INT temp: %.2f\n", internal_temperature);

    if (read_mensage(SOLICITA_TEMP_REF, &ref_temperature) == FAIL)
    {
        printf("Falhou REF\n");
        stop();
    }
    // printf("REF temp: %.2f\n", ref_temperature);
}

void command_handle(int command)
{
    unsigned char data;
    printf("ON: %d\n", is_on);

    if (command == LIGA && !is_on)
    {
        data = LIGADO;
        is_on = TRUE;
        timer = 0;
        write_mensage(ENVIA_TEMPORIZADOR, &timer);
        printf("Envia tempo\n");
        write_mensage(ENVIA_ESTADO_SISTEMA, &data);
        printf("Envia Estado\n");
    }
    else if (command == DESLIGA)
    {
        data = DESLIGADO;
        is_running = FALSE;
        is_on = FALSE;
        is_start = FALSE;
        printf("Envia Estado\n");
        write_mensage(ENVIA_ESTADO_SISTEMA, &data);
    }
    else if (is_on)
    {
        switch (command)
        {
        case INICIA:
            data = FUNCIONANDO;
            is_start = TRUE;
            printf("Inicia\n");
            write_mensage(ENVIA_ESTADO_FUNCIONAMENTO, &data);
            start_time = time(NULL);
            break;

        case CANCELA:
            data = PARADO;
            is_on = FALSE;
            is_start = FALSE;
            printf("Cancela\n");
            write_mensage(ENVIA_ESTADO_FUNCIONAMENTO, &data);
            break;

        case TEMPO_MAIS:
            timer++;
            printf("Timer %d\n", timer);
            write_mensage(ENVIA_TEMPORIZADOR, &timer);
            break;

        case TEMPO_MENOS:
            timer--;
            printf("Timer %d\n", timer);
            write_mensage(ENVIA_TEMPORIZADOR, &timer);
            break;

        default:
            break;
        }
    }
}
void init()
{
    open_serial();
    lcd_init();
    init_bme280();
    pid_configura_constantes(KP, KI, KD);
    setup_gpio();
    // clear_buffer(); // 500 ms
}

void stop()
{
    unsigned char data = DESLIGADO;

    shutdown_lcd();
    close_bme280();
    softPwmWrite(RESISTOR_PIN, 0);
    softPwmWrite(FAN_PIN, 0);
    softPwmStop(RESISTOR_PIN);
    softPwmStop(FAN_PIN);
    write_mensage(ENVIA_ESTADO_FUNCIONAMENTO, &data);
    write_mensage(ENVIA_ESTADO_SISTEMA, &data);
    printf("\n Finalizando programa... \n");
    is_running = FALSE;
    close_serial();
}

void show_lcd()
{
    char first_line[17];
    char second_line[17];
    time_t delta_time;
    int minutes;
    int seconds;

    if (is_start)
    {
        delta_time = time(NULL) - start_time;
        minutes = (timer * 60 - delta_time) / 60;
        seconds = (timer * 60 - delta_time) % 60;
    }
    else
    {
        minutes = timer;
        seconds = 0;
    }

    sprintf(first_line, "TI:%.2f TR:%.1f", internal_temperature, ref_temperature);

    sprintf(second_line, "TIME: %dm %ds", minutes, seconds);

    write_lcd(first_line, second_line);
}

void shutdown_lcd()
{
    char first_line[17] = "DESLIGANDO";
    char second_line[17] = "SISTEMA";
    write_lcd(first_line, second_line);
    usleep(500000);
    clear_lcd();
}

void temperature_controller()
{

    if (!is_start)
        return;

    request_temperatures();
    printf("Temperaturas\n");
    // COMPARA TI E TR
    compare_tr_ti(internal_temperature);

    // PID
    pid_atualiza_referencia(ref_temperature);
    double signal_temperature = pid_controle(internal_temperature);

    printf("SIGNAL: %.2lf\n", signal_temperature);

    if (signal_temperature >= 0)
    {
        softPwmWrite(RESISTOR_PIN, signal_temperature);
        softPwmWrite(FAN_PIN, 0);
    }
    else
    {
        if (signal_temperature >= FAN_LIMIT)
        {
            signal_temperature = -1 * FAN_LIMIT;
        }
        softPwmWrite(FAN_PIN, signal_temperature);
        softPwmWrite(RESISTOR_PIN, 0);
    }
    write_mensage(ENVIA_SINAL_CONTROLE, &internal_temperature);
}

void compare_tr_ti()
{
    printf("Entrou aqui\n");
    if (round_temperature(internal_temperature) == round_temperature(ref_temperature) && !init_countdown)
    {
        init_countdown = TRUE;
        printf("Temperatura alcançada %.2f\n", internal_temperature);
        start_time = time(NULL);
    }
}
int round_temperature(double number)
{
    return (number >= 0) ? (int)(number + 0.5) : (int)(number - 0.5);
}
void setup_gpio()
{
    wiringPiSetup();
    pinMode(RESISTOR_PIN, OUTPUT);
    pinMode(FAN_PIN, OUTPUT);
    softPwmCreate(RESISTOR_PIN, 0, 100);
    softPwmCreate(FAN_PIN, 0, 100);
}

void timer_controller()
{

    time_t delta_time = time(NULL) - start_time;
    if (!init_countdown && (delta_time > timer * 60) && (timer * 60 - delta_time) < 0)
        return;

    printf("Timer\n");
    if (init_countdown && delta_time >= timer)
    {
        init_countdown = FALSE;
        is_start = FALSE;
    }
}
