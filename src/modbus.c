#include "crc.h"
#include "modbus.h"
#include "uart.h"

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define BYTE 1
unsigned char code;

int build_mensage(unsigned char *buffer, unsigned char code, unsigned char sub_code, void *data, int size_data)
{
	int idx = 0;
	// Contruindo msg
	buffer[idx++] = ENDERECO;
	buffer[idx++] = code;
	buffer[idx++] = sub_code;

	memcpy(&buffer[idx], matricula, sizeof(matricula));
	idx += sizeof(matricula);
	if (data != NULL)
	{
		memcpy(&buffer[idx], data, size_data);
		idx += size_data;
	}
	short crc = calcula_CRC(buffer, idx);

	memcpy(&buffer[idx], &crc, sizeof(short));
	idx += sizeof(short);

	return idx;
}

void write_mensage(unsigned char sub_code, void *data)
{
	int size = 0;
	unsigned char *buffer = malloc(13);

	if (sub_code == ENVIA_SINAL_CONTROLE || sub_code == ENVIA_SINAL_REF || sub_code == ENVIA_TEMPORIZADOR)
	{
		size = build_mensage(buffer, ENVIA, sub_code, data, sizeof(int));
	}
	else if (sub_code == ENVIA_ESTADO_SISTEMA || sub_code == ENVIA_ESTADO_FUNCIONAMENTO)
	{
		size = build_mensage(buffer, ENVIA, sub_code, data, BYTE);
	}

	write_serial(buffer, size);
	usleep(600000); // 600 ms
	if (sub_code != ENVIA_SINAL_CONTROLE && sub_code != ENVIA_SINAL_REF)
	{
		unsigned char rx_buffer[9];
		read_serial(rx_buffer, size);
	}
	free(buffer);
}

int read_mensage(unsigned char sub_code, void *data)
{
	int rx_length = 0;
	int tx_length = 0;
	int size = 9;
	int error = 0;
	short crc_recv;
	unsigned char rx_buffer[9];
	unsigned char tx_buffer[13];

	tx_length = build_mensage(tx_buffer, SOLICITA, sub_code, NULL, 0);

	while (error < 3)
	{
		write_serial(tx_buffer, tx_length);
		usleep(600000); // 600 ms

		rx_length = read_serial(rx_buffer, size);
		int length = rx_length - sizeof(short);

		if (rx_length != -1)
		{
			memcpy(&crc_recv, &rx_buffer[length], sizeof(short));
			short crc = calcula_CRC(rx_buffer, length);

			if (crc_recv == crc && sub_code == rx_buffer[2])
			{
				memcpy(data, &rx_buffer[3], 4 * BYTE);
				break;
			}
			else
			{
				printf("ERROR: %d\n", error);
				error++;
				rx_length = -1;
			}
		}
	}

	return rx_length;
}
void open_modbus()
{
	int fid = open_serial();
	if (fid == -1)
	{
		printf("ERRO: N??o foi poss??vel abrir comuni????o com a UART\n");
		exit(0);
	}
}
void close_modbus()
{
	close_serial();
}