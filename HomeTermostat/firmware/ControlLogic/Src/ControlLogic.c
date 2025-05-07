#include "ControlLogic.h"
#include "specialCodes.h"

uint8_t systemMode;  //Сurrent operating mode
uint8_t currentDisplayedTemperature;
uint8_t ledPointFlag; //For switch point in segments.

struct tControlData tempControlData;
struct encoderData encoder;
struct displayMode display;
struct flashTimer softFlashTimerData;
struct buttonData encoderButton;//Checks if the encoder button is pressed.
struct initState initDevice;

struct requestTemperatureTimer requestTemperatureSoftTimer;

//After start read system params and decides on the mode of operation.
uint16_t initWorkMode()
{
  uint16_t setTemperature = eeprom_readUint16(FLASH_SET_TEMPERATURE_POS);
  
  //Not set temperature. Init.
  if(setTemperature == 0xFFFF)
  {
    eeprom_writeUint16(FLASH_SET_TEMPERATURE_POS, DEFAULT_SET_TEMPERATURE);
    tempControlData.roomSetedTemperature = DEFAULT_SET_TEMPERATURE;
  }
  else
  {
    tempControlData.roomSetedTemperature = setTemperature;
  }

  encoder.lastEncoderValue = tempControlData.roomSetedTemperature;
  encoder.value = tempControlData.roomSetedTemperature;
  TIM2->CNT = tempControlData.roomSetedTemperature * 2;

  //Set hot water sensor address ?
  initDevice.hasRoomSensorAddress = isSetRoomSensorAddress();

  //If not set address how water sensor - uset first as room.
  tempControlData.ignoreHotWaterTemperature = true;
  


  /*
  uint8_t initFlag = eeprom_read_byte(FLASH_INIT_FLAG_POS);

  if(initFlag != FLASH_INIT_FLAG)
  {
    systemMode = 0;
    //eeprom_write_byte(FLASH_INIT_FLAG_POS, FLASH_INIT_FLAG);
  }  
  
  if(initFlag == FLASH_INIT_FLAG)
  {
    systemMode = 1;
   
  }  
*/
  return 95;//initFlag;
//#define FLASH_INIT_FLAG 0x37

    //FLASH_INIT_FLAG_POS 
   //
}

//Generates the data displayed on the led.
uint16_t prepareDisplay(void)
{
  if(tempControlData.sensorQuantity == 0) //E1 - temp sensor not found.
  {
    return createErrorCode(NoSensor);
  }

  //Show settings.
  if(encoder.hasRotate || display.showEncoderData)
  {
    encoder.hasRotate = false;
    display.showEncoderData = true;

    tempControlData.roomSetedTemperature = encoder.value; 
    blinkDigits(true);
    softFlashTimerEnabled(); 
    softFlashTimerReset();

    return showEncoderValue();
  }

  //Press encoder button.
  if(encoderButton.isPressed || display.pressEncoderBtn)
  {
    display.pressEncoderBtn = true;
    encoderButton.isPressed = false;
    blinkDigits(true);
    return showEncoderValue();
  }
  
  blinkDigits(false);
  
  //We inform you that we have saved the information in flash.
  if(softFlashTimerData.dataWillSave)
  {
    softFlashTimerData.dataWillSave = false;
    ledPointFlag = 0;
    return WRITE_TO_FLASH_SIMVOL;
  }

  uint16_t displayValue = emptySettingsModeDisplay();
  showHeaterState();
  setPoints(ledPointFlag);

  return displayValue;
}

//No config hot water sensor.
uint16_t emptySettingsModeDisplay()
{  
  uint8_t count = tempControlData.sensorQuantity;
  
  if(count == 1)
  {
    return showSingleSensorTemperature();
  }

  if(currentDisplayedTemperature == 0)
  {    
    currentDisplayedTemperature ++;
    ledPointFlag = 0;
    return count; //Displaying the number of temperature sensors. 
  }
  else
  {     
    uint8_t tempArrPos = currentDisplayedTemperature - 1;
    if(currentDisplayedTemperature > count)
    {
       currentDisplayedTemperature = 0;
       ledPointFlag = 0;
       return count;
    }
    
    currentDisplayedTemperature ++;

    //On decimal point if t < 99.9.
    uint16_t temperature = tempControlData.tempatureBuff[tempArrPos];
    switchDecimalPoint(temperature);

    return temperature;
  }
}

//On/Off decimal point if t < 99.9 or t > 99.9.
void switchDecimalPoint(uint16_t temperature)
{
  if(temperature <= 999)
  {
    ledPointFlag = 2;
  }
  else
  {
    ledPointFlag = 0;
  }
}

//If single ds18b20.
uint16_t showSingleSensorTemperature()
{
    uint16_t temperature = tempControlData.tempatureBuff[0];
    switchDecimalPoint(temperature);

    return temperature;
}

uint16_t createErrorCode(enum ErrorCodes err)
{
  return LED_ERROR_ARIA + err;
}

  //Set hot water sensor address ?
uint8_t isSetRoomSensorAddress()
{
    //Set hot water sensor address ?
    uint8_t address[DS18B20_ADDR_SIZE];
    memset(address, 0, DS18B20_ADDR_SIZE);
    eeprom_readArray(FLASH_T0_ADDRESS_POS, address, DS18B20_ADDR_SIZE);
    
    uint8_t empty = 0;
    for (uint8_t i = 0; i < DS18B20_ADDR_SIZE; i++)
    {
      if(address[i] != 0xFF)
      {
        empty = 1;
        break;
      }
    }
    
    return empty;
}

void readTemperature()
{
  if(!requestTemperatureTimer()) 
  {
    return;
  }

  DS18B20_ReadAll();
  DS18B20_StartAll();

  tempControlData.sensorQuantity = DS18B20_Quantity();  

  //For fast find if error.
  if(tempControlData.sensorQuantity == 0)
  {
    requestTemperatureSoftTimer.disabled = true;
  }
  else
  {
    requestTemperatureSoftTimer.disabled = false;
  }
  

  for(uint8_t i = 0; i < MAX_SENSOR_QUANTITY; i++)
	{
    tempControlData.data[i] = DS18B20_GetSensorData(i);

    if(DS18B20_GetValidDataFlag(i))
    {
        tempControlData.tempatureBuff[i] = round(DS18B20_GetTemperature(i) * 10);  
    }   
  }   

  if(tempControlData.sensorQuantity == 1)
  {
    tempControlData.roomTempature = tempControlData.tempatureBuff[0];
  }
}

//Every REQUEST_TEMPERATURE_INTERVAL return true. 
bool requestTemperatureTimer()
{
   if(requestTemperatureSoftTimer.disabled)
   {
    return true;
   }

  //Delay after first run.
  if(!requestTemperatureSoftTimer.isNoFirstRun)
  {
     if(requestTemperatureSoftTimer.initCounter <= DELAY_USE_SLOW_TEMPERATURE_INTERVAL)
     {
      requestTemperatureSoftTimer.initCounter ++;
      return true;
     }
     else
     {
      requestTemperatureSoftTimer.initCounter = 0;
      requestTemperatureSoftTimer.isNoFirstRun = true;
     }
  }

  if(requestTemperatureSoftTimer.counter <= READ_TEMPERATURE_INTERVAL)
  {
    requestTemperatureSoftTimer.counter ++;
    return false;
  }
  else
  {
    requestTemperatureSoftTimer.counter = 0;
    return true;
  }
}

//Blink point.
void showHeaterState()
{
  if (tempControlData.heaterOn)
  {
    ledPointFlag |= 0x01;
  }
  else
  {
    ledPointFlag &= 0xFE;
  }
}

//Logic for heater
void heatingControl()
{
  uint16_t currentTemperature = tempControlData.roomTempature;
  
  if(currentTemperature >= tempControlData.roomSetedTemperature + HYSTERESIS)
  {
    tempControlData.heaterOn = false; //Off heater.
  }

  if(currentTemperature <=  tempControlData.roomSetedTemperature - HYSTERESIS)
  {
    tempControlData.heaterOn = true; //On heater.
  }
  
  //Has two t. sensor.
  if(tempControlData.ignoreHotWaterTemperature == false && tempControlData.sensorQuantity == 2)
  {
    //The hot water stopped heating up, the flame went out ? 
    if(tempControlData.hotWaterTempature < THRESHOLD_HOT_WATER)
    {
      tempControlData.heaterOn = false; //Off heater.
    }
  }
  
  if(tempControlData.heaterOn)
  {
    HAL_GPIO_WritePin(HEATER_GPIO, HEATER_PIN, GPIO_PIN_SET);
  }
  else
  {
    HAL_GPIO_WritePin(HEATER_GPIO, HEATER_PIN, GPIO_PIN_RESET);
  }
}

//Detects whether the encoder has rotated
void encoderHandler()
{  
  uint32_t encoderValue = TIM2->CNT / 2; 
    
  if(encoder.lastEncoderValue == encoderValue)
  {
     return;
  }

  //int diff = encoderValue - encoder.lastEncoderValue; 
  encoder.value = encoderValue;
  encoder.lastEncoderValue = encoderValue;
  encoder.hasRotate = true;

  /*
  if(diff > 0 && encoder.value < MAX_ENCODER_VALUE)
  {
    encoder.value ++;
    return;
  }

  if(diff < 0 && encoder.value > 0)
  {
    encoder.value --;
    return;
  }
    */

 

   /*
    MAX_ENCODER_VALUE
   //Encoder test
    displayValue = TIM2->CNT;
    displayValue = displayValue / 4;
    if(displayValue > 999)
    {
      displayValue /= 10;
    }

    temperatureForSend = displayValue * 2;
    */ 
}

void showSettingsTimer()
{
  if(!display.showEncoderData) 
  {
    return;
  }

  if(display.encoderDisplayTimer < PERIOD_SHOW_SETTINSG)
  {
    display.encoderDisplayTimer ++;
  } 
  else
  {
    display.encoderDisplayTimer = 0;
    display.showEncoderData = false;
  }
}

void pressedEncoderBtnTimer()
{
  if(!display.pressEncoderBtn) 
  {
    return;
  }

  if(display.pressEncoderBtnDisplayTimer < PERIOD_SHOW_SETTINSG)
  {
    display.pressEncoderBtnDisplayTimer ++;
  } 
  else
  {
    display.pressEncoderBtnDisplayTimer = 0;
    display.pressEncoderBtn = false;
  }
}

//Returns the value of the current setpoint temperature.
uint16_t showEncoderValue()
{
  uint16_t setTemperature = encoder.value;
  switchDecimalPoint(setTemperature);
  setPoints(ledPointFlag);

  if(setTemperature > 999)
  {
    setTemperature /= 10;
  }

  return setTemperature;
}

//Delay before save to flash.
void softFlashTimer()
{
  if(!softFlashTimerData.enable)
  {
    return;
  }
  
  if(softFlashTimerData.timer <= DELAY_BEFORE_WRITE_TO_FLASH)
  {
    softFlashTimerData.timer ++;
  }
  else
  {
    softFlashTimerData.timer = 0;
    softFlashTimerData.enable = false;
    softFlashTimerHandler();
  }
}

void softFlashTimerEnabled()
{
  softFlashTimerData.enable = true;
}

void softFlashTimerReset()
{
  softFlashTimerData.timer = 0;
}

void softFlashTimerHandler()
{
  softFlashTimerData.dataWillSave = true;
}

//Must be called every second.
void secondHanler()
{
  heatingControl();
  showSettingsTimer();
  softFlashTimer();
  pressedEncoderBtnTimer();
}

void buttonHandler()
{
  // IF Button Is Pressed
  if(HAL_GPIO_ReadPin(BUTTON_GPIO, BUTTON_PIN) == GPIO_PIN_RESET)
  {
    if(encoderButton.pressedInterval < COUNT_TICKS_BUTTON_IS_PRESSED)
    {
      encoderButton.pressedInterval ++;  
    }
    else
    {
      encoderButton.pressedInterval = 0;
      encoderButton.isPressed = true;
    }
    
  }
  else
  {
    encoderButton.pressedInterval = 0; 
  }
}

//Writes sensor addresses to the flash if necessary
void sensorAddressInitProcess()
{
   //Has valid room sensor address.
   if(initDevice.hasRoomSensorAddress)
   {
     return;
   }

   //Not init, if no ds18b20. 
   if(tempControlData.sensorQuantity == 0 || tempControlData.sensorQuantity == 2)
   {
     return;
   }

   //Timer.
   if(initDevice.counter <= DELAY_BEFORE_WRITE_ROOM_ADDR)
   {
    initDevice.counter ++;
   }
   else
   {
    initDevice.counter = 0;
    initDevice.hasRoomSensorAddress = true;
   }
}

//Saves the address of the first device in flash, which is used as a room address.
void writeRoomSensorAddress()
{
  
}