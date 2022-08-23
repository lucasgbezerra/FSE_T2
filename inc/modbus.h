#ifndef MODBUS_H_
#define MODBUS_H_

// Códigos do protocolo de comunicação
#define ADRRESS 0X01

#define REQUEST 0X23
#define SEND 0x16

#define REQUEST_INT 0xA1
#define REQUEST_FLOAT 0xA2
#define REQUEST_CHAR 0xA3
#define SEND_INT 0xB1
#define SEND_FLOAT 0xB2
#define SEND_CHAR 0xB3

void unzipMessage(int fid);
void requestData(int fid, unsigned char cod);
void show(unsigned char *buffer);
unsigned char option();
void init();

#endif
