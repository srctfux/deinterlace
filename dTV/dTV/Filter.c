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
#include "settings.h"

long NumFilters = 0;
BOOL TNoiseFilterOn;
BOOL GammaFilterOn;

FILTER_METHOD* Filters[100] = {NULL,};

void Filter_DoInput(DEINTERLACE_INFO *pInfo, BOOL HurryUp)
{
	int i;
	for(i = 0; i < NumFilters; i++)
	{
		if(Filters[i]->bActive && Filters[i]->bOnInput)
		{
			if(!HurryUp || Filters[i]->bAlwaysRun)
			{
				Filters[i]->pfnAlgorithm(pInfo);
			}
		}
	}
}

void Filter_DoOutput(DEINTERLACE_INFO *pInfo, BOOL HurryUp)
{
	int i;
	for(i = 0; i < NumFilters; i++)
	{
		if(Filters[i]->bActive && !Filters[i]->bOnInput)
		{
			if(!HurryUp || Filters[i]->bAlwaysRun)
			{
				Filters[i]->pfnAlgorithm(pInfo);
			}
		}
	}
}

BOOL LoadFilterPlugins()
{
	return FALSE;
}

void UnloadFilterPlugins()
{
}


BOOL TNoiseFilter_OnChange(long NewValue)
{
	// TODO
	return FALSE;
}

BOOL GammaFilter_OnChange(long NewValue)
{
	// TODO
	return FALSE;
}

BOOL ProcessFilterSelection(HWND hWnd, WORD wMenuID)
{
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING FilterSettings[FILTER_SETTING_LASTONE] =
{
	{
		"Noise Filter", ONOFF, 0, &TNoiseFilterOn,
		FALSE, 0, 1, 1, 1,
		NULL,
		"NoiseFilter", "UseTemporalNoiseFilter", TNoiseFilter_OnChange,
	},
	{
		"Gamma Filter", ONOFF, 0, &GammaFilterOn,
		FALSE, 0, 1, 1, 1,
		NULL,
		"GammaFilter", "UseGammaFilter", GammaFilter_OnChange,
	},
};

SETTING* Filter_GetSetting(long nIndex, FILTER_SETTING Setting)
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

LONG Filter_HandleSettingsMsg(HWND hWnd, UINT message, UINT wParam, LONG lParam, BOOL* bDone)
{
	return 0;
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
	for(i = 0; i < NumFilters; i++)
	{
		CheckMenuItem(hMenu, Filters[i]->MenuId, Filters[i]->bActive ? MF_CHECKED : MF_UNCHECKED);
	}
}

