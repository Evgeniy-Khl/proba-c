#include "main.h"
#include "global.h"   // здесь определена структура eeprom

#define DEN           20     // denominator - делитель для вывода на дисплей в секундах 0,0
#define DURATION      80
#define MINRELAYMODE  0 // релейный режим работы  0-НЕТ; 1->по кан.[0] 2->по кан.[1] 3->по кан.[0]&[1] 4->ШИ по кан.[1]
#define MAXRELAYMODE  4

extern uint8_t servis, setup, beepOn, Hih, topOwner, topUser, botUser, modules;
extern int16_t buf;

//void saveservis(struct eeprom *t){
// switch (servis)
//   {
//     case 7:  t->identif = buf&0x3F; break;      // C7 -> identif
//     case 8:  t->ForceHeat=buf&0x3F; break;      // C8 -> FORCEHEAT Форсированный нагрев
//     case 9:  t->TurnTime= buf&0x3FF; break;     // C9 -> TURNTIME
//     case 10: t->TimeOut=(buf&0x7F)*60; break;   // C10-> TIME OUT
//     case 11: t->HihEnable= buf&0x1; break;      // C11-> разрешено использовать HIH-4000
//     case 12: t->KoffCurr= buf&0xFF; break;      // C12-> KoffCurr маштабный коэф. по току симистора
//     case 13: t->coolOn =  buf&0x7F; break;      // C13-> порог включения вентилятора обдува сисмистора
//     case 14: t->coolOff = buf&0x7F; break;      // C14-> порог отключения вентилятора обдува сисмистора
//     case 15: t->Zonality= buf&0x3F; break;      // C15-> порог зональности в камере
//   }
// servis=0; buf=0; beepOn=DURATION;
//}

void saveset(struct eeprom *t){
 switch (setup)
   {
    case 1:  t->spT[0]=buf; break;                                                                   // У1  Уставка температуры
    case 2:  if (Hih) t->spRH[1]=buf; else t->spT[1]=buf; break;                                     // У2  Уставка влажности
    case 3:  if (buf<1) buf=1; t->timer[0]=buf; break;                                               // У3  время отключенного состояния
    case 4:  t->timer[1]=buf; break;                                                                 // У4  время включенного состояния (секунды)
    case 5:  buf&=0x1F; if (buf<1) buf=1; t->alarm[0]=buf; break;                                    // У5  тревога по каналу 1
    case 6:  buf&=0x1F; if (buf<1) buf=1; t->alarm[1]=buf; break;                                    // У6  тревога по каналу 2
    case 7:  buf&=0x1F; if (buf<t->extOff[0]) buf=t->extOff[0]; t->extOn[0]=buf; break;              // У7  смещение для ВКЛ. вспомогательного канала 1
    case 8:  if(buf>t->extOn[0]) buf=t->extOn[0]; else if (buf<1) buf=1; t->extOff[0]=buf; break;    // У8  смещение для ОТКЛ. вспомогательного канала 1
    case 9:  buf&=0x1F; if (buf<t->extOff[1]) buf=t->extOff[1]; t->extOn[1]=buf; break;              // У9  смещение для ВКЛ. вспомогательного канала 2
    case 10: if(buf>t->extOn[1]) buf=t->extOn[1]; else if (buf<1) buf=1; t->extOff[1]=buf; break;    // У10 смещение для ОТКЛ. вспомогательного канала 2
    case 11: 
             if(buf<1) buf=1; buf&=0xFF;
             if(modules&4) t->spCO2[0]=buf; else t->air[0]=buf;
      break;                                                                                         // У11 MAX CO2 / Пауза между ПРОВЕТРИВАНИЕМ
    case 12:
             if(buf<0) buf=0; buf&=0xFF;
             if(modules&4) t->spCO2[1]=buf; else t->air[1]=buf;
      break;                                                                                         // У12 MIN CO2 / Длительность ПРОВЕТРИВАНЯ
    //------------------------------------ П00 -------------------------------------------------------------------
    case 16: switch (buf) {
              case  5: topOwner=14;            break;// разрешение вводить корекцию датчика влажности
              case 15: topUser=TOPKOFF; botUser=BOTKOFF; break; // разрешение вводить корекцию коэфициентов cof[3];
//              case 31: reset(0x0000, eep.data);          break; // сброс параметров
                          }; 
    break;//--------------------------- Меню специалиста ---------------------------------------------------------
               
    case 17: if(buf) t->extendMode=1; else t->extendMode=0; break;           // расширенный режим работы  0-СИРЕНА; 1-АВАРИЙНОЕ ОТКЛЮЧЕНИЕ
    case 18: if(buf>MAXRELAYMODE) buf=MAXRELAYMODE; else if(buf<MINRELAYMODE) buf=MINRELAYMODE; t->relayMode=buf;
             if(t->relayMode==4) topUser=PULSMENU; else topUser=TOPUSER; break;//релейный режим работы
    case 19: buf&=0x3F;  if(buf<1) buf=1; t->minRun=buf*DEN; break;       // ограничено 0.1-6.3 секунд;
    case 20: buf&=0xFF;  if(buf<1) buf=1; t->maxRun=buf*DEN; break;       // ограничено 0.1-25.5 секунд;
    case 21: buf&=0x3FF; if(buf<5) buf=5; t->period=buf*200; break;       // ограничено 5-999 секунд (16 мин.39 сек.);
    
    case 26: if(buf>32) buf=32; else if (buf<-64) buf=-64; t->spRH[0]=buf; break; // подстройка датчика HIH-4000
    case 27: t->Hysteresis = buf&3; break;                                // гистерезис
    case 28: buf&=0x03F; if(buf<1) buf=1; t->K[0]=buf; break;         // ограничено 1 - 63;
    case 29: buf&=0x3FF; if(buf<100) buf=100; t->Ti[0]=buf; break;    // ограничено 100 - 1023;
    case 30: buf&=0x03F; if(buf<1) buf=1; t->K[1]=buf; break;         // ограничено 1 - 63;
    case 31: buf&=0x3FF; if(buf<100) buf=100; t->Ti[1]=buf; break;    // ограничено 100 - 1023;
   }; 
 setup=0; buf=0; beepOn=DURATION;
}
