
#include "TM1638.h"

extern SPI_HandleTypeDef hspi2;
uint8_t DisplBuffer[17]={0}, keyBuffer[4]={0};
const uint8_t digit[] = 
{
  0x3F, // 0
  0x06, // 1
  0x5B, // 2
  0x4F, // 3
  0x66, // 4
  0x6D, // 5
  0x7D, // 6
  0x07, // 7
  0x7F, // 8
  0x6F, // 9
  0x77, // A
  0x7C, // B
  0x39, // C
  0x5E, // d
  0x79, // E
  0x71, // F
  0x76, // H
  0x78, // t
  0x73, // P
  0x63, // TOPo
  0x37, // �
  0x23, // TOPn
  0x54, // n
  0x5C, // o
  0x58, // c
  0x62, // TOPu
  0x1C, // u
  0x40, // MINUS Mid
  0x01, // MINUS Top
  0x08  // MINUS Bott
};

void SendCmdTM1638(uint8_t cmd){
	STB_L();
	DisplBuffer[0] = cmd;       // Set command
	HAL_SPI_Transmit(&hspi2,(uint8_t*)DisplBuffer, 1, 5000);
	STB_H();
}

void SendDataTM1638(void){
	SendCmdTM1638(0x40);          // Set command for writing data into display memory, in the mode of auto address increment by (40H)
	STB_L();
	DisplBuffer[0] = 0x0C0;       // Set starting address (0C0H)
	HAL_SPI_Transmit(&hspi2,(uint8_t*)DisplBuffer, 17, 5000);
	STB_H();
}

void ReadKeyTM1638(void){
	STB_L();  
	keyBuffer[0] = 0x42; 			    // Set data commend for key reading (42H)
	HAL_SPI_Transmit(&hspi2,(uint8_t*)keyBuffer, 1, 5000);

	HAL_SPI_Receive(&hspi2,(uint8_t*)keyBuffer, 4, 5000);
	STB_H();
}

void LedOn(uint8_t pos, uint8_t led){
    pos <<= 1; pos+=2;
    DisplBuffer[pos] |= led;
}

void LedInverse(uint8_t pos, uint8_t led){
    pos <<= 1; pos+=2;
    DisplBuffer[pos] ^= led;
}

void LedOff(uint8_t pos){
    pos <<= 1; pos+=2;
    DisplBuffer[pos] = 0;
}

void AllLedOff(void){
    for (uint8_t i=2;i<17;i+=2) DisplBuffer[i] = 0;
}

void PointOn(uint8_t pos){
    pos <<= 1; pos++;
    DisplBuffer[pos] |= 0x80;
}

void PointInverse(uint8_t pos){
    pos <<= 1; pos++;
    DisplBuffer[pos] ^= 0x80;
}

void PointOff(uint8_t pos){
    pos <<= 1; pos++;
    DisplBuffer[pos] &= 0x7F;
}

void setChar(uint8_t pos, uint8_t nuber)
{
    pos <<= 1; pos++;
    DisplBuffer[pos] = digit[nuber];
}
