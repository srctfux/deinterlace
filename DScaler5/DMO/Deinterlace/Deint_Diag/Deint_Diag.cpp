////////////////////////////////////////////////////////////////////////////
// $Id: Deint_Diag.cpp,v 1.15 2004-12-07 16:53:32 adcockj Exp $
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
// Revision 1.14  2004/12/06 18:04:54  adcockj
// Major improvements to deinterlacing
//
// Revision 1.13  2004/04/28 16:32:36  adcockj
// Better dynamic connection
//
// Revision 1.12  2004/03/15 17:17:04  adcockj
// Basic registry saving support
//
// Revision 1.11  2004/02/06 12:17:15  adcockj
// Major changes to the Libraries to remove ATL and replace with YACL
// First draft of Mpeg2 video decoder filter
// Broken DScalerFilter part converted to new library
//
// Revision 1.10  2003/10/31 17:19:37  adcockj
// Added support for manual pulldown selection (works with Elecard Filters)
//
// Revision 1.9  2003/09/30 16:59:26  adcockj
// Improved handling of small format changes
//
// Revision 1.8  2003/09/24 07:01:01  adcockj
// fix some release issues
//
// Revision 1.7  2003/09/19 16:12:13  adcockj
// Further improvements
//
// Revision 1.6  2003/07/30 06:58:01  adcockj
// Fixed another chroma position problem
//
// Revision 1.5  2003/07/29 07:01:54  adcockj
// Fixed some issues with YV12 and general chroma positioning
//
// Revision 1.4  2003/07/25 16:02:56  adcockj
// Fixed compile problem
//
// Revision 1.3  2003/07/25 16:00:54  adcockj
// Remove 704 stuff
//
// Revision 1.2  2003/07/18 09:26:34  adcockj
// Corrections to assembler files (does not compile)
//
// Revision 1.1  2003/05/21 13:41:12  adcockj
// Added new deinterlace methods
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <initguid.h>
#include "Deint_Diag.h"
#include "..\..\..\Common\CPUID.h"
#include "yacl\include\combook.cpp"

HINSTANCE g_hInstance = NULL;

extern "C"
{
    void __cdecl Deint_Diag_Core_YUY2_MMX(BYTE* M1, BYTE* T2, BYTE* B2, BYTE* M3, BYTE* T4, BYTE* B4, BYTE* Dest, DWORD Count);
    void __cdecl Deint_Diag_Core_YUY2_SSE(BYTE* M1, BYTE* T2, BYTE* B2, BYTE* M3, BYTE* T4, BYTE* B4, BYTE* Dest, DWORD Count);
    void __cdecl Deint_Diag_Core_YUY2_3DNOW(BYTE* M1, BYTE* T2, BYTE* B2, BYTE* M3, BYTE* T4, BYTE* B4, BYTE* Dest, DWORD Count);
    void __cdecl Deint_Diag_Core_Packed_MMX(BYTE* M1, BYTE* T2, BYTE* B2, BYTE* M3, BYTE* T4, BYTE* B4, BYTE* Dest, DWORD Count);
    void __cdecl Deint_Diag_Core_Packed_SSE(BYTE* M1, BYTE* T2, BYTE* B2, BYTE* M3, BYTE* T4, BYTE* B4, BYTE* Dest, DWORD Count);
    void __cdecl Deint_Diag_Core_Packed_3DNOW(BYTE* M1, BYTE* T2, BYTE* B2, BYTE* M3, BYTE* T4, BYTE* B4, BYTE* Dest, DWORD Count);
    typedef void (__cdecl DIAG_FUNC)(BYTE* M1, BYTE* T2, BYTE* B2, BYTE* M3, BYTE* T4, BYTE* B4, BYTE* Dest, DWORD Count);
    extern unsigned __int64 MOVE;
}

int FunctionIndex = 0;

#define DIAG_YUY2 0
#define DIAG_PACK 1

DIAG_FUNC* DiagFunctions[3][2] = 
{
	{Deint_Diag_Core_YUY2_MMX, Deint_Diag_Core_Packed_MMX,},
	{Deint_Diag_Core_YUY2_3DNOW, Deint_Diag_Core_Packed_3DNOW,},
	{Deint_Diag_Core_YUY2_SSE, Deint_Diag_Core_Packed_SSE,},
};

#define NUM_FIELDS 2

// Diag method requires :
//  4 fields to operate on
//  introduce one field delay
//  And has a medium high complexity
CDeint_Diag::CDeint_Diag() :
    CDeintDMO(L"Diag", NUM_FIELDS, 1, 7)
{
}

CDeint_Diag::~CDeint_Diag()
{
}


STDMETHODIMP CDeint_Diag::Process(IInterlacedBufferStack* Stack, IMediaBuffer* pOutputBuffer)
{
#ifdef _DEBUG
    DWORD NumFields(0);
    Stack->get_NumFields(&NumFields);
    ASSERT(NumFields >= NUM_FIELDS);
#endif

    HRESULT hr;

    VIDEOINFOHEADER2* InputInfo = (VIDEOINFOHEADER2*)(InputType(0)->pbFormat);
    VIDEOINFOHEADER2* OutputInfo = (VIDEOINFOHEADER2*)(OutputType(0)->pbFormat);

    SI(IInterlacedField) pFields[2];
    BYTE* pInputData[2];
    DWORD InputLength[2];
    SI(IMediaBuffer) pMap;
    BYTE* pMapData;
    DWORD MapLength;
    BYTE* pOutputData;
    DWORD OutputLength;
    DWORD InputPitch;
    DWORD TwoLinePitch;
    DWORD OutputPitch;

    hr = Stack->GetField(0, pFields[0].GetReleasedInterfaceReference()); 
    if(FAILED(hr)) return hr;

    hr = pFields[0]->GetBufferAndLength(&pInputData[0], &InputLength[0]);
    if(FAILED(hr)) return hr;

    hr = Stack->GetField(1, pFields[1].GetReleasedInterfaceReference()); 
    if(FAILED(hr)) return hr;

    hr = pFields[1]->GetBufferAndLength(&pInputData[1], &InputLength[1]);
    if(FAILED(hr)) return hr;

    hr = Stack->GetMovementMap(pMap.GetReleasedInterfaceReference()); 
    if(FAILED(hr)) return hr;

    hr = pMap->GetBufferAndLength(&pMapData, &MapLength);
    if(FAILED(hr)) return hr;

    BOOLEAN IsTopLine = FALSE;

    hr = pFields[0]->get_TopFieldFirst(&IsTopLine);
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
        InputPitch = InputInfo->bmiHeader.biWidth * 2;
        TwoLinePitch = InputPitch * 2;
        OutputPitch = OutputInfo->bmiHeader.biWidth * 2;

        if(IsTopLine == TRUE)
        {
            pInputData[1] += InputPitch;
            pMapData += InputPitch;

            memcpy(pOutputData, pInputData[0], InputInfo->bmiHeader.biWidth * 2);
            pOutputData += OutputPitch;

            for(int i(0); i < InputInfo->bmiHeader.biHeight/2 - 1; ++i)
            {
                memcpy(pOutputData, pInputData[1], InputInfo->bmiHeader.biWidth * 2);
                pOutputData += OutputPitch;

                pInputData[0] += TwoLinePitch;

				DiagFunctions[FunctionIndex][DIAG_YUY2](
															pInputData[0],
															pInputData[1],
															pInputData[1] + TwoLinePitch,
															pMapData + InputPitch,
															pMapData,
															pMapData + TwoLinePitch,
															pOutputData,
															InputPitch
														);
                pOutputData += OutputPitch;

                pInputData[1] += TwoLinePitch;
                pMapData += TwoLinePitch;
            }

            memcpy(pOutputData, pInputData[1], InputInfo->bmiHeader.biWidth * 2);
            pOutputData += OutputPitch;
        }
        else
        {
            pInputData[0] += InputPitch;

            for(int i(0); i < InputInfo->bmiHeader.biHeight/2 - 1; ++i)
            {
                memcpy(pOutputData, pInputData[1], InputInfo->bmiHeader.biWidth * 2);
                pOutputData += OutputPitch;

                DiagFunctions[FunctionIndex][DIAG_YUY2](
															pInputData[0],
															pInputData[1],
															pInputData[1] + TwoLinePitch,
															pMapData + InputPitch,
															pMapData,
															pMapData + TwoLinePitch,
															pOutputData,
															InputPitch
														);
                pOutputData += OutputPitch;

                pInputData[0] += TwoLinePitch;
                pInputData[1] += TwoLinePitch;
                pMapData += TwoLinePitch;
            }

            memcpy(pOutputData, pInputData[1], InputInfo->bmiHeader.biWidth * 2);
            pOutputData += OutputPitch;
            memcpy(pOutputData, pInputData[0], InputInfo->bmiHeader.biWidth * 2);
            pOutputData += OutputPitch;
        }
    }
    // otherwise it's a 4:2:0 plannar format and we just 
    // worry about deinterlacing the luma and just copy the chroma
    else
    {
        InputPitch = InputInfo->bmiHeader.biWidth;
        TwoLinePitch = InputPitch * 2;
        OutputPitch = OutputInfo->bmiHeader.biWidth;
        
        // process luma
        if(IsTopLine == TRUE)
        {
            pInputData[1] += InputPitch;
            pMapData += InputPitch;

            memcpy(pOutputData, pInputData[0], InputPitch);
            pOutputData += OutputPitch;

            for(int i(0); i < InputInfo->bmiHeader.biHeight/2 - 1; ++i)
            {
                memcpy(pOutputData, pInputData[1], InputPitch);
                pOutputData += OutputPitch;

                pInputData[0] += TwoLinePitch;

                DiagFunctions[FunctionIndex][DIAG_PACK](
															pInputData[0],
															pInputData[1],
															pInputData[1] + TwoLinePitch,
															pMapData + InputPitch,
															pMapData,
															pMapData + TwoLinePitch,
															pOutputData,
															InputInfo->bmiHeader.biWidth
														);
                pOutputData += OutputPitch;

                pInputData[1] += TwoLinePitch;
                pMapData += TwoLinePitch;
            }
            memcpy(pOutputData, pInputData[1], InputInfo->bmiHeader.biWidth);
            pInputData[1] += InputPitch;
            pInputData[0] += TwoLinePitch;
            pOutputData += OutputPitch;
            pMapData += InputPitch;

            ProcessChromaBottom(pInputData[1], pInputData[0], pMapData, pOutputData, InputInfo, OutputInfo);
        }
        else
        {
            pInputData[0] += InputPitch;

            for(int i(0); i < InputInfo->bmiHeader.biHeight/2 - 1; ++i)
            {
                memcpy(pOutputData, pInputData[1], InputInfo->bmiHeader.biWidth);
                pOutputData += OutputPitch;

                DiagFunctions[FunctionIndex][DIAG_PACK](
															pInputData[0],
															pInputData[1],
															pInputData[1] + TwoLinePitch,
															pMapData + InputPitch,
															pMapData,
															pMapData + TwoLinePitch,
															pOutputData,
															InputInfo->bmiHeader.biWidth
														);
                pOutputData += OutputPitch;

                pInputData[0] += TwoLinePitch;
                pInputData[1] += TwoLinePitch;
                pMapData += TwoLinePitch;
            }

            memcpy(pOutputData, pInputData[1], InputInfo->bmiHeader.biWidth);
            pOutputData += OutputPitch;
            memcpy(pOutputData, pInputData[0], InputInfo->bmiHeader.biWidth);
            pOutputData += OutputPitch;
            pInputData[1] += TwoLinePitch;
            pInputData[0] += InputPitch;
            pMapData += TwoLinePitch;
    
            ProcessChromaTop(pInputData[1], pInputData[0], pMapData, pOutputData, InputInfo, OutputInfo);
        }
    }
    return S_OK;
}

void CDeint_Diag::ProcessChromaTop(BYTE* pUpperChroma, BYTE* pLowerChroma, BYTE* pMapData, BYTE* pOutputData, VIDEOINFOHEADER2* InputInfo, VIDEOINFOHEADER2* OutputInfo)
{
	DWORD LineLength;
	if(InputInfo->rcSource.right > 0)
	{
		LineLength = InputInfo->rcSource.right / 2;
	}
	else
	{
		LineLength = InputInfo->bmiHeader.biWidth / 2;
	}
    // copy V then U
    // there are biWidth / 2 x biHeight/2 of V 
    // followed by biWidth / 2 x biHeight/2 of U
    pLowerChroma += InputInfo->bmiHeader.biWidth / 2;
    int i;
    for(i = 0; i < InputInfo->bmiHeader.biHeight/4 - 1; ++i)
    {
        memcpy(pOutputData, pUpperChroma, LineLength);
        pOutputData += OutputInfo->bmiHeader.biWidth / 2;

        DiagFunctions[FunctionIndex][DIAG_PACK](
													pLowerChroma,
													pUpperChroma,
													pUpperChroma + InputInfo->bmiHeader.biWidth,
													pMapData + InputInfo->bmiHeader.biWidth / 2,
													pMapData,
													pMapData + InputInfo->bmiHeader.biWidth,
													pOutputData,
													InputInfo->bmiHeader.biWidth / 2
												);

        pOutputData += OutputInfo->bmiHeader.biWidth / 2;

        pUpperChroma += InputInfo->bmiHeader.biWidth;
        pLowerChroma += InputInfo->bmiHeader.biWidth;

        pMapData += InputInfo->bmiHeader.biWidth;
    }
    memcpy(pOutputData, pUpperChroma, LineLength);
    pOutputData += OutputInfo->bmiHeader.biWidth / 2;

    pUpperChroma += InputInfo->bmiHeader.biWidth;

    memcpy(pOutputData, pLowerChroma, LineLength);
    pOutputData += OutputInfo->bmiHeader.biWidth / 2;
    
    pLowerChroma += InputInfo->bmiHeader.biWidth / 2;

    pMapData += InputInfo->bmiHeader.biWidth;

    // do U

    pLowerChroma += InputInfo->bmiHeader.biWidth / 2;

    for(i = 0; i < InputInfo->bmiHeader.biHeight/4 - 1; ++i)
    {
        memcpy(pOutputData, pUpperChroma, LineLength);
        pOutputData += OutputInfo->bmiHeader.biWidth / 2;

        DiagFunctions[FunctionIndex][DIAG_PACK](
													pLowerChroma,
													pUpperChroma,
													pUpperChroma + InputInfo->bmiHeader.biWidth,
													pMapData + InputInfo->bmiHeader.biWidth / 2,
													pMapData,
													pMapData + InputInfo->bmiHeader.biWidth,
													pOutputData,
													InputInfo->bmiHeader.biWidth / 2
												);

        pOutputData += OutputInfo->bmiHeader.biWidth / 2;

        pUpperChroma += InputInfo->bmiHeader.biWidth;
        pLowerChroma += InputInfo->bmiHeader.biWidth;

        pMapData += InputInfo->bmiHeader.biWidth;
    }

    memcpy(pOutputData, pUpperChroma, LineLength);
    pOutputData += OutputInfo->bmiHeader.biWidth / 2;

    pUpperChroma += 3 * InputInfo->bmiHeader.biWidth / 2;

    memcpy(pOutputData, pLowerChroma, LineLength);
    pOutputData += OutputInfo->bmiHeader.biWidth / 2;
    
    pLowerChroma += InputInfo->bmiHeader.biWidth / 2;
}

void CDeint_Diag::ProcessChromaBottom(BYTE* pLowerChroma, BYTE* pUpperChroma, BYTE* pMapData, BYTE* pOutputData, VIDEOINFOHEADER2* InputInfo, VIDEOINFOHEADER2* OutputInfo)
{
	DWORD LineLength;
	if(InputInfo->rcSource.right > 0)
	{
		LineLength = InputInfo->rcSource.right / 2;
	}
	else
	{
		LineLength = InputInfo->bmiHeader.biWidth / 2;
	}
    // copy V then U
    // there are biWidth / 2 x biHeight/2 of V 
    // followed by biWidth / 2 x biHeight/2 of U

    memcpy(pOutputData, pUpperChroma, LineLength);
    pOutputData += OutputInfo->bmiHeader.biWidth / 2;

    pUpperChroma += InputInfo->bmiHeader.biWidth;

    pLowerChroma += InputInfo->bmiHeader.biWidth / 2;
    pMapData += InputInfo->bmiHeader.biWidth / 2;
    int i;
    for(i = 0; i < InputInfo->bmiHeader.biHeight/4 - 1; ++i)
    {
        memcpy(pOutputData, pLowerChroma, LineLength);
        pOutputData += OutputInfo->bmiHeader.biWidth / 2;

        DiagFunctions[FunctionIndex][DIAG_PACK](
													pUpperChroma,
													pLowerChroma,
													pLowerChroma + InputInfo->bmiHeader.biWidth,
													pMapData + InputInfo->bmiHeader.biWidth / 2,
													pMapData,
													pMapData + InputInfo->bmiHeader.biWidth,
													pOutputData,
													InputInfo->bmiHeader.biWidth / 2
												);

        pOutputData += OutputInfo->bmiHeader.biWidth / 2;

        pUpperChroma += InputInfo->bmiHeader.biWidth;
        pLowerChroma += InputInfo->bmiHeader.biWidth;

        pMapData += InputInfo->bmiHeader.biWidth;
    }

    memcpy(pOutputData, pLowerChroma, LineLength);
    pOutputData += OutputInfo->bmiHeader.biWidth / 2;
    
    pLowerChroma += InputInfo->bmiHeader.biWidth / 2;

    pMapData += InputInfo->bmiHeader.biWidth / 2;

    // do U

    memcpy(pOutputData, pUpperChroma, LineLength);
    pOutputData += OutputInfo->bmiHeader.biWidth / 2;

    pUpperChroma += InputInfo->bmiHeader.biWidth;

    pLowerChroma += InputInfo->bmiHeader.biWidth / 2;

    pMapData += InputInfo->bmiHeader.biWidth / 2;
    for(i = 0; i < InputInfo->bmiHeader.biHeight/4 - 1; ++i)
    {
        memcpy(pOutputData, pLowerChroma, LineLength);
        pOutputData += OutputInfo->bmiHeader.biWidth / 2;

        DiagFunctions[FunctionIndex][DIAG_PACK](
													pUpperChroma,
													pLowerChroma,
													pLowerChroma + InputInfo->bmiHeader.biWidth,
													pMapData + InputInfo->bmiHeader.biWidth / 2,
													pMapData,
													pMapData + InputInfo->bmiHeader.biWidth,
													pOutputData,
													InputInfo->bmiHeader.biWidth / 2
												);

        pOutputData += OutputInfo->bmiHeader.biWidth / 2;

        pUpperChroma += InputInfo->bmiHeader.biWidth;
        pLowerChroma += InputInfo->bmiHeader.biWidth;

        pMapData += InputInfo->bmiHeader.biWidth;
    }

    pUpperChroma += InputInfo->bmiHeader.biWidth;

    memcpy(pOutputData, pLowerChroma, LineLength);
    pOutputData += OutputInfo->bmiHeader.biWidth / 2;
    
    pLowerChroma += InputInfo->bmiHeader.biWidth / 2;
}


HRESULT CDeint_Diag::GetClassID(CLSID *pClsid)
{
	// Check for valid pointer
	if( NULL == pClsid )
	{
		return E_POINTER;
	}

	*pClsid = CLSID_CDeint_Diag;
	return S_OK;

} // GetClassID

STDMETHODIMP CDeint_Diag::get_Name(BSTR* Name)
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

STDMETHODIMP CDeint_Diag::get_License(eFreeLicense* License)
{
    if(License == NULL)
    {
        return E_POINTER;
    }
    *License = LGPL;
    return S_OK;
}

STDMETHODIMP CDeint_Diag::get_Authors(BSTR* Authors)
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

HRESULT CDeint_Diag::ParamChanged(DWORD dwParamIndex)
{
    // there are no parameters
    return E_UNEXPECTED;
}

///////////////////////
//
// Required COM stuff
//

BEGIN_COCLASS_TABLE(Classes)
    IMPLEMENTS_COCLASS(CDeint_Diag)
END_COCLASS_TABLE()

IMPLEMENT_DLL_MODULE_ROUTINES()

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
	if (DLL_PROCESS_ATTACH == dwReason)
	{
        g_hInstance = hInstance;
		DisableThreadLibraryCalls(hInstance);
        CPU_SetupFeatureFlag();
        if(CpuFeatureFlags & FEATURE_SSE)
        {
            FunctionIndex = 2;
        }
        else if(CpuFeatureFlags & FEATURE_3DNOW)
        {
            FunctionIndex = 1;
        }
        else
        {
            FunctionIndex = 0;
        }
	}
	else if (DLL_PROCESS_DETACH == dwReason)
    {
    }
	return TRUE;    // ok
}

STDAPI DllRegisterServer()
{
    HRESULT hr = DMODllRegisterDeintDMO(L"Diag", CLSID_CDeint_Diag);
	if(FAILED(hr))
	{
		return hr;
	}
	return ClassTableUpdateRegistry(GetThisInstance(), Classes, 0, FALSE, TRUE);
}

STDAPI DllUnregisterServer()
{
    HRESULT hr = DMODllUnregisterDeintDMO(CLSID_CDeint_Diag);
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

