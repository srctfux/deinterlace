/////////////////////////////////////////////////////////////////////////////
// DI_Greedy2Frame.h
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock, Tom Barry, Steve Grimm  All rights reserved.
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
// 07 Feb 2001   John Adcock           Created new method based on 2-frame and greedy
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __GREEDY2FRAME_H___
#define __GREEDY2FRAME_H___

#include "settings.h"

// Get Hold of the DI_Adaptive.c file settings
SETTING* DI_Greedy2Frame_GetSetting(DI_GREEDY2FRAME_SETTING Setting);
void DI_Greedy2Frame_ReadSettingsFromIni();
void DI_Greedy2Frame_WriteSettingsToIni();

#endif