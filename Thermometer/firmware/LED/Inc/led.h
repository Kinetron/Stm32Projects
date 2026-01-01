#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "gpio.h"
#include "main.h"

_BEGIN_STD_C

#define LED_MATRIX_SIZE 25
#define OFF_LED_SEGMENT_VALUE 20

#define BLINK_LED_INTERVAL 40

#define MINUS_LATTER 10

#define E_LATTER 15
#define F_LATTER 16
#define L_LATTER 17
#define A_LATTER 11
#define N_LATTER 21
#define O_LATTER 22
#define R_LATTER 23
#define U_LATTER 24

extern const uint8_t ledMatrix[LED_MATRIX_SIZE];

void initLed();

//Convert dig to 7 seg led matrix.
void digToSegments(uint8_t dig, uint8_t point);

//Disables zeros in the higher registers. If need.
void offZeros(uint8_t *arr);

//Generates an error code
void createErrorCodes(uint16_t value);

//Run logic in the main loop of the program.
void ledDisplayHandler(uint16_t digit);

//Generate a dynamic indication
void dynamicIndication();

//Convert hex number to 3 decimal digit.
void hexToDec(uint16_t value);

//Soft timer.
bool blinkLedTimerHandler();

//On/Off points.
void setPoints(uint8_t data);

//On/Off blink digits.
void blinkDigits(bool on);

//On/Off minus in 4 digits.
void onMinus(bool on);

_END_STD_C