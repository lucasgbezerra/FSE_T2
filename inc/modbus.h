#ifndef MODBUS_H_
#define MODBUS_H_

// Códigos do protocolo de comunicação
#define ENDERECO 0X01

#define SOLICITA 0X23
#define ENVIA 0x16

#define LIGA 0x01
#define DESLIGA 0x02
#define INICIA 0x03
#define CANCELA 0x04
#define TEMPO_MAIS 0x06
#define TEMPO_MENOS 0x07
#define SOLICITA_TEMP_INT 0xC1
#define SOLICITA_TEMP_REF 0xC2
#define LE_COMANDO 0xC3
#define ENVIA_SINAL_CONTROLE 0xD1
#define ENVIA_SINAL_REF 0xD2
#define ENVIA_ESTADO 0xD3
#define MODO_CONTROLE 0xD4
static const unsigned char matricula[] = {5, 7, 7, 0};

void le_mensagem();
void escreve_mensagem(unsigned char *mensagem, unsigned char tam_mensagem, unsigned char codigo, unsigned char sub_codigo);

#endif
