////////////////////////////////////////////////////////////////////////////
// $Id: Deint_Weave.cpp,v 1.8 2004-04-28 16:32:36 adcockj Exp $
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
// Revision 1.7  2004/03/15 17:17:04  adcockj
// Basic registry saving support
//
// Revision 1.6  2004/02/06 12:17:15  adcockj
// Major changes to the Libraries to remove ATL and replace with YACL
// First draft of Mpeg2 video decoder filter
// Broken DScalerFilter part converted to new library
//
// Revision 1.5  2003/10/31 17:19:37  adcockj
// Added support for manual pulldown selection (works with Elecard Filters)
//
// Revision 1.4  2003/09/24 07:01:03  adcockj
// fix some release issues
//
// Revision 1.3  2003/09/19 16:12:14  adcockj
// Further improvements
//
// Revision 1.2  2003/07/25 16:00:55  adcockj
// Remove 704 stuff
//
// Revision 1.1  2003/05/21 13:41:12  adcockj
// Added new deinterlace methods
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <initguid.h>
#include "Deint_Weave.h"
#include "yacl\include\combook.cpp"

HINSTANCE g_hInstance = NULL;

// Weave method requires :
//  2 fields to operate on
//  Doesn't introduce any delay
//  And has a very low complexity
CDeint_Weave::CDeint_Weave() :
    CDeintDMO(L"Weave", 2, 0, 1)
{
}

CDeint_Weave::~CDeint_Weave()
{
}

STDMETHODIMP CDeint_Weave::Process(IInterlacedBufferStack* Stack, IMediaBuffer* pOutputBuffer)
{
#ifdef _DEBUG
    DWORD NumFields(0);
    Stack->get_NumFields(&NumFields);
    ASSERT(NumFields > 1);
#endif

    VIDEOINFOHEADER2* InputInfo = (VIDEOINFOHEADER2*)(InputType(0)->pbFormat);
    VIDEOINFOHEADER2* OutputInfo = (VIDEOINFOHEADER2*)(OutputType(0)->pbFormat);

    SI(IInterlacedField) pNewerBuffer;
    SI(IInterlacedField) pOlderBuffer;

    HRESULT hr = Stack->GetField(0, pNewerBuffer.GetReleasedInterfaceReference()); 
    if(FAILED(hr)) return hr;

    hr = Stack->GetField(1, pOlderBuffer.GetReleasedInterfaceReference()); 
    if(FAILED(hr)) return hr;

    BOOLEAN IsTopLine = FALSE;

    hr = pNewerBuffer->get_TopFieldFirst(&IsTopLine);
    if(FAILED(hr)) return hr;

    BYTE* pInputDataNewer;
    DWORD InputLengthNewer;
    BYTE* pInputDataOlder;
    DWORD InputLengthOlder;
    BYTE* pOutputData;
    DWORD OutputLength;
    
    hr = pNewerBuffer->GetBufferAndLength(&pInputDataNewer, &InputLengthNewer);
    if(FAILED(hr)) return hr;

    hr = pOlderBuffer->GetBufferAndLength(&pInputDataOlder, &InputLengthOlder);
    if(FAILED(hr)) return hr;

    hr = pOutputBuffer->GetBufferAndLength(&pOutputData, &OutputLength);
    if(FAILED(hr)) return hr;

    if(OutputLength < OutputInfo->bmiHeader.biSizeImage)
    {
        hr = pOutputBuffer->SetLength(OutputInfo->bmiHeader.biSizeImage);
        if(FAILED(hr)) return hr;
    }
    // do 4:2:2 format up here and we need to 
    // worry about deinterlacing both luma and chroma
    if(InputInfo->bmiHeader.biCompression == MAKEFOURCC('Y','U','Y','2'))
    {
		DWORD LineLength;
		if(InputInfo->rcSource.right > 0)
		{
			LineLength = InputInfo->rcSource.right * 2;
		}
		else
		{
			LineLength = InputInfo->bmiHeader.biWidth * 2;
		}

		if(IsTopLine == TRUE)
        {
			pInputDataOlder += InputInfo->bmiHeader.biWidth * 2;
            for(int i(0); i < InputInfo->bmiHeader.biHeight/2; ++i)
            {
                memcpy(pOutputData, pInputDataNewer, LineLength);
                pOutputData += OutputInfo->bmiHeader.biWidth * 2;
                memcpy(pOutputData, pInputDataOlder, LineLength);
                pOutputData += OutputInfo->bmiHeader.biWidth * 2;
                pInputDataNewer += InputInfo->bmiHeader.biWidth * 4;
                pInputDataOlder += InputInfo->bmiHeader.biWidth * 4;
            }
        }
        else
        {
            pInputDataNewer += InputInfo->bmiHeader.biWidth * 2;
            for(int i(0); i < InputInfo->bmiHeader.biHeight/2; ++i)
            {
                memcpy(pOutputData, pInputDataOlder, LineLength);
                pOutputData += OutputInfo->bmiHeader.biWidth * 2;
                memcpy(pOutputData, pInputDataNewer, LineLength);
                pOutputData += OutputInfo->bmiHeader.biWidth * 2;
                pInputDataNewer += InputInfo->bmiHeader.biWidth * 4;
                pInputDataOlder += InputInfo->bmiHeader.biWidth * 4;
            }
        }
    }
    // otherwise it's a 4:2:0 plannar format and we just 
    // worry about deinterlacing the luma and just copy the chroma
    else
    {
		DWORD LineLength;
		if(InputInfo->rcSource.right > 0)
		{
			LineLength = InputInfo->rcSource.right;
		}
		else
		{
			LineLength = InputInfo->bmiHeader.biWidth;
		}

		// process luma
        if(IsTopLine == TRUE)
        {
            pInputDataOlder += InputInfo->bmiHeader.biWidth;
            for(int i(0); i < InputInfo->bmiHeader.biHeight/2; ++i)
            {
                memcpy(pOutputData, pInputDataNewer, LineLength);
                pOutputData += OutputInfo->bmiHeader.biWidth;
                memcpy(pOutputData, pInputDataOlder, LineLength);
                pOutputData += OutputInfo->bmiHeader.biWidth;
                pInputDataNewer += InputInfo->bmiHeader.biWidth * 2;
                pInputDataOlder += InputInfo->bmiHeader.biWidth * 2;
            }
            pInputDataOlder -= InputInfo->bmiHeader.biWidth;
        }
        else
        {
            pInputDataNewer += InputInfo->bmiHeader.biWidth;
            for(int i(0); i < InputInfo->bmiHeader.biHeight/2; ++i)
            {
                memcpy(pOutputData, pInputDataOlder, LineLength);
                pOutputData += OutputInfo->bmiHeader.biWidth;
                memcpy(pOutputData, pInputDataNewer, LineLength);
                pOutputData += OutputInfo->bmiHeader.biWidth;
                pInputDataNewer += InputInfo->bmiHeader.biWidth * 2;
                pInputDataOlder += InputInfo->bmiHeader.biWidth * 2;
            }
            pInputDataNewer -= InputInfo->bmiHeader.biWidth;
        }
        ProcessPlanarChroma(pInputDataNewer, pOutputData, InputInfo, OutputInfo);
    }
    return S_OK;
}


HRESULT CDeint_Weave::GetClassID(CLSID *pClsid)
{
	// Check for valid pointer
	if( NULL == pClsid )
	{
		return E_POINTER;
	}

	*pClsid = CLSID_CDeint_Weave;
	return S_OK;

} // GetClassID

STDMETHODIMP CDeint_Weave::get_Name(BSTR* Name)
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

STDMETHODIMP CDeint_Weave::get_License(eFreeLicense* License)
{
    if(License == NULL)
    {
        return E_POINTER;
    }
    *License = LGPL;
    return S_OK;
}

STDMETHODIMP CDeint_Weave::get_Authors(BSTR* Authors)
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


HRESULT CDeint_Weave::ParamChanged(DWORD dwParamIndex)
{
    // there are no parameters
    return E_UNEXPECTED;
}

///////////////////////
//
// Required COM stuff
//

BEGIN_COCLASS_TABLE(Classes)
    IMPLEMENTS_COCLASS(CDeint_Weave)
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
    HRESULT hr = DMODllRegisterDeintDMO(L"Weave", CLSID_CDeint_Weave);
	if(FAILED(hr))
	{
		return hr;
	}
	return ClassTableUpdateRegistry(GetThisInstance(), Classes, 0, FALSE, TRUE);
}

STDAPI DllUnregisterServer()
{
    HRESULT hr = DMODllUnregisterDeintDMO(CLSID_CDeint_Weave);
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
