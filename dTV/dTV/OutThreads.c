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
// Refminements made by Mark Rejhon and Steve Grimm
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
// 09 Aug 2000   John Adcock           Changed WaitForNextFrame to use current RISC
//                                     pointer rather than the status flag
//                                     Also changed VBI processing 
//
// 17 Sep 2000   Mark Rejhon           Implemented Steve Grimm's changes
//                                     Some cleanup done.
//                                     Made refinements to Steve Grimm's changes
//
// 02 Jan 2001   John Adcock           Fixed bug at end of GetCombFactor assember
//                                     Made PAL pulldown detect remember last video
//                                     mode
//                                     Removed bTV plug-in
//                                     Added Scaled BOB method
//
// 05 Jan 2001   John Adcock           First attempt at judder fix
//                                     Added loop to make sure that we are never
//                                     too early for a flip
//                                     Changed default for gPulldownMode to 2 frame
//
// 07 Jan 2001   John Adcock           Added Adaptive deinterlacing method
//                                     Split code that did adaptive method
//                                     out of UpdateNTSCPulldownMode
//                                     Added gNTSCFilmFallbackMode setting
//                                     Fixed PAL detection bug
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
#include "other.h"
#include "bt848.h"
#include "VBI_VideoText.h"
#include "vbi.h"
#include "deinterlace.h"
#include "AspectRatio.h"
#include "dtv.h"
#define DOLOGGING
#include "DebugLog.h"
#include "vbi.h"
#include "Settings.h"
#include "Filter.h"
#include "Status.h"
#include "FD_60Hz.h"
#include "FD_50Hz.h"
#include "FD_Common.h"
#include "CPU.h"

// Thread related variables
BOOL                bStopThread = FALSE;
BOOL                bIsPaused = FALSE;
BOOL                bRequestStreamSnap = FALSE;
HANDLE              OutThread;

// Dynamically updated variables
int                 CurrentFrame=0;
BOOL                bAutoDetectMode = TRUE;


// TRB 10/28/00 changes, parms, and new fields for sync problem fixes
DDSURFACEDESC		ddsd;						// also add a surface descriptor for Lock			
HRESULT             FlipResult = 0;             // Need to try again for flip?
BOOL                Wait_For_Flip = TRUE;       // User parm, default=TRUE
BOOL	            DoAccurateFlips = TRUE;     // User parm, default=TRUE
BOOL	            Hurry_When_Late = FALSE;    // " , default=FALSE, skip processing if behind
long				Sleep_Interval = 0;         // " , default=0, how long to wait for BT chip
long				RefreshRate = 0;
BOOL bIsOddField = FALSE;
BOOL bWaitForVsync = FALSE;

// FIXME: should be able to get of this variable
long OverlayPitch = 0;

// Statistics
long nTotalDropFrames = 0;
long nDropFramesLastSec = 0;
long nSecTicks = 0;
long nInitialTicks = -1;
long nLastTicks = 0;
long nTotalDeintModeChanges = 0;

// cope with older DX header files
#if !defined(DDFLIP_DONOTWAIT)
	#define DDFLIP_DONOTWAIT 0
#endif

///////////////////////////////////////////////////////////////////////////////
void Start_Thread()
{
	DWORD LinkThreadID;

	CurrentFrame = 0;

	// make sure we start with a clean sheet of paper
	Overlay_Clean();

	bStopThread = FALSE;

	OutThread = CreateThread((LPSECURITY_ATTRIBUTES) NULL,	// No security.
							 (DWORD) 0,	                    // Same stack size.
							 YUVOutThread,		            // Thread procedure.
							 NULL,	                        // Parameter.
							 (DWORD) 0,	                    // Start immediatly.
							 (LPDWORD) & LinkThreadID);	    // Thread ID.
}

///////////////////////////////////////////////////////////////////////////////
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
		while(i-- > 0 && !Thread_Stopped)
		{
			if (GetExitCodeThread(OutThread, &ExitCode) == TRUE)
			{
				if (ExitCode != STILL_ACTIVE)
				{
					Thread_Stopped = TRUE;
				}
				else
				{
					Sleep(100);
				}
			}
			else
			{
				Thread_Stopped = TRUE;
			}
		}

		if (Thread_Stopped == FALSE)
		{
			TerminateThread(OutThread, 0);
			Sleep(100);
		}
		CloseHandle(OutThread);
		OutThread = NULL;
	}
}

void Pause_Capture()
{
	bIsPaused = TRUE;
}

void UnPause_Capture()
{
	bIsPaused = FALSE;
}

void RequestStreamSnap()
{
   bRequestStreamSnap = TRUE;
}


// save the info structure to a snapshot file
// these files will make it easier to test 
// deinterlacing techniques as we can start
// to exchange the actual data we are each looking
// at and have the ability to recreate results
void SaveStreamSnapshot(DEINTERLACE_INFO *info)
{
	FILE *file;
	char name[13];
	int n = 0;
	int i = 0;
	int j;
	struct stat st;

	while (n < 100)
	{
		sprintf(name,"sn%06d.dtv",++n) ;
		if (stat(name, &st))
			break;
	}

	if(n == 100)
	{
		ErrorBox("Could not create a file.  You may have too many snapshots already.");
		return;
	}

	file = fopen(name,"wb");
	if (!file)
	{
		ErrorBox("Could not open file in SaveStreamSnapshot");
		return;
	}

	// just save the info struct
	// most of the data is pointers which will be useless
	// to anyone else
	// but NULLs will be useful in determining how many
	// fields we have.
	// The rest will contain all the data we need to use
	// the data in a test program
	fwrite(info, sizeof(DEINTERLACE_INFO), 1, file);

	// save all the Odd fields first
	i = 0;
	while(i < MAX_FIELD_HISTORY && info->OddLines[i] != NULL)
	{
		for(j = 0; j < info->FieldHeight; ++j)
		{
			fwrite(info->OddLines[i][j], info->LineLength, 1, file);
		}
		i++;      
	}

	// then all the even frames
	i = 0;
	while(i < MAX_FIELD_HISTORY && info->EvenLines[i] != NULL)
	{
		for(j = 0; j < info->FieldHeight; ++j)
		{
			fwrite(info->OddLines[i][j], info->LineLength, 1, file);
		}
	i++;      
	}
	fclose(file);
}

void Pause_Toggle_Capture()
{
	if(bIsPaused)
	{
		UnPause_Capture();
	}
	else
	{
		Pause_Capture();
	}
}

///////////////////////////////////////////////////////////////////////////////
void Start_Capture()
{
	int nFlags = BT848_CAP_CTL_CAPTURE_EVEN | BT848_CAP_CTL_CAPTURE_ODD;
	if (Capture_VBI == TRUE)
	{
		nFlags |= BT848_CAP_CTL_CAPTURE_VBI_EVEN | BT848_CAP_CTL_CAPTURE_VBI_ODD;
	}

	BT848_MaskDataByte(BT848_CAP_CTL, 0, 0x0f);

	BT848_CreateRiscCode(nFlags);
	BT848_MaskDataByte(BT848_CAP_CTL, (BYTE) nFlags, (BYTE) 0x0f);
	BT848_SetDMA(TRUE);

	// ame sure half height modes are set correctly
	PrepareDeinterlaceMode();

	Start_Thread();
}

///////////////////////////////////////////////////////////////////////////////
void Stop_Capture()
{
	//  Stop The Output Thread
	Stop_Thread();

	// stop capture
	BT848_MaskDataByte(BT848_CAP_CTL, 0, 0x0f);
}

///////////////////////////////////////////////////////////////////////////////
void Reset_Capture()
{
	Stop_Capture();
	Overlay_Clean();
	BT848_ResetHardware();
	BT848_SetGeoSize();
	WorkoutOverlaySize();
	Start_Capture();
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
BOOL WaitForNextField(BOOL LastField, BOOL* RunningLate)
{
	BOOL bIsOddField;
	int OldPos = (CurrentFrame * 2 + LastField + 1) % 10;
	*RunningLate = TRUE;
	while(OldPos == BT848_GetRISCPosAsInt())
	{
		if(!DoAccurateFlips)
		{
			Sleep(Sleep_Interval);
		}
		*RunningLate = FALSE;			// if we waited then we are not late
	}

	switch(BT848_GetRISCPosAsInt())
	{
	case 0: bIsOddField = TRUE;  CurrentFrame = 4; break;
	case 1: bIsOddField = FALSE; CurrentFrame = 0; break;
	case 2: bIsOddField = TRUE;  CurrentFrame = 0; break;
	case 3: bIsOddField = FALSE; CurrentFrame = 1; break;
	case 4: bIsOddField = TRUE;  CurrentFrame = 1; break;
	case 5: bIsOddField = FALSE; CurrentFrame = 2; break;
	case 6: bIsOddField = TRUE;  CurrentFrame = 2; break;
	case 7: bIsOddField = FALSE; CurrentFrame = 3; break;
	case 8: bIsOddField = TRUE;  CurrentFrame = 3; break;
	case 9: bIsOddField = FALSE; CurrentFrame = 4; break;
	}

	return bIsOddField;
}

//
// Add a function to Lock the overlay surface and update some info from it.
// We always lock and write to the back buffer.
// Flipping takes care of the proper buffer addresses.
// Some of this info can change each time.  
// We also check to see if we still need to Flip because the
// non-waiting last flip failed.  If so, try it one more time,
// then give up.  Tom Barry 10/26/00
//
BYTE* LockOverlay()
{
	HRESULT ddrval;

	if (FAILED(FlipResult))				// prev flip was busy?
	{
		ddrval = IDirectDrawSurface_Flip(lpDDOverlay, NULL, DDFLIP_DONOTWAIT);  
		if(ddrval == DDERR_SURFACELOST)
		{
			return NULL;
		}
		FlipResult = 0;					// but no time to try any more
	}

	memset(&ddsd, 0x00, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddrval = IDirectDrawSurface_Lock(lpDDOverlayBack, NULL, &ddsd, 
		DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL);
	if(FAILED(ddrval))
	{
		return NULL;
	}

	OverlayPitch = ddsd.lPitch;			// Set new pitch, may change
	return ddsd.lpSurface;
}

DWORD WINAPI YUVOutThread(LPVOID lpThreadParameter)
{
	char Text[128];
	int i, j;
	int nLineTarget;
	int nFrame = 0;
	DWORD dwLastSecondTicks;
	short* ppEvenLines[5][DTV_MAX_HEIGHT / 2];
	short* ppOddLines[5][DTV_MAX_HEIGHT / 2];
	BYTE* pDest;
	int LastEvenFrame = 0;
	int LastOddFrame = 0;
	BOOL bFlipNow = TRUE;
	BOOL bMissedFrame;
	HRESULT ddrval;
	DEINTERLACE_INFO info;
	DWORD FlipFlag;
	DEINTERLACE_METHOD* PrevDeintMethod = NULL;
	DEINTERLACE_METHOD* CurrentMethod = NULL;
	BOOL FlipAdjust;
	LARGE_INTEGER TimerFrequency;
	BOOL bIsPAL = BT848_GetTVFormat()->Is25fps;
	double RunningAverageCounterTicks;
	double StartAverageCounterTicks;
	LARGE_INTEGER LastFieldTime;
	LARGE_INTEGER CurrentFieldTime;
	LARGE_INTEGER LastFlipTime;
	LARGE_INTEGER CurrentFlipTime;
	BOOL RunningLate = FALSE;
	double Weight = 0.005;
	DWORD CurrentTickCount;
	int nHistory = 0;

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

	// set up Deinterlace Info struct
	memset(&info, 0, sizeof(info));
	info.CpuFeatureFlags = CpuFeatureFlags;
	if(CpuFeatureFlags & FEATURE_SSE)
	{
		info.pMemcpy = memcpySSE;
	}
	else
	{
		info.pMemcpy = memcpyMMX;
	}

	// catch anything fatal in this loop so we don't crash the machine
	__try
	{
		if (lpDDOverlay == NULL || lpDDOverlayBack == NULL)
		{
			LOG(" No Overlay surface Created");
			ExitThread(-1);
		}

		// Sets processor Affinity and Thread priority according to menu selection
		SetThreadProcessorAndPriority();

		// Set up 5 sets of pointers to the start of odd and even lines
		for (j = 0; j < 5; j++)
		{
			for (i = 0; i < CurrentY; i += 2)
			{
				ppOddLines[j][i / 2] = (short *) pDisplay[j] + (i + 1) * 1024;
				ppEvenLines[j][i / 2] = (short *) pDisplay[j] + i * 1024;
			}
		}
		PrevDeintMethod = GetCurrentDeintMethod();

		// reset the static variables in the detection code
		if (bIsPAL)
			UpdatePALPulldownMode(NULL);
		else
			UpdateNTSCPulldownMode(NULL);

		// start the capture off
		BT848_Restart_RISC_Code();

		dwLastSecondTicks = GetTickCount();
		while(!bStopThread)
		{
			// update with any changes
			CurrentMethod = GetCurrentDeintMethod();
			
			info.IsOdd = WaitForNextField(info.IsOdd, &RunningLate);
			if(DoAccurateFlips)
			{
				// we've just got a new field
				// we are going to time the odd to odd
				// input frequency
				if(info.IsOdd)
				{
					QueryPerformanceCounter(&CurrentFieldTime);
					if(!RunningLate)
					{
						// if we're not running late then
						// and we got a good clean run last time
						if(LastFieldTime.QuadPart != 0)
						{
							// gets the last ticks odd - odd
							double RecentTicks = (double)(CurrentFieldTime.QuadPart - LastFieldTime.QuadPart);
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
						LastFieldTime.QuadPart = CurrentFieldTime.QuadPart;
					}
					else
					{
						// if we're running late then
						// time will be rubbish
						// so make sure it won't be used
						LastFieldTime.QuadPart = 0;
					}
				}
				// we have to sleep somewhere might as well be here
				// since we don't sleep anywhere in the WaitForNextField
				// anymore
				Sleep(Sleep_Interval);
			}
			
			// if we don't have Hurry_When_Late set
			// then make sure we don't hurry as we're
			// assuming we'll catch up
			if(!Hurry_When_Late)
			{
				RunningLate = FALSE;
			}

			if(bIsPaused == FALSE)
			{
				info.CurrentFrame = CurrentFrame;
				info.OverlayPitch = OverlayPitch;
				info.LineLength = CurrentX * 2;
				info.FrameWidth = CurrentX;
				info.FrameHeight = CurrentY;
				info.FieldHeight = CurrentY / 2;
				info.CombFactor = -1;
				info.FieldDiff = -1;

				bMissedFrame = FALSE;
				bFlipNow = FALSE;

				// if we are trying to do the flips properly then we may
				// get behind on processsing and so need to be able to catch up
				// from at least a lag of two whole fields
				// if we find we are behind then set the FlipAdjust
				// to ask for a speed up later on
				if(DoAccurateFlips)
				{
					if(info.IsOdd)
					{
						if((LastEvenFrame + 1) % 5 == CurrentFrame &&
							(LastOddFrame + 1) % 5 == CurrentFrame)
						{
							info.IsOdd = FALSE;
							LOG(" Slightly late");
							FlipAdjust = TRUE;
						}
						if((LastEvenFrame + 1) % 5 == CurrentFrame &&
							(LastOddFrame + 2) % 5 == CurrentFrame)
						{
							CurrentFrame = (CurrentFrame + 4) % 5;
							info.CurrentFrame = CurrentFrame;
							LOG(" Very late");
							FlipAdjust = TRUE;
						}
					}
					else
					{
						if((LastEvenFrame + 1) % 5 == CurrentFrame &&
								(LastOddFrame + 2) % 5 == CurrentFrame)
						{
							info.IsOdd = TRUE;
							LOG(" Slightly late");
							CurrentFrame = (CurrentFrame + 4) % 5;
							info.CurrentFrame = CurrentFrame;
							FlipAdjust = TRUE;
						}
						if((LastEvenFrame + 2) % 5 == CurrentFrame &&
								(LastOddFrame + 2) % 5 == CurrentFrame)
						{
							CurrentFrame = (CurrentFrame + 4) % 5;
							info.CurrentFrame = CurrentFrame;
							FlipAdjust = TRUE;
							LOG(" Very late");
						}
					}
				}
				
				if(info.IsOdd)
				{
					// If we skipped the previous field, note the missing field in the deinterlace
					// info structure and force this field to be bobbed.
					if (LastEvenFrame != CurrentFrame)
					{
						memmove(&info.EvenLines[1], &info.EvenLines[0], sizeof(info.EvenLines) - sizeof(info.EvenLines[0]));
						info.EvenLines[0] = NULL;
						bMissedFrame = TRUE;
						nFrame++;
						LOG(" Dropped Frame");
						// if we dropped a frame
						// then it seems a good idea
						// to start again with the running average
						// we could have changed inputs had a glitch
						// of some sort, done some ff frew
						if(DoAccurateFlips)
						{
							RunningAverageCounterTicks = StartAverageCounterTicks;
						}
					}
					memmove(&info.OddLines[1], &info.OddLines[0], sizeof(info.OddLines) - sizeof(info.OddLines[0]));
					info.OddLines[0] = ppOddLines[CurrentFrame];
					LastOddFrame = CurrentFrame;
				}
				else
				{
					if(LastOddFrame != ((CurrentFrame + 4) % 5))
					{
						memmove(&info.OddLines[1], &info.OddLines[0], sizeof(info.OddLines) - sizeof(info.OddLines[0]));
						info.OddLines[0] = NULL;
						bMissedFrame = TRUE;
						nFrame++;
						LOG(" Dropped Frame");
						// if we dropped a frame
						// then it seems a good idea
						// to start again with the running average
						// we could have changed inputs had a glitch
						// of some sort, done some ff frew
						if(DoAccurateFlips)
						{
							RunningAverageCounterTicks = StartAverageCounterTicks;
						}
					}
					memmove(&info.EvenLines[1], &info.EvenLines[0], sizeof(info.EvenLines) - sizeof(info.EvenLines[0]));
					info.EvenLines[0] = ppEvenLines[CurrentFrame];
					LastEvenFrame = CurrentFrame;
				}
				// update the source area
				GetSourceRect(&info.SourceRect);
				
				// if we're running late then tell the user
				if (RunningLate)
				{
					nFrame++;
					LOG("Running Late");
				}
				
				// do any filters that operarate on the input
				// only
				Filter_DoInput(&info, (RunningLate || bMissedFrame));

				if(!bMissedFrame)
				{
					if(bAutoDetectMode == TRUE && bIsPAL)
					{
						UpdatePALPulldownMode(&info);
					}

					if(bAutoDetectMode == TRUE && !bIsPAL)
					{
						UpdateNTSCPulldownMode(&info);
					}
					
					if(CurrentMethod->bNeedCombFactor && info.CombFactor == -1)
					{
						GetCombFactor(&info);
					}

					if(CurrentMethod->bNeedFieldDiff && info.FieldDiff == -1)
					{
						CompareFields(&info);
					}
				}

				if (Capture_VBI == TRUE)
				{
					BYTE * pVBI = (LPBYTE) pVBILines[CurrentFrame];
					if (info.IsOdd)
					{
						pVBI += CurrentVBILines * 2048;
					}
					for (nLineTarget = 0; nLineTarget < CurrentVBILines ; nLineTarget++)
					{
						VBI_DecodeLine(pVBI + nLineTarget * 2048, nLineTarget, info.IsOdd);
					}
				}

				__try
				{
					if (!RunningLate)
					{
						pDest = LockOverlay();	// Ready to access screen, Lock back buffer berfore accessing
												// can't do this until after Lock Call
						if(pDest == NULL)
						{
							PostMessage(hWnd, WM_COMMAND, IDM_OVERLAY_STOP, 0);
							PostMessage(hWnd, WM_COMMAND, IDM_OVERLAY_START, 0);
							LOG(" Falling out after LockOverlay");
							ExitThread(1);
							return 0;
						}
						info.Overlay = pDest;
					}

					if(info.IsOdd)
					{
						if(info.EvenLines[0] == NULL)
						{
							nHistory = 1;
						}
						else if(info.OddLines[0] == NULL)
						{
							nHistory = 2;
						}
						else if(info.EvenLines[1] == NULL)
						{
							nHistory = 3;
						}
						else
						{
							nHistory = 4;
						}
					}
					else
					{
						if(info.OddLines[0] == NULL)
						{
							nHistory = 1;
						}
						else if(info.EvenLines[0] == NULL)
						{
							nHistory = 2;
						}
						else if(info.OddLines[1] == NULL)
						{
							nHistory = 3;
						}
						else
						{
							nHistory = 4;
						}
					}

					if (RunningLate)
					{
						;     // do nothing
					}
					// if we have dropped a field then do BOB 
					// or if we need to get more history
					// if we are doing a half height mode then just do that
					// anyway as it will be just as fast
					else if(!CurrentMethod->bIsHalfHeight && (bMissedFrame || nHistory < CurrentMethod->nFieldsRequired))
					{
						bFlipNow = Bob(&info);
					}
					// When we first detect film mode we will be on the right flip mode in PAL
					// and at the end of a three series in NTSC this will be the starting point for
					// our 2.5 field timings
					else if(PrevDeintMethod != CurrentMethod && IsFilmMode())
					{
						bFlipNow = Weave(&info);
					}
					else
					{
						bFlipNow = CurrentMethod->pfnAlgorithm(&info);
					}
					
					AdjustAspectRatio(ppEvenLines[LastEvenFrame], ppOddLines[LastOddFrame]);
				}					
				// if there is any exception thrown in the above then just carry on
				__except (EXCEPTION_EXECUTE_HANDLER) 
				{ 
					LOG(" Crash in output code");
				}

				// somewhere above we will have locked the buffer, unlock before flip
				if (!RunningLate)
				{
					ddrval = IDirectDrawSurface_Unlock(lpDDOverlayBack, NULL);
					if(ddrval == DDERR_SURFACELOST)
					{
						PostMessage(hWnd, WM_COMMAND, IDM_OVERLAY_STOP, 0);
						PostMessage(hWnd, WM_COMMAND, IDM_OVERLAY_START, 0);
						LOG(" Falling out after Surface Unlock");
						ExitThread(1);
						return 0;
					}

					if (bFlipNow)
					{
						// Do any filters that run on the output
						Filter_DoOutput(&info, (RunningLate || bMissedFrame));

						// setup flip flag
						// the odd and even flags may help the scaled bob
						// on some cards
						FlipFlag = (Wait_For_Flip)?DDFLIP_WAIT:DDFLIP_DONOTWAIT;
						if(CurrentMethod->nMethodIndex == INDEX_SCALER_BOB)
						{
							FlipFlag |= (info.IsOdd)?DDFLIP_ODD:DDFLIP_EVEN;
						}

						// Need to wait for a good time to flip
						// only if we have been in the same mode for at least one flip
						if(DoAccurateFlips && PrevDeintMethod == CurrentMethod)
						{
							LONGLONG TicksToWait;
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
							if(bMissedFrame == FALSE && FlipAdjust == FALSE)
							{
								while(!bStopThread && (CurrentFlipTime.QuadPart - LastFlipTime.QuadPart) < TicksToWait)
								{
									QueryPerformanceCounter(&CurrentFlipTime);
								}
							}
						}

						FlipResult = IDirectDrawSurface_Flip(lpDDOverlay, NULL, FlipFlag); 
						if(FlipResult == DDERR_SURFACELOST)
						{
							PostMessage(hWnd, WM_COMMAND, IDM_OVERLAY_STOP, 0);
							PostMessage(hWnd, WM_COMMAND, IDM_OVERLAY_START, 0);
							LOG(" Falling out after flip");
							ExitThread(1);
							return 0;
						}

						if(DoAccurateFlips)
						{
							FlipAdjust = FALSE;
							LastFlipTime.QuadPart = CurrentFlipTime.QuadPart;
						}
					}
				}
			}
			
			CurrentTickCount = GetTickCount();
			if (dwLastSecondTicks + 1000 < CurrentTickCount)
			{
				nTotalDropFrames += nFrame;
				nDropFramesLastSec = nFrame;
				nFrame = 0;
				nSecTicks += CurrentTickCount - dwLastSecondTicks;
				dwLastSecondTicks = CurrentTickCount;
				CurrentMethod->ModeTicks += CurrentTickCount - nLastTicks;
				nLastTicks = CurrentTickCount;
				if (IsStatusBarVisible())
				{
					sprintf(Text, "%d DF/S", nDropFramesLastSec);
					StatusBar_ShowText(STATUS_FPS, Text);
				}
			}

         // if asked save the current info to a file
         if(bRequestStreamSnap == TRUE)
         {
            SaveStreamSnapshot(&info);
			bRequestStreamSnap = FALSE;
         }

			// save the last pulldown mode so that we know if its changed
			PrevDeintMethod = CurrentMethod;
		}

		BT848_SetDMA(FALSE);
	}
	// if there is any exception thrown then exit the thread
	__except (EXCEPTION_EXECUTE_HANDLER) 
    { 
		LOG(" Crash in OutThreads main loop");
		ExitThread(1);
		return 0;
	}
	// end of __try loop
    
	ExitThread(0);
	return 0;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING OutThreadsSettings[OUTTHREADS_SETTING_LASTONE] =
{
	{
		"Hurry When Late", ONOFF, 0, &Hurry_When_Late,
		FALSE, 0, 1, 1, 1,
		NULL,
		"Threads", "Hurry_When_Late", NULL,
	},
	{
		"Wait For Flip", ONOFF, 0, &Wait_For_Flip,
		TRUE, 0, 1, 1, 1,
		NULL,
		"Threads", "Wait_For_Flip", NULL,
	},
	{
		"Do Accurate Flips", ONOFF, 0, &DoAccurateFlips,
		FALSE, 0, 1, 1, 1,
		NULL,
		"Threads", "DoAccurateFlips", NULL,
	},
	{
		"Sleep Interval", SLIDER, 0, &Sleep_Interval,
		0, 0, 100, 1, 1,
		NULL,
		"Threads", "Sleep_Interval", NULL,
	},
	{
		"Auto Detect Mode", ONOFF, 0, &bAutoDetectMode,
		TRUE, 0, 1, 1, 1,
		NULL,
		"Pulldown", "bAutoDetectMode", NULL,
	},
	// don't use gPulldownMethiod anymore
	{
		NULL, ITEMFROMLIST, 0, NULL,
		0, 0, 0, 1, 1,
		NULL,
		NULL, NULL, NULL,
	},
	{
		"Refresh Rate", SLIDER, 0, &RefreshRate,
		0, 0, 120, 1, 1,
		NULL,
		"Pulldown", "RefreshRate", NULL,
	},
	{
		"Wait For VSync", ONOFF, 0, &bWaitForVsync,
		FALSE, 0, 1, 1, 1,
		NULL,
		"Threads", "bWaitForVsync", NULL,
	},
};

SETTING* OutThreads_GetSetting(OUTTHREADS_SETTING Setting)
{
	if(Setting > -1 && Setting < OUTTHREADS_SETTING_LASTONE)
	{
		return &(OutThreadsSettings[Setting]);
	}
	else
	{
		return NULL;
	}
}

void OutThreads_ReadSettingsFromIni()
{
	int i;
	for(i = 0; i < OUTTHREADS_SETTING_LASTONE; i++)
	{
		Setting_ReadFromIni(&(OutThreadsSettings[i]));
	}
}

void OutThreads_WriteSettingsToIni()
{
	int i;
	for(i = 0; i < OUTTHREADS_SETTING_LASTONE; i++)
	{
		Setting_WriteToIni(&(OutThreadsSettings[i]));
	}
}

void OutThreads_SetMenu(HMENU hMenu)
{
	CheckMenuItem(hMenu, IDM_CAPTURE_PAUSE, bIsPaused?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_AUTODETECT, bAutoDetectMode?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_JUDDERTERMINATOR, DoAccurateFlips?MF_CHECKED:MF_UNCHECKED);
}
