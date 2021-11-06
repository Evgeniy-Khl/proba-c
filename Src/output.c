#include "main.h"
#include "global.h"   // ����� ���������� ��������� eeprom � ��������� rampv
#include "output.h"

extern uint8_t disableBeep;
extern int16_t pulsPeriod;
uint8_t ok0, ok1;
uint16_t valRun;
float iPart[3];

uint8_t heatCondition(int16_t err, uint8_t alarm, uint8_t extOn){
  uint8_t warn = 0;
  if(abs(err)<alarm) ok0 = 1;               // ����� �� �������� ��������
  if(ok0 && err>alarm){ok0 = 2; warn=1;}    // ������� �������������� !
  if(err < -alarm)   ok0 = 3;               // ��������, ��������� ����������
  alarm +=extOn;                            // � ������ ��������� �������� ��� ������ �����������
  if(err < -alarm) warn = 1;                // ������� �������� !
  return warn;
}

uint8_t humCondition(int16_t err, uint8_t alarm, uint8_t extOn){
  uint8_t warn = 0;
  if(abs(err)<alarm) ok1 = 1;               // ����� �� �������� ��������
  if(ok1 && err>alarm){ok1 = 2; warn=1;}    // ������ ���� !
  if(err < -alarm)   ok1 = 3;               // ������
  alarm +=extOn;                            // � ������ ��������� �������� ��� ������ ����������
  if(err < -alarm) warn = 1;                // �����o ������ !
  return warn;
}

int16_t UpdatePID(int16_t err, uint8_t cn, struct eeprom *t){
 int16_t maxVal;
 float pPart, Ud;
  if(cn) maxVal=t->maxRun; else maxVal=MAXPULS; // maxRun = 10000 = 10 ������; 1100 -> 1.1 ���.
  pPart = (float) err * t->K[cn];               // ������ ���������������� �����
//---- ������� ����������� pPart ---------------
  if (pPart < 0) pPart = 0;
  else if (pPart > maxVal) pPart = maxVal;      // ������� �����������
//----------------------------------------------
  iPart[cn] += (float) t->K[cn] / t->Ti[cn] * err;  // ���������� ������������ �����
  Ud = pPart + iPart[cn];                       // ����� ���������� �� �����������
//---- ������� ����������� Ud ------------------
  if (Ud < 0) Ud = 0;
  else if (Ud > maxVal) Ud = maxVal;            // ������� �����������
  iPart[cn] = Ud - pPart;                       // "��������������" ��������
  return Ud;
};

uint16_t heater(int16_t err, struct eeprom *t){
  uint16_t result;
  // �������� ����� ������  0-���; 1->�� ���.[0] 2->�� ���.[1] 3->�� ���.[0]&[1]
  if(t->relayMode&1){
    if(err > 0) result = MAXPULS; else result=0;
  }
  else result = UpdatePID(err, 0, t);
  return result;
}

uint16_t humidifier(int16_t err, struct eeprom *t){
  uint16_t result;
  static int8_t direction;
  // �������� ����� ������  0-���; 1->�� ���.[0] 2->�� ���.[1] 3->�� ���.[0]&[1] 4->�� ���.[1] ���������� �����
  if(t->relayMode&2){
    if(err > t->Hysteresis) {result = MAXPULS; direction = 1;}  // ���� (��������+Hysteresis) �������� �����������
    else if(err < 0) {result = 0; direction = -1;}              // ���� �������� ��������� �����������
    else if(direction>0) result = MAXPULS;                      // ���������� ���������
    else result = 0;                                            // ���������� �������
  }
  else if(t->relayMode==4){
    valRun = UpdatePID(err, 1, t);                              // ����������� ������������ ���. ���������
    if(valRun < t->minRun) valRun = t->minRun;
    else if(valRun > t->period) valRun = t->period;             // ����. ������� �� ������ ��������� ����.�������
    if(err<=0) valRun = 0;                                      // ���������� ������� �� 2 ������ ���� �������
  }
  else result = UpdatePID(err, 0, t);
  return result;
}

uint8_t RelayPos(int16_t err, uint8_t cn, uint8_t Hysteresis){
 uint8_t x=UNCHANGED;
  if(err > Hysteresis) x = ON;          // ���� (��������-offSet) ��������
  if(err < 0) x = OFF;                  // ���� �������� ���������
  return x;
}

uint8_t RelayNeg(int16_t err, uint8_t cn, int8_t extOn, int8_t extOff){
 uint8_t x=UNCHANGED;
  err += extOn;                         // err -> ������������� ��������
  if (err < 0) x = ON;                  // ���� (��������+offSet) ��������
  if (err > extOn-extOff) x = OFF;      // ���� (��������+0,2) ���������
  return x;
}

// ����������� ����� ������  0-������; 1-����. 2-���� ����. 3-���� �����. 4-���� ����. 5-������ ����������
void extra_2(struct eeprom *t, struct rampv *ram){
  uint8_t byte = UNCHANGED;
  int16_t err;
  switch (t->extendMode){
  	// ���. ����� -> ����.
    case 1: if(VENTIL) byte = ON; else byte = OFF;
  		break;
    // ���. ����� -> ������������� ������
    case 2: 
            err = t->spT[0]-ram->pvT[0];
            if(err > (t->ForceHeat+t->Hysteresis)) byte = ON;
            else if(err < t->ForceHeat) byte = OFF;
  		break;
  	// ���. ����� -> ������������� ����������
    case 3: 
            err = t->spT[0]-ram->pvT[0];
            byte = RelayNeg(err, 0, t->extOn[1],t->extOff[1]);
  		break;
    // ���. ����� -> ������������� ��������
    case 4: 
            err = t->spT[1]-ram->pvT[1];
            byte = RelayNeg(err, 1, t->extOn[1],t->extOff[1]);
      break;
  	// ���. ����� -> ������������ ������ ����������
    case 5: 
            err = t->spT[1]-ram->pvT[1];
            byte = RelayPos(err, 1, t->Hysteresis);
            if(!(ok0&1)) byte = OFF;  // ���������� ���������� ��� ��������� � ��������������
  		break;
    // ���. ����� -> ������
    default: 
            err = ram->errors + ram->warning + ram->fuses + ALARM;
            if(err && disableBeep==0) byte = ON; else byte = OFF;
  }
  if(!(ram->fuses&0x04)) byte=OFF;    // �������������� ���. ����� �2
  switch (byte){
      case ON:  EXT2 = ON;  break;
      case OFF: EXT2 = OFF; break;
  }
}

void OutPulse(int16_t err, struct eeprom *t)
{
  if(ok0==0||ok0==2){valRun = 0; return;};        // ���������� ������� �� 2 ������ ���� ���� �������� ��� ��������������
  valRun = UpdatePID(err,1,t);                    // ����������� ������������ ���. ���������
  if(valRun < t->minRun) valRun = t->minRun;
  else if(valRun > t->period) valRun = t->period; // ����. ������� �� ������ ��������� ����.�������
  if(err<=0) valRun = 0;                          // ���������� ������� �� 2 ������ ���� �������
}

