#include "crc.h"
#include "modbus.h"
#include "uart.h"

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static const unsigned char matricula[] = {5, 7, 7, 0};
unsigned char codigo;
void unzipMessage(int fid)
{
	unsigned char *rx_buffer;
	int rx_length = 0;
	int size = 256;
	short crc_recv;

	rx_buffer = malloc(size);
	rx_length = readSerial(fid, rx_buffer, size);
	
	int length = rx_length - sizeof(short);
	memcpy(&crc_recv, &rx_buffer[length], sizeof(short));
	short crc = calcula_CRC(rx_buffer, length);
	
	
	if (crc_recv == crc)
		show(rx_buffer);

	free(rx_buffer);
}

void show(unsigned char *buffer)
{

	if (codigo == REQUEST_INT)
	{
		int dado;
		memcpy(&dado, &buffer[2], sizeof(int));
		printf("Int: %d\n", dado);
	}
	if (codigo == REQUEST_FLOAT)
	{
		float dado;
		memcpy(&dado, &buffer[2], sizeof(float));
		printf("Float: %f\n", dado);
	}
	if(codigo == REQUEST_CHAR){
		char *dado;
		int tam = (int)buffer[3];

		dado = malloc(tam);
		memcpy(dado, &buffer[4], tam);
		printf("String: %s\n", dado);

		free(dado);
	}
}

void requestData(int fid, unsigned char cod)
{
	unsigned char *buffer = malloc(13);
	int idx = 0;
	short crc;

	buffer[idx++] = ADRRESS;
	buffer[idx++] = REQUEST;
	buffer[idx++] = cod;
	memcpy(&buffer[idx], matricula, sizeof(matricula));
	idx += sizeof(matricula);

	crc = calcula_CRC(buffer, idx);
	memcpy(&buffer[idx], &crc, sizeof(short));
	idx += sizeof(short);

	writeSerial(fid, buffer, idx);
	free(buffer);
}

unsigned char option()
{
	int op;

	printf("1 - Solicitar INT\n");
	printf("2 - Solicitar FLOAT\n");
	printf("3 - Solicitar CHAR\n");
	printf("4 - Enviar INT\n");
	printf("5 - Enviar FLOAT\n");
	printf("6 - Enviar CHAR\n");
	printf("Escolha uma opção: ");
	scanf("%d", &op);

	switch (op)
	{
	case 1:
		return REQUEST_INT;
	case 2:
		return REQUEST_FLOAT;
	case 3:
		return REQUEST_CHAR;
	case 4:
		return SEND_INT;
	case 5:
		return SEND_FLOAT;
	case 6:
		return SEND_CHAR;
	default:
		return REQUEST_INT;
		break;
	}
}

void init()
{
	// unsigned char cod;
	int fid = -1;

	fid = openSerial();
	codigo = option();

	requestData(fid, codigo);
	if (fid != -1)
	{
		sleep(1);
		unzipMessage(fid);
		close(fid);
	}
}

