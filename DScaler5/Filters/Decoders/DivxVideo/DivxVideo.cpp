///////////////////////////////////////////////////////////////////////////////
// $Id: DivxVideo.cpp,v 1.1 2004-11-05 17:45:53 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
// DivxVideo.dll - DirectShow filter for deinterlacing and video processing
// Copyright (c) 2004 John Adcock
///////////////////////////////////////////////////////////////////////////////
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
///////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "moreuuids.h"
#include <initguid.h>
#include "DivxDecoder.h"
#include "CPUID.h"

#include "..\GenDMOProp\GenDMOProp_i.c"
#include "yacl\include\combook.cpp"

HINSTANCE g_hInstance = NULL;

BEGIN_COCLASS_TABLE(Classes)
    IMPLEMENTS_COCLASS(CDivxDecoder)
END_COCLASS_TABLE()

IMPLEMENT_DLL_MODULE_ROUTINES()

extern "C"
{
CRITICAL_SECTION g_csStaticDataLock;
}

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hInstance = hInstance;
        DisableThreadLibraryCalls(hInstance);
        CPU_SetupFeatureFlag();
		InitializeCriticalSection( &g_csStaticDataLock );
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
		DeleteCriticalSection( &g_csStaticDataLock );
    }
    return TRUE;    // ok
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    return ClassTableGetClassObject(Classes, rclsid, riid, ppv);
}

STDAPI DllCanUnloadNow(void)
{
    return ModuleIsIdle() ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{

    REGPINTYPES InputTypes[] = {    
            { &MEDIATYPE_Video, &MEDIASUBTYPE_xvid },
            { &MEDIATYPE_Video, &MEDIASUBTYPE_XVID },
            { &MEDIATYPE_Video, &MEDIASUBTYPE_divx },
            { &MEDIATYPE_Video, &MEDIASUBTYPE_DIVX },
            { &MEDIATYPE_Video, &MEDIASUBTYPE_div3 },
            { &MEDIATYPE_Video, &MEDIASUBTYPE_DIV3 },
            { &MEDIATYPE_Video, &MEDIASUBTYPE_dx50 },
            { &MEDIATYPE_Video, &MEDIASUBTYPE_DX50 },
            { &MEDIATYPE_Video, &MEDIASUBTYPE_mp43 },
            { &MEDIATYPE_Video, &MEDIASUBTYPE_MP43 },
            { &MEDIATYPE_Video, &MEDIASUBTYPE_mp42 },
            { &MEDIATYPE_Video, &MEDIASUBTYPE_MP42 },
            { &MEDIATYPE_Video, &MEDIASUBTYPE_mp41 },
            { &MEDIATYPE_Video, &MEDIASUBTYPE_MP41 },
            { &MEDIATYPE_Video, &MEDIASUBTYPE_H263 },
            { &MEDIATYPE_Video, &MEDIASUBTYPE_h263 },
    };

    REGPINTYPES OutputTypes[] = {   
        {&MEDIATYPE_Video, &MEDIASUBTYPE_YUY2},
        {&MEDIATYPE_Video, &MEDIASUBTYPE_YV12},
    };
    
    REGFILTERPINS2 Pins[2] = {{ 0, 1, countof(InputTypes), InputTypes, 0, NULL, &GUID_NULL}, 
                              { REG_PINFLAG_B_OUTPUT , 1, countof(OutputTypes), OutputTypes, 0, NULL, &GUID_NULL}};

    REGFILTER2 RegInfo;
   
    RegInfo.dwVersion = 2;
    RegInfo.dwMerit = MERIT_PREFERRED;
    RegInfo.cPins2 = 2;
    RegInfo.rgPins2 = Pins;
    
  
    HRESULT hr = RegisterFilter(CLSID_CDivxDecoder, L"DScaler Divx Video Decoder", &RegInfo);
    CHECK(hr);
    return ClassTableUpdateRegistry(GetThisInstance(), Classes, 0, FALSE, TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
    HRESULT hr = UnregisterFilter(CLSID_CDivxDecoder);
    CHECK(hr);
    return ClassTableUpdateRegistry(GetThisInstance(), Classes, 0, FALSE, FALSE);
}