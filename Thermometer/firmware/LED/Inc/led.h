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

const uint8_t ledMatrix[LED_MATRIX_SIZE] =
{
  0x0A, //0
  0x7B, //1
  0x26, //2
  0x62, //3
  0x53, //4
  0xC2, //5
  0x82, //6
  0x6b, //7
  0x02, //8
  0x42, //9
  0xF7, //-
  0x03, //A 
  0x92, //b
  0x8E, //C
  0x32, //d
  0x86, //E
  0x87, //F
  0x9E, //L
  0x8F, //L mirror
  0x13, //H
  0xFD, //off segment
  0xB3, //n
  0xB2, //o
  0xb7, //r
  0xBA //u
};

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