#ifndef UART_H_
#define UART_H_

// Códigos do protocolo de comunicação
void writeSerial(int fid, unsigned char *tx_buffer, int size);
int readSerial(int fid, unsigned char *rx_buffer, int size_buffer);
int openSerial();

#endif
