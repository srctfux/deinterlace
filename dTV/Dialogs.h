/////////////////////////////////////////////////////////////////////////////
// dialogs.h
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

#ifndef __DIALOGS_H___
#define __DIALOGS_H___

#include "defines.h"
#include "structs.h"
#include "globals.h"

BOOL APIENTRY VideoTextProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY VideoTextUnterTitelProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY SplashProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY VideoSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY ICSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY VTSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY VDSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY VTInfoProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY VPSInfoProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY ICInfoProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY VDInfoProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY VDInfoProcRaw(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY PDCProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY AudioSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY AudioSettingProc1(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY AboutProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
VOID APIENTRY DrawEntireItem(HWND hDlg, LPDRAWITEMSTRUCT lpdis, INT Typ);
BOOL APIENTRY CardSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY PLLSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY ChipSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY SelectCardProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

#endif

#include "defines.h"
#include "structs.h"
#include "globals.h"

