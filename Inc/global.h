/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GLOBAL_H
#define __GLOBAL_H
extern struct {
  uint8_t RXBuffer[2];
  uint8_t TXBuffer[2];
  uint8_t buf[60];
  uint8_t ind;
  uint8_t timeOut;
  uint8_t devOk;
} bluetoothData;

extern struct rampv {
    uint16_t cellID;              // 2 ���� ind=0         ������� ����� �������
    int16_t pvT[MAX_DEVICES];     // 8 ���� ind=2-ind=9   �������� [MAX_DEVICES] �������� �����������
    int16_t pvRH;                 // 2 ���� ind=10;ind=11 �������� ������� ������������� ���������
    int16_t pvCO2[3];             // 6 ���� ind=12-ind=17 �������� ������� CO2 ---------- ����� 18 bytes ------------
    uint8_t pvTimer;              // 1 ���� ind=18        �������� �������
    uint8_t pvTmrCount;           // 1 ���� ind=19        �������� �������� �������� ��������
    uint8_t pvFlap;               // 1 ���� ind=20        ��������� �������� 
    int8_t power;                 // 1 ���� ind=21        �������� ���������� �� ����
    uint8_t fuses;                // 1 ���� ind=22        �������� ��������� 
    uint8_t errors, warning;      // 2 ���� ind=23;ind=24 ������ � ��������������
    uint8_t cost0, cost1;         // 2 ���� ind=25;ind=26 ������� ��������
    uint8_t date, hours;          // 2 ���� ind=27;ind=28 �������� ����� � �����
    uint8_t other0;               // 1 ���� ind=29        ��������� ��������
  } pv;// ------------------ ����� 27 bytes -------------------------------

extern struct eeprom {
    int16_t spT[2];     // 4 ���� ind=0-ind=3   ������� ����������� sp[0].spT->����� ������; sp[1].spT->������� ������
    int16_t spRH[2];    // 4 ���� ind=4-ind=7   sp[0].spRH->���������� HIH-5030; sp[1].spRH->������� ��������� ������ HIH-5030
    int16_t K[2];       // 4 ���� ind=8-ind=11  ���������������� �����.
    int16_t Ti[2];      // 4 ���� ind=12-ind=15 ������������ �����.
    int16_t minRun;     // 2 ���� ind=16;ind=17
    int16_t maxRun;     // 2 ���� ind=18;ind=19
    int16_t period;     // 2 ���� ind=20;ind=21
    int16_t TimeOut;    // 2 ���� ind=22;ind=23 ����� �������� ������ ������ ����������
    int16_t EnergyMeter;// 2 ���� ind=24;ind=25   ----------- ����� 26 bytes ------------
    int8_t timer[2];    // 2 ���� ind=26;ind=27 [0]-������.��������e [1]-�����.��������e
    int8_t alarm[2];    // 2 ���� ind=28;ind=29 ������ 5 = 0.5 ��.C
    int8_t extOn[2];    // 2 ���� ind=30;ind=31 �������� ��� ���. ���������������� ������
    int8_t extOff[2];   // 2 ���� ind=32;ind=33 �������� ��� ����. ���������������� ������
    uint8_t air[2];     // 2 ���� ind=34;ind=35 ������ ������������� air[0]-�����; air[1]-������; ���� air[1]=0-���������
    uint8_t spCO2;      // 1 ���� ind=36;       ������� �������� ��� ���������� ������������ ��2
    uint8_t identif;    // 1 ���� ind=37;       ������� ����� �������
    uint8_t condition;  // 1 ���� ind=38;       ��������� ������ (����. ���. ����������, � �.�.)
    uint8_t extendMode; // 1 ���� ind=39;       ����������� ����� ������  0-������; 1-����. 2-���� ����. 3-���� �����. 4-���� ����. 5-������ ����������
    uint8_t relayMode;  // 1 ���� ind=40;       �������� ����� ������  0-���; 1->�� ���.[0] 2->�� ���.[1] 3->�� ���.[0]&[1]
    uint8_t programm;   // 1 ���� ind=41;       ������ �� ���������
    uint8_t Hysteresis; // 1 ���� ind=42;       ���������� ������ ����������
    uint8_t ForceHeat;  // 1 ���� ind=43;       ������������� ������ 5 = 0.5 ���.�.
    uint8_t TurnTime;   // 1 ���� ind=44;       ����� �������� ������� ������ � ��������
    uint8_t HihEnable;  // 1 ���� ind=45;       ���������� ������������� ������� ���������
    uint8_t KoffCurr;   // 1 ���� ind=46;       ��������� ����. �� ���� ���������  (160 ��� AC1010 ��� 80 ��� �������)
    uint8_t coolOn;     // 1 ���� ind=47;       ����� ��������� ����������� ������ ���������� coolOn=80~>65 ���.�.
    uint8_t coolOff;    // 1 ���� ind=48;       ����� ���������� ����������� ������ ����������
    uint8_t Zonality;   // 1 ���� ind=49;       ����� ����������� � ������ ----------- 24 bytes ------------
} sp;// ------------------ ����� 50 bytes -------------------------------

extern union Byte portFlag;
extern union Byte portOut;
/* ---��������� � �������� ������ -----*/
//extern struct byte {
//    unsigned a0: 1;
//    unsigned a1: 1;
//    unsigned a2: 1;
//    unsigned a3: 1;
//    unsigned a4: 1;
//    unsigned a5: 1;
//    unsigned a6: 1;
//    unsigned a7: 1;
//};
///* ---��������� ���������� � ������� ������ -----*/
//union Byte {
//    unsigned char value;
//    struct byte bitfield;
//}portOut;

//#define CANAL1	portOut.bitfield.a0  // �������� �����������
//#define CANAL2	portOut.bitfield.a1  // ���� ��� �������� ����������
//#define EXT1		portOut.bitfield.a2  // ���� ���������������� ������
//#define EXT2		portOut.bitfield.a3  // ���� ���������������� ������
//#define TURN		portOut.bitfield.a4  // �������
//#define COOLER	portOut.bitfield.a5  // ���������� ����������

#endif /* __GLOBAL_H */
