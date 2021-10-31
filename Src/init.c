#include "main.h"
#include "TM1638.h"
#include "ds18b20.h"
#include "displ.h"
#include "module.h"
#include "global.h"   // ����� ���������� ��������� eeprom
#include "am2301.h"

extern uint8_t ds18b20_amount, beepOn, modules, pvTimer, pvAeration, topUser;
extern int16_t humAdc;
extern float PVold1, PVold2;

void init(struct eeprom *t, struct rampv *ram){
 int8_t i;
  for(i=0;i<8;i++) {setChar(i,SIMBL_BL); PointOn(i);}  // only decimal points is on
  SendDataTM1638();
	SSD1306_Init(); // sub display initialization
//---------- ����� �������� ---------------------------------------------------------------------------------------------------
  ds18b20_port_init();
  ds18b20_count(MAX_DEVICES);     // ��������� ������� �������� ���� error = 0 ������� �������
  setChar(0,ds18b20_amount); setChar(1,SIMBL_d); setChar(2,0);  // "2d0"
  if(ds18b20_amount) ds18b20_Convert_T();
//  if(ds18b20_amount > 2) checkSensor(); // ��������� �� ����������� ������� ����������� �������� ����
  if(t->HihEnable){                       // ���� ��������� ���� HIH-5030 � V humAdc 
    if(humAdc > 500){                     // humAdc => 500 mV ��� RH=0%
      HIH5030 = 1; setChar(2,1);          // ���� ��������� HIH-5030 "2d1"
      PVold1 = PVold2 = humAdc;
    }
  }
  else {
//    AM2301 = am2301_Read(ram, t->spRH[0]);
    HAL_Delay(1000);
    am2301_port_init();
    AM2301 = am2301_Start();
    if(AM2301) setChar(2,1);              // ���� ��������� AM2301 "2d1"
  }
  i = t->identif;
  setChar(3,SIMBL_n); setChar(4,i/10); setChar(5,i%10); // "n01"
//---------- ������ ��������� --------------------------------------------
  displ_3(VERSION,VERS,0,0);
  SendDataTM1638();
  beepOn=DURATION;
	HAL_Delay(3000);
//---------- ����� ������� ���������� -----------------------------------------------------------------------------------------
	modules = 0;
//  if(sd_check()) modules|=0x20;    // SD search
  if(rtc_check()) modules|=0x10;    // Real Time Clock search and availability of EEPROM
  if(module_check(ID_HALL)) {modules|=1; t->state|=0x40;} else t->state&=0xBF;  // ���� ������ ��������� �������� �������� ����� �������� ���������
  if(module_check(ID_HORIZON)) {modules|=2; t->state|=0x20;} else t->state&=0xDF; // ���� ������ ��������� �������� �������� ����� �������� ���������
  if(module_check(ID_CO2)) modules|=4;    // ������ CO2
  if(module_check(ID_FLAP)) modules|=8;   // ������ ��������� �������� 
  setChar(0,SIMBL_u); setChar(1,modules/10); setChar(2,modules%10); // "u00"
//---------- ������ ���� ------------------------------------------------------------------------------------------------------
  if(t->KoffCurr==0) ram->warning = 0x80;   // �������� ���������� ���� ��������� !!!
  i = ram->warning;
  setChar(3,SIMBL_o); setChar(4,i/10); setChar(5,i%10); // "o00"
//---------- ������ ����� --------------------------------------------
  displ_3(0,0,0,0);
  SendDataTM1638();
  beepOn=DURATION;
	HAL_Delay(3000);
  if(ds18b20_amount) ds18b20_Convert_T(); else while(ds18b20_amount == 0){beepOn=DURATION; HAL_Delay(500); ds18b20_count(MAX_DEVICES);}// ���� ������� �� ���������� - ������������� ��������� � ���� �������
  ram->cellID  = t->identif;
  ram->pvTimer = t->timer[0];
  pvAeration   = t->air[0];
//**********????????????******************
  ram->pvT[0]=999;
  ram->pvT[1]=999;
  ram->pvT[2]=999;
  ram->pvT[3]=999;
  ram->pvRH = 999;
  ram->date  = 1;
  ram->hours = 23;
  ram->fuses =0xFF;
}
