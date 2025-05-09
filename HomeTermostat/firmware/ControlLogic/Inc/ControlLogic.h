#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "gpio.h"
#include "stdio.h"
#include "main.h"
#include "stm32_eeprom.h"
#include "ds18b20.h"
#include "ErrorCodes.h"

#define MAX_SENSOR_QUANTITY 2
#define MAX_ENCODER_VALUE 999
#define DELAY_BEFORE_WRITE_TO_FLASH 5
#define COUNT_TICKS_BUTTON_IS_PRESSED 50 //how long does the tap btn count as active
#define DELAY_BEFORE_WRITE_ROOM_ADDR 30 //If the device is not initialized and one sensor is connected, then after this time it will be considered as a room sensor.

#define DELAY_USE_SLOW_TEMPERATURE_INTERVAL 10 //The time after run when reads temperature overy second.
#define READ_TEMPERATURE_INTERVAL 9 //every 7 second read temperature.

#define FLASH_T0_ADDRESS_POS 0  //8byte ds18b20 address hot water temperature sensor.
#define FLASH_SET_TEMPERATURE_POS 9 //Set room temperature. 2 byte

#define DEFAULT_SET_TEMPERATURE 300//251  //25.1 degree celsius
#define DS18B20_ADDR_SIZE 8

#define HYSTERESIS 10 //1.0 degree celsius
#define THRESHOLD_HOT_WATER 390 //44.0 if hot water t < THRESHOLD_HOT_WATER heater off. It need for protect pump.

#define PERIOD_SHOW_SETTINSG 3

//temperature control
struct tControlData
{
    uint16_t roomSetedTemperature;
    int sensorQuantity;
    bool ignoreHotWaterTemperature; 

    Ds18b20Sensor_t data[MAX_SENSOR_QUANTITY]; //For room and hot water sensor.
    uint8_t roomAddress[DS18B20_ADDR_SIZE];
    uint16_t tempatureBuff[MAX_SENSOR_QUANTITY];
    uint16_t roomTempature;
    uint16_t hotWaterTempature;

    bool heaterOn;
};

struct encoderData
{
  uint32_t lastEncoderValue;
  uint16_t value;
  bool hasRotate;
};

struct displayMode
{
   bool showEncoderData;
   uint8_t encoderDisplayTimer;

   //For press button.
   bool pressEncoderBtn;
   uint8_t pressEncoderBtnDisplayTimer;
};

//Delay before save to flash.
struct flashTimer
{
   bool enable;
   uint8_t timer;
   bool dataWillSave;
};

//Checks if the encoder button is pressed.
struct buttonData
{
    uint8_t pressedInterval;
    bool isPressed;
};

struct initState
{
    uint8_t counter;
    uint8_t initMode;

    bool hasRoomSensorAddress;
};

//Measures the intervals between read temperature sensors.
struct requestTemperatureTimer
{
   uint8_t counter;
   uint8_t initCounter;
   
   bool disabled;
   bool isNoFirstRun;
};

_BEGIN_STD_C

//After start read system params and decides on the mode of operation.
uint16_t initWorkMode(void);

//Generates the data displayed on the led.
uint16_t prepareDisplay(void);

//No config hot water sensor.
uint16_t emptySettingsModeDisplay();
uint16_t createErrorCode(enum ErrorCodes err);

//Set hot water sensor address ?
uint8_t isSetRoomSensorAddress();

//Read temperature from sensors.
void readTemperature();

//Every REQUEST_TEMPERATURE_INTERVAL return true. 
bool requestTemperatureTimer();

//Knowing the address of the room temperature sensor, it tries to get the temperature.
void findAndGetRoomTemperature();

//Blink point.
void showHeaterState();

//On/Off decimal point if t < 99.9 or t > 99.9.
void switchDecimalPoint(uint16_t temperature);

//If single ds18b20.
uint16_t showSingleSensorTemperature();

//If device init - show room temperature.
uint16_t showRoomTemperature();

//Logic for heater
void heatingControl();

//Must be called every second.
void secondHanler();

//Detects whether the encoder has rotated
void encoderHandler();

//Soft timer for delay show settings.
void showSettingsTimer();

//Returns the value of the current setpoint temperature.
uint16_t showEncoderValue();

//Delay before save to flash.
void softFlashTimer();
void softFlashTimerEnabled();
void softFlashTimerReset();
void softFlashTimerHandler();

//Reads whether the button is pressed
void buttonHandler();

//Writes sensor addresses to the flash if necessary
uint16_t sensorAddressInitProcess();

//Saves the address of the first device in flash, which is used as a room address.
void writeRoomSensorAddress();

//Clears all settings.
void cleanFlashSetting();

/*
If the device has saved the address value of the sensor measuring the temperature in the room,
and the user has replaced the sensor.
The device erases the old address and starts the reinitialization process.
The second sensor must be disabled, otherwise the first one will be considered a temperature sensor in the room.
And it, in turn, may turn out to be a hot water sensor.
Therefore, it is important to use one sensor when replacing it – then the device will work correctly.
*/
void resetRoomSensorAdress();

//Read external off pump input.
bool externalOffPinHandler();

_END_STD_C