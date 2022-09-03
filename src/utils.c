#include "utils.h"

int round_temperature(double number)
{
    return (number >= 0) ? (int)(number + 0.5) : (int)(number - 0.5);
}

int compare_temperature(float temp1, float temp2)
{
    return (round_temperature(temp1) == round_temperature(temp2));
}