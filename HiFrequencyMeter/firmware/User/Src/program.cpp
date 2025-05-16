/*
For test use MCO output PA8
Input frequency to TIM2_ETR (PA0)
TIM1 - measured interval 1sec

Without a divider, we can measure the max frequency 72/3 = 24Mhz
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gpio.h"
#include "main.h"
#include "stdio.h"
#include "lcd1602.h"

extern IWDG_HandleTypeDef hiwdg;
extern TIM_HandleTypeDef htim1;

volatile uint32_t TimeTickMs = 0;
uint32_t oldTimeTickHSec = 0;

bool secondTimerHandler = false; //one second has passed
uint16_t displayValue;

bool isInit;
uint16_t testCounter;
uint16_t testCounter1;
uint32_t frequency;
char lcdBuffer[20];

#define DELAY_VALUE 1
struct delayTimerData_t
{
  uint16_t counter;
};

delayTimerData_t delayTimerData;

void setup(void)
{ 
  init_lcd_pins(); 
  HAL_IWDG_Refresh(&hiwdg);
  lcd_init();
}

//
/**
 * \brief   It is performed periodically in the body of the main loop.
 *
 */
void loop(void)
{ 
  HAL_IWDG_Refresh(&hiwdg); 

  if(!isInit)
  {      
    lcd_put_cur(0, 0);
    lcd_send_string("Hello from ");

    lcd_put_cur(1, 0);
    lcd_send_string("freq");

    isInit = true;
  }

   if(secondTimerHandler == true)
   {
      //Wait, befor show count.
     if(delayTimerData.counter < DELAY_VALUE)
     {
      delayTimerData.counter ++;
      secondTimerHandler = false;
      return;
     } 

     lcd_clear();
    
     snprintf(lcdBuffer, 20, "%lu Hz", frequency);
     lcd_put_cur(0, 0);
     lcd_send_string(lcdBuffer); 

     sprintf(lcdBuffer, "%d", testCounter1);
     lcd_put_cur(1, 0);
     lcd_send_string(lcdBuffer); 


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
  if(htim->Instance == TIM1)
	{
    frequency = TIM2->CNT + (TIM3->CNT << 16); 
    HAL_TIM_Base_Stop_IT(&htim1);
    
    testCounter1 ++;
    TIM2->CNT = 0;
    TIM3->CNT = 0;
    HAL_TIM_Base_Start_IT(&htim1);
  }
 if (htim->Instance == TIM3)
 { 

 }
}