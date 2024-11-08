#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gpio.h"
#include "usart.h"
#include "main.h"
#include "ssd1306.h"
#include "stdio.h"
#include "ssd1306_fonts.h"

extern IWDG_HandleTypeDef hiwdg;
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim2;

#define DISPLAY_VOLTAGE 0
#define DISPLAY_PERCENTS 1

#define ADC_NUMBER_OF_CHANNELS 1
#define MAX_QUANTITY_MEASUREMENTS 16

#define ADC_REFERENCE_VOLTAGE 33 //*10
#define DIVISION_COEFFICIENTS_VOLTAGE 1 //5.309

volatile uint32_t TimeTickMs = 0;
uint32_t oldTimeTickHSec = 0;
bool secondTimerHandler = false; //one second has passed

char displayStr[300]; //To display data on the screen.

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

    //Delay 1 second.
    //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13, GPIO_PIN_RESET); //White LED on.
    HAL_IWDG_Refresh(&hiwdg);
    HAL_Delay(500);
    
    ssd1306_Init(); 
        
    HAL_IWDG_Refresh(&hiwdg);   
}

//Convert float value to string
void toFloatStr(float data, char* str)
{
  sprintf(str, "%d.%02d", (uint32_t)data, (uint16_t)((data - (uint32_t)data) * 100.)); 
}

void toFloatStrShort(float data, char* str)
{
  sprintf(str, "%d.%d", (uint32_t)data, (uint16_t)((data - (uint32_t)data) * 10.)); 
}

//Show on display the current or voltage
void printDisplayParameter(float data, uint8_t paramType, bool shortFormat)
{
   if(shortFormat)
   {
      toFloatStrShort(data, displayStr);
   }
   else
   {
     toFloatStr(data, displayStr);
   }
   

   char* simvol = "B";

   switch (paramType)
   {
    case DISPLAY_VOLTAGE: 
      ssd1306_SetCursor(0, 18); //0, 45
    break;
  
    case DISPLAY_PERCENTS:
      sprintf(displayStr,"%d ", (uint8_t)data);
      ssd1306_SetCursor(1, 45);//85,18
      
      simvol = "%";
    break; 
 
    default:
    break;
   }
  

   if(simvol != 0)
   {
       //For string with 3digits.
       /*
      if((strlen(displayStr) < 4) && paramType != DISPLAY_PERCENTS)
      {
        ssd1306_MoveCursor(7, 0);
      }    
      */
      //ssd1306_WriteString(displayStr, Font_11x18, White);
      
   }
   else
   {
    //ssd1306_PrintString("", 2);
    //ssd1306_WriteString("", Font_11x18, White);
    //ssd1306_WriteSpecialSimvolString(simvol, SpecialCharacters_11x18, White); 
   }
  
   ssd1306_WriteSpecialSimvolString(displayStr, SpecialCharacters_11x18, White); 
   ssd1306_MoveCursor(0, 7);
   ssd1306_WriteSpecialSimvolString(simvol, SpecialCharacters_7x10, White);    
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
      
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);      

      ssd1306_Fill(Black);
      ssd1306_SetCursor(0, 0);
      ssd1306_PrintString("Напряжение питания", 2);

      currentVoltage = (float)(adcResults[0]) * ADC_REFERENCE_VOLTAGE * DIVISION_COEFFICIENTS_VOLTAGE / 40960;
      printDisplayParameter(currentVoltage, DISPLAY_VOLTAGE, false);

      printDisplayParameter(99.15, DISPLAY_PERCENTS, true); 

      ssd1306_UpdateScreen();

      /*
      ssd1306_Fill(Black);
      ssd1306_SetCursor(0, 20);      
      //sprintf(displayStr, "%d", ModbusRegister[0]);
       ssd1306_WriteSpecialSimvolString("1", SpecialCharacters_11x18, White);
       ssd1306_WriteSpecialSimvolString("-1", SpecialCharacters_11x18, White)
         ssd1306_SetCursor(0, 40);
               ssd1306_WriteSpecialSimvolString(displayStr, SpecialCharacters_11x18, White);
      ssd1306_UpdateScreen();
          HAL_Delay(500);
      */
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
 if (htim->Instance == TIM2)
 {  
  
 }

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