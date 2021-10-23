#include "main.h"
#include "global.h"   // ����� ���������� ��������� eeprom � ��������� rampv
#include "keypad.h"

extern int8_t countsec, getButton, displmode;
extern uint8_t ok0, ok1, changeDispl, keyBuffer[], keynum, setup, Hih, waitset, waitkey, topOwner, topUser, botUser, modules, servis, beepOn, disableBeep, psword, Check, EEPsave;
extern int16_t buf, alarmErr;

void pushkey(void){
 uint8_t xx, keykod;
  getButton = 0;
  HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
  ReadKeyTM1638();
  keykod=0;
  for (uint8_t i=0;i<4;i++){
  xx =  keyBuffer[i]; 
  xx <<= i; 
  keykod |= xx;
  if (keyBuffer[i]==0x01) LedInverse(i);
  else if (keyBuffer[i]==0x10) LedInverse(i+4);
  }
  SendDataTM1638();  
}
/*
void checkkey(struct eeprom *t, int16_t pvT0){
  uint8_t xx, keykod;
//HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); 
  ReadKeyTM1638();  
  keykod=0;
  for (uint8_t i=0;i<4;i++){xx =  keyBuffer[i]; xx <<= i; keykod |= xx;}   // ���������� ������� ������
  SendDataTM1638();
  if ((keynum == keykod)&&keynum) {xx = 1; getButton=0;}  // ��� ���������� �������� ��������� ������� ������ (getButton=0) �������������� �������� �� ������������ ������
  else {keynum = keykod; xx= 0; waitkey=WAITCOUNT; getButton=waitkey/4;}
  if (xx){
    Check=1;
    if (setup){                // ����� �������������� ������� � ����������
        waitset=10;            // ���������� ����� ���������
        switch (keykod)
          {
           case KEY_1:
             {
              if (++setup>topOwner) setup=1;           // ���� ������������
              switch (setup)
                {
                 case 1:  buf=t->spT[0]; break;         // �1 ������� �����������
                 case 2:  if(Hih) buf=t->spRH[1]; else buf=t->spT[1]; break;// �2 ������� ���������
                 case 3:  buf=t->timer[0]; break;       // �3 ����� ������������ ���������
                 case 4:  buf=t->timer[1]; break;       // �4 ����� ����������� ��������� (���� �� 0 �� ��� �������)
                 case 5:  buf=t->alarm[0]; break;       // �5 ������� �� ������ 1
                 case 6:  buf=t->alarm[1]; break;       // �6 ������� �� ������ 2
                 case 7:  buf=t->extOn[0]; break;       // �7 �������� ��� ���. ���������������� ������ 1
                 case 8:  buf=t->extOff[0]; break;      // �8 �������� ��� ����. ���������������� ������ 1
                 case 9:  buf=t->extOn[1]; break;       // �9 �������� ��� ���. ���������������� ������ 2
                 case 10: buf=t->extOff[1]; break;      // �10 �������� ��� ����. ���������������� ������ 2
                 case 11: if(modules&4) buf=t->spCO2; else buf=t->air[0]; break;// �13 MAX CO2 / ����� ����� ��������������
                 case 12: if(modules&4) buf=t->spCO2; else buf=t->air[1]; break;// �14 MIN CO2 / ������������ ������������
                } 
             } break;
           case KEY_2:
             {
              buf++; EEPsave=1; if (waitkey) waitkey--;
              switch (setup)
               {
                case 1:  t->spT[0]=buf; break;                                                                   // �1  ������� �����������
                case 2:  if (Hih) t->spRH[1]=buf; else t->spT[1]=buf; break;                                     // �2  ������� ���������
                case 3:  if (buf<1) buf=1; t->timer[0]=buf; break;                                               // �3  ����� ������������ ���������
                case 4:  t->timer[1]=buf; break;                                                                 // �4  ����� ����������� ��������� (�������)
                case 5:  buf&=0x1F; if (buf<1) buf=1; t->alarm[0]=buf; break;                                    // �5  ������� �� ������ 1
                case 6:  buf&=0x1F; if (buf<1) buf=1; t->alarm[1]=buf; break;                                    // �6  ������� �� ������ 2
                case 7:  buf&=0x1F; if (buf<t->extOff[0]) buf=t->extOff[0]; t->extOn[0]=buf; break;              // �7  �������� ��� ���. ���������������� ������ 1
                case 8:  if(buf>t->extOn[0]) buf=t->extOn[0]; else if (buf<1) buf=1; t->extOff[0]=buf; break;    // �8  �������� ��� ����. ���������������� ������ 1
                case 9:  buf&=0x1F; if (buf<t->extOff[1]) buf=t->extOff[1]; t->extOn[1]=buf; break;              // �9  �������� ��� ���. ���������������� ������ 2
                case 10: if(buf>t->extOn[1]) buf=t->extOn[1]; else if (buf<1) buf=1; t->extOff[1]=buf; break;    // �10 �������� ��� ����. ���������������� ������ 2
                case 11: 
                         if(buf<1) buf=1; buf&=0xFF;
                         if(modules&4) t->spCO2=buf; else t->air[0]=buf;
                  break;                                                                                         // �11 MAX CO2 / ����� ����� ��������������
                case 12:
                         if(buf<0) buf=0; buf&=0xFF;
                         if(modules&4) t->spCO2=buf; else t->air[1]=buf;
                  break;                                                                                         // �12 MIN CO2 / ������������ ������������
                //------------------------------------ �00 -------------------------------------------------------------------
                case 16: switch (buf){
                          case  3: topOwner=14; break;// ���������� ������� �������� ������� ���������
                          case 10: topUser=TOPKOFF; botUser=BOTKOFF; break; // ���������� ������� �������� ������������ cof[3];
                         };
                         EEPsave=0; waitset=20;
                  break;
                //--------------------------- ���� ����������� ---------------------------------------------------------
                case 17: if(buf) t->extendMode=1; else t->extendMode=0; break;           // ����������� ����� ������  0-������; 1-��������� ����������
                case 18: if(buf>MAXRELAYMODE) buf=MAXRELAYMODE; else if(buf<MINRELAYMODE) buf=MINRELAYMODE; t->relayMode=buf;
                         if(t->relayMode==4) topUser=PULSMENU; else topUser=TOPUSER; break;//�������� ����� ������
                case 19: buf&=0x3F;  if(buf<1) buf=1; t->minRun=buf*DEN; break;       // ���������� 0.1-6.3 ������;
                case 20: buf&=0xFF;  if(buf<1) buf=1; t->maxRun=buf*DEN; break;       // ���������� 0.1-25.5 ������;
                case 21: buf&=0x3FF; if(buf<5) buf=5; t->period=buf*200; break;       // ���������� 5-999 ������ (16 ���.39 ���.);
                
                case 26: if(buf>32) buf=32; else if (buf<-64) buf=-64; t->spRH[0]=buf; break; // ���������� ������� HIH-4000
                case 27: t->Hysteresis = buf&3; break;                                // ����������
                case 28: buf&=0x03F; if(buf<1) buf=1; t->K[0]=buf; break;         // ���������� 1 - 63;
                case 29: buf&=0x3FF; if(buf<100) buf=100; t->Ti[0]=buf; break;    // ���������� 100 - 1023;
                case 30: buf&=0x03F; if(buf<1) buf=1; t->K[1]=buf; break;         // ���������� 1 - 63;
                case 31: buf&=0x3FF; if(buf<100) buf=100; t->Ti[1]=buf; break;    // ���������� 100 - 1023;
               }; 
             } break;
           case KEY_3:
             {
              ++setup;
              if (setup>topUser||setup<botUser) setup=botUser;// ���� �����������
              switch (setup)
                {
                 case 16: buf=0; break;               // �0 ����� ����������
                 case 17: buf=t->extendMode; break;   // �1 = ����������� ����� ������
                 case 18: buf=t->relayMode; break;    // �2 = ����� ���/����
                
                 case 19: buf=t->minRun/DEN; break;   // �3 = 200 - 1,0 ���.
                 case 20: buf=t->maxRun/DEN; break;   // �4 = 2000- 10,0 ���.
                 case 21: buf=t->period/200; break;   // �5 = 3000-  15 ���.
                 
                 case 26: buf=t->spRH[0]; break;      // �10 ���������� ������� HIH-4000
                 case 27: buf=t->Hysteresis; break;   // �11 ���������� ������ ���������� 
                 case 28: buf=t->K[0]; break;         // �12 = 20
                 case 29: buf=t->Ti[0]; break;        // �13 = 500
                 case 30: buf=t->K[1]; break;         // �14 = 15
                 case 31: buf=t->Ti[1]; break;        // �15 = 900
                }
             } break;
           case KEY_4:
             {
              buf--; EEPsave=1; if (waitkey) waitkey--;
              switch (setup)
               {
                case 1:  t->spT[0]=buf; break;                                                                   // �1  ������� �����������
                case 2:  if (Hih) t->spRH[1]=buf; else t->spT[1]=buf; break;                                     // �2  ������� ���������
                case 3:  if (buf<1) buf=1; t->timer[0]=buf; break;                                               // �3  ����� ������������ ���������
                case 4:  t->timer[1]=buf; break;                                                                 // �4  ����� ����������� ��������� (�������)
                case 5:  buf&=0x1F; if (buf<1) buf=1; t->alarm[0]=buf; break;                                    // �5  ������� �� ������ 1
                case 6:  buf&=0x1F; if (buf<1) buf=1; t->alarm[1]=buf; break;                                    // �6  ������� �� ������ 2
                case 7:  buf&=0x1F; if (buf<t->extOff[0]) buf=t->extOff[0]; t->extOn[0]=buf; break;              // �7  �������� ��� ���. ���������������� ������ 1
                case 8:  if(buf>t->extOn[0]) buf=t->extOn[0]; else if (buf<1) buf=1; t->extOff[0]=buf; break;    // �8  �������� ��� ����. ���������������� ������ 1
                case 9:  buf&=0x1F; if (buf<t->extOff[1]) buf=t->extOff[1]; t->extOn[1]=buf; break;              // �9  �������� ��� ���. ���������������� ������ 2
                case 10: if(buf>t->extOn[1]) buf=t->extOn[1]; else if (buf<1) buf=1; t->extOff[1]=buf; break;    // �10 �������� ��� ����. ���������������� ������ 2
                case 11: 
                         if(buf<1) buf=1; buf&=0xFF;
                         if(modules&4) t->spCO2=buf; else t->air[0]=buf;
                  break;                                                                                         // �11 MAX CO2 / ����� ����� ��������������
                case 12:
                         if(buf<0) buf=0; buf&=0xFF;
                         if(modules&4) t->spCO2=buf; else t->air[1]=buf;
                  break;                                                                                         // �12 MIN CO2 / ������������ ������������
                //------------------------------------ �00 -------------------------------------------------------------------
                case 16: switch (buf){
                          case  3: topOwner=14;            break;// ���������� ������� �������� ������� ���������
                          case 10: topUser=TOPKOFF; botUser=BOTKOFF; break; // ���������� ������� �������� ������������ cof[3];
                         };
                         EEPsave=0; waitset=20;
                  break;
                //--------------------------- ���� ����������� ---------------------------------------------------------
                case 17: if(buf) t->extendMode=1; else t->extendMode=0; break;           // ����������� ����� ������  0-������; 1-��������� ����������
                case 18: if(buf>MAXRELAYMODE) buf=MAXRELAYMODE; else if(buf<MINRELAYMODE) buf=MINRELAYMODE; t->relayMode=buf;
                         if(t->relayMode==4) topUser=PULSMENU; else topUser=TOPUSER; break;//�������� ����� ������
                case 19: buf&=0x3F;  if(buf<1) buf=1; t->minRun=buf*DEN; break;       // ���������� 0.1-6.3 ������;
                case 20: buf&=0xFF;  if(buf<1) buf=1; t->maxRun=buf*DEN; break;       // ���������� 0.1-25.5 ������;
                case 21: buf&=0x3FF; if(buf<5) buf=5; t->period=buf*200; break;       // ���������� 5-999 ������ (16 ���.39 ���.);
                
                case 26: if(buf>32) buf=32; else if (buf<-64) buf=-64; t->spRH[0]=buf; break; // ���������� ������� HIH-4000
                case 27: t->Hysteresis = buf&3; break;                                // ����������
                case 28: buf&=0x03F; if(buf<1) buf=1; t->K[0]=buf; break;         // ���������� 1 - 63;
                case 29: buf&=0x3FF; if(buf<100) buf=100; t->Ti[0]=buf; break;    // ���������� 100 - 1023;
                case 30: buf&=0x03F; if(buf<1) buf=1; t->K[1]=buf; break;         // ���������� 1 - 63;
                case 31: buf&=0x3FF; if(buf<100) buf=100; t->Ti[1]=buf; break;    // ���������� 100 - 1023;
               }; 
             } break;
           case KEY_6: setup=0;EEPsave = 0;displmode=0;psword=0;buf=0;beepOn=DURATION*3; break;
          }; 
       }
    else if (servis)          // ��������� �����  ----------------------------
       {
        waitset=15;           // ���������� ��������� �����
        switch (keykod)
          {
           case KEY_2:
            {
              buf++; EEPsave=1; waitkey=WAITCOUNT;
              switch (servis)
               {
                 case 7:  t->identif = buf&0x3F; break;      // C7 -> identif
                 case 8:  t->ForceHeat=buf&0x3F; break;      // C8 -> FORCEHEAT ������������� ������
                 case 9:  t->TurnTime= buf&0x3FF; break;     // C9 -> TURNTIME
                 case 10: t->TimeOut=(buf&0x7F)*60; break;   // C10-> TIME OUT
                 case 11: t->HihEnable= buf&0x1; break;      // C11-> ��������� ������������ HIH-4000
                 case 12: t->KoffCurr= buf&0xFF; break;      // C12-> KoffCurr ��������� ����. �� ���� ���������
                 case 13: t->coolOn =  buf&0x7F; break;      // C13-> ����� ��������� ����������� ������ ����������
                 case 14: t->coolOff = buf&0x7F; break;      // C14-> ����� ���������� ����������� ������ ����������
                 case 15: t->Zonality= buf&0x3F; break;      // C15-> ����� ����������� � ������
               }
            } break;
           case KEY_3:
            {
              ++servis; servis&=0x0F; displmode=0; waitkey=WAITCOUNT; beepOn=DURATION;
              switch (servis)
                {
                 case 7: buf=t->identif; break;           // C7 -> identif
                 case 8: buf=t->ForceHeat; break;         // C8 -> FORCEHEAT
                 case 9: buf=t->TurnTime; break;          // C9 -> TURNTIME
                 case 10: buf=t->TimeOut/60; break;       // C10-> TIME OUT
                 case 11: buf=t->HihEnable; break;        // C11-> ��������� ������������ HIH-4000
                 case 12: buf=t->KoffCurr; break;         // C12-> KoffCurr ��������� ����. �� ���� ���������
                 case 13: buf=t->coolOn; break;           // C13-> ����� ��������� ����������� ������ ����������
                 case 14: buf=t->coolOff; break;          // C14-> ����� ���������� ����������� ������ ����������
                 case 15: buf=t->Zonality; break;         // C15-> ����� ����������� � ������
                 default: buf=0;
                }
            } break;
           case KEY_4:
            {
              buf--; EEPsave=1; waitkey=WAITCOUNT;
              switch (servis)
               {
                 case 7:  t->identif = buf&0x3F; break;      // C7 -> identif
                 case 8:  t->ForceHeat=buf&0x3F; break;      // C8 -> FORCEHEAT ������������� ������
                 case 9:  t->TurnTime= buf&0x3FF; break;     // C9 -> TURNTIME
                 case 10: t->TimeOut=(buf&0x7F)*60; break;   // C10-> TIME OUT
                 case 11: t->HihEnable= buf&0x1; break;      // C11-> ��������� ������������ HIH-4000
                 case 12: t->KoffCurr= buf&0xFF; break;      // C12-> KoffCurr ��������� ����. �� ���� ���������
                 case 13: t->coolOn =  buf&0x7F; break;      // C13-> ����� ��������� ����������� ������ ����������
                 case 14: t->coolOff = buf&0x7F; break;      // C14-> ����� ���������� ����������� ������ ����������
                 case 15: t->Zonality= buf&0x3F; break;      // C15-> ����� ����������� � ������
               }
            } break;
           case KEY_6: servis=0; EEPsave = 0; displmode=0; psword=0; buf=0; topUser=TOPUSER; botUser=BOTUSER; t->state &=0xE7; beepOn=DURATION*3; break;
          }
       }
    else if(psword==10)       // ����� ������ ������ -------------------------
       {
        switch (keykod)
          {
           case KEY_1: setup=1; servis=0; displmode=0; buf=t->spT[0]; waitset=20; waitkey=WAITCOUNT; beepOn=DURATION*2; break;
           case KEY_3: if((t->state&7)==0) {servis=1; setup=0; displmode=0; waitset=20; waitkey=WAITCOUNT; beepOn=DURATION*2;} break;
           case KEY_6: psword=0; displmode=0; buf=0; t->state &=0xE7; servis=0; setup=0; waitkey=WAITCOUNT; beepOn=DURATION*3; break;// ������ ���������
           case KEY_6_5: if((t->state&7)==0){t->state|=1; t->state&=0x7F; beepOn=DURATION*2;} else {t->state&=0x60; beepOn=DURATION*3;} countsec=-5; ok0=0; ok1=0; psword=0; break;
           case KEY_7_5: if((t->state&0x1F)==0) t->state|=0x80; countsec=-5; psword=0; break;//�������� ������� ������ ��� ����������� ������ !!!
           case KEY_8_5: if(t->state&0x80) t->state&=0x7F; countsec=-5; psword=0; break;//��������� ������� ������ ��� ����������� ������ !!!
           case KEY_4_5_6_7: displmode=-10; break;
          }
       }
    else                      // ����� �� ���������
       {
        switch (keykod)
          {
           case KEY_3:   buf++; psword++; waitset=5; break;
           case KEY_4:   buf++; buf <<= 1; psword++; waitset=5; break;
           case KEY_5:   ++displmode; displmode&=3; waitset=20; break;
           case KEY_6:   psword=0; displmode=0; buf=0; t->state &=0xE7; servis=0; waitkey=WAITCOUNT; beepOn=DURATION*3; break;// ������ ���������
           case KEY_7:   disableBeep=10; alarmErr = abs(t->spT[0]-pvT0); break;          // ������� ��������� �� 10 ���.
           case KEY_8:   disableBeep=10; alarmErr = abs(t->spT[0]-pvT0); break;          // ������� ��������� �� 10 ���.
           case KEY_7_8: t->state |=0x08; beepOn=DURATION*2; break;              // �������� ������� !!
           case KEY_8_6: if((t->state&7)==1){t->state |=0x02; beepOn=DURATION*2;} break; // �������� ����� "���������� � ����������"
//           case KEY_7_6: pvTimer=1; rotate_trays(); break;                    // �������������� �������
          };
       };
//-----------------------------------------------------------------------------------------------------------------------------------------------------
     if(psword==4) {if(buf==12) {psword=10; waitset=20; beepOn=DURATION*2;} else psword=0;}   // ������ ������ -> KEY_3+KEY_4+KEY_3+KEY_4 ��������� ����� �������� ������ (20 ���.)!     
//-----------------------------------------------------------------------------------------------------------------------------------------------------
  }
}
*/
