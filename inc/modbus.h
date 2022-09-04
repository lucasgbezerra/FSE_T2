#ifndef MODBUS_H_
#define MODBUS_H_

// Códigos do protocolo de comunicação
#define ENDERECO 0X01

#define SOLICITA 0X23
#define ENVIA 0x16

#define SOLICITA_TEMP_INT 0xC1
#define SOLICITA_TEMP_REF 0xC2
#define LE_COMANDO 0xC3
#define ENVIA_SINAL_CONTROLE 0xD1
#define ENVIA_SINAL_REF 0xD2
#define ENVIA_ESTADO_SISTEMA 0xD3
#define ENVIA_ESTADO_FUNCIONAMENTO 0xD5
#define ENVIA_TEMPORIZADOR 0xD6

#define FAIL -1
static const unsigned char matricula[] = {5, 7, 7, 0};

// void le_mensagem();
int read_mensage(unsigned char sub_code, void *data);
int build_mensage(unsigned char *buffer, unsigned char code, unsigned char sub_code, void *data, int size_data);
void write_mensage(unsigned char sub_code, void *data);
void close_modbus();
void open_modbus();
#endif
