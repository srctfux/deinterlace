/////////////////////////////////////////////////////////////////////////////
// FLT_TNoise.c
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

#include "stdafx.h"
#include "deinterlace.h"
#include "cpu.h"
#include "FLT_Gamma.h"

unsigned char GammaTable[256];

long Gamma = 1000;
BOOL bUseStoredTable = FALSE;

BOOL Filter_Gamma(DEINTERLACE_INFO *info)
{
	short *Pixels;
	short *Table;
	int y;
	int Cycles;

	// Need to have the current and next-to-previous fields to do the filtering.
	if ((info->IsOdd && info->OddLines[0] == NULL) ||
		(! info->IsOdd && info->EvenLines[0] == NULL))
	{
		return FALSE;
	}

	Cycles = info->LineLength / 4;

	Table = (short*)GammaTable;

	for (y = 0; y < info->FieldHeight; y++)
	{
		if (info->IsOdd)
		{
			Pixels = info->OddLines[0][y];
		}
		else
		{
			Pixels = info->EvenLines[0][y];
		}

		_asm
		{
			mov ecx, Cycles
			mov edx, dword ptr[Pixels]
			mov ebx, dword ptr[Table]
LOOP_LABEL:
			mov al,byte ptr[edx]
			xlatb
			//mov al, 127
			mov byte ptr[edx], al
			add edx, 2			
			mov al,byte ptr[edx]
			xlatb
			//mov al, 127
			mov byte ptr[edx], al
			add edx, 2			
			loop LOOP_LABEL
		}

	}
	return TRUE;
}

double GetGammaAdjustedValue(double Input, double Gamma)
{
	if(Input < 0.0812)
	{
		return Input * Gamma;
	}
	else
	{
		return pow((Input + 0.099) / 1.099, Gamma);
	}
}

BOOL Gamma_OnChange(long NewValue)
{
	int i;
	double AdjustedValue;

	Gamma = NewValue;
	for (i = 0;  i < 256; i++)
	{
		AdjustedValue = 255.0 * GetGammaAdjustedValue((double)(i) / 255.0, (double)Gamma / 1000.0);
		GammaTable[i] = (unsigned char)AdjustedValue;
	}
	return FALSE;
}

BOOL UseStoredTable_OnChange(long NewValue)
{
	char szEntry[10];
	int i;
	bUseStoredTable = NewValue;
	if(bUseStoredTable)
	{
		for(i = 0; i < 256; i++)
		{
			sprintf(szEntry, "%d", i);
			GammaTable[i] = (unsigned char)GetPrivateProfileInt("Gamma", szEntry, i, "Gamma.ini");
		}
		return FALSE;
	}
	else
	{
		return Gamma_OnChange(Gamma);
	}
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING FLT_GammaSettings[FLT_GAMMA_SETTING_LASTONE] =
{
	{
		"Gamma", SLIDER, 0, &Gamma,
		1300, 0, 3000, 10, 1000,
		NULL,
		"GammaFilter", "Gamma", Gamma_OnChange,
	},
	{
		"Use Stored Gamma Table", YESNO, 0, &bUseStoredTable,
		FALSE, 0, 1, 1, 1,
		NULL,
		"GammaFilter", "bUseStoredTable", UseStoredTable_OnChange,
	},
};

SETTING* FLT_Gamma_GetSetting(FLT_GAMMA_SETTING Setting)
{
	if(Setting > -1 && Setting < FLT_GAMMA_SETTING_LASTONE)
	{
		return &(FLT_GammaSettings[Setting]);
	}
	else
	{
		return NULL;
	}
}

void FLT_Gamma_ReadSettingsFromIni()
{
	int i;
	for(i = 0; i < FLT_GAMMA_SETTING_LASTONE; i++)
	{
		Setting_ReadFromIni(&(FLT_GammaSettings[i]));
	}
	Gamma_OnChange(Gamma);
	UseStoredTable_OnChange(bUseStoredTable);
}

void FLT_Gamma_WriteSettingsToIni()
{
	int i;
	for(i = 0; i < FLT_GAMMA_SETTING_LASTONE; i++)
	{
		Setting_WriteToIni(&(FLT_GammaSettings[i]));
	}
}

