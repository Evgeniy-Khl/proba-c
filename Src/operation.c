#include "main.h"
#include "module.h"
#include "global.h"   // здесь определена структура eeprom и структура rampv

uint8_t ok0, ok1;
extern uint8_t pwTriac0, pwTriac1, current, modules, valRun;
extern uint16_t statPw[];
float iPart[3];

int16_t UpdatePID(int16_t err, uint8_t cn, struct eeprom *t){
 int16_t maxVal;
 float pPart, Ud;
  if(cn && (t->relayMode==4)) maxVal=t->maxRun; else maxVal=200;    // maxRun = 2000 = 10 секунд
  pPart = (float) err * t->K[cn];             // расчет пропорциональной части
//---- функция ограничения pPart ---------------
  if (pPart < 0) pPart = 0;
  else if (pPart > maxVal) pPart = maxVal;     // функция ограничения
//----------------------------------------------
  iPart[cn] += (float) t->K[cn] / t->Ti[cn] * err;   // приращение интегральной части
  Ud = pPart + iPart[cn];                      // выход регулятора до ограничения
//---- функция ограничения Ud ------------------
  if (Ud < 0) Ud = 0;
  else if (Ud > maxVal) Ud = maxVal;           // функция ограничения
  iPart[cn] = Ud - pPart;                      // "антинасыщяющая" поправка
  return Ud;
};

int16_t checkPV(uint8_t cn, struct eeprom *t, struct rampv *ram){
 int16_t err;
 signed char d;
  d = t->alarm[cn];
  if(cn && HIH5030){       // канал ВЛАЖНОСТИ HIH-4000
     if(ram->pvRH > 100) {ram->errors |= (cn+1); err = 0x7FF;}
     else err = t->spRH[1] - ram->pvRH;
     if(abs(err)<d)  ok1 = 1;               // вышли на заданное значение
     if(ok0==0) ok1 = 0;                    // сброс канала влажности
     if(ok1){
      if(err>d)  {ok1=2; ram->warning |= 2;}     // СУХО
      if(err<-d) {ok1=3; ram->warning |= 2;}     // ВЛАЖНО
     }
  }
  else if(cn){       // канал ВЛАЖНОСИТИ психрометр
     if (ram->pvT[cn] >= 850) {ram->errors |= (cn+1); err = 0x7FF;}
     else err = t->spT[cn] - ram->pvT[cn];
     if(abs(err)<d)  ok1 = 1;               // вышли на заданное значение
     if(ok0==0) ok1 = 0;                    // сброс канала влажности
     if(ok1){
      if(err>d)  {ok1=2; ram->warning |= 2;}     // СУХО
      if(err<-d) {ok1=3; ram->warning |= 2;}     // ВЛАЖНО
     }
  }
  else {             // канал ТЕМПЕРАТУРЫ  психрометр
     if (ram->pvT[cn] >= 850) {ram->errors |= (cn+1); err = 0x7FF;}
     else err = t->spT [cn]- ram->pvT[cn];
     if(abs(err)<d) ok0 = 1;                // вышли на заданное значение
     if(err<-d) ok0=3;                      // ПЕРЕГРЕВ, разрешаем увлажнение
     if(ok0 && err>d){ok0=2; ram->warning |= 1;} // ПЕРЕОХЛАЖДЕНИЕ
     d +=t->extOn[0];                       // с учетом воздушной заслонки для канала температуры
     if(err<-d) ram->warning |= 1;               // ВКЛ. аварии
  }
  return err;
}

uint8_t OutPW(uint8_t cn, struct eeprom *t, struct rampv *ram){
 signed char d;
 int16_t err;
  err = checkPV(cn, t, ram);
  if(err==0x7FF) return OFF;
  if(t->relayMode & (cn+1)){                 // если установлен релейный режим для [0]-канала -> 1  или  для [1]-канала -> 2
     switch (cn){
       case 0: d = HEATER; break;  // канал 1 (симистор НАГРЕВАТЕЛЯ)
       case 1: d = HUMIDI; break;  // канал 2 (симистор УВЛАЖНЕНИЯ)
     } 
     if(err > t->Hysteresis) err=255;       // ниже (заданной-offSet) включить
     else if(err < 0) err=0;                // выше заданной отключить
     else if(d) err=255;                    // оставить прежний сигнал
     else err=0;                            // оставить прежний сигнал
  }
  else err = UpdatePID(err,cn, t);          // определение мощности канала 1 (10 А)
  if(err>255) err=255;
  return err;
}

void OutPulse(uint8_t cn, struct eeprom *t, struct rampv *ram){
 int16_t err;
  err = checkPV(cn, t, ram);
  if(err == 0x7FF){valRun = 0; return;};
  if(ok0==0||ok0==2){valRun = 0; return;};        // отключение впрыска по 2 каналу если идет разогрев
  valRun = UpdatePID(err,1, t);                   // определение длительности ВКЛ. состояния
  if(valRun < t->minRun) valRun = t->minRun;
  else if(valRun > t->period) valRun = t->period; // длит. впрыска не должна превыщать длит.переода
  if(err<=0) valRun = 0;                          // отключение впрыска по 2 каналу если перелив
}

uint8_t RelayPos(uint8_t cn, uint8_t offSet, struct eeprom *t, struct rampv *ram){	// [n] канал № 1 или 2
 signed char d;
 int16_t err;
  err = checkPV(cn, t, ram);
  if (err == 0x7FF) return OFF;
  d = UNCHANGED;
  if(err > offSet) d = ON;              // ниже (заданной-offSet) включить
  if(err < 0) d = OFF;                  // выше заданной отключить
  return d;
}

uint8_t RelayNeg(uint8_t cn, uint8_t offSet, uint8_t bias, struct eeprom *t, struct rampv *ram){	// [n] канал № 1 или 2
 uint8_t x=UNCHANGED;
 int16_t err;
  err = checkPV(cn, t, ram);
  if (err == 0x7FF) return OFF;
  err += offSet;                         // err -> отрицательная величина
  if (err < 0) x = ON;                   // выше (заданной+offSet) включить
  if (err > offSet-bias) x = OFF;        // ниже (заданной+0,2) отключить
  return x;
}

void heat_wet(struct eeprom *t, struct rampv *ram){
    pwTriac0 = OutPW(0, t, ram);
    if((ram->fuses&5)<5) pwTriac0 = OFF;             // КОРОТКОЕ замыкание или ОСТАНОВ тихоходного вентилятора
    if(OVRHEAT) {pwTriac0=OFF; ram->errors|=0x80;}  // ПЕРЕГРЕВ СИМИСТОРА !!!
    ram->power = pwTriac0/2;
    statPw[0]+=ram->power;               // расчет эконометра
    if(pwTriac0) HEATER = ON;            // включить канал 1 (симистор НАГРЕВАТЕЛЯ)

    if(t->relayMode == 4) OutPulse(1, t, ram);       // импульсное управление увлажнителем
    else pwTriac1 = OutPW(1, t, ram);        
    if(!(ok0&1)) pwTriac1=OFF;           // отключение УВЛАЖНЕНИЯ при РАЗОГРЕВЕ и ПЕРЕОХЛАЖДЕНИИ
    if(!(ram->fuses&8)) pwTriac1=OFF;    // ПРЕДОХРАНИТЕЛЬ увлажнителя
    if(pwTriac1) HUMIDI = ON;            // включить канал 2 (симистор УВЛАЖНЕНИЯ)
    statPw[1]+=(pwTriac1/2);             // расчет эконометра
}

void extra_1(struct eeprom *t, struct rampv *ram){
    uint8_t byte;
    byte = RelayNeg(0,t->extOn[0],t->extOff[0], t, ram);// доп. канал -> охлаждение
    if(VENTIL || CARBON){
      current = checkPV(0, t, ram);                  // Проверяем отклонение температуры
      if(current<(t->alarm[0])) byte = ON;      // если в заданных пределах открыть для ПРОВЕТРИВАНИЯ
      else if(current>(t->alarm[0]*2)) byte = OFF; // иначе закрыть
    }
    if(!(ram->fuses&1)) byte = ON;                   // ОСТАНОВ тихоходного вентилятора
    if(!(ram->fuses&0x10)) byte=OFF;                 // ПРЕДОХРАНИТЕЛЬ доп. канал №1
    switch (byte){
      case ON:  FLAP = ON; ram->pvFlap = FLAPOPEN; if(modules&8) chkflap(SETFLAP, &ram->pvFlap); break;// -- положение заслонки --
      case OFF: FLAP = OFF; ram->pvFlap=FLAPCLOSE; if(modules&8) chkflap(DATAREAD, &ram->pvFlap); break;// установка заслонки; сброс флага запрещения принудительной подачи воды
    }
  
}

void extra_2(struct eeprom *t, struct rampv *ram){
  uint8_t byte = UNCHANGED;
  if(t->extendMode==2){if((t->spT[0]-ram->pvT[0])>(t->ForceHeat+t->Hysteresis)) byte = ON; 
  else if((t->spT[0]-ram->pvT[0])<t->ForceHeat) byte = OFF;} // доп. канал -> Форсированный нагрев
  else if(t->extendMode==3) byte = RelayNeg(0,t->extOn[1],t->extOff[1], t, ram);// доп. канал -> Форсированное охлаждение
  else if(t->extendMode==4) byte = RelayNeg(1,t->extOn[1],t->extOff[1], t, ram);// доп. канал -> Форсированное осушение
  else if(t->extendMode==5) byte = RelayPos(1,t->Hysteresis, t, ram);              // доп. канал -> Дублирование канала увлажнения
  if(!(ok0&1) && t->extendMode==5) byte = OFF;  // отключение УВЛАЖНЕНИЯ при РАЗОГРЕВЕ и ПЕРЕОХЛАЖДЕНИИ
  if(!(ram->fuses&0x20)) byte=OFF;                // ПРЕДОХРАНИТЕЛЬ доп. канал №2
  if(t->extendMode>1){
    switch (byte){
        case ON:  EXT2 = ON;  break;
        case OFF: EXT2 = OFF; break;
    }
  }
}

