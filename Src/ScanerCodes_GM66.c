#include "..\Inc\ScanerCodes_GM66.h"
#include "stdint.h"
#include "main.h"
extern UART_HandleTypeDef huart6;
extern uint8_t Command[20];
//extern uint8_t UartData[100];
/*******************************************************************/
//Calculate CRC
//
/*
unsigned int crc_cal_by_bit(uint8_t* ptr, unsigned int len)
{ 
unsigned int crc = 0;
	while(len-- != 0)
	{ 
		for(uint8_t i = 0x80; i != 0; i /= 2)
		{ crc *= 2;
			if((crc&0x10000) !=0)
				crc ^= 0x11021;
			if((*ptr&i) != 0)
				crc ^= 0x1021;
		} 
		ptr++;
	} 
	return crc;
}
*/
/*******************************************************************/
//ReadZoneBit
//Address : 0x0000~0x00FF( 2 bytes) , address to start reading zone bit
//Num : 0x00~0xFF( 1 byte) , Numbers of zone bit for Sequential read, 0x00= 256byets
uint8_t* WriteZoneBit(uint16_t Adress, uint8_t Data, uint8_t* Command)
{
		Command[0] = 0x7E;
		Command[1] = 0x0; 
		Command[2] = 0x08;
		Command[3] = 0x1; 
		Command[4] = (uint8_t)(Adress>>8); 
		Command[5] = (uint8_t)(Adress);
		Command[6] = (uint8_t)(Data);
		//unsigned int CRC = crc_cal_by_bit(&Command[0], 7);
		//Command[7] = (uint8_t)(CRC>>8);
		//Command[8] = (uint8_t)(CRC);
		Command[7] = 0xAB;
		Command[8] = 0xCD;
	return &Command[0];
}
uint8_t* ReadZoneBit(uint16_t Adress, uint8_t Num, uint8_t* Command)
{
		Command[0] = 0x7E;
		Command[1] = 0x0; 
		Command[2] = 0x07;
		Command[3] = 0x1; 
		Command[4] = (uint8_t)(Adress>>8); 
		Command[5] = (uint8_t)(Adress);
		Command[6] = (uint8_t)(Num);
		//unsigned int CRC = crc_cal_by_bit(&Command[0], 7);
		//Command[7] = (uint8_t)(CRC>>8);
		//Command[8] = (uint8_t)(CRC);
		Command[7] = 0xAB;
		Command[8] = 0xCD;
	return &Command[0];
}
uint8_t* ReadCode(uint8_t* Command)
{
		Command[0] = 0x7E;
		Command[1] = 0x0; 
		Command[2] = 0x08;
		Command[3] = 0x1; 
		Command[4] = 0x0; 
		Command[5] = 0x2;
		Command[6] = 0x1;
		Command[7] = 0xAB;
		Command[8] = 0xCD;
		return Command;
}

void WriteZoneByte(uint16_t Adress, uint8_t Data)
{
		WriteZoneBit( Adress, Data, Command );
		HAL_UART_Transmit(&huart6, Command, 9, 100);
}
/*
void ReadZoneByte(uint16_t Adress, uint8_t Number)
{
		ReadZoneBit( Adress, Number, Command );
		HAL_UART_Transmit(&huart6, Command, 9, 100);
		HAL_UART_Receive_IT(&huart6, UartData, 100 );	
}
*/

void InitBarcodeReader()
{
	// Open LED when successfully read/Mute off/Standard/No light/Command mode
	WriteZoneByte(0x0, 0xD1);
	//serial port output
	WriteZoneByte(0x0D, 0x4);
}
