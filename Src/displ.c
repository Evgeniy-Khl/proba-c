#include "main.h"
#include "global.h"   // ����� ���������� ��������� eeprom � ��������� rampv
#include "displ.h"
#include "proc.h"

extern int8_t displmode, countsec, disableBeep;
extern uint8_t ok0, ok1, psword, setup, servis;
extern int16_t buf, currAdc;

//------- ������������ ��������� --------------------------------------------------------- 
void ledOut(uint8_t condition, uint8_t fuses){
 uint8_t led, i;
  led = portOut.value & 0x0F;       // 0b00001111
  for(i=0;i<8;i++) LedOff(i);
  i=0;
  while (i<4){
  	if(led & 1) LedOn(i,1);
  	led >>= 1; i++;
  }
  if(TURN) LedOn(4,1); else LedOn(5,1); // ����� ������; ����� �����
  if(condition&0x01) LedOn(7,1);        // �������
  led = fuses;
  i=0;
  while (i<8){
  	if(led & 1) LedOn(i,2);
  	led >>= 1; i++;
  }
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

void displ_3(int16_t val, int8_t mode, int8_t blink){
	uint8_t chr;
	switch (mode){
		case ERRORS:  chr=SIMBL_A;    break;  // A
		case FUSES:   chr=SIMBL_Pe;   break;  // �
    case SETUP:   chr=SIMBL_MBott; break; // MINUS Bott
    case SETUP2:  chr=SIMBL_M_Top; break; // MINUS Top
		case SERVIS:  chr=SIMBL_c;    break;  // c
		case CONTROL: chr=SIMBL_P;    break;  // P
		case PASS:    chr=SIMBL_TOPn; break;  // TOPn
    case VERS:    chr=SIMBL_u;    break;  // u
    case MODUL:   chr=SIMBL_o;    break;  // o
		case DISPL:   chr=SIMBL_d;    break;  // d
		default:      chr=SIMBL_BL;
	}
  if(blink){
    if(countsec&1){val=0; chr=SIMBL_BL;} // ������� �������
    else if(disableBeep==0) beeper_ON(DURATION);
  }
	if (val<100){
		if (val==-10){
			setChar(6, SIMBL_Pe); // �
			setChar(7, SIMBL_A);  // A     
		}
		else if (val==0){
			setChar(6, chr);
			setChar(7, SIMBL_BL);
		}
		else if (val<10){
			setChar(6, chr);
			setChar(7, val);
		}
		else {
			setChar(6, val/10); 
			setChar(7, val%10);
		}
	}
	else {
		setChar(6, chr);
		setChar(7, SIMBL_TOPo);
	}
}


void display(struct eeprom *t, struct rampv *ram){
 int8_t xx=0, yy=CONTROL, blink = 1;  
  switch (displmode){
    case 0: 
       displ_1(ram->pvT[0],COMMA); if(HIH5030||AM2301) displ_2(ram->pvRH,NOCOMMA); else displ_2(ram->pvT[1],COMMA);
       if(psword==10) {xx=-10;  blink=0;}                           // ������ ������
       else if(psword>0) {xx=psword; yy=PASS; blink=0;}             // �������� ������
       else if(ram->errors&0x70) xx=66+((ram->errors&0x70)>>4);     // ������ ���������� ��� ������
       else if(ram->errors&0x0F) xx=50+(ram->errors&0x0F);          // ������ ���������� ��� ������
       else if(ram->warning&0x70) xx=40+((ram->warning&0x70)>>4);   // ������ ���������� ��� ��������������
       else if(ram->warning&0x0F) xx=24+(ram->warning&0x0F);        // ������ ���������� ��� ��������������
       else if(ram->fuses&0x70) {xx=16+((ram->fuses&0x70)>>4);}     // ������ ���������� ��� ��������������
       else if(ram->fuses&0x0F) {xx=(ram->fuses&0x0F); yy=FUSES;}   // ������ F � ��� ��������������
       else if(t->condition&0x01) {xx=1; yy=CONTROL; blink=0;}          // ����� "���������"
       else if(t->condition&0x02) {xx=2; yy=CONTROL; blink=0;}          // ����� "���������� � ����������"
       else if(t->condition&0x04) {xx=3; yy=CONTROL; blink=0;}          // ����� "���������� � ���������"
       else if((t->condition&0x18)==0x08) {xx=4; yy=CONTROL; blink=0;}  // �������� ����������
       else if(t->condition&0x80) {xx=5; yy=CONTROL; blink=0;}          // ������� ������ ��� ����������� ������ !!!
       else if(VENTIL) {xx=6; yy=CONTROL; blink=0;}                     // �������������
//       else if(CARBON) {xx=7; yy=CONTROL; blink=0;}                     // ���������� ���
       else if(t->condition==0) {xx=ram->cellID&0x1F; yy=0; blink=0;}   // ��������! ���������� ����� ����� "n1"
       else {xx=0; yy=SETUP;}                                           // ����������� �����
       break;
    //---------------������� t0;-------------------------������� RH;---------------------������� t1;-----------------"d1"---------
    case 1: displ_1(t->spT[0],COMMA); if(HIH5030||AM2301) displ_2(t->spRH[1],NOCOMMA); else displ_2(t->spT[1],COMMA); xx=displmode; yy=DISPL; blink=0; break;
    //-------------------t1;--------------------t2;------------------"d2"---------
    case 2: displ_1(ram->pvT[1],COMMA); displ_2(ram->pvT[2],COMMA); xx=displmode; yy=DISPL; blink=0; break;
    //--------------- CO2 ------------------------ flap ----------------------"d3"------------------
    case 3: displ_1(ram->pvCO2[0]/10,NOCOMMA); displ_2(ram->pvFlap,NOCOMMA); xx=displmode; yy=DISPL; blink=0; break;
  }
  if(ok0>1) if(countsec&1) clr_1();
  if(ok1>1) if(countsec&1) clr_2();
  displ_3(xx,yy,blink);
  SendDataTM1638();
}

void display_setup(struct eeprom *t){
	if(buf>999) buf=999; else if(buf<-99) buf=-99;
	switch (setup){
		case 1: displ_1(buf,COMMA); clr_2(); break;                    // �1
		case 2: if(HIH5030||AM2301){clr_1(); displ_2(buf,NOCOMMA);} else {displ_1(buf,COMMA); clr_2();} break;// �2
		case 3: if(buf<0) buf=0; displ_1(buf,NOCOMMA); clr_2(); break; // �3 ����� ������������ ���������
		case 4: if(buf<0) buf=0; displ_1(buf,NOCOMMA); clr_2(); break; // �4 ����� ����������� ��������� (���� �� 0 �� ��� �������)
		case 5: displ_1(buf,COMMA); clr_2(); break;                    // �5 ������� �� ������ 1
		case 6: if(HIH5030||AM2301){clr_1(); displ_2(buf,NOCOMMA);} else {displ_1(buf,COMMA); clr_2();} break;// �6 ������� �� ������ 2
		case 7: displ_1(buf,COMMA); clr_2(); break;                    // �7 �������� ��� ���. ���������������� ������ 1
		case 8: displ_1(buf,COMMA); clr_2(); break;                    // �8 �������� ��� ����. ���������������� ������ 1
		case 9:                                                        // �9 �������� ��� ���. ���������������� ������ 2
			if(t->extendMode==4){clr_1(); if(HIH5030||AM2301) displ_2(buf,NOCOMMA); else displ_2(buf,COMMA);}
			else {displ_1(buf,COMMA); clr_2();}
			break;
		case 10:                                                       // �10 �������� ��� ����. ���������������� ������ 2
			if(t->extendMode==4){clr_1(); if(HIH5030||AM2301) displ_2(buf,NOCOMMA); else displ_2(buf,COMMA);}
			else {displ_1(buf,COMMA); clr_2();}
			break;

		case 19: clr_1(); displ_2(buf,COMMA); break;                   // �3 = 200 - 1,0 ���.
		case 20: clr_1(); displ_2(buf,COMMA); break;                   // �4 = 2000- 10,0 ���.
		case 21: clr_1(); displ_2(buf,NOCOMMA); break;                 // �5 = 3000-  15 ���.

		case 26: clr_1(); displ_2(buf,NOCOMMA); break;                 // �10 ���������� ������� HIH-4000
		case 27: clr_1(); displ_2(buf,NOCOMMA); break;                 // �11 ����������
		default: if(buf<0) buf=0; displ_1(buf,NOCOMMA); clr_2();
	}
	if (setup > 25) displ_3(setup-25,SETUP2,0);
  else if (setup > 15) displ_3(setup-16,VERS,0);
  else displ_3(setup,SETUP,0);
	SendDataTM1638();
}

void display_servis(struct rampv *ram){
	switch (servis){
		case 1: displ_1(currAdc,COMMA); clr_2(); break;     // C1 -> ������; ���� ����
		case 2: displ_1(ram->pvRH,NOCOMMA); /*displ_2(read_adc(3),NOCOMMA);*/ break;   // C2 -> ���������; �������� ���
		case 3: displ_1(ram->pvCO2[0],NOCOMMA); displ_2(ram->pvFlap,NOCOMMA); break;      // C3 -> ��������; ��2, ����������� ����.
		case 8: displ_1(buf,COMMA); clr_2(); break;                           // C8 -> ������� �������������� �������
		default: displ_1(buf,NOCOMMA); clr_2();
	}
	displ_3(servis,SERVIS,0);
	SendDataTM1638();
}

