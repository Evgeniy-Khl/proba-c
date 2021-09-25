#include "main.h"

// Передача команды + Возврат каретки (carriage return, CR) = (0x0D, '\r')
const char name_bt[]={"AT+NAME?\r"};    // AT+NAME?
const char version[]={"AT+VERSION?\r"}; // AT+VERSION?
const char addr_bt[]={"AT+ADDR?\r"};    // AT+ADDR?
const char field01[]={"t01="};        // поле 1
const char field02[]={"t02="};        // поле 2
const char field03[]={"t03="};        // поле 3 
extern char SSDBuffer[];
extern UART_HandleTypeDef huart1;

extern uint8_t RXBuffer[60], TXBuffer[60], sizeX;

typedef struct USART_prop{
  uint8_t usart_buf[60];
  uint8_t usart_cnt;
  uint8_t is_tcp_connect;//статус попытки создать соединение TCP с сервером
  uint8_t is_text;//статус попытки передать текст серверу
} USART_prop_ptr;
USART_prop_ptr usartprop;
/*
void UART1_Transmit(char *str)
{
  strcat(str,"\r");
  memcpy (TXBuffer, str, sizeof(*str));
  HAL_UART_Transmit(&huart1, TXBuffer, sizeof(TXBuffer), 100);
}

void putjson()
{
int8_t tmpv0, tmpv1;
 int16_t val;
  if(CONECT){
   putsf("{");
   putchar(0x22);putsf(field01);putchar(0x22);putchar(0x3A);
   val = t.point[0]; tmpv0 = val%10; tmpv1 = val/10;
   sprintf(buff,"%2i.%u",tmpv1,tmpv0);
   putchar(0x22);puts(buff);putchar(0x22);putchar(0x2C); putchar(0x0D);
   putchar(0x22);putsf(field02);putchar(0x22);putchar(0x3A);
   val = t.point[1]; tmpv0 = val%10; tmpv1 = val/10;
   sprintf(buff,"%2i.%u",tmpv1,tmpv0);
   putchar(0x22);puts(buff);putchar(0x22);
   putsf("}"); putchar(0x0D);
  }
}
*/
void string_parse(char* buf_str){
//  strcat(buf_str,"EV\r\n");
  HAL_UART_Transmit(&huart1,(uint8_t*)buf_str,strlen(buf_str),0x1000);
HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
}

void UART1_RxCpltCallback(void){
 uint8_t b;
 b = RXBuffer[0];
 //если вдруг случайно превысим длину буфера
  if (usartprop.usart_cnt>59){
    usartprop.usart_cnt=0;
    HAL_UART_Receive_IT(&huart1,(uint8_t*)RXBuffer,1);
    return;
  }
  usartprop.usart_buf[usartprop.usart_cnt] = b;
  if(b==0x0A){
    usartprop.usart_buf[usartprop.usart_cnt+1]=0;
    string_parse((char*)usartprop.usart_buf);
    usartprop.usart_cnt=0;
    HAL_UART_Receive_IT(&huart1,(uint8_t*)RXBuffer,1);
    return;
  }
  usartprop.usart_cnt++;
  HAL_UART_Receive_IT(&huart1,(uint8_t*)RXBuffer,1);
}

void bluetoothCheck(void){
 int8_t y;
 char str1[10];
  SSD1306_Fill(SSD1306_COLOR_BLACK); //clear oled
  y=0;
  
  sprintf(str1,"AT");
  
//  sprintf(SSDBuffer,"BLUETOOTH:"); 
  SSD1306_GotoXY(0,y);
  SSD1306_Puts(str1, &Font_7x10, SSD1306_COLOR_WHITE);
  SSD1306_UpdateScreen();
  y+=10;
  
  sizeX=strlen(str1);
//  memcpy (TXBuffer, name_bt, sizeX);
  HAL_UART_Transmit(&huart1, (uint8_t*)str1, sizeX, 100);  // Передача команды str=[AT+NAME?]
  SSD1306_GotoXY(0,y);
  sizeX=strlen((char*)RXBuffer);
  if(sizeX>0){
//  memcpy (SSDBuffer, usartprop.usart_buf, sizeX);
    SSD1306_Puts((char*)RXBuffer, &Font_7x10, SSD1306_COLOR_WHITE);
  }
  else {
    SSD1306_Puts("zero", &Font_7x10, SSD1306_COLOR_WHITE);
  }
  SSD1306_UpdateScreen();
  y+=10;
/*  

  if(!Bltooth_Ok){sprintf(buff,"Ошибка Bluetooth!"); TFT_DrawString(buff,15,170,1,1,RED,WHITE);}
  else TFT_DrawString(ptr_char,15,170,1,0,BLUE,WHITE);
  puttobltooth(version);  // Передача команды str=[AT+VERSION?]
  if(!Bltooth_Ok){sprintf(buff,"Ошибка Bluetooth!"); TFT_DrawString(buff,15,190,1,1,RED,WHITE);}
  else TFT_DrawString(ptr_char,15,190,1,0,BLUE,WHITE);
  puttobltooth(addr_bt);  // Передача команды str=[AT+ADDR?]
  if(!Bltooth_Ok){sprintf(buff,"Ошибка Bluetooth!"); TFT_DrawString(buff,15,210,1,1,RED,WHITE);}
  else TFT_DrawString(ptr_char,15,210,1,0,BLUE,WHITE);
*/
}

