/////////////////////////////////////////////////////////////////////////////
// bTVPlugin.h
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
// 09 Aug 2000   John Adcock           Fixed bug at end of GetCombFactor assember
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _BTVPLUGIN_HEADER_
#define _BTVPLUGIN_HEADER_

#include "defines.h"
#include "structs.h"
#include "globals.h"

BOOL BTVPluginLoad(const char * szPluginName);
void BTVPluginUnload();
void BTVPluginConfig(HWND hWnd);
int BTVPluginGetProps(BTV_V1_PROPS *pProps);
long BTVPluginDoField(BTV_V1_PARAMS *pParams);

extern char szBTVPluginName[MAX_PATH];
extern BOOL bUseBTVPlugin;
extern BTV_V1_PARAMS BTVParams;

#endif
