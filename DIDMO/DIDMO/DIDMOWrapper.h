// DIDMOWrapper.h : Declaration of the CDIDMOWrapper

#ifndef __DIDMOWRAPPER_H_
#define __DIDMOWRAPPER_H_

#include "resource.h"       // main symbols

#include <dmodshow.h>
#include <dshow.h>

/////////////////////////////////////////////////////////////////////////////
// CDIDMOWrapper
class ATL_NO_VTABLE CDIDMOWrapper : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDIDMOWrapper, &CLSID_DIDMOWrapper>,
	public IDIDMOWrapper
{
public:
	CDIDMOWrapper()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_DIDMOWRAPPER)
DECLARE_NOT_AGGREGATABLE(CDIDMOWrapper)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDIDMOWrapper)
	COM_INTERFACE_ENTRY(IDIDMOWrapper)
	COM_INTERFACE_ENTRY_AGGREGATE_BLIND(m_DIDMOFilter.p)
END_COM_MAP()

	HRESULT FinalConstruct()
	{
		CComQIPtr<IDMOWrapperFilter> pWrap;
		HRESULT hr=m_DIDMOFilter.CoCreateInstance(CLSID_DMOWrapperFilter,this);
		if(FAILED(hr))
			return hr;
		
		hr=m_DIDMOFilter.QueryInterface(&pWrap);
		if(FAILED(hr))
			return hr;
		hr=pWrap->Init(CLSID_Deinterlace,CLSID_VideoCompressorCategory);
		return hr;
	}
	
	void FinalRelease()
	{
		m_DIDMOFilter.Release();
	}

// IDIDMOWrapper
public:
private:
	CComPtr<IUnknown> m_DIDMOFilter;
};

#endif //__DIDMOWRAPPER_H_
