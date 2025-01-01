#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gpio.h"
#include "usart.h"
#include "main.h"
#include "ssd1306.h"
#include "stdio.h"
#include "ssd1306_fonts.h"
#include "eeprom.h"
#include "ModbusRTU_Slave.h"

#define NUMBER_OF_MEASUREMENTS 45

extern IWDG_HandleTypeDef hiwdg;

volatile uint32_t TimeTickMs = 0;
uint32_t oldTimeTickHSec = 0;

bool secondTimerHandler = false; //one second has passed
float lastCurrent;

extern uint16_t ModbusRegister[NUMBER_OF_REGISTER];
extern uint16_t AnalogOutputHoldingRegister[NUMBER_OF_ANALOG_REGISTER];
extern bool ModbusCoil[NUMBER_OF_COIL];

extern TIM_HandleTypeDef htim2;

bool measuredPeriodReady = false;//The measured period is ready
float resultFrequency = 0;

//Floating average
uint64_t timerPeriodValues[NUMBER_OF_MEASUREMENTS];
uint8_t measurementCounter = 0; // 0 - (NUMBER_OF_MEASUREMENTS - 1)

uint64_t timerPeriodValues_temp[NUMBER_OF_MEASUREMENTS];

uint8_t i = 0; 

uint32_t numberOfsignalMeasure = 0; //increases if there is a signal, reset every second.
uint32_t lastNumberOfsignalMeasure = 0;
bool deviceStartup = false; 
float tempSum = 0;
float tempFloat = 0;
float averageAllMeasurements;
float sigma = 0;
int goodValueCount = 0;
float avgPeriod = 0; //The calculated period of the last measurement.
int goodValueCountTmp = 0;

bool isFirstCaptured = false;

uint32_t tim2Value, tim3Value;
uint64_t timer32Register; //32 bit register for measuring values. Min freq 0.13hz

bool hasFirstValues = false; //Have 15 measuring.

/**
 * \brief  Performs initialization. 
 *
 */
void init( void )
{
   hasFirstValues = false;
}

/**
 * \brief  Performs additional settings.
 *
 */
void setup( void )
{
    //Delay 1 second.
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET); //White LED on.
    HAL_IWDG_Refresh(&hiwdg);
    HAL_Delay(500);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    HAL_IWDG_Refresh(&hiwdg);
    HAL_Delay(500);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    HAL_IWDG_Refresh(&hiwdg);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET); //White LED off.
    
    HAL_IWDG_Refresh(&hiwdg);
    deviceStartup = true;

}

//The average value of all measurements
float avarage()
{
  tempSum = 0;
  for(i = 0; i < NUMBER_OF_MEASUREMENTS; i++)
  {
    tempSum+= timerPeriodValues_temp[i];
  } 
   
  tempFloat = tempSum / NUMBER_OF_MEASUREMENTS;
  return tempFloat;  
}

//calculation of the standard deviation
float calculateSigma(float avarage)
{
  tempSum = 0;
  for(i = 0; i < NUMBER_OF_MEASUREMENTS; i++)
  {
    tempFloat = (float)(timerPeriodValues_temp[i]) - avarage;
    tempSum+= tempFloat * tempFloat;
  }

  tempFloat = tempSum / (NUMBER_OF_MEASUREMENTS - 1);

  return sqrt(tempFloat);  
}

//floating average
float calculatingFloatingAverage(float avarage, float sigma)
{
  tempSum = 0;
  goodValueCount = 0;
  for(i = 0; i < NUMBER_OF_MEASUREMENTS; i++)
  {
    tempFloat = abs((float)(timerPeriodValues_temp[i]) - avarage);
    if(tempFloat > sigma) continue;

    tempSum+= timerPeriodValues_temp[i];
    goodValueCount++;
  }

  goodValueCountTmp = goodValueCount;
  return tempSum / goodValueCount;
}

void calculatingAverage()
{
    if(measuredPeriodReady) //synchronization with the timer2
    { 
       //Fast copy main to temp array.
      memcpy(timerPeriodValues_temp, timerPeriodValues, 8 * NUMBER_OF_MEASUREMENTS); 

      HAL_IWDG_Refresh(&hiwdg);
      averageAllMeasurements = avarage();
      HAL_IWDG_Refresh(&hiwdg);
      sigma = calculateSigma(averageAllMeasurements);
      HAL_IWDG_Refresh(&hiwdg);

      avgPeriod = calculatingFloatingAverage(averageAllMeasurements, sigma);
      HAL_IWDG_Refresh(&hiwdg);       
      
      if(avgPeriod > 10) //so that there is no division by zero
      {           
        resultFrequency = ((float)70000000 / (((float)avgPeriod) / 8));       
      }  

      measuredPeriodReady = false; 
      HAL_IWDG_Refresh(&hiwdg); 
    }  
}

void resetFrequencyData()
{
  if(deviceStartup == true & (lastNumberOfsignalMeasure == numberOfsignalMeasure))
  {
    avgPeriod = 0; //Clear value.
    resultFrequency = 0;
    memset(timerPeriodValues_temp, 0, sizeof(timerPeriodValues_temp)); 
    goodValueCountTmp = 0;   
  }
  else
  {
    if(numberOfsignalMeasure > 0xFFFFF) 
    {
      numberOfsignalMeasure = 0;
    }

    lastNumberOfsignalMeasure = numberOfsignalMeasure;
  }
}

void updateOutputs()
{   
   if(ModbusCoil[0] == true)
   {
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
   }
   else
   {
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
   }
}

/*
void moveRightMeasurement()
{
    for (i = NUMBER_OF_MEASUREMENTS - 1; i > 0; i--) 
    {
        timerPeriodValues[i] = timerPeriodValues[i - 1];
    }
}
*/

void moveLeftMeasurement()
{
   i = 1;
   while (i < NUMBER_OF_MEASUREMENTS)
   {
      timerPeriodValues[i - 1] = timerPeriodValues[i];
      i++;
   }
}

/**
 * \brief   It is performed periodically in the body of the main loop.
 *
 */
void loop( void )
{   
    HAL_IWDG_Refresh(&hiwdg);
    uartDataHandler();
    
    calculatingAverage();
   
    memcpy(ModbusRegister, &resultFrequency, sizeof(float)); //Convert to array.
    //memcpy(&ModbusRegister[3], &averageAllMeasurements, sizeof(float)); //Convert to array.
    memcpy(&ModbusRegister[5], &sigma, sizeof(float)); //Convert to array.
    memcpy(&ModbusRegister[7], &goodValueCountTmp, 1); //Convert to array.
    /* 
    memcpy(&ModbusRegister[9], &windowSize, 1); //Convert to array.
    */

    updateOutputs();
    
    //Second timer.
    if(secondTimerHandler == true)
    {
      HAL_IWDG_Refresh(&hiwdg);  
      
      HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);    
      
      HAL_IWDG_Refresh(&hiwdg);
      
      resetFrequencyData(); //Clear data if no signal. 
      
      secondTimerHandler = false;
      deviceStartup = true;      
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
            if(!isFirstCaptured)
            {
              TIM3->CNT = 0;
              TIM2->CNT = 0;
              TIM1->CNT = 0;

              isFirstCaptured = true;
            } 
            else
            {
              tim2Value = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_3);
              tim3Value = TIM3->CNT;
              timer32Register = tim2Value + (tim3Value * 65535);

              isFirstCaptured = false;

              if(!hasFirstValues) //no 15 measurement
              {
                timerPeriodValues[measurementCounter] = timer32Register;
              }
              else
              {
                 moveLeftMeasurement();
                 timerPeriodValues[NUMBER_OF_MEASUREMENTS - 1] = timer32Register;

               if(measuredPeriodReady == false) //The main program get the data
               {              
                 measuredPeriodReady = true;              
               }
              }              

             if(measurementCounter < NUMBER_OF_MEASUREMENTS - 1)
             {              
                measurementCounter ++;
             }
             else
             {
                hasFirstValues = true;
                measurementCounter = 0;

               //Synchronization with the main program
               if(measuredPeriodReady == false) //The main program get the data
               {              
                 measuredPeriodReady = true;              
               }          
             }   

              numberOfsignalMeasure++; 
            }      
        }
    }
}

/*
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{   
 if (htim->Instance == TIM2)
 {  
    tim2InterruptsCounter ++;   
 }
}
*/