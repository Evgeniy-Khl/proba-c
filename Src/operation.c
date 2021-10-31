#include "main.h"
#include "module.h"
#include "global.h"   // ����� ���������� ��������� eeprom � ��������� rampv

uint8_t ok0, ok1;
extern uint8_t pwTriac0, pwTriac1, current, modules, valRun;
extern uint16_t statPw[];
float iPart[3];

int16_t UpdatePID(int16_t err, uint8_t cn, struct eeprom *t){
 int16_t maxVal;
 float pPart, Ud;
  if(cn && (t->relayMode==4)) maxVal=t->maxRun; else maxVal=200;    // maxRun = 2000 = 10 ������
  pPart = (float) err * t->K[cn];             // ������ ���������������� �����
//---- ������� ����������� pPart ---------------
  if (pPart < 0) pPart = 0;
  else if (pPart > maxVal) pPart = maxVal;     // ������� �����������
//----------------------------------------------
  iPart[cn] += (float) t->K[cn] / t->Ti[cn] * err;   // ���������� ������������ �����
  Ud = pPart + iPart[cn];                      // ����� ���������� �� �����������
//---- ������� ����������� Ud ------------------
  if (Ud < 0) Ud = 0;
  else if (Ud > maxVal) Ud = maxVal;           // ������� �����������
  iPart[cn] = Ud - pPart;                      // "��������������" ��������
  return Ud;
};

int16_t checkPV(uint8_t cn, struct eeprom *t, struct rampv *ram){
 int16_t err;
 signed char d;
  d = t->alarm[cn];
  if(cn && HIH5030){       // ����� ��������� HIH-4000
     if(ram->pvRH > 100) {ram->errors |= (cn+1); err = 0x7FF;}
     else err = t->spRH[1] - ram->pvRH;
     if(abs(err)<d)  ok1 = 1;               // ����� �� �������� ��������
     if(ok0==0) ok1 = 0;                    // ����� ������ ���������
     if(ok1){
      if(err>d)  {ok1=2; ram->warning |= 2;}     // ����
      if(err<-d) {ok1=3; ram->warning |= 2;}     // ������
     }
  }
  else if(cn){       // ����� ���������� ����������
     if (ram->pvT[cn] >= 850) {ram->errors |= (cn+1); err = 0x7FF;}
     else err = t->spT[cn] - ram->pvT[cn];
     if(abs(err)<d)  ok1 = 1;               // ����� �� �������� ��������
     if(ok0==0) ok1 = 0;                    // ����� ������ ���������
     if(ok1){
      if(err>d)  {ok1=2; ram->warning |= 2;}     // ����
      if(err<-d) {ok1=3; ram->warning |= 2;}     // ������
     }
  }
  else {             // ����� �����������  ����������
     if (ram->pvT[cn] >= 850) {ram->errors |= (cn+1); err = 0x7FF;}
     else err = t->spT [cn]- ram->pvT[cn];
     if(abs(err)<d) ok0 = 1;                // ����� �� �������� ��������
     if(err<-d) ok0=3;                      // ��������, ��������� ����������
     if(ok0 && err>d){ok0=2; ram->warning |= 1;} // ��������������
     d +=t->extOn[0];                       // � ������ ��������� �������� ��� ������ �����������
     if(err<-d) ram->warning |= 1;               // ���. ������
  }
  return err;
}

uint8_t OutPW(uint8_t cn, struct eeprom *t, struct rampv *ram){
 signed char d;
 int16_t err;
  err = checkPV(cn, t, ram);
  if(err==0x7FF) return OFF;
  if(t->relayMode & (cn+1)){                 // ���� ���������� �������� ����� ��� [0]-������ -> 1  ���  ��� [1]-������ -> 2
     switch (cn){
       case 0: d = HEATER; break;  // ����� 1 (�������� �����������)
       case 1: d = HUMIDI; break;  // ����� 2 (�������� ����������)
     } 
     if(err > t->Hysteresis) err=255;       // ���� (��������-offSet) ��������
     else if(err < 0) err=0;                // ���� �������� ���������
     else if(d) err=255;                    // �������� ������� ������
     else err=0;                            // �������� ������� ������
  }
  else err = UpdatePID(err,cn, t);          // ����������� �������� ������ 1 (10 �)
  if(err>255) err=255;
  return err;
}

void OutPulse(uint8_t cn, struct eeprom *t, struct rampv *ram){
 int16_t err;
  err = checkPV(cn, t, ram);
  if(err == 0x7FF){valRun = 0; return;};
  if(ok0==0||ok0==2){valRun = 0; return;};        // ���������� ������� �� 2 ������ ���� ���� ��������
  valRun = UpdatePID(err,1, t);                   // ����������� ������������ ���. ���������
  if(valRun < t->minRun) valRun = t->minRun;
  else if(valRun > t->period) valRun = t->period; // ����. ������� �� ������ ��������� ����.�������
  if(err<=0) valRun = 0;                          // ���������� ������� �� 2 ������ ���� �������
}

uint8_t RelayPos(uint8_t cn, uint8_t offSet, struct eeprom *t, struct rampv *ram){	// [n] ����� � 1 ��� 2
 signed char d;
 int16_t err;
  err = checkPV(cn, t, ram);
  if (err == 0x7FF) return OFF;
  d = UNCHANGED;
  if(err > offSet) d = ON;              // ���� (��������-offSet) ��������
  if(err < 0) d = OFF;                  // ���� �������� ���������
  return d;
}

uint8_t RelayNeg(uint8_t cn, uint8_t offSet, uint8_t bias, struct eeprom *t, struct rampv *ram){	// [n] ����� � 1 ��� 2
 uint8_t x=UNCHANGED;
 int16_t err;
  err = checkPV(cn, t, ram);
  if (err == 0x7FF) return OFF;
  err += offSet;                         // err -> ������������� ��������
  if (err < 0) x = ON;                   // ���� (��������+offSet) ��������
  if (err > offSet-bias) x = OFF;        // ���� (��������+0,2) ���������
  return x;
}

void heat_wet(struct eeprom *t, struct rampv *ram){
    pwTriac0 = OutPW(0, t, ram);
    if((ram->fuses&5)<5) pwTriac0 = OFF;             // �������� ��������� ��� ������� ����������� �����������
    if(OVRHEAT) {pwTriac0=OFF; ram->errors|=0x80;}  // �������� ��������� !!!
    ram->power = pwTriac0/2;
    statPw[0]+=ram->power;               // ������ ����������
    if(pwTriac0) HEATER = ON;            // �������� ����� 1 (�������� �����������)

    if(t->relayMode == 4) OutPulse(1, t, ram);       // ���������� ���������� ������������
    else pwTriac1 = OutPW(1, t, ram);        
    if(!(ok0&1)) pwTriac1=OFF;           // ���������� ���������� ��� ��������� � ��������������
    if(!(ram->fuses&8)) pwTriac1=OFF;    // �������������� �����������
    if(pwTriac1) HUMIDI = ON;            // �������� ����� 2 (�������� ����������)
    statPw[1]+=(pwTriac1/2);             // ������ ����������
}

void extra_1(struct eeprom *t, struct rampv *ram){
    uint8_t byte;
    byte = RelayNeg(0,t->extOn[0],t->extOff[0], t, ram);// ���. ����� -> ����������
    if(VENTIL || CARBON){
      current = checkPV(0, t, ram);                  // ��������� ���������� �����������
      if(current<(t->alarm[0])) byte = ON;      // ���� � �������� �������� ������� ��� �������������
      else if(current>(t->alarm[0]*2)) byte = OFF; // ����� �������
    }
    if(!(ram->fuses&1)) byte = ON;                   // ������� ����������� �����������
    if(!(ram->fuses&0x10)) byte=OFF;                 // �������������� ���. ����� �1
    switch (byte){
      case ON:  FLAP = ON; ram->pvFlap = FLAPOPEN; if(modules&8) chkflap(SETFLAP, &ram->pvFlap); break;// -- ��������� �������� --
      case OFF: FLAP = OFF; ram->pvFlap=FLAPCLOSE; if(modules&8) chkflap(DATAREAD, &ram->pvFlap); break;// ��������� ��������; ����� ����� ���������� �������������� ������ ����
    }
  
}

void extra_2(struct eeprom *t, struct rampv *ram){
  uint8_t byte = UNCHANGED;
  if(t->extendMode==2){if((t->spT[0]-ram->pvT[0])>(t->ForceHeat+t->Hysteresis)) byte = ON; 
  else if((t->spT[0]-ram->pvT[0])<t->ForceHeat) byte = OFF;} // ���. ����� -> ������������� ������
  else if(t->extendMode==3) byte = RelayNeg(0,t->extOn[1],t->extOff[1], t, ram);// ���. ����� -> ������������� ����������
  else if(t->extendMode==4) byte = RelayNeg(1,t->extOn[1],t->extOff[1], t, ram);// ���. ����� -> ������������� ��������
  else if(t->extendMode==5) byte = RelayPos(1,t->Hysteresis, t, ram);              // ���. ����� -> ������������ ������ ����������
  if(!(ok0&1) && t->extendMode==5) byte = OFF;  // ���������� ���������� ��� ��������� � ��������������
  if(!(ram->fuses&0x20)) byte=OFF;                // �������������� ���. ����� �2
  if(t->extendMode>1){
    switch (byte){
        case ON:  EXT2 = ON;  break;
        case OFF: EXT2 = OFF; break;
    }
  }
}

