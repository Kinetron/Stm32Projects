#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gpio.h"
#include "stdio.h"
#include "main.h"
#include "stm32_eeprom.h"
#include "ErrorCodes.h"

#define FLASH_INIT_FLAG 0x37
#define FLASH_INIT_FLAG_POS 0

_BEGIN_STD_C

//After start read system params and decides on the mode of operation.
uint16_t initWorkMode(void);

//Generates the data displayed on the led.
uint16_t prepareDisplay(void);

//No config hot water sensor.
uint16_t emptySettingsModeDisplay(uint8_t count);
uint16_t createErrorCode(enum ErrorCodes err);

_END_STD_C