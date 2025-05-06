#include "led.h"
#include "ErrorCodes.h"

uint8_t pointData; //which digits the dot be displayed
uint8_t displayArray[3] = {0, 0, 0};
uint8_t displayBuffer[3] = {0, 0, 0};
uint8_t currentSegment; //For dynamic led.
bool needBlink;

uint16_t blinkLedTimer;
uint8_t blinkLedDirection;

void initLed()
{
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET); //Off segment 1
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); //Off segment 2
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); //Off segment 3

  //off anods
  GPIOA->ODR |= 0x0F0;    
  GPIOB->ODR |= 0xC03; 
}

//Convert hex number to 3 decimal digit.
void hexToDec(uint16_t value)
{
   uint8_t digits[3] = {0, 0, 0};
	
	while (value >= 100)
	{
		value-= 100;
		digits[2]++;
	}	
	
	while (value >= 10)
	{
		value-= 10;
		digits[1]++;
	}	
	
	while (value >= 1)
	{
		value-= 1;
		digits[0]++;
	}	
 
  //offZeros(digits);
  memcpy(displayArray, digits, 3);
}

//Convert dig to 7 seg led matrix.
void digToSegments(uint8_t dig, uint8_t point)
{
   if(dig > LED_MATRIX_SIZE - 1) //Protected overflow.
   {
     return;
   }

   uint8_t led = ledMatrix[dig];
   uint8_t portA = (led & 0xF0) | 0x0F;
   GPIOA->ODR &= portA;  

   uint16_t portB = (uint16_t)(led & 0x0F);
   uint16_t lsb = (portB & 0x03) | 0xFFFC;

   if (point)
   {
    lsb &= 0xFFFD; //dp - pb1
   }
   else
   {
     lsb |= 2; //Off point;
   }

   GPIOB->ODR &= lsb;

   portB >>= 2;
   uint16_t msb = (portB & 0x03);// | 0xFFFC;
   msb <<= 10;
   msb |= 0xF3FF;
   
   GPIOB->ODR &= msb;   
}

//Disables zeros in the higher registers. If need.
void offZeros(uint8_t *arr)
{
  bool msbZero = true;
  for (uint8_t i = 2; i > 0; i--)
  {
     if(!msbZero)
     {
      break;
     }

     if(arr[i] == 0)
     {
      arr[i] = OFF_LED_SEGMENT_VALUE;
     }
     else
     {
       msbZero = false;
     }
  } 
}

//Generates an error code
void createErrorCodes(uint16_t value)
{
   uint16_t code = value - LED_ERROR_ARIA;
   uint8_t digits[3] = {0, 0, 0};

   digits[2] = E_LATTER;
   digits[1] = code;
   digits[0] = OFF_LED_SEGMENT_VALUE;

   memcpy(displayArray, digits, 3); 
}

//Run logic in the main loop of the program.
void ledDisplayHandler(uint16_t digit)
{
  if(digit < LED_ERROR_ARIA)
  {
    hexToDec(digit);
  }
  else
  {
    createErrorCodes(digit);
  }
}

//Soft timer.
bool blinkLedTimerHandler()
{  
   if(!blinkLedDirection)
   {
      if(blinkLedTimer < BLINK_LED_INTERVAL)
      {
        blinkLedTimer ++;
        return false;
      }
      else
      {
        blinkLedDirection = 1;
        return true;
      }
   }
   else
   {
      if(blinkLedTimer)
      {
        blinkLedTimer --;
        return true;
      }
      else
      {
        blinkLedDirection = 0;
        return false;
      }
   } 
}

//Generate a dynamic indication
void dynamicIndication()
{  
  memcpy(displayBuffer, displayArray, 3); 
  
  if(needBlink == true && blinkLedTimerHandler())
  {
    memset(displayBuffer, OFF_LED_SEGMENT_VALUE, 3); //Off display.
  }

  uint8_t pointOn = 0;
   //off anods
   GPIOA->ODR |= 0x0F0;    
   GPIOB->ODR |= 0xC03; 

   switch (currentSegment)
   {
     case 0:
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET); //Off segment 1
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); //Off segment 2
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); //Off segment 3
        pointOn = pointData & 0x01;
        digToSegments(displayBuffer[0], pointOn);
        currentSegment = 1;              
     break;
   
     case 1:
       HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET); //Off segment 1
       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); //Off segment 2
       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); //Off segment 3
       pointOn = (pointData & 0x02) >> 1;
       digToSegments(displayBuffer[1], pointOn);
       currentSegment = 2;
     break;

     case 2:
       HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET); //Off segment 1
       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); //Off segment 2
       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); //Off segment 3
       pointOn = (pointData & 0x04) >> 2;
       digToSegments(displayBuffer[2], pointOn);
       currentSegment = 0;    
     break;

    default:
    break;
   }
}