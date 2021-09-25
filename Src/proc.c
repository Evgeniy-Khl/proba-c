#include "main.h"
#include "global.h"   // ����� ���������� ��������� eeprom � ��������� rampv
#include "proc.h"
#include "module.h"


extern const float A4, A5, A6;
extern int8_t countsec;
extern uint8_t portOut, ok0, ok1, devices, familycode[], outbuffer[], ext[], Hih, cmdmodule, pvVCool;
extern uint8_t Carbon, Aeration, pvAeration, beepOn, modules, Superheat, Alarm, disableBeep;
extern int16_t alarmErr;
extern float stold[2][2];

void rotate_trays(uint8_t timer0, uint8_t timer1, struct rampv *ram){ // ������������ ������
  if(TURN_IN(portOut)){if(--ram->pvTimer==0) {ram->pvTimer=timer0; TURN_OFF(portOut); cmdmodule=NEW_TURN;}}
  else {if(--ram->pvTimer==0) {if (timer1) {ram->pvTimer=timer1; TURN_ON(portOut);} else {ram->pvTimer=timer0; TURN_ON(portOut);} cmdmodule=NEW_TURN;}};
}

void CO2_check(uint16_t spCO20, uint16_t spCO21, uint16_t pvCO20){ // �������� ������������ ��2 ������������� ������ ������
 uint16_t sp;
  sp=(int)spCO20*100;
  if(pvCO20>sp) Carbon = ON;
  sp=(int)spCO21*100;
  if(pvCO20<sp) Carbon = OFF;
  if(ext[2]==255) pvCO20 = 0;// ���� ������ ��2 ����������� ��������� �� ������ ���������� �� ����������
}

void aeration_check(uint8_t air0, uint8_t air1){ // �������������
  if(Aeration){if(--pvAeration==0) {pvAeration=air0; Aeration = OFF;}}
  else {if(--pvAeration==0) {pvAeration=air1; Aeration = ON;}}
}

uint8_t sethorizon(uint8_t timer0, uint8_t TurnTime, struct rampv *ram){ // ��������� � ��������
 uint8_t state=0;
 static uint8_t counter=0;
  if(counter) --counter;                                   // �������� ������� ������ TIMETURN
  if(counter==0){
     if (TURN_IN(portOut)){                                             // ���� ����� � ������� ��������� �� ����� � ��������
        ram->pvTimer=timer0; state = 0x10; TURN_OFF(portOut);    // �������� ����������
        cmdmodule=SETHORIZON;                              // ������� ��������������� ������
     }
     else {TURN_ON(portOut); counter=TurnTime;}                   // ���� ����� � ������ ��������� �� ��������� ������
  }
  return state;
}

uint16_t ratioCurr(uint16_t curr, uint8_t KoffCurr){
  if(curr<14) curr=0;                                      // ���� ���� ���� < 1,4� �� ��� ����� 0�; 90mV ��� ������� ����  (10mV==1)
  curr*=KoffCurr; curr/=100;                               // ��� - 10� = 1� = 102 ADC
  return curr;
}

uint16_t powerCurr(uint16_t curr, uint8_t KoffCurr, struct rampv *ram){
  if(ram->power<100) {curr*=ram->power; curr/=100;}
  else if(curr<10&&countsec>=0&&KoffCurr) ram->errors|=0x08;    // ���� ��� < 1,0 �. -> ���������� ���� �����������
  return curr;
}
uint8_t statF2(uint8_t n, uint16_t statPw){
 float val;
  val = A4 * stold[n][0] - A5 * stold[n][1] + A6 * statPw;
  stold[n][1] = stold[n][0];
  stold[n][0] = val;
  return val;
};

#define DOOR	1//	PINB.0   // �������� ������
void chkdoor(struct eeprom *t, struct rampv *ram){
 static uint16_t counter;
  if (DOOR){  // ����� �������
     ram->fuses |= 0x80;  // ��������� ������
     if(t->state&4){     //-- ���� "���������� � ���������" �� �������� ������ --
        t->state &=0xFB; t->state |=0x01; counter = 0; countsec=-5; ok0=0; ok1=0; ram->pvFlap=FLAPCLOSE; if(modules&8) chkflap(DATAREAD, &ram->pvFlap);
        if (t->extendMode==1) EXT2_OFF(portOut);// ���. ����� -> ���������� ����������
      }
     else if((t->state&3)==3) {beepOn = DURATION/16; if(++counter>300) {t->state &= 0xF9; counter = 0;}}//-- ���� ��������� �������� �� ������� "���������� � ����������"
   }
  else { // ����� �������
     ram->fuses &= 0x7F;  // ��������� ������
     if((t->state&7)==3)//-- ���� "���������� � ����������" �� ��������� ������ --
      {
       t->state &=0xFC; t->state |=0x04; counter = 0; ram->power=0; CANAL2_OFF(portOut); EXT1_ON(portOut); ram->pvFlap=FLAPOPEN; if(modules&8) chkflap(SETFLAP, &ram->pvFlap);
       if (t->extendMode==1) EXT2_ON(portOut);// ���. ����� -> ���������� ����������
      }
     else if((t->state&7)==1) beepOn = DURATION*2;//-- ���� ������ ���. �� ���. �������.
     else if(t->state&4) {if(++counter>t->TimeOut) {beepOn = DURATION; ram->warning |=0x20;}}//-- ���� ��������� �������� �� ���. �������.(����� "���������� � ���������")
   }
}

/*************************************************************
T=20,0���.�
I=3.9A   V=102
I=5,9A   V=110 ��� V=115 �������� �����������.
I=9,7A   V>120 ��� V=117 �������� ��� �� �����������.
*************************************************************/

uint8_t fan_adc(uint16_t val, uint8_t coolOn, uint8_t coolOff){
  val >>=2;
  if (Superheat){if (val < 108) {COOLER_ON(portOut); Superheat=0;}}
  else if (val > 110) {COOLER_ON(portOut); Superheat=1;}          // val > 100->80 ���.�.
  else if (val > coolOn) COOLER_ON(portOut);
  else if (val < coolOff) COOLER_OFF(portOut);
  if (!DOOR) {COOLER_ON(portOut); pvVCool=100;}                  // ����� �������  ������������� ������������ �� "��� ������ ���������!"
  return val;
}

void fan_power(uint8_t coolOn, uint8_t coolOff, uint8_t power){
  if (power > coolOn) COOLER_ON(portOut);
  else if (power < coolOff) COOLER_OFF(portOut);
  if (!DOOR) {COOLER_ON(portOut); pvVCool=100;}                  // ����� �������  ������������� ������������ �� "��� ������ ���������!"
}

void alarm(struct eeprom *t, struct rampv *ram){
 uint8_t al;
 int16_t error;
   al=~ram->fuses; al>>=2; al&=0x1F;
   if(ram->errors+(ram->warning & 0x7F)+al) Alarm=1;
   al = OFF;
   error = abs(t->spT[0]-ram->pvT[0]);
   if((ram->warning & 3)&&(error-alarmErr)>2) disableBeep=0;         // ���� ��� ��������� ������ ���������� ������������ ������ ����� ����������
   if(Alarm){
     if(disableBeep==0) {beepOn = DURATION*2; al = ON;};        // ������������ ��������� ������� � �������� ����� 4
   }
   else disableBeep = 0;
   if(!(ram->fuses&0x20) && t->extendMode==0) EXT2_OFF(portOut);     // �������������� ���. ����� �2
}
