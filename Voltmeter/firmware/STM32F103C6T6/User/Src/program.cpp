#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gpio.h"
#include "main.h"
#include "stdio.h"

#define LED_MATRIX_SIZE 21
#define OFF_LED_SEGMENT_VALUE 20

extern IWDG_HandleTypeDef hiwdg;

volatile uint32_t TimeTickMs = 0;
uint32_t oldTimeTickHSec = 0;

bool secondTimerHandler = false; //one second has passed

uint8_t currentSegment;

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
uint16_t displayValue;

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

//Disables zeros in the higher registers.
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
 
  offZeros(digits);
  memcpy(displayArray, digits, 3);
}

/**
 * \brief   It is performed periodically in the body of the main loop.
 *
 */
void loop( void )
{   
    //HAL_IWDG_Refresh(&hiwdg);
    
    //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);

    //Second timer.
    if(secondTimerHandler == true)
    {
      //HAL_IWDG_Refresh(&hiwdg);  
    
      //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);     
      displayValue ++;

      if(pointData == 0)
      {
        pointData =1;
      }

      if(pointData < 8)
      {
        pointData <<= 1;        
      }
      else
      {
        pointData = 1;
      }
      secondTimerHandler = false;
    }   

    hexToDec(displayValue);
}

void dynamicIndication()
{

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
        digToSegments(displayArray[0], pointOn);
        currentSegment = 1;              
     break;
   
     case 1:
       HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET); //Off segment 1
       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); //Off segment 2
       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); //Off segment 3
       pointOn = (pointData & 0x02) >> 1;
       digToSegments(displayArray[1], pointOn);
       currentSegment = 2;
     break;

     case 2:
       HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET); //Off segment 1
       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); //Off segment 2
       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); //Off segment 3
       pointOn = (pointData & 0x04) >> 2;
       digToSegments(displayArray[2], pointOn);
       currentSegment = 0;    
     break;

    default:
    break;
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

    /*
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
      }*/    
   }
}