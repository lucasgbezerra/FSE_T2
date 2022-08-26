#ifndef UART_H_
#define UART_H_

// Códigos do protocolo de comunicação

void write_serial(unsigned char *tx_buffer, int size);
int read_serial(unsigned char *rx_buffer, int size_buffer);
void open_serial();
void close_serial();

#endif
