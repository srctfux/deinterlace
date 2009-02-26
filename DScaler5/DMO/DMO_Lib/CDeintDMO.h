////////////////////////////////////////////////////////////////////////////
// $Id$
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

#pragma once 

#include "CVideoDMO.h"

/////////////////////////////////////////////////////////////////////////////
// CInPlaceDMO
class CDeintDMO : 
    public CVideoDMO,
    public IDeinterlace
{

public:
    CDeintDMO(LPCWSTR Name, long FieldsToBuffer, long FieldsDelay, long Complexity);    // Constructor
    ~CDeintDMO();    // Destructor

protected:
    //IMediaObjectImpl Methods   
    STDMETHOD(InternalProcessInput)(DWORD dwInputStreamIndex, IMediaBuffer *pBuffer, DWORD dwFlags, REFERENCE_TIME rtTimestamp, REFERENCE_TIME rtTimelength);
    STDMETHOD(InternalProcessOutput)(DWORD dwFlags, DWORD cOutputBufferCount, DMO_OUTPUT_DATA_BUFFER *pOutputBuffers, DWORD *pdwStatus);
    HRESULT InternalGetInputType(DWORD dwInputStreamIndex, DWORD dwTypeIndex, DMO_MEDIA_TYPE *pmt);
    HRESULT InternalGetOutputType(DWORD dwOutputStreamIndex, DWORD dwTypeIndex, DMO_MEDIA_TYPE *pmt);
    HRESULT InternalCheckInputType(DWORD dwInputStreamIndex, const DMO_MEDIA_TYPE *pmt);
    HRESULT InternalCheckOutputType(DWORD dwInputStreamIndex, const DMO_MEDIA_TYPE *pmt);

    // IDeinterlace methods
    STDMETHOD(get_ComplexityIndex)(long* pComplexity);
    STDMETHOD(Process)(IInterlacedBufferStack* Stack, IMediaBuffer* pOutBuffer) = 0;

protected:
    virtual HRESULT ProcessSingleFrame(IInterlacedBufferStack* Stack, IMediaBuffer* pOutBuffer, DWORD MidIndex);
    void ProcessPlanarChroma(BYTE* pInputData, BYTE* pOutputData, VIDEOINFOHEADER2* InputInfo, VIDEOINFOHEADER2* OutputInfo);

protected:
    long m_ComplexityIndex;
};
