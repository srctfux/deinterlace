/////////////////////////////////////////////////////////////////////////////
// $Id: DIDMOWrapper.h,v 1.2 2001-09-19 17:55:24 tobbej Exp $
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
//
//////////////////////////////////////////////////////////////////////////////

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
