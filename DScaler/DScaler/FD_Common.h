/////////////////////////////////////////////////////////////////////////////
// $Id: FD_Common.h,v 1.9 2001-07-13 16:14:56 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//  This file is subject to the terms of the GNU General Public License as
//  published by the Free Software Foundation.  A copy of this license is
//  included with this software distribution in the file COPYING.  If you
//  do not have a copy, you may obtain a copy by writing to the Free
//  Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
//
// Change Log
//
// Date          Developer             Changes
//
// 09 Jan 2001   John Adcock           Split into new file
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __FD_COMMON_H___
#define __FD_COMMON_H___

#include "settings.h"
#include "deinterlace.h"


// Get Hold of the FD_50Hz.c file settings
SETTING* FD_Common_GetSetting(FD_COMMON_SETTING Setting);
void FD_Common_ReadSettingsFromIni();
void FD_Common_WriteSettingsToIni();
void FD_Common_SetMenu(HMENU hMenu);
void FD_Common_ShowUI();

#define MAXMODESWITCHES 50  // Maximum number of switches to track in TrackModeSwitches()

void ResetModeSwitches();
BOOL TrackModeSwitches();

void PerformFilmDetectCalculations(DEINTERLACE_INFO* pInfo, BOOL NeedComb, BOOL NeedDiff);

BOOL Weave(DEINTERLACE_INFO* pInfo);
BOOL Bob(DEINTERLACE_INFO* pInfo);


#endif