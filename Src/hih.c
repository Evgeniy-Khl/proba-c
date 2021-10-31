#include "main.h"
#include "hih.h"

#define VSUPPLY   	3200

float PVold1, PVold2;
extern const float A1, A2, A3;

// ��� HIH-5030
// Vout=(Vsupply)*(0.00636*(sensor RH) + 0.1515), typical at 25 ���.C
// True RH = (Sensor RH)/(1.0546 - 0.00216*T), T in ���.C
uint16_t ValDcToRH(int16_t Vadc, int16_t spRH, int16_t pvT0){
// Vadc �������� �������� ADC -> � ���������� �������� ������������� ��������� (%)
 float tmpRH, tmpK;
 int16_t retVal;
  tmpRH =(float)A1*PVold1-A2*PVold2+A3*Vadc;
  PVold2 = PVold1;
  PVold1 = tmpRH;
  tmpRH = (float)tmpRH/VSUPPLY;
  tmpRH -= 0.1515; tmpRH /= 0.00636; tmpRH *= 10.0;
  if(pvT0<850){tmpK = 0.00216 * pvT0/10; tmpK = 1.0546 - tmpK;}// �������� �� �����������
  else tmpK=1;
  retVal = tmpRH / tmpK;
  retVal += spRH;             //sp[0].spRH->���������� HIH-5030!!
  if (retVal>1000) retVal=999; else if (retVal<0) retVal=0;
  return tmpRH;
}
