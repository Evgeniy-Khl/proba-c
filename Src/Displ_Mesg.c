#include "main.h"
#include "global.h"   // здесь определена структура eeprom и структура rampv
char SSDBuffer[20];
extern uint8_t show, ok0, ok1, setup, waitset, waitkey, servis, displmode, modules, disableBeep, psword, keynum;
extern int16_t alarmErr, buf, currAdc, thermistorAdc, humAdc;

void dsplMss(uint8_t *data, struct rampv *ram){
  uint8_t x,y,i;
  switch(show){
    case 1:
          SSD1306_Fill(SSD1306_COLOR_BLACK); //clear oled
          y=0;
          sprintf(SSDBuffer,"ID  =%2i  condi=%2x",*(data+37),*(data+38)); 
          SSD1306_GotoXY(0,y);
          SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
          SSD1306_UpdateScreen();
          y+=10;
          sprintf(SSDBuffer,"eMode=%2x  rMode=%2i",*(data+39),*(data+40)); 
          SSD1306_GotoXY(0,y);
          SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
          SSD1306_UpdateScreen();
          y+=10;
          sprintf(SSDBuffer,"setup=%2i  servs=%2i",setup,servis); 
          SSD1306_GotoXY(0,y);
          SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
          SSD1306_UpdateScreen();
          y+=10;
          sprintf(SSDBuffer,"wtset=%2i  wtkey=%2i",waitset,waitkey); 
          SSD1306_GotoXY(0,y);
          SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
          SSD1306_UpdateScreen();
          y+=10;
          sprintf(SSDBuffer,"knm=%2i  kkd=%2i",keynum,keynum); 
          SSD1306_GotoXY(0,y);
          SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
          SSD1306_UpdateScreen();
        break;
    case 2:
        SSD1306_Fill(SSD1306_COLOR_BLACK); //clear oled
        y=0;

        sprintf(SSDBuffer,"T0=%3i  T1=%3i",*data+*(data+1)*256,*(data+2)+*(data+3)*256); 
        SSD1306_GotoXY(0,y);
        SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
        SSD1306_UpdateScreen();
        y+=10;
        sprintf(SSDBuffer,"H0=%3i  H1=%3i",*(data+4)+*(data+5)*256,*(data+6)+*(data+7)*256); 
        SSD1306_GotoXY(0,y);
        SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
        SSD1306_UpdateScreen();
        y+=10;
        sprintf(SSDBuffer,"P0=%3i  P1=%3i",*(data+8)+*(data+9)*256,*(data+10)+*(data+11)*256); 
        SSD1306_GotoXY(0,y);
        SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
        SSD1306_UpdateScreen();
        y+=10;
        sprintf(SSDBuffer,"I0=%3i  I1=%3i",*(data+12)+*(data+13)*256,*(data+14)+*(data+15)*256); 
        SSD1306_GotoXY(0,y);
        SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
        SSD1306_UpdateScreen();
        y+=10;
        sprintf(SSDBuffer,"R0=%3i  R1=%3i",*(data+16)+*(data+17)*256,*(data+18)+*(data+19)*256); 
        SSD1306_GotoXY(0,y);
        SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
        SSD1306_UpdateScreen();
        y+=10; 
        sprintf(SSDBuffer,"Pe=%3i  Ou=%3i",*(data+20)+*(data+21)*256,*(data+22)+*(data+23)*256); 
        SSD1306_GotoXY(0,y);
        SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
        SSD1306_UpdateScreen();
        break;
    case 3:
        SSD1306_Fill(SSD1306_COLOR_BLACK); //clear oled
        y = 0;
        
        for (i=13;i<19;i++){
          x = i*2;
          sprintf(SSDBuffer,"D%i=%2i  D%i=%2i",x,*(data+x),x+1, *(data+x+1)); 
          SSD1306_GotoXY(0,y);
          SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
          SSD1306_UpdateScreen();
          y+=10;
        }
        break;
    case 4:
        SSD1306_Fill(SSD1306_COLOR_BLACK); //clear oled
        y = 0;
        
        for (i=19;i<25;i++){
          x = i*2;
          sprintf(SSDBuffer,"D%i=%2i  D%i=%2i",x,*(data+x),x+1, *(data+x+1)); 
          SSD1306_GotoXY(0,y);
          SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
          SSD1306_UpdateScreen();
          y+=10;
        }
        break;
        default:
          SSD1306_Fill(SSD1306_COLOR_BLACK); //clear oled
          y=0;
          
          sprintf(SSDBuffer,"pOut=%2x  Flag=%2x",portOut.value,portFlag.value); 
          SSD1306_GotoXY(0,y);
          SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
          SSD1306_UpdateScreen();
          y+=10;
          sprintf(SSDBuffer,"Wrng=%2x  Erro=%2x",ram->warning,ram->errors); 
          SSD1306_GotoXY(0,y);
          SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
          SSD1306_UpdateScreen();
          y+=10;
          sprintf(SSDBuffer,"T0 =%3i  T1 =%3i",ram->pvT[0],ram->pvT[1]); 
          SSD1306_GotoXY(0,y);
          SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
          SSD1306_UpdateScreen();
          y+=10;
          sprintf(SSDBuffer,"Vi=%4i  Vh=%4i",currAdc, humAdc);
          SSD1306_GotoXY(0,y);
          SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
          SSD1306_UpdateScreen();
          y+=10;
          if(AM2301) sprintf(SSDBuffer,"RH=%4i  T3=%4i",ram->pvRH,ram->pvT[2]);
          else sprintf(SSDBuffer,"RH=%4i  A1=%4i",ram->pvRH,humAdc);
          SSD1306_GotoXY(0,y);
          SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
          SSD1306_UpdateScreen();
          y+=10;
          sprintf(SSDBuffer,"Fus=%3i  Modul=%i",ram->fuses,modules); 
          SSD1306_GotoXY(0,y);
          SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
          SSD1306_UpdateScreen();
  }
}
