/////////////////////////////////////////////////////////////////////////////
// $Id: DSRendFilter.cpp,v 1.5 2002-03-08 11:14:04 tobbej Exp $
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
// Revision 1.4  2002/02/07 13:08:20  tobbej
// fixed some syncronization problems
//
// Revision 1.3  2002/02/06 15:01:24  tobbej
// fixed race condition betwen stop and recive
// updated some comments
//
// Revision 1.2  2002/02/03 20:01:39  tobbej
// made framerate counter work
//
// Revision 1.1.1.1  2002/02/03 10:52:53  tobbej
// First import of new direct show renderer filter
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DSRendFilter.cpp Implementation of CDSRendFilter
 */

#include "StdAfx.h"
#include "DSRend.h"
#include "DSRendFilter.h"
CDSRendFilter::CDSRendFilter() :
	m_iAvgFrameRate(0),
	m_iAvg(0),
	m_iDev(0),
	m_cFramesDrawn(0),
	m_cFramesDropped(0),
	m_iJitter(0),
	m_pGraph(NULL),
	m_pFilterName(NULL),
	m_InputPin(this),
	m_filterState(State_Stopped),
	m_pEventSink(NULL),
	m_tStart(0)
{

}

CDSRendFilter::~CDSRendFilter()
{

}

HRESULT CDSRendFilter::FinalConstruct()
{
	ATLTRACE(_T("%s(%d) : CDSRendFilter::FinalConstruct\n"),__FILE__,__LINE__);
	return S_OK;
}

HRESULT CDSRendFilter::FinalRelease()
{
	ATLTRACE(_T("%s(%d) : CDSRendFilter::FinalRelease\n"),__FILE__,__LINE__);
	return S_OK;
}

// IPersist
HRESULT CDSRendFilter::GetClassID(CLSID *pClassID)
{
	ATLTRACE(_T("%s(%d) : CDSRendFilter::GetClassID\n"),__FILE__,__LINE__);
	if(pClassID==NULL)
		return E_POINTER;
	*pClassID=GetObjectCLSID();

	return S_OK;
}

// IMediaFilter
HRESULT CDSRendFilter::Stop()
{
	ATLTRACE(_T("%s(%d) : CDSRendFilter::Stop\n"),__FILE__,__LINE__);
	CAutoLockCriticalSection lock(&m_Lock);
	
	if(m_filterState==State_Stopped)
		return S_OK;
	
	//allow Stop() to succeed even if the filter is not connected
	if(!m_InputPin.isConnected())
	{
		m_filterState=State_Stopped;
		return S_OK;
	}
	
	//free buffered sample and sync with rendering thread
	stopWait();
	CAutoLockCriticalSection rendLock(&m_renderLock);
	m_sampleLock.Lock();
	m_pSample=NULL;
	m_sampleLock.Unlock();

	m_filterState=State_Stopped;
	return S_OK;
}

HRESULT CDSRendFilter::Pause()
{
	ATLTRACE(_T("%s(%d) : CDSRendFilter::Pause\n"),__FILE__,__LINE__);
	CAutoLockCriticalSection lock(&m_Lock);
	
	if(m_filterState==State_Paused)
		return S_OK;
	if(!m_InputPin.isConnected())
	{
		m_filterState=State_Paused;
		return S_OK;
	}
	m_filterState=State_Paused;
	return S_OK;
}

HRESULT CDSRendFilter::Run(REFERENCE_TIME tStart)
{
	ATLTRACE(_T("%s(%d) : CDSRendFilter::Run\n"),__FILE__,__LINE__);
	CAutoLockCriticalSection lock(&m_Lock);
	
	if(m_filterState==State_Running)
		return S_OK;
	
	//check if input is not connected
	if(!m_InputPin.isConnected())
	{
		//send end of stream notification to filtergraph manager
		//since the input is not connected, IPin::EndOfStream will not be called
		sendNotifyMsg(EC_COMPLETE,S_OK,(LONG_PTR)(IBaseFilter*)this);
		m_filterState=State_Running;
		return S_OK;
	}
	
	//reset stats
	m_iAvgFrameRate=0;
	m_cFramesDrawn=0;
	m_cFramesDropped=0;

	m_tStart=tStart;
	m_filterState=State_Running;
	
	return S_OK;
}

bool CDSRendFilter::isStopped()
{
	CAutoLockCriticalSection lock(&m_Lock);
	return m_filterState==State_Stopped;
}

HRESULT CDSRendFilter::GetState(DWORD dwMilliSecsTimeout,FILTER_STATE *State)
{
	ATLTRACE(_T("%s(%d) : CDSRendFilter::GetState\n"),__FILE__,__LINE__);
	CAutoLockCriticalSection lock(&m_Lock);

	*State=m_filterState;
	return S_OK;
}

HRESULT CDSRendFilter::SetSyncSource(IReferenceClock *pClock)
{
	ATLTRACE(_T("%s(%d) : CDSRendFilter::SetSyncSource\n"),__FILE__,__LINE__);
	CAutoLockCriticalSection lock(&m_Lock);

	m_pRefClk=pClock;
	return S_OK;
}

HRESULT CDSRendFilter::GetSyncSource(IReferenceClock **pClock)
{
	ATLTRACE(_T("%s(%d) : CDSRendFilter::GetSyncSource\n"),__FILE__,__LINE__);
	CAutoLockCriticalSection lock(&m_Lock);

	if(pClock==NULL)
	{
		return E_POINTER;
	}
	if(m_pRefClk!=NULL)
	{
		m_pRefClk.p->AddRef();
	}

	*pClock=m_pRefClk;
	return S_OK;
}

// IBaseFilter
HRESULT CDSRendFilter::EnumPins(IEnumPins **ppEnum)
{
	//ATLTRACE(_T("%s(%d) : CDSRendFilter::EnumPins\n"),__FILE__,__LINE__);
	if(ppEnum==NULL)
		return E_POINTER;

	CComObject<CComEnum<IEnumPins,&IID_IEnumPins,IPin*,_CopyInterface<IPin> > > *pEnum=NULL;
	HRESULT hr=CComObject<CComEnum<IEnumPins,&IID_IEnumPins,IPin*,_CopyInterface<IPin> > >::CreateInstance(&pEnum);
	if(FAILED(hr))
		return hr;

	IPin* pins[1];
	pins[0]=&m_InputPin;

	hr=pEnum->Init(&pins[0],&pins[1],GetUnknown(),AtlFlagCopy);
	if(FAILED(hr))
	{
		delete pEnum;
		return hr;
	}
	//addref the enumerator interface, nessesary since CreateInstance dont addref the new object
	pEnum->AddRef();
	*ppEnum=pEnum;

	return hr;
}

HRESULT CDSRendFilter::FindPin(LPCWSTR Id,IPin **ppPin)
{
	ATLTRACE(_T("%s(%d) : CDSRendFilter::FindPin\n"),__FILE__,__LINE__);
	if(ppPin==NULL)
	{
		return E_POINTER;
	}
	//shoud probably check Id first and return VFW_E_NOT_FOUND if Id dont match the pin,
	//but since the filter only has one input it cant be any other pin
	HRESULT hr=m_InputPin.QueryInterface(IID_IPin,(void**)&(*ppPin));
	if(FAILED(hr))
	{
		*ppPin=NULL;
		return hr;
	}
	return S_OK;
}

HRESULT CDSRendFilter::QueryFilterInfo(FILTER_INFO *pInfo)
{
	ATLTRACE(_T("%s(%d) : CDSRendFilter::QueryFilterInfo\n"),__FILE__,__LINE__);
	if(pInfo==NULL)
		return E_POINTER;
	
	if(m_pFilterName!=NULL)
	{
		wcsncpy(pInfo->achName,m_pFilterName,128);
	}
	else
	{
		memset(pInfo->achName,0,sizeof(pInfo->achName));
	}
	
	pInfo->pGraph=m_pGraph;
	if(m_pGraph!=NULL)
	{
		m_pGraph->AddRef();
	}

	return S_OK;
}

HRESULT CDSRendFilter::JoinFilterGraph(IFilterGraph *pGraph,LPCWSTR pName)
{
	ATLTRACE(_T("%s(%d) : CDSRendFilter::JoinFilterGraph\n"),__FILE__,__LINE__);
	CAutoLockCriticalSection lock(&m_Lock);
	
	//dont hold a reference on the IMediaEventSink or IFilterGraph interface
	//(circular reference problem)
	m_pGraph=pGraph;
	if(pGraph==NULL)
	{
		m_pEventSink=NULL;
	}
	else
	{
		HRESULT hr=pGraph->QueryInterface(IID_IMediaEventSink,(void **)&m_pEventSink);
		if(SUCCEEDED(hr))
		{
			m_pEventSink->Release();
		}
		else
		{
			m_pEventSink=NULL;
		}
	}

	//delete filter name
	if(m_pFilterName!=NULL)
	{
		delete[] m_pFilterName;
		m_pFilterName=NULL;
	}

	//copy filter name if any
	if(pName!=NULL)
	{
		int size=wcslen(pName)+1;
		m_pFilterName=new WCHAR[size];
		wcsncpy(m_pFilterName,pName,size);
	}

	return S_OK;
}

HRESULT CDSRendFilter::QueryVendorInfo(LPWSTR *pVendorInfo)
{
	ATLTRACE(_T("%s(%d) : CDSRendFilter::QueryVendorInfo\n"),__FILE__,__LINE__);
	return E_NOTIMPL;
}

// IQualProp
HRESULT CDSRendFilter::get_AvgFrameRate(int *piAvgFrameRate)
{
	//ATLTRACE(_T("%s(%d) : CDSRendFilter::get_AvgFrameRate\n"),__FILE__,__LINE__);
	CAutoLockCriticalSection lock(&m_Lock);

	if(piAvgFrameRate==NULL)
		return E_POINTER;
	
	//update framerate if we have a reference clock
	if(m_pRefClk!=NULL)
	{
		REFERENCE_TIME now;
		HRESULT hr=m_pRefClk->GetTime(&now);
		if(SUCCEEDED(hr))
		{
			m_iAvgFrameRate=m_cFramesDrawn*10000/((now-m_tStart)/100000);
		}
	}
	
	*piAvgFrameRate=m_iAvgFrameRate;
	return S_OK;
}

HRESULT CDSRendFilter::get_AvgSyncOffset(int *piAvg)
{
	//ATLTRACE(_T("%s(%d) : CDSRendFilter::get_AvgSyncOffset\n"),__FILE__,__LINE__);
	CAutoLockCriticalSection lock(&m_Lock);

	if(piAvg==NULL)
		return E_POINTER;
	/*
	*piAvg=m_iAvg;
	return S_OK;
	*/
	return E_FAIL;
}

HRESULT CDSRendFilter::get_DevSyncOffset(int *piDev)
{
	//ATLTRACE(_T("%s(%d) : CDSRendFilter::get_DevSyncOffset\n"),__FILE__,__LINE__);
	CAutoLockCriticalSection lock(&m_Lock);

	if(piDev==NULL)
		return E_POINTER;
	/*
	*piDev=m_iDev;
	return S_OK;
	*/
	return E_FAIL;
}

HRESULT CDSRendFilter::get_FramesDrawn(int *pcFramesDrawn)
{
	//ATLTRACE(_T("%s(%d) : CDSRendFilter::get_FramesDrawn\n"),__FILE__,__LINE__);
	CAutoLockCriticalSection lock(&m_Lock);

	if(pcFramesDrawn==NULL)
		return E_POINTER;
	*pcFramesDrawn=m_cFramesDrawn;
	return S_OK;
}

HRESULT CDSRendFilter::get_FramesDroppedInRenderer(int *pcFrames)
{
	//ATLTRACE(_T("%s(%d) : CDSRendFilter::get_FramesDroppedInRenderer\n"),__FILE__,__LINE__);
	CAutoLockCriticalSection lock(&m_Lock);

	if(pcFrames==NULL)
		return E_POINTER;
	*pcFrames=m_cFramesDropped;
	return S_OK;
}

HRESULT CDSRendFilter::get_Jitter(int *piJitter)
{
	//ATLTRACE(_T("%s(%d) : CDSRendFilter::get_Jitter\n"),__FILE__,__LINE__);
	CAutoLockCriticalSection lock(&m_Lock);

	if(piJitter==NULL)
		return E_POINTER;
	/*
	*piJitter=m_iJitter;
	return S_OK;
	*/
	return E_FAIL;
}

HRESULT CDSRendFilter::sendNotifyMsg(long eventCode,LONG_PTR param1,LONG_PTR param2)
{
	ATLTRACE(_T("%s(%d) : CDSRendFilter::sendNotifyMsg\n"),__FILE__,__LINE__);
	if(m_pEventSink!=NULL)
	{
		HRESULT hr=m_pEventSink->Notify(eventCode,param1,param2);
		return hr;
	}
	//maybe we shoud return a different error code if the IMediaEventSink interface is not found?
	return E_NOTIMPL;
}

HRESULT CDSRendFilter::waitForTime(REFERENCE_TIME rtStreamTime)
{
	//ATLTRACE(_T("%s(%d) : CDSRendFilter::waitForTime\n"),__FILE__,__LINE__);
	
	if(m_pRefClk==NULL)
	{
		return VFW_E_NO_CLOCK;
	}
	HRESULT hr=m_pRefClk->AdviseTime(m_tStart,rtStreamTime,(HEVENT)m_refClockEvent.getHandle(),&m_refClockCookie);
	if(FAILED(hr))
		return hr;

	if(m_refClockEvent.wait(INFINITE))
		return S_OK;

	return E_FAIL;
}

void CDSRendFilter::stopWait()
{
	ATLTRACE(_T("%s(%d) : CDSRendFilter::stopWait\n"),__FILE__,__LINE__);
	
	m_refClockEvent.setEvent();
	if(m_pRefClk!=NULL)
	{
		HRESULT hr=m_pRefClk->Unadvise(m_refClockCookie);
		ATLASSERT(SUCCEEDED(hr));
	}
}

HRESULT CDSRendFilter::beginFlush()
{
	ATLTRACE(_T("%s(%d) : CDSRendFilter::beginFlush\n"),__FILE__,__LINE__);
	stopWait();
	
	CAutoLockCriticalSection lock(&m_sampleLock);
	m_pSample=NULL;
	
	return S_OK;
}

HRESULT CDSRendFilter::renderSample(IMediaSample *pSample)
{
	//input pins holds the render lock

	//is the old sample retrieved?
	if(m_nextSampleReady.wait(0))
	{
		m_cFramesDropped++;
	}
	m_sampleLock.Lock();
	m_pSample=pSample;
	m_sampleLock.Unlock();
	m_nextSampleReady.setEvent();

	return S_OK;
}

STDMETHODIMP CDSRendFilter::GetNextSample(IMediaSample **ppSample,DWORD dwTimeout)
{
	//ATLTRACE(_T("%s(%d) : CDSRendFilter::GetNextSample\n"),__FILE__,__LINE__);
	if(ppSample==NULL)
	{
		return E_POINTER;
	}
	
	if(!m_nextSampleReady.wait(dwTimeout))
	{
		*ppSample=NULL;
		return VFW_E_TIMEOUT;
	}
	m_sampleLock.Lock();
	if(m_pSample==NULL)
	{
		m_sampleLock.Unlock();
		*ppSample=NULL;
		return E_FAIL;
	}
	m_cFramesDrawn++;
	*ppSample=m_pSample;
	
	//addref the returned sample and release our buffered sample,
	//this is nessesary if the upstream filter only provides one singel IMediaSample
	(*ppSample)->AddRef();
	m_pSample=NULL;
	m_sampleLock.Unlock();
	return S_OK;
}

// IMediaSeeking
HRESULT CDSRendFilter::getMediaSeeking(CComPtr<IMediaSeeking> &pSeeking)
{
	CComPtr<IPin> pOutPin;
	HRESULT hr=m_InputPin.ConnectedTo(&pOutPin);
	if(FAILED(hr))
		return E_NOTIMPL;
	hr=pOutPin.QueryInterface(&pSeeking);
	if(FAILED(hr))
		return E_NOTIMPL;
	return S_OK;
}

HRESULT CDSRendFilter::GetCapabilities(DWORD *pCapabilities)
{
	CComPtr<IMediaSeeking> pSeeking;
	if(FAILED(getMediaSeeking(pSeeking)))
		return E_NOTIMPL;
	return pSeeking->GetCapabilities(pCapabilities);;
}

HRESULT CDSRendFilter::CheckCapabilities( DWORD *pCapabilities)
{
	CComPtr<IMediaSeeking> pSeeking;
	if(FAILED(getMediaSeeking(pSeeking)))
		return E_NOTIMPL;

	return pSeeking->CheckCapabilities(pCapabilities);
}

HRESULT CDSRendFilter::IsFormatSupported(const GUID *pFormat)
{
	CComPtr<IMediaSeeking> pSeeking;
	if(FAILED(getMediaSeeking(pSeeking)))
		return E_NOTIMPL;

	return pSeeking->IsFormatSupported(pFormat);
}

HRESULT CDSRendFilter::QueryPreferredFormat(GUID *pFormat)
{
	CComPtr<IMediaSeeking> pSeeking;
	if(FAILED(getMediaSeeking(pSeeking)))
		return E_NOTIMPL;

	return pSeeking->QueryPreferredFormat(pFormat);
}

HRESULT CDSRendFilter::GetTimeFormat(GUID *pFormat)
{
	CComPtr<IMediaSeeking> pSeeking;
	if(FAILED(getMediaSeeking(pSeeking)))
		return E_NOTIMPL;

	return pSeeking->GetTimeFormat(pFormat);
}

HRESULT CDSRendFilter::IsUsingTimeFormat(const GUID *pFormat)
{
	CComPtr<IMediaSeeking> pSeeking;
	if(FAILED(getMediaSeeking(pSeeking)))
		return E_NOTIMPL;

	return pSeeking->IsUsingTimeFormat(pFormat);
}

HRESULT CDSRendFilter::SetTimeFormat(const GUID *pFormat)
{
	CComPtr<IMediaSeeking> pSeeking;
	if(FAILED(getMediaSeeking(pSeeking)))
		return E_NOTIMPL;

	return pSeeking->SetTimeFormat(pFormat);
}

HRESULT CDSRendFilter::GetDuration(LONGLONG *pDuration)
{
	CComPtr<IMediaSeeking> pSeeking;
	if(FAILED(getMediaSeeking(pSeeking)))
		return E_NOTIMPL;

	return pSeeking->GetDuration(pDuration);
}

HRESULT CDSRendFilter::GetStopPosition(LONGLONG *pStop)
{
	CComPtr<IMediaSeeking> pSeeking;
	if(FAILED(getMediaSeeking(pSeeking)))
		return E_NOTIMPL;

	return pSeeking->GetStopPosition(pStop);
}

HRESULT CDSRendFilter::GetCurrentPosition(LONGLONG *pCurrent)
{
	CComPtr<IMediaSeeking> pSeeking;
	if(FAILED(getMediaSeeking(pSeeking)))
		return E_NOTIMPL;

	return pSeeking->GetCurrentPosition(pCurrent);
}

HRESULT CDSRendFilter::ConvertTimeFormat(LONGLONG *pTarget,const GUID *pTargetFormat,LONGLONG Source,const GUID *pSourceFormat)
{
	CComPtr<IMediaSeeking> pSeeking;
	if(FAILED(getMediaSeeking(pSeeking)))
		return E_NOTIMPL;

	return pSeeking->ConvertTimeFormat(pTarget,pTargetFormat,Source,pSourceFormat);
}

HRESULT CDSRendFilter::SetPositions(LONGLONG *pCurrent,DWORD dwCurrentFlags,LONGLONG *pStop,DWORD dwStopFlags)
{
	CComPtr<IMediaSeeking> pSeeking;
	if(FAILED(getMediaSeeking(pSeeking)))
		return E_NOTIMPL;

	return pSeeking->SetPositions(pCurrent,dwCurrentFlags,pStop,dwStopFlags);
}

HRESULT CDSRendFilter::GetPositions(LONGLONG *pCurrent,LONGLONG *pStop)
{
	CComPtr<IMediaSeeking> pSeeking;
	if(FAILED(getMediaSeeking(pSeeking)))
		return E_NOTIMPL;

	return pSeeking->GetPositions(pCurrent,pStop);
}

HRESULT CDSRendFilter::GetAvailable(LONGLONG *pEarliest,LONGLONG *pLatest)
{
	CComPtr<IMediaSeeking> pSeeking;
	if(FAILED(getMediaSeeking(pSeeking)))
		return E_NOTIMPL;

	return pSeeking->GetAvailable(pEarliest,pLatest);
}

HRESULT CDSRendFilter::SetRate(double dRate)
{
	CComPtr<IMediaSeeking> pSeeking;
	if(FAILED(getMediaSeeking(pSeeking)))
		return E_NOTIMPL;

	return pSeeking->SetRate(dRate);
}

HRESULT CDSRendFilter::GetRate(double *pdRate)
{
	CComPtr<IMediaSeeking> pSeeking;
	if(FAILED(getMediaSeeking(pSeeking)))
		return E_NOTIMPL;

	return pSeeking->GetRate(pdRate);
}

HRESULT CDSRendFilter::GetPreroll(LONGLONG *pllPreroll)
{
	CComPtr<IMediaSeeking> pSeeking;
	if(FAILED(getMediaSeeking(pSeeking)))
		return E_NOTIMPL;

	return pSeeking->GetPreroll(pllPreroll);
}

