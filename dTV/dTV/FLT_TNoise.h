#ifndef _NOISE_H_
#define _NOISE_H_ 1

#include "settings.h"
#include "Filter.h"

SETTING* FLT_TNoise_GetSetting(FLT_TNOISE_SETTING Setting);
void FLT_TNoise_ReadSettingsFromIni();
void FLT_TNoise_WriteSettingsToIni();

#endif /* _NOISE_H_ */