/////////////////////////////////////////////////////////////////////////////
// ProgramList.h
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

#ifndef __PROGRAMLIST_H___
#define __PROGRAMLIST_H___

#include "defines.h"
#include "structs.h"
#include "globals.h"

void Write_Program_List();
void Load_Program_List();

BOOL APIENTRY ProgramListProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY ListFeldSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY KanalNummerProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
VOID APIENTRY HandleFocusStateKanalListe(HWND hDlg, LPDRAWITEMSTRUCT lpdis);
VOID APIENTRY DrawEntireItemKanalListe(HWND hDlg, LPDRAWITEMSTRUCT lpdis, INT Typ);
void GetFeldName(short id, char *zeile);
BOOL ValidModes(char Mode);
BOOL APIENTRY AnalogScanProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

#endif