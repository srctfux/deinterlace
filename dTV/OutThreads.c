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


short pPALplusCode[] = {  18,  27,  36,  45,  54,  63,  72,  81,  90, 100, 110, 120, 134, 149};
short pPALplusData[] = { 160, 178, 196, 214, 232, 250, 268, 286, 304, 322, 340, 358, 376, 394};
short nLevelLow      =  45;
short nLevelHigh     = 135;

BOOL bStopThread = FALSE;
HANDLE OutThread;

struct TPulldownDetect
{
	ePULLDOWNMODES Mode;
	WORD nFlags1;
	WORD nFlags2;
};

struct TPulldownDetect PulldownDetect[] =
{
	{VIDEO_MODE, 0, 0,},
	// if it's 101 010 101 then 2:2 flip on odd
	{FILM_22_PULLDOWN_ODD, 0525, 0252,},
	// if it's 010 101 010 then 2:2 flip on even
	{FILM_22_PULLDOWN_EVEN, 0252, 0525,},
	// if it's 101 101 011 then 3:2 flip on first
	{FILM_32_PULLDOWN_0, 0553, 0224,},
	// if it's 110 101 101 then 3:2 flip on Second
	{FILM_32_PULLDOWN_1, 0655, 0122,},
	// if it's 010 110 101 then 3:2 flip on Third
	{FILM_32_PULLDOWN_2, 0265, 0512,},
	// if it's 011 010 110 then 3:2 flip on Forth
	{FILM_32_PULLDOWN_3, 0326, 0451,},
	// if it's 101 011 010 then 3:2 flip on Fifth
	{FILM_32_PULLDOWN_4, 0532, 0245,},
};

void Start_Thread()
{
	int i;
	DWORD LinkThreadID;

	CurrentFrame = 0;

	for (i = 0; i < 5; i++)
	{
		pDisplay[i] = Display_dma[i]->dwUser;
	}

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

	// resety intercast settings
	InterCast.esc = 0;
	InterCast.done = 0;
	InterCast.pnum = 0;
	InterCast.ok = 0;
	InterCast.datap = 0;
	InterCast.lastci = 0xff;

	BT848_SetRiscJumpsDecode(nFlags);

	BT848_MaskDataByte(BT848_CAP_CTL, (BYTE) nFlags, (BYTE) 0x0f);

	if (nFlags & 0x0f)
	{
		BT848_SetDMA(TRUE);
	}
	else
	{
		BT848_SetDMA(FALSE);
	}

	Start_Thread();
	if (Capture_VBI == TRUE)
	{
		VBI_Start();
	}
}

void Stop_Capture()
{
	BT848_MaskDataByte(BT848_CAP_CTL, 0, 0x0f);

	//  Stop The Output Thread
	Stop_Thread();
	if (Capture_VBI == TRUE)
	{
		VBI_Stop();
	}
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


ePULLDOWNMODES GuessCorrectPALPulldownMode(WORD Flags1, WORD Flags2)
{
	ePULLDOWNMODES i;
	for(i = 1; i < 3; i++)
	{
		if((Flags1 & PulldownDetect[i].nFlags1) == PulldownDetect[i].nFlags1)
		{
			return i;
		}
	}
	return VIDEO_MODE;
}

ePULLDOWNMODES GuessCorrectNTSCPulldownMode(WORD Flags1, WORD Flags2)
{
	ePULLDOWNMODES i;
	for(i = 3; i < PULLDOWNMODES_LAST_ONE; i++)
	{
		if((Flags1 & PulldownDetect[i].nFlags1) == PulldownDetect[i].nFlags1 &&
			(Flags2 & PulldownDetect[i].nFlags2) == 0 )
		{
			return i;
		}
	}
	return VIDEO_MODE;
}

void UpdatePALPulldownMode(struct TPulldowmMode* PulldownMode, long* CombFactors)
{
	WORD Flags1 = 0;
	WORD Flags2 = 0;
	int i;

	for(i = 0; i < 9; i++)
	{
		int Diff = CombFactors[i + 1] - CombFactors[i];
		if(Diff * 100 / (CombFactors[i + 1]  + 1) < PulldownThresholdHigh)
		{
			Flags1 |= 1 << i;
		}
		if(Diff * 100 / (CombFactors[i + 1]  + 1) > PulldownThresholdLow)
		{
			Flags2 |= 1 << i;
		}
	}

	if(gPulldownMode == VIDEO_MODE)
	{
		// if we are in video mode then look for the same
		// pattern being repeated PulldownRepeatCount times
		ePULLDOWNMODES BestGuess = GuessCorrectPALPulldownMode(Flags1, Flags2);
		if(BestGuess != 0 && BestGuess == PulldownMode->LastGuess)
		{
			if((Flags2 & PulldownDetect[gPulldownMode].nFlags2) == 0)
			{
				PulldownMode->LastGuessCount++;
			}
		}
		else if(BestGuess != 0)
		{
			PulldownMode->LastGuessCount = 1;
			PulldownMode->LastGuess = BestGuess;
		}
		if(PulldownMode->LastGuessCount > PulldownRepeatCount)
		{
			gPulldownMode = PulldownMode->LastGuess;
			PulldownMode->nCount = PulldownMode->LastGuessCount;
			PulldownMode->LastGuessCount = 0;
			UpdatePulldownStatus();
		}
	}
	else
	{
		// is it not OK to carry on using what we started with
		// then update the structure
		if((Flags1 & PulldownDetect[gPulldownMode].nFlags1) != PulldownDetect[gPulldownMode].nFlags1)
		{
			ePULLDOWNMODES BestGuess = GuessCorrectPALPulldownMode(Flags1, Flags2);
			if(BestGuess == PulldownMode->LastGuess)
			{
				if((Flags2 & PulldownDetect[gPulldownMode].nFlags2) == 0)
				{
					PulldownMode->LastGuessCount++;
				}
			}
			else
			{
				PulldownMode->LastGuessCount = 0;
			}
			PulldownMode->nCount--;
			if(PulldownMode->nCount == 0)
			{
				// if we have got some sort of new lock then jump straight to it
				if(PulldownMode->LastGuessCount > 2)
				{
					gPulldownMode = PulldownMode->LastGuess;
					PulldownMode->nCount = PulldownMode->LastGuessCount;
					PulldownMode->LastGuessCount = 1;
					UpdatePulldownStatus();
				}
				else
				{
					gPulldownMode = VIDEO_MODE;
					PulldownMode->nCount = PulldownMode->LastGuessCount;
					UpdatePulldownStatus();
				}
			}
		}
		else
		{
			PulldownMode->LastGuess = gPulldownMode;
			PulldownMode->LastGuessCount = 0;
			if(PulldownMode->nCount < PulldownRepeatCount)
			{
				if((Flags2 & PulldownDetect[gPulldownMode].nFlags2) == 0)
				{
					PulldownMode->nCount++;
				}
			}
		}
	}
}

void UpdateNTSCPulldownMode(struct TPulldowmMode* PulldownMode, long* CombFactors)
{
	WORD Flags1 = 0;
	WORD Flags2 = 0;
	int i;

	for(i = 0; i < 9; i++)
	{
		int Diff = CombFactors[i + 1] - CombFactors[i];
		if(Diff * 100 / (CombFactors[i + 1]  + 1) < PulldownThresholdLow)
		{
			Flags1 |= 1 << i;
		}
		else if(Diff * 100 / (CombFactors[i + 1] + 1) > PulldownThresholdHigh)
		{
			Flags2 |= 1 << i;
		}
	}

	if(gPulldownMode == VIDEO_MODE)
	{
		// if we are in video mode then look for the same
		// pattern being repeated PulldownRepeatCount times
		ePULLDOWNMODES BestGuess = GuessCorrectNTSCPulldownMode(Flags1, Flags2);
		if(BestGuess == PulldownMode->LastGuess)
		{
			PulldownMode->LastGuessCount++;
		}
		else
		{
			PulldownMode->LastGuessCount = 1;
			PulldownMode->LastGuess = BestGuess;
		}
		if(PulldownMode->LastGuessCount > PulldownRepeatCount)
		{
			gPulldownMode = PulldownMode->LastGuess;
			PulldownMode->nCount = PulldownMode->LastGuessCount;
			PulldownMode->LastGuessCount = 0;
			UpdatePulldownStatus();
		}
	}
	else
	{
		// is it not OK to carry on using what we started with
		// then update the structure
		if((Flags1 & !PulldownDetect[gPulldownMode].nFlags1) != !PulldownDetect[gPulldownMode].nFlags1 &&
			(Flags2 & !PulldownDetect[gPulldownMode].nFlags2) > 0)
		{
			ePULLDOWNMODES BestGuess = GuessCorrectNTSCPulldownMode(Flags1, Flags2);
			if(BestGuess == PulldownMode->LastGuess)
			{
				PulldownMode->LastGuessCount++;
				PulldownMode->LastGuess = BestGuess;
			}
			else
			{
				PulldownMode->LastGuessCount = 0;
			}
			PulldownMode->nCount--;
			if(PulldownMode->nCount == 0)
			{
				// if we have got some sort of new lock then jump straight to it
				if(PulldownMode->LastGuessCount > 2)
				{
					gPulldownMode = PulldownMode->LastGuess;
					PulldownMode->nCount = PulldownMode->LastGuessCount;
					PulldownMode->LastGuessCount = 1;
					UpdatePulldownStatus();
				}
				else
				{
					gPulldownMode = VIDEO_MODE;
					PulldownMode->nCount = PulldownMode->LastGuessCount;
					UpdatePulldownStatus();
				}
			}
		}
		else
		{
			PulldownMode->LastGuess = gPulldownMode;
			PulldownMode->LastGuessCount = 0;
			if(PulldownMode->nCount < PulldownRepeatCount)
			{
				PulldownMode->nCount++;
			}
		}
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
	DWORD dwLastCount;
	BYTE* lpCurOverlay = lpOverlayBack;
	//long CombFactors[10];
	short* ppEvenLines[5][CLEARLINES];
	short* ppOddLines[5][CLEARLINES];
	BYTE* pDest;
	//struct TPulldowmMode PulldownMode;
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

	// make sure we start with a clean sheet of paper
	Clean_Overlays();

	// display the current pulldown mode
	UpdatePulldownStatus();
	
	dwLastCount = GetTickCount();

	while(!bStopThread)
	{
		bIsOddField = WaitForNextField(bIsOddField);

		pDest = lpCurOverlay;
		if(bIsOddField)
		{
			//CombFactors[CurrentFrame * 2 + 1] = GetCombFactor(ppEvenLines[CurrentFrame], ppOddLines[CurrentFrame]);
			if(CurrentFrame == 4)
			{
				// update the gPulldownMode based on last set of 5 frames
				// doesn't check for dropped frames but this shouldn't matter
				// too much
				//UpdatePALPulldownMode(&PulldownMode, CombFactors);
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
				nFrame++;
			}
			else if(gPulldownMode == VIDEO_MODE)
			{
				// copy first line in bob way
				memcpyBOBMMX(pDest,
							ppOddLines[CurrentFrame][0], 
							CurrentX * 2);
				// copy each middle odd line and interpolate even lines
				for (nLineTarget = 1; nLineTarget < CurrentY / 2; nLineTarget++)
				{
					VideoDeinterlaceMMX(pDest, 
									ppOddLines[CurrentFrame][nLineTarget - 1], 
									ppEvenLines[CurrentFrame][nLineTarget], 
									ppOddLines[CurrentFrame][nLineTarget], 
									CurrentX * 2);
					pDest += 2 * OverlayPitch;
				}
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
			//CombFactors[CurrentFrame * 2] = GetCombFactor(ppOddLines[(CurrentFrame + 4) % 5], ppEvenLines[CurrentFrame] + 1);

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
				nFrame++;
			}
			else if(gPulldownMode == VIDEO_MODE)
			{
				// copy first line
				memcpyMMX(pDest, 
						ppEvenLines[CurrentFrame][0], 
						CurrentX * 2);
				pDest += OverlayPitch;
				// copy each middle odd line and interpolate even lines
				for (nLineTarget = 1; nLineTarget < CurrentY / 2; nLineTarget++)
				{
					VideoDeinterlaceMMX(pDest, 
									ppEvenLines[CurrentFrame][nLineTarget - 1], 
									ppOddLines[CurrentFrame][nLineTarget - 1], 
									ppEvenLines[CurrentFrame][nLineTarget], 
									CurrentX * 2);
					pDest += 2 * OverlayPitch;
				}
				// copy last line in a bob way
				memcpyMMX(pDest, 
						ppEvenLines[CurrentFrame][CurrentY / 2 - 1], 
						CurrentX * 2);
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
	//long CombFactors[10];
	short* ppEvenLines[5][CLEARLINES];
	short* ppOddLines[5][CLEARLINES];
	BYTE* pDest;
	//struct TPulldowmMode PulldownMode;
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

	// make sure we start with a clean sheet of paper
	Clean_Overlays();

	// display the current pulldown mode
	UpdatePulldownStatus();
	
	dwLastCount = GetTickCount();

	while(!bStopThread)
	{
		bIsOddField = WaitForNextField(bIsOddField);

		pDest = lpCurOverlay;
		if(bIsOddField)
		{
			//CombFactors[CurrentFrame * 2 + 1] = GetCombFactor(ppEvenLines[CurrentFrame], ppOddLines[CurrentFrame]);
			if(CurrentFrame == 4)
			{
				// update the gPulldownMode based on last set of 5 frames
				// doesn't check for dropped frames but this shouldn't matter
				// too much
				//UpdateNTSCPulldownMode(&PulldownMode, CombFactors);
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
				nFrame++;
			}
			else if(gPulldownMode == VIDEO_MODE)
			{
				// copy first line in bob way
				memcpyBOBMMX(pDest,
							ppOddLines[CurrentFrame][0], 
							CurrentX * 2);
				// copy each middle odd line and interpolate even lines
				for (nLineTarget = 1; nLineTarget < CurrentY / 2; nLineTarget++)
				{
					VideoDeinterlaceMMX(pDest, 
									ppOddLines[CurrentFrame][nLineTarget - 1], 
									ppEvenLines[CurrentFrame][nLineTarget], 
									ppOddLines[CurrentFrame][nLineTarget], 
									CurrentX * 2);
					pDest += 2 * OverlayPitch;
				}
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
			//CombFactors[CurrentFrame * 2] = GetCombFactor(ppOddLines[(CurrentFrame + 4) % 5], ppEvenLines[CurrentFrame] + 1);

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
				nFrame++;
			}
			else if(gPulldownMode == VIDEO_MODE)
			{
				// copy first line
				memcpyMMX(pDest, 
						ppEvenLines[CurrentFrame][0], 
						CurrentX * 2);
				pDest += OverlayPitch;
				// copy each middle odd line and interpolate even lines
				for (nLineTarget = 1; nLineTarget < CurrentY / 2; nLineTarget++)
				{
					VideoDeinterlaceMMX(pDest, 
									ppEvenLines[CurrentFrame][nLineTarget - 1], 
									ppOddLines[CurrentFrame][nLineTarget - 1], 
									ppEvenLines[CurrentFrame][nLineTarget], 
									CurrentX * 2);
					pDest += 2 * OverlayPitch;
				}
				// copy last line in a bob way
				memcpyMMX(pDest, 
						ppEvenLines[CurrentFrame][CurrentY / 2 - 1], 
						CurrentX * 2);
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
	ExitThread(0);
	return 0;
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
	if (VBI_Flags > 0 && bIsOddField == FALSE)
	{
		SetEvent(VBI_Event);
	}

	return bIsOddField;
}

