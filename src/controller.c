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
int control_signal = 0;

int is_running = TRUE;
int is_on = FALSE;
int is_start = FALSE;
int init_countdown = FALSE;
int is_waiting = FALSE;
int is_heating = FALSE;
int is_cooling = FALSE;
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
        printf("Desligando...\n");
        data = DESLIGADO;

        init_countdown = FALSE;
        is_heating = FALSE;
        is_cooling = TRUE;
        cool_down(get_temperature());
        is_start = FALSE;
        is_waiting = FALSE;
        is_running = FALSE;
        is_on = FALSE;
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
            // preheat = TRUE;
            is_waiting = FALSE;
            is_heating = TRUE;
            write_mensage(ENVIA_ESTADO_FUNCIONAMENTO, &data);
            printf("OK\n");
            break;

        case CANCELA:
            printf("Parando...");
            data = PARADO;
            is_start = FALSE;
            init_countdown = FALSE;
            // preheat = FALSE;
            is_waiting = TRUE;
            is_heating = FALSE;
            is_cooling = FALSE;
            write_mensage(ENVIA_ESTADO_FUNCIONAMENTO, &data);
            printf("OK\n");
            break;

        case TEMPO_MAIS:
            timer++;
            write_mensage(ENVIA_TEMPORIZADOR, &timer);
            printf("Tempo: %d\n", timer);
            break;

        case TEMPO_MENOS:
            if (timer <= 0)
                break;
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

void control_actuators(int fan, float resistor)
{
    softPwmWrite(FAN_PIN, fan);
    softPwmWrite(RESISTOR_PIN, resistor);
    write_mensage(ENVIA_SINAL_CONTROLE, &control_signal);
}

void cool_down()
{
    printf("Resfriando...\n");
    float room_temperature = get_temperature();
    while (internal_temperature > room_temperature)
    {
        request_temperatures();
        room_temperature = get_temperature();

        control_signal = -MAX_ACTUATOR;
        control_actuators(MAX_ACTUATOR, 0);
    }
    control_signal = 0;
    control_actuators(0, 0);

    unsigned char data = PARADO;
    is_start = FALSE;
    is_cooling = FALSE;
    write_mensage(ENVIA_ESTADO_FUNCIONAMENTO, &data);
    write_mensage(ENVIA_TEMPORIZADOR, &timer);
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

    if (is_start && is_heating && !init_countdown)
    {
        sprintf(first_line, "TI:%.2f TR:%.1f", internal_temperature, ref_temperature);
        sprintf(second_line, "PRE-AQUECENDO");
    }
    else if (is_cooling)

    {
        sprintf(first_line, "TI:%.2f TA:%.1f", internal_temperature, get_temperature());
        sprintf(second_line, "RESFRIANDO");
    }
    else if (is_waiting)
    {
        sprintf(first_line, "TI:%.2f TR:%.1f", internal_temperature, ref_temperature);
        sprintf(second_line, "TIME: %dm %ds STOP", minutes, seconds);
    }
    else
    {
        sprintf(first_line, "TI:%.2f TR:%.1f", internal_temperature, ref_temperature);
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

void temperature_controller()
{

    if (is_waiting && control_signal != 0)
    {
        control_signal = 0;
        control_actuators(0, 0);
        printf("AQUI\n");
        return;
    }
    if (!is_start)
        return;

    // request_temperatures();
    if (!is_heating)
    {
        cool_down(get_temperature());
        is_heating = TRUE;
        return;
    }

    // if (preheat && compare_temperature(internal_temperature, ref_temperature))
    if (is_heating && !init_countdown && compare_temperature(internal_temperature, ref_temperature))
    {
        init_countdown = TRUE;
        // preheat = FALSE;
        printf("Temperatura alcançada %.2f\n", internal_temperature);
    }
    pid_atualiza_referencia(ref_temperature);

    // PID
    double signal_temperature = pid_controle(internal_temperature);
    control_signal = (int)signal_temperature;

    if (signal_temperature >= 0)
    {
        // softPwmWrite(RESISTOR_PIN, signal_temperature);
        // softPwmWrite(FAN_PIN, 0);
        control_actuators(0, signal_temperature);
    }
    else
    {
        if (signal_temperature > FAN_MIN_LIMIT)
        {
            signal_temperature = FAN_MIN_LIMIT;
        }
        // softPwmWrite(FAN_PIN, -signal_temperature);
        // softPwmWrite(RESISTOR_PIN, 0);
        control_actuators(-signal_temperature, 0);
    }
    printf("SIGNAL: %.2lf\n", signal_temperature);
    // write_mensage(ENVIA_SINAL_CONTROLE, &control_signal);
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
            is_cooling = TRUE;
        }
        if (seconds == 60)
        {
            seconds = 0;
            timer--;
        }
        gettimeofday(&end, NULL);
        save_on_file("data.csv", internal_temperature, ref_temperature, get_temperature(), control_signal);
        int t = 1000000 - (end.tv_usec - start.tv_usec);
        if (t > 1000000)
            t = 830000;
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
