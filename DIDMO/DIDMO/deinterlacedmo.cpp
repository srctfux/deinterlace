/////////////////////////////////////////////////////////////////////////////
// $Id: deinterlacedmo.cpp,v 1.1 2001-08-08 15:37:02 tobbej Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Torbjörn Jansson.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//  This file is subject to the terms of the GNU General Public License as
//  published by the Free Software Foundation.  A copy of this license is
//  included with this software distribution in the file COPYING.  If you
//  do not have a copy, you may obtain a copy by writing to the Free
//  Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.1.1.1  2001/07/30 16:14:44  tobbej
// initial import of new dmo filter
//
//
//////////////////////////////////////////////////////////////////////////////


// DeinterlaceDMO.cpp : Implementation of DLL Exports.
// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL, 
//      run nmake -f DeinterlaceDMOps.mk in the project directory.

#include "stdafx.h"
#include "resource.h"

#define FIX_LOCK_NAME
#include <dmo.h>
#include <dmoimpl.h>

#include <initguid.h>
#include "DeinterlaceDMO.h"

#include "DeinterlaceDMO_i.c"
#include "Deinterlace.h"
#include "DeinterlaceProperties.h"
#include "DeinterlaceSettings.h"


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_Deinterlace, CDeinterlace)
OBJECT_ENTRY(CLSID_DeinterlaceProperties, CDeinterlaceProperties)
OBJECT_ENTRY(CLSID_DeinterlaceSettings, CDeinterlaceSettings)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_DEINTERLACEDMOLib);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    return TRUE;    // ok
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
	//register DMO, the category may need changing
	HRESULT hr = DMORegister(L"Deinterlace DMO",CLSID_Deinterlace,DMOCATEGORY_VIDEO_ENCODER,0,0,NULL,0,NULL);
    // registers object, typelib and all interfaces in typelib
    return _Module.RegisterServer(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
	DMOUnregister(CLSID_Deinterlace, DMOCATEGORY_VIDEO_ENCODER);
    return _Module.UnregisterServer(TRUE);
}
