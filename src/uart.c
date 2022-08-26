#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include "uart.h"

int fid = -1;

void write_serial(unsigned char *tx_buffer, int size)
{
    if (fid != -1)
        write(fid, tx_buffer, size);
}

int read_serial(unsigned char *rx_buffer, int size_buffer)
{
    if (fid == -1){
        printf("FID -1\n");
        return -1;
    }
    int size = read(fid, rx_buffer, size_buffer);

    return size;
}
void open_serial()
{
    fid = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);

    if (fid == -1)
    {
        printf("Erro - Não foi possível iniciar a UART.\n");
        // return -1;
    }

    printf("UART inicializada!\n");

    struct termios options;
    tcgetattr(fid, &options);
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(fid, TCIFLUSH);
    tcsetattr(fid, TCSANOW, &options);

    // return fid;
}

void close_serial(){
    fid = -1;
    close(fid);
}
