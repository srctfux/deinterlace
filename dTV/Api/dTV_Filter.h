
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

#ifndef __DTV_FILTER_H___
#define __DTV_FILTER_H___

#include "dTV_Control.h"
#include "dTV_ApiCommon.h"

typedef void (__stdcall FILTERPLUGINSTART)(void);
typedef void (__stdcall FILTERPLUGINEXIT)(void);

typedef struct
{
	// What to display when selected
	char* szName;
	// What to put in the Menu (NULL to use szName)
	char* szMenuName;
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
	// call this if plugin needs to do anything before it is used
	FILTERPLUGINSTART* pfnPluginStart;
	// call this if plugin needs to deallocate anything
	FILTERPLUGINEXIT* pfnPluginExit;
} FILTER_METHOD;


// Call this function to get plug-in info
typedef FILTER_METHOD* (__stdcall GETFILTERPLUGININFO)(long CpuFeatureFlags);

#endif