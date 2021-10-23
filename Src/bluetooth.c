#include "main.h"

extern UART_HandleTypeDef huart1;

struct {
  uint8_t RXBuffer[2];
  uint8_t TXBuffer[2];
  uint8_t buf[60];
  uint8_t ind;
  uint8_t timeOut;
  uint8_t devOk;
} bluetoothData;


void data_parse(char* buf_str){
//  strcat(buf_str,"EV\r\n");
  HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
}

void bluetoothCallback(void){
  uint8_t first, second;
  first = bluetoothData.RXBuffer[0]; second = bluetoothData.RXBuffer[1];
  bluetoothData.timeOut = 0;
 //если вдруг случайно превысим длину буфера
  if (bluetoothData.ind>58){
    bluetoothData.ind=0;
    HAL_UART_Receive_IT(&huart1,(uint8_t*)bluetoothData.RXBuffer,2);
    return;
  }
  bluetoothData.buf[bluetoothData.ind] = first;
  bluetoothData.buf[bluetoothData.ind+1] = second;
 // -----------  "\r\n"  --------------------------------------
  if(first==0x0D && second == 0x0A && bluetoothData.devOk == 1){
    bluetoothData.buf[bluetoothData.ind+3]=0;
    
    data_parse((char*)bluetoothData.buf);
    
    bluetoothData.ind=0;
    HAL_UART_Receive_IT(&huart1,(uint8_t*)bluetoothData.RXBuffer,2);
  }
// ------------  "OK"  ----------------------------------------
  if(first==0x4F && second == 0x4B && bluetoothData.ind == 0){
    bluetoothData.devOk = 1;
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);  // LED On
  }
  bluetoothData.ind += 2;
  HAL_UART_Receive_IT(&huart1,(uint8_t*)bluetoothData.RXBuffer,2);
}

void bluetoothName(void){
 char str1[15];
 uint8_t sizeX;
  SSD1306_Fill(SSD1306_COLOR_BLACK); //clear oled
  sprintf(str1,"AT+NAMEIsida01");
  SSD1306_GotoXY(0,0);
  SSD1306_Puts(str1, &Font_7x10, SSD1306_COLOR_WHITE);
  SSD1306_UpdateScreen();  
  sizeX=strlen(str1);
  bluetoothData.ind = 0;
  HAL_UART_Transmit(&huart1, (uint8_t*)str1, sizeX, 0x1000);  // Передача команды str=[AT+NAME<>]
}

