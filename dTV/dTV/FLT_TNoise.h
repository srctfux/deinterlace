#ifndef _NOISE_H_
#define _NOISE_H_ 1

#include "settings.h"
#include "deinterlace.h"

// Get Hold of the FD_50Hz.c file settings
SETTING* FLT_TNoise_GetSetting(FLT_TNOISE_SETTING Setting);
void FLT_TNoise_ReadSettingsFromIni();
void FLT_TNoise_WriteSettingsToIni();
void FLT_TNoise_SetMenu(HMENU hMenu);

DEINTERLACE_FUNC NoiseFilter_Temporal;

#endif /* _NOISE_H_ */