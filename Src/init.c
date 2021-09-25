#include "main.h"
#include "TM1638.h"
#include "ds18b20.h"
#include "displ.h"
#include "module.h"
#include "global.h"   // ����� ���������� ��������� eeprom

extern uint8_t ds18b20_amount, Hih, Thermistor, beepOn, modules, outbuffer[], pvTimer, pvAeration, topUser;
extern int16_t RHadc, thermistorAdc, kWattHore;
extern float PVold1, PVold2;

void init0(uint8_t KoffCurr){
 uint8_t error;
  for(int8_t i=0;i<8;i++) {setChar(i,SIMBL_BL);}// BL
	error = SSD1306_Init();
	error = rtc_check();
	error = 0;
//--- ������������� ----------------
  if(thermistorAdc>10 /*&& thermistorAdc<125*/) Thermistor=1;   // ������������� ��������
  else error = 1;
//--- �-� ��������� ������ ---------
//  if(pvVCool>10) FanFeedback=1;           
//  else error |= 2;
//--- ������ ���� ------------------
  if(KoffCurr==0) error |= 4;   // �������� ���������� ���� ��������� !!!
//---------- ��������� ������ --------------------------------------------
  setChar(0,SIMBL_E); setChar(1,error/10); setChar(2,error%10);
//---------- ������ ��������� --------------------------------------------
  displ_3(VERSION,VERS,0,0);
  SendDataTM1638();
  beepOn=DURATION;
	HAL_Delay(3000);
}

void init1(struct eeprom *t, struct rampv *ram){
 uint8_t error=0;
  ds18b20_port_init();
  error = ds18b20_count(MAX_DEVICES);     // ��������� ������� �������� ���� error = 0 ������� �������
  setChar(0,ds18b20_amount); setChar(1,SIMBL_d);
//  if(ds18b20_amount > 2) checkSensor();   // ��������� �� ����������� ������� ����������� �������� ����
//  if(t->HihEnable && RHadc > 85){         // ���� ��������� ���� HIH-5030 � V RHadc 
    RHadc = 0;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!??????????????????????????????????????????????????????????
    if(RHadc > 85){
      Hih=1; setChar(2,1);                 // ���� ��������� HIH-5030
      PVold1 = PVold2 = RHadc;
    }
    else {Hih=0; setChar(2,0);}// ??????????????!!!!!!!!!!!!!!!!!!!!!!!!!???????????????????????????????
  ds18b20_Convert_T();
//  if(module_check(ID_CO2)) modules=4;     // ���� ������ ���������
//  HAL_Delay(1500);
//  if(module_check(ID_FLAP)) modules|=8;   // ���� ������ ���������
  error = t->identif;
  setChar(3,SIMBL_H); setChar(4,error/10); setChar(5,error%10);
//---------- ����� ������� ���������� -----------------------------------------------------------------------------------------
  outbuffer[0]=DATAREAD;                  // Function Command
  outbuffer[1]=RESET;                     // Data 1
  outbuffer[2]=0x00;                      // Data 2
//  if(module_check(ID_HALL)) {modules|=1; t->state|=0x40;}   // ���� ������ ��������� �������� ��������
//  else t->state&=0xBF;            // ����� �������� ���������
//  if(module_check(ID_HORIZON)) {modules|=2; t->state|=0x20;}// ���� ������ ��������� �������� ��������
//  else t->state&=0xDF;            // ����� �������� ���������
//  modules|=1; t->state|=0x40;
  displ_3(modules,MODUL,0,0);
 
	SendDataTM1638();
  beepOn=DURATION;
  HAL_Delay(3000);
  //-----------------------------------------------------------------------------------------------------------------------------
  ram->pvT[0]=999;
  ram->pvT[1]=999;
  ram->pvT[2]=999;
  ram->pvT[3]=999;
  ram->date  = 31;
  ram->hours = 24;
  ram->fuses =0xFF;
  ram->cellID  = t->identif;
  ram->pvTimer = t->timer[0];
  pvAeration   = t->air[0];
  kWattHore    = t->EnergyMeter;
  if(t->relayMode==4) topUser=PULSMENU;
  
}
