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
// Change Log
//
// Date          Developer             Changes
//
// 10 Aug 2000   John Adcock           Initial code
//
/////////////////////////////////////////////////////////////////////////////

#include "bTVPlugin.h"

typedef void (__cdecl * pfn_plugin_Initialize)();
typedef void (__cdecl * pfn_plugin_UnInitialize)();
typedef void (__cdecl * pfn_plugin_Config)(HWND hWnd, int LangCode);
typedef int (__cdecl * pfn_plugin_GetProps)(BTV_V1_PROPS *pProps, int LangCode);
typedef long (__cdecl * pfn_plugin_DoField)(BTV_V1_PARAMS *pRarams);

typedef struct
{
	pfn_plugin_Initialize plugin_Initialize;
	pfn_plugin_UnInitialize plugin_UnInitialize;
	pfn_plugin_Config plugin_Config;
	pfn_plugin_GetProps plugin_GetProps;
	pfn_plugin_DoField plugin_DoField;
} BTV_PLUGIN;

BTV_PLUGIN BTVPlugin;
HMODULE hLibrary = NULL;;

BOOL BTVPluginLoad(const char * szPluginName)
{
	UINT nErrorMode;

	if(hLibrary != NULL)
	{
		BTVPluginUnload();
	}

	nErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX);
	hLibrary = LoadLibrary(szPluginName);
	SetErrorMode(nErrorMode);

	if(hLibrary == NULL)
	{
		return FALSE;
	}

	BTVPlugin.plugin_Initialize = (pfn_plugin_Initialize) GetProcAddress(hLibrary, "plugin_Initialize");
	if(BTVPlugin.plugin_Initialize == NULL)
	{
		FreeLibrary(hLibrary);
		hLibrary = NULL;
		return FALSE;
	}
	BTVPlugin.plugin_UnInitialize = (pfn_plugin_UnInitialize)GetProcAddress(hLibrary, "plugin_UnInitialize");
	if(BTVPlugin.plugin_UnInitialize == NULL)
	{
		FreeLibrary(hLibrary);
		hLibrary = NULL;
		return FALSE;
	}
	BTVPlugin.plugin_Config = (pfn_plugin_Config)GetProcAddress(hLibrary, "plugin_Config");
	if(BTVPlugin.plugin_Config == NULL)
	{
		FreeLibrary(hLibrary);
		hLibrary = NULL;
		return FALSE;
	}
	BTVPlugin.plugin_GetProps = (pfn_plugin_GetProps)GetProcAddress(hLibrary, "plugin_GetProps");
	if(BTVPlugin.plugin_GetProps == NULL)
	{
		FreeLibrary(hLibrary);
		hLibrary = NULL;
		return FALSE;
	}
	BTVPlugin.plugin_DoField = (pfn_plugin_DoField)GetProcAddress(hLibrary, "plugin_DoField");
	if(BTVPlugin.plugin_DoField == NULL)
	{
		FreeLibrary(hLibrary);
		hLibrary = NULL;
		return FALSE;
	}

	BTVPlugin.plugin_Initialize();

	return TRUE;
}

void BTVPluginUnload()
{
	if(hLibrary != NULL)
	{
		BTVPlugin.plugin_UnInitialize();
		FreeLibrary(hLibrary);
	}
}

void BTVPluginConfig(HWND hWnd)
{
	BTVPlugin.plugin_Config(hWnd, 0);
}

int BTVPluginGetProps(BTV_V1_PROPS *pProps)
{
	return BTVPlugin.plugin_GetProps(pProps, 0);
}

long BTVPluginDoField(BTV_V1_PARAMS *pParams)
{
	return BTVPlugin.plugin_DoField(pParams);
}
