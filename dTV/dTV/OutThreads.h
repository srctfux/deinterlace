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

#include "settings.h"
#include "deinterlace.h"

// Get Hold of the OutThreads.c file settings
SETTING* OutThreads_GetSetting(OUTTHREADS_SETTING Setting);
void OutThreads_ReadSettingsFromIni();
void OutThreads_WriteSettingsToIni();
void OutThreads_SetMenu(HMENU hMenu);

void Start_Capture();
void Stop_Capture();
void Reset_Capture();
void Pause_Capture();
void UnPause_Capture();
void Pause_Toggle_Capture();

void Start_Thread();
void Stop_Thread();

void SetupCaptureFlags();

DWORD WINAPI YUVOutThread(LPVOID lpThreadParameter);

void UpdatePulldownStatus();
BOOL WaitForNextField(BOOL LastField, BOOL* RunningLate);
void SetDeinterlaceMode(int mode);
char *DeinterlaceModeName(int mode);
BYTE * LockOverlay();		// Lock overlay, returns ptr to buffer

extern ePULLDOWNMODES gPulldownMode;

// Statistics
extern long	nTotalDropFrames;
extern long	nDropFramesLastSec;
extern long nSecTicks;
extern long nInitialTicks;
extern long nLastTicks;
extern long nTotalDeintModeChanges;
extern long nDeintModeChanges[PULLDOWNMODES_LAST_ONE];
extern long nDeintModeTicks[PULLDOWNMODES_LAST_ONE];

#endif