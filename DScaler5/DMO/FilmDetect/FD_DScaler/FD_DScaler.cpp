////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004 John Adcock.  All rights reserved.
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
// Revision 1.3  2004/12/15 13:04:08  adcockj
// added simple statistics display
//
// Revision 1.2  2004/12/13 16:59:56  adcockj
// flag based film detection
//
// Revision 1.1  2004/09/10 15:40:00  adcockj
// Added stub project for film detection
//
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <initguid.h>
#include "FD_DScaler.h"
#include "yacl\include\combook.cpp"

HINSTANCE g_hInstance = NULL;

CFD_DScaler::CFD_DScaler() :
    CSimpleInPlaceVideoDMO(L"Film Detect", IS_FILMDETECT)
{
    m_DetectedType = FULLRATEVIDEO;
    m_DetectedIndex = 0;
}

CFD_DScaler::~CFD_DScaler()
{
}

HRESULT CFD_DScaler::DetectFilm(IInterlacedBufferStack* Stack, eDeinterlaceType* DetectedType, DWORD* DetectedIndex)
{
    DWORD NumFields(0);
    Stack->get_NumFields(&NumFields);
    if(NumFields > 0)
    {
        IInterlacedField* Field = NULL;
        Stack->GetField(0, &Field);
        eDetectionHint Hint;
        Field->get_Hint(&Hint);
        DWORD FieldNum;
        Field->get_FieldNumber(&FieldNum);

        IMediaBuffer* MovementMap = NULL;
        Stack->GetMovementMap(&MovementMap);
        BOOLEAN IsTopField = FALSE;
        Field->get_TopFieldFirst(&IsTopField);

        if(m_DetectedType == FULLRATEVIDEO)
        {
            CheckVideo(Hint, FieldNum, Field);
        }
        else if(m_DetectedType == PULLDOWN_32)
        {
            Check32(Hint, FieldNum, Field);
        }
        else if(m_DetectedType == PULLDOWN_22)
        {
            Check22(Hint, FieldNum, Field);
        }
        Field->Release();
    }

    *DetectedType = m_DetectedType;
    *DetectedIndex = m_DetectedIndex;

    return S_OK;
}

void CFD_DScaler::CheckVideo(eDetectionHint Hint, DWORD FieldNumber, IInterlacedField* Field)
{
    if(Hint == WEAVE_WITH_AFTERNEXT_32)
    {
        m_DetectedType = PULLDOWN_32;
        m_DetectedIndex = (7 - FieldNumber % 5) % 5;
    }
    else if(Hint == WEAVE_WITH_NEXT_22)
    {
        m_DetectedType = PULLDOWN_22;
        m_DetectedIndex = FieldNumber % 2;
    }
}

void CFD_DScaler::Check32(eDetectionHint Hint, DWORD FieldNumber, IInterlacedField* Field)
{
    switch((FieldNumber + m_DetectedIndex) % 5)
    {
    case 0:
        if(Hint != WEAVE_WITH_NEXT_22)
        {
            m_DetectedType = FULLRATEVIDEO;
            m_DetectedIndex = 0;
        }
        break;
    case 1:
        if(Hint != WEAVE_WITH_PREV_22)
        {
            m_DetectedType = FULLRATEVIDEO;
            m_DetectedIndex = 0;
        }
        break;
    case 2:
        if(Hint != WEAVE_WITH_AFTERNEXT_32)
        {
            m_DetectedType = FULLRATEVIDEO;
            m_DetectedIndex = 0;
        }
        break;
    case 3:
        if(Hint != WEAVE_WITH_NEXT_32)
        {
            m_DetectedType = FULLRATEVIDEO;
            m_DetectedIndex = 0;
        }
        break;
    case 4:
        if(Hint != WEAVE_WITH_PREV_32)
        {
            m_DetectedType = FULLRATEVIDEO;
            m_DetectedIndex = 0;
        }
        break;
    }
    if(m_DetectedType == FULLRATEVIDEO)
    {
        CheckVideo(Hint, FieldNumber, Field);
    }
}

void CFD_DScaler::Check22(eDetectionHint Hint, DWORD FieldNumber, IInterlacedField* Field)
{
    switch((FieldNumber +  m_DetectedIndex) % 2)
    {
    case 0:
        if(Hint != WEAVE_WITH_NEXT_22)
        {
            m_DetectedType = FULLRATEVIDEO;
            m_DetectedIndex = 0;
        }
        break;
    case 1:
        if(Hint != WEAVE_WITH_PREV_22)
        {
            m_DetectedType = FULLRATEVIDEO;
            m_DetectedIndex = 0;
        }
        break;
    }
    if(m_DetectedType == FULLRATEVIDEO)
    {
        CheckVideo(Hint, FieldNumber, Field);
    }
}


HRESULT CFD_DScaler::ResetDectection()
{
    m_DetectedType = FULLRATEVIDEO;
    m_DetectedIndex = 0;
    return S_OK;
}


HRESULT CFD_DScaler::ParamChanged(DWORD dwParamIndex)
{
    return E_UNEXPECTED;
}

HRESULT CFD_DScaler::GetClassID(CLSID *pClsid)
{
	// Check for valid pointer
	if( NULL == pClsid )
	{
		return E_POINTER;
	}

	*pClsid = CLSID_CFD_DScaler;
	return S_OK;

} // GetClassID

STDMETHODIMP CFD_DScaler::get_Name(BSTR* Name)
{
    if(Name == NULL)
    {
        return E_POINTER;
    }
    TCHAR Result[MAX_PATH];
    if(LoadString(g_hInstance, IDS_NAME, Result, MAX_PATH))
    {
        wchar_t wResult[MAX_PATH];
        ustrcpy(wResult, Result);
        *Name = SysAllocString(wResult);
        return S_OK;
    }
    else
    {
        return E_UNEXPECTED;
    }
}

STDMETHODIMP CFD_DScaler::get_License(eFreeLicense* License)
{
    if(License == NULL)
    {
        return E_POINTER;
    }
    *License = LGPL;
    return S_OK;
}

STDMETHODIMP CFD_DScaler::get_Authors(BSTR* Authors)
{
    if(Authors == NULL)
    {
        return E_POINTER;
    }
    TCHAR Result[MAX_PATH];
    if(LoadString(g_hInstance, IDS_AUTHORS, Result, MAX_PATH))
    {
        wchar_t wResult[MAX_PATH];
        ustrcpy(wResult, Result);
        *Authors = SysAllocString(wResult);
        return S_OK;
    }
    else
    {
        return E_UNEXPECTED;
    }
}


///////////////////////
//
// Required COM stuff
//

BEGIN_COCLASS_TABLE(Classes)
    IMPLEMENTS_COCLASS(CFD_DScaler)
END_COCLASS_TABLE()

IMPLEMENT_DLL_MODULE_ROUTINES()

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
	if (DLL_PROCESS_ATTACH == dwReason)
	{
        g_hInstance = hInstance;
		DisableThreadLibraryCalls(hInstance);
	}
	else if (DLL_PROCESS_DETACH == dwReason)
    {
    }
	return TRUE;    // ok
}

STDAPI DllRegisterServer()
{
    HRESULT hr = DMODllRegisterDeintDMO(L"DScaler Film Detect", CLSID_CFD_DScaler);
	if(FAILED(hr))
	{
		return hr;
	}
	return ClassTableUpdateRegistry(GetThisInstance(), Classes, 0, FALSE, TRUE);
}

STDAPI DllUnregisterServer()
{
    HRESULT hr = DMODllUnregisterDeintDMO(CLSID_CFD_DScaler);
	if(FAILED(hr))
	{
		return hr;
	}
	return ClassTableUpdateRegistry(GetThisInstance(), Classes, 0, FALSE, FALSE);
}


STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    return ClassTableGetClassObject(Classes, rclsid, riid, ppv);
}

STDAPI DllCanUnloadNow(void)
{
    return ModuleIsIdle() ? S_OK : S_FALSE;
}
