/////////////////////////////////////////////////////////////////////////////
// FD_50Hz.c
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock. All rights reserved.
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
// 02 Jan 2001   John Adcock           Made PAL pulldown detect remember last video
//                                     mode
//
// 07 Jan 2001   John Adcock           Fixed PAL detection bug
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 09 Jan 2001   John Adcock           Split out into new file
//                                     Changed functions to use DEINTERLACE_INFO
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OutThreads.h"
#include "FD_50Hz.h"
#include "FD_Common.h"
#define DOLOGGING
#include "DebugLog.h"
#include "DI_BobAndWeave.h"

ePULLDOWNMODES gPALFilmFallbackMode = VIDEO_MODE_2FRAME;
// Default values which can be overwritten by the INI file
long PulldownThresholdLow = -1500;
long PulldownThresholdHigh = 200;
long PALPulldownRepeatCount = 3;
long PALPulldownRepeatCount2 = 1;


///////////////////////////////////////////////////////////////////////////////
// UpdatePALPulldownMode
//
// This is the 2:2 pulldown detection for PAL.
///////////////////////////////////////////////////////////////////////////////
void UpdatePALPulldownMode(DEINTERLACE_INFO *pInfo)
{
	static long LastCombFactor;
	static long RepeatCount;
	static long LastPolarity;
	static long LastDiff;
	static DWORD StartFilmTicks = 0;

	// call with pInfo as NULL to reset static variables when we start the thread
	// each time
	if(pInfo == NULL)
	{
		LastCombFactor = 0;
		RepeatCount = 0;
		LastPolarity = -1;
		LastDiff = 0;
		UpdatePulldownStatus();
		ResetModeSwitches();
		StartFilmTicks = 0;
		return;
	}

	GetCombFactor(pInfo);
	if(!DeintMethods[gPulldownMode].bIsFilmMode)
	{
		if((pInfo->CombFactor - LastCombFactor) < PulldownThresholdLow && LastDiff > PulldownThresholdLow)
		{
			if(LastPolarity == pInfo->IsOdd)
			{
				if(RepeatCount < PALPulldownRepeatCount)
				{
					if(RepeatCount == 0)
					{
						RepeatCount = 1;
					}
					else
					{
						LOG(" Film Detect Gap %d", (GetTickCount() - StartFilmTicks));
						if((GetTickCount() - StartFilmTicks) < 100)
						{
							RepeatCount++;
							LOG(" Upped RepeatCount %d", RepeatCount);
						}
						else
						{
							RepeatCount = 1;					
							LOG(" Upped RepeatCount - Too long", RepeatCount);
						}
					}
				}
				else
				{
					if(pInfo->IsOdd == TRUE)
					{
						gPulldownMode = FILM_22_PULLDOWN_ODD;
						UpdatePulldownStatus();
						LOG(" Gone to Odd");
					}
					if(pInfo->IsOdd == FALSE)
					{
						gPulldownMode = FILM_22_PULLDOWN_EVEN;
						UpdatePulldownStatus();
						LOG(" Gone to Even");
					}
				}
			}
			else
			{
				LastPolarity = pInfo->IsOdd;
				RepeatCount = 1;
				LOG(" Reset RepeatCount %d", RepeatCount);
			}
			StartFilmTicks = GetTickCount();
		}
	}
	else
	{
		if((pInfo->CombFactor - LastCombFactor) < PulldownThresholdLow)
		{
			if(LastPolarity != pInfo->IsOdd)
			{
				if(LastDiff > PulldownThresholdLow)
				{
					RepeatCount--;
					LOG(" Downed RepeatCount 1 %d", RepeatCount);
				}
			}
			else
			{
				if(RepeatCount < PALPulldownRepeatCount)
				{
					RepeatCount++;
					LOG(" Upped RepeatCount 1 %d", RepeatCount);
				}
			}
		}
		
		if((pInfo->CombFactor - LastCombFactor) > PulldownThresholdHigh && LastDiff > PulldownThresholdLow)
		{
			if(gPulldownMode == FILM_22_PULLDOWN_ODD && pInfo->IsOdd == TRUE)
			{
				RepeatCount--;
				LOG(" Downed RepeatCount 2 %d", RepeatCount);
			}
			if(gPulldownMode == FILM_22_PULLDOWN_EVEN && pInfo->IsOdd == FALSE)
			{
				RepeatCount--;
				LOG(" Downed RepeatCount 2 %d", RepeatCount);
			}
		}
		// FIXME: Should have a different parameter here
		if(RepeatCount <= (PALPulldownRepeatCount - PALPulldownRepeatCount2))
		{
			gPulldownMode = gPALFilmFallbackMode;
			UpdatePulldownStatus();
			LOG(" Back To Video Mode");
			RepeatCount = PALPulldownRepeatCount - 1;
			LastPolarity = !pInfo->IsOdd;
			StartFilmTicks = GetTickCount();
		}
	}

	LastDiff = (pInfo->CombFactor - LastCombFactor);
	LastCombFactor = pInfo->CombFactor;
}

BOOL FilmModePAL(DEINTERLACE_INFO *pInfo)
{
	BOOL bFlipNow;
	// Film mode.  If we have an entire new frame, display it.
	bFlipNow = DoWeWantToFlipPAL(pInfo);
	if (bFlipNow)
		Weave(pInfo);
	return bFlipNow;
}


///////////////////////////////////////////////////////////////////////////////
// Flip decisioning for film modes.
// Returns true if the current field has given us a complete frame.
// If we're still waiting for another field, returns false.
///////////////////////////////////////////////////////////////////////////////
BOOL DoWeWantToFlipPAL(DEINTERLACE_INFO *pInfo)
{
	BOOL RetVal;
	switch(gPulldownMode)
	{
	case FILM_22_PULLDOWN_ODD:   RetVal = pInfo->IsOdd;  break;
	case FILM_22_PULLDOWN_EVEN:  RetVal = !pInfo->IsOdd;  break;
	default:
		RetVal = FALSE;
		break;
	}
	return RetVal;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING FD50Settings[FD50_SETTING_LASTONE] =
{
	{
		"Pulldown Threshold Low", SLIDER, 0, &PulldownThresholdLow,
		-1500, -5000, -1, 100, NULL,
		"Pulldown", "PulldownThresholdLow", NULL,
	},
	{
		"Pulldown Threshold High", SLIDER, 0, &PulldownThresholdHigh,
		200, 1, 5000, 100, NULL,
		"Pulldown", "PulldownThresholdHigh", NULL,
	},
	{
		"PAL Film Fallback Mode", ITEMFROMLIST, 0, &gPALFilmFallbackMode,
		VIDEO_MODE_2FRAME, 0, PULLDOWNMODES_LAST_ONE - 1, 1, DeintModeNames,
		"Pulldown", "PALFilmFallbackMode", NULL,
	},
	{
		"PAL Pulldown Repeat Count In", SLIDER, 0, &PALPulldownRepeatCount,
		3, 1, 10, 1, NULL,
		"Pulldown", "PALPulldownRepeatCount", NULL,
	},
	{
		"PAL Pulldown Repeat Count Out", SLIDER, 0, &PALPulldownRepeatCount2,
		1, 1, 10, 1, NULL,
		"Pulldown", "PALPulldownRepeatCount2", NULL,
	},
};

SETTING* FD50_GetSetting(FD50_SETTING Setting)
{
	if(Setting > -1 && Setting < FD50_SETTING_LASTONE)
	{
		return &(FD50Settings[Setting]);
	}
	else
	{
		return NULL;
	}
}

void FD50_ReadSettingsFromIni()
{
	int i;
	for(i = 0; i < FD50_SETTING_LASTONE; i++)
	{
		Setting_ReadFromIni(&(FD50Settings[i]));
	}
}

void FD50_WriteSettingsToIni()
{
	int i;
	for(i = 0; i < FD50_SETTING_LASTONE; i++)
	{
		Setting_WriteToIni(&(FD50Settings[i]));
	}
}

