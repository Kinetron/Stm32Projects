#include "ControlLogic.h"
#include "specialCodes.h"

struct tControlData tempControlData;
struct requestTemperatureTimer requestTemperatureSoftTimer;

//Only for test
struct LedTest ledTestData;

//Generates the data displayed on the led.
uint16_t getDisplayData(void)
{
  //Return errors.
  if(tempControlData.errorCode > 0)
  {
    setPoints(0);
    return createErrorCode(tempControlData.errorCode);
  }
 
  //float t =  displayTemperatureTest();
  float t = tempControlData.tempature;
  uint16_t displayValue = calculateTemperature(t);
  setPoints(switchDecimalPoint(t));
  
  bool showMinus = false;
  if(t < 0)
  {
    showMinus = true;
  }

  onMinus(showMinus);
  return displayValue;
}

//Convert temperature to int, switch poin, add minus.
uint16_t calculateTemperature(float temperature)
{
  uint16_t mul = 10;

  #ifdef TWO_DIGITS_PRECISION
    mul = 100;
  #endif

  if(temperature < 0)
  {
    mul = 10; // Off two digits precision. There is not enough space on the display. 
  }

  if(temperature > 99.9)
  {
    mul = 10; // Off two digits precision. There is not enough space on the display. 
  }

  return abs(round(temperature * mul));
}

//On/Off decimal point if t < 99.9 or t > 99.9.
uint8_t switchDecimalPoint(float temperature)
{
  bool doublePrecision = false;   
  uint8_t pointPos = 0;

  #ifdef TWO_DIGITS_PRECISION
    doublePrecision = true;
  #endif

  if(doublePrecision)
  {
    if(temperature <= 99.9)
    {
      pointPos = 4;
    }
    else
    {
      pointPos = 2;
    }    

    if(temperature < 0)
    {
      pointPos = 2;
    }

    return pointPos;
  }
  else
  {
    return 2;
  }  
}

uint16_t createErrorCode(enum ErrorCodes err)
{
  return LED_ERROR_ARIA + err;
}

void readTemperature()
{
  if(!requestTemperatureTimer()) 
  {
    return;
  }

  if(offOwLinePause())
  {
    return;
  }

  //Reinitialization to avoid loss of communication with the sensor. If noise in line.
  //In main.c run init.
  if(tempControlData.isInit)
  {
    DS18B20_Init(DS18B20_Resolution_12bits);
    //DS18B20_Init(DS18B20_Resolution_9bits);
    tempControlData.isInit = false;
    return;
  }  

  DS18B20_ReadAll();
  DS18B20_StartAll();

  tempControlData.sensorQuantity = DS18B20_Quantity();  

  if(tempControlData.sensorQuantity == 0)
  {
    tempControlData.errorCode = NoSensor;    
    reInitBus();
    return;
  }

  requestTemperatureSoftTimer.disabled = false;
  tempControlData.errorCode = 0;

  tempControlData.tempature = DS18B20_GetTemperature(0);

  if(!DS18B20_GetValidDataFlag(0))
  {
    //ignore crc for bad ds18b20 work only 5v.
    #ifndef USE_BAD_SENSOR 
     tempControlData.errorCode = CrcError; 
     reInitBus();
    #endif
  }
}

//Resets timers and restarts the bus.
void reInitBus()
{
  tempControlData.setLowOw = true; //Reset line.
  requestTemperatureSoftTimer.disabled = true; //For fast find if error.
  tempControlData.isInit = true;
}

//Every REQUEST_TEMPERATURE_INTERVAL return true. 
bool requestTemperatureTimer()
{
   if(requestTemperatureSoftTimer.disabled)
   {
    requestTemperatureSoftTimer.initCounter = 0;
     requestTemperatureSoftTimer.counter = 0;
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

bool offOwLinePause()
{
  if(!tempControlData.setLowOw)
  {
    return false;
  }

  if(tempControlData.offPauseCnt < OW_LINE_PAUSE)
  {
    DS18B20_LowLine();
    tempControlData.offPauseCnt ++;
    return true;
  }
  else
  {
    tempControlData.offPauseCnt = 0;
    tempControlData.setLowOw = false;
    return false;
  }  
}

//Generates different temperature options, including negative ones to check the display.
float displayTemperatureTest()
{
   switch (ledTestData.cnt)
   {
    case 0:
     ledTestData.cnt = 1;
    return 20.10;
  
    case 1:
     ledTestData.cnt = 2;
    return 99.99;
   
    case 2:
     ledTestData.cnt = 3;
    return 125.12;

    case 3:
     ledTestData.cnt = 4;
    return -15.51;

    case 4:
     ledTestData.cnt = 5;
    return -55.39;

    case 5:
     ledTestData.cnt = 6;
    return 89.10;

    case 6:
     ledTestData.cnt = 0;
    return 0.27;

   default:
    break;
   } 
}


















