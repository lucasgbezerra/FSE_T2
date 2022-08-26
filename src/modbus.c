#include "crc.h"
#include "modbus.h"
#include "uart.h"

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

unsigned char codigo;

void escreve_mensagem(unsigned char *mensagem, unsigned char tam_mensagem, unsigned char codigo, unsigned char sub_codigo)
{
	int idx = 0;
	unsigned char *buffer = malloc(9);

	// Contruindo mensagem
	buffer[idx++] = ENDERECO;
	buffer[idx++] = codigo;
	buffer[idx++] = sub_codigo;
	memcpy(&buffer[idx], matricula, sizeof(matricula));
	idx += sizeof(matricula);

	if (mensagem != NULL)
	{

		printf("%s\n", mensagem);
		buffer = realloc(buffer, sizeof(char) * tam_mensagem);
		memcpy(&buffer[idx], mensagem, tam_mensagem);
		idx += tam_mensagem;
	}
	short crc = calcula_CRC(buffer, idx);

	memcpy(&buffer[idx], &crc, sizeof(short));
	idx += sizeof(short);
	
	// for (int i = 0; i < idx; i++)
	// 	printf("%2x ", buffer[i]);
	// printf("\n");
	
	write_serial(buffer, idx);
	free(buffer);
}

void le_mensagem()
{
	unsigned char *rx_buffer;
	int rx_length = 0;
	int size = 9;
	short crc_recv;

	rx_buffer = malloc(size);

	rx_length = read_serial(rx_buffer, size);


	int length = rx_length - sizeof(short);
	memcpy(&crc_recv, &rx_buffer[length], sizeof(short));
	short crc = calcula_CRC(rx_buffer, length);

	if (crc_recv == crc){
		float temp = 0;
		memcpy(&temp, &rx_buffer[3], 4);
		printf("TEMP: %2f\n", temp);

	}

	// for (int i = 0; i < 9; i++)
	// 	printf("%2x ", rx_buffer[i]);
	// printf("\n");

	free(rx_buffer);
}
