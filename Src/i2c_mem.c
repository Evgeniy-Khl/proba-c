// ��� ��� ��� AT24C32N �����, ��� �������� ����� (A0, A1 � A2), ���������� �������� ������ �������. �� 0x50 �� 0x57.
#include "main.h"
#include "displ.h"
#include "global.h"   // ����� ���������� ��������� eeprom � ��������� rampv

extern I2C_HandleTypeDef EEPROM_I2C_PORT;

struct {
  uint8_t eepAddr;
  uint8_t sizeAddr;
  uint8_t pageSize;
} eepMem;

char writing0[9], writing1[9], i;

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
//  HAL_Delay(wait);
}

void eep_write(uint16_t memAddr, uint8_t *data){
	int16_t writebyte;
	uint8_t i, amount = EEP_DATA;
	uint8_t *begData = data;
	uint16_t begMemAddr = memAddr;
  EEPSAVE = 0;
	/* ------------ display ------------------ */
	sprintf(writing0, "Start");//"Starting the test - writing to the memory...\r\n"
	sprintf(writing1, "writing");
	mem_display(1000, (char*)writing0, (char*)writing1);
	/* --------------------------------------- */
  for(i=0;i<8;i++) setChar(i, SIMBL_M_Top);
  SendDataTM1638();
  
	if (amount-eepMem.pageSize > 0) writebyte = eepMem.pageSize;
	else writebyte = amount;
	while (amount > 0){
		ret_stat = HAL_I2C_Mem_Write(&EEPROM_I2C_PORT, eepMem.eepAddr, memAddr, eepMem.sizeAddr, (uint8_t*)data, writebyte, HAL_MAX_DELAY);
		if(ret_stat) dspl_error(ret_stat);
		/* ------------ display ------------------ */
		sprintf(writing0, "Waiting");//"OK, now waiting until device is ready...\r\n"
		sprintf(writing1, "is ready");
		mem_display(1000, (char*)writing0, (char*)writing1);
		/* --------------------------------------- */
		for(i=0;i<100;i++) { // wait...
		  ret_stat = HAL_I2C_IsDeviceReady(&EEPROM_I2C_PORT, eepMem.eepAddr, 1, HAL_MAX_DELAY);
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
		if (amount-eepMem.pageSize > 0) {writebyte = eepMem.pageSize; for(i=0;i<8;i++) setChar(i, SIMBL_MINUS); SendDataTM1638();}
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
    for(i=0;i<8;i++) setChar(i, SIMBL_MBott);
    SendDataTM1638();
	}
	else {
    SSD1306_Puts("failed!", &Font_11x18, SSD1306_COLOR_WHITE);
    for(i=0;i<8;i++) {setChar(i,SIMBL_BL); PointOn(i);}// BL+�����
    SendDataTM1638();
  }
	SSD1306_UpdateScreen();
	HAL_Delay(1000);
	/* --------------------------------------- */
}

void eep_read(uint16_t memAddr, uint8_t *data){
	/* ------------ display ------------------ */
//	sprintf(writing0, "Start");//"Device is ready, now reading...\r\n"
//	sprintf(writing1, "reading");
//	mem_display(1000, (char*)writing0, (char*)writing1);
	/* --------------------------------------- */
	ret_stat = HAL_I2C_IsDeviceReady(&EEPROM_I2C_PORT, eepMem.eepAddr, 1, HAL_MAX_DELAY);
	if(ret_stat) {dspl_error(ret_stat); return;}
	memAddr = 0x0000;
	ret_stat = HAL_I2C_Mem_Read(&EEPROM_I2C_PORT, eepMem.eepAddr, memAddr, eepMem.sizeAddr, (uint8_t*)data, EEP_DATA, HAL_MAX_DELAY);
	if(ret_stat) dspl_error(ret_stat);
}

void eep_initial(uint16_t memAddr, uint8_t *data){
	/* ------------ display ------------------ */
	sprintf(writing0, "Initial");//"Device is ready, now reading...\r\n"
	sprintf(writing1, "EEPROM");
	mem_display(1000, (char*)writing0, (char*)writing1);
	/* --------------------------------------- */
  for(int8_t i=0;i<8;i++) setChar(i,SIMBL_TOPo);
  SendDataTM1638();

	const uint8_t source[EEP_DATA]={
	/* ---------------------- uint16_t ---------------------------------*/
	/* ������� ����������� */
	0x5E,0x01,// data[0,1];   spT[0]->����� ������ = 350 => 35 ��.C
	0x2C,0x01,// data[2,3];   spT[1]->������� ������ = 300 => 30 ��.C
	/* ������� ��������� */
	 0, 0,    // data[4,5];   spRH[0]->���������� HIH-5030 = 0
	67, 0,    // data[6,7];   spRH[1]->������� ��������� ������ HIH-5030 = 67%
 125, 0,    // data[8,9];   K[0] ���������������� �����. = 125
 100, 0,    // data[10,11]; K[1] ���������������� �����. = 100
	0x84,0x03,// data[12,13]; Ti[0] ������������ �����. = 900
	0x84,0x03,// data[14,15]; Ti[1] ������������ �����. = 900
	0xF4,0x01,// data[16,17]; minRun = 500    -> 0.5���.
	0x10,0x27,// data[18,19]; maxRun = 10000  -> 10 ���.
	0x60,0xEA,// data[20,21]; period = 60000  -> 1 ���..
	0x08,0x07,// data[22,23]; TimeOut ����� �������� ������ ������ ���������� = 1800
	 0, 0,    // data[24,25]; EnergyMeter = 0
	/* ---------------------- uint8_t ---------------------------------*/
	 60,      // data[26]; timer[0]  ������.��������e = 60 ���.
	  0,      // data[27]; timer[1]  �����.��������e  = 0 ���.
	  5,      // data[28]; alarm[0]  5 = 0.5 ��.C
	  5,      // data[29]; alarm[1]  5 = 0.5 ��.C
	  5,      // data[30]; extOn[0]  �������� ��� ���. ���������������� ������ 1 = 5 => 0.5 ��.C
	  5,      // data[31]; extOn[1]  �������� ��� ���. ���������������� ������ 2 = 5 => 0.5 ��.C
	  2,      // data[32]; extOff[0] �������� ��� ����. ���������������� ������ 1 = 2 => 0.2 ��.C
	  2,      // data[33]; extOff[1] �������� ��� ����. ���������������� ������ 2 = 2 => 0.2 ��.C
   60,      // data[34]; air[0]    ������ ������������� �����; = 60 ���.
	  0,      // data[35]; air[1]    ������ ������������� air[1]-������; ���� air[1]=0-���������
	 40,      // data[36]; spCO2     ������� �������� ��� ���������� ������������ ��2 MAX; = 40 => 4000
	  1,      // data[37]; identif ������� ����� �������
	  0,      // data[38]; condition ��������� ������ (����. ���. ����������, � �.�.)
	  0,      // data[39]; extendMode ����������� ����� ������  0-������; 1-����. 2-���� ����. 3-���� �����. 4-���� ����. 5-������ ����������
	  2,      // data[40]; relayMode �������� ����� ������  0-���; 1->�� ���.[0] 2->�� ���.[1] 3->�� ���.[0]&[1]
	  0,      // data[41]; programm ������ �� ���������
	  1,      // data[42]; Hysteresis ���������� ������ ����������
	  5,      // data[43]; ForceHeat ������������� ������ = 5 => 0.5 ���.�.
	 80,      // data[44]; TurnTime ����� �������� ������� ������ � �������� = 80 sec.
	  0,      // data[45]; HihEnable ���������� ������������� ������� ��������� = 0 => �� ���������!
	100,      // data[46]; KoffCurr ��������� ����. �� ���� ���������  (160 ��� AC1010)
	 80,      // data[47]; coolOn ����� ��������� ����������� ������ ���������� coolOn=80~>65 ���.�.
	 70,      // data[48]; coolOff ����� ���������� ����������� ������ ����������
	 20,      // data[49]; Zonality ����� ����������� � ������ = 20 => 0.2 ��.C
	};
	memcpy(data, source, EEP_DATA);   // ����������� ������ ������ � ������
	ret_stat = HAL_I2C_IsDeviceReady(&EEPROM_I2C_PORT, eepMem.eepAddr, 1, HAL_MAX_DELAY);
	if(ret_stat) {dspl_error(ret_stat); return;}

	eep_write(memAddr, data);

//	/* ------------------- ����� ������ ��� ������������� ------------------------------------ */
//	/* ------------ display ------------------ */
//	sprintf(writing0, "Writing");//"Starting the test - writing to the memory...\r\n"
//	sprintf(writing1, "duplicate");
//	mem_display(1000, (char*)writing0, (char*)writing1);
//	/* --------------------------------------- */
//	uint16_t x;
//	x = EEP_DATA/pageSize;
//	x++;
//	x*=pageSize;  // �������� ����� ����� ������ �� �������� �������� ������
//	memAddr += x;
//	ret_stat = HAL_I2C_IsDeviceReady(&EEPROM_I2C_PORT, eepAddr, 1, HAL_MAX_DELAY);
//	if(ret_stat) {dspl_error(ret_stat); return;}
//	eep_write(memAddr, data);
}

uint8_t rtc_check(void){
	uint8_t i=0;
	/* ------------ display ------------------ */
	sprintf(writing0, "Looking");
	sprintf(writing1, "EEPROM");
	mem_display(500, (char*)writing0, (char*)writing1);
	/* --------------------------------------- */
  
	ret_stat = HAL_I2C_IsDeviceReady(&EEPROM_I2C_PORT, 0xD0, 1, HAL_MAX_DELAY); // [0xD0] � ����� ���������� DS3231 real-time clock (RTC)
	SSD1306_GotoXY(0,40);
	if (ret_stat == HAL_OK){
	SSD1306_Puts("24C32", &Font_11x18, SSD1306_COLOR_WHITE);
  
	eepMem.eepAddr = (0x57 << 1);  // HAL expects address to be shifted one bit to the left
	eepMem.sizeAddr = I2C_MEMADD_SIZE_16BIT;
	eepMem.pageSize = 32;          // AT24C32A ��� AT24C64A. The 32K/64K EEPROM is capable of 32-byte page writes
	i = 1;
	}
	else SSD1306_Puts("24C04", &Font_11x18, SSD1306_COLOR_WHITE);
	return i;
}
///* ------------------- ������������� �� ����� ������ ------------------------------------ */
//uint8_t reset(uint16_t memAddr, uint8_t *data){
//	uint8_t source[EEP_DATA];	// ������ ����� ��������� ���������
//	uint16_t x;
//	x = EEP_DATA/pageSize;
//	x++;
//	x*=pageSize;	// �������� ����� ����� ������ �� �������� �������� ������
//	memAddr += x;	// ����� ���� �������� ������
//	eep_read(memAddr, source);
//	memcpy(data, source, EEP_DATA);
//	ret_stat = HAL_I2C_IsDeviceReady(&EEPROM_I2C_PORT, eepAddr, 1, HAL_MAX_DELAY);
//	if(ret_stat) {dspl_error(ret_stat); return 0;}

//	eep_write(memAddr, data);
//	return 1;
//}
