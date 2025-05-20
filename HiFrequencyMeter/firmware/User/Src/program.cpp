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
#include "tim.h"

#define READ_BTN_PERIOD 15000
#define LCD_BUFF_SIZE 16
#define MAX_CAPTURE_WAIT 2

extern IWDG_HandleTypeDef hiwdg;
extern TIM_HandleTypeDef htim1;

volatile uint32_t TimeTickMs = 0;
uint32_t oldTimeTickHSec = 0;

bool secondTimerHandler = false; //one second has passed
uint16_t displayValue;

bool isInit;

uint32_t frequency;
uint32_t mul;

float lowFrequency;

uint32_t tim2Value, tim3Value;
uint64_t timer32Register; //32 bit register for measuring values. Min freq 0.13hz
bool isFirstCaptured;
bool haslowFreqValue;
bool enableLowFreqRange; //0-100Hz

uint8_t waitCaptureTimer;

char lcdBuffer[LCD_BUFF_SIZE];
char tmpBuffer[LCD_BUFF_SIZE];
char lcdBufferLine1[LCD_BUFF_SIZE];

enum ranges
{
  RANGE_24Mhz,
  RANGE_48Mhz,
  RANGE_96Mhz,
  RANGE_190Mhz,
  RANGE_LOW_FREQ
};

struct readBtnRate_t
{
   uint32_t readBtnTimerCnt;
   bool lastPressedState;
};

readBtnRate_t  readBtnRate;
uint8_t currentRange;

 
void setup(void)
{ 
  init_lcd_pins(); 
  HAL_IWDG_Refresh(&hiwdg);
  lcd_init();
  
  mul = 1;
  sprintf(lcdBufferLine1, "24Mhz");
}

bool readButton()
{
  //Soft timer.
  if(readBtnRate.readBtnTimerCnt < READ_BTN_PERIOD)
  {
    readBtnRate.readBtnTimerCnt ++;
    
  }
  else
  { 
    readBtnRate.readBtnTimerCnt = 0;

    bool pressedState = false;
    if(HAL_GPIO_ReadPin(RANGE_BTN_GPIO, RANGE_BTN_PIN) == GPIO_PIN_RESET)
    {
      pressedState = true;
    }
    
    //The user must release the button.
    if(readBtnRate.lastPressedState != pressedState)
    {
      readBtnRate.lastPressedState = pressedState;
      if(pressedState)
      {
        return true; 
      }
    }
       
  }  

  return false;
}

//Changes the range and switches the divider.
void switchRange()
{
    currentRange++;

  	switch(currentRange)
		{
			case RANGE_24Mhz:
				SetClockPrescaleTim2(TIM_CLOCKPRESCALER_DIV1);
        sprintf(lcdBufferLine1, "24Mhz");
        mul = 1;
			break;

			case RANGE_48Mhz:
				SetClockPrescaleTim2(TIM_CLOCKPRESCALER_DIV2);
        sprintf(lcdBufferLine1, "48Mhz");
         mul = 2;
			break;

			case RANGE_96Mhz:
				SetClockPrescaleTim2(TIM_CLOCKPRESCALER_DIV4);
        sprintf(lcdBufferLine1, "96Mhz");
         mul = 4;
			break;

			case RANGE_190Mhz:
				SetClockPrescaleTim2(TIM_CLOCKPRESCALER_DIV8);
        sprintf(lcdBufferLine1, "190Mhz");
         mul = 8;
			break;

      case RANGE_LOW_FREQ:        
        enableLowFreqRange = true; //0-100Hz
        sprintf(lcdBufferLine1, "LowFreq");

        SwitchToPeriodMeasureMode(); //Reconfig timers.
        ClearPeriodCounters();
      break;

      case RANGE_LOW_FREQ + 1:
       SwitchToFrequencyMeasureMode();
       SetClockPrescaleTim2(TIM_CLOCKPRESCALER_DIV1);
       sprintf(lcdBufferLine1, "24Mhz");
       mul = 1;
       currentRange = 0;
       enableLowFreqRange = false;
       ClearPeriodCounters();
      break;

			default:
			break;
		}    
}

void spaceBefore(char* buffer, int arrSize)
{
    for (size_t i = 0; i < arrSize; i++)
    {
        if (buffer[i] != 0)
        {
            break;
        }
        
        buffer[i] = ' ';
    }
}

//Converts a string 8000000 to 8.000.000Hz
void formatFreq()
{
    memset(lcdBuffer, 0, LCD_BUFF_SIZE - 1);
    uint8_t len = strlen(tmpBuffer);
    
    uint8_t pos = LCD_BUFF_SIZE - 1;
    lcdBuffer[pos] = '\0';
    pos--;
    lcdBuffer[pos] = 'z';
    pos--;
    lcdBuffer[pos] = 'H';
    pos--;

    uint8_t digCount = 0;
    for (int i = len - 1; i >= 0; i--)
    {       
        lcdBuffer[pos] = tmpBuffer[i];
        if (pos >= 0)
        {
            pos--;
        }
        digCount++;
        
        if (digCount == 3)
        {
            lcdBuffer[pos] = '.';
            if (pos >= 0)
            {
                pos--;
            }
            digCount = 0;
        }
        
    }

    spaceBefore(lcdBuffer, LCD_BUFF_SIZE - 1);
}

bool endWaitCapture()
{
  if(waitCaptureTimer < MAX_CAPTURE_WAIT)
  {
    waitCaptureTimer++;
  }
  else
  {
    waitCaptureTimer = 0;
    return true;
  }

   return false;
}

void calculateLowFrequency()
{
   if(!haslowFreqValue)
   {
    return;
   }

   if(tim2Value == 0 && tim3Value == 0)
   {
    lowFrequency = 0;
    return;  
   }
   timer32Register = tim2Value + (tim3Value * 65535);
   lowFrequency = (float)72000000 / ((float)timer32Register);
}

//
/**
 * \brief   It is performed periodically in the body of the main loop.
 *
 */
void loop(void)
{ 
  HAL_IWDG_Refresh(&hiwdg); 
  calculateLowFrequency();
  
  if(secondTimerHandler == true)
   {
     lcd_clear();
    
     if(!enableLowFreqRange)
     {
       frequency = frequency * mul;
       snprintf(tmpBuffer, 20, "%lu", frequency); //lcdBuffer
       formatFreq();      
     }
     else
     {
       if(endWaitCapture())
       {
         tim2Value = 0;
         tim3Value = 0;
         ClearPeriodCounters();
         lowFrequency = 0;
       }
      
       snprintf(lcdBuffer, 16, "%.3f Hz", lowFrequency);
     }

      lcd_put_cur(0, 0); 
      lcd_send_string(lcdBuffer); 

      lcd_put_cur(1, 0);
      lcd_send_string(lcdBufferLine1); 
     
    secondTimerHandler = false;
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
   }

   if(readButton())
   {
      switchRange();
      secondTimerHandler = true; //For fast update screen
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
  //One second interrupt.  
  if(htim->Instance == TIM1)
	{
    frequency = TIM2->CNT + (TIM3->CNT << 16); 
    HAL_TIM_Base_Stop_IT(&htim1);
    
    //testCounter1 ++;
    TIM2->CNT = 0;
    TIM3->CNT = 0;
    HAL_TIM_Base_Start_IT(&htim1);
  }
 if (htim->Instance == TIM3)
 { 

 }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)
        {
            waitCaptureTimer = 0;
            if(!isFirstCaptured)
            {
              TIM3->CNT = 0;
              TIM2->CNT = 0;
              isFirstCaptured = true;
            } 
            else
            {
              haslowFreqValue  = false;
              tim2Value = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_3);
              tim3Value = TIM3->CNT;
              haslowFreqValue  = true;

              isFirstCaptured = false;
            }
          }
     }
}