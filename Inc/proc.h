#ifndef __PROC_H
#define __PROC_H

void checkSensor(void);
//uint8_t tableRH(signed int maxT, signed int minT)
void CO2_check(uint16_t spCO20, uint16_t spCO21, uint16_t pvCO20);
void aeration_check(uint8_t air0, uint8_t air1);
uint16_t ratioCurr(uint16_t curr, uint8_t KoffCurr);
uint8_t statF2(uint8_t n, uint16_t statPw);
uint8_t fan_adc(uint16_t val, uint8_t coolOn, uint8_t coolOff);
void fan_power(uint8_t coolOn, uint8_t coolOff, uint8_t power);

#endif /* __PROC_H */

