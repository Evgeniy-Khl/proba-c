#include "main.h"
#include "global.h"   // здесь определена структура eeprom и структура rampv
extern char SSDBuffer[];
extern uint8_t show, ok0, ok1, changeDispl, setup, Hih, waitset, waitkey, servis, displmode, modules, disableBeep, psword, keynum, keyBuffer[];
extern uint8_t pvThermistor;
extern int16_t buf, alarmErr, buf, current, currAdc, thermistorAdc, RHadc;
extern uint32_t cnt1;

void dsplMss(uint8_t *data, struct rampv *ram){
  uint8_t x,y,i;
  switch(show){
    case 1:
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
    case 2:
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
    case 3:
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
    case 4:
          SSD1306_Fill(SSD1306_COLOR_BLACK); //clear oled
          y=0;
          
          sprintf(SSDBuffer,"state=%x  pswrd=%i",*(data+40),psword); 
          SSD1306_GotoXY(0,y);
          SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
          SSD1306_UpdateScreen();
          y+=10;
          sprintf(SSDBuffer,"setup=%i  servs=%i",setup,servis); 
          SSD1306_GotoXY(0,y);
          SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
          SSD1306_UpdateScreen();
          y+=10;
          sprintf(SSDBuffer,"wtset=%i  wtkey=%i",waitset,waitkey); 
          SSD1306_GotoXY(0,y);
          SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
          SSD1306_UpdateScreen();
          y+=10;
          sprintf(SSDBuffer,"Wning=%i  Error=%i",ram->warning,ram->errors); 
          SSD1306_GotoXY(0,y);
          SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
          SSD1306_UpdateScreen();
          y+=10;
          sprintf(SSDBuffer,"Fuses=%i  Modul=%i",ram->fuses,modules); 
          SSD1306_GotoXY(0,y);
          SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
          SSD1306_UpdateScreen();
          y+=10;
          sprintf(SSDBuffer,"knm=%i  kkd=%i",keynum,keynum); 
          SSD1306_GotoXY(0,y);
          SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
          SSD1306_UpdateScreen();
        break;
        default:
          SSD1306_Fill(SSD1306_COLOR_BLACK); //clear oled
          y=0;
          
          sprintf(SSDBuffer,"T0 =%3i  T1 =%i",ram->pvT[0],ram->pvT[1]); 
          SSD1306_GotoXY(0,y);
          SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
          SSD1306_UpdateScreen();
          y+=10;
          sprintf(SSDBuffer,"Cur=%3i  A2=%4i",current,currAdc);
          SSD1306_GotoXY(0,y);
          SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
          SSD1306_UpdateScreen();
          y+=10;
          sprintf(SSDBuffer,"Tmp=%3i  A3=%4i",pvThermistor,thermistorAdc); 
          SSD1306_GotoXY(0,y);
          SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
          SSD1306_UpdateScreen();
          y+=10;
          sprintf(SSDBuffer,"RH =%3i  A4=%4i",ram->pvRH,RHadc);  
          SSD1306_GotoXY(0,y);
          SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
          SSD1306_UpdateScreen();
          y+=10;
          sprintf(SSDBuffer,"Cnt1=%10u",cnt1); 
          SSD1306_GotoXY(0,y);
          SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
          SSD1306_UpdateScreen();
          y+=10;
          sprintf(SSDBuffer,"Cnt1=%10u",cnt1); 
          SSD1306_GotoXY(0,y);
          SSD1306_Puts(SSDBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
          SSD1306_UpdateScreen();
  }
}
