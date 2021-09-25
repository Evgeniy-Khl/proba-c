#include "main.h"
#include "global.h"   // ����� ���������� ��������� eeprom � ��������� rampv
#include "ds18b20.h"

//--------------------------------------------------
uint8_t LastDeviceFlag;
uint8_t LastDiscrepancy;
uint8_t LastFamilyDiscrepancy;
uint8_t ROM_NO[8];

extern uint8_t familycode[8][8];
extern int8_t ds18b20_amount;
extern int16_t pvRH;

//--------------------------------------------------
__STATIC_INLINE void DelayMicro(__IO uint32_t micros){
micros *= (SystemCoreClock / 1000000) / 8;
/* Wait till done */
while (micros--) ;
}
//--------------------------------------------------
void ds18b20_port_init(void){
  HAL_GPIO_DeInit(GPIOB, GPIO_PIN_9);
  GPIOB->CRH |= GPIO_CRH_MODE9;
  GPIOB->CRH |= GPIO_CRH_CNF9_0;
  GPIOB->CRH &= ~GPIO_CRH_CNF9_1;
}
//-----------------------------------------------
uint8_t ds18b20_SearhRom(uint8_t *Addr){
  uint8_t id_bit_number;
  uint8_t last_zero, rom_byte_number, search_result;
  uint8_t id_bit, cmp_id_bit;
  uint8_t rom_byte_mask, search_direction;
  //����������������� ����������
  id_bit_number = 1;
  last_zero = 0;
  rom_byte_number = 0;
  rom_byte_mask = 1;
  search_result = 0;
	if (!LastDeviceFlag){
		ds18b20_Reset();
		ds18b20_WriteByte(0xF0);
	}
	do{
		id_bit = ds18b20_ReadBit();
		cmp_id_bit = ds18b20_ReadBit();
		if ((id_bit == 1) && (cmp_id_bit == 1))	break;
		else{
			if (id_bit != cmp_id_bit)
				search_direction = id_bit; // bit write value for search
			else{
				if (id_bit_number < LastDiscrepancy)
					search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
				else
					search_direction = (id_bit_number == LastDiscrepancy);
				if (search_direction == 0){
					last_zero = id_bit_number;
					if (last_zero < 9) LastFamilyDiscrepancy = last_zero;
				}
			}
			if (search_direction == 1) ROM_NO[rom_byte_number] |= rom_byte_mask;
			else ROM_NO[rom_byte_number] &= ~rom_byte_mask;
			ds18b20_WriteBit(search_direction);
			id_bit_number++;
			rom_byte_mask <<= 1;
			if (rom_byte_mask == 0){
				rom_byte_number++;
				rom_byte_mask = 1;
			}
		}
  } while(rom_byte_number < 8); // ��������� ����� � 0 �� 7 � �����
	if (!(id_bit_number < 65)){
	  // search successful so set LastDiscrepancy,LastDeviceFlag,search_result
		LastDiscrepancy = last_zero;
		// check for last device
		if (LastDiscrepancy == 0)	LastDeviceFlag = 1;
		search_result = 1;	
  }
	if (!search_result || !ROM_NO[0]){
		LastDiscrepancy = 0;
		LastDeviceFlag = 0;
		LastFamilyDiscrepancy = 0;
		search_result = 0;
	}
	else{
    for (int i = 0; i < 8; i++) Addr[i] = ROM_NO[i];
  }	
  return search_result;
}
//-----------------------------------------------
uint8_t ds18b20_Reset(void){
  uint8_t status;
  GPIOB->BSRR =GPIO_BSRR_BR9;//������ ������� (�������� 11 ��� ����� B ) ?????????????????????????????????????
  DelayMicro(485);//�������� ��� ������� �� 480 �����������
  GPIOB->BSRR =GPIO_BSRR_BS9;//������� ������� (���������� 11 ��� ����� B ) ?????????????????????????????????????
  DelayMicro(65);//�������� ��� ������� �� 60 �����������
  status = (GPIOB->IDR & GPIO_IDR_IDR9 ? 1 : 0);//��������� ������� ?????????????????????????????????????
  DelayMicro(500);//�������� ��� ������� �� 480 �����������
  //(�� ������ ������ ������� ��������, ��� ��� ����� ���� ���������� � ��������)
  return (status ? 1 : 0);//����� ���������
}
//--------------------------------------------------
uint8_t ds18b20_ReadBit(void){
  uint8_t bit = 0;
  GPIOB->BSRR =GPIO_BSRR_BR9;//������ �������
  DelayMicro(2);
  GPIOB->BSRR =GPIO_BSRR_BS9;//������� �������
  DelayMicro(13);
  bit = (GPIOB->IDR & GPIO_IDR_IDR9 ? 1 : 0);//��������� �������
  DelayMicro(45);
  return bit;
}
//-----------------------------------------------
uint8_t ds18b20_ReadByte(void){
  uint8_t data = 0;
  for (uint8_t i = 0; i < 8; i++)
  data += ds18b20_ReadBit() << i;
  return data;
}
//-----------------------------------------------
void ds18b20_WriteBit(uint8_t bit){
  GPIOB->BSRR =GPIO_BSRR_BR9;//������ �������
  DelayMicro(bit ? 3 : 65);
  GPIOB->BSRR =GPIO_BSRR_BS9;//������� �������
  DelayMicro(bit ? 65 : 3);
}
//-----------------------------------------------
void ds18b20_WriteByte(uint8_t dt){
  for (uint8_t i = 0; i < 8; i++){
    ds18b20_WriteBit(dt >> i & 1);
    //Delay Protection
    DelayMicro(5);
  }
}
//-----------------------------------------------
uint8_t ds18b20_count(uint8_t amount){
  uint8_t i, dt[8];
  ds18b20_amount = 0;
  for(i = 0; i < amount; i++){
    if(ds18b20_SearhRom(dt)){
      memcpy(familycode[ds18b20_amount],dt,sizeof(dt));
      ds18b20_amount++;
    }
    else break;
  }
  return 0;
}
//-----------------------------------------------
void ds18b20_Convert_T(void){
  ds18b20_Reset();
  ds18b20_WriteByte(0xCC);  //SKIP ROM
  ds18b20_WriteByte(0x44);  //CONVERT T
}
//----------------------------------------------------------
//void ds18b20_ReadStratcpad(uint16_t y_pos, uint8_t DevNum){
//  uint8_t i, crc, dt[8];
//  ds18b20_Reset();            // 1 Wire Bus initialization
//	ds18b20_WriteByte(0x55);  // Load MATCH ROM [55H] comand
//	for(i = 0; i < 8; i++){ds18b20_WriteByte(familycode[DevNum][i]);}
//  ds18b20_WriteByte(0xBE);  //READ SCRATCHPAD
//  for(i=0;i<8;i++){dt[i] = ds18b20_ReadByte();}
//  crc = ds18b20_ReadByte(); // Read CRC byte
//  i = dallas_crc8(dt, 8);
////  if (i==crc){
////    sprintf(buffTFT,"PAD %02X %02X %02X %02X %02X %02X %02X %02X",
////       dt[0], dt[1], dt[2], dt[3], dt[4], dt[5], dt[6], dt[7]);
////    ILI9341_WriteString(5, y_pos, buffTFT, Font_11x18, ILI9341_WHITE, ILI9341_BLACK);
////  }
//}
//----------------------------------------------------------
void ds18b20_WriteScratchpad(uint8_t DevNum, uint8_t th, int8_t tl)   // ������ � configuration register
{
 uint8_t i;
  ds18b20_Reset();          // 1 Wire Bus initialization
  ds18b20_WriteByte(0x55);  // Load MATCH ROM [55H] comand
  for(i = 0; i < 8; i++){
			ds18b20_WriteByte(familycode[DevNum][i]);
		}
  ds18b20_WriteByte(0x4E);  // Load WRITE SCRATCHPAD [4EH] command
  ds18b20_WriteByte(th);    // TH
  ds18b20_WriteByte(tl);    // TL
  ds18b20_WriteByte(0x7F);  // configuration register
  ds18b20_Reset();          // 1 Wire Bus initialization
  ds18b20_WriteByte(0xCC);  // Load Skip ROM [CCH] command
  ds18b20_WriteByte(0x48);  // Load COPY SCRATCHPAD [48h] command
}
//----------------------------------------------------------
uint8_t dallas_crc8(uint8_t * data, uint8_t size){
 uint8_t crc = 0;
  for (uint8_t i = 0; i < size; ++i){
    uint8_t inbyte = data[i];
    for (uint8_t j = 0; j < 8; ++j){
      uint8_t mix = (crc ^ inbyte) & 0x01;
      crc >>= 1;
      if ( mix ) crc ^= 0x8C;
      inbyte >>= 1;
    }
  }
  return crc;
}
//----------------------------------------------------------
/*
  Name  : CRC-16 CCITT
  Poly  : 0x1021    x^16 + x^12 + x^5 + 1
  Init  : 0xFFFF
  Revert: false
  XorOut: 0x0000
  Check : 0x29B1 ("123456789")
  MaxLen: 4095 ���� (32767 ���) - �����������
    ���������, �������, ������� � ���� �������� ������
*/
uint16_t CRC16(uint8_t *pcBlock, uint16_t len){
  uint16_t crc = 0xFFFF;
  uint8_t i;
  while (len--){
    crc ^= *pcBlock++ << 8;
    for (i = 0; i < 8; i++)
        crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
  }
  return crc;
}
//----------------------------------------------------------
uint8_t ds18b20_GetSign(uint16_t dt){
  //�������� 11-� ���
  if (dt&(1<<11)) return 1;
  else return 0;
}
//----------------------------------------------------------
float ds18b20_Convert(uint16_t dt){
  float t;
  t = (float) ((dt&0x07FF)>>4); //��������� �������� � ������� ����
  //�������� ������� �����
  t += (float)(dt&0x000F) / 16.0f;
  return t;
}

//----------------------------------------------------------
void temperature_check(struct rampv *ram){
 int16_t correction;
 uint8_t item, byte, crc, try_cnt=0;
 union ds {uint8_t data[8]; int16_t val;} buffer;
  for (item=0; item < ds18b20_amount;){
    ds18b20_Reset(); // 1 Wire Bus initialization
    ds18b20_WriteByte(0x55); // Load MATCH ROM [55H] comand
    for (byte=0; byte < 8; byte++) ds18b20_WriteByte(familycode[item][byte]); // Load cont. byte
    ds18b20_WriteByte(0xBE); // Read Scratchpad command [BE]
    for (byte=0; byte < 8; byte++){
        buffer.data[byte] = ds18b20_ReadByte(); // Read cont. byt
    }
    crc = ds18b20_ReadByte(); // Read CRC byte
    byte = dallas_crc8(buffer.data, 8);
    if (byte==crc){
      try_cnt = 0;
      if (buffer.val<0){
        buffer.val = -buffer.val;
        ram->pvT[item] = buffer.val*10/16;
        ram->pvT[item] = -ram->pvT[item];
      }
      else ram->pvT[item] =  buffer.val*10/16;
      crc = buffer.data[2]&TUNING;
      if (crc==TUNING){correction = buffer.data[3]; ram->pvT[item] +=correction;}// �������� ��������� �������
    }
    else if (++try_cnt > 2) {try_cnt = 0; ram->pvT[item] = 1990;};// (199) ���� ������ ����� X ��� �� ������ �� ���������� 
    if (try_cnt==0) item++;
   }
  ds18b20_Convert_T();
}

void checkSensor(void){
 uint8_t item, byte, crc, try_cnt=0, tempbuffer[9];
 union ds {uint8_t data[8]; int16_t val;} buffer;
    for (item=0; item < ds18b20_amount;){
    ds18b20_Reset(); // 1 Wire Bus initialization
    ds18b20_WriteByte(0x55); // Load MATCH ROM [55H] comand
    for (byte=0; byte < 8; byte++) ds18b20_WriteByte(familycode[item][byte]); // Load cont. byte
    ds18b20_WriteByte(0xBE); // Read Scratchpad command [BE]
    for (byte=0; byte < 8; byte++){
        buffer.data[byte] = ds18b20_ReadByte(); // Read cont. byt
    }
    crc = ds18b20_ReadByte(); // Read CRC byte
    byte = dallas_crc8(buffer.data, 8);
    if (byte==crc){
      try_cnt = 0;
      if(buffer.data[2]==SENSOREGG){  // ���� ������ ����������� �������� ����
 //---------------------------------------------------------------------------------
        if(item < 2){                 // � ���� ������ ������ ����� ���������� �� ������ ������!
          for (byte=0; byte<9; byte++) tempbuffer[byte] = familycode[2][byte];// familycode ������ ������ ��������� �� ��������� �����
          for (byte=0; byte<9; byte++) familycode[2][byte] = familycode[item][byte];// familycode ������� ����������� �������� ���� ��������� �� ������ ������
          for (byte=0; byte<9; byte++) familycode[item][byte] = tempbuffer[byte];// familycode ������� ����������� � ������ ������ ��������� �� �������������� �����.
        }
        return;                       // ������ ����� ���� ������ �����
 //---------------------------------------------------------------------------------        
      }
    }
    else if (++try_cnt > 2) try_cnt = 0;     // ���� ������ ����� X ��� �� ������ �� ����������
    if (try_cnt==0) item++;
   }
}
