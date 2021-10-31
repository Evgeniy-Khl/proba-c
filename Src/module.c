#include "main.h"
#include "global.h"   // ����� ���������� ��������� eeprom � ��������� rampv
#include "module.h"
#include "ds18b20.h"

extern uint8_t beepOn;
uint8_t outbuffer[4], inbuffer[4], cmdmodule;
//--------------------------------------------------
__STATIC_INLINE void DelayMicro(__IO uint32_t micros){
  micros *= (SystemCoreClock / 1000000) / 8;
  while (micros--) ;  // Wait till done
}
//--------------------------------------------------
void module_port_init(void){
  HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8);
  GPIOB->CRH |= GPIO_CRH_MODE8;
  GPIOB->CRH |= GPIO_CRH_CNF8_0;
  GPIOB->CRH &= ~GPIO_CRH_CNF8_1;
}
//-----------------------------------------------
uint8_t module_Reset(void){
  uint8_t status;
  GPIOB->BSRR =GPIO_BSRR_BR8;//������ ������� (�������� 8 ��� ����� B )
  DelayMicro(485);//�������� ��� ������� �� 480 �����������
  GPIOB->BSRR =GPIO_BSRR_BS8;//������� ������� (���������� 8 ��� ����� B )
  DelayMicro(65);//�������� ��� ������� �� 60 �����������
  status = (GPIOB->IDR & GPIO_IDR_IDR8 ? 1 : 0);//��������� �������
  DelayMicro(500);//�������� ��� ������� �� 480 �����������
  //(�� ������ ������ ������� ��������, ��� ��� ����� ���� ���������� � ��������)
  return (status ? 1 : 0);//����� ���������
}
//--------------------------------------------------
uint8_t module_ReadBit(void){
  uint8_t bit = 0;
  GPIOB->BSRR =GPIO_BSRR_BR8;//������ �������
  DelayMicro(2);
  GPIOB->BSRR =GPIO_BSRR_BS8;//������� �������
  DelayMicro(13);
  bit = (GPIOB->IDR & GPIO_IDR_IDR8 ? 1 : 0);//��������� �������
  DelayMicro(45);
  return bit;
}
//-----------------------------------------------
uint8_t module_ReadByte(void){
  uint8_t data = 0;
  for (uint8_t i = 0; i < 8; i++)
  data += module_ReadBit() << i;
  return data;
}
//-----------------------------------------------
void module_WriteBit(uint8_t bit){
  GPIOB->BSRR =GPIO_BSRR_BR8;//������ �������
  DelayMicro(bit ? 3 : 65);
  GPIOB->BSRR =GPIO_BSRR_BS8;//������� �������
  DelayMicro(bit ? 65 : 3);
}
//-----------------------------------------------
void module_WriteByte(uint8_t dt){
  for (uint8_t i = 0; i < 8; i++){
    module_WriteBit(dt >> i & 1);
    //Delay Protection
    DelayMicro(5);
  }
}
//--------------------------------------------------
int8_t module_check(uint8_t fc){
 int8_t i, byte;
  outbuffer[3] = dallas_crc8(outbuffer, 3);// ����������� ����� ��� ������ 3 ����
  for (i=0; i<5; i++)
   {
    module_Reset();             // 1 Wire Bus initialization
    module_WriteByte(fc);      // Load Family code
    for (byte=0; byte<4; byte++) module_WriteByte(outbuffer[byte]);
    DelayMicro(COUNT);          // ������� ���� ������ ���������� ����������
    for (byte=0; byte<4; byte++) inbuffer[byte] = module_ReadByte();// Read 4 byte
    byte = module_ReadByte();  // Read CRC byte #5
    if(byte == dallas_crc8(inbuffer,4)) break;
    GPIOB->BSRR =GPIO_BSRR_BR8;//������ ������� (�������� 8 ��� ����� B)
    DelayMicro(200);            //�������� ��� ������� �� 200 �����������
    GPIOB->BSRR =GPIO_BSRR_BS8;//������� ������� (���������� 8 ��� ����� B)
    DelayMicro(800);            //�������� ��� ������� �� 800 �����������
   };
  if(i>4) byte=0; else byte=1;
  return byte;
}

int8_t chkcooler(uint8_t state){ // �������� �������� ����������� �����������
 int8_t byte;
 static uint8_t countTry;
  outbuffer[0]=DATAREAD;        // Function Command
  outbuffer[1]=0x00;            // Data 1
  outbuffer[2]=0x00;            // Data 2
  byte = module_check(ID_HALL); // ������������� �����
  if(byte)                      // ���� ���� ������� ...
   {
     byte = inbuffer[0];
     countTry = 0;
     if((state&0x41)==0x41){    // ���� ������� ���������� ����������� �����������
       if(byte<1) ALARM = 1;    // ������ �������� �����������
      }
   }
  else if(++countTry>5) byte = -1; // ����� ������ �����
  return byte;
}

int8_t chkhorizon(uint8_t state){ // �������� ������� ����� ��������
 int8_t byte;
 static uint8_t countTry;
  outbuffer[0]=DATAREAD;          // Function Command
  outbuffer[1]=cmdmodule;         // Data 1
  if(state&0x20) outbuffer[2]=1;  // Data 2 ������� ���������� �������� ������
  else outbuffer[2]=0;            // Data 2 �������� ���������� �������� ������
  byte = module_check(ID_HORIZON);// ������������� �����
  if(byte)                        // ���� ���� ������� ...
   {
     byte = inbuffer[0];
     countTry = 0;
     switch(cmdmodule)
       {
         case NEW_TURN:
          {
            if(inbuffer[1]==2)                                 // 2-> ������� �� ������
             {
               cmdmodule=0;
               if((state&0x21)==0x21) {ALARM = 1;}         // ���� ������� ������ � ���������� �������� ������
               if((state&0xA0)==0xA0) {beepOn = DURATION;} // ���� ������� ������ ������� ������
             } 
            else if(inbuffer[1]==3) {cmdmodule=0;} // 3-> ������� ������
//            else fuses |= 2;                                   // 1-> ������ �������
          }; break;
         case SETHORIZON:
          {
            if((state&0x18)==0) cmdmodule=0;                   // ������ ��������� � ��������
          }; break;
         default: cmdmodule=0;
       };
   }
  else if(++countTry>5) byte = -1; // ����� ������ ��������
  return byte;
}

int8_t readCO2(struct rampv *ram){ // ������ ������ ��2
 int8_t byte;
 static uint8_t countTry;
  outbuffer[0]=DATAREAD;       // Function Command
  outbuffer[1]=0x00;           // Data 1
  outbuffer[2]=0x00;           // Data 2
  byte = module_check(ID_CO2); // ������������� �����
  if(byte)                     // ���� ���� ������� ...
   {
     byte = inbuffer[0];       // �����. ������. �����(0x30) + ����� ������ 
     byte = inbuffer[0]&0x03;  // ����� ������
     //inbuffer[1]; �� ������������
     ram->pvCO2[byte] = inbuffer[2]+inbuffer[3]*256;// �������� CO2
     countTry = 0;
   }
  else if(++countTry>5) {byte = -1; for (byte=0;byte<3;byte++) ram->pvCO2[byte]=0;}// ����� ������ CO2
  return byte;
}

int8_t chkflap(uint8_t cmd, uint8_t *pvF){ // �������� ��������� ��������� ��������
 int8_t byte;
 static uint8_t countTry;
  outbuffer[0]=cmd;             // Function Command
  outbuffer[1]=*pvF;            // Data 1
  outbuffer[2]=0x00;            // Data 2
  byte = module_check(ID_FLAP); // ������������� �����
  if(byte)                      // ���� ���� ������� ...
   {
     *pvF = inbuffer[0];
     countTry = 0;
   }
  else if(++countTry>5) {byte = -1; *pvF = 0;} // ����� ��������������� ������
  return byte;
}
