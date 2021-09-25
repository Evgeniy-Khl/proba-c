#include "main.h"
#include "ssd1306.h"
#include <string.h>
extern uint8_t aTxBuffer[];

void rtc_error(uint8_t status)
{
  SSD1306_GotoXY(0,40);
  switch(status)
   {
     case HAL_OK:
         SSD1306_Puts("Ok!", &Font_11x18, SSD1306_COLOR_WHITE); break;
     case HAL_ERROR:
         SSD1306_Puts("Error", &Font_11x18, SSD1306_COLOR_WHITE); break;
     case HAL_BUSY:
         SSD1306_Puts("Busy", &Font_11x18, SSD1306_COLOR_WHITE); break;
     case HAL_TIMEOUT:
         SSD1306_Puts("TimeOut", &Font_11x18, SSD1306_COLOR_WHITE); break;
   }
  SSD1306_UpdateScreen();
  HAL_Delay(2000);
}
/*-
* ¬ходные параметры в первой функции:
* EEPROM_I2C_PORT Ч идентификатор шины I2C.
* [0xD0] Ч адрес устройства DS3231 real-time clock (RTC)
* &aTxBuffer - адрес масива в котором содержитс€ кол-во байт дл€ передачи.
* amount Ч количество байт, которые мы будем передавать.
* [1000] - timeout
-*/
uint8_t ret_stat;

void RTC_Write(uint8_t address, uint8_t amount)
{
  aTxBuffer[0]=address;
  ret_stat = HAL_I2C_Master_Transmit(&EEPROM_I2C_PORT, 0xD0, (uint8_t*)aTxBuffer, amount, 1000); // HAL expects address to be shifted one bit to the left (0xD0 = 0x68<<1)
  if(ret_stat) rtc_error(ret_stat);
}

void RTC_Read(uint8_t amount)
{
  ret_stat = HAL_I2C_Master_Receive(&EEPROM_I2C_PORT, 0xD0, (uint8_t*)aTxBuffer, amount, 1000); // HAL expects address to be shifted one bit to the left (0xD0 = 0x68<<1)
  if(ret_stat) rtc_error(ret_stat);
}

void RTC_init()
{
  aTxBuffer[1]=0;     // EOSC=0 BBSQW=0 CONV=0 RS2=0 RS1=0 INTCN=0 A2IE=0 A1IE=0
  aTxBuffer[2]=0;     // EN32kHz=0 BSY=0 A2F=0 A1F=0
  RTC_Write(0x0E, 3); // Control Register (0Eh) [aTxBuffer[0]=0x0E]
}
