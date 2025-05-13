#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "gpio.h"
#include "stdio.h"
#include "main.h"
#include "ds18b20.h"
#include "ErrorCodes.h"

#define DELAY_USE_SLOW_TEMPERATURE_INTERVAL 60 //The time after run when reads temperature overy second.
#define READ_TEMPERATURE_INTERVAL 20 //every 20 second read temperature.

#define OW_LINE_PAUSE 2

//temperature control
struct tControlData
{
    uint8_t initCnt;

    int sensorQuantity;
    float tempature;
    bool isInit;
    uint8_t errorCode;

    uint8_t offPauseCnt;
    bool setLowOw;
};

//Measures the intervals between read temperature sensors.
struct requestTemperatureTimer
{
   uint8_t counter;
   uint8_t initCounter;
   
   bool disabled;
   bool isNoFirstRun;
};

//Only for test
struct LedTest
{
    uint8_t cnt;
    float temperature;
};


_BEGIN_STD_C

//Generates the data displayed on the led.
uint16_t getDisplayData(void);

uint16_t createErrorCode(enum ErrorCodes err);

//Read temperature from sensors.
void readTemperature();

//Convert temperature to int, switch poin, add minus.
uint16_t calculateTemperature(float temperature);

//Every REQUEST_TEMPERATURE_INTERVAL return true. 
bool requestTemperatureTimer();

//On/Off decimal point if t < 99.9 or t > 99.9.
uint8_t switchDecimalPoint(float temperature);

//Generates different temperature options, including negative ones to check the display.
float displayTemperatureTest();

bool offOwLinePause(); 

void reInitBus();

_END_STD_C