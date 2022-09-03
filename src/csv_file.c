#include <stdio.h>
#include <time.h>

#include "csv_file.h"


void create_file(char *file_name)
{
    FILE *file = fopen(file_name, "w+");
    fprintf(file, "Data, Hora, Temperatura Interna, Temperatura Externa, Temperatura Usuario, Valor Acionamento\n");
    fclose(file);
}

// void load_file()
// {
//     FILE *file = fopen(file_name, "r");
//     if (!file)
//     {
//         printf("Arquivo não existe\n");
//     }

//     char buffer[1024];
//     int row = 0;
//     int column = 0;
//     while (fgets(buffer, 1024, file))
//     {
//         column = 0;
//         row++;

//         if (row == 1)
//             continue;

//         char *value = strtok(buffer, ", ");
//         while (value)
//         {
//             if (column == 0)
//             {
//                 dados[row - 2].tempo = strtof(value, NULL);
//             }

//             if (column == 1)
//             {
//                 dados[row - 2].temp = strtof(value, NULL);
//             }

//             value = strtok(NULL, ", ");
//             column++;
//         }
//     }
//     fclose(file);
// }

void save_on_file(char *file_name, float ti, float tr, float ta, int signal)
{
    struct tm  *date;

    if (ti == 0)
        return;

    FILE *file;
    time_t seconds;

    time(&seconds);
    date = localtime(&seconds);

    file = fopen(file_name, "a");

    fprintf(file, "%d/%d/%d, ", date->tm_mday, date->tm_mon + 1, date->tm_year + 1900);
    fprintf(file, "%d:%d:%d, ", date->tm_hour, date->tm_min, date->tm_sec);
    fprintf(file, "%.2lf, %.2lf, %d\n", ta, tr, signal);

    fclose(file);
}