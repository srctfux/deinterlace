/////////////////////////////////////////////////////////////////////////////
// DI_Adaptive.h
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 Steve Grimm.  All rights reserved.
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
// 14 Jan 2001   John Adcock           Split into new file
//                                     as part of global Variable Tidy up
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __TWOFRAME_H___
#define __TWOFRAME_H___

#include "settings.h"

typedef enum
{
	TWOFRAMESPATIALTOLERANCE = 0,
	TWOFRAMETEMPORALTOLERANCE,
	DI_TWOFRAME_SETTING_LASTONE,
} DI_TWOFRAME_SETTING;

// Get Hold of the DI_Adaptive.c file settings
SETTING* DI_TwoFrame_GetSetting(DI_TWOFRAME_SETTING Setting);
void DI_TwoFrame_ReadSettingsFromIni();
void DI_TwoFrame_WriteSettingsToIni();

#endif