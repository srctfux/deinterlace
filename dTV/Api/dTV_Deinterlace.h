/////////////////////////////////////////////////////////////////////////////
// dTV_Deinterlace.h
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
// This header file is free software; you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 27 Mar 2001   John Adcock           Separated code to support plug-ins
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __DTV_DEINTERLACE_H___
#define __DTV_DEINTERLACE_H___

#include "dTV_Control.h"
#include "dTV_ApiCommon.h"

struct _DEINTERLACE_METHOD;

typedef void (__stdcall DEINTERLACEPLUGINSTART)(long NumPlugIns, struct _DEINTERLACE_METHOD** OtherPlugins);
typedef void (__stdcall DEINTERLACEPLUGINSWITCHTO)(HWND hwndMain, HWND hwndStatus);
typedef void (__stdcall DEINTERLACEPLUGINEXIT)(void);

typedef struct _DEINTERLACE_METHOD
{
	// What to display when selected
	char* szName;
	// Short Name
	// What to display when used in adaptive mode (NULL to use szName)
	char* szShortName;
	// Do we need to shrink the overlay by half
	BOOL bIsHalfHeight;
	// Is this a film mode
	BOOL bIsFilmMode;
	// Pointer to Algorithm function (cannot be NULL)
	DEINTERLACE_FUNC* pfnAlgorithm;
	// flip frequency in 50Hz mode
	unsigned long FrameRate50Hz;
	// flip frequency in 60Hz mode
	unsigned long FrameRate60Hz;
	// number of settings
	long nSettings;
	// pointer to start of Settings[nSettings]
	SETTING* pSettings;
	// Index Number (position in menu) should map to old enum value
	// and should be unique
	long nMethodIndex;
	// call this if plugin needs to do anything before it is used
	DEINTERLACEPLUGINSTART* pfnPluginStart;
	// call this if plugin needs to do anything before it is used
	DEINTERLACEPLUGINSWITCHTO* pfnPluginSwitchTo;
	// call this if plugin needs to deallocate anything
	DEINTERLACEPLUGINEXIT* pfnPluginExit;
	// how many fields are required to run this plug-in
	long nFieldsRequired;
	// Track number of mode Changes
	long ModeChanges;
	// Track Time in mode
	long ModeTicks;
	// the offset used by the external settings API
	long nSettingsOffset;
	// Dll module so that we can unload the dll cleanly at the end
	HMODULE hModule;
	// Menu Id used for this plug-in, use 0 to automatically allocate one
	DWORD MenuId;
	// do we need FieldDiff filled in in info
	BOOL bNeedFieldDiff;
	// do we need CombFactor filled in in info
	BOOL bNeedCombFactor;
} DEINTERLACE_METHOD;

// Call this function to init a plug-in
// On exit pDeintMethod is a pointer to deinterlace properties
typedef DEINTERLACE_METHOD* (__cdecl GETDEINTERLACEPLUGININFO)(long CpuFeatureFlags);


#endif
