/////////////////////////////////////////////////////////////////////////////
// settings.h
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
//
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Original Release
//                                     Translated most code from German
//                                     Combined Header files
//                                     Cut out all decoding
//                                     Cut out digital hardware stuff
//
// 21 Dec 2000   John Adcock           Added function to setup ini file name
//
// 05 Jan 2001   John Adcock           Added GetRefreshRate
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __SETTINGS_H___
#define __SETTINGS_H___

// This is the Header for the new UI code
// This is currently in developement and is not to be used
// It has only been checked in so that I can work on the 
// crashing problems

typedef enum
{
	NOT_PRESENT = 0,
	YESNO,
	ITEMFROMLIST,
	SLIDER_UCHAR,
	SLIDER_CHAR,
	SLIDER_UINT,
	SLIDER_INT,
	SLIDER_ULONG,
	SLIDER_LONG,
	NUMBER_UCHAR,
	NUMBER_CHAR,
	NUMBER_UINT,
	NUMBER_INT,
	NUMBER_ULONG,
	NUMBER_LONG,
	SUBMENU,

} SETTING_TYPE;

// Function called when setting value changes
// return value indicates whether.rest of screen needs to be
// refreshed
typedef BOOL (SETTING_ONCHANGE)(long NewValue);

typedef struct
{
	char* szDisplayName;
	SETTING_TYPE Type;
	BOOL bHasChanged;
	void* pValue;
	long Default;
	long MaxValue;
	long MinValue;
	long StepValue;
	long PrevValue;
	char** pszList;
	SETTING_ONCHANGE* pfnOnChange;
} SETTING;

#define SETTINGS_PER_MENU 8

typedef struct
{
   char* szDisplayName;
   SETTING Elements[SETTINGS_PER_MENU];
} UI_SUBMENU;

void DisplayUISubMenuAsDialog(UI_SUBMENU* pSubMenu);

// End of new UI code header

void SetIniFileForSettings(LPSTR Name);
void LoadSettingsFromIni();
void WriteSettingsToIni();
void WritePrivateProfileInt(LPCTSTR lpAppName,  LPCTSTR lpKeyName,  int nValue, LPCTSTR lpFileName);
DWORD GetRefreshRate();

#define INIFILE "dTV.ini"

#endif
