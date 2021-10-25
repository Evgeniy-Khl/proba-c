#include "main.h"
#include "TM1638.h"
#include "ds18b20.h"
#include "displ.h"
#include "module.h"
#include "global.h"   // здесь определена структура eeprom

extern uint8_t ds18b20_amount, Hih, beepOn, modules, outbuffer[], pvTimer, pvAeration, topUser;
extern int16_t RHadc, thermistorAdc, kWattHore;
extern float PVold1, PVold2;

void init(struct eeprom *t, struct rampv *ram){
  for(int8_t i=0;i<8;i++) {setChar(i,SIMBL_BL);}  // only decimal points is on
	SSD1306_Init(); // sub display initialization
//---------- Поиск датчиков ---------------------------------------------------------------------------------------------------
  ds18b20_port_init();
  ds18b20_count(MAX_DEVICES);     // проверяем наличие датчиков если error = 0 датчики найдены
  setChar(0,ds18b20_amount); setChar(1,SIMBL_d);  // "0d"
  if(ds18b20_amount) ds18b20_Convert_T();
//  if(ds18b20_amount > 2) checkSensor();   // проверяем на подключение датчика температуры скорлупы яйца
//  if(t->HihEnable){         // если разрешено ищем HIH-5030 в V RHadc 
    RHadc = 0;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!??????????????????????????????????????????????????????????
    if(RHadc > 85){
      Hih=1; setChar(2,1);                 // если обнаружен HIH-5030
      PVold1 = PVold2 = RHadc;
    }
    else {Hih=0; setChar(2,0);}// ??????????????!!!!!!!!!!!!!!!!!!!!!!!!!???????????????????????????????
//---------- Поиск модулей расширения -----------------------------------------------------------------------------------------
	if(rtc_check()) modules=0x10;    // Real Time Clock search and availability of EEPROM
  if(module_check(ID_HALL)) {modules|=1; t->state|=0x40;} else t->state&=0xBF;  // если модуль обнаружен включаем контроль иначе контроль отключаем
  if(module_check(ID_HORIZON)) {modules|=2; t->state|=0x20;} else t->state&=0xDF; // если модуль обнаружен включаем контроль иначе контроль отключаем
  if(module_check(ID_CO2)) modules|=4;    // модуль CO2
  if(module_check(ID_FLAP)) modules|=8;   // модуль воздушных заслонок 
  setChar(3,SIMBL_n); setChar(4,modules/10); setChar(5,modules%10); // "n00"
//---------- Датчик тока ------------------------------------------------------------------------------------------------------
  if(t->KoffCurr==0) ram->warning = 0x80;   // ОТКЛЮЧЕН мониторинг тока симистора !!!
//---------- Версия программы --------------------------------------------
  displ_3(VERSION,VERS,0,0);
  SendDataTM1638();
  beepOn=DURATION;
	HAL_Delay(3000);
  if(ds18b20_amount) ds18b20_Convert_T(); else while(ds18b20_amount == 0){beepOn=DURATION; HAL_Delay(500); ds18b20_count(MAX_DEVICES);}// если датчики не обнаружены - останавливаем программу и ищем датчики

}

void init1(struct eeprom *t, struct rampv *ram){
//  error = t->identif;
//  setChar(3,SIMBL_H); setChar(4,error/10); setChar(5,error%10);

//  outbuffer[0]=DATAREAD;                  // Function Command
//  outbuffer[1]=RESET;                     // Data 1
//  outbuffer[2]=0x00;                      // Data 2

//  displ_3(modules,MODUL,0,0);
// 
//	SendDataTM1638();
//  beepOn=DURATION;
//  HAL_Delay(3000);
  //-----------------------------------------------------------------------------------------------------------------------------
  ram->pvT[0]=999;
  ram->pvT[1]=999;
  ram->pvT[2]=999;
  ram->pvT[3]=999;
  ram->date  = 1;
  ram->hours = 23;
  ram->fuses =0xFF;
  ram->cellID  = t->identif;
  ram->pvTimer = t->timer[0];
  pvAeration   = t->air[0];
  kWattHore    = t->EnergyMeter;
  if(t->relayMode==4) topUser=PULSMENU;
}
