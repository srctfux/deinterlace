/////////////////////////////////////////////////////////////////////////////
// Filter.c
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 03 Feb 2001   John Adcock           Initial Version
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Filter.h"

DEINTERLACE_FUNC NoiseFilter_Temporal;
DEINTERLACE_FUNC Filter_Gamma;

FILTER_METHOD Filters[FILTERS_LAST_ONE] = 
{
	{
		"Gamma Filter",
		FALSE,
		TRUE,
		Filter_Gamma,
		IDM_GAMMA_FILTER,
	},
	{
		"Temporal Noise Filter",
		FALSE,
		TRUE,
		NoiseFilter_Temporal,
		IDM_NOISE_FILTER,
	},
};

void Filter_DoInput(DEINTERLACE_INFO *pInfo)
{
	int i;
	for(i = 0; i < FILTERS_LAST_ONE; i++)
	{
		if(Filters[i].bActive && Filters[i].bOnInput)
		{
			Filters[i].pfnAlgorithm(pInfo);
		}
	}
}

void Filter_DoOutput(DEINTERLACE_INFO *pInfo)
{
	int i;
	for(i = 0; i < FILTERS_LAST_ONE; i++)
	{
		if(Filters[i].bActive && !Filters[i].bOnInput)
		{
			Filters[i].pfnAlgorithm(pInfo);
		}
	}
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING FilterSettings[FILTER_SETTING_LASTONE] =
{
	{
		"Noise Filter", ONOFF, 0, &Filters[TEMPORAL_NOISE].bActive,
		FALSE, 0, 1, 1, 1,
		NULL,
		"NoiseFilter", "UseTemporalNoiseFilter", NULL,
	},
	{
		"Gamma Filter", ONOFF, 0, &Filters[GAMMA].bActive,
		FALSE, 0, 1, 1, 1,
		NULL,
		"GammaFilter", "UseGammaFilter", NULL,
	},
};

SETTING* Filter_GetSetting(FILTER_SETTING Setting)
{
	if(Setting > -1 && Setting < FILTER_SETTING_LASTONE)
	{
		return &(FilterSettings[Setting]);
	}
	else
	{
		return NULL;
	}
}

void Filter_ReadSettingsFromIni()
{
	int i;
	for(i = 0; i < FILTER_SETTING_LASTONE; i++)
	{
		Setting_ReadFromIni(&(FilterSettings[i]));
	}
}


void Filter_WriteSettingsToIni()
{
	int i;
	for(i = 0; i < FILTER_SETTING_LASTONE; i++)
	{
		Setting_WriteToIni(&(FilterSettings[i]));
	}
}

void Filter_SetMenu(HMENU hMenu)
{
	int i;
	for(i = 0; i < FILTERS_LAST_ONE; i++)
	{
		CheckMenuItem(hMenu, Filters[i].MenuId, Filters[i].bActive ? MF_CHECKED : MF_UNCHECKED);
	}
}

