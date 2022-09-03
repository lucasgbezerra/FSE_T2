#ifndef CONTROLLER_H_
#define CONTROLLER_H_

// Comandos
#define LIGA 0x01
#define DESLIGA 0x02
#define INICIA 0x03
#define CANCELA 0x04
#define TEMPO_MAIS 0x05
#define TEMPO_MENOS 0x06

#define DESLIGADO 0X00
#define LIGADO 0X01
#define PARADO 0X00
#define FUNCIONANDO 0X01

#define KP 30.0
#define KI 0.2
#define KD 400.0

#define RESISTOR_PIN 4
#define FAN_PIN 5
#define FAN_LIMIT -40

void main_controller();
void read_commands();
void turn_off();
void command_handle(int command);
void request_temperatures();
void init();
void show_lcd();
void stop();
void shutdown_lcd();
void temperature_controller(int is_heating);
void compare_tr_ti();
int round_temperature(double number);
void setup_gpio();
void timer_controller();
void sigintHandler(int sig_num);
void exit_thread();
void finish_pwm();
void cool_down(int room_temperature);

#endif