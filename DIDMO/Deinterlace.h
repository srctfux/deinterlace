/////////////////////////////////////////////////////////////////////////////
// $Id: Deinterlace.h,v 1.2 2001-08-07 20:22:35 tobbej Exp $
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


#if !defined(AFX_DEINTERLACE_H__A0A0DA1F_F61E_4595_989A_B0B8552A6C4C__INCLUDED_)
#define AFX_DEINTERLACE_H__A0A0DA1F_F61E_4595_989A_B0B8552A6C4C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"       // main symbols

#include "HistoryData.h"
#include "DS_ApiCommon.h"
#include "DS_Deinterlace.h"
#include "cpu.h"

/////////////////////////////////////////////////////////////////////////////
// CDeinterlace

class CDeinterlace : 
	public IMediaObjectImpl<CDeinterlace, 1, 1>,			// 1 input, 1 outputs
	public IDispatchImpl<IDeinterlace, &IID_IDeinterlace, &LIBID_DEINTERLACEDMOLib>, 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDeinterlace,&CLSID_Deinterlace>,
	public ISupportErrorInfo,
	public ISpecifyPropertyPagesImpl<CDeinterlace>
{
public:
	CDeinterlace():m_DIPlugin(NULL),m_InputSize(0),m_LastFrameProcessed(true)
	{
		m_pUnkMarshaler = NULL;
		CPU_SetupFeatureFlag();
	}
BEGIN_COM_MAP(CDeinterlace)
	COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
	COM_INTERFACE_ENTRY(IMediaObject)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IDeinterlace)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()


BEGIN_PROP_MAP(CDeinterlace)
	PROP_PAGE(CLSID_DeinterlaceProperties)
	PROP_PAGE(CLSID_DeinterlaceSettings)
END_PROP_MAP()

//DECLARE_NOT_AGGREGATABLE(CDeinterlace) 
// Remove the comment from the line above if you don't want your object to 
// support aggregation. 

DECLARE_REGISTRY_RESOURCEID(IDR_Deinterlace)
DECLARE_GET_CONTROLLING_UNKNOWN()
	
	HRESULT FinalConstruct()
	{
		return CoCreateFreeThreadedMarshaler(
			GetControllingUnknown(), &m_pUnkMarshaler.p);
	}

	void FinalRelease()
	{
		m_pUnkMarshaler.Release();
		
		//unload plugin if its loaded
		if(m_DIPlugin!=NULL)
			UnloadPlugin();
	}

	CComPtr<IUnknown> m_pUnkMarshaler;

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// IMediaObjectImpl
	HRESULT InternalGetInputStreamInfo(DWORD dwInputStreamIndex, DWORD *pdwFlags);
	HRESULT InternalGetOutputStreamInfo(DWORD dwOutputStreamIndex, DWORD *pdwFlags);
	HRESULT InternalCheckInputType(DWORD dwInputStreamIndex, const DMO_MEDIA_TYPE *pmt);
	HRESULT InternalCheckOutputType(DWORD dwOutputStreamIndex, const DMO_MEDIA_TYPE *pmt);
	HRESULT InternalGetInputType(DWORD dwInputStreamIndex, DWORD dwTypeIndex,DMO_MEDIA_TYPE *pmt);
	HRESULT InternalGetOutputType(DWORD dwOutputStreamIndex, DWORD dwTypeIndex,DMO_MEDIA_TYPE *pmt);
	HRESULT InternalGetInputSizeInfo(DWORD dwInputStreamIndex, DWORD *pcbSize,DWORD *pcbMaxLookahead, DWORD *pcbAlignment);
	HRESULT InternalGetOutputSizeInfo(DWORD dwOutputStreamIndex, DWORD *pcbSize,DWORD *pcbAlignment);
	HRESULT InternalGetInputMaxLatency(DWORD dwInputStreamIndex, REFERENCE_TIME *prtMaxLatency);
	HRESULT InternalSetInputMaxLatency(DWORD dwInputStreamIndex, REFERENCE_TIME rtMaxLatency);
	HRESULT InternalFlush();
	HRESULT InternalDiscontinuity(DWORD dwInputStreamIndex);
	HRESULT InternalAllocateStreamingResources();
	HRESULT InternalFreeStreamingResources();
	HRESULT InternalProcessInput(DWORD dwInputStreamIndex, IMediaBuffer *pBuffer,DWORD dwFlags, REFERENCE_TIME rtTimestamp,REFERENCE_TIME rtTimelength);
	HRESULT InternalProcessOutput(DWORD dwFlags, DWORD cOutputBufferCount,DMO_OUTPUT_DATA_BUFFER *pOutputBuffers,DWORD *pdwStatus);
	HRESULT InternalAcceptingInput(DWORD dwInputStreamIndex);

	CHistoryData m_InputHistory[MAX_FIELD_HISTORY];
	int m_InputSize;
	bool m_LastFrameProcessed;
	
	DEINTERLACE_METHOD *m_DIPlugin;
	DEINTERLACE_INFO m_DIInfo;
	

// IDeinterlace
public:
	STDMETHOD(PluginHasUI)(BOOL *hasUI);
	STDMETHOD(ShowPluginUI)(long *hwndParent);
	STDMETHOD(get_SettingValue)(int nIndex, long *pValue);
	STDMETHOD(put_SettingValue)(int nIndex,long lValue);
	STDMETHOD(GetSettingCount)(long *pCount);
	STDMETHOD(GetSetting)(int nIndex,DISETTING *);
	STDMETHOD(GetPluginName)(unsigned char **szName);
	STDMETHOD(IsPluginLoaded)();
	STDMETHOD(UnloadPlugin)();
	STDMETHOD(LoadPlugin)(LPCSTR szFileName);
private:
	HRESULT CanPerformDeinterlace(const DMO_MEDIA_TYPE *pmt);
};

#endif // !defined(AFX_DEINTERLACE_H__A0A0DA1F_F61E_4595_989A_B0B8552A6C4C__INCLUDED_)
