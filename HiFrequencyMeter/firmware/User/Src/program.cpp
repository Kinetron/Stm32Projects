#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gpio.h"
#include "main.h"
#include "stdio.h"
#include "lcd1602.h"

extern IWDG_HandleTypeDef hiwdg;

volatile uint32_t TimeTickMs = 0;
uint32_t oldTimeTickHSec = 0;

bool secondTimerHandler = false; //one second has passed
uint16_t displayValue;

bool isInit;
uint16_t testCounter;
char lcdBuffer[10];

#define DELAY_VALUE 5
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
    lcd_send_string("another world");

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
    
     sprintf(lcdBuffer, "%d", testCounter);
     lcd_put_cur(0, 0);
     lcd_send_string(lcdBuffer); 

    testCounter ++;

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
}