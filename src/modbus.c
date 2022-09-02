#include "crc.h"
#include "modbus.h"
#include "uart.h"

#include <unistd.h>
#include <string.h>
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
		// if (sub_code == ENVIA_SINAL_CONTROLE || sub_code == ENVIA_TEMPORIZADOR)
		// {
		// 	printf("%d\n", (*(int *)data));
		// }
		// else if (sub_code == ENVIA_SINAL_REF)
		// {
		// 	printf("%.2f\n", *((float *)data));
		// }
		// else
		// {
		// 	printf("%c\n", (*(char *)data));
		// }
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
	// if (sub_code == SOLICITA_TEMP_INT)
	// {
	// 	size = build_mensage(buffer, SOLICITA, sub_code, NULL, 0);
	// }
	if (sub_code == ENVIA_SINAL_CONTROLE || sub_code == ENVIA_SINAL_REF || sub_code == ENVIA_TEMPORIZADOR)
	{
		size = build_mensage(buffer, ENVIA, sub_code, data, sizeof(int));
	}
	else if (sub_code == ENVIA_ESTADO_SISTEMA || sub_code == ENVIA_ESTADO_FUNCIONAMENTO)
	{
		size = build_mensage(buffer, ENVIA, sub_code, data, BYTE);
	}

	// printf("Escreve Mensagem: \n");
	// for (int i = 0; i < size; i++)
	// 	printf("%2x ", buffer[i]);
	// printf("\n");

	write_serial(buffer, size);
	free(buffer);
}

int read_mensage(unsigned char sub_code, void *data)
{
	int rx_length = 0;
	int tx_length = 0;
	int size = 9;
	short crc_recv;
	unsigned char *rx_buffer = malloc(size);
	unsigned char *tx_buffer = malloc(13);

	tx_length = build_mensage(tx_buffer, SOLICITA, sub_code, NULL, 0);
	write_serial(tx_buffer, tx_length);
	usleep(500000);

	rx_length = read_serial(rx_buffer, size);
	int length = rx_length - sizeof(short);
	memcpy(&crc_recv, &rx_buffer[length], sizeof(short));
	short crc = calcula_CRC(rx_buffer, length);
	;

	// printf("Lê Mensagem: \n");
	// for (int i = 0; i < 9; i++)
	// 	printf("%2x ", rx_buffer[i]);
	// printf("\n");

	if (crc_recv == crc && sub_code == rx_buffer[2])
	{
		memcpy(data, &rx_buffer[3], 4 * BYTE);
		free(rx_buffer);
		free(tx_buffer);
		return rx_length;
	}
	else
	{
		free(rx_buffer);
		free(tx_buffer);
		return FAIL;
	}
}
