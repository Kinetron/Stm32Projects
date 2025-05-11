#include "PulseSender.h"

uint32_t dataOutCounter; //Count pulse packet.
bool dataPacketSend; //=true if send.
uint8_t dataOutPauseCnt; //Pause counter.
uint32_t temperatureForSend; //For send data as pulse packet use tim4.
bool temperatureDataOutState;

void pulseSenderHandler()
{
  if(dataPacketSend) //Pause
  {      
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
    return;
  }

   //Pulse count.
  if(dataOutCounter < temperatureForSend)
  {
    dataOutCounter ++; 
  }
  else
  {
    dataOutCounter = 0;
    dataPacketSend = true;
    temperatureDataOutState = true; //Low level in output.
  }

  if(temperatureDataOutState)
  {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET); 
    temperatureDataOutState = false;    
  } 
  else
  {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET); 
    temperatureDataOutState = true; 
  }
}

//Generates a pause after sending a burst of pulses
void outDataPause()
{
  if(dataPacketSend)
  {
    if(dataOutPauseCnt <= DATA_PAUSE_INTERVAL )
    {
      dataOutPauseCnt ++;
    }
    else
    {
      dataOutPauseCnt = 0;
      dataPacketSend = false; 
    }
  }  
}