/*
 * Modbus_Lib.c
 *
 *  Created on: Feb 10, 2022
 *      Author: Ümit Can Güveren
 */
#include <ModbusRTU_Slave.h>
#include "main.h"

uint8_t uartRxData;
uint8_t DataCounter;
uint8_t uartTimeCounter;

uint16_t ModbusRegister[NUMBER_OF_REGISTER];
uint16_t AnalogOutputHoldingRegister[NUMBER_OF_ANALOG_REGISTER];

bool ModbusCoil[NUMBER_OF_COIL];

char ModbusRx[BUFFERSIZE];
char tempModbusRx[BUFFERSIZE];
char ModbusTx[BUFFERSIZE];

uint16_t rxCRC;

//The package contains our address
bool getPacketForAs = false;

extern UART_HandleTypeDef huart1;

/*Receive data interrupt*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	uartTimeCounter = 0;

	//For some reason, we have not processed the data package yet.
	if(getPacketForAs)
	{		
		return;
	}
	
	ModbusRx[DataCounter] = uartRxData;

	//GetFullPack from master;
	if(DataCounter == 7) 
	{
		if(ModbusRx[0] == SLAVEID)
		{
			getPacketForAs = true;
		}

		DataCounter = 0;
	}
	else
	{
		DataCounter++;
	}

	HAL_UART_Receive_IT(&huart1 , &uartRxData , 1);
}

/*calls the corresponding function according to the received function command*/
void transmitDataMake(char *msg, uint8_t Lenght)
{
	switch(msg[1])
	{
	case ReadCoil:
		makePacket_01(msg, Lenght);
		break;

	case ReadHoldingRegister:
		makePacket_03(msg, Lenght);
		break;
    
    case ReadInputRegisters:
		makePacket_04(msg, Lenght);
	break;

	case WriteSingleRegister:
		makePacket_06(msg, Lenght);
		break;

	case WriteSingleCoil:
		makePacket_05(msg, Lenght);
		break;

	case WriteMultipleCoils:
		makePacket_15(msg, Lenght);
		break;

	case WriteMultipleResisters:
		makePacket_16(msg, Lenght);
		break;
	}

}

/*Runs when data retrieval is complete and check CRC*/
void uartDataHandler(void)
{
	if(getPacketForAs == false) return;

	uint8_t tempCounter;
	uint16_t CRCValue;

	memcpy(tempModbusRx, ModbusRx, 8);

	tempCounter = 8;//DataCounter;
	memset(ModbusRx, 0, BUFFERSIZE);
	memset(ModbusTx, 0, BUFFERSIZE);

	/*CRC Check*/
	CRCValue = MODBUS_CRC16(tempModbusRx, tempCounter - 2);
	rxCRC = (tempModbusRx[tempCounter -1] << 8) | (tempModbusRx[tempCounter - 2]);

	/*If the calculated CRC value and the received CRC value are equal and the Slave ID is correct, respond to the receiving data.  */
	if(rxCRC == CRCValue && tempModbusRx[0] == SLAVEID)
	{
	   transmitDataMake(&tempModbusRx[0], tempCounter);
	}

	getPacketForAs = false;
}

/* This function should be called in systick timer */
/* This function should be called in systick timer */
void uartTimer(void)
{
	//has pause - reset;
	if(uartTimeCounter > 3)
	{
		uartTimeCounter = 0;
		DataCounter = 0;
	}
	else
	{
		uartTimeCounter++; 
	}
}

void sendMessage(char *msg, uint8_t len)
{
	HAL_GPIO_WritePin(RS485_TX_EN_PORT, RS485_TX_EN_PIN, GPIO_PIN_SET);
	HAL_UART_Transmit_IT(&huart1, (uint8_t *)msg, len);
}

/*Transmit data interrupt*/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
	HAL_GPIO_WritePin(RS485_TX_EN_PORT, RS485_TX_EN_PIN, GPIO_PIN_RESET);
  }
}


/*****Modbus Function*****/
//The function are used to respond to receiving modbus data.

/*Send coil data*/
void makePacket_01(char *msg, uint8_t Lenght)
{
	uint16_t RegAddress, NumberCoils, NumberByte, CRCValue;
	RegAddress = (msg[2] << 8) | (msg[3]);	//Starting address of the coils to be read
	NumberCoils = (msg[4] << 8) | (msg[5]); //Number of coils to be read
	NumberByte = findByte(NumberCoils);		//Data byte to be transmit

	uint8_t tempCoil[NumberByte];
	uint8_t CoilCount, i = 0, ByteCount = 0;

	ModbusTx[0] = msg[0];
	ModbusTx[1] = msg[1];
	ModbusTx[2] = NumberByte;


	for(CoilCount = 0; CoilCount < NumberCoils; CoilCount++)
	{
			if( CoilCount % 8 == 0 && CoilCount != 0)
			{
				ByteCount++;
				i = 0;
			}
			tempCoil[ByteCount] |= (ModbusCoil[RegAddress + CoilCount] << i);
			i++;
	}

	for(i = 0; i < ByteCount + 1; i++)
	{
		ModbusTx[3 + i] = tempCoil[i];
	}

	/*Calculating the CRC value of the data to be sent*/
	CRCValue = MODBUS_CRC16(ModbusTx, 3 + i);
	ModbusTx[3 + i] = (CRCValue & 0x00FF);
	ModbusTx[4 + i] = (CRCValue >> 8);
	/**************************************************/

	sendMessage(ModbusTx, 5 + i); //Send response data
	memset(tempCoil, 0, 8 * ByteCount); //Clear tempCoil array
}

/*Send register data*/
void makePacket_03(char *msg, uint8_t Lenght)
{
	uint8_t i, m = 0;

	uint16_t RegAddress = 0;
	uint16_t NumberOfReg = 0;
	uint16_t CRCValue;

	RegAddress = (msg[2] << 8) | (msg[3]);
	NumberOfReg = (msg[4] << 8) | (msg[5]);
	ModbusTx[0] = msg[0];
	ModbusTx[1] = msg[1];
	ModbusTx[2] = (NumberOfReg * 2);

	for(i = 0; i < NumberOfReg * 2; i += 2)
	{
		ModbusTx[3 + i] = (uint8_t)(ModbusRegister[RegAddress + m] >> 8);
		ModbusTx[4 + i] = (uint8_t)(ModbusRegister[RegAddress + m] & 0x00FF);
		m++;
	}

	//CRC Calculate
	CRCValue = MODBUS_CRC16(ModbusTx, 3 + (NumberOfReg * 2 ));
	ModbusTx[4 + (NumberOfReg * 2 )] = (CRCValue >> 8);
	ModbusTx[3 + (NumberOfReg * 2 )] = (CRCValue & 0x00FF);
	/********************************************************/
	sendMessage(ModbusTx, 5 + (NumberOfReg * 2 ));
}

/*Read Input Registers*/
void makePacket_04(char *msg, uint8_t Lenght)
{
	uint8_t i, m = 0;

	uint16_t RegAddress = 0;
	uint16_t NumberOfReg = 0;
	uint16_t CRCValue;

	RegAddress = (msg[2] << 8) | (msg[3]);
	NumberOfReg = (msg[4] << 8) | (msg[5]);
	ModbusTx[0] = msg[0];
	ModbusTx[1] = msg[1];
	ModbusTx[2] = (NumberOfReg * 2);

	for(i = 0; i < NumberOfReg * 2; i += 2)
	{
		ModbusTx[3 + i] = (uint8_t)(ModbusRegister[RegAddress + m] >> 8);
		ModbusTx[4 + i] = (uint8_t)(ModbusRegister[RegAddress + m] & 0x00FF);
		m++;
	}

	//CRC Calculate
	CRCValue = MODBUS_CRC16(ModbusTx, 3 + (NumberOfReg * 2 ));
	ModbusTx[4 + (NumberOfReg * 2 )] = (CRCValue >> 8);
	ModbusTx[3 + (NumberOfReg * 2 )] = (CRCValue & 0x00FF);
	/********************************************************/
	sendMessage(ModbusTx, 5 + (NumberOfReg * 2 ));
}

/*Write single coil*/
void makePacket_05(char *msg, uint8_t Lenght)
{
	uint16_t RegAddress, RegValue;
	RegAddress = (msg[2] << 8) | (msg[3]);
	RegValue = (msg[4] << 8) | (msg[5]);

	ModbusCoil[RegAddress] = (bool)RegValue;

	sendMessage(msg, Lenght);
}

/*Write single register*/
void makePacket_06(char *msg, uint8_t Lenght)
{
	uint16_t RegAddress, RegValue;
	RegAddress = (msg[2] << 8) | (msg[3]);
	RegValue = (msg[4] << 8) | (msg[5]);

	AnalogOutputHoldingRegister[RegAddress] = RegValue;
	sendMessage(msg, Lenght);
}

/*Write multiple coils*/
void makePacket_15(char *msg, uint8_t Lenght)
{
	uint16_t NumberOfCoils, CRCValue;
	uint8_t i = 0, m = 0, k = 0;
	//uint16_t NumberOfByte, RegAddress; -> not used
	//RegAddress = (msg[2] << 8) | (msg[3]);

	NumberOfCoils = (msg[4] << 8) | (msg[5]);

	//NumberOfByte = msg[6];

	for(i = 0; i < NumberOfCoils; i++)
	{
		ModbusCoil[i] = (msg[7 + m] & (0x01 << k)) ;

		k++;
		if(k % 8 == 0 && k != 0)
		{
			m++;
			k = 0;
		}
	}

	memcpy(ModbusTx, msg, 6);

	CRCValue = MODBUS_CRC16(ModbusTx, 6);
	ModbusTx[6] = (CRCValue & 0x00FF);
	ModbusTx[7] = (CRCValue >> 8);
	sendMessage(ModbusTx, 8);
}


/*Write multiple registers*/
void makePacket_16(char *msg, uint8_t Lenght)
{
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
	uint16_t RegAddress, NumberOfReg, CRCValue;
	uint8_t i,m = 0;
	RegAddress = (msg[2] << 8) | (msg[3]);
	NumberOfReg = (msg[4] << 8) | (msg[5]);

	for(i = 0; i < NumberOfReg; i++)
	{
		ModbusRegister[RegAddress + i] = (uint16_t)((uint16_t)msg[7 + m] << 8) | (msg[8 + m]);
		m += 2;
	}

    memcpy(ModbusTx, msg, 6);

	CRCValue = MODBUS_CRC16(ModbusTx, 6);
	ModbusTx[6] = (CRCValue & 0x00FF);
	ModbusTx[7] = (CRCValue >> 8);
	sendMessage(ModbusTx, 8);
}

uint8_t findByte(int16_t NumberOfCoil)
{
	volatile uint8_t NumberOfByte = 0;

	while(NumberOfCoil >= 0)
	{
		NumberOfCoil -= 8;

		NumberOfByte++;
		if(NumberOfCoil < 0)
		{
			break;
		}
	}
	return NumberOfByte;
}

uint16_t MODBUS_CRC16(char *buf, uint8_t len )
{
	static const uint16_t table[2] = { 0x0000, 0xA001 };
	uint16_t crc = 0xFFFF;
	unsigned int i = 0;
	char bit = 0;
	unsigned int xor = 0;

	for( i = 0; i < len; i++ )
	{
		crc ^= buf[i];

		for( bit = 0; bit < 8; bit++ )
		{
			xor = crc & 0x01;
			crc >>= 1;
			crc ^= table[xor];
		}
	}

	return crc;
}

