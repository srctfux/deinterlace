////////////////////////////////////////////////////////////////////////////
// $Id: DMO_Lib.cpp,v 1.4 2003-09-30 16:59:26 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 John Adcock.  All rights reserved.
// This software was based on sample code generated by the 
// DMO project wizard.  That code is (c) Microsoft Corporation
/////////////////////////////////////////////////////////////////////////////
//
// This file is free software; you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.3  2003/07/18 09:26:34  adcockj
// Corrections to assembler files (does not compile)
//
// Revision 1.2  2003/05/22 06:43:48  adcockj
// Minor Fixes to Major bugs
//
// Revision 1.1  2003/05/16 16:19:12  adcockj
// Added new files into DMO framework
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <initguid.h>
#include <medparam.h>
#include "..\..\GenDMOProp\GenDMOProp.h"
#include "..\..\Common\CPUID.h"

#include "DMO_Lib.h"

extern _ATL_OBJMAP_ENTRY ObjectMap[];

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
	return (0 == _Module.GetLockCount()) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return _Module.GetClassObject(rclsid, riid, ppv);
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DMODllRegisterVideoDMO(LPCWSTR Name, const CLSID& Clsid)
{
	// Register ourself as a Video effect DMO
	DMO_PARTIAL_MEDIATYPE mt[3];
	mt[0].type    = MEDIATYPE_Video;
	mt[0].subtype = MEDIASUBTYPE_YUY2;
	mt[1].type    = MEDIATYPE_Video;
	mt[1].subtype = MEDIASUBTYPE_YV12;
	mt[2].type    = MEDIATYPE_Video;
	mt[2].subtype = MEDIASUBTYPE_NV12;

	HRESULT hr = DMORegister(Name,
							 Clsid, 
							 DMOCATEGORY_VIDEO_EFFECT,
							 0,
							 3,
							 &mt[0],
							 3,
							 &mt[0]);

	return _Module.RegisterServer();
}

STDAPI DMODllRegisterAudioDMO(LPCWSTR Name, const CLSID& Clsid, bool IsFloat)
{
	// Register ourself as a PCM audio effect DMO
	DMO_PARTIAL_MEDIATYPE mt;
	mt.type = MEDIATYPE_Audio;
    if(IsFloat)
    {
	    mt.subtype = MEDIASUBTYPE_PCM;
    }
    else
    {
        mt.subtype = MEDIASUBTYPE_IEEE_FLOAT;
    }
    

	HRESULT hr = DMORegister(Name,
							 Clsid, 
							 DMOCATEGORY_AUDIO_EFFECT,
							 0,
							 1,
							 &mt,
							 1,
							 &mt);
	// registers object
	return _Module.RegisterServer();
}

STDAPI DMODllRegisterDeintDMO(LPCWSTR Name, const CLSID& Clsid)
{
    CPU_SetupFeatureFlag();
    DWORD d = CpuFeatureFlags;

	// Register ourself as a PCM audio effect DMO
	DMO_PARTIAL_MEDIATYPE mt[3];
	mt[0].type    = MEDIATYPE_Video;
	mt[0].subtype = MEDIASUBTYPE_YUY2;
	mt[1].type    = MEDIATYPE_Video;
	mt[1].subtype = MEDIASUBTYPE_YV12;
	mt[2].type    = MEDIATYPE_Video;
	mt[2].subtype = MEDIASUBTYPE_NV12;

	HRESULT hr = DMORegister(Name,
							 Clsid, 
							 DMOCATEGORY_VIDEO_EFFECT,
							 0,
							 3,
							 &mt[0],
							 3,
							 &mt[0]);

	return _Module.RegisterServer();
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DMODllUnregisterVideoDMO(const CLSID& Clsid)
{
   	DMOUnregister(Clsid, DMOCATEGORY_VIDEO_EFFECT);
	return _Module.UnregisterServer();
}

STDAPI DMODllUnregisterAudioDMO(const CLSID& Clsid)
{
   	DMOUnregister(Clsid, DMOCATEGORY_AUDIO_EFFECT);
	return _Module.UnregisterServer();
}

STDAPI DMODllUnregisterDeintDMO(const CLSID& Clsid)
{
    // \todo create special category
   	DMOUnregister(Clsid, DMOCATEGORY_VIDEO_EFFECT);
	return _Module.UnregisterServer();
}


