#ifndef _GAMMA_H_
#define _GAMMA_H_ 1

#include "settings.h"
#include "Filter.h"

SETTING* FLT_Gamma_GetSetting(FLT_GAMMA_SETTING Setting);
void FLT_Gamma_ReadSettingsFromIni();
void FLT_Gamma_WriteSettingsToIni();



#endif /* _NOISE_H_ */