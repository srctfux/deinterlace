/////////////////////////////////////////////////////////////////////////////
// $Id: DSRendInPin.h,v 1.4 2002-03-11 19:26:01 tobbej Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Torbjörn Jansson.  All rights reserved.
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
// Revision 1.3  2002/02/07 13:08:20  tobbej
// fixed some syncronization problems
//
// Revision 1.2  2002/02/06 15:01:23  tobbej
// fixed race condition betwen stop and recive
// updated some comments
//
// Revision 1.1.1.1  2002/02/03 10:52:53  tobbej
// First import of new direct show renderer filter
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DSRendInPin.h Declaration of the CDSRendInPin
 */

#ifndef __DSRENDINPIN_H_
#define __DSRENDINPIN_H_

#include "resource.h"       // main symbols
#include "CustomComObj.h"
#include "Event.h"

//forward declaration of the filter class
class CDSRendFilter;

/**
 * The input pin of the renderer filter.
 * @todo implement scheduling of samples. done, but might need some tweaking.
 * @todo add support for changing format when graph is running
 */
class ATL_NO_VTABLE CDSRendInPin : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDSRendInPin, &CLSID_DSRendInPin>,
	public IPin,
	public IMemInputPin,
	public IAMFilterMiscFlags
{
public:
	CDSRendInPin(CDSRendFilter *pFilter);
	~CDSRendInPin()
	{
	
	}
	HRESULT FinalConstruct();
	HRESULT FinalRelease();

DECLARE_REGISTRY_RESOURCEID(IDR_DSRENDINPIN)
DECLARE_NOT_AGGREGATABLE(CDSRendInPin)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDSRendInPin)
	COM_INTERFACE_ENTRY(IPin)
	COM_INTERFACE_ENTRY(IMemInputPin)
	COM_INTERFACE_ENTRY(IAMFilterMiscFlags)
END_COM_MAP()
	
	//allow CCustomComObject to access m_pFilter
	friend class CCustomComObject<CDSRendInPin,CDSRendFilter*>;

// IPin
	STDMETHOD(Connect(IPin *pReceivePin,const AM_MEDIA_TYPE *pmt));
	STDMETHOD(ReceiveConnection(IPin *pConnector,const AM_MEDIA_TYPE *pmt));
	STDMETHOD(Disconnect());
	STDMETHOD(ConnectedTo(IPin **ppPin));
	STDMETHOD(ConnectionMediaType(AM_MEDIA_TYPE *pmt));
	STDMETHOD(QueryPinInfo(PIN_INFO *pInfo));
	STDMETHOD(QueryId(LPWSTR *Id));
	STDMETHOD(QueryAccept(const AM_MEDIA_TYPE *pmt));
	STDMETHOD(EnumMediaTypes(IEnumMediaTypes **ppEnum));
	STDMETHOD(QueryInternalConnections(IPin **apPin,ULONG *nPin));
	STDMETHOD(EndOfStream());
	STDMETHOD(BeginFlush());
	STDMETHOD(EndFlush());
	STDMETHOD(NewSegment(REFERENCE_TIME tStart,REFERENCE_TIME tStop,double dRate));
	STDMETHOD(QueryDirection(PIN_DIRECTION *pPinDir));
	
// IMemInputPin
	STDMETHOD(GetAllocator(IMemAllocator **ppAllocator));
	STDMETHOD(NotifyAllocator(IMemAllocator *pAllocator,BOOL bReadOnly));
	STDMETHOD(GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps));
	STDMETHOD(Receive(IMediaSample *pSample));
	STDMETHOD(ReceiveMultiple(IMediaSample **pSamples,long nSamples,long *nSamplesProcessed));
	STDMETHOD(ReceiveCanBlock());

// IAMFilterMiscFlags
	ULONG STDMETHODCALLTYPE GetMiscFlags(void);

public:
	/// @return true if connected
	bool isConnected() {return m_pConnected!=NULL;};
	
	void resumePause() {m_resumePauseEvent.setEvent();}

private:
	/**
	 * Checks if a mediatype is acceptable
	 * @param pmt mediatype to check
	 */
	HRESULT CheckMediaType(const AM_MEDIA_TYPE *pmt);
	
	/// @return true if pin is flushing
	bool isFlushing() {return m_bFlushing;}

private:
	///The filter that this pin belongs to.
	CDSRendFilter *m_pFilter;
	
	///streaming lock
	CComAutoCriticalSection m_Lock;
	
	///Event used to make the rendering block when filter is paused
	CEvent m_resumePauseEvent;

	///pin that this filter is connected to. if this is NULL this pin is not connected
	CComPtr<IPin> m_pConnected;
	///mediatype of pin connection.
	AM_MEDIA_TYPE m_mt;

	///number of buffers we need from the allocator. for now it is always set to 2 in the constructor
	long m_cAllocBuffers;
	
	///flag to signal flushing.
	bool m_bFlushing;
};

#endif //__DSRENDINPIN_H_
