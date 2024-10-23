#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gpio.h"
#include "usart.h"
#include "main.h"
#include "stdio.h"

extern IWDG_HandleTypeDef hiwdg;

volatile uint32_t TimeTickMs = 0;
uint32_t oldTimeTickHSec = 0;

int period = 100;
int interval = 0;

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
    // Setting the default state.  
    //if ( HAL_GPIO_ReadPin( USER_LED_GPIO_Port, USER_LED_Pin ) == GPIO_PIN_SET )
   HAL_GPIO_WritePin( USER_LED_GPIO_Port, USER_LED_Pin, GPIO_PIN_RESET );

   HAL_IWDG_Refresh(&hiwdg);
}

/**
 * \brief   It is performed periodically in the body of the main loop.
 *
 */
void loop( void )
{
    HAL_Delay( 10 );
    HAL_IWDG_Refresh(&hiwdg);
}

void HAL_SYSTICK_Callback( void )
{
    TimeTickMs++;

    if (TimeTickMs - oldTimeTickHSec > 1000)
    {
        oldTimeTickHSec = TimeTickMs;

        if(interval < 12) 
        {
            interval ++;
        }
        else
        {
          period += 10;        
          TIM2->ARR = period;
          interval = 0;
        }
    }    
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{   
   if (htim->Instance == TIM2)
   {
      HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_11);  
   }
}

//When all data from usart is received, an interrupt will be triggered.
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
}