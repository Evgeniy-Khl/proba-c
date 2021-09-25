#include "main.h"
#include "global.h"   // здесь определена структура eeprom и структура rampv
#include "displ.h"

extern int8_t displmode, countsec;
extern uint8_t ok0, ok1, psword, show, keynum, setup, servis, Hih, digit[], pvThermistor, Alarm, Aeration, Carbon, Superheat, portOut;
extern int16_t kWattHore, buf, current;
//------- Светодиодная индикация --------------------------------------------------------- 
void leddisplay(uint8_t state, uint8_t fuses){
 uint8_t led, i;
  led = portOut & 0x0F;               // 0b00001111
  for(i=0;i<8;i++) LedOff(i);
  i=0;
  do
  {
  	if (led & 1) LedOn(i);
  	led >>= 1;
  	i++;
  } while (i<4);
//  if(!(TURN_IN(portOut))) LedOn(x);   // лотки в верху
  if(!(fuses&0x80)) LedOn(4);         // дверь открыта
  if(!(fuses&0x01)) LedOn(5);         // останов вентилятора
//  if(!(fuses&0x02)) LedOn(x);         // нет поворота лотков
  if(state&0x10) LedOn(6);            // ГОРИЗОНТ УСТАНОВЛЕН
  if(Alarm&countsec) LedOn(7);        // Led Alarm
}

void displ_1(int16_t val, uint8_t comma){
 uint8_t neg=0;
  if (val<0) {neg=1; val=-val;}
  if (val<1000){
    if (neg){
       if (val<100){
         setChar(0, SIMBL_MINUS); 
         setChar(1, (val/10)%10); 
         if (comma) PointOn(1); else PointOff(1);
         setChar(2, val%10);
       }
       else {
          setChar(0, SIMBL_MINUS);
          setChar(1, val/100);
          setChar(2, (val/10)%10);
       }
    }
    else {
       if (val<100){
         setChar(0, SIMBL_BL);
         setChar(1, (val/10)%10); 
         if (comma) PointOn(1); else PointOff(1);
         setChar(2, val%10);
        }
       else {
         setChar(0, val/100);
         setChar(1, (val/10)%10); 
         if (comma) PointOn(1); else PointOff(1);
         setChar(2, val%10);
       }
    }
  }
  else {
     setChar(0, SIMBL_MINUS);// -> 1
     setChar(1, SIMBL_MINUS);// -> 9
     setChar(2, SIMBL_MINUS);// -> 9
  }
}

void displ_2(int16_t val, uint8_t comma){
 uint8_t neg=0;
  if (comma) comma=0x80;
  if (val<0) {neg=1; val=-val;}
  if (val<1000){
    if (neg){
       if (val<100){
         setChar(3, SIMBL_MINUS); 
         setChar(4, (val/10)%10); 
         if (comma) PointOn(4); else PointOff(4);
         setChar(5, val%10);
       }
       else {
          setChar(3, SIMBL_MINUS);
          setChar(4, val/100);
          setChar(5, (val/10)%10);
       }
    }
    else {
       if (val<100){
         setChar(3, SIMBL_BL);
         setChar(4, (val/10)%10); 
         if (comma) PointOn(4); else PointOff(4);
         setChar(5, val%10);
        }
       else {
         setChar(3, val/100);
         setChar(4, (val/10)%10); 
         if (comma) PointOn(4); else PointOff(4);
         setChar(5, val%10);
       }
    }
  }
  else {
     setChar(3, SIMBL_MINUS);// -> 1
     setChar(4, SIMBL_MINUS);// -> 9
     setChar(5, SIMBL_MINUS);// -> 9
  }
}

void clr_1(void){
	uint8_t byte;
	for (byte=0; byte<3; byte++) setChar(byte, SIMBL_BL);
}

void clr_2(void){
	uint8_t byte;
	for (byte=3; byte<6; byte++) setChar(byte, SIMBL_BL);
}

void clr_3(void){
	uint8_t byte;
	for (byte=6; byte<8; byte++) setChar(byte, SIMBL_BL);
}

void displ_3(int16_t val, int8_t mode, int8_t errors, int8_t warning){
	uint8_t chr;
	switch (mode){
		case ERRORS:  chr=SIMBL_A;    break;  // A
		case FUSES:   chr=SIMBL_Pe;   break;  // П
    case SETUP:   chr=SIMBL_u;    break;  // u
    case SETUP2:  chr=SIMBL_TOPn; break;  // TOPn
		case SERVIS:  chr=SIMBL_c;    break;  // c
		case CONTROL: chr=SIMBL_P;    break;  // P
		case PASS:    chr=SIMBL_TOPn; break;  // TOPn
    case VERS:    chr=SIMBL_n;    break;  // n
    case MODUL:   chr=SIMBL_o;    break;  // o
		case DISPL:   chr=SIMBL_d;    break;  // d
		default:      chr=SIMBL_BL;
	}
  if(Superheat&(countsec&1)||errors&0x7F||warning&0x7F){val=0; chr=SIMBL_BL;} // мигание дисплея
	if (val<100){
		if (val==-10){
			setChar(6, SIMBL_Pe); // П
			setChar(7, SIMBL_A);  // A     
		}
		else if (val==0 && chr==SIMBL_BL){
			setChar(6, chr);
			setChar(7, SIMBL_BL);
		}
		else if (val<10){
			setChar(6, chr);
			setChar(7, val%10);
		}
		else {
			setChar(6, val/10); 
			setChar(7, val%10);
		}
	}
	else {
		setChar(6, SIMBL_TOPo);
		setChar(7, SIMBL_TOPo);
	}
}


void display(struct eeprom *t, struct rampv *ram){
 int8_t i, xx=0, yy=CONTROL;  
  switch (displmode){
    case 0: 
       i=ram->fuses & 0x7F;// i>>=2; i&=0x1F;
       displ_1(ram->pvT[0],COMMA); if(Hih) displ_2(ram->pvRH,NOCOMMA); else displ_2(ram->pvT[1],COMMA);
       if(psword==10) xx=-10;
       else if(psword>0) {xx=psword; yy=PASS;}
       else if(Superheat) xx=90;
       else if(ram->errors&0x70) xx=66+((ram->errors&0x70)>>4);
       else if(ram->errors&0x0F) xx=50+ram->errors&0x0F;
       else if(i) {xx=i; yy=FUSES;}
       else if(ram->warning&0x70) xx=36+((ram->warning&0x70)>>4);
       else if(ram->warning&0x0F) xx=20+ram->warning&0x0F;
       else if(t->state&0x01) xx=1;         // Режим "ИНКУБАЦИЯ"
       else if(t->state&0x02) xx=2;         // Режим "подгототка к ОХЛАЖДЕНИЮ"
       else if(t->state&0x04) xx=3;         // Режим "подгототка к ВКЛЮЧЕНИЮ"
       else if((t->state&0x18)==0x08) xx=4; // ГОРИЗОНТ УСТАНОВЛЕН
       else if(t->state&0x80) xx=5;         // Поворот лотков при ОТКЛЮЧЕННОЙ камере !!!
       else if(Aeration) xx=6;              // Проветривание
       else if(Carbon) xx=7;                // Углекислый газ
       else {xx=0; yy=0;}                   // ОТКЛЮЧЕН!
       break;
    //---------------уставка t0;-------------------------уставка RH;---------------------уставка t1;-----------------"d1"---------
    case 1: displ_1(t->spT[0],COMMA); if(Hih) displ_2(t->spRH[1],NOCOMMA); else displ_2(t->spT[1],COMMA); xx=displmode; yy=DISPL; break;
    //-------------------t1;--------------------t2;------------------"d2"---------
    case 2: displ_1(ram->pvT[1],COMMA); displ_2(ram->pvT[2],COMMA); xx=displmode; yy=DISPL; break;
    //--------------- CO2 ------------------------ flap ----------------------"d3"------------------
    case 3: displ_1(ram->pvCO2[0]/10,NOCOMMA); displ_2(ram->pvFlap,NOCOMMA); xx=displmode; yy=DISPL; break;
  }
  if(ok0>1) if(countsec&1) clr_1();
  if(ok1>1) if(countsec&1) clr_2();
  displ_3(xx,yy,ram->errors,ram->warning);
  leddisplay(t->state, ram->fuses);                     // Светодиодная индикация
  SendDataTM1638();
}

void display_setup(struct eeprom *t){
	if(buf>999) buf=999; else if(buf<-99) buf=-99;
	switch (setup){
		case 1: displ_1(buf,COMMA); clr_2(); break;                    // У1
		case 2: if(Hih){clr_1(); displ_2(buf,NOCOMMA);} else {displ_1(buf,COMMA); clr_2();} break;// У2
		case 3: if(buf<0) buf=0; displ_1(buf,NOCOMMA); clr_2(); break; // У3 время отключенного состояния
		case 4: if(buf<0) buf=0; displ_1(buf,NOCOMMA); clr_2(); break; // У4 время включенного состояния (если не 0 то это секунды)
		case 5: displ_1(buf,COMMA); clr_2(); break;                    // У5 тревога по каналу 1
		case 6: if(Hih){clr_1(); displ_2(buf,NOCOMMA);} else {displ_1(buf,COMMA); clr_2();} break;// У6 тревога по каналу 2
		case 7: displ_1(buf,COMMA); clr_2(); break;                    // У7 смещение для ВКЛ. вспомогательного канала 1
		case 8: displ_1(buf,COMMA); clr_2(); break;                    // У8 смещение для ОТКЛ. вспомогательного канала 1
		case 9:                                                        // У9 смещение для ВКЛ. вспомогательного канала 2
			if(t->extendMode==4){clr_1(); if(Hih) displ_2(buf,NOCOMMA); else displ_2(buf,COMMA);}
			else {displ_1(buf,COMMA); clr_2();}
			break;
		case 10:                                                       // У10 смещение для ОТКЛ. вспомогательного канала 2
			if(t->extendMode==4){clr_1(); if(Hih) displ_2(buf,NOCOMMA); else displ_2(buf,COMMA);}
			else {displ_1(buf,COMMA); clr_2();}
			break;

		case 19: clr_1(); displ_2(buf,COMMA); break;                   // П3 = 200 - 1,0 сек.
		case 20: clr_1(); displ_2(buf,COMMA); break;                   // П4 = 2000- 10,0 сек.
		case 21: clr_1(); displ_2(buf,NOCOMMA); break;                 // П5 = 3000-  15 сек.

		case 26: clr_1(); displ_2(buf,NOCOMMA); break;                 // П10 подстройка датчика HIH-4000
		case 27: clr_1(); displ_2(buf,NOCOMMA); break;                 // П11 гистерезис
		default: if(buf<0) buf=0; displ_1(buf,NOCOMMA); clr_2();
	}
	if (setup > 25) displ_3(setup-25,SETUP2,0,0);
  else if (setup > 15) displ_3(setup-16,VERS,0,0);
  else displ_3(setup,SETUP,0,0);
	SendDataTM1638();
}

void display_servis(struct rampv *ram){
	switch (servis){
		case 1: displ_1(current,COMMA); displ_2(pvThermistor,NOCOMMA); break;     // C1 -> НАГРЕВ; Сила тока
		case 2: displ_1(ram->pvRH,NOCOMMA); /*displ_2(read_adc(3),NOCOMMA);*/ break;   // C2 -> ВЛАЖНОСТЬ; значение АЦП
		case 3: displ_1(ram->pvCO2[0],NOCOMMA); displ_2(ram->pvFlap,NOCOMMA); break;      // C3 -> ЗАСЛОНКА; СО2, СЕРВОПРИВОД град.
		case 8: displ_1(buf,COMMA); clr_2(); break;                           // C8 -> Уставка форсированного нагрева
		default: displ_1(buf,NOCOMMA); clr_2();
	}
	displ_3(servis,SERVIS,0,0);
	SendDataTM1638();
}

