
/////////////////////////////////////////////////////////////////////////////
// deinterlace.h
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
// Change Log
//
// Date          Developer             Changes
//
// 03 Feb 2001   John Adcock           Initial Version
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __FILTER_H___
#define __FILTER_H___

#include "dTV_Filter.h"

BOOL LoadFilterPlugins();
void UnloadFilterPlugins();


SETTING* Filter_GetSetting(long nIndex, FILTER_SETTING Setting);
LONG Filter_HandleSettingsMsg(HWND hWnd, UINT message, UINT wParam, LONG lParam, BOOL* bDone);
void Filter_ReadSettingsFromIni();
void Filter_SetMenu(HMENU hMenu);
void Filter_WriteSettingsToIni();

void Filter_DoInput(DEINTERLACE_INFO *info, BOOL HurryUp);
void Filter_DoOutput(DEINTERLACE_INFO *info, BOOL HurryUp);

#endif