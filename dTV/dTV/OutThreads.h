/////////////////////////////////////////////////////////////////////////////
// OutThreads.h
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
// 07 Jan 2001   John Adcock           Added gNTSCFilmFallbackMode setting
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __OUTTHREADS_H___
#define __OUTTHREADS_H___

#include "deinterlace.h"

#define MAXMODESWITCHES 50	// Maximum number of switches to track in TrackModeSwitches()


void Start_Capture();
void Stop_Capture();
void Reset_Capture();

void Start_Thread();
void Stop_Thread();

void SetupCaptureFlags();

DWORD WINAPI YUVOutThread(LPVOID lpThreadParameter);

void UpdatePALPulldownMode(long CombFactor, BOOL IsOddField);
void UpdateNTSCPulldownMode(long FieldDiff, BOOL OnOddField, short **evenField, short **oddField);

BOOL DoWeWantToFlip(BOOL bIsOddField);
void UpdatePulldownStatus();
BOOL WaitForNextField(BOOL LastField);
void SetDeinterlaceMode(int mode);
char *DeinterlaceModeName(int mode);
BYTE * LockOverlay();		// Lock overlay, returns ptr to buffer

extern ePULLDOWNMODES gPulldownMode;
extern ePULLDOWNMODES gPALFilmFallbackMode;
extern ePULLDOWNMODES gNTSCFilmFallbackMode;
extern ePULLDOWNMODES StaticImageMode;
extern ePULLDOWNMODES LowMotionMode;
extern ePULLDOWNMODES HighMotionMode;

extern long PulldownThresholdLow;
extern long PulldownThresholdHigh;
extern long PulldownRepeatCount;
extern long PulldownRepeatCount2;
extern DWORD dwLastFlipTicks;

extern long PulldownSwitchInterval;
extern long PulldownSwitchMax;

extern long Threshold32Pulldown;
extern long ThresholdPulldownMismatch;
extern long ThresholdPulldownComb;
extern long LowMotionFieldCount;
extern long StaticImageFieldCount;
extern BOOL bAutoDetectMode;
extern BOOL bFallbackToVideo;
extern BOOL bIsPaused;

extern BOOL	Wait_For_Flip;          // User parm, default=TRUE
extern BOOL	DoAccurateFlips;        // User parm, default=TRUE
extern BOOL	Hurry_When_Late;        // " , default=FALSE, skip processing if behind
extern long	Sleep_Interval;         // " , default=0, how long to wait for BT chip

#endif