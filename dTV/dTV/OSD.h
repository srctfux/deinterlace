/////////////////////////////////////////////////////////////////////////////
// OSD.h
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
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 8 Nov 2000    Michael Eskin         Initial onscreen display
//
// 28 Nov 2000   Mark Rejhon           Reorganization and visual improvements
//
// 24 Feb 2001   Michael Samblanet     Added invalidation rectangle
//
// 25 Feb 2001   Laurent Garnier       Added management of multiple OSD texts
//
// 03 Mar 2001   Laurent Garnier       Added functions OSD_ShowInfosScreen
//                                     and OSD_GetLineYpos
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __OSD_H___
#define __OSD_H___

#include "settings.h"

// Get Hold of the OSD.c file settings
SETTING* OSD_GetSetting(OSD_SETTING Setting);
void OSD_ReadSettingsFromIni();
void OSD_WriteSettingsToIni();
void OSD_SetMenu(HMENU hMenu);


// Make sure that the timer ID does not conflict with those in DTV.H
#define OSD_TIMER_ID			42
#define OSD_TIMER_DELAY			4000

#define OSD_MAX_TEXT			25

typedef enum
{
	OSD_XPOS_LEFT = 0,
	OSD_XPOS_RIGHT,
	OSD_XPOS_CENTER,
} OSD_TEXT_XPOS;

typedef struct OSD_INFO_TAG
{
    char            szText[512];       // Text of OSD
    double          dfSize;            // Size of OSD as percentage of screen height
	long			textColor;         // Text color (RGB)
	OSD_TEXT_XPOS	textXpos;          // Text position / Xpos
    double          dfXpos;            // X position (0 = left, 1 = right)
    double          dfYpos;            // Y position (0 = top, 1 = bottom)
	RECT            currentRect;       // MRS 2-24-01 Saves the current drawn rectangle (used to limit invalidation area)
} OSD_INFO;

int OSD_GetNbText();
void OSD_ClearAllTexts();
void OSD_AddText(LPCTSTR szText, double dfSize, long textColor, OSD_TEXT_XPOS textXpos, double dfXpos, double dfYpos);
void OSD_Show(HWND hWnd, BOOL persistent);
void OSD_ShowText(HWND hWnd, LPCTSTR szText, double dfSize);
void OSD_ShowTextPersistent(HWND hWnd, LPCTSTR szText, double dfSize);
void OSD_ShowTextOverride(HWND hWnd, LPCTSTR szText, double dfSize);
void OSD_Redraw(HWND hWnd, HDC hDC);
void OSD_Clear(HWND hWnd);
double OSD_GetLineYpos (int nLine, double dfMargin, double dfSize);
void OSD_ShowInfosScreen(HWND hWnd, double dfSize);

#endif