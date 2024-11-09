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

#define ADC_REFERENCE_VOLTAGE 32.5 //~3.3*10
#define DIVISION_COEFFICIENTS_VOLTAGE 4.5047 //14v /3.3v

#define DELAY_ON_TIME 60 //delay load on time

#define MIN_SUPPLY_VOLTAGE 10.3
#define MAX_SUPPLY_VOLTAGE 13

#define MIN_BATTERY_VOLTAGE 7.2  //2.4 *3
#define MAX_BATTERY_VOLTAGE 12.6 //4.2 *3

#define DC_SUPLAY_VOLTAGE 12.1

#define DIFF_VOLTAGE_BATTERY (MAX_BATTERY_VOLTAGE - MIN_BATTERY_VOLTAGE)

#define DISPLAY_ON_TIME 10

#define LOAD_ON_IGNORE_ADC 3

volatile uint32_t TimeTickMs = 0;
uint32_t oldTimeTickHSec = 0;
bool secondTimerHandler = false; //one second has passed

char displayStr[300]; //To display data on the screen.

uint32_t adcAvgBuff[ADC_NUMBER_OF_CHANNELS];
uint32_t adcResults[ADC_NUMBER_OF_CHANNELS];
uint32_t adcData[ADC_NUMBER_OF_CHANNELS]; //Measured adc values.
uint8_t numberMeasurements = 0;

uint8_t delayTimer = 0;
bool loadState;  //Load on/off.

float currentVoltage;

uint8_t displayIndicatorTimerCounter;
bool offByLowVoltage;

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
  
   char* simvol = "V";

   switch (paramType)
   {
    case DISPLAY_VOLTAGE: 
      ssd1306_SetCursor(0, 18); //0, 45
    break;
  
    case DISPLAY_PERCENTS:
      //sprintf(displayStr,"%d ", (uint8_t)data);
      ssd1306_SetCursor(1, 45);//85,18
      
      simvol = "%";
    break; 
 
    default:
    break;
   }
  
   ssd1306_WriteSpecialSimvolString(displayStr, SpecialCharacters_11x18, White); 
   ssd1306_MoveCursor(0, 7);
   ssd1306_WriteSpecialSimvolString(simvol, SpecialCharacters_7x10, White);    
}

void loadControl(float voltage)
{
   bool loadNeedOn = false;
   bool lowVoltage = false;

   if(loadState == false)
   {
      //Delay timer.
      if(delayTimer < DELAY_ON_TIME)
      {
        delayTimer ++;
      }
      else
      {
         loadNeedOn = true; 
      }
   }
     
    //No power on line. Wait line voltage.
   if(offByLowVoltage && DC_SUPLAY_VOLTAGE > voltage)
   {
      return;
   }
 
   lowVoltage = true;
   if(voltage > MIN_SUPPLY_VOLTAGE)
   {
     lowVoltage = false;
   } 

   //Voltage control
   if(!lowVoltage && voltage < MAX_SUPPLY_VOLTAGE)
   {
      if(loadNeedOn)
      {
        loadState = true;
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
        offByLowVoltage = false;
      }
   }
   else
   {    
     if(lowVoltage)
     {
        offByLowVoltage = true;
     }

     //problem with power
     loadState = false;
     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
     delayTimer = 0;
   }   
}

float calcPersents(float batteryVoltage)
{
  float persentCharge = ( (batteryVoltage - MIN_BATTERY_VOLTAGE) / DIFF_VOLTAGE_BATTERY ) * 100;

  if(persentCharge > 100)
  {
    persentCharge = 100;
  }

  return persentCharge;
}

//Сalculates the time after pressing the display power button.
bool displayIndicatorTimer(bool btnState)
{
    if(btnState)
    {
      displayIndicatorTimerCounter = 0;
    }

    if(displayIndicatorTimerCounter < DISPLAY_ON_TIME)
    {
      displayIndicatorTimerCounter ++;
      return true;
    }

    return false;
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
     
      ssd1306_Fill(Black);
      ssd1306_SetCursor(0, 0);

      ssd1306_WriteString("Supply voltage", Font_7x10, White);

      currentVoltage = (float)(adcResults[0]) * ADC_REFERENCE_VOLTAGE * DIVISION_COEFFICIENTS_VOLTAGE / 40960;
      printDisplayParameter(currentVoltage, DISPLAY_VOLTAGE, false);

      printDisplayParameter(calcPersents(currentVoltage), DISPLAY_PERCENTS, true);
      ssd1306_UpdateScreen();
      HAL_IWDG_Refresh(&hiwdg);
      loadControl(currentVoltage);
      
      //Off display if button no pressed.
      if(displayIndicatorTimer(HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_8)) == false)
      {
        ssd1306_SetContrast(0);
      }
      else
      {
        ssd1306_SetContrast(255);
      }

      //Output led state.
      if(HAL_GPIO_ReadPin (GPIOB, GPIO_PIN_1))
      {
         HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET); 
      }
      else
      {
         HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6); 
      }

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