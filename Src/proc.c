#include "main.h"
#include "global.h"   // ����� ���������� ��������� eeprom � ��������� rampv
#include "proc.h"
#include "module.h"


const float A4=1.8, A5=0.81, A6=0.01;  // ������� a=0.9 (A1=2a; A2=a^2; A3=(1-a)^2)
const float A1=1.6, A2=0.64, A3=0.04;  // ������� a=0.8 (A1=2a; A2=a^2; A3=(1-a)^2)
const float VREF = 3.3f;

uint8_t pvAeration;
extern SPI_HandleTypeDef hspi2;
extern int8_t countsec;
extern uint8_t ok0, ok1, ext[], cmdmodule, modules, disableBeep;
extern int16_t alarmErr, beepOn;
float stold[2][2];

void rotate_trays(uint8_t timer0, uint8_t timer1, struct rampv *ram){ // ������������ ������
  uint8_t result;
  if(TURN){if(--ram->pvTimer==0) {ram->pvTimer=timer0; result = OFF; cmdmodule=NEW_TURN;}}
  else {if(--ram->pvTimer==0) {if (timer1) {ram->pvTimer=timer1; result = ON;} else {ram->pvTimer=timer0; result = ON;} cmdmodule=NEW_TURN;}};
  if(ram->fuses&0x08) result = OFF;      // �������������� ���� ���������
  switch (result){
    case ON:  TURN = ON; break;
    case OFF: TURN = OFF; break;
  }
}

//void CO2_check(uint16_t spCO20, uint16_t spCO21, uint16_t pvCO20){ // �������� ������������ ��2 ������������� ������ ������
// uint16_t sp;
//  sp=(int)spCO20*100;
//  if(pvCO20>sp) CARBON = ON;
//  sp=(int)spCO21*100;
//  if(pvCO20<sp) CARBON = OFF;
//  if(ext[2]==255) pvCO20 = 0;// ���� ������ ��2 ����������� ��������� �� ������ ���������� �� ����������
//}

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
     else {
      if((ram->fuses&0x08)==0) TURN = ON;      // �������������� ���� ��������� 
      counter=TurnTime;
     }                   // ���� ����� � ������ ��������� �� ��������� ������
  }
  return state;
}

uint16_t adcTomV(uint16_t curr){
  float res;
  if(curr < 125) curr = 0;                          // ������ 100 mV
  else {res = curr * VREF / 4096; curr = res*1000;} // � mV
  return curr;
}


uint8_t statF2(uint8_t n, uint16_t statPw){
 float val;
  val = A4 * stold[n][0] - A5 * stold[n][1] + A6 * statPw;
  stold[n][1] = stold[n][0];
  stold[n][0] = val;
  return val;
}

void chkdoor(struct eeprom *t, struct rampv *ram){
 static uint16_t counter;
  uint8_t doorCondition = HAL_GPIO_ReadPin(Door_GPIO_Port, Door_Pin);// �������� ������
  doorCondition = 1;  //?????????????????????????????????????????????????????????
  if(doorCondition){  // ����� �������
     ram->fuses &= 0x7F;  // ��������� ������
     if(t->condition&4){      //-- ���� "���������� � ���������" �� �������� ������ --
        t->condition &=0xFB; t->condition |=0x01; counter = 0; countsec=-5; ok0=0; ok1=0; ram->pvFlap=FLAPCLOSE; if(modules&8) chkflap(DATAREAD, &ram->pvFlap);
        if (t->extendMode==1) EXT2 = OFF; // ���. ����� (extendMode==1->����������)
      }
     else if((t->condition&3)==3) {beeper_ON(DURATION/16); if(++counter>300) {t->condition &= 0xF9; counter = 0;}}//-- ���� ��������� �������� �� ������� "���������� � ����������"
   }
  else { // ����� �������
     ram->fuses |= 0x80;  // ��������� ������
     if((t->condition&7)==3)  //-- ���� "���������� � ����������" �� ��������� ������ --
      {
       t->condition &=0xFC; t->condition |=0x04; counter = 0; ram->power=0; HUMIDI = OFF; FLAP = ON; ram->pvFlap=FLAPOPEN; if(modules&8) chkflap(SETFLAP, &ram->pvFlap);
       if (t->extendMode==1) EXT2 = ON; // ���. ����� (extendMode==1->����������)
      }
     else if((t->condition&7)==1) beeper_ON(DURATION*2);//-- ���� ������ ���. �� ���. �������.
     else if(t->condition&4) {if(++counter>t->TimeOut) {beeper_ON(DURATION); ALARM = 1;}}//-- ���� ��������� �������� �� ���. �������.(����� "���������� � ���������")
   }
}

#define SYSCLOCK 72000000U
void sysTick_Init(void){
  MODIFY_REG(SysTick->LOAD, SysTick_LOAD_RELOAD_Msk, SYSCLOCK / 1000 - 1);
  CLEAR_BIT(SysTick->VAL, SysTick_VAL_CURRENT_Msk);
  SET_BIT(SysTick->CTRL, SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk);
}

void beeper_ON(uint16_t duration){
  HAL_GPIO_WritePin(Beeper_GPIO_Port, Beeper_Pin, GPIO_PIN_SET);  // Beeper On
  beepOn = duration;
}

void set_Output(void){
  HAL_SPI_Transmit(&hspi2,(uint8_t*)&portOut, 1, 5000);
	RCK_H();
  HAL_Delay(1);
  RCK_L();
}

