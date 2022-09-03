#include <stdio.h>
#include <time.h>
#include <softPwm.h>
#include <wiringPi.h>
#include <pthread.h>
#include <signal.h>

#include "modbus.h"
#include "controller.h"
#include "control_lcd.h"
#include "bme280_controller.h"
#include "uart.h"
#include "pid.h"
#include "utils.h"

// #define TRUE (1 == 1)
// #define FALSE (!TRUE)

// Temperaturas
float ref_temperature = 0;
float internal_temperature = 0;

int is_running = TRUE;
int is_on = FALSE;
int is_start = FALSE;
int init_countdown = FALSE;
int preheat = FALSE;
int timer = 0;

pthread_t thread_timer;

void main_controller()
{
    int *ptr;
    while (is_running)
    {
        read_commands(); // 600 ms
        if (is_on)
        {
            // request_temperatures(); // 1,2s
            // show_lcd();
            // timer_controller();
            temperature_controller(TRUE);
            // printf("is on\n");
        }
    }
    pthread_join(thread_timer, (void **)&(ptr));
    printf("saiu do loop\n");
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
void turn_off()
{
    unsigned char data = DESLIGADO;
    is_running = FALSE;
    is_on = FALSE;
    is_start = FALSE;
    init_countdown = FALSE;
    printf("Envia Estado\n");
    write_lcd("RESFRIANDO", "SISTEMA");
    cool_down(get_temperature());
    write_mensage(ENVIA_ESTADO_SISTEMA, &data);
    stop();
}

void command_handle(int command)
{
    unsigned char data;

    if (command == LIGA && !is_on)
    {
        data = LIGADO;
        is_on = TRUE;
        timer = 0;
        write_mensage(ENVIA_TEMPORIZADOR, &timer);
        printf("Envia tempo\n");
        write_mensage(ENVIA_ESTADO_SISTEMA, &data);
        printf("Envia Estado\n");
        pthread_create(&(thread_timer), NULL, timer_controller, NULL);
    }
    else if (command == DESLIGA)
    {
        turn_off();
    }
    else if (is_on)
    {
        switch (command)
        {
        case INICIA:
            data = FUNCIONANDO;
            is_start = TRUE;
            preheat = TRUE;
            printf("Inicia\n");
            write_mensage(ENVIA_ESTADO_FUNCIONAMENTO, &data);
            break;

        case CANCELA:
            data = PARADO;
            is_start = FALSE;
            init_countdown = FALSE;
            preheat = FALSE;
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
    write_lcd("INICIALIZANDO", "SISTEMA");
    init_bme280();
    pid_configura_constantes(KP, KI, KD);
    setup_gpio();
    // clear_buffer(); // 500 ms
}

void cool_down(int room_temperature)
{
    if (is_on)
    {
        return;
    }
    is_start = TRUE;
    while (!compare_temperature(internal_temperature, room_temperature))
    {
        // request_temperatures();
        temperature_controller(FALSE);
    }

    is_start = FALSE;
    printf("TEMPERATURA ALCANÇADA");
}

void finish_pwm()
{
    softPwmWrite(RESISTOR_PIN, 0);
    softPwmWrite(FAN_PIN, 0);
    softPwmStop(RESISTOR_PIN);
    softPwmStop(FAN_PIN);
}
void stop()
{
    shutdown_lcd();
    printf("\n Finalizando programa... \n");
    close_bme280();
    finish_pwm();
    close_serial();
    clear_lcd();
    printf("\n Finalizando programa... \n");
    exit(0);
}

void show_lcd(int time)
{
    char first_line[17];
    char second_line[17];
    int minutes;
    int seconds;

    minutes = time / 60;
    seconds = time % 60;

    sprintf(first_line, "TI:%.2f TR:%.1f", internal_temperature, ref_temperature);

    if (preheat)
    {
        sprintf(second_line, "PRE-AQUECENDO");
    }
    else
    {
        sprintf(second_line, "TIME: %dm %ds", minutes, seconds);
    }

    write_lcd(first_line, second_line);
}

void shutdown_lcd()
{
    char first_line[17] = "DESLIGANDO";
    char second_line[17] = "SISTEMA";
    write_lcd(first_line, second_line);
}

void temperature_controller(int is_heating)
{

    if (!is_start)
        return;

    request_temperatures();
    if (is_heating)
    {
        if (preheat && compare_temperature(internal_temperature, ref_temperature))
        {
            init_countdown = TRUE;
            preheat = FALSE;
            printf("Temperatura alcançada %.2f\n", internal_temperature);
        }
        pid_atualiza_referencia(ref_temperature);
    }
    else
    {
        pid_atualiza_referencia((float)get_temperature());
    }

    // PID
    double signal_temperature = pid_controle(internal_temperature);

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
    int signal = (int)signal_temperature;
    printf("SIGNAL: %.2lf\n", signal_temperature);
    write_mensage(ENVIA_SINAL_CONTROLE, &signal);
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
    int seconds = 0;
    while (is_on)
    {
        if (internal_temperature != 0)
            show_lcd(timer * 60 - seconds);
        if (init_countdown)
        {
            printf("Tempo: %d\n", timer);
            seconds++;
        }
        if (seconds == 60)
        {
            seconds = 0;
            timer--;
        }
        if (timer <= 0)
        {
            timer = 0;
            seconds = 0;
            init_countdown = FALSE;
            is_start = FALSE;
        }
        usleep(850000);
    }
}

void exit_thread()
{
    pthread_exit(NULL);
}

void sigintHandler(int sig_num)
{

    // signal(SIGINT, sigintHandler);
    pthread_kill(thread_timer, SIGUSR1);
    stop();
    // printf("KILL: %d\n", sig_num);
    exit(0);
}
