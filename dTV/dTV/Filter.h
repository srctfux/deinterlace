
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

#include "Settings.h"
#include "Deinterlace.h"

// Get Hold of the Deinterlace.c file settings
SETTING* Filter_GetSetting(FILTER_SETTING Setting);
void Filter_ReadSettingsFromIni();
void Filter_SetMenu(HMENU hMenu);
void Filter_WriteSettingsToIni();

typedef enum
{
	GAMMA = 0,
	TEMPORAL_NOISE,
	FILTERS_LAST_ONE
} eFILTERS;

typedef struct
{
	// What to display when selected
	char* szName;
	// Are we active Initially FALSE
	BOOL bActive;
	// Do we get called on Input
	BOOL bOnInput;
    // Pointer to Algorithm function (cannot be NULL)
    DEINTERLACE_FUNC* pfnAlgorithm;
	// id of menu to display status
	int MenuId;
	// Always run - do we run if there has been an overrun
	BOOL bAlwaysRun;
} FILTER_METHOD;


void Filter_DoInput(DEINTERLACE_INFO *info, BOOL HurryUp);
void Filter_DoOutput(DEINTERLACE_INFO *info, BOOL HurryUp);

#endif