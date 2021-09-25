#include "main.h"
#include "ssd1306.h"
#include <string.h>

extern uint8_t aTxBuffer[];
extern char nrf_str1[];

void RTC_Convert(void)
{
  SSD1306_Fill(SSD1306_COLOR_BLACK); //clear oled
  SSD1306_GotoXY(0,0);

  sprintf(nrf_str1,"%02x.",aTxBuffer[4]);//date
  SSD1306_Puts(nrf_str1, &Font_11x18, SSD1306_COLOR_WHITE);

  sprintf(nrf_str1,"%02x.",aTxBuffer[5]);//month
  SSD1306_Puts(nrf_str1, &Font_11x18, SSD1306_COLOR_WHITE);

  sprintf(nrf_str1,"%02x",aTxBuffer[6]);//year
  SSD1306_Puts(nrf_str1, &Font_11x18, SSD1306_COLOR_WHITE);
/*  
  day = RTC_ConvertFromDec(aTxBuffer[3]); //Преобразуем в десятичный формат
  d1 = ((temp/10)%10) + 0x30;
  d2 = (temp%10) + 0x30;
  sprintf(nrf_str1,"%c%c",d1,d2);
*/   
  SSD1306_GotoXY(0,30);
  sprintf(nrf_str1,"%02x:",aTxBuffer[2]);// hore
  SSD1306_Puts(nrf_str1, &Font_11x18, SSD1306_COLOR_WHITE);
  
  sprintf(nrf_str1,"%02x:",aTxBuffer[1]);// min.
  SSD1306_Puts(nrf_str1, &Font_11x18, SSD1306_COLOR_WHITE);
  
  sprintf(nrf_str1,"%02x",aTxBuffer[0]);// sec.
  SSD1306_Puts(nrf_str1, &Font_11x18, SSD1306_COLOR_WHITE);
  SSD1306_UpdateScreen();
}
