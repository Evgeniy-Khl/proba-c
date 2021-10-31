#include "main.h"
#include "global.h"   // ����� ���������� ��������� eeprom � ��������� rampv
#include "proc.h"
#include "module.h"


const float A4=1.8, A5=0.81, A6=0.01;  // ������� a=0.9 (A1=2a; A2=a^2; A3=(1-a)^2)
const float A1=1.6, A2=0.64, A3=0.04;  // ������� a=0.8 (A1=2a; A2=a^2; A3=(1-a)^2)
uint8_t pvAeration;
extern int8_t countsec;
extern uint8_t ok0, ok1, ext[], cmdmodule, beepOn, modules, disableBeep;
extern int16_t alarmErr;
float stold[2][2];

void rotate_trays(uint8_t timer0, uint8_t timer1, struct rampv *ram){ // ������������ ������
  if(TURN){if(--ram->pvTimer==0) {ram->pvTimer=timer0; TURN = OFF; cmdmodule=NEW_TURN;}}
  else {if(--ram->pvTimer==0) {if (timer1) {ram->pvTimer=timer1; TURN = ON;} else {ram->pvTimer=timer0; TURN = ON;} cmdmodule=NEW_TURN;}};
}

void CO2_check(uint16_t spCO20, uint16_t spCO21, uint16_t pvCO20){ // �������� ������������ ��2 ������������� ������ ������
 uint16_t sp;
  sp=(int)spCO20*100;
  if(pvCO20>sp) CARBON = ON;
  sp=(int)spCO21*100;
  if(pvCO20<sp) CARBON = OFF;
  if(ext[2]==255) pvCO20 = 0;// ���� ������ ��2 ����������� ��������� �� ������ ���������� �� ����������
}

void aeration_check(uint8_t air0, uint8_t air1){ // �������������
  if(VENTIL){if(--pvAeration==0) {pvAeration=air0; VENTIL = OFF;}}
  else {if(--pvAeration==0) {pvAeration=air1; VENTIL = ON;}}
}

uint8_t sethorizon(uint8_t timer0, uint8_t TurnTime, struct rampv *ram){ // ��������� � ��������
 uint8_t state=0;
 static uint8_t counter=0;
  if(counter) --counter;                                   // �������� ������� ������ TIMETURN
  if(counter==0){
     if (TURN){                                            // ���� ����� � ������� ��������� �� ����� � ��������
        ram->pvTimer=timer0; state = 0x10; TURN = OFF;     // �������� ����������
        cmdmodule=SETHORIZON;                              // ������� ��������������� ������
     }
     else {TURN = ON; counter=TurnTime;}                   // ���� ����� � ������ ��������� �� ��������� ������
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
        if (t->extendMode==1) EXT2 = OFF; // ���. ����� (extendMode==1->����������)
      }
     else if((t->state&3)==3) {beepOn = DURATION/16; if(++counter>300) {t->state &= 0xF9; counter = 0;}}//-- ���� ��������� �������� �� ������� "���������� � ����������"
   }
  else { // ����� �������
     ram->fuses &= 0x7F;  // ��������� ������
     if((t->state&7)==3)//-- ���� "���������� � ����������" �� ��������� ������ --
      {
       t->state &=0xFC; t->state |=0x04; counter = 0; ram->power=0; HUMIDI = OFF; FLAP = ON; ram->pvFlap=FLAPOPEN; if(modules&8) chkflap(SETFLAP, &ram->pvFlap);
       if (t->extendMode==1) EXT2 = ON; // ���. ����� (extendMode==1->����������)
      }
     else if((t->state&7)==1) beepOn = DURATION*2;//-- ���� ������ ���. �� ���. �������.
     else if(t->state&4) {if(++counter>t->TimeOut) {beepOn = DURATION; ram->warning |=0x20;}}//-- ���� ��������� �������� �� ���. �������.(����� "���������� � ���������")
   }
}

void alarm(struct eeprom *t, struct rampv *ram){
 uint8_t al;
 int16_t error;
   al=~ram->fuses; al>>=2; al&=0x1F;
   if(ram->errors+(ram->warning & 0x7F)+al) ALARM = 1;
   al = OFF;
   error = abs(t->spT[0]-ram->pvT[0]);
   if((ram->warning & 3)&&(error-alarmErr)>2) disableBeep=0;  // ���� ��� ��������� ������ ���������� ������������ ������ ����� ����������
   if(ALARM){
     if(disableBeep==0) {beepOn = DURATION*2; al = ON;};      // ������������ ��������� ������� � �������� ����� 4
   }
   else disableBeep = 0;
   if(!(ram->fuses&0x20) && t->extendMode==0) EXT2 = OFF;     // �������������� ���. ����� �2
}
