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
#include "FLT_TNoise.h"
#include "Status.h"
#include "FD_60Hz.h"
#include "FD_50Hz.h"
#include "FD_Common.h"

// Thread related variables
BOOL                bStopThread = FALSE;
BOOL                bIsPaused = FALSE;
HANDLE              OutThread;

// Dynamically updated variables
ePULLDOWNMODES      gPulldownMode = VIDEO_MODE_2FRAME;
int                 CurrentFrame=0;
DWORD               dwLastFlipTicks = -1;


// TRB 10/28/00 changes, parms, and new fields for sync problem fixes
DDSURFACEDESC		ddsd;						// also add a surface descriptor for Lock			
BOOL				RunningLate = FALSE;        // Set when we are not keeping up
HRESULT             FlipResult = 0;             // Need to try again for flip?
BOOL                Wait_For_Flip = TRUE;       // User parm, default=TRUE
BOOL	            DoAccurateFlips = TRUE;     // User parm, default=TRUE
BOOL	            Hurry_When_Late = FALSE;    // " , default=FALSE, skip processing if behind
long				Sleep_Interval = 0;         // " , default=0, how long to wait for BT chip
BOOL bIsOddField = FALSE;

// FIXME: should be able to get of this variable
long OverlayPitch = 0;


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
BOOL WaitForNextField(BOOL LastField)
{
	BOOL bIsOddField;
	int OldPos = (CurrentFrame * 2 + LastField + 1) % 10;
	RunningLate = Hurry_When_Late;        // user specified bool parm
	while(OldPos == BT848_GetRISCPosAsInt())
	{
		Sleep(Sleep_Interval);
		RunningLate = FALSE;			// if we waited then we are not late
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

///////////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////
// SetDeinterlaceMode
//
// Sets the deinterlace mode as a result of a menu selection.  This turns off
// autodetection, updates the mode indicator, etc.
///////////////////////////////////////////////////////////////////////////////
void SetDeinterlaceMode(int mode)
{
	gPulldownMode = mode;
	UpdatePulldownStatus();
	SetHalfHeight(DeintMethods[mode].bIsHalfHeight);
}

///////////////////////////////////////////////////////////////////////////////
// Translates a deinterlace mode name to human-readable form.
// Parameter: mode to translate, or -1 to return name of current mode.
char *DeinterlaceModeName(int mode)
{
	if (mode < 0)
		mode = gPulldownMode;

	if(mode < PULLDOWNMODES_LAST_ONE)
		return DeintMethods[mode].szName;

	return "Unknown Pulldown Mode";
}

///////////////////////////////////////////////////////////////////////////////
// Updates the pulldown mode status indicator in the window footer if the mode
// is different than the one currently listed there.
void UpdatePulldownStatus()
{
	static ePULLDOWNMODES lastPulldownMode = PULLDOWNMODES_LAST_ONE;

	if (gPulldownMode != lastPulldownMode)
	{
		StatusBar_ShowText(STATUS_PAL, DeinterlaceModeName(gPulldownMode));
		lastPulldownMode = gPulldownMode;
	}
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
		IDirectDrawSurface_Flip(lpDDOverlay, NULL, DDFLIP_DONOTWAIT);  
		FlipResult = 0;					// but no time to try any more
	}

	memset(&ddsd, 0x00, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddrval = IDirectDrawSurface_Lock(lpDDOverlayBack, NULL, &ddsd, 
		DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL);
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
	long FlipTicks;
	short* ppEvenLines[5][DTV_MAX_HEIGHT / 2];
	short* ppOddLines[5][DTV_MAX_HEIGHT / 2];
	BYTE* pDest;
	int LastEvenFrame = 0;
	int LastOddFrame = 0;
	int CombNum = 0;
	BOOL bFlipNow = TRUE;
	BOOL bMissedFrame;
	HRESULT ddrval;
	DEINTERLACE_INFO info;
	DWORD FlipFlag;
	ePULLDOWNMODES PrevPulldownMode = PULLDOWNMODES_LAST_ONE;
	DWORD RefreshRate;

	BOOL bIsPAL = TVSettings[TVTYPE].Is25fps;

	RefreshRate = GetRefreshRate();

	if (lpDDOverlay == NULL || lpDDOverlayBack == NULL)
	{
		ExitThread(-1);
	}

	// Sets processor Affinity and Thread priority according to menu selection
	SetupProcessorAndThread();

	// Set up 5 sets of pointers to the start of odd and even lines
	for (j = 0; j < 5; j++)
	{
		for (i = 0; i < CurrentY; i += 2)
		{
			ppOddLines[j][i / 2] = (short *) pDisplay[j] + (i + 1) * 1024;
			ppEvenLines[j][i / 2] = (short *) pDisplay[j] + i * 1024;
		}
	}

	PrevPulldownMode = gPulldownMode;

	// reset the static variables in the detection code
	if (bIsPAL)
		UpdatePALPulldownMode(NULL);
	else
		UpdateNTSCPulldownMode(NULL);

	memset(&info, 0, sizeof(info));

	// display the current pulldown mode
	UpdatePulldownStatus();
	
	// start the capture off
	BT848_Restart_RISC_Code();

	dwLastSecondTicks = GetTickCount();
	while(!bStopThread)
	{
		info.IsOdd = WaitForNextField(info.IsOdd);
		if(bIsPaused == FALSE)
		{
			info.OverlayPitch = OverlayPitch;
			info.LineLength = CurrentX * 2;
			info.FrameWidth = CurrentX;
			info.FrameHeight = CurrentY;
			info.FieldHeight = CurrentY / 2;

			bMissedFrame = FALSE;
			bFlipNow = FALSE;

			if(info.IsOdd)
			{
				memmove(&info.OddLines[1], &info.OddLines[0], sizeof(info.OddLines) - sizeof(info.OddLines[0]));
				info.OddLines[0] = ppOddLines[CurrentFrame];
				LastOddFrame = CurrentFrame;

				// If we skipped the previous field, note the missing field in the deinterlace
				// info structure and force this field to be bobbed.
				if (LastEvenFrame != CurrentFrame)
				{
					// in film mode in the 60Hz mode
					// we might wait quite a long time after doing a flip
					// on the 2 field part of the 3:2 pulldown
					// we might then get a single frame behind
					// we need to cope with this so we fill the info struct properly
					// rather than dropping a frame
					if(DoAccurateFlips && DeintMethods[gPulldownMode].bIsFilmMode && !bIsPAL &&
						(LastEvenFrame + 1) % 5 == CurrentFrame)
					{
						memmove(&info.EvenLines[1], &info.EvenLines[0], sizeof(info.EvenLines) - sizeof(info.EvenLines[0]));
						info.EvenLines[0] = ppEvenLines[CurrentFrame];
						LastEvenFrame = CurrentFrame;
					}
					else
					{
						memmove(&info.EvenLines[1], &info.EvenLines[0], sizeof(info.EvenLines) - sizeof(info.EvenLines[0]));
						info.EvenLines[0] = NULL;
						bMissedFrame = TRUE;
						nFrame++;
					}
				}
			}
			else
			{
				memmove(&info.EvenLines[1], &info.EvenLines[0], sizeof(info.EvenLines) - sizeof(info.EvenLines[0]));
				info.EvenLines[0] = ppEvenLines[CurrentFrame];
				LastEvenFrame = CurrentFrame;

				// If we skipped the previous field, note the missing field in the deinterlace
				// info structure and force this field to be bobbed.
				if(LastOddFrame != ((CurrentFrame + 4) % 5))
				{
					// in film mode in the 60Hz mode
					// we might wait quite a long time after doing a flip
					// on the 2 field part of the 3:2 pulldown
					// we might then get a single frame behind
					// we need to cope with this so we fill the info struct properly
					// rather than dropping a frame
					if(DoAccurateFlips && DeintMethods[gPulldownMode].bIsFilmMode && !bIsPAL &&
						(LastOddFrame + 2) % 5 == CurrentFrame)
					{
						memmove(&info.OddLines[1], &info.OddLines[0], sizeof(info.OddLines) - sizeof(info.OddLines[0]));
						info.OddLines[0] = ppOddLines[((CurrentFrame + 4) % 5)];
						LastOddFrame = CurrentFrame;
					}
					else
					{
						memmove(&info.OddLines[1], &info.OddLines[0], sizeof(info.OddLines) - sizeof(info.OddLines[0]));
						info.OddLines[0] = NULL;
						bMissedFrame = TRUE;
						nFrame++;
					}
				}
			}
			// update the source area
			GetSourceRect(&info.SourceRect);

			if(!bMissedFrame)
			{
				if((bAutoDetectMode == TRUE && bIsPAL) || DeintMethods[gPulldownMode].bRequiresCombFactor)
				{
					info.CombFactor = GetCombFactor(&info);
					LOG(" Frame %d %c CF = %d", CurrentFrame, info.IsOdd ? 'O' : 'E', info.CombFactor);
				}

				if((bAutoDetectMode == TRUE && !bIsPAL) || DeintMethods[gPulldownMode].bRequiresFieldDiff)
				{
					info.FieldDiff = CompareFields(&info);
					
					LOG(" Frame %d %c CR = %d", CurrentFrame, info.IsOdd ? 'O' : 'E', info.FieldDiff);
				}

				if(bAutoDetectMode == TRUE && bIsPAL)
				{
					UpdatePALPulldownMode(&info);
				}

				if(bAutoDetectMode == TRUE && !bIsPAL)
				{
					UpdateNTSCPulldownMode(&info);
				}
			}

			if (!RunningLate && Capture_VBI == TRUE)
			{
				BYTE * pVBI = (LPBYTE) pVBILines[CurrentFrame];
				if (info.IsOdd)
				{
					pVBI += CurrentVBILines * 2048;
				}
				for (nLineTarget = 0; nLineTarget < CurrentVBILines ; nLineTarget++)
				{
					VBI_DecodeLine(pVBI + nLineTarget * 2048, nLineTarget);
				}
			}

			if (!RunningLate)
			{
				pDest = LockOverlay();	// Ready to access screen, Lock back buffer berfore accessing
										// can't do this until after Lock Call
				info.Overlay = pDest;

				NoiseFilter_Temporal(&info);
			}

			if (RunningLate)
			{
				;     // do nothing
			}
			// if we have dropped a field then do BOB 
			// if we are doing a half height mode then just do that
			// anyway as it will be just as fast
			else if(bMissedFrame && !DeintMethods[gPulldownMode].bIsHalfHeight)
			{
				bFlipNow = Bob(&info);
			}
			// When we first detect film mode we will be on the right flip mode in PAL
			// and at the end of a three series in NTSC this will be the starting point for
			// our 2.5 field timings
			else if(PrevPulldownMode != gPulldownMode && DeintMethods[gPulldownMode].bIsFilmMode)
			{
				bFlipNow = Weave(&info);
			}
			else
			{
				bFlipNow = DeintMethods[gPulldownMode].pfnAlgorithm(&info);
			}
			
			AdjustAspectRatio(ppEvenLines[LastEvenFrame], ppOddLines[LastOddFrame]);

			// somewhere above we will have locked the buffer, unlock before flip
			if (!RunningLate)
			{
				ddrval = IDirectDrawSurface_Unlock(lpDDOverlayBack, NULL);

				if (bFlipNow)
				{
					// Need to wait for a good time to flip
					// only if we have been in the same mode for at least one flip
					if(DoAccurateFlips && PrevPulldownMode == gPulldownMode && RefreshRate > 0)
					{
						if(bIsPAL)
						{
							if(RefreshRate % DeintMethods[gPulldownMode].FrameRate50Hz == 0)
							{
								while((GetTickCount() - FlipTicks) < (1000 / DeintMethods[gPulldownMode].FrameRate50Hz - 3));
							}
						}
						else
						{
							if(RefreshRate % DeintMethods[gPulldownMode].FrameRate60Hz == 0)
							{
								while((GetTickCount() - FlipTicks) < (1000/DeintMethods[gPulldownMode].FrameRate60Hz - 3));
							}
						}
					}

					// setup flip flag
					// the odd and even flags may help the scaled bob
					// on some cards
					FlipFlag = (Wait_For_Flip)?DDFLIP_WAIT:DDFLIP_DONOTWAIT;
					if(gPulldownMode == SCALER_BOB)
					{
						FlipFlag |= (info.IsOdd)?DDFLIP_ODD:DDFLIP_EVEN;
					}
					FlipResult = IDirectDrawSurface_Flip(lpDDOverlay, NULL, FlipFlag); 
					LOG(" Flip ms (%dms)", timeGetTime() % 1000);
					FlipTicks = GetTickCount();
					if (dwLastFlipTicks > -1 && FlipTicks - dwLastFlipTicks > (1000 / 23))
					{
						// We should always be running at 24fps or greater.  Check for
						// 23fps instead to allow some wiggle room, but if more than
						// 1/23 of a second has passed since the last flip, something's
						// screwy.
						LOG(" Long time since last flip (%dms)", dwLastFlipTicks - FlipTicks);
					}
					dwLastFlipTicks = FlipTicks;
				}
			}
		}
		
		// save the last pulldown mode so that we know if its changed
		PrevPulldownMode = gPulldownMode;

		if (bDisplayStatusBar == TRUE)
		{
			if (dwLastSecondTicks + 1000 < GetTickCount())
			{
				sprintf(Text, "%d DF/S", nFrame);
				StatusBar_ShowText(STATUS_FPS, Text);
				nFrame = 0;
				dwLastSecondTicks = GetTickCount();
			 }
		}
	}

	BT848_SetDMA(FALSE);
	ExitThread(0);
	return 0;
}
