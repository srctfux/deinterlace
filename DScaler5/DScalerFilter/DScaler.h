///////////////////////////////////////////////////////////////////////////////
// $Id: DScaler.h,v 1.1.1.1 2003-04-30 13:01:20 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
// DScalerFilter.dll - DirectShow filter for deinterlacing and video processing
// Copyright (c) 2003 John Adcock
///////////////////////////////////////////////////////////////////////////////
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
///////////////////////////////////////////////////////////////////////////////

#ifndef __DSCALER_H_
#define __DSCALER_H_

#include "resource.h"       // main symbols
#include "InputPin.h"
#include "OutputPin.h"

class ATL_NO_VTABLE DECLSPEC_UUID("0D71870A-7563-11D7-B84A-0002A5623377") CDScaler : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDScaler, &__uuidof(CDScaler)>,
    public ISpecifyPropertyPages,
    public IBaseFilter,
	public IPersistStream,
    public IMediaSeeking,
    public IMediaParamInfo,
    public IMediaParams
{
public:
	CDScaler();

DECLARE_REGISTRY_RESOURCEID(IDR_DSCALER)
DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDScaler)
	COM_INTERFACE_ENTRY(IBaseFilter)
	COM_INTERFACE_ENTRY(IMediaFilter)
	COM_INTERFACE_ENTRY2(IPersist, IPersistStream)
	COM_INTERFACE_ENTRY(IMediaSeeking)
	COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
	COM_INTERFACE_ENTRY(IPersistStream)
    COM_INTERFACE_ENTRY(IMediaParamInfo)
    COM_INTERFACE_ENTRY(IMediaParams)
	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

	HRESULT FinalConstruct();
	void FinalRelease();

	CComPtr<IUnknown> m_pUnkMarshaler;

// IBaseFilter
public:
    STDMETHOD(EnumPins)(IEnumPins **ppEnum);
    STDMETHOD(FindPin)(LPCWSTR Id, IPin **ppPin);
    STDMETHOD(QueryFilterInfo)(FILTER_INFO *pInfo);
    STDMETHOD(JoinFilterGraph)(IFilterGraph *pGraph, LPCWSTR pName);
    STDMETHOD(QueryVendorInfo)(LPWSTR *pVendorInfo);
    STDMETHOD(Stop)(void);
    STDMETHOD(Pause)(void);
    STDMETHOD(Run)(REFERENCE_TIME tStart);
    STDMETHOD(GetState)(DWORD dwMilliSecsTimeout, FILTER_STATE *State);
    STDMETHOD(SetSyncSource)(IReferenceClock *pClock);
    STDMETHOD(GetSyncSource)(IReferenceClock **pClock);
    STDMETHOD(GetClassID)(CLSID __RPC_FAR *pClassID);

// IMediaSeeking
public:
    STDMETHOD(GetCapabilities)(DWORD *pCapabilities);
    STDMETHOD(CheckCapabilities)(DWORD *pCapabilities);
    STDMETHOD(IsFormatSupported)(const GUID *pFormat);
    STDMETHOD(QueryPreferredFormat)(GUID *pFormat);
    STDMETHOD(GetTimeFormat)(GUID *pFormat);
    STDMETHOD(IsUsingTimeFormat)(const GUID *pFormat);
    STDMETHOD(SetTimeFormat)(const GUID *pFormat);
    STDMETHOD(GetDuration)(LONGLONG *pDuration);
    STDMETHOD(GetStopPosition)(LONGLONG *pStop);
    STDMETHOD(GetCurrentPosition)(LONGLONG *pCurrent);
    STDMETHOD(ConvertTimeFormat)(
                                    LONGLONG *pTarget,
                                    const GUID *pTargetFormat,
                                    LONGLONG Source,
                                    const GUID *pSourceFormat
                                );
    STDMETHOD(SetPositions)( 
                            LONGLONG *pCurrent,
                            DWORD dwCurrentFlags,
                            LONGLONG *pStop,
                            DWORD dwStopFlags
                           );
    STDMETHOD(GetPositions)(LONGLONG *pCurrent, LONGLONG *pStop);
    STDMETHOD(GetAvailable)(LONGLONG *pEarliest, LONGLONG *pLatest);
    STDMETHOD(SetRate)(double dRate);
    STDMETHOD(GetRate)(double *pdRate);
    STDMETHOD(GetPreroll)(LONGLONG *pllPreroll);

// ISpecifyPropertyPages
public:
    STDMETHOD(GetPages)(CAUUID *pPages);

// IPersistStream
public:
    STDMETHOD(IsDirty)(void);
    STDMETHOD(Load)(IStream __RPC_FAR *pStm);
    STDMETHOD(Save)(IStream __RPC_FAR *pStm, BOOL fClearDirty);
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER __RPC_FAR *pcbSize);

// IMediaParams
public:
    STDMETHOD(GetParam)(DWORD dwParamIndex, MP_DATA *pValue);
    STDMETHOD(SetParam)(DWORD dwParamIndex,MP_DATA value);
    STDMETHOD(AddEnvelope)(DWORD dwParamIndex,DWORD cPoints,MP_ENVELOPE_SEGMENT *ppEnvelope);
    STDMETHOD(FlushEnvelope)( DWORD dwParamIndex,REFERENCE_TIME refTimeStart,REFERENCE_TIME refTimeEnd);
    STDMETHOD(SetTimeFormat)( GUID guidTimeFormat,MP_TIMEDATA mpTimeData);

// IMediaParamInfo
public:
    STDMETHOD(GetParamCount)(DWORD *pdwParams);
    STDMETHOD(GetParamInfo)(DWORD dwParamIndex,MP_PARAMINFO *pInfo);
    STDMETHOD(GetParamText)(DWORD dwParamIndex,WCHAR **ppwchText);
    STDMETHOD(GetNumTimeFormats)(DWORD *pdwNumTimeFormats);
    STDMETHOD(GetSupportedTimeFormat)(DWORD dwFormatIndex,GUID *pguidTimeFormat);        
    STDMETHOD(GetCurrentTimeFormat)( GUID *pguidTimeFormat,MP_TIMEDATA *pTimeData);


public:
    CInputPin* m_InputPin;
    COutputPin* m_OutputPin;
    CComPtr<IReferenceClock> m_RefClock;
    FILTER_STATE m_State;
    IFilterGraph2* m_Graph;
    WCHAR m_Name[MAX_FILTER_NAME];
    BOOL m_IsDirty;
};

#endif