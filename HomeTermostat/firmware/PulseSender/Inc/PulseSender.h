#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "gpio.h"
#include "main.h"

#define DATA_PAUSE_INTERVAL 2

_BEGIN_STD_C

void pulseSenderHandler();

//Generates a pause after sending a burst of pulses
void outDataPause();

_END_STD_C