// ��� ��� ��� AT24C32N �����, ��� �������� ����� (A0, A1 � A2), ���������� �������� ������ �������. �� 0x50 �� 0x57.
#include "main.h"
extern I2C_HandleTypeDef EEPROM_I2C_PORT;

char writing0[9], writing1[9], i;
extern uint16_t eepAddr;
extern uint16_t sizeAddr;
extern uint8_t pageSize;
HAL_StatusTypeDef ret_stat;

void dspl_error(uint8_t status)
{
  SSD1306_GotoXY(0,40);
  switch(status)
   {
     case HAL_OK:
         SSD1306_Puts("Ok!", &Font_11x18, SSD1306_COLOR_WHITE); break;
     case HAL_ERROR:
         SSD1306_Puts("Error", &Font_11x18, SSD1306_COLOR_WHITE); break;
     case HAL_BUSY:
         SSD1306_Puts("Busy", &Font_11x18, SSD1306_COLOR_WHITE); break;
     case HAL_TIMEOUT:
         SSD1306_Puts("TimeOut", &Font_11x18, SSD1306_COLOR_WHITE); break;
   }
  SSD1306_UpdateScreen();
  HAL_Delay(5000);
}

void mem_display(uint16_t wait, char* str0, char* str1){
  SSD1306_Fill(SSD1306_COLOR_BLACK); //clear oled
  SSD1306_GotoXY(0,0);
  SSD1306_Puts(str0, &Font_11x18, SSD1306_COLOR_WHITE);
  SSD1306_GotoXY(0,20);
  SSD1306_Puts(str1, &Font_11x18, SSD1306_COLOR_WHITE);
  SSD1306_UpdateScreen();
  HAL_Delay(wait);
}

void eep_write(uint16_t memAddr, uint8_t *data){
  int16_t writebyte;
  uint8_t amount = EEP_DATA;
  uint8_t *begData = data;
  uint16_t begMemAddr = memAddr;
  /* ------------ display ------------------ */
  sprintf(writing0, "Start");//"Starting the test - writing to the memory...\r\n"
  sprintf(writing1, "writing");
  mem_display(1000, (char*)writing0, (char*)writing1);
  /* --------------------------------------- */
  if (amount-pageSize > 0) writebyte = pageSize;
  else writebyte = amount;
  while (amount > 0){
    ret_stat = HAL_I2C_Mem_Write(&EEPROM_I2C_PORT, eepAddr, memAddr, sizeAddr, (uint8_t*)data, writebyte, HAL_MAX_DELAY);
    if(ret_stat) dspl_error(ret_stat);
    /* ------------ display ------------------ */
    sprintf(writing0, "Waiting");//"OK, now waiting until device is ready...\r\n"
    sprintf(writing1, "is ready");
    mem_display(1000, (char*)writing0, (char*)writing1);
    /* --------------------------------------- */
    for(i=0;i<100;i++) { // wait...
      ret_stat = HAL_I2C_IsDeviceReady(&EEPROM_I2C_PORT, eepAddr, 1, HAL_MAX_DELAY);
      if(ret_stat == HAL_OK) break;
    }
    /* ------------ display ------------------ */
    if (i>99) {
      sprintf(writing0, "Device");//"Done, now comparing...\r\n"
      sprintf(writing1, "notReady");
      mem_display(2000, (char*)writing0, (char*)writing1);
      return;
    }
    /* --------------------------------------- */
    memAddr += writebyte;
    data += writebyte;
    amount -= writebyte;
    if (amount-pageSize > 0) writebyte = pageSize;
    else writebyte = amount;
  }
  /* ----------------- NOW COMPARING... ----------------------------- */
  uint8_t temp[EEP_DATA];
  eep_read(begMemAddr, temp);
  /* ------------ display ------------------ */
  sprintf(writing0, "Now");
  sprintf(writing1, "comparing");
  mem_display(1000, (char*)writing0, (char*)writing1);
  /* --------------------------------------- */
  SSD1306_GotoXY(0,40);
	if(memcmp(begData, temp, EEP_DATA) == 0){
  /* ------------ display ------------------ */
    SSD1306_Puts("passed!", &Font_11x18, SSD1306_COLOR_WHITE);
  }
  else SSD1306_Puts("failed!", &Font_11x18, SSD1306_COLOR_WHITE);
  SSD1306_UpdateScreen();
  HAL_Delay(3000);
  /* --------------------------------------- */
}

void eep_read(uint16_t memAddr, uint8_t *data){
  /* ------------ display ------------------ */
  sprintf(writing0, "Start");//"Device is ready, now reading...\r\n"
  sprintf(writing1, "reading");
  mem_display(1000, (char*)writing0, (char*)writing1);
  /* --------------------------------------- */
  ret_stat = HAL_I2C_IsDeviceReady(&EEPROM_I2C_PORT, eepAddr, 1, HAL_MAX_DELAY);
  if(ret_stat) {dspl_error(ret_stat); return;}
  memAddr = 0x0000;
  ret_stat = HAL_I2C_Mem_Read(&EEPROM_I2C_PORT, eepAddr, memAddr, sizeAddr, (uint8_t*)data, EEP_DATA, HAL_MAX_DELAY);
  if(ret_stat) dspl_error(ret_stat);
}

void eep_initial(uint16_t memAddr, uint8_t *data){
  /* ------------ display ------------------ */
  sprintf(writing0, "Initial");//"Device is ready, now reading...\r\n"
  sprintf(writing1, "EEPROM");
  mem_display(1000, (char*)writing0, (char*)writing1);
  /* --------------------------------------- */
  
  uint8_t source[EEP_DATA]={
    /* ---------------------- uint16_t ---------------------------------*/
    /* ������� ����������� */
    0x5E,0x01,// spT[0]->����� ������ = 350 => 35 ��.C
    0x2C,0x01,// spT[1]->������� ������ = 300 => 30 ��.C
    /* ������� ��������� */
     0, 0,    // spRH[0]->���������� HIH-5030 = 0
    67, 0,    // spRH[1]->������� ��������� ������ HIH-5030 = 67%
    25, 0,    // K[0] ���������������� �����. = 25
    20, 0,    // K[1] ���������������� �����. = 20
    0x84,0x03,// Ti[0] ������������ �����. = 900
    0x84,0x03,// Ti[1] ������������ �����. = 900
    0x64,0x00,// minRun = 100
    0x58,0x02,// maxRun = 600
    0xB8,0x0B,// period = 3000
    0x08,0x07,// TimeOut ����� �������� ������ ������ ���������� = 1800
     0, 0,    // EnergyMeter = 0
    /* ---------------------- uint8_t ---------------------------------*/
     60,      // D[26] timer[0] -������.��������e = 60 ���.
      0,      // timer[1] -�����.��������e  = 0 ���.
      5,      // alarm[0] 5 = 0.5 ��.C
      5,      // alarm[1] 5 = 0.5 ��.C
      5,      // extOn[0] �������� ��� ���. ���������������� ������ = 5 => 0.5 ��.C
      5,      // extOn[1] �������� ��� ���. ���������������� ������ = 5 => 0.5 ��.C
      2,      // extOff[0] �������� ��� ����. ���������������� ������ = 2 => 0.2 ��.C
      2,      // extOff[1] �������� ��� ����. ���������������� ������ = 2 => 0.2 ��.C
      
     40,      // spCO2[0] ������� �������� ��� ���������� ������������ ��2 MAX; = 40 => 4000
     40,      // spCO2[1] ������� �������� ��� ���������� ������������ ��2 MIN; = 30 => 1000
      1,      // spCO2[2] ������� �������� ��� ���������� ������������ ��2 ???; = 1
     60,      // air[0] ������ ������������� �����; = 60 ���.
      0,      // air[1] ������ ������������� air[1]-������; ���� air[1]=0-���������
      1,      // D[39] identif ������� ����� �������
      0,      // state ��������� ������ (����. ���. ����������, � �.�.)
      0,      // extendMode ����������� ����� ������  0-������; 1-����. 2-���� ����. 3-���� �����. 4-���� ����. 5-������ ����������
      2,      // relayMode �������� ����� ������  0-���; 1->�� ���.[0] 2->�� ���.[1] 3->�� ���.[0]&[1]
      0,      // programm ������ �� ���������
      1,      // Hysteresis ���������� ������ ����������
      5,      // ForceHeat ������������� ������ = 5 => 0.5 ���.�.
     80,      // TurnTime ����� �������� ������� ������ � �������� = 80 sec.
      0,      // HihEnable ���������� ������������� ������� ��������� = 0 => �� ���������!
    160,      // KoffCurr ��������� ����. �� ���� ���������  (160 ��� AC1010)
     80,      // coolOn ����� ��������� ����������� ������ ���������� coolOn=80~>65 ���.�.
     70,      // coolOff ����� ���������� ����������� ������ ����������
     20,      // Zonality ����� ����������� � ������ = 20 => 0.2 ��.C
  };
  memcpy(data, source, EEP_DATA);
  ret_stat = HAL_I2C_IsDeviceReady(&EEPROM_I2C_PORT, eepAddr, 1, HAL_MAX_DELAY);
  if(ret_stat) {dspl_error(ret_stat); return;}
  
  eep_write(memAddr, data);
  
  /* ------------------- ����� ������ ��� ������������� ------------------------------------ */
  /* ------------ display ------------------ */
  sprintf(writing0, "Writing");//"Starting the test - writing to the memory...\r\n"
  sprintf(writing1, "duplicate");
  mem_display(1000, (char*)writing0, (char*)writing1);
  /* --------------------------------------- */
  uint16_t x;
  x = EEP_DATA/pageSize;
  x++;
  x*=pageSize;  // �������� ����� ����� ������ �� �������� �������� ������
  memAddr += x;
  
  ret_stat = HAL_I2C_IsDeviceReady(&EEPROM_I2C_PORT, eepAddr, 1, HAL_MAX_DELAY);
  if(ret_stat) {dspl_error(ret_stat); return;}
  
  eep_write(memAddr, data);
}

uint8_t rtc_check(void){
  uint8_t i=0;
  /* ------------ display ------------------ */
  sprintf(writing0, "Looking");
  sprintf(writing1, "EEPROM");
  mem_display(1000, (char*)writing0, (char*)writing1);
  /* --------------------------------------- */
  ret_stat = HAL_I2C_IsDeviceReady(&EEPROM_I2C_PORT, 0xD0, 1, HAL_MAX_DELAY); // [0xD0] � ����� ���������� DS3231 real-time clock (RTC)
  SSD1306_GotoXY(0,40);
  if (ret_stat == HAL_OK){
    SSD1306_Puts("24C32", &Font_11x18, SSD1306_COLOR_WHITE);
    eepAddr = (0x57 << 1);  // HAL expects address to be shifted one bit to the left
    sizeAddr = I2C_MEMADD_SIZE_16BIT;
    pageSize = 32;          // AT24C32A ��� AT24C64A. The 32K/64K EEPROM is capable of 32-byte page writes
    i = 1;
  }
  else SSD1306_Puts("24C04", &Font_11x18, SSD1306_COLOR_WHITE);
  SSD1306_UpdateScreen();
  HAL_Delay(1000);
  return i;
}
