/////////////////////////////////////////////////////////////////////////////
// OutThread.c
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
//
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Original Release
//                                     Translated most code from German
//                                     Combined Header files
//                                     Cut out all decoding
//                                     Cut out digital hardware stuff
//
// 09 Aug 2000   John Adcock           Fixed bug at end of GetCombFactor assember
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OutThreads.h"
#include "other.h"
#include "bt848.h"
#include "bTVPlugin.h"
#include "vt.h"
#include "vbi.h"
#include "deinterlace.h"
//#define DOLOGGING
#include "DebugLog.h"
#include "vbi.h"


short pPALplusCode[] = {  18,  27,  36,  45,  54,  63,  72,  81,  90, 100, 110, 120, 134, 149};
short pPALplusData[] = { 160, 178, 196, 214, 232, 250, 268, 286, 304, 322, 340, 358, 376, 394};
short nLevelLow      =  45;
short nLevelHigh     = 135;

BOOL bStopThread = FALSE;
BOOL bIsPaused = FALSE;
HANDLE OutThread;

ePULLDOWNMODES gPulldownMode = VIDEO_MODE;

long PulldownThresholdLow = -2500;
long PulldownThresholdHigh = 2500;
long PulldownRepeatCount = 5;
long Threshold32Pulldown = 100;
BOOL bAutoDetectMode = FALSE;

void Start_Thread()
{
	int i;
	DWORD LinkThreadID;

	CurrentFrame = 0;

	for (i = 0; i < 5; i++)
	{
		pDisplay[i] = Display_dma[i]->dwUser;
	}

	// make sure we start with a clean sheet of paper
	Overlay_Clean();

	bStopThread = FALSE;

	if(TVSettings[TVTYPE].Is25fps)
	{
		OutThread = CreateThread((LPSECURITY_ATTRIBUTES) NULL,	// No security.
								 (DWORD) 0,	// Same stack size.
								 YUVOutThreadPAL,	// Thread procedure.
								 NULL,	// Parameter.
								 (DWORD) 0,	// Start immediatly.
								 (LPDWORD) & LinkThreadID);	// Thread ID.
	}
	else
	{
		OutThread = CreateThread((LPSECURITY_ATTRIBUTES) NULL,	// No security.
								 (DWORD) 0,	// Same stack size.
								 YUVOutThreadNTSC,	// Thread procedure.
								 NULL,	// Parameter.
								 (DWORD) 0,	// Start immediatly.
								 (LPDWORD) & LinkThreadID);	// Thread ID.
	}
}

void Stop_Thread()
{
	DWORD ExitCode;
	int i;
	BOOL Thread_Stopped = FALSE;

	if (OutThread != NULL)
	{
		i = 5;
		SetThreadPriority(OutThread, THREAD_PRIORITY_NORMAL);
		bStopThread = TRUE;
		while(i > 0 && !Thread_Stopped)
		{
			if (GetExitCodeThread(OutThread, &ExitCode) == TRUE)
			{
				if (ExitCode != STILL_ACTIVE)
				{
					Thread_Stopped = TRUE;
				}
			}
			else
			{
				Thread_Stopped = TRUE;
			}
			Sleep(100);
			i--;
		}

		if (Thread_Stopped == FALSE)
		{
			TerminateThread(OutThread, 0);
		}
		Sleep(50);
		CloseHandle(OutThread);
		OutThread = NULL;
	}
}

void Start_Capture()
{
	int nFlags = BT848_CAP_CTL_CAPTURE_EVEN | BT848_CAP_CTL_CAPTURE_ODD;
	if (Capture_VBI == TRUE)
	{
		nFlags |= BT848_CAP_CTL_CAPTURE_VBI_EVEN | BT848_CAP_CTL_CAPTURE_VBI_ODD;
	}

	BT848_MaskDataByte(BT848_CAP_CTL, 0, 0x0f);

	// reset intercast settings
	InterCast.esc = 0;
	InterCast.done = 0;
	InterCast.pnum = 0;
	InterCast.ok = 0;
	InterCast.datap = 0;
	InterCast.lastci = 0xff;

	BT848_SetRiscJumpsDecode(nFlags);

	BT848_MaskDataByte(BT848_CAP_CTL, (BYTE) nFlags, (BYTE) 0x0f);

	BT848_SetDMA(TRUE);

	Start_Thread();
}

void Stop_Capture()
{
	//  Stop The Output Thread
	Stop_Thread();

	// stop capture
	BT848_MaskDataByte(BT848_CAP_CTL, 0, 0x0f);
}

BOOL WaitForNextField(BOOL LastField)
{
	BOOL bIsOddField;
	DWORD stat = BT848_ReadDword(BT848_INT_STAT);

	while(LastField == ((stat & BT848_INT_FIELD) == BT848_INT_FIELD))
	{
		Sleep(5);
		stat = BT848_ReadDword(BT848_INT_STAT);
	}

	bIsOddField = ((stat & BT848_INT_FIELD) == BT848_INT_FIELD);

	switch(stat >> 28)
	{
	case 1:
		CurrentFrame = 0;
		break;
	case 2:
		CurrentFrame = 1;
		break;
	case 3:
		CurrentFrame = 2;
		break;
	case 4:
		CurrentFrame = 3;
		break;
	case 5:
		CurrentFrame = 4;
		break;
	default:
		break;
	}

	if(stat & BT848_INT_FDSR)
	{
		BT848_Restart_RISC_Code();
		CurrentFrame = 0;
		UpdatePALPulldownMode(-1, FALSE);
		UpdateNTSCPulldownMode(-1, FALSE);
	}
	return bIsOddField;
}

void SetupProcessorAndThread()
{
	DWORD rc;
	int ProcessorMask;

	ProcessorMask = 1 << (DecodeProcessor);
	rc = SetThreadAffinityMask(GetCurrentThread(), ProcessorMask);
	
	if (ThreadClassId == 0)
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
	else if (ThreadClassId == 1)
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
	else if (ThreadClassId == 2)
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
	else if (ThreadClassId == 3)
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
	else if (ThreadClassId == 4)
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
}

void UpdatePALPulldownMode(long CombFactor, BOOL IsOddField)
{
	static long LastCombFactor;
	static long RepeatCount;
	static long LastPolarity;
	static long LastDiff;

	// call with CombFactors -1 to reset static variables when we start the thread
	// each time
	if(CombFactor == -1)
	{
		LastCombFactor = 0;
		RepeatCount = 0;
		LastPolarity = -1;
		gPulldownMode = VIDEO_MODE;
		LastDiff = 0;
		return;
	}
	if(gPulldownMode == VIDEO_MODE)
	{
		if((CombFactor - LastCombFactor) < PulldownThresholdLow && LastDiff > PulldownThresholdLow)
		{
			if(LastPolarity == IsOddField)
			{
				if(RepeatCount < PulldownRepeatCount)
				{
					RepeatCount++;
					LOG("Upped RepeatCount %d", RepeatCount);
				}
				else
				{
					if(IsOddField == TRUE)
					{
						gPulldownMode = FILM_22_PULLDOWN_ODD;
						UpdatePulldownStatus();
						LOG("Gone to Odd");
					}
					if(IsOddField == FALSE)
					{
						gPulldownMode = FILM_22_PULLDOWN_EVEN;
						UpdatePulldownStatus();
						LOG("Gone to Even");
					}
				}
			}
			else
			{
				LastPolarity = IsOddField;
				RepeatCount = 1;
				LOG("Reset RepeatCount %d", RepeatCount);
			}
		}
	}
	else
	{
		if((CombFactor - LastCombFactor) < PulldownThresholdLow)
		{
			if(LastPolarity != IsOddField)
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
		if((CombFactor - LastCombFactor) > PulldownThresholdHigh && LastDiff > PulldownThresholdLow)
		{
			if(gPulldownMode == FILM_22_PULLDOWN_ODD && IsOddField == TRUE)
			{
				RepeatCount--;
				LOG("Downed RepeatCount 2 %d", RepeatCount);
			}
			if(gPulldownMode == FILM_22_PULLDOWN_EVEN && IsOddField == FALSE)
			{
				RepeatCount--;
				LOG("Downed RepeatCount 2 %d", RepeatCount);
			}
		}
		if(RepeatCount == 0)
		{
			gPulldownMode = VIDEO_MODE;
			RepeatCount = 0;
			UpdatePulldownStatus();
			LOG("Back To Video Mode");
			LastPolarity = -1;
		}
	}

	LastDiff = (CombFactor - LastCombFactor);
	LastCombFactor = CombFactor;
}


///////////////////////////////////////////////////////////////////////////////
// UpdateNTSCPulldownMode
//
// OK mark I've given up ;-)
// this is my attempt to implement Mark Rejhon's 3:2 pulldown code
// the alogoritm and comments below are taken from Mark's post to the AVS
// Home Computer Forum reproduced in the file 32Spec.htm that should be with
// this source.
// the key to getting this to work will be choosing the right value for the 
// Threshold32Pulldown variable
// this should probably vary depending on the input source, higher for video
// lower for TV and cable and very low for laserdisk
///////////////////////////////////////////////////////////////////////////////
void UpdateNTSCPulldownMode(long FieldDiff, BOOL OnOddField)
{
	static long MAX_MISMATCH = 12;
	static long MAX_VERIFY_CYCLES = 2;
	static long MISMATCH_COUNT = 0;
	static long MOVIE_FIELD_CYCLE = 0;
	static long MOVIE_VERIFY_CYCLE = 0;
	
	// call with FieldDiff -1 to reset static variables when we start the thread
	// each time
	if(FieldDiff == -1)
	{
        MOVIE_VERIFY_CYCLE = 0;
        MOVIE_FIELD_CYCLE = 0;
		MISMATCH_COUNT = 0;
		gPulldownMode = VIDEO_MODE;
		UpdatePulldownStatus();
		return;
	}

    if(FieldDiff > Threshold32Pulldown)
	{
        if(MISMATCH_COUNT <= MAX_MISMATCH)
		{
			MISMATCH_COUNT++;
		}
        else
		{
            //
            // There has been no duplicate fields lately.
            // It's probably video source.
            //
            // MAX_MISMATCH should be a reasonably high value so
            // that we do not have an oversensitive hair-trigger
            // in switching to video source everytime there is
            // video noise or a single spurious field added/dropped
            // during a movie causing mis-synchronization problems. 
            gPulldownMode = VIDEO_MODE;
            MOVIE_VERIFY_CYCLE = 0;
            MOVIE_FIELD_CYCLE = 0;
			UpdatePulldownStatus();
        }
	}
    else
	{
		// It's either a stationary image OR a duplicate field in a movie
		if(MISMATCH_COUNT == 4)
		{
			//
			// 3:2 pulldown is a cycle of 5 fields where there is only
			// one duplicate field pair, and 4 mismatching pairs.
			// We need to continue detection for at least 2 cycles
			// to be very certain that it is actually 3:2 pulldown
			// This would mean a latency of 10 fields.
			//
			if(MOVIE_VERIFY_CYCLE >= MAX_VERIFY_CYCLES)
			{
				//
				// This executes regardless whether we've just entered or
				// if we're *already* in 3:2 pulldown. Either way, we are
				// currently now (re)synchronized to 3:2 pulldown and that
				// we've now detected the duplicate field.
				//
				if(OnOddField == TRUE)
				{
					switch(CurrentFrame)
					{
					case 0:
			            gPulldownMode = FILM_32_PULLDOWN_2;
						break;
					case 1:
			            gPulldownMode = FILM_32_PULLDOWN_3;
						break;
					case 2:
			            gPulldownMode = FILM_32_PULLDOWN_4;
						break;
					case 3:
			            gPulldownMode = FILM_32_PULLDOWN_0;
						break;
					case 4:
			            gPulldownMode = FILM_32_PULLDOWN_1;
						break;
					}
					UpdatePulldownStatus();
				}
				else
				{
					switch(CurrentFrame)
					{
					case 0:
			            gPulldownMode = FILM_32_PULLDOWN_4;
						break;
					case 1:
			            gPulldownMode = FILM_32_PULLDOWN_0;
						break;
					case 2:
			            gPulldownMode = FILM_32_PULLDOWN_1;
						break;
					case 3:
			            gPulldownMode = FILM_32_PULLDOWN_2;
						break;
					case 4:
			            gPulldownMode = FILM_32_PULLDOWN_3;
						break;
					}
					UpdatePulldownStatus();
				}
			}
			else
			{
				MOVIE_VERIFY_CYCLE++;
			}
		}
		else
		{
			//
			// If execution point hits here, it is probably just
			// stationary video. That would produce lots of duplicate
			// field pairs. We can't determine whether it's video or
			// movie source. Therefore, we want to keep doing the
			// current algorithm from a prior detection. Therefore,
			// don't modify any variables EXCEPT the MISMATCH_COUNT
			// variable which will be reset to 0 below.
			// Also, occasionally we'll hit here during synchronization
			// problems during a movie, such as a spurious field added
			// or dropped. Reset MISMATCH_COUNT to 0 below and let
			// detection cycle happen again in order to re-synchronize.
			// Keep the MODE variable the way it currently is.
		}
		MISMATCH_COUNT = 0;
	}
}

BOOL DoWeWantToFlip(BOOL bFlipNow, BOOL bIsOddField)
{
	BOOL RetVal;
	switch(gPulldownMode)
	{
	case VIDEO_MODE:
		RetVal = TRUE;
		break;
	case SIMPLE_WEAVE:
		RetVal = TRUE;
		break;
	case INTERPOLATE_BOB:
		RetVal = TRUE;
		break;
	case BTV_PLUGIN:
		RetVal = bFlipNow;
		break;
	case FILM_22_PULLDOWN_ODD:
		RetVal = bIsOddField;
		break;
	case FILM_22_PULLDOWN_EVEN:
		RetVal = !bIsOddField;
		break;
	case FILM_32_PULLDOWN_0:
		switch(CurrentFrame)
		{
		case 0:
			RetVal = FALSE;
			break;
		case 1:
			RetVal = !bIsOddField;
			break;
		case 2:
			RetVal = !bIsOddField;
			break;
		case 3:
			RetVal = bIsOddField;
			break;
		case 4:
			RetVal = bIsOddField;
			break;
		}
		break;
	case FILM_32_PULLDOWN_1:
		switch(CurrentFrame)
		{
		case 0:
			RetVal = bIsOddField;
			break;
		case 1:
			RetVal = FALSE;
			break;
		case 2:
			RetVal = !bIsOddField;
			break;
		case 3:
			RetVal = !bIsOddField;
			break;
		case 4:
			RetVal = bIsOddField;
			break;
		}
		break;
	case FILM_32_PULLDOWN_2:
		switch(CurrentFrame)
		{
		case 0:
			RetVal = bIsOddField;
			break;
		case 1:
			RetVal = bIsOddField;
			break;
		case 2:
			RetVal = FALSE;
			break;
		case 3:
			RetVal = !bIsOddField;
			break;
		case 4:
			RetVal = !bIsOddField;
			break;
		}
		break;
	case FILM_32_PULLDOWN_3:
		switch(CurrentFrame)
		{
		case 0:
			RetVal = !bIsOddField;
			break;
		case 1:
			RetVal = bIsOddField;
			break;
		case 2:
			RetVal = bIsOddField;
			break;
		case 3:
			RetVal = FALSE;
			break;
		case 4:
			RetVal = !bIsOddField;
			break;
		}
		break;
	case FILM_32_PULLDOWN_4:
		switch(CurrentFrame)
		{
		case 0:
			RetVal = !bIsOddField;
			break;
		case 1:
			RetVal = !bIsOddField;
			break;
		case 2:
			RetVal = bIsOddField;
			break;
		case 3:
			RetVal = bIsOddField;
			break;
		case 4:
			RetVal = FALSE;
			break;
		}
		break;
	default:
		RetVal = FALSE;
		break;
	}
	return RetVal;
}

void UpdatePulldownStatus()
{
	switch(gPulldownMode)
	{
	case VIDEO_MODE:
		SetWindowText(hwndPalField, "Video Deinterlace");
		break;
	case SIMPLE_WEAVE:
		SetWindowText(hwndPalField, "Simple Weave");
		break;
	case INTERPOLATE_BOB:
		SetWindowText(hwndPalField, "Interpolated BOB");
		break;
	case BTV_PLUGIN:
		SetWindowText(hwndPalField, "Using bTV Plugin");
		break;
	case FILM_22_PULLDOWN_ODD:
		SetWindowText(hwndPalField, "2:2 Pulldown Flip on Odd");
		break;
	case FILM_22_PULLDOWN_EVEN:
		SetWindowText(hwndPalField, "2:2 Pulldown Flip on Even");
		break;
	case FILM_32_PULLDOWN_0:
		SetWindowText(hwndPalField, "3:2 Pulldown Skip 1st Full Frame");
		break;
	case FILM_32_PULLDOWN_1:
		SetWindowText(hwndPalField, "3:2 Pulldown Skip 2nd Full Frame");
		break;
	case FILM_32_PULLDOWN_2:
		SetWindowText(hwndPalField, "3:2 Pulldown Skip 3rd Full Frame");
		break;
	case FILM_32_PULLDOWN_3:
		SetWindowText(hwndPalField, "3:2 Pulldown Skip 4th Full Frame");
		break;
	case FILM_32_PULLDOWN_4:
		SetWindowText(hwndPalField, "3:2 Pulldown Skip 5th Full Frame");
		break;
	default:
		SetWindowText(hwndPalField, "Unknown Pulldown Mode");
		break;
	}
}

void Weave(short** pOddLines, short** pEvenLines, BYTE* lpOverlay)
{
	int i;

	for (i = 0; i < CurrentY / 2; i++)
	{
		memcpyMMX(lpOverlay, pEvenLines[i], CurrentX * 2);
		lpOverlay += OverlayPitch;

		memcpyMMX(lpOverlay, pOddLines[i], CurrentX * 2);
		lpOverlay += OverlayPitch;
	}
}

DWORD WINAPI YUVOutThreadPAL(LPVOID lpThreadParameter)
{
	char Text[128];
	int i, j;
	int nLineTarget;
	int nFrame = 0;
	DWORD dwLastSecondTicks;
	DWORD dwLastFlipTicks = 0;
	BYTE* lpCurOverlay = lpOverlayBack;
	long CombFactor;
	short* ppEvenLines[5][CLEARLINES];
	short* ppOddLines[5][CLEARLINES];
	BYTE* pDest;
	int LastEvenFrame = 0;
	int LastOddFrame = 0;
	int CombNum = 0;
	BOOL bFlipNow = TRUE;
	BOOL bIsOddField = FALSE;

	if (lpDDOverlay == NULL || lpDDOverlay == NULL || lpOverlayBack == NULL || lpOverlay == NULL)
	{
		ExitThread(-1);
	}

	// Sets processor Affinity and Thread priority according to menu selection
	SetupProcessorAndThread();

	// reset the static variables in the detection code
	UpdatePALPulldownMode(-1, FALSE);

	// Set up 5 sets of pointers to the start of odd and even lines
	// we will always go up to the limit so that we can use bTV
	for (j = 0; j < 5; j++)
	{
		for (i = 0; i < BTV_VER1_HEIGHT; i += 2)
		{
			ppOddLines[j][i / 2] = (short *) pDisplay[j] + (i + 1) * 1024;
			ppEvenLines[j][i / 2] = (short *) pDisplay[j] + i * 1024;
		}
	}

	// start the capture off
	BT848_Restart_RISC_Code();

	// display the current pulldown mode
	UpdatePulldownStatus();
	
	dwLastSecondTicks = GetTickCount();

	while(!bStopThread)
	{
		bIsOddField = WaitForNextField(bIsOddField);
		if(bIsPaused == FALSE)
		{
			pDest = lpCurOverlay;
			if(bIsOddField)
			{
				if(bAutoDetectMode == TRUE)
				{
					CombFactor = GetCombFactor(ppEvenLines[CurrentFrame], ppOddLines[CurrentFrame]);
					UpdatePALPulldownMode(CombFactor, TRUE);
					LOG(" Frame %d O CF = %d", CurrentFrame, CombFactor);
				}

				if (Capture_VBI == TRUE)
				{
					BYTE * pVBI = (LPBYTE) Vbi_dma[CurrentFrame]->dwUser;
					for (nLineTarget = VBI_lpf; nLineTarget < 2 * VBI_lpf ; nLineTarget++)
					{
						VBI_DecodeLine(pVBI + nLineTarget * 2048, nLineTarget - VBI_lpf);
					}
				}

				// if we have dropped a field then do BOB 
				if(LastEvenFrame != CurrentFrame || gPulldownMode == INTERPOLATE_BOB)
				{
					for (nLineTarget = 0; nLineTarget < CurrentY / 2; nLineTarget++)
					{
						memcpyBOBMMX(pDest,
									ppOddLines[CurrentFrame][nLineTarget], 
									CurrentX * 2);
						pDest += 2 * OverlayPitch;
					}
					bFlipNow = TRUE;
					if(LastEvenFrame != CurrentFrame)
					{
						nFrame++;
					}
				}
				else if(gPulldownMode == VIDEO_MODE)
				{
					DeinterlaceOdd(ppOddLines[CurrentFrame], ppEvenLines[LastEvenFrame], lpCurOverlay);
				}
				else if(gPulldownMode == SIMPLE_WEAVE)
				{
					Weave(ppOddLines[CurrentFrame], ppEvenLines[LastEvenFrame], lpCurOverlay);
				}
				else if(gPulldownMode == BTV_PLUGIN)
				{
					BYTE* pDestEven[CLEARLINES];
					BYTE* pDestOdd[CLEARLINES];
					
					// set up desination pointers
					// may need to optimize this later
					for (i = 0; i < BTV_VER1_HEIGHT; i += 2)
					{
						pDestEven[i / 2] = pDest;
						pDest += OverlayPitch;
						pDestOdd[i / 2] = pDest;
						pDest += OverlayPitch;
					}
					
					BTVParams.IsOddField = 1;
					BTVParams.ppCurrentField = ppOddLines[CurrentFrame];
					BTVParams.ppLastField = ppEvenLines[LastEvenFrame];
					BTVParams.ppEvenDest = pDestEven;
					BTVParams.ppOddDest = pDestOdd;
					bFlipNow = BTVPluginDoField(&BTVParams);
				}
				else
				{
					pDest += OverlayPitch;
					for (nLineTarget = 0; nLineTarget < CurrentY / 2; nLineTarget++)
					{
						// copy latest data to destination buffer
						memcpyMMX(pDest, 
							ppOddLines[CurrentFrame][nLineTarget], 
							CurrentX * 2);
						pDest += 2 * OverlayPitch;
					}
				}
				LastOddFrame = CurrentFrame;
			}
			else
			{
				if(bAutoDetectMode == TRUE)
				{
					// need to add one to the even lines
					// so that the top line is between the
					// top two even lines
					CombFactor = GetCombFactor(ppEvenLines[CurrentFrame], ppOddLines[(CurrentFrame + 4) % 5]);
					UpdatePALPulldownMode(CombFactor, FALSE);
					LOG(" Frame %d E CF = %d", CurrentFrame, CombFactor);
				}

				if (Capture_VBI == TRUE)
				{
					BYTE * pVBI = (LPBYTE) Vbi_dma[CurrentFrame]->dwUser;
					for (nLineTarget = 0; nLineTarget < VBI_lpf ; nLineTarget++)
					{
						VBI_DecodeLine(pVBI + nLineTarget * 2048, nLineTarget);
					}
				}

				// if we have dropped a field then do BOB
				if(LastOddFrame != ((CurrentFrame + 4) % 5) || gPulldownMode == INTERPOLATE_BOB)
				{
					for (nLineTarget = 0; nLineTarget < CurrentY / 2; nLineTarget++)
					{
						// copy latest field data to both odd and even rows
						memcpyBOBMMX(pDest, 
									ppEvenLines[CurrentFrame][nLineTarget],
									CurrentX * 2);
						pDest += 2 * OverlayPitch;
					}
					bFlipNow = TRUE;
					if(LastOddFrame != ((CurrentFrame + 4) % 5))
					{
						nFrame++;
					}
				}
				else if(gPulldownMode == VIDEO_MODE)
				{
					DeinterlaceEven(ppOddLines[LastOddFrame], ppEvenLines[CurrentFrame], lpCurOverlay);
				}
				else if(gPulldownMode == SIMPLE_WEAVE)
				{
					Weave(ppOddLines[LastOddFrame], ppEvenLines[CurrentFrame], lpCurOverlay);
				}
				else if(gPulldownMode == BTV_PLUGIN)
				{
					BYTE* pDestEven[CLEARLINES];
					BYTE* pDestOdd[CLEARLINES];
					
					// set up desination pointers
					// may need to optimize this later
					for (i = 0; i < BTV_VER1_HEIGHT; i += 2)
					{
						pDestEven[i / 2] = pDest;
						pDest += OverlayPitch;
						pDestOdd[i / 2] = pDest;
						pDest += OverlayPitch;
					}
					
					BTVParams.IsOddField = 1;
					BTVParams.ppCurrentField = ppOddLines[CurrentFrame];
					BTVParams.ppLastField = ppEvenLines[LastEvenFrame];
					BTVParams.ppEvenDest = pDestEven;
					BTVParams.ppOddDest = pDestOdd;
					bFlipNow = BTVPluginDoField(&BTVParams);
				}
				else
				{
					for (nLineTarget = 0; nLineTarget < CurrentY / 2; nLineTarget++)
					{
						// copy latest data to destination buffer
						memcpyMMX(pDest, 
									ppEvenLines[CurrentFrame][nLineTarget], 
									CurrentX  * 2);
						pDest += 2 * OverlayPitch;
					}
				}
				LastEvenFrame = CurrentFrame;
			}

			if(DoWeWantToFlip(bFlipNow, bIsOddField))
			{
				// wait for a good time to flip
				//sprintf(Text, "%d\n", GetTickCount() - dwLastFlipTicks);
				//OutputDebugString(Text);
				IDirectDrawSurface_Flip(lpDDOverlay, lpDDOverlayBack, DDFLIP_WAIT);
				dwLastFlipTicks = GetTickCount();
				if(lpCurOverlay == lpOverlay)
				{
					lpCurOverlay = lpOverlayBack;
				}
				else
				{
					lpCurOverlay = lpOverlay;
				}
			}
		}

		if (bDisplayStatusBar == TRUE)
		{
			if (dwLastSecondTicks + 1000 < GetTickCount())
			{
				sprintf(Text, "%d DF/S", nFrame);
				SetWindowText(hwndFPSField, Text);
				nFrame = 0;
				dwLastSecondTicks = GetTickCount();
			}
		}
	}
	ExitThread(0);
	return 0;
}

DWORD WINAPI YUVOutThreadNTSC(LPVOID lpThreadParameter)
{
	char Text[128];
	int i, j;
	int nLineTarget;
	int nFrame = 0;
	DWORD dwLastCount;
	BYTE* lpCurOverlay = lpOverlayBack;
	long CompareResult;
	short* ppEvenLines[5][CLEARLINES];
	short* ppOddLines[5][CLEARLINES];
	BYTE* pDest;
	int LastEvenFrame = 0;
	int LastOddFrame = 0;
	int CombNum = 0;
	BOOL bFlipNow = TRUE;
	BOOL bIsOddField = FALSE;

	if (lpDDOverlay == NULL || lpDDOverlay == NULL || lpOverlayBack == NULL || lpOverlay == NULL)
	{
		ExitThread(-1);
	}

	// Sets processor Affinity and Thread priority according to menu selection
	SetupProcessorAndThread();

	// start the capture off
	BT848_Restart_RISC_Code();

	// Set up 5 sets of pointers to the start of odd and even lines
	// we will always go up to the limit so that we can use bTV
	for (j = 0; j < 5; j++)
	{
		for (i = 0; i < BTV_VER1_HEIGHT; i += 2)
		{
			ppOddLines[j][i / 2] = (short *) pDisplay[j] + (i + 1) * 1024;
			ppEvenLines[j][i / 2] = (short *) pDisplay[j] + i * 1024;
		}
	}

	// reset the static variables in the detection code
	UpdateNTSCPulldownMode(-1, FALSE);

	// display the current pulldown mode
	UpdatePulldownStatus();
	
	dwLastCount = GetTickCount();

	while(!bStopThread)
	{
		bIsOddField = WaitForNextField(bIsOddField);
		if(bIsPaused == FALSE)
		{
			pDest = lpCurOverlay;
			if(bIsOddField)
			{
				if(bAutoDetectMode == TRUE)
				{
					CompareResult = CompareFields(ppOddLines[(CurrentFrame + 4) % 5], ppOddLines[CurrentFrame]);
					UpdateNTSCPulldownMode(CompareResult, TRUE);
					LOG(" Frame %d O CR = %d", CurrentFrame, CompareResult);
				}

				if (Capture_VBI == TRUE)
				{
					BYTE * pVBI = (LPBYTE) Vbi_dma[CurrentFrame]->dwUser;
					for (nLineTarget = VBI_lpf; nLineTarget < 2 * VBI_lpf ; nLineTarget++)
					{
						VBI_DecodeLine(pVBI + nLineTarget * 2048, nLineTarget - VBI_lpf);
					}
				}

				// if we have dropped a field then do BOB 
				if(LastEvenFrame != CurrentFrame || gPulldownMode == INTERPOLATE_BOB)
				{
					for (nLineTarget = 0; nLineTarget < CurrentY / 2; nLineTarget++)
					{
						memcpyBOBMMX(pDest,
									ppOddLines[CurrentFrame][nLineTarget], 
									CurrentX * 2);
						pDest += 2 * OverlayPitch;
					}
					bFlipNow = TRUE;
					if(LastEvenFrame != CurrentFrame)
					{
						nFrame++;
					}
				}
				else if(gPulldownMode == VIDEO_MODE)
				{
					DeinterlaceOdd(ppOddLines[CurrentFrame], ppEvenLines[LastEvenFrame], lpCurOverlay);
				}
				else if(gPulldownMode == SIMPLE_WEAVE)
				{
					Weave(ppOddLines[CurrentFrame], ppEvenLines[LastEvenFrame], lpCurOverlay);
				}
				else if(gPulldownMode == BTV_PLUGIN)
				{
					BYTE* pDestEven[CLEARLINES];
					BYTE* pDestOdd[CLEARLINES];
					
					// set up desination pointers
					// may need to optimize this later
					for (i = 0; i < BTV_VER1_HEIGHT; i += 2)
					{
						pDestEven[i / 2] = pDest;
						pDest += OverlayPitch;
						pDestOdd[i / 2] = pDest;
						pDest += OverlayPitch;
					}
					
					BTVParams.IsOddField = 1;
					BTVParams.ppCurrentField = ppOddLines[CurrentFrame];
					BTVParams.ppLastField = ppEvenLines[LastEvenFrame];
					BTVParams.ppEvenDest = pDestEven;
					BTVParams.ppOddDest = pDestOdd;
					bFlipNow = BTVPluginDoField(&BTVParams);
				}
				else
				{
					pDest += OverlayPitch;
					for (nLineTarget = 0; nLineTarget < CurrentY / 2; nLineTarget++)
					{
						// copy latest data to destination buffer
						memcpyMMX(pDest, 
							ppOddLines[CurrentFrame][nLineTarget], 
							CurrentX * 2);
						pDest += 2 * OverlayPitch;
					}
				}
				LastOddFrame = CurrentFrame;
			}
			else
			{
				if(bAutoDetectMode == TRUE)
				{
					CompareResult = CompareFields(ppEvenLines[(CurrentFrame + 4) % 5], ppEvenLines[CurrentFrame]);
					UpdateNTSCPulldownMode(CompareResult, FALSE);
					LOG(" Frame %d E CR = %d", CurrentFrame, CompareResult);
				}

				if (Capture_VBI == TRUE)
				{
					BYTE * pVBI = (LPBYTE) Vbi_dma[CurrentFrame]->dwUser;
					for (nLineTarget = 0; nLineTarget < VBI_lpf ; nLineTarget++)
					{
						VBI_DecodeLine(pVBI + nLineTarget * 2048, nLineTarget);
					}
				}

				// if we have dropped a field then do BOB
				if(LastOddFrame != ((CurrentFrame + 4) % 5) || gPulldownMode == INTERPOLATE_BOB)
				{
					for (nLineTarget = 0; nLineTarget < CurrentY / 2; nLineTarget++)
					{
						// copy latest field data to both odd and even rows
						memcpyBOBMMX(pDest, 
									ppEvenLines[CurrentFrame][nLineTarget],
									CurrentX * 2);
						pDest += 2 * OverlayPitch;
					}
					bFlipNow = TRUE;
					if(LastOddFrame != ((CurrentFrame + 4) % 5))
					{
						nFrame++;
					}
				}
				else if(gPulldownMode == VIDEO_MODE)
				{
					DeinterlaceEven(ppOddLines[LastOddFrame], ppEvenLines[CurrentFrame], lpCurOverlay);
				}
				else if(gPulldownMode == SIMPLE_WEAVE)
				{
					Weave(ppOddLines[LastOddFrame], ppEvenLines[CurrentFrame], lpCurOverlay);
				}
				else if(gPulldownMode == BTV_PLUGIN)
				{
					BYTE* pDestEven[CLEARLINES];
					BYTE* pDestOdd[CLEARLINES];
					
					// set up desination pointers
					// may need to optimize this later
					for (i = 0; i < BTV_VER1_HEIGHT; i += 2)
					{
						pDestEven[i / 2] = pDest;
						pDest += OverlayPitch;
						pDestOdd[i / 2] = pDest;
						pDest += OverlayPitch;
					}
					
					BTVParams.IsOddField = 1;
					BTVParams.ppCurrentField = ppOddLines[CurrentFrame];
					BTVParams.ppLastField = ppEvenLines[LastEvenFrame];
					BTVParams.ppEvenDest = pDestEven;
					BTVParams.ppOddDest = pDestOdd;
					bFlipNow = BTVPluginDoField(&BTVParams);
				}
				else
				{
					for (nLineTarget = 0; nLineTarget < CurrentY / 2; nLineTarget++)
					{
						// copy latest data to destination buffer
						memcpyMMX(pDest, 
									ppEvenLines[CurrentFrame][nLineTarget], 
									CurrentX  * 2);
						pDest += 2 * OverlayPitch;
					}
				}
				LastEvenFrame = CurrentFrame;
			}

			if(DoWeWantToFlip(bFlipNow, bIsOddField))
			{
				IDirectDrawSurface_Flip(lpDDOverlay, lpDDOverlayBack, DDFLIP_WAIT);
				if(lpCurOverlay == lpOverlay)
				{
					lpCurOverlay = lpOverlayBack;
				}
				else
				{
					lpCurOverlay = lpOverlay;
				}
			}
		}

		if (bDisplayStatusBar == TRUE)
		{
			if (dwLastCount + 1000 < GetTickCount())
			{
				sprintf(Text, "%d DF/S", nFrame);
				SetWindowText(hwndFPSField, Text);
				nFrame = 0;
				dwLastCount = GetTickCount();
			}
		}
	}
	BT848_SetDMA(FALSE);
	ExitThread(0);
	return 0;
}

