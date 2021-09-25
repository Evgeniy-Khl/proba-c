#ifndef LCD_H_
#define LCD_H_
/*
* LCD_RS - H/L register select signal ���� / �������
* LCD_RW - H/L read/write signal
* LCD_EN - H --> L enable signal      ����� ������ / ������
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

#define e1    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_SET)   // ��������� ����� E � 1
#define e0    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_RESET) // ��������� ����� E � 0
#define rs1   HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET)   // ��������� ����� RS � 1 (������)
#define rs0   HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET) // ��������� ����� RS � 0 (�������)
 
#endif /* LCD_H_ */
