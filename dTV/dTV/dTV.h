/////////////////////////////////////////////////////////////////////////////
// dTV.h
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
/////////////////////////////////////////////////////////////////////////////

#ifndef __DTV_H___
#define __DTV_H___

#include "defines.h"
#include "structs.h"
#include "globals.h"


/// Neu

void Write_Nit_List();
void Load_Nit_List();
BOOL prozess_DatumZeit(int Nummer, unsigned char setting);

void WorkoutOverlaySize();
void Init_More();
void More_Even(int pp);
void More_Odd();
void ChangeChannel(int NewChannel);

VOID APIENTRY HandleFocusState(HWND hDlg, LPDRAWITEMSTRUCT lpdis);

BOOL APIENTRY ToggleSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);

void SetupProcessorAndThread();

LONG APIENTRY MainWndProc(HWND, UINT, UINT, LONG);

void MainWndOnCreate(HWND hWnd);
void MainWndOnInitBT(HWND hWnd);
void SaveWindowPos(HWND hWnd);
void SetMenuAnalog();
void CleanUpMemory();
UINT get_feature_flags(void);

void Overlay_Stop(HWND hWnd);
void Overlay_Start(HWND hWnd);
void ShowText(HWND hWnd, LPCTSTR szText);
void OSD_ShowVideoSource(HWND hWnd, int nVideoSource);

int GetCurrentAdjustmentStepCount();
int AdjustSliderUp(int * pnValue, int nUpper);
int AdjustSliderDown(int * pnValue, int nLower);

#define TIMER_STATUS        1
#define TIMER_STATUS_MS     2000

#define TIMER_KEYNUMBER     99
#define TIMER_KEYNUMBER_MS  1000

#define TIMER_MSP           8
#define TIMER_MSP_MS        10000

#define TIMER_AUTOSAVE      55
#define TIMER_AUTOSAVE_MS   1500

#endif
