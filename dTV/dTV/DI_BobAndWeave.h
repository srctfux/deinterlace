/////////////////////////////////////////////////////////////////////////////
// DI_BobAndWeave.h
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

#ifndef __BOBANDWEAVE_H___
#define __BOBANDWEAVE_H___

#include "settings.h"

// Get Hold of the  DI_BobAndWeave.c file settings
SETTING* DI_BobWeave_GetSetting(DI_BOBWEAVE_SETTING Setting);
void DI_BobWeave_ReadSettingsFromIni();
void DI_BobWeave_WriteSettingsToIni();

BOOL Bob(DEINTERLACE_INFO *info);
BOOL Weave(DEINTERLACE_INFO *info);



#endif