#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gpio.h"
#include "main.h"
#include "stdio.h"

extern IWDG_HandleTypeDef hiwdg;
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim2;

#define ADC_NUMBER_OF_CHANNELS 1
#define MAX_QUANTITY_MEASUREMENTS 16

#define ADC_REFERENCE_VOLTAGE 32.5 //~3.3*10
#define DIVISION_COEFFICIENTS_VOLTAGE 2 //4.7k + 4.7k divider

#define MAX_CHARGE_VOLTAGE 3.6
#define MIN_DISCHARGE_VOLTAGE 3.5

volatile uint32_t TimeTickMs = 0;
uint32_t oldTimeTickHSec = 0;
bool secondTimerHandler = false; //one second has passed

uint32_t adcAvgBuff[ADC_NUMBER_OF_CHANNELS];
uint32_t adcResults[ADC_NUMBER_OF_CHANNELS];
uint32_t adcData[ADC_NUMBER_OF_CHANNELS]; //Measured adc values.
uint8_t numberMeasurements = 0;

float currentVoltage;

/**
 * \brief  Performs initialization. 
 *
 */
void init( void )
{
   
}

/**
 * \brief  Performs additional settings.
 *
 */
void setup( void )
{
    HAL_ADC_Start_DMA(&hadc1, adcData, ADC_NUMBER_OF_CHANNELS); // start adc in DMA mode 
    HAL_IWDG_Refresh(&hiwdg);
    HAL_Delay(500);        
    HAL_IWDG_Refresh(&hiwdg);   
}

void loadControl(float voltage)
{
  if(voltage < MIN_DISCHARGE_VOLTAGE)
  {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET); //off discharge, on charge.
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET); //Led on board
  }

  if(voltage > MAX_CHARGE_VOLTAGE)
  {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET); //on discharge, off charge.
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET); //Led on board
  }
}

/**
 * \brief   It is performed periodically in the body of the main loop.
 *
 */
void loop( void )
{   
    HAL_IWDG_Refresh(&hiwdg);
       
    //Second timer.
    if(secondTimerHandler == true)
    {
      HAL_IWDG_Refresh(&hiwdg);  
   
      currentVoltage = (float)(adcResults[0]) * ADC_REFERENCE_VOLTAGE * DIVISION_COEFFICIENTS_VOLTAGE / 40960;
    
      HAL_IWDG_Refresh(&hiwdg);
      loadControl(currentVoltage);

      secondTimerHandler = false;   
    }   
}

void HAL_SYSTICK_Callback( void )
{
    TimeTickMs++;
    if (TimeTickMs - oldTimeTickHSec > 1000)
    {
      oldTimeTickHSec = TimeTickMs;
      secondTimerHandler = true;
    }    
}


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)
        {
            TIM2->CNT = 0;                            
        }
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{   
 if (htim->Instance == TIM3)
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
}