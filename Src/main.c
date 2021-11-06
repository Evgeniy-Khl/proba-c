/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "proc.h"
#include "module.h"
#include "ds18b20.h"
#include "hih.h"
//#include "stm32f1xx_hal_adc.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
void bluetoothCallback(void);
void bluetoothName(void);
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
volatile uint16_t adc[2] = {0,0};      // � ��� ��� ������ ���, ������� ������ �� ���� ���������
volatile uint8_t flag = 0;             // ���� ��������� �������������� ���

union Byte portOut;
union Byte portFlag;

extern uint8_t ds18b20_amount, disableBeep, topOwner, topUser, botUser, ok0, ok1, psword, pvAeration;
extern int16_t buf, valRun;

extern struct {
  uint8_t RXBuffer[2];
  uint8_t TXBuffer[2];
  uint8_t buf[60];
  uint8_t ind;
  uint8_t timeOut;
  uint8_t devOk;
} bluetoothData;

extern struct {
  uint8_t eepAddr;
  uint8_t sizeAddr;
  uint8_t pageSize;
} eepMem;

// -------- ISIDA ------
uint8_t show=0, getButton=0, modules=0, setup, servis, waitset, waitkey=WAITCOUNT;
int8_t countsec=-15, displmode;
/*
ext[0] ������ �����
ext[1] ������ ���������
ext[2] ������ ��2
ext[3] ������ ��������
*/
int16_t pwTriac0, pwTriac1, pulsPeriod, currAdc, humAdc, beepOn, alarmErr, statPw[2]; 

/* ----------------------------- BEGIN point value -------------------------------------- */
union pointvalue{
  uint8_t pvdata[30];
  struct rampv {
   uint16_t cellID;               // 2 ���� ind=0         ������� ����� � ��� �������
    int16_t pvT[MAX_DEVICES];     // 8 ���� ind=2-ind=9   �������� [MAX_DEVICES] �������� �����������
    int16_t pvRH;                 // 2 ���� ind=10;ind=11 �������� ������� ������������� ���������
    int16_t pvCO2[3];             // 6 ���� ind=12-ind=17 �������� ������� CO2 ---------- ����� 18 bytes ------------
    uint8_t pvTimer;              // 1 ���� ind=18        �������� �������
    uint8_t pvTmrCount;           // 1 ���� ind=19        �� ������������ !!!!!!!!!!!!!!!!!!!!!!!!!
    uint8_t pvFlap;               // 1 ���� ind=20        ��������� �������� 
    int8_t power;                 // 1 ���� ind=21        �������� ���������� �� ����
    uint8_t fuses;                // 1 ���� ind=22        �������� ��������� 
    uint8_t errors, warning;      // 2 ���� ind=23;ind=24 ������ � ��������������
    uint8_t cost0, cost1;         // 2 ���� ind=25;ind=26 ������� ��������
    uint8_t date, hours;          // 2 ���� ind=27;ind=28 �������� ����� � �����
    uint8_t other0;               // 1 ���� ind=29        �� ������������ !!!!!!!!!!!!!!!!!!!!!!!!!
  } pv;// ------------------ ����� 30 bytes -------------------------------
} upv;

uint32_t summCurr=0;

/* ----------------------------- BEGIN EEPROM ------------------------------------------- */
union serialdata{
  uint8_t data[EEP_DATA];
  struct eeprom {
    int16_t spT[2];     // 4 ���� ind=0-ind=3   ������� ����������� sp[0].spT->����� ������; sp[1].spT->������� ������
    int16_t spRH[2];    // 4 ���� ind=4-ind=7   sp[0].spRH->���������� HIH-5030; sp[1].spRH->������� ��������� ������ HIH-5030
    int16_t K[2];       // 4 ���� ind=8-ind=11  ���������������� �����.
    int16_t Ti[2];      // 4 ���� ind=12-ind=15 ������������ �����.
    int16_t minRun;     // 2 ���� ind=16;ind=17 ���������� ���������� ������� �����������
    int16_t maxRun;     // 2 ���� ind=18;ind=19 �� ������������ !!!!!!!!!!!!!!!!!!!!!!!!!
    int16_t period;     // 2 ���� ind=20;ind=21 ���������� ���������� ������� �����������
    int16_t TimeOut;    // 2 ���� ind=22;ind=23 ����� �������� ������ ������ ����������
    int16_t EnergyMeter;// 2 ���� ind=24;ind=25 ������� ������������ �������  ----------- ����� 26 bytes ------------
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
    uint8_t coolOn;     // 1 ���� ind=47;       �� ������������ !!!!!!!!!!!!!!!!!!!!!!!!!
    uint8_t coolOff;    // 1 ���� ind=48;       �� ������������ !!!!!!!!!!!!!!!!!!!!!!!!!
    uint8_t Zonality;   // 1 ���� ind=49;       ����� ����������� � ������ ----------- 24 bytes ------------
  } sp;                 // ------------------ ����� 50 bytes -------------------------------
} eep;

/* ------------------------------ END EEPROM -------------------------------------------- */
#include "output.h"
#include "displ.h"
void dsplMss(uint8_t *data, struct rampv *ram);
void temperature_check(struct rampv *ram);
void am2301_port_init(void);
uint8_t am2301_Start(void);
uint8_t am2301_Read(struct rampv *ram, uint8_t biasHum);
void display(struct eeprom *t, struct rampv *ram);
void display_setup(struct eeprom *t);
void display_servis(struct rampv *ram);
void displ_3(int16_t val, int8_t mode, int8_t blink);
void rotate_trays(uint8_t timer0, uint8_t timer1, struct rampv *ram);
uint8_t sethorizon(uint8_t timer0, uint8_t TurnTime, struct rampv *ram);
int8_t chkflap(uint8_t cmd, uint8_t *pvF);
uint8_t readCO2(struct rampv *ram);
//------
void saveservis(struct eeprom *t);
void saveset(struct eeprom *t);
uint8_t reset(uint16_t memAddr, uint8_t *data);
void checkkey(struct eeprom *t, int16_t pvT0);
//void pushkey(void);
void chkdoor(struct eeprom *t, struct rampv *ram);
void init(struct eeprom *t, struct rampv *ram);
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_SPI2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  int8_t tmpbyte;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  sysTick_Init();   // 1mS
  bluetoothData.TXBuffer[0] = 0x0D;  bluetoothData.TXBuffer[1] = 0x0A;  // "\r\n"
  eepMem.eepAddr = (0x50 << 1); // HAL expects address to be shifted one bit to the left
  eepMem.sizeAddr = I2C_MEMADD_SIZE_8BIT;
  eepMem.pageSize = 16;          // AT24C04A ��� AT24C08A. The 4K/8K EEPROM is capable of 16-byte page writes
  portOut.value = 0;
  portFlag.value = 0;
  topOwner=MAXOWNER;
  topUser=TOPUSER;
  botUser=BOTUSER;
  CHECK = 1;
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_SPI2_Init();
  MX_USART1_UART_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start_IT(&htim4);	/* ------  ������ 1��.   ������ 1000 ��.  ----*/
	HAL_TIM_Base_Start_IT(&htim3);	/* ------  ������ 6��.   ������ 166 ��.  ----*/
  HAL_ADCEx_Calibration_Start(&hadc1);            // ���������a ���
  HAL_UART_Receive_IT(&huart1,(uint8_t*)bluetoothData.RXBuffer,2);
//  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);  // LED Off
  //--------- �������� ���������� ��� ������� ������ ������������� ��������� --------
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc, 2);  // �������� ���
  while(flag==0);
  flag = 0;
  //----------- ������ ������ ��������. ��������� ���� ������ -----------------------
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc, 2);  // �������� ���
  while(flag==0);
  flag = 0;
  currAdc = adc[0]; humAdc = adc[1];
  adc[0] = 0; adc[1] = 0;

  for(int8_t i=0;i<8;i++) {setChar(i,SIMBL_BL); PointOn(i);}// BL+�����
  SendDataTM1638();
	SendCmdTM1638(0x8F);       // Transmit the display control command to set maximum brightness (8FH)
  
  eep_read(0x0000, eep.data);
  if (eep.sp.identif == 0 || eep.sp.identif > 30) eep_initial(0x0000, eep.data);
  
  bluetoothName();
  init(&eep.sp, &upv.pv);   // �������������
  temperature_check(&upv.pv);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  getButton = waitkey/4;
	while (1)
	{
  //----------------------------------- ������ ����� ���������� ��� ������ �� ����� ��� � ������� DMA� -------------------------------------------

  /* ------------------------------------------- BEGIN ������ TIM3 6 ��. ----------------------------------------------------------------------- */
      if (getButton>waitkey/4) checkkey(&eep.sp, upv.pv.pvT[0]);  // 
//      if (getButton>waitkey/4) pushkey();
  /* -------------------------------------------- END ������ TIM3 6 ��. ------------------------------------------------------------------------ */
  /* ------------------------------------------- BEGIN ������ TIM4 1 ��. ----------------------------------------------------------------------- */
      if(CHECK){   // ------- ����� ������� --------------------------------------------------------------
        CHECK=0; DISPLAY=1; ALARM=0; upv.pv.errors=0; upv.pv.warning=0; upv.pv.pvTmrCount = countsec;
        HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc, 2);
        while(flag==0);
        flag = 0;
        currAdc = adcTomV(adc[0]);      // Channel 8 (Port B0) � ��.
        humAdc  = adcTomV(adc[1]);      // Channel (Port B1) 9 � ��.
        adc[0] = 0; adc[1] = 0;
// ----------- ************************************** --------------------------------------------------
        temperature_check(&upv.pv);
        if(AM2301) am2301_Read(&upv.pv, eep.sp.spRH[0]);
        else if(HIH5030){ 
           if(humAdc > 500){                          // humAdc => 500 mV ��� RH=0%
              upv.pv.pvRH = mVToRH(humAdc, eep.sp.spRH[0], upv.pv.pvT[0]);  // ������� � ���������� �������� ������������� ��������� (%)
           }
           else upv.pv.pvRH = 255;
        }
  //---------------------- ��������� ������; "���������� � ����������"; "���������� � ���������" --------------------------------------------------
        chkdoor(&eep.sp, &upv.pv);
        if((eep.sp.condition&0x18)==0x08) eep.sp.condition|= sethorizon(eep.sp.timer[0], eep.sp.TurnTime, &upv.pv);  // ��������� � �������������� ���������
  //------------------------------------------ ����� ������� ���������� ---------------------------------------------------------------------------
        if(modules&1) {
          tmpbyte = chkcooler(eep.sp.condition);
          if(tmpbyte<0) upv.pv.errors |= 0x40;        // ����� ������ �����
          else if(tmpbyte>0) upv.pv.warning |= 0x20;  // ������� ����������� �����������
        }
        if(modules&2) {
          tmpbyte = chkhorizon(eep.sp.condition);
          if(tmpbyte<0) upv.pv.errors |= 0x40; // ����� ������ ��������
          else if(tmpbyte>0) upv.pv.warning |= 0x40;  // ��� �������� ������
        }
        if(modules&4) {
          tmpbyte = readCO2(&upv.pv);
          if(tmpbyte<0) upv.pv.errors |= 0x20;        // ����� ������ CO2
        }
        if(modules&8) {
          tmpbyte = chkflap(0,&upv.pv.pvFlap);
          if(tmpbyte<0) upv.pv.errors |= 0x20;        // ����� ������ ��������
        }
  //------------------------------------------ ������ �������� � ������ ---------------------------------------------------------------------------
        if (eep.sp.condition&1){
          // --------------------------------------- ����������� -----------------------------------------------------------------
          if(HAL_GPIO_ReadPin(OVERHEAT_GPIO_Port, OVERHEAT_Pin)==GPIO_PIN_RESET) upv.pv.errors |= 0x10;  // �������� ��������� !!!
          else if((upv.pv.warning&0x20)==0){    // ��� �������� ����������� ����������� !!!
            if(upv.pv.pvT[0] < 850){
              int16_t err = eep.sp.spT[0] - upv.pv.pvT[0];
              if(heatCondition(err, eep.sp.alarm[0], eep.sp.extOn[0])) upv.pv.warning |= 0x01;
              pwTriac0 = heater(err, &eep.sp);
              if(pwTriac0) HEATER = 1;  // HEATER On
            }
            else upv.pv.errors |= 0x01;   // ������ ������� ����������� !!!
          }
          upv.pv.power = pwTriac0 / 10;
          if(upv.pv.power > 100) upv.pv.power = 100; else if(upv.pv.power < 0) upv.pv.power = 0;
          // --------------------------------------- ����������� -----------------------------------------------------------------
          if(ok0&1){  // ���������� ���������� ��� ��������� � ��������������
            if(HIH5030||AM2301){  // ��������� ����������� ������ ���������
              int16_t err = eep.sp.spRH[1] - upv.pv.pvRH;
              if(humCondition(err, eep.sp.alarm[1], eep.sp.extOn[1])) upv.pv.warning |= 0x02;
              // �������� ����� ������  0-���; 1->�� ���.[0] 2->�� ���.[1] 3->�� ���.[0]&[1] 4->�� ���.[1] ���������� �����
              if(eep.sp.relayMode&4) valRun = humidifier(err, &eep.sp);
              else {
                pwTriac1 = humidifier(err, &eep.sp);
                if(pwTriac1 && (upv.pv.fuses&0x01)==0) HUMIDI = 1;  // HUMIDIFIER On
              }
            }
            else if(upv.pv.pvT[1] < 850){
              int16_t err = eep.sp.spT[1] - upv.pv.pvT[1];
              if(humCondition(err, eep.sp.alarm[1], eep.sp.extOn[1])) upv.pv.warning |= 0x02;
              // �������� ����� ������  0-���; 1->�� ���.[0] 2->�� ���.[1] 3->�� ���.[0]&[1] 4->�� ���.[1] ���������� �����
              if(eep.sp.relayMode&4) valRun = humidifier(err, &eep.sp);
              else {
                pwTriac1 = humidifier(err, &eep.sp);
                if(pwTriac1 && (upv.pv.fuses&0x01)==0) HUMIDI = 1;  // HUMIDIFIER On
              }
            }
            else upv.pv.errors |= 0x02;   // ������ ������� ��������� !!!
          }
          // --------------------------------------- ���������� -----------------------------------------------------------------
          if(upv.pv.warning&0x20) tmpbyte = ON;     // ������� ����������� �����������
          else {
            if(upv.pv.pvT[0] < 850){
              int16_t err = eep.sp.spT[0] - upv.pv.pvT[0];
              tmpbyte = RelayNeg(err, 0, eep.sp.extOn[0],eep.sp.extOff[0]);// ���. ����� -> ����������
            }
            else upv.pv.errors |= 0x01;             // ������ ������� ����������� !!!
          }
          if(upv.pv.fuses&0x02) tmpbyte = OFF;      // �������������� ���. ����� �1
          switch (tmpbyte){
            case ON:  FLAP = ON;  upv.pv.pvFlap = FLAPOPEN;  if(modules&8) chkflap(SETFLAP,  &upv.pv.pvFlap); break;// ��������� ��������
            case OFF: FLAP = OFF; upv.pv.pvFlap = FLAPCLOSE; if(modules&8) chkflap(DATAREAD, &upv.pv.pvFlap); break;// ��������� ��������; ����� ����� ���������� �������������� ������ ����
          }
          // --------------------------------------- ��������������� -----------------------------------------------------------------
          extra_2(&eep.sp, &upv.pv);
  //---------------------------------------- ����������� ����������� ������ -----------------------------------------------------------------------
            if (ok0){
                if(HIH5030||AM2301) {if(ds18b20_amount>1) {if(abs(upv.pv.pvT[0]-upv.pv.pvT[1])>eep.sp.Zonality) upv.pv.warning |=0x08;}} // ������� ������� ����������.
                else     {if(ds18b20_amount>2) {if(abs(upv.pv.pvT[0]-upv.pv.pvT[2])>eep.sp.Zonality) upv.pv.warning |=0x08;}};// ������� ������� ����������.
            }
            if(!(HIH5030||AM2301) && (upv.pv.pvT[1]-upv.pv.pvT[0])>20) {upv.pv.warning =0x10; pwTriac0 = 500;}					// ������������ ������������ �������� !!
  //--------------------------------------- ������� ������ ���������� ������ ----------------------------------------------------------------------
            if(eep.sp.KoffCurr){
              currAdc *= eep.sp.KoffCurr; currAdc /= 100;                 // ��������� mV � mA
              summCurr += currAdc;                                        // ������������ ���� ��������� ������ �������
              if(upv.pv.power==100 && countsec>=0 && currAdc<1000) upv.pv.errors|=0x08;  // ���� ��� < 1,0 �. -> ���������� ���� ����������� 
            }
            // ������ ��������� �������� (����� � ���� ����� ����� ������� ��������)
            int16_t newErr = abs(eep.sp.spT[0]-upv.pv.pvT[0]);
            if((upv.pv.warning & 3)&&(newErr-alarmErr)>2) disableBeep=0;  // ���� ��� ��������� ������ ���������� ������������ ������ ����� ����������
            if(countsec>59){
                countsec=0; if (disableBeep) disableBeep--;
              if(!(eep.sp.condition&0x18)) rotate_trays(eep.sp.timer[0], eep.sp.timer[1], &upv.pv);  // ����������� ������ ���� // ������ ���. // ������� ������ ��� ����������� ������
//              if(upv.pv.pvCO2[0]>0) CO2_check(eep.sp.spCO2, eep.sp.spCO2, upv.pv.pvCO2[0]); // �������� ������������ ��2
              else if(eep.sp.air[1]>0) aeration_check(eep.sp.air[0], eep.sp.air[1]);    // ������������� ����������� ������ ���� air[1]>0
              statPw[0]/=60; statPw[1]/=60;     // ������ ������ ��������
              upv.pv.cost0=statF2(0, statPw[1]); upv.pv.cost1=statF2(1, statPw[1]); // ������ ������ ��������
              statPw[0]=0; statPw[1]=0;         // ������ ������ ��������
              summCurr *= 220;
              eep.sp.EnergyMeter += (summCurr/60);// ������ ��.������� �� ������
              summCurr = 0;
            } 
        }
  //---------------------------------------------- ������ ��������� -------------------------------------------------------------------------------
        else if((eep.sp.condition&7)==0){
//            if(eep.sp.relayMode == 4) valRun = 0;                       // ��������� ���������� ���������� ������������
            if(servis){                                                   // ������� ��������� �����
              switch (servis){
                 case 1: pwTriac0=255; portOut.value = 0x01; break;       // �����������
                 case 2: pwTriac1=255; portOut.value = 0x02; break;       // �����������
                 case 3: portOut.value = 0x04; upv.pv.pvFlap = FLAPOPEN; if(modules&8) chkflap(SETFLAP, &upv.pv.pvFlap); break; // �������������, ����������� 90���.
                 case 4: portOut.value = 0x08; break;                     // �������������� �����
                 case 5: portOut.value = 0x20; break;                     // ����� �����
                 default: portOut.value = 0; upv.pv.pvFlap=FLAPCLOSE;
                          if(modules&8) chkflap(DATAREAD, &upv.pv.pvFlap);// ��� ���������, ����������� 0���.
              }
            }
            else {
               upv.pv.power=OFF; portOut.value = OFF; upv.pv.pvFlap=FLAPCLOSE; if(modules&8) chkflap(DATAREAD, &upv.pv.pvFlap); VENTIL = OFF;// CARBON = OFF;
               if(currAdc>1000){upv.pv.errors|=0x04;}   // ���� ���� ���� > 1000 mV ������ ���������!
  //--------------------------------------- ������� ������ ��� ����������� ������ !!! ----------------------------------------------------------
               if(countsec>59){countsec=0; if(eep.sp.condition&0x80) rotate_trays(eep.sp.timer[0], eep.sp.timer[1], &upv.pv);}
  //--------------------------------------------------------------------------------------------------------------------------------------------
            }
        }
        if(waitset){
          if(--waitset==0) {if(EEPSAVE) eep_write(0x0000, eep.data); servis=0;setup=0;displmode=0;psword=0;buf=0;topUser=TOPUSER;botUser=BOTUSER;}// ������������ � ��������� ������, ����� ������ 
        }
        if(TURN && eep.sp.timer[1]){if(--upv.pv.pvTimer==0) { upv.pv.pvTimer=eep.sp.timer[0]; TURN = OFF;}} // ������ ��� sp[1].timer>0 -> ������������ �����
        // -------------------------------------------------------------------------------------------
        if(HAL_GPIO_ReadPin(KEY_S2_GPIO_Port, KEY_S2_Pin)==GPIO_PIN_RESET){if (++show > 4) show = 0;}
        dsplMss(eep.data, &upv.pv);
        // -------------------------------------------------------------------------------------------
        HAL_UART_Transmit(&huart1,(uint8_t*)upv.pvdata,30,0x1000);
        HAL_UART_Transmit(&huart1,(uint8_t*)eep.data,50,0x1000);
        HAL_UART_Transmit(&huart1,(uint8_t*)bluetoothData.TXBuffer,2,0x1000);
      }
      /* ---------------- ��������� ------------------------------------------------- */
      if(DISPLAY){
        if(setup) display_setup(&eep.sp);
        else if(servis) display_servis(&upv.pv);
        else display(&eep.sp, &upv.pv);
        ledOut(eep.sp.condition, upv.pv.fuses); SendDataTM1638(); set_Output();
      }
  /* -------------------------------------------- END ������ TIM4 1 ��. ------------------------------------------------------------------------ */
//      if(rx_buffer_overflow) check_command();      // ���� ���������� ���� - "�������� ����������"
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
      if(pwTriac0 < 0) {pwTriac0=0; HEATER = 0; LEDOFF = 1;}  // HEATER Off
      if(pwTriac1 < 0) {pwTriac1=0; HUMIDI = 0; LEDOFF = 1;}  // HUMIDIFIER Off
      if(eep.sp.relayMode == 4){                              // ���������� ����� ������ ������
        if(pulsPeriod < 0){
          pulsPeriod = eep.sp.period;
          pwTriac1=valRun;
          if(pwTriac1 && (upv.pv.fuses&0x01)==0) HUMIDI = 1;  // HUMIDIFIER On
        }
      }
      if(LEDOFF) {LEDOFF = 0; ledOut(eep.sp.condition, upv.pv.fuses); SendDataTM1638(); set_Output();}
      if(beepOn < 0) {beepOn=0; HAL_GPIO_WritePin(Beeper_GPIO_Port, Beeper_Pin, GPIO_PIN_RESET);}  // Beeper Off
      if(bluetoothData.ind == 0)  bluetoothData.timeOut=0; 
      else if(bluetoothData.timeOut >= 10) {
        // ������ �������� ������ 10 mS.
        HAL_UART_AbortReceive_IT(&huart1); // ��������� ������
        bluetoothData.ind = 0; // ������� �������� ������� �����
        bluetoothData.timeOut = 0;
        HAL_UART_Receive_IT(&huart1,(uint8_t*)bluetoothData.RXBuffer,2); // ������ ������
      }
	} // ---------------------------------- END while (1) ------------------------------------------------------------------------------------------
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Common config 
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel 
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel 
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 400000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_HARD_OUTPUT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_1LINE;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_LSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 2999;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 2399;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 23999;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 2999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, OUT_RCK_Pin|DISPL_STB_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DE485_GPIO_Port, DE485_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(Beeper_GPIO_Port, Beeper_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED2_Pin */
  GPIO_InitStruct.Pin = LED2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(LED2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : KEY_S2_Pin */
  GPIO_InitStruct.Pin = KEY_S2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(KEY_S2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PA1 PA2 PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : OUT_RCK_Pin DISPL_STB_Pin */
  GPIO_InitStruct.Pin = OUT_RCK_Pin|DISPL_STB_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : Bluetooth_STATE_Pin */
  GPIO_InitStruct.Pin = Bluetooth_STATE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(Bluetooth_STATE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DE485_Pin */
  GPIO_InitStruct.Pin = DE485_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(DE485_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : AM2301_Pin */
  GPIO_InitStruct.Pin = AM2301_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(AM2301_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Beeper_Pin */
  GPIO_InitStruct.Pin = Beeper_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Beeper_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Door_Pin OVERHEAT_Pin OneW_2R_Pin OneWR_1_Pin */
  GPIO_InitStruct.Pin = Door_Pin|OVERHEAT_Pin|OneW_2R_Pin|OneWR_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){  
  if(hadc->Instance == ADC1) flag = 1;
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
  if(huart==&huart1) bluetoothCallback();
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
