///////////////////////////////////////////////////////////////////////////////
// $Id: DSBufferedInputPin.cpp,v 1.1 2004-05-24 06:29:27 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.8  2004/04/29 16:16:46  adcockj
// Yet more reconnection fixes
//
// Revision 1.7  2004/04/20 16:30:31  adcockj
// Improved Dynamic Connections
//
// Revision 1.6  2004/04/14 16:31:34  adcockj
// Subpicture fixes, AFD started and minor fixes
//
// Revision 1.5  2004/02/27 17:08:16  adcockj
// Improved locking at state changes
// Better error handling at state changes
//
// Revision 1.4  2004/02/25 17:14:03  adcockj
// Fixed some timing bugs
// Tidy up of code
//
// Revision 1.3  2004/02/12 17:06:45  adcockj
// Libary Tidy up
// Fix for stopping problems
//
// Revision 1.2  2004/02/10 13:24:12  adcockj
// Lots of bug fixes + corrected interlaced YV12 upconversion
//
// Revision 1.1  2004/02/06 12:17:17  adcockj
// Major changes to the Libraries to remove ATL and replace with YACL
// First draft of Mpeg2 video decoder filter
// Broken DScalerFilter part converted to new library
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DSBasePin.h"
#include "DSBufferedInputPin.h"
#include "DSOutputPin.h"
#include "DSBaseFilter.h"
#include "EnumMediaTypes.h"
#include "MediaBufferWrapper.h"
#include "Process.h"

CDSBufferedInputPin::CDSBufferedInputPin() :
    CDSInputPin()
{
    LOG(DBGLOG_ALL, ("CDSBufferedInputPin::CDSBufferedInputPin\n"));

    m_SamplesReadyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_ThreadStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_WorkerThread = NULL;
    
}

CDSBufferedInputPin::~CDSBufferedInputPin()
{
    CloseHandle(m_SamplesReadyEvent);
    CloseHandle(m_ThreadStopEvent);

    LOG(DBGLOG_ALL, ("CDSBufferedInputPin::~CDSBufferedInputPin\n"));
}


STDMETHODIMP CDSBufferedInputPin::BeginFlush(void)
{
    LOG(DBGLOG_FLOW, ("CDSBufferedInputPin::BeginFlush\n"));

    HRESULT hr = CDSInputPin::BeginFlush();

    return hr;
}

STDMETHODIMP CDSBufferedInputPin::EndFlush(void)
{
    HRESULT hr = CDSInputPin::EndFlush();

    return hr;
}

STDMETHODIMP CDSBufferedInputPin::Receive(IMediaSample *InSample)
{
    LOG(DBGLOG_ALL, ("CDSBufferedInputPin::Receive\n"));

    if(InSample == NULL)
    {
        return E_POINTER;
    }
    if(m_Flushing == TRUE)
    {
        return S_FALSE;
    }
    if(m_Filter->m_State == State_Stopped)
    {
        return VFW_E_WRONG_STATE;
    }

    HRESULT hr = S_OK;

    CProtectCode WhileVarInScope2(&m_SamplesLock);
    m_Samples.push(InSample);

    SetEvent(m_SamplesReadyEvent);
    
    return hr;
}

void CDSBufferedInputPin::ProcessingThread(void* pParam)
{
    CDSBufferedInputPin* pThis = (CDSBufferedInputPin*)pParam;

    while(1)
    {
        HRESULT hr = S_OK;
        DWORD dwWaitResult;
        HANDLE hEvents[2]; 

        hEvents[0] = pThis->m_ThreadStopEvent;  // thread's read event
        hEvents[1] = pThis->m_SamplesReadyEvent; 

        dwWaitResult = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);

        switch (dwWaitResult) 
        {
            case WAIT_OBJECT_0: 
                ResetEvent(pThis->m_ThreadStopEvent);
                ExitThread(0);
                break; 
            case WAIT_OBJECT_0 + 1:
                ResetEvent(pThis->m_SamplesReadyEvent);
                hr = pThis->ProcessBufferedSamples();
                break;
            default: 
                ExitThread(1); 
                break;
        }
    }
}

HRESULT CDSBufferedInputPin::ProcessBufferedSamples()
{
    long size = 1;
    HRESULT hr = S_OK;

    while(size > 0 && SUCCEEDED(hr))
    {
        SI(IMediaSample) InSample;
        
        {
    	    CProtectCode WhileVarInScope(&m_SamplesLock);
            size = m_Samples.size();
            if(size > 0)
            {
                InSample = m_Samples.front();
                m_Samples.pop();
            }
        }

        if(InSample)
        {
            hr = ProcessBufferedSample(InSample.GetNonAddRefedInterface());
        }

        {
    	    CProtectCode WhileVarInScope(&m_SamplesLock);
            size = m_Samples.size();
        }
    }

    return hr;
}

HRESULT CDSBufferedInputPin::ProcessBufferedSample(IMediaSample* InSample)
{
    // all code below here is protected
    // from runnning at the same time as other 
    // functions with this line
    CProtectCode WhileVarInScope(this);
    
    AM_SAMPLE2_PROPERTIES InSampleProperties;
    ZeroMemory(&InSampleProperties, sizeof(AM_SAMPLE2_PROPERTIES));

    //LogSample(InSample, "New Input Sample");

    HRESULT hr = GetSampleProperties(InSample, &InSampleProperties);
    CHECK(hr);

    // check for media type changes on the input side
    // a NULL means the type is the same as last time
    if(InSampleProperties.pMediaType != NULL)
    {
		FixupMediaType(InSampleProperties.pMediaType);

        // this shouldn't ever fail as a good filter will have
        // called this already but I've seen a filter ignore the
        // results of a queryaccept
        hr = QueryAccept(InSampleProperties.pMediaType);
        if(hr != S_OK)
        {
            FreeMediaType(InSampleProperties.pMediaType);
            return VFW_E_INVALIDMEDIATYPE;
        }
        SetType(InSampleProperties.pMediaType);
    }

	// check to see if we are blocked
    // need to check this before we get each sample
    CheckForBlocking();

	// make sure we don't bother sending nothing down
	if(InSampleProperties.lActual > 0 && InSampleProperties.pbBuffer != NULL) 
	{
		// Send the sample to the filter for processing 
		hr = m_Filter->ProcessSample(InSample, &InSampleProperties, this);
	}

    // make sure that anything that needs to be cleaned up
    // is actually cleaned up
    if(InSampleProperties.pMediaType != NULL)
    {
        FreeMediaType(InSampleProperties.pMediaType);
    }
    return hr;
}


STDMETHODIMP CDSBufferedInputPin::ReceiveCanBlock(void)
{
    LOG(DBGLOG_ALL, ("CDSBufferedInputPin::ReceiveCanBlock\n"));
    return S_OK;
}

HRESULT CDSBufferedInputPin::Activate()
{
    HRESULT hr = CDSInputPin::Activate();
    CHECK(hr);

    DWORD IDThread;

    LOG(DBGLOG_FLOW, ("CDSBufferedInputPin::Activate\n"));

    m_WorkerThread = CreateThread(NULL, 0, 
        (LPTHREAD_START_ROUTINE) ProcessingThread, 
        this,  // pass event handle
        0, &IDThread); 

    return hr;
}

HRESULT CDSBufferedInputPin::Deactivate()
{
    HRESULT hr = CDSInputPin::Deactivate();
    CHECK(hr);

    LOG(DBGLOG_ALL, ("CDSOutputPin::Deactivate\n"));

    SetEvent(m_ThreadStopEvent);

    return hr;
}

