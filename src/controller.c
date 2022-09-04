#include <stdio.h>
#include <softPwm.h>
#include <wiringPi.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>

#include "modbus.h"
#include "controller.h"
#include "control_lcd.h"
#include "bme280_controller.h"
#include "pid.h"
#include "utils.h"
#include "csv_file.h"
#include "menu_controller.h"

// Temperaturas
float ref_temperature = 0;
float internal_temperature = 0;
int signal_temp = 0;

int is_running = TRUE;
int is_on = FALSE;
int is_start = FALSE;
int init_countdown = FALSE;
int preheat = FALSE;
int is_heating;
int timer = 0;

st_menu menu;
pthread_t thread_timer;

void main_controller()
{
    int *ptr;
    while (is_running)
    {
        read_commands(); // 600 ms
        if (is_on)
        {
            request_temperatures();
            temperature_controller();
        }
    }
    pthread_join(thread_timer, (void **)&(ptr));
    // printf("saiu do loop\n");
}

void read_commands()
{
    int command = 0;

    int error = read_mensage(LE_COMANDO, &command);
    if (error != FAIL && command != 0)
    {
        // printf("Comando: %d\n", command);
        command_handle(command);
    }
}

void request_temperatures()
{
    if (read_mensage(SOLICITA_TEMP_INT, &internal_temperature) == FAIL)
    {
        printf("Erro: falha na leitura da Temperatura Interna\n");
        stop();
    }

    if (read_mensage(SOLICITA_TEMP_REF, &ref_temperature) == FAIL)
    {
        printf("Erro: falha na leitura da Temperatura de Referência\n");
        stop();
    }
}

void command_handle(int command)
{
    unsigned char data;

    if (command == LIGA && !is_on)
    {
        printf("Ligando...");
        data = LIGADO;
        is_on = TRUE;
        timer = 0;
        write_mensage(ENVIA_TEMPORIZADOR, &timer);
        write_mensage(ENVIA_ESTADO_SISTEMA, &data);
        printf("OK\n");
        pthread_create(&(thread_timer), NULL, (void *)timer_controller, NULL);
    }
    else if (command == DESLIGA)
    {
        data = DESLIGADO;
        is_running = FALSE;
        is_on = FALSE;
        is_start = FALSE;
        init_countdown = FALSE;

        cool_down(get_temperature());
        write_mensage(ENVIA_ESTADO_SISTEMA, &data);
        stop();
    }
    else if (is_on)
    {
        switch (command)
        {
        case INICIA:
            printf("Iniciando...");
            data = FUNCIONANDO;
            is_start = TRUE;
            preheat = TRUE;
            is_heating = TRUE;
            write_mensage(ENVIA_ESTADO_FUNCIONAMENTO, &data);
            printf("OK\n");
            break;

        case CANCELA:
            printf("Cancelando...");
            data = PARADO;
            is_start = FALSE;
            init_countdown = FALSE;
            preheat = FALSE;
            write_mensage(ENVIA_ESTADO_FUNCIONAMENTO, &data);
            printf("OK\n");
            break;

        case TEMPO_MAIS:
            timer++;
            write_mensage(ENVIA_TEMPORIZADOR, &timer);
            printf("Tempo: %d\n", timer);
            break;

        case TEMPO_MENOS:
            timer--;
            write_mensage(ENVIA_TEMPORIZADOR, &timer);
            printf("Tempo: %d\n", timer);
            break;
        case MENU:
            if (is_start)
                break;
            int idx = menu_controller(&menu);
            ref_temperature = menu.temperatures[idx];
            timer = menu.times[idx];

            write_mensage(ENVIA_SINAL_REF, &ref_temperature);
            write_mensage(ENVIA_TEMPORIZADOR, &timer);
            printf("Opção: %s\n", menu.options[menu.current]);
            break;
        default:
            break;
        }
    }
}
void init()
{
    open_modbus();
    lcd_init();
    write_lcd("INICIALIZANDO", "SISTEMA");
    init_bme280();
    pid_configura_constantes(KP, KI, KD);
    setup_gpio();
    create_file("data.csv");
    menu_init(&menu);
}

void cool_down(int room_temperature)
{

    char first_line[16];
    char second_line[16];

    is_start = TRUE;
    is_heating = FALSE;
    while (!compare_temperature(internal_temperature, room_temperature))
    {
        if (internal_temperature < room_temperature)
        {
            is_start = FALSE;
            break;
        }
        request_temperatures();
        temperature_controller();

    }
    is_start = FALSE;
    unsigned char data = PARADO;
    write_mensage(ENVIA_ESTADO_FUNCIONAMENTO, &data);
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

    printf("\nFinalizando programa...");
    shutdown_lcd();
    close_bme280();
    finish_pwm();
    close_modbus();
    printf("OK\n");
    clear_lcd();
    exit(0);
}

void show_lcd(int time_sec)
{
    char first_line[17];
    char second_line[17];

    int minutes = (timer * 60 - time_sec) / 60;
    int seconds = (timer * 60 - time_sec) % 60;

    if (is_heating || preheat)
    {
        sprintf(first_line, "TI:%.2f TR:%.1f", internal_temperature, ref_temperature);
        sprintf(second_line, "TIME: %dm %ds", minutes, seconds);
    }else{
        sprintf(first_line, "TI:%.2f TA:%.1f", internal_temperature, get_temperature());
        sprintf(second_line, "RESFRIANDO");
    }
    if (preheat)
    {
        sprintf(first_line, "TI:%.2f TR:%.1f", internal_temperature, ref_temperature);
        sprintf(second_line, "PRE-AQUECENDO");
    }

    write_lcd(first_line, second_line);
}

void shutdown_lcd()
{
    char first_line[17] = "DESLIGANDO";
    char second_line[17] = "SISTEMA";
    write_lcd(first_line, second_line);
}

void temperature_controller()
{

    if (!is_start)
        return;

    // request_temperatures();

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
        if (signal_temperature > FAN_LIMIT)
        {
            signal_temperature = FAN_LIMIT;
        }
        softPwmWrite(FAN_PIN, -signal_temperature);
        softPwmWrite(RESISTOR_PIN, 0);
    }
    signal_temp = (int)signal_temperature;
    printf("SIGNAL: %.2lf\n", signal_temperature);
    write_mensage(ENVIA_SINAL_CONTROLE, &signal_temp);
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
    struct timeval start;
    struct timeval end;
    while (is_on)
    {
        gettimeofday(&start, NULL);
        if (internal_temperature != 0)
            show_lcd(seconds);
        if (init_countdown)
        {
            seconds++;
        }
        if (timer <= 0 && init_countdown)
        {
            timer = 0;
            seconds = 0;
            init_countdown = FALSE;
            is_heating = FALSE;
        }
        if (seconds == 60)
        {
            seconds = 0;
            timer--;
        }
        save_on_file("data.csv", internal_temperature, ref_temperature, (float)get_temperature(), signal_temp);
        gettimeofday(&end, NULL);
        int t = 1000000 - (end.tv_usec - start.tv_usec);
        printf("T: %d\n", t);
        usleep(t);
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
    printf("KILL: %d\n", sig_num);
    exit(0);
}
