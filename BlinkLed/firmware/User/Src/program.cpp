#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gpio.h"
#include "main.h"
#include "stdio.h"

volatile uint32_t TimeTickMs = 0;
uint32_t oldTimeTickHSec = 0;
bool secondTimerHandler = false; //one second has passed
bool ledStatus;

void setup(void)
{  

}

//
/**
 * \brief   It is performed periodically in the body of the main loop.
 *
 */
void loop(void)
{  
   if(secondTimerHandler == true)
   {
      if(ledStatus)
      {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);   
      }
      else
      {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);    
      }    

      ledStatus = !ledStatus;
      secondTimerHandler = false;
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

 }

 if (htim->Instance == TIM4)
 { 
 
 } 
}