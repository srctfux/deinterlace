/////////////////////////////////////////////////////////////////////////////
// DI_VideoBob.h
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John adcock, Mark Rejhon, Steve Grimm.  All rights reserved.
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

#ifndef __VIDEOBOB_H___
#define __VIDEOBOB_H___

#include "settings.h"

BOOL DeinterlaceVideoBob(DEINTERLACE_INFO *info);

// Get Hold of the  DI_BobAndWeave.c file settings
SETTING* DI_VideoBob_GetSetting(DI_VIDEOBOB_SETTING Setting);
void DI_VideoBob_ReadSettingsFromIni();
void DI_VideoBob_WriteSettingsToIni();

#endif