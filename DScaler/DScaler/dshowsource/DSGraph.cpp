/////////////////////////////////////////////////////////////////////////////
// $Id: DSGraph.cpp,v 1.10 2002-04-07 14:52:13 tobbej Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Torbj�rn Jansson.  All rights reserved.
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
// Revision 1.9  2002/04/03 19:52:30  tobbej
// added some more logging to help track the filters submenu problem
//
// Revision 1.8  2002/03/26 19:48:59  adcockj
// Improved error handling in DShow code
//
// Revision 1.7  2002/03/17 21:43:23  tobbej
// added input resolution submenu
//
// Revision 1.6  2002/03/15 23:07:16  tobbej
// changed dropped frames counter to include dropped frames in source filter.
// added functions to enable/disable graph clock.
// started to make changing resolution posibel.
//
// Revision 1.5  2002/02/13 17:01:42  tobbej
// new filter properties menu
//
// Revision 1.4  2002/02/07 22:09:11  tobbej
// changed for new file input
//
// Revision 1.3  2002/02/05 17:27:46  tobbej
// update dropped/drawn fields stats
//
// Revision 1.2  2002/02/03 11:02:34  tobbej
// various updates for new filter
//
// Revision 1.1  2001/12/17 19:30:24  tobbej
// class for managing the capture graph
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DSGraph.cpp implementation of the CDShowGraph class.
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT

#include "dscaler.h"
#include "DSGraph.h"
#include "debug.h"
#include "CaptureDevice.h"
#include "DShowFileSource.h"
#include "PinEnum.h"
#include "DebugLog.h"

#include "..\..\..\DSRend\DSRend_i.c"

#include <dvdmedia.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDShowGraph::CDShowGraph(string device,string deviceName)
:m_pSource(NULL),m_pGraphState(State_Stopped)
{
	initGraph();
	createRenderer();

	m_pSource=new CDShowCaptureDevice(m_pGraph,device,deviceName);

#ifdef _DEBUG
	//this makes it posibel to connect graphedit to the graph
	AddToRot(m_pGraph,&m_hROT);
#endif
}

CDShowGraph::CDShowGraph(string filename)
:m_pSource(NULL),m_pGraphState(State_Stopped)
{
	initGraph();
	createRenderer();

	m_pSource=new CDShowFileSource(m_pGraph,filename);

#ifdef _DEBUG
	AddToRot(m_pGraph,&m_hROT);
#endif
}

CDShowGraph::~CDShowGraph()
{
	if(m_pSource!=NULL)
	{
		delete m_pSource;
		m_pSource=NULL;
	}

#ifdef _DEBUG
	RemoveFromRot(m_hROT);
#endif
}

void CDShowGraph::initGraph()
{
	HRESULT hr=m_pBuilder.CoCreateInstance(CLSID_CaptureGraphBuilder2);
	HRESULT hr1=m_pGraph.CoCreateInstance(CLSID_FilterGraph);

	if(FAILED(hr) || FAILED(hr1))
	{
		throw CDShowException("Failed to create graph");
	}
	
	//connect capture graph builder with the graph
	m_pBuilder->SetFiltergraph(m_pGraph);

	//get mediacontrol so we can start and stop the filter graph
	hr=m_pGraph.QueryInterface(&m_pControl);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to get IMediaControl interface",hr);
	}
}

void CDShowGraph::createRenderer()
{
	HRESULT hr=m_renderer.CoCreateInstance(CLSID_DSRendFilter);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to create dscaler input renderer filter\nMake sure that the filter is properly registered",hr);
	}
	
	hr=m_pGraph->AddFilter(m_renderer,L"DScaler video renderer");
	if(FAILED(hr))
	{
		throw CDShowException("Failed to add video renderer to filter graph",hr);
	}
	hr=m_renderer.QueryInterface(&m_DSRend);
	if(FAILED(hr))
	{
		throw CDShowException("QueryInterface failed on video renderer",hr);
	}
}

bool CDShowGraph::getNextSample(CComPtr<IMediaSample> &pSample)
{
	if(m_DSRend==NULL)
	{
		return false;
	}

	//FIXME: fix the timeout
	pSample=NULL;
	HRESULT hr=m_DSRend->GetNextSample(&pSample.p,400);
	if(FAILED(hr))
	{
		TRACE("GetNextSample failed\n");
		return false;
	}
	
	return true;
}

void CDShowGraph::start()
{
	if(m_pSource!=NULL)
	{
		if(!m_pSource->isConnected())
		{
			m_pSource->connect(m_renderer);
		}

		HRESULT hr=m_pControl->Run();
		if(FAILED(hr))
		{
			throw CDShowException("Failed to start filter graph",hr);
		}
		m_pGraphState=State_Running;
	}
}

void CDShowGraph::pause()
{
	if(m_pSource!=NULL)
	{
		if(!m_pSource->isConnected())
		{
			m_pSource->connect(m_renderer);
		}

		HRESULT hr=m_pControl->Pause();
		if(FAILED(hr))
		{
			throw CDShowException("Failed to pause filter graph",hr);
		}
		m_pGraphState=State_Paused;
	}
}

void CDShowGraph::stop()
{
	HRESULT hr=m_pControl->Stop();
	if(FAILED(hr))
	{
		throw CDShowException("Failed to stop filter graph",hr);
	}
	m_pGraphState=State_Stopped;
}

CDShowBaseSource* CDShowGraph::getSourceDevice()
{
	return m_pSource;
};

void CDShowGraph::getConnectionMediatype(AM_MEDIA_TYPE *pmt)
{
	ASSERT(pmt!=NULL);
	
	if(m_renderer==NULL)
	{
		throw CDShowException("Null pointer!!");
	}
	
	CDShowPinEnum pins(m_renderer,PINDIR_INPUT);
	CComPtr<IPin> inPin=pins.next();
	if(inPin==NULL)
	{
		throw CDShowException("Cant find input pin on video renderer");
	}
	
	HRESULT hr=inPin->ConnectionMediaType(pmt);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to get media type",hr);
	}
}

void CDShowGraph::showPropertyPage(HWND hParent,string caption,CComPtr<IBaseFilter> pFilter)
{
	USES_CONVERSION;
	CAUUID pages;
	HRESULT hr;

	if(pFilter==NULL)
	{
		return;
	}

	CComPtr<ISpecifyPropertyPages> pSProp;
	CComPtr<IUnknown> pUnk;
	
	hr=pFilter.QueryInterface(&pSProp);
	if(FAILED(hr))
	{
		//the filter dont have any property pages
		return;
	}

	//all filters must have a IUnknown so this shoud always work
	hr=pFilter.QueryInterface(&pUnk);
	ASSERT(SUCCEEDED(hr));

	hr=pSProp->GetPages(&pages);
	if(FAILED(hr))
	{
		//FIXME
	}
	
	hr=OleCreatePropertyFrame(hParent,0,0,A2OLE(caption.c_str()),1,&pUnk.p,pages.cElems,pages.pElems,MAKELCID(MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),SORT_DEFAULT),0,NULL);
	if(FAILED(hr))
	{
		AfxMessageBox("Failed to show property page!",MB_OK|MB_ICONEXCLAMATION);
	}
	
	CoTaskMemFree(pages.pElems);
}

bool CDShowGraph::getFilterName(int index,string &filterName,bool &hasPropertyPages)
{
	USES_CONVERSION;
	CDShowGenericEnum<IEnumFilters,IBaseFilter> filterEnum;
	HRESULT hr=m_pGraph->EnumFilters(&filterEnum.m_pEnum);
	if(FAILED(hr))
	{
		CDShowException tmp("",hr);
		LOG(1, "Failed to get filter enumerator!!! : %s",(LPCSTR)tmp.getErrorText());
		return false;
	}

	CComPtr<IBaseFilter> pFilter;
	try
	{
		pFilter=filterEnum[index];
	}
	catch(CDShowException e)
	{
        LOG(1, "DShow Exception - %s", (LPCSTR)e.getErrorText());
		return false;
	}

	FILTER_INFO info;
	hr=pFilter->QueryFilterInfo(&info);
	if(SUCCEEDED(hr))
	{
		filterName=W2A(info.achName);
		if(info.pGraph!=NULL)
		{
			info.pGraph->Release();
			info.pGraph=NULL;
		}
		CComPtr<ISpecifyPropertyPages> pPages;
		hasPropertyPages=SUCCEEDED(pFilter.QueryInterface(&pPages));
		return true;
	}
	return false;
}

void CDShowGraph::showPropertyPage(HWND hParent,int index)
{
	USES_CONVERSION;
	CDShowGenericEnum<IEnumFilters,IBaseFilter> filterEnum;
	HRESULT hr=m_pGraph->EnumFilters(&filterEnum.m_pEnum);
	if(FAILED(hr))
		return;
	
	CComPtr<IBaseFilter> pFilter;
	try
	{
		pFilter=filterEnum[index];
	}
	catch(CDShowException e)
	{
        LOG(1, "DShow Exception - %s", (LPCSTR)e.getErrorText());
		return;
	}
	FILTER_INFO info;
	string filterName;
	hr=pFilter->QueryFilterInfo(&info);
	if(SUCCEEDED(hr))
	{
		filterName=W2A(info.achName);
		if(info.pGraph!=NULL)
		{
			info.pGraph->Release();
			info.pGraph=NULL;
		}
		showPropertyPage(hParent,filterName,pFilter);
	}
}

long CDShowGraph::getDroppedFrames()
{
	HRESULT hr;
	if(m_pQualProp==NULL)
	{
		hr=m_renderer->QueryInterface(IID_IQualProp,(void**)&m_pQualProp);
		if(FAILED(hr))
		{
			throw CDShowException("Failed to get IQualProp interface on renderer filter (most likely a bug)",hr);
		}
	}
	
	int frames;
	hr=m_pQualProp->get_FramesDroppedInRenderer(&frames);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to get dropped frames count",hr);
	}
	return frames+m_pSource->getNumDroppedFrames();
}

void CDShowGraph::changeRes(long x,long y)
{
	if(m_renderer==NULL)
	{
		return;
	}

	if(m_pStreamCfg==NULL)
	{
		//this will throw an exception if it cant find IAMStreamConfig interface
		findStreamConfig();
	}

	FILTER_STATE oldState=getState();
	if(oldState!=State_Stopped)
	{
		//the only time the format can be changed is when the graph is stopped
		stop();
	}

	//get current mediatype
	AM_MEDIA_TYPE *mt=NULL;
	HRESULT hr=m_pStreamCfg->GetFormat(&mt);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to get old mediatype",hr);
	}
	
	//create the new media type
	AM_MEDIA_TYPE newType;
	memset(&newType,0,sizeof(AM_MEDIA_TYPE));
	BITMAPINFOHEADER *pbmiHeader=NULL;

	newType.majortype=MEDIATYPE_Video;
	newType.subtype=MEDIASUBTYPE_YUY2;
	newType.bFixedSizeSamples=TRUE;
	
	//copy some info from the old media type and initialize the new format block
	if(mt->pbFormat!=NULL)
	{
		if(mt->formattype==FORMAT_VideoInfo)
		{
			VIDEOINFOHEADER *newInfoHeader=(VIDEOINFOHEADER*)CoTaskMemAlloc(sizeof(VIDEOINFOHEADER));
			memset(newInfoHeader,0,sizeof(VIDEOINFOHEADER));
			newType.formattype=FORMAT_VideoInfo;
			newType.cbFormat=sizeof(VIDEOINFOHEADER);
			newType.pbFormat=(BYTE*)newInfoHeader;

			VIDEOINFOHEADER *videoInfo=(VIDEOINFOHEADER*)mt->pbFormat;
			newInfoHeader->dwBitRate=videoInfo->dwBitRate;
			newInfoHeader->dwBitErrorRate=videoInfo->dwBitErrorRate;
			newInfoHeader->AvgTimePerFrame=videoInfo->AvgTimePerFrame;
			newInfoHeader->bmiHeader=videoInfo->bmiHeader;
			pbmiHeader=&newInfoHeader->bmiHeader;
		}
		else if(mt->formattype==FORMAT_VideoInfo2)
		{
			VIDEOINFOHEADER2 *newInfoHeader=(VIDEOINFOHEADER2*)CoTaskMemAlloc(sizeof(VIDEOINFOHEADER2));
			memset(newInfoHeader,0,sizeof(VIDEOINFOHEADER2));
			newType.formattype=FORMAT_VideoInfo2;
			newType.cbFormat=sizeof(VIDEOINFOHEADER2);
			newType.pbFormat=(BYTE*)newInfoHeader;

			VIDEOINFOHEADER2 *videoInfo2=(VIDEOINFOHEADER2*)mt->pbFormat;
			newInfoHeader->dwBitRate=videoInfo2->dwBitRate;
			newInfoHeader->dwBitErrorRate=videoInfo2->dwBitErrorRate;
			newInfoHeader->AvgTimePerFrame=videoInfo2->AvgTimePerFrame;
			newInfoHeader->bmiHeader=videoInfo2->bmiHeader;
			pbmiHeader=&newInfoHeader->bmiHeader;
		}
	}
	
	//if pbFormat is null then there is something strange going on with the renderer filter
	//most likely a bug
	ASSERT(newType.pbFormat!=NULL);

	pbmiHeader->biSize=sizeof(BITMAPINFOHEADER);
	pbmiHeader->biWidth=x;
	pbmiHeader->biHeight=y;
	pbmiHeader->biSizeImage=x*y*pbmiHeader->biBitCount/8;

	//change the format
	hr=m_pStreamCfg->SetFormat(&newType);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to change resolution",hr);
	}
	
	//restore old graph state
	if(oldState==State_Running)
	{
		start();
	}
	else if(oldState==State_Paused)
	{
		pause();
	}
}

bool CDShowGraph::isValidRes(long x,long y)
{
	if(m_renderer==NULL)
	{
		return false;
	}

	if(m_pStreamCfg==NULL)
	{
		try
		{
			findStreamConfig();
		}
		catch(CDShowException e)
		{
            LOG(1, "DShow Exception - %s", (LPCSTR)e.getErrorText());
			return false;
		}
	}
	int iCount,iSize;
	HRESULT hr=m_pStreamCfg->GetNumberOfCapabilities(&iCount,&iSize);
	if(FAILED(hr))
		return false;

	for(int i=0;i<iCount;i++)
	{
		AM_MEDIA_TYPE *pMT=NULL;
		VIDEO_STREAM_CONFIG_CAPS vCaps;
		hr=m_pStreamCfg->GetStreamCaps(i,&pMT,(BYTE*)&vCaps);
		if(SUCCEEDED(hr))
		{
			//free the mediatype
			if(pMT->pUnk!=NULL)
			{
				pMT->pUnk->Release();
				pMT->pUnk=NULL;
			}
			if(pMT->cbFormat>0 && pMT->pbFormat!=NULL)
			{
				CoTaskMemFree(pMT->pbFormat);
				pMT->pbFormat=NULL;
			}
			
			//maybe check VIDEO_STREAM_CONFIG_CAPS::VideoStandard too

			//check x and y
			if(x<=vCaps.MaxOutputSize.cx && y<=vCaps.MaxOutputSize.cy && 
				x>=vCaps.MinOutputSize.cx && y>=vCaps.MinOutputSize.cy &&
				x%vCaps.OutputGranularityX==0 && y%vCaps.OutputGranularityY==0)
			{
				return true;
			}
		}
	}
	return false;
}
void CDShowGraph::findStreamConfig()
{
	CDShowPinEnum rendPins(m_renderer,PINDIR_INPUT);
	CComPtr<IPin> inPin;

	inPin=rendPins.next();
	
	//if this assert is trigered there is most likely s bug in the renderer filter
	ASSERT(inPin!=NULL);

	//get the upstream pin
	CComPtr<IPin> outPin;
	HRESULT hr=inPin->ConnectedTo(&outPin);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to find pin",hr);
	}

	//get IAMStreamConfig on the output pin
	m_pStreamCfg=NULL;
	hr=outPin.QueryInterface(&m_pStreamCfg);
	if(FAILED(hr))
	{
		throw CDShowException("Query interface for IAMStreamConfig failed",hr);
	}
}

void CDShowGraph::disableClock()
{
	//prevent multiple disableClock calls
	if(m_pOldRefClk!=NULL)
		return;

	CComPtr<IMediaFilter> pMFilter;
	HRESULT hr=m_pGraph.QueryInterface(&pMFilter);
	if(FAILED(hr))
	{
		throw CDShowException("QueryInterface for IMediaFilter failed on filter graph",hr);
	}
	
	//save old reference clock
	hr=pMFilter->GetSyncSource(&m_pOldRefClk);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to get old reference clock",hr);
	}
	
	//disable the current one
	hr=pMFilter->SetSyncSource(NULL);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to set reference clock",hr);
	}
}

void CDShowGraph::restoreClock()
{
	if(m_pOldRefClk==NULL)
		return;
	
	CComPtr<IMediaFilter> pMFilter;
	HRESULT hr=m_pGraph.QueryInterface(&pMFilter);
	if(FAILED(hr))
	{
		throw CDShowException("QueryInterface for IMediaFilter failed on filter graph",hr);
	}

	//restor the reference clock
	hr=pMFilter->SetSyncSource(m_pOldRefClk);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to set reference clock",hr);
	}
	m_pOldRefClk=NULL;

}
#endif
