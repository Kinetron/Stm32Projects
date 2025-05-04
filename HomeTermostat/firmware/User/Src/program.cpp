#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gpio.h"
#include "main.h"
#include "stdio.h"

#define LED_MATRIX_SIZE 21
#define OFF_LED_SEGMENT_VALUE 20
#define ADC_NUMBER_OF_CHANNELS 3
#define MAX_QUANTITY_MEASUREMENTS 16
#define ADC_REFERENCE_VOLTAGE 33.0 //~3.3*10
#define DIVISION_COEFFICIENTS_VOLTAGE 6.067 //20k 1% + 100k 1%divider
#define DIVISION_COEFFICIENTS_CURRENT 11.600
#define DISPLAY_BLINK_VOLTAGE 14.65

#define BLINK_LED_INTERVAL 70
#define CHANGE_INDICATION_PERIOD 5 //switch U and I on display.

#define DATA_PAUSE_INTERVAL 2

extern IWDG_HandleTypeDef hiwdg;
extern ADC_HandleTypeDef hadc1;

volatile uint32_t TimeTickMs = 0;
uint32_t oldTimeTickHSec = 0;

bool secondTimerHandler = false; //one second has passed

uint8_t currentSegment; //For dynamic led.
extern uint8_t tempatureSensorCount;

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
  0xFD //off segment
};

uint8_t pointData; //which digits the dot be displayed
uint8_t displayArray[3] = {0, 0, 0};
uint8_t displayBuffer[3] = {0, 0, 0};
uint16_t displayValue;
bool needBlink;

uint16_t blinkLedTimer;
uint8_t blinkLedDirection;

uint32_t adcData[ADC_NUMBER_OF_CHANNELS]; //Measured adc values.
uint32_t adcAvgBuff[ADC_NUMBER_OF_CHANNELS];
uint32_t adcResults[ADC_NUMBER_OF_CHANNELS];
uint8_t numberMeasurements = 0;

uint8_t changeIndicationTimer;
bool changeIndicationSwitch;

uint32_t temperatureForSend; //For send data as pulse packet use tim4.
bool temperatureDataOutState;
uint32_t dataOutCounter; //Count pulse packet.
bool dataPacketSend; //=true if send.
uint8_t dataOutPauseCnt; //Pause counter.

bool readTemperatureFlag;
uint8_t switchTemperature;

void initLed()
{
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET); //Off segment 1
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); //Off segment 2
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); //Off segment 3

  //off anods
  GPIOA->ODR |= 0x0F0;    
  GPIOB->ODR |= 0xC03; 
}

void setup( void )
{  
   HAL_IWDG_Refresh(&hiwdg);
   changeIndicationSwitch = true;
   TIM2->CNT = 247 * 4;  

   displayValue = 18;
}

//Convert dig to 7 seg led matrix.
void digToSegments(uint8_t dig, uint8_t point)
{
   if(dig > LED_MATRIX_SIZE - 1) //Protected overflow.
   {
     return;
   }

   uint8_t led = ledMatrix[dig];
   uint8_t portA = (led & 0xF0) | 0x0F;
   GPIOA->ODR &= portA;  

   uint16_t portB = (uint16_t)(led & 0x0F);
   uint16_t lsb = (portB & 0x03) | 0xFFFC;

   if (point)
   {
    lsb &= 0xFFFD; //dp - pb1
   }
   else
   {
     lsb |= 2; //Off point;
   }

   GPIOB->ODR &= lsb;

   portB >>= 2;
   uint16_t msb = (portB & 0x03);// | 0xFFFC;
   msb <<= 10;
   msb |= 0xF3FF;
   
   GPIOB->ODR &= msb;   
}

//Disables zeros in the higher registers. If need.
void offZeros(uint8_t *arr)
{
  bool msbZero = true;
  for (uint8_t i = 2; i > 0; i--)
  {
     if(!msbZero)
     {
      break;
     }

     if(arr[i] == 0)
     {
      arr[i] = OFF_LED_SEGMENT_VALUE;
     }
     else
     {
       msbZero = false;
     }
  } 
}

//Convert hex number to 3 decimal digit.
void hexToDec(uint16_t value)
{
   uint8_t digits[3] = {0, 0, 0};
	
	while (value >= 100)
	{
		value-= 100;
		digits[2]++;
	}	
	
	while (value >= 10)
	{
		value-= 10;
		digits[1]++;
	}	
	
	while (value >= 1)
	{
		value-= 1;
		digits[0]++;
	}	
 
  //offZeros(digits);
  memcpy(displayArray, digits, 3);
}

//Switch on display voltage or current.
bool changeIndicationParam()
{
   if(changeIndicationTimer < CHANGE_INDICATION_PERIOD)
   {
     changeIndicationTimer ++;
   }
   else
   {
     changeIndicationTimer = 0;
     changeIndicationSwitch = !changeIndicationSwitch;
   }

   return changeIndicationSwitch;
}

//Generates a pause after sending a burst of pulses
void outDataPause()
{
  if(dataPacketSend)
  {
    if(dataOutPauseCnt <= DATA_PAUSE_INTERVAL )
    {
      dataOutPauseCnt ++;
    }
    else
    {
      dataOutPauseCnt = 0;
      dataPacketSend = false; 
    }
  }  
}
void tim4InterruptHanler()
{
  if(dataPacketSend) //Pause
  {      
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
    return;
  }

   //Pulse count.
  if(dataOutCounter < temperatureForSend)
  {
    dataOutCounter ++; 
  }
  else
  {
    dataOutCounter = 0;
    dataPacketSend = true;
    temperatureDataOutState = true; //Low level in output.
  }

  if(temperatureDataOutState)
  {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET); 
    temperatureDataOutState = false;    
  } 
  else
  {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET); 
    temperatureDataOutState = true; 
  }
}

void getTemperature()
{
  requestTemperature();
  if(readTemperatureFlag)
  {
    displayValue = 253;// 
    //float t = readTemperature();
    readTemperatureFlag = false;
  }
  else
  {
    displayValue = 31;
    //
    readTemperatureFlag = true;
  }
}

//
/**
 * \brief   It is performed periodically in the body of the main loop.
 *
 */
void loop( void )
{  
    //Second timer.
    /*
 
    
      float voltage = (float)(adcResults[1]) * ADC_REFERENCE_VOLTAGE * DIVISION_COEFFICIENTS_VOLTAGE / 40960;
      float current = (float)(adcResults[0]) * ADC_REFERENCE_VOLTAGE * DIVISION_COEFFICIENTS_CURRENT / 40960;
      
      float resutValue = voltage;

      if(!changeIndicationParam())
      {
        resutValue = current;
      }

      uint8_t mux = 100;
      //Switch point and mux.
      if(resutValue < 9.990)
      {
        pointData = (pointData & 0x01) | 0x04;
      }
      else
      {
        pointData = (pointData & 0x01) | 0x02;
        mux = 10;
      }

      displayValue = (uint16_t)(resutValue * mux);
      
      //Blink if charge end
      if(voltage >= DISPLAY_BLINK_VOLTAGE)
      {
        needBlink = true;
      } 
      else
      {
        needBlink = false;
      }

      secondTimerHandler = false;

      //pwmControl(current);
    }   
*/

   if(secondTimerHandler == true)
   {
    //outDataPause(); //Generates a pause after sending a burst of pulses
    requestTemperature();
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

    // getTemperatureSensorCount();
    //getTemperature();
    secondTimerHandler = false;
   }
  /*
   //Encoder test
    displayValue = TIM2->CNT;
    displayValue = displayValue / 4;
    if(displayValue > 999)
    {
      displayValue /= 10;
    }

    temperatureForSend = displayValue * 2;
    */
    hexToDec(displayValue);
    HAL_IWDG_Refresh(&hiwdg);
    
  }

//Soft timer.
bool blinkLedTimerHandler()
{  
   if(!blinkLedDirection)
   {
      if(blinkLedTimer < BLINK_LED_INTERVAL)
      {
        blinkLedTimer ++;
        return false;
      }
      else
      {
        blinkLedDirection = 1;
        return true;
      }
   }
   else
   {
      if(blinkLedTimer)
      {
        blinkLedTimer --;
        return true;
      }
      else
      {
        blinkLedDirection = 0;
        return false;
      }
   } 
}

//Generate a dynamic indication
void dynamicIndication()
{  
  memcpy(displayBuffer, displayArray, 3); 
  
  if(needBlink == true && blinkLedTimerHandler())
  {
    memset(displayBuffer, OFF_LED_SEGMENT_VALUE, 3); //Off display.
  }

  uint8_t pointOn = 0;
   //off anods
   GPIOA->ODR |= 0x0F0;    
   GPIOB->ODR |= 0xC03; 

   switch (currentSegment)
   {
     case 0:
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET); //Off segment 1
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); //Off segment 2
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); //Off segment 3
        pointOn = pointData & 0x01;
        digToSegments(displayBuffer[0], pointOn);
        currentSegment = 1;              
     break;
   
     case 1:
       HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET); //Off segment 1
       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); //Off segment 2
       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); //Off segment 3
       pointOn = (pointData & 0x02) >> 1;
       digToSegments(displayBuffer[1], pointOn);
       currentSegment = 2;
     break;

     case 2:
       HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET); //Off segment 1
       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); //Off segment 2
       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); //Off segment 3
       pointOn = (pointData & 0x04) >> 2;
       digToSegments(displayBuffer[2], pointOn);
       currentSegment = 0;    
     break;

    default:
    break;
   }
}

void calculateAdc()
{
  if(numberMeasurements < MAX_QUANTITY_MEASUREMENTS)
      {
         for(uint8_t i = 0; i < ADC_NUMBER_OF_CHANNELS; i++)
         {
           adcAvgBuff[i]+= adcData[i]; 
         }

         numberMeasurements++;    
      }
      else
      {
        for(uint8_t i = 0; i < ADC_NUMBER_OF_CHANNELS; i++)
         {
           adcResults[i]= adcAvgBuff[i] / numberMeasurements; 
           adcAvgBuff[i] = 0;
         }

         numberMeasurements = 0;
      }    
}

void HAL_SYSTICK_Callback(void)
{
    TimeTickMs++;
    if (TimeTickMs - oldTimeTickHSec > 1000)
    {
      oldTimeTickHSec = TimeTickMs;
      secondTimerHandler = true;
    }    
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{   
 if (htim->Instance == TIM3)
 { 
    dynamicIndication();
    calculateAdc();     
 }

 if (htim->Instance == TIM4)
 { 
    tim4InterruptHanler();  
 } 
}