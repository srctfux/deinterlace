/////////////////////////////////////////////////////////////////////////////
// FieldTiming.c
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
// 09 Jan 2001   John Adcock           Split out into new file
//                                     Changed functions to use DEINTERLACE_INFO
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FieldTiming.h"
#include "BT848.h"
#include "DebugLog.h"

LARGE_INTEGER TimerFrequency;
double RunningAverageCounterTicks;
double StartAverageCounterTicks;
LARGE_INTEGER LastFieldTime;
LARGE_INTEGER CurrentFieldTime;
LARGE_INTEGER LastFlipTime;
LARGE_INTEGER CurrentFlipTime;
BOOL bIsPAL;
BOOL FlipAdjust;
int nDroppedFields = 0;
int nUsedFields = 0;
double Weight = 0.005;

void Timing_Setup()
{
	bIsPAL = BT848_GetTVFormat()->Is25fps;

	// get the Frequency of the high resolution timer
	QueryPerformanceFrequency(&TimerFrequency);

	if(bIsPAL)
	{
		RunningAverageCounterTicks = (double)TimerFrequency.QuadPart / 25.0;
		StartAverageCounterTicks = RunningAverageCounterTicks;
	}
	else
	{
		RunningAverageCounterTicks = (double)TimerFrequency.QuadPart / 29.97;
		StartAverageCounterTicks = RunningAverageCounterTicks;
	}
	FlipAdjust = FALSE;
	Timing_Reset();
}

void UpdateRunningAverage(LARGE_INTEGER* pNewFieldTime)
{
	// if we're not running late then
	// and we got a good clean run last time
	if(LastFieldTime.QuadPart != 0)
	{
		// gets the last ticks odd - odd
		double RecentTicks = (double)(pNewFieldTime->QuadPart - LastFieldTime.QuadPart);
		// only allow values within 5% if current value
		// should prevent spurious values getting through
		if(RecentTicks > RunningAverageCounterTicks * 0.95 &&
			RecentTicks < RunningAverageCounterTicks * 1.05)
		{
			// update the average
			// we're doing this weighted average because
			// it has lots of nice properties
			// especially that we don't need to keep a 
			// data history
			RunningAverageCounterTicks = Weight * RecentTicks + (1.0 - Weight) * RunningAverageCounterTicks;
			LOG(" Last %f", RecentTicks);
			LOG(" Running Average %f", RunningAverageCounterTicks);
		}
		else
		{
			LOG(" Last %f (IGNORED)", RecentTicks);
			LOG(" Old Running Average %f", RunningAverageCounterTicks);
		}
	}
	// save current value for next time
	LastFieldTime.QuadPart = pNewFieldTime->QuadPart;
}


////////////////////////////////////////////////////////////////////////////////////
// The following function will continually check the position in the RISC code
// until it is  is different from what we already have.
// We know were we are so we set the current field to be the last one
// that has definitely finished.
//
// Added code here to use a user specified parameter for how long to sleep.  Note that
// windows timer tick resolution is really MUCH worse than 1 millesecond.  Who knows 
// what logic W98 really uses?
//
// Note also that sleep(0) just tells the operating system to dispatch some other
// task now if one is ready, not to sleep for zero seconds.  Since I've taken most
// of the unneeded waits out of other processing here Windows will eventually take 
// control away from us anyway, We might as well choose the best time to do it, without
// waiting more than needed. 
//
// Also added code to HurryWhenLate.  This checks if the new field is already here by
// the time we arrive.  If so, assume we are not keeping up with the BT chip and skip
// some later processing.  Skip displaying this field and use the CPU time gained to 
// get back here faster for the next one.  This should help us degrade gracefully on
// slower or heavily loaded systems but use all available time for processing a good
// picture when nothing else is running.  TRB 10/28/00
//
void Timing_WaitForNextFieldNormal(DEINTERLACE_INFO* pInfo)
{
	int NewPos;
	int Diff;
	int OldPos = (pInfo->CurrentFrame * 2 + pInfo->IsOdd + 1) % 10;
	while(OldPos == (NewPos = BT848_GetRISCPosAsInt()))
	{
		Sleep(pInfo->SleepInterval);
		pInfo->bRunningLate = FALSE;			// if we waited then we are not late
	}

	Diff = (10 + NewPos - OldPos) % 10;
	if(Diff > 1)
	{
		// delete all history
		memset(pInfo->EvenLines, 0, MAX_FIELD_HISTORY * sizeof(short**));
		memset(pInfo->OddLines, 0, MAX_FIELD_HISTORY * sizeof(short**));
		pInfo->bMissedFrame = TRUE;
		nDroppedFields += Diff - 1;
		LOG(" Dropped Frame");
	}
	else
	{
		pInfo->bMissedFrame = FALSE;
		if (pInfo->bRunningLate)
		{
			nDroppedFields++;
			LOG("Running Late");
		}
	}

	switch(NewPos)
	{
	case 0: pInfo->IsOdd = TRUE;  pInfo->CurrentFrame = 4; break;
	case 1: pInfo->IsOdd = FALSE; pInfo->CurrentFrame = 0; break;
	case 2: pInfo->IsOdd = TRUE;  pInfo->CurrentFrame = 0; break;
	case 3: pInfo->IsOdd = FALSE; pInfo->CurrentFrame = 1; break;
	case 4: pInfo->IsOdd = TRUE;  pInfo->CurrentFrame = 1; break;
	case 5: pInfo->IsOdd = FALSE; pInfo->CurrentFrame = 2; break;
	case 6: pInfo->IsOdd = TRUE;  pInfo->CurrentFrame = 2; break;
	case 7: pInfo->IsOdd = FALSE; pInfo->CurrentFrame = 3; break;
	case 8: pInfo->IsOdd = TRUE;  pInfo->CurrentFrame = 3; break;
	case 9: pInfo->IsOdd = FALSE; pInfo->CurrentFrame = 4; break;
	}
}

void Timing_WaitForNextFieldAccurate(DEINTERLACE_INFO* pInfo)
{
	int NewPos;
	int Diff;
	int OldPos = (pInfo->CurrentFrame * 2 + pInfo->IsOdd + 1) % 10;
	
	// accuarate time is incomapatable with hurry when late
	pInfo->bRunningLate = TRUE;

	while(OldPos == (NewPos = BT848_GetRISCPosAsInt()))
	{
		pInfo->bRunningLate = FALSE;			// if we waited then we are not late
	}

	Diff = (10 + NewPos - OldPos) % 10;
	if(Diff == 1)
	{
	}
	else if(Diff == 2) 
	{
		NewPos = (OldPos + 1) % 10;
		FlipAdjust = TRUE;
		LOG(" Slightly late");
	}
	else if(Diff == 3) 
	{
		NewPos = (OldPos + 1) % 10;
		FlipAdjust = TRUE;
		LOG(" Very late");
	}
	else
	{
		// delete all history
		memset(pInfo->EvenLines, 0, MAX_FIELD_HISTORY * sizeof(short**));
		memset(pInfo->OddLines, 0, MAX_FIELD_HISTORY * sizeof(short**));
		pInfo->bMissedFrame = TRUE;
		nDroppedFields += Diff - 1;
		LOG(" Dropped Frame");
		Timing_Reset();
	}

	switch(NewPos)
	{
	case 0: pInfo->IsOdd = TRUE;  pInfo->CurrentFrame = 4; break;
	case 1: pInfo->IsOdd = FALSE; pInfo->CurrentFrame = 0; break;
	case 2: pInfo->IsOdd = TRUE;  pInfo->CurrentFrame = 0; break;
	case 3: pInfo->IsOdd = FALSE; pInfo->CurrentFrame = 1; break;
	case 4: pInfo->IsOdd = TRUE;  pInfo->CurrentFrame = 1; break;
	case 5: pInfo->IsOdd = FALSE; pInfo->CurrentFrame = 2; break;
	case 6: pInfo->IsOdd = TRUE;  pInfo->CurrentFrame = 2; break;
	case 7: pInfo->IsOdd = FALSE; pInfo->CurrentFrame = 3; break;
	case 8: pInfo->IsOdd = TRUE;  pInfo->CurrentFrame = 3; break;
	case 9: pInfo->IsOdd = FALSE; pInfo->CurrentFrame = 4; break;
	}
	
	// we've just got a new field
	// we are going to time the odd to odd
	// input frequency
	if(pInfo->IsOdd)
	{
		QueryPerformanceCounter(&CurrentFieldTime);
		if(!(pInfo->bRunningLate))
		{
			UpdateRunningAverage(&CurrentFieldTime);
		}
		else
		{
			// if we're running late then
			// time will be rubbish
			// so make sure it won't be used
			LastFieldTime.QuadPart = 0;
		}
	}
	pInfo->bRunningLate = FALSE;

	// we have to sleep somewhere might as well be here
	// since we don't sleep anywhere in the WaitForNextField
	// anymore
	Sleep(pInfo->SleepInterval);
}

void Timing_WaitForNextField(DEINTERLACE_INFO* pInfo)
{
	if(pInfo->bDoAccurateFlips)
	{
		Timing_WaitForNextFieldAccurate(pInfo);
	}
	else
	{
		Timing_WaitForNextFieldNormal(pInfo);
	}
	if (!pInfo->bRunningLate)
	{
		nUsedFields++;
	}
}

void Timing_Reset()
{
	LastFieldTime.QuadPart = 0;
	CurrentFieldTime.QuadPart = 0;
	LastFlipTime.QuadPart = 0;
	CurrentFlipTime.QuadPart = 0;
}

void Timing_WaitForTimeToFlip(DEINTERLACE_INFO* pInfo, DEINTERLACE_METHOD* CurrentMethod, BOOL* bStopThread)
{
	if(pInfo->bMissedFrame == FALSE && FlipAdjust == FALSE)
	{
		if(LastFlipTime.QuadPart == 0)
		{
			QueryPerformanceCounter(&LastFlipTime);
		}
		else
		{
			LONGLONG TicksToWait;
			//int NewPos;
			//int OldPos = BT848_GetRISCPosAsInt();

			// work out the required ticks between flips
			if(bIsPAL)
			{
				TicksToWait = (LONGLONG)(RunningAverageCounterTicks * 25.0 / (double)CurrentMethod->FrameRate50Hz);
			}
			else
			{
				TicksToWait = (LONGLONG)(RunningAverageCounterTicks * 30.0 / (double)CurrentMethod->FrameRate60Hz);
			}
			QueryPerformanceCounter(&CurrentFlipTime);
			while(!(*bStopThread) && (CurrentFlipTime.QuadPart - LastFlipTime.QuadPart) < TicksToWait)
			{
				//NewPos = BT848_GetRISCPosAsInt();
				//if((NewPos + 9) % 10 == OldPos)
				//{
				//	if((NewPos & 1) == 0)
				//	{
				//		UpdateRunningAverage(&CurrentFlipTime);
				//	}
				//	OldPos = NewPos;
				//}
				QueryPerformanceCounter(&CurrentFlipTime);
			}
			LastFlipTime.QuadPart = CurrentFlipTime.QuadPart;
		}
	}
	else
	{
		QueryPerformanceCounter(&LastFlipTime);
	}
	FlipAdjust = FALSE;
}

int Timing_GetDroppedFields()
{
	return nDroppedFields;
}

void Timing_ResetDroppedFields()
{
	nDroppedFields = 0;
}

int Timing_GetUsedFields()
{
	return nUsedFields;
}

void Timing_ResetUsedFields()
{
	nUsedFields = 0;
}