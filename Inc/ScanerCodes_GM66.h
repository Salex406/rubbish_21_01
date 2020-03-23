#include "stdint.h"
unsigned int crc_cal_by_bit(unsigned char* ptr, unsigned int len);
uint8_t* ReadZoneBit(uint16_t Adress, uint8_t Num, uint8_t* Command);
uint8_t* ReadCode(uint8_t* Command);
uint8_t* WriteZoneBit(uint16_t Adress, uint8_t Num, uint8_t* Command);
uint8_t* ReadZoneBit(uint16_t Adress, uint8_t Num, uint8_t* Command);
void WriteZoneByte(uint16_t Adress, uint8_t Data);
//void ReadZoneByte(uint16_t Adress, uint8_t Number);
void InitBarcodeReader(void);

