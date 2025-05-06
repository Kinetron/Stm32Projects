
#include "ControlLogic.h"


uint8_t systemMode;  //Сurrent operating mode
uint8_t currentDisplayedTemperature;

//After start read system params and decides on the mode of operation.
uint16_t initWorkMode()
{
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

  return 93;//initFlag;
//#define FLASH_INIT_FLAG 0x37

    //FLASH_INIT_FLAG_POS 
   //
}

//Generates the data displayed on the led.
uint16_t prepareDisplay(void)
{
  uint8_t cnt = getTemperatureSensorCount();
  if(cnt == 0) //E1 - temp sensor not found.
  {
    return createErrorCode(NoSensor);
  }


  return emptySettingsModeDisplay(cnt);


  /*
  switch (switchTemperature)
  {
    case 0:
    displayValue = getTempatureArr(0);
    switchTemperature = 1;
    break;
  
    case 1:
    displayValue = getTempatureArr(1);
    switchTemperature = 2;
    break;

    case 2:
    displayValue = getTemperatureSensorCount();
    switchTemperature = 0;
    break;

  default:
    break;
  }  
    */ 
}

//No config hot water sensor.
uint16_t emptySettingsModeDisplay(uint8_t count)
{
  if(currentDisplayedTemperature == 0)
  {    
    currentDisplayedTemperature ++;
    return count; //Displaying the number of temperature sensors. 
  }
  else
  {     
    uint8_t tempArrPos = currentDisplayedTemperature - 1;
    if(currentDisplayedTemperature > count)
    {
       currentDisplayedTemperature = 0;
       return count;
    }
    
    currentDisplayedTemperature ++;
    return getTempatureArr(tempArrPos);
  }
}

uint16_t createErrorCode(enum ErrorCodes err)
{
  return LED_ERROR_ARIA + err;
}