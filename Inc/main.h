/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "fonts.h"
#include "ssd1306.h"
#include "TM1638.h"
#include "i2c.h"
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED2_Pin GPIO_PIN_13
#define LED2_GPIO_Port GPIOC
#define KEY_S2_Pin GPIO_PIN_0
#define KEY_S2_GPIO_Port GPIOA
#define SSD_SCL_Pin GPIO_PIN_10
#define SSD_SCL_GPIO_Port GPIOB
#define SSD_SDA_Pin GPIO_PIN_11
#define SSD_SDA_GPIO_Port GPIOB
#define DISPL_STB_Pin GPIO_PIN_14
#define DISPL_STB_GPIO_Port GPIOB
#define Blutooth_TX_Pin GPIO_PIN_9
#define Blutooth_TX_GPIO_Port GPIOA
#define Blutooth_RX_Pin GPIO_PIN_10
#define Blutooth_RX_GPIO_Port GPIOA
#define AM2301_Pin GPIO_PIN_12
#define AM2301_GPIO_Port GPIOA
#define Beeper_Pin GPIO_PIN_15
#define Beeper_GPIO_Port GPIOA
#define Door_Pin GPIO_PIN_3
#define Door_GPIO_Port GPIOB
#define mem_SCL_Pin GPIO_PIN_6
#define mem_SCL_GPIO_Port GPIOB
#define mem_SDA_Pin GPIO_PIN_7
#define mem_SDA_GPIO_Port GPIOB
#define OneWr2_Pin GPIO_PIN_8
#define OneWr2_GPIO_Port GPIOB
#define OneWR_Pin GPIO_PIN_9
#define OneWR_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
#define VERSION       1       // версия ПО
#define EEPROM_I2C_PORT hi2c1
#define EEP_DATA      50      // ВСЕГО 50 bytes
#define MAX_DEVICES   4       // максимальное количество микросхем DS18b20 подключенных 1 Wire шине
#define WAITCOUNT     16      // максимальная пауза перед реакцией на кнопку
#define MAXOWNER 	    12      // максимальная количество меню для пользователя
#define BOTUSER 	    16      // начальный пункт меню для специалиста
#define TOPUSER 	    18      // максимальная количество меню для специалиста кроме ШИМ
#define PULSMENU 	    21      // максимальная количество меню для специалиста включая ШИМ
#define BOTKOFF 	    26      // начальный пункт меню коэфициентов
#define TOPKOFF 	    31      // конечный пункт меню коэфициентов

#define ON          1
#define OFF         0
#define UNCHANGED   2
#define FLAPOPEN    100     // значение при котором зслонка полностью открыта
#define FLAPCLOSE   0       // значение при котором зслонка полностью закрыта
#define DATAREAD    0xA1    // Read Scratchpad
#define SETFLAP     0xA2    // аварийное полное открытие заслонки
#define SETHORIZON  0xA3    // команда установки в горизонт
#define NEW_TURN    0xA5    // команда установки проверки прохода поворота
#define TUNING      170
#define SENSOREGG   171
#define DURATION    80
#define COUNT       500

#define ID_HALL         0xF1    // идентификатор блока
#define ID_HORIZON      0xF3    // идентификатор блока
#define ID_CO2          0xF5    // идентификатор блока
#define ID_FLAP         0xF7    // идентификатор блока
#define RESETCMD        0xC1    // Generate Reset Pulse

#define CANAL1_ON(x)  x |= 1<<0  // симистор НАГРЕВАТЕЛЯ
#define CANAL1_OFF(x) x &= ~(1<<0)
#define CANAL1_IN(x)  x &= (1<<0)
#define CANAL2_ON(x)  x |= 1<<1  // реле или симистор УВЛАЖНЕНИЯ
#define CANAL2_OFF(x) x &= ~(1<<1)
#define CANAL2_IN(x)  x &= (1<<1)
#define EXT1_ON(x)    x |= 1<<2  // реле вспомогательного канала
#define EXT1_OFF(x)   x &= ~(1<<2)
#define EXT2_ON(x)    x |= 1<<3  // реле вспомогательного канала
#define EXT2_OFF(x)   x &= ~(1<<3)
#define TURN_ON(x)    x |= 1<<4  // поворот
#define TURN_OFF(x)   x &= ~(1<<4)
#define TURN_IN(x)    x &= (1<<4)
#define COOLER_ON(x)  x |= 1<<5  // вентилятор охладителя
#define COOLER_OFF(x) x &= ~(1<<5)  // вентилятор охладителя
#define ALL_OFF(x)    x &= ~((1<<0)|(1<<1)|(1<<2)|(1<<3))

#define SIMBL_A     10
#define SIMBL_B     11
#define SIMBL_C     12
#define SIMBL_d     13
#define SIMBL_E     14
#define SIMBL_F     15
#define SIMBL_H     16
#define SIMBL_t     17
#define SIMBL_P     18
#define SIMBL_TOPo  19
#define SIMBL_Pe    20
#define SIMBL_TOPn  21
#define SIMBL_n     22
#define SIMBL_o     23
#define SIMBL_c     24
#define SIMBL_TOPu  25
#define SIMBL_u     26
#define SIMBL_MINUS 27
#define SIMBL_M_Top 28
#define SIMBL_MBott 29
#define SIMBL_BL    30

#define DEN           20     // denominator - делитель для вывода на дисплей в секундах 0,0
#define MINRELAYMODE  0 // релейный режим работы  0-НЕТ; 1->по кан.[0] 2->по кан.[1] 3->по кан.[0]&[1] 4->ШИ по кан.[1]
#define MAXRELAYMODE  4
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
