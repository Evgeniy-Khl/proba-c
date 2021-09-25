#ifndef LCD_H_
#define LCD_H_
/*
* LCD_RS - H/L register select signal дата / команда
* LCD_RW - H/L read/write signal
* LCD_EN - H --> L enable signal      строб записи / чтения
* DB0-DB7   - H/L data bus line
*
*/
#define d4_set()   HAL_GPIO_WritePin(D0_GPIO_Port, D0_Pin, GPIO_PIN_SET)
#define d5_set()   HAL_GPIO_WritePin(D1_GPIO_Port, D1_Pin, GPIO_PIN_SET)
#define d6_set()   HAL_GPIO_WritePin(D2_GPIO_Port, D2_Pin, GPIO_PIN_SET)
#define d7_set()   HAL_GPIO_WritePin(D3_GPIO_Port, D3_Pin, GPIO_PIN_SET)
#define d4_reset() HAL_GPIO_WritePin(D0_GPIO_Port, D0_Pin, GPIO_PIN_RESET)
#define d5_reset() HAL_GPIO_WritePin(D1_GPIO_Port, D1_Pin, GPIO_PIN_RESET)
#define d6_reset() HAL_GPIO_WritePin(D2_GPIO_Port, D2_Pin, GPIO_PIN_RESET)
#define d7_reset() HAL_GPIO_WritePin(D3_GPIO_Port, D3_Pin, GPIO_PIN_RESET)

#define e1    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_SET)   // установка линии E в 1
#define e0    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_RESET) // установка линии E в 0
#define rs1   HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET)   // установка линии RS в 1 (данные)
#define rs0   HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET) // установка линии RS в 0 (команда)
 
#endif /* LCD_H_ */
