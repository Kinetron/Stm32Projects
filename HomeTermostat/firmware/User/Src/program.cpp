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

void setup(void)
{  
   HAL_IWDG_Refresh(&hiwdg);
   displayValue = initWorkMode();  
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
    outDataPause(); //Generates a pause after sending a burst of pulses
    readTemperature();
    displayValue = prepareDisplay();
   
    secondHanler(); 
    secondTimerHandler = false;
   }

   ledDisplayHandler(displayValue);
   encoderHandler();    
   
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
    blinkLedTimerHandler();
    buttonHandler();
 }

 if (htim->Instance == TIM4)
 { 
    pulseSenderHandler();  
 } 
}