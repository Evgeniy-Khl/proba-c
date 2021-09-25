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
#define VREF_mlV   	3200
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
void UART1_RxCpltCallback(void);
void bluetoothCheck(void);
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

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
const float A4=1.8, A5=0.81, A6=0.01;  // порядок a=0.9 (A1=2a; A2=a^2; A3=(1-a)^2)
const float A1=1.6, A2=0.64, A3=0.04;  // порядок a=0.8 (A1=2a; A2=a^2; A3=(1-a)^2)
volatile uint16_t adc[3] = {0,};       // у нас три канала АЦП, поэтому массив из трех элементов
volatile uint8_t flag = 0;             // флаг окончания преобразования АЦП
int8_t getButton=0;
uint8_t changeDispl=0, x=0, y=0, power=0, show=5, Hih=0, Alarm=0, Aeration, Carbon, Check=1, Thermistor, Superheat, EEPsave=0, keynum=0;
uint8_t DisplBuffer[17]={0}, keyBuffer[4]={0};
uint8_t RXBuffer[10] = {0}, TXBuffer[2] = {0x0D,0x0A};
volatile uint32_t cnt1;
const uint8_t digit[] = 
{
  0x3F, // 0
  0x06, // 1
  0x5B, // 2
  0x4F, // 3
  0x66, // 4
  0x6D, // 5
  0x7D, // 6
  0x07, // 7
  0x7F, // 8
  0x6F, // 9
  0x77, // A
  0x7C, // B
  0x39, // C
  0x5E, // d
  0x79, // E
  0x71, // F
  0x76, // H
  0x78, // t
  0x73, // P
  0x63, // TOPo
  0x37, // П
  0x23, // TOPn
  0x54, // n
  0x5C, // o
  0x58, // c
  0x62, // TOPu
  0x1C, // u
  0x40, // MINUS Mid
  0x01, // M_Top
  0x08  // M_Bott
};

uint16_t eepAddr = (0x50 << 1); // HAL expects address to be shifted one bit to the left
uint16_t sizeAddr = I2C_MEMADD_SIZE_8BIT;
uint8_t pageSize = 16;          // AT24C04A или AT24C08A. The 4K/8K EEPROM is capable of 16-byte page writes
char SSDBuffer[20];
// -------- ISIDA ------
int8_t countsec=0, displmode, sizeX;
uint8_t pwTriac0, pwTriac1, portOut, valRun, valLoad, Superheat, setup, psword, servis, keydata, waitset, waitkey=WAITCOUNT;
uint8_t countmin, countcooler, pvVCool, pvThermistor, cmdmodule, modules=0;
uint8_t outLed, beepOn, disableBeep, pvAeration, topOwner=MAXOWNER, topUser=TOPUSER, botUser=BOTUSER;
int8_t ds18b20_amount, ext[4];
uint8_t ok0, ok1, familycode[MAX_DEVICES][9], outbuffer[4], inbuffer[4];
int16_t alarmErr, buf, current, currAdc, thermistorAdc, RHadc; 
int16_t kWattHore, statPw[2];  // (?? байт)
uint32_t summCurr=0;
float PVold1=80, PVold2, iPart[3], stold[2][2];
/* ----------------------------- BEGIN point value -------------------------------------- */
union pointvalue{
  uint8_t pvdata[28];
  struct rampv {
    uint8_t cellID;               // 1 байт ind=0         сетевой номер прибора
    int16_t pvT[MAX_DEVICES];     // 8 байт ind=1-ind=8   значения [MAX_DEVICES] датчиков температуры
    int16_t pvRH;                 // 2 байт ind=9;ind=10  значение датчика относительной влажности
    int16_t pvCO2[3];             // 6 байт ind=11-ind=16 значения датчика CO2 ---------- ИТОГО 17 bytes ------------
    uint8_t pvTimer;              // 1 байт ind=17        значение таймера
    uint8_t pvTmrCount;           // 1 байт ind=18        значение счетчика проходов поворота
    uint8_t pvFlap;               // 1 байт ind=19        положение заслонки 
    uint8_t power, fuses;         // 2 байт ind=20;ind=21 мощьность подаваемая на тены и короткие замыкания
    uint8_t errors, warning;      // 2 байт ind=22;ind=23 ошибки и предупреждения
    uint8_t cost0, cost1;         // 2 байт ind=24;ind=25 затраты ресурсов
    uint8_t date, hours;          // 2 байт ind=26;ind=27 счетчики суток и часов
  } pv;// ------------------ ИТОГО 28 bytes -------------------------------
} upv;
/*
ext[0] модуль Холла
ext[1] модуль ГОРИЗОНТА
ext[2] модуль СО2
ext[3] модуль заслонок
*/

/* ----------------------------- BEGIN EEPROM ------------------------------------------- */
union serialdata{
  uint8_t data[EEP_DATA];
  struct eeprom {
    int16_t spT[2];     // 4 байт ind=0-ind=3   Уставка температуры sp[0].spT->Сухой датчик; sp[1].spT->Влажный датчик
    int16_t spRH[2];    // 4 байт ind=4-ind=7   sp[0].spRH->ПОДСТРОЙКА HIH-5030; sp[1].spRH->Уставка влажности Датчик HIH-5030
    int16_t K[2];       // 4 байт ind=8-ind=11  пропорциональный коэфф.
    int16_t Ti[2];      // 4 байт ind=12-ind=15 интегральный коэфф.
    int16_t minRun;     // 2 байт ind=16;ind=17 импульсное управление насосом увлажнителя
    int16_t maxRun;     // 2 байт ind=18;ind=19 импульсное управление насосом увлажнителя
    int16_t period;     // 2 байт ind=20;ind=21 импульсное управление насосом увлажнителя
    int16_t TimeOut;    // 2 байт ind=22;ind=23 время ожидания начала режима охлаждения
    int16_t EnergyMeter;// 2 байт ind=24;ind=25 счетчик элктрической энергии  ----------- ИТОГО 26 bytes ------------
    int8_t timer[2];    // 2 байт ind=26;ind=27 [0]-отключ.состояниe [1]-включ.состояниe
    int8_t alarm[2];    // 2 байт ind=28;ind=29 дельта 5 = 0.5 гр.C
    int8_t extOn[2];    // 2 байт ind=30;ind=31 смещение для ВКЛ. вспомогательного канала
    int8_t extOff[2];   // 2 байт ind=32;ind=33 смещение для ОТКЛ. вспомогательного канала
    uint8_t air[2];     // 2 байт ind=34;ind=35 таймер проветривания air[0]-пауза; air[1]-работа; если air[1]=0-ОТКЛЮЧЕНО
    uint8_t spCO2;      // 1 байт ind=36;       опорное значение для управления концетрацией СО2
    uint8_t identif;    // 1 байт ind=37;       сетевой номер прибора
    uint8_t state;      // 1 байт ind=38;       состояние камеры (ОТКЛ. ВКЛ. ОХЛАЖДЕНИЕ, и т.д.)
    uint8_t extendMode; // 1 байт ind=39;       расширенный режим работы  0-СИРЕНА; 1-ВЕНТ. 2-Форс НАГР. 3-Форс ОХЛЖД. 4-Форс ОСУШ. 5-Дубляж увлажнения
    uint8_t relayMode;  // 1 байт ind=40;       релейный режим работы  0-НЕТ; 1->по кан.[0] 2->по кан.[1] 3->по кан.[0]&[1]
    uint8_t programm;   // 1 байт ind=41;       работа по программе
    uint8_t Hysteresis; // 1 байт ind=42;       Гистерезис канала увлажнения
    uint8_t ForceHeat;  // 1 байт ind=43;       Форсированный нагрев 5 = 0.5 грд.С.
    uint8_t TurnTime;   // 1 байт ind=44;       время ожидания прохода лотков в секундах
    uint8_t HihEnable;  // 1 байт ind=45;       разрешение использования датчика влажности
    uint8_t KoffCurr;   // 1 байт ind=46;       маштабный коэф. по току симистора  (160 для AC1010 или 80 для другого)
    uint8_t coolOn;     // 1 байт ind=47;       порог включения вентилятора обдува сисмистора coolOn=80~>65 грд.С.
    uint8_t coolOff;    // 1 байт ind=48;       порог отключения вентилятора обдува сисмистора
    uint8_t Zonality;   // 1 байт ind=49;       порог зональности в камере ----------- 24 bytes ------------
  } sp;                 // ------------------ ИТОГО 50 bytes -------------------------------
} eep;
//struct eeprom *p_sp = &eep.sp;
//union serialdata *serData = &eep;
/* ------------------------------ END EEPROM -------------------------------------------- */
void dsplMss(uint8_t *data, struct rampv *ram);
void temperature_check(struct rampv *ram);
void display_servis(struct rampv *ram);
void rotate_trays(uint8_t timer0, uint8_t timer1, struct rampv *ram);
uint16_t powerCurr(uint16_t curr, uint8_t KoffCurr, struct rampv *ram);
uint8_t sethorizon(uint8_t timer0, uint8_t TurnTime, struct rampv *ram);
int8_t chkflap(uint8_t cmd, uint8_t *pvF);
uint8_t readCO2(struct rampv *ram);
//------
void display(struct eeprom *t, struct rampv *ram);
void display_setup(struct eeprom *t);
void saveservis(struct eeprom *t);
void saveset(struct eeprom *t);
uint8_t reset(uint16_t memAddr, uint8_t *data);
void displ_3(int16_t val, int8_t mode);
void checkkey(struct eeprom *t, int16_t pvT0);
void heat_wet(struct eeprom *t);
void extra_1(struct eeprom *t);
void extra_2(struct eeprom *t);
void chkdoor(struct eeprom *t, struct rampv *ram);
void alarm(struct eeprom *t, int16_t pvT0);
void init0(uint8_t KoffCurr);
void init1(struct eeprom *t, struct rampv *ram);
void leddisplay(uint8_t state);
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
static void MX_SPI1_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_USART1_UART_Init(void);
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
  MX_SPI1_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start_IT(&htim4);	/* ------  таймер 1Гц.   период 1000 мс.  ----*/
	HAL_TIM_Base_Start_IT(&htim3);	/* ------  таймер 6Гц.   период 166 мс.  ----*/
	HAL_TIM_Base_Start_IT(&htim2);	/* ------  таймер 200Гцю период 5 мс.  ----*/
  HAL_ADCEx_Calibration_Start(&hadc1);            // калибровкa АЦП
  HAL_UART_Receive_IT(&huart1,(uint8_t*)RXBuffer,1);
  //--------- ХОЛОСТОЕ ВЫПОЛНЕНИЕ при котором каналы распологаются правильно --------
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc, 3);  // стартуем АЦП
  while(flag==0);
  flag = 0;
  //----------- Теперь каналы сдвинуты. Последний стал первым -----------------------
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc, 3);  // стартуем АЦП
  cnt1=0;
  while(flag==0);
  flag = 0;
  currAdc = adc[0]; thermistorAdc = adc[1]; RHadc = adc[2]*VREF_mlV/4095;
  adc[0] = 0; adc[1] = 0; adc[2] = 0;

  for(int8_t i=0;i<8;i++) {setChar(i,SIMBL_BL); PointOn(i);}// BL+точки
//  setChar(0,currAdc/1000); setChar(1,currAdc/100); setChar(2,currAdc/10); setChar(3,currAdc%10);
//  setChar(4,thermistorAdc/1000); setChar(5,thermistorAdc/100); setChar(6,thermistorAdc/10); setChar(7,thermistorAdc%10);
  SendDataTM1638();
	SendCmdTM1638(0x8F);       // Transmit the display control command to set maximum brightness (8FH)
//  dsplMss(eep.data);
//  HAL_Delay(5000);
  eep_read(0x0000, eep.data);
  if (eep.sp.identif == 0 || eep.sp.identif == 255) eep_initial(0x0000, eep.data);
  init0(eep.sp.KoffCurr);   // инициализация 0
  init1(&eep.sp, &upv.pv);  // инициализация 1
  temperature_check(&upv.pv);
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
  bluetoothCheck();
  HAL_Delay(5000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  getButton = waitkey/4;
	while (1)
	{
  //----------------------------------- Теперь будем опрашивать три канала на одном АЦП с помощью DMA… -------------------------------------------

  /* ------------------------------------------- BEGIN таймер TIM3 6 Гц. ----------------------------------------------------------------------- */
      if (getButton>waitkey/4) {checkkey(&eep.sp, upv.pv.pvT[0]); /*HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc, 3);*/}
  /* -------------------------------------------- END таймер TIM3 6 Гц. ------------------------------------------------------------------------ */
  /* ------------------------------------------- BEGIN таймер TIM4 1 Гц. ----------------------------------------------------------------------- */
      if (Check){   // ------- новая секунда
        Check=0; Alarm=0; upv.pv.errors=0; upv.pv.warning=0; countsec++; upv.pv.pvTmrCount = countsec;
        HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc, 3);
        while(flag==0);
        flag = 0;
        currAdc = adc[0];         // Channel 2  currAdc
        thermistorAdc = adc[1];   // Channel 3  thermistorAdc
        RHadc = adc[2]*VREF_mlV/4095;        // Channel 4  RHadc
        adc[0] = 0; adc[1] = 0; adc[2] = 0;
// ----------- ************************************** --------------------------------------------------
        temperature_check(&upv.pv);
        HAL_UART_Transmit(&huart1,(uint8_t*)upv.pvdata,28,0x1000);
        HAL_UART_Transmit(&huart1,(uint8_t*)TXBuffer,2,0x1000);
// ----------- ************************************** --------------------------------------------------
        if(Thermistor) pvThermistor = fan_adc(thermistorAdc, eep.sp.coolOn, eep.sp.coolOff);// температура силового элемента (включение вентилятора)
        else fan_power(80, 60, upv.pv.power);
        if(Hih){ 
           if(RHadc > 400){                             // RHadc => 500 mV для RH=0%
              upv.pv.pvRH = ValDcToRH(RHadc, eep.sp.spRH[0], upv.pv.pvT[0]);  // перевод в десятичное значение относительной влажности (%)
           }
           else upv.pv.pvRH = 255;
        }
  //---------------------- Состояние дверей; "подгототка к ОХЛАЖДЕНИЮ"; "подгототка к ВКЛЮЧЕНИЮ" --------------------------------------------------
      chkdoor(&eep.sp, &upv.pv);
      if((eep.sp.state&0x18)==0x08) eep.sp.state|= sethorizon(eep.sp.timer[0], eep.sp.TurnTime, &upv.pv);  // Установка в горизонтальное положение
  //------------------------------------------ Опрос модулей расширения ---------------------------------------------------------------------------
      if(modules&1) {
        tmpbyte = chkcooler(eep.sp.state);
        if(tmpbyte<0) upv.pv.errors |= 0x40; // Отказ модуля Холла
        ext[0] = tmpbyte;
      } else ext[0] = -1;  // НЕПОДКЛЮЧЕН !
      if(modules&2) {
        tmpbyte = chkhorizon(eep.sp.state);
        if(tmpbyte<0) upv.pv.errors |= 0x40; // Отказ модуля ГОРИЗОНТ
        ext[1] = tmpbyte;
      } else ext[1] = -1;  // НЕПОДКЛЮЧЕН !
      if(modules&4) {
        tmpbyte = readCO2(&upv.pv);
        if(tmpbyte<0) upv.pv.errors |= 0x20; // Отказ модуля CO2
        ext[2] = tmpbyte;
      } else ext[2] = -1;  // НЕПОДКЛЮЧЕН !
      if(modules&8) {
        tmpbyte = chkflap(0,&upv.pv.pvFlap);
        ext[3] = tmpbyte;
        if(tmpbyte<0) upv.pv.errors |= 0x20; // Отказ модуля CO2
      } else ext[3] = -1;  // НЕПОДКЛЮЧЕН !
  //------------------------------------------ КАМЕРА ВКЛЮЧЕНА в работу ---------------------------------------------------------------------------
        if (eep.sp.state&1){
            heat_wet(&eep.sp);  // (Симистор; Симистор)
            extra_1(&eep.sp);
            extra_2(&eep.sp);
  //---------------------------------------- ЗОНАЛЬНОСТЬ температуры камеры -----------------------------------------------------------------------
            if (ok0){
                if (Hih) {if(ds18b20_amount>1) {if(abs(upv.pv.pvT[0]-upv.pv.pvT[1])>eep.sp.Zonality) upv.pv.warning |=0x08;}} // Большой перепад температур.
                else     {if(ds18b20_amount>2) {if(abs(upv.pv.pvT[0]-upv.pv.pvT[2])>eep.sp.Zonality) upv.pv.warning |=0x08;}};// Большой перепад температур.
            }
            if(!Hih && (upv.pv.pvT[1]-upv.pv.pvT[0])>20) {upv.pv.warning =0x10; pwTriac0 = 50;}					// Неправильная конфигурация датчиков !!
  //--------------------------------------- ПОВОРОТ ЛОТКОВ Статистика камеры ----------------------------------------------------------------------
            current = ratioCurr(currAdc, eep.sp.KoffCurr);      // сила тока симистора
//            summCurr += powerCurr(current, eep.sp.KoffCurr);  // суммирование тока симистора
            alarm(&eep.sp, upv.pv.pvT[0]);                      // АНАЛИЗ АВАРИЙНОЙ СИТУАЦИИ (важно в этом месте после анализа мощности)
            if(countsec>59){
                countsec=0; if (disableBeep) disableBeep--;
                if(!(eep.sp.state&0x18)) rotate_trays(eep.sp.timer[0], eep.sp.timer[1], &upv.pv);  // выполняется только если // Камера ВКЛ. // Поворот лотков при ОТКЛЮЧЕННОЙ камере
                if(upv.pv.pvCO2[0]>0) CO2_check(eep.sp.spCO2, eep.sp.spCO2, upv.pv.pvCO2[0]); // Проверка концентрации СО2
                else if(eep.sp.air[1]>0) aeration_check(eep.sp.air[0], eep.sp.air[1]);    // Проветривание выполняется только если air[1]>0
                statPw[0]/=60; statPw[1]/=60;    // расчет затрат ресурсов
                upv.pv.cost0=statF2(0, statPw[1]); upv.pv.cost1=statF2(1, statPw[1]); // расчет затрат ресурсов
                statPw[0]=0; statPw[1]=0;        // расчет затрат ресурсов
                if(++countmin>59) {countmin=0; summCurr*=11; eep.sp.EnergyMeter+=(summCurr/180000); kWattHore=eep.sp.EnergyMeter; summCurr=0;}// (summCurr*220)/(3600*1000)
            } 
        }
  //---------------------------------------------- КАМЕРА ОТКЛЮЧЕНА -------------------------------------------------------------------------------
        else if((eep.sp.state&7)==0){
//            if(eep.sp.relayMode == 4) valRun = 0;                       // ОТКЛЮЧИТЬ импульсное управление увлажнителем
            if(servis){                                                   // включен СЕРВИСНЫЙ режим
              current = ratioCurr(currAdc, eep.sp.KoffCurr);              // сила тока симистора
              switch (servis){
                 case 1: pwTriac0=255; portOut = 0x01; break;             // НАГРЕВ
                 case 2: pwTriac1=255; portOut = 0x02; break;             // УВЛАЖНИТЕЛЬ
                 case 3: portOut = 0x04; upv.pv.pvFlap = FLAPOPEN; if(modules&8) chkflap(SETFLAP, &upv.pv.pvFlap); break; // ПРОВЕТРИВАНИЕ, СЕРВОПРИВОД 90грд.
                 case 4: portOut = 0x08; break;                           // ДОПОЛНИТЕЛЬНЫЙ КАНАЛ
                 case 5: portOut = 0x20; break;                           // ЛОТКИ ВВЕРХ
                 default: portOut = 0; upv.pv.pvFlap=FLAPCLOSE;
                          if(modules&8) chkflap(DATAREAD, &upv.pv.pvFlap);// ВСЕ ОТКЛЮЧЕНО, СЕРВОПРИВОД 0грд.
              }
              leddisplay(eep.sp.state); SendDataTM1638();                 // Светодиодная индикация
            }
            else {
               power=OFF; ALL_OFF(portOut); upv.pv.pvFlap=FLAPCLOSE; if(modules&8) chkflap(DATAREAD, &upv.pv.pvFlap); Aeration=OFF; Carbon=OFF;
               current=ratioCurr(currAdc, eep.sp.KoffCurr);       // сила тока симистора когда блок ОТКЛЮЧЕН !
               if(current>10){upv.pv.errors|=0x04; beepOn=DURATION*2;}   // если сила тока > 1А ПРОБОЙ СИМИСТОРА!
  //--------------------------------------- Поворот лотков при ОТКЛЮЧЕННОЙ камере !!! ----------------------------------------------------------
               if(countsec>59){countsec=0; if(eep.sp.state&0x80) rotate_trays(eep.sp.timer[0], eep.sp.timer[1], &upv.pv);}
  //--------------------------------------------------------------------------------------------------------------------------------------------
            }
        }
        if(waitset){
          if(--waitset==0) {if(EEPsave) eep_write(0x0000, eep.data); servis=0;setup=0;displmode=0;psword=0;buf=0;topUser=TOPUSER;botUser=BOTUSER;}// возвращяемся к основному экрану, сброс пароля 
        }
        if(TURN_IN(portOut) && eep.sp.timer[1]){if(--upv.pv.pvTimer==0) { upv.pv.pvTimer=eep.sp.timer[0]; TURN_OFF(portOut);}} // только при sp[1].timer>0 -> асиметричный режим
        /* ---------------- ИНДИКАЦИЯ ------------------------------------------------- */
        if(setup) display_setup(&eep.sp);
        else if(servis) display_servis(&upv.pv);
        else display(&eep.sp, &upv.pv);
        // -------------------------------------------------------------------------------------------
        if(HAL_GPIO_ReadPin(KEY_S2_GPIO_Port, KEY_S2_Pin)==GPIO_PIN_RESET){if (++show > 5) show = 1;}
        dsplMss(eep.data, &upv.pv);
//        cnt1=0;
        // -------------------------------------------------------------------------------------------
      }
  /* -------------------------------------------- END таймер TIM4 1 Гц. ------------------------------------------------------------------------ */
//      if(rx_buffer_overflow) check_command();      // если установлен флаг - "комманда компьютера"
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
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
  hadc1.Init.NbrOfConversion = 3;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel 
  */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel 
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel 
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
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
  hspi1.Init.Direction = SPI_DIRECTION_1LINE;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_LSB;
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
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 1499;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 239;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

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
  htim3.Init.Prescaler = 3999;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 2999;
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
  HAL_GPIO_WritePin(DISPL_STB_GPIO_Port, DISPL_STB_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : LED2_Pin */
  GPIO_InitStruct.Pin = LED2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(LED2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : KEY_S2_Pin */
  GPIO_InitStruct.Pin = KEY_S2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(KEY_S2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DISPL_STB_Pin */
  GPIO_InitStruct.Pin = DISPL_STB_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DISPL_STB_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : OneWr2_Pin OneWR_Pin */
  GPIO_InitStruct.Pin = OneWr2_Pin|OneWR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){  
  cnt1++;
  if(hadc->Instance == ADC1) flag = 1;
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
  if(huart==&huart1){
//    UART1_RxCpltCallback();
    HAL_UART_Receive_IT(&huart1,(uint8_t*)RXBuffer,2);
  }
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
