
#include "TM1638.h"

extern uint8_t DisplBuffer[], keyBuffer[];
extern uint8_t changeDispl;
extern SPI_HandleTypeDef hspi2;
extern uint8_t digit[];

void SendCmdTM1638(uint8_t cmd){
	STB_L();
	DisplBuffer[0] = cmd;       // Set command
	HAL_SPI_Transmit(&hspi2,(uint8_t*)DisplBuffer, 1, 5000);
	STB_H();
}

void SendDataTM1638(void){
	changeDispl = 0;
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

void LedOn(uint8_t pos){
    pos <<= 1; pos+=2;
    DisplBuffer[pos] = 1;
    changeDispl = 1;
}

void LedInverse(uint8_t pos){
    pos <<= 1; pos+=2;
    DisplBuffer[pos] ^= 1;
    changeDispl = 1;
}

void LedOff(uint8_t pos){
    pos <<= 1; pos+=2;
    DisplBuffer[pos] = 0;
    changeDispl = 1;
}

void AllLedOff(void){
    for (uint8_t i=2;i<17;i+=2) DisplBuffer[i] = 0;
    changeDispl = 1;
}

void PointOn(uint8_t pos){
    pos <<= 1; pos++;
    DisplBuffer[pos] |= 0x80;
    changeDispl = 1;
}

void PointInverse(uint8_t pos){
    pos <<= 1; pos++;
    DisplBuffer[pos] ^= 0x80;
    changeDispl = 1;
}

void PointOff(uint8_t pos){
    pos <<= 1; pos++;
    DisplBuffer[pos] &= 0x7F;
    changeDispl = 1;
}

void setChar(uint8_t pos, uint8_t nuber)
{
    pos <<= 1; pos++;
    DisplBuffer[pos] = digit[nuber];
    changeDispl = 1;
}
