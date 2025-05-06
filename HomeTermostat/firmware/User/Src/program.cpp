#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gpio.h"
#include "main.h"
#include "stdio.h"
#include "led.h"
#include "ControlLogic.h"
#include "PulseSender.h"


extern IWDG_HandleTypeDef hiwdg;

volatile uint32_t TimeTickMs = 0;
uint32_t oldTimeTickHSec = 0;

bool secondTimerHandler = false; //one second has passed
uint16_t displayValue;


void setup( void )
{  
   HAL_IWDG_Refresh(&hiwdg);

   TIM2->CNT = 247 * 4;  

   displayValue = initWorkMode();  
}

//
/**
 * \brief   It is performed periodically in the body of the main loop.
 *
 */
void loop( void )
{  
   if(secondTimerHandler == true)
   {
    //outDataPause(); //Generates a pause after sending a burst of pulses
    requestTemperature();
    displayValue = prepareDisplay();
        
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

    ledDisplayHandler(displayValue);
    
    HAL_IWDG_Refresh(&hiwdg);
    
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
 }

 if (htim->Instance == TIM4)
 { 
    pulseSenderHandler();  
 } 
}