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

ePULLDOWNMODES      gPALFilmFallbackMode = VIDEO_MODE_2FRAME;
// Default values which can be overwritten by the INI file
long                PulldownThresholdLow = -2000;
long                PulldownThresholdHigh = 2000;


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

	// call with pInfo as NULL to reset static variables when we start the thread
	// each time
	if(pInfo == NULL)
	{
		LastCombFactor = 0;
		RepeatCount = 0;
		LastPolarity = -1;
		LastDiff = 0;
		UpdatePulldownStatus();
		dwLastFlipTicks = -1;
		return;
	}

	GetCombFactor(pInfo);
	if(!DeintMethods[gPulldownMode].bIsFilmMode)
	{
		if((pInfo->CombFactor - LastCombFactor) < PulldownThresholdLow && LastDiff > PulldownThresholdLow)
		{
			if(LastPolarity == pInfo->IsOdd)
			{
				if(RepeatCount < PulldownRepeatCount)
				{
					RepeatCount++;
					LOG("Upped RepeatCount %d", RepeatCount);
				}
				else
				{
					if(pInfo->IsOdd == TRUE)
					{
						gPulldownMode = FILM_22_PULLDOWN_ODD;
						UpdatePulldownStatus();
						LOG("Gone to Odd");
					}
					if(pInfo->IsOdd == FALSE)
					{
						gPulldownMode = FILM_22_PULLDOWN_EVEN;
						UpdatePulldownStatus();
						LOG("Gone to Even");
					}
				}
			}
			else
			{
				LastPolarity = pInfo->IsOdd;
				RepeatCount = 1;
				LOG("Reset RepeatCount %d", RepeatCount);
			}
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
					LOG("Downed RepeatCount 1 %d", RepeatCount);
				}
			}
			else
			{
				if(RepeatCount < PulldownRepeatCount)
				{
					RepeatCount++;
					LOG("Upped RepeatCount 1 %d", RepeatCount);
				}
			}
		}
		if((pInfo->CombFactor - LastCombFactor) > PulldownThresholdHigh && LastDiff > PulldownThresholdLow)
		{
			if(gPulldownMode == FILM_22_PULLDOWN_ODD && pInfo->IsOdd == TRUE)
			{
				RepeatCount--;
				LOG("Downed RepeatCount 2 %d", RepeatCount);
			}
			if(gPulldownMode == FILM_22_PULLDOWN_EVEN && pInfo->IsOdd == FALSE)
			{
				RepeatCount--;
				LOG("Downed RepeatCount 2 %d", RepeatCount);
			}
		}
		if(RepeatCount == PulldownRepeatCount - PulldownRepeatCount2)
		{
			gPulldownMode = gPALFilmFallbackMode;
			RepeatCount = 0;
			UpdatePulldownStatus();
			LOG("Back To Video Mode");
			LastPolarity = -1;
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
