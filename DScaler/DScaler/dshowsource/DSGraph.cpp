/////////////////////////////////////////////////////////////////////////////
// $Id: DSGraph.cpp,v 1.1 2001-12-17 19:30:24 tobbej Exp $
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
#include "PinEnum.h"

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
:m_pSource(NULL)
{
	initGraph();
	createRenderer();

	m_pSource=new CDShowCaptureDevice(m_pGraph,device,deviceName);

#ifdef _DEBUG
	//this makes it posibel to connect graphedit to the graph
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
	HRESULT hr=m_renderer.CoCreateInstance(CLSID_DScalerInputRenderer);
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

bool CDShowGraph::getNextSample(BYTE *pEvenField,BYTE *pOddField,long cbSize)
{
	if(m_DSRend==NULL)
	{
		return false;
	}

	//FIXME: fix the timeout
	HRESULT hr=m_DSRend->GetNextSample(pEvenField,pOddField,&cbSize,400);
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
		
		//testing
		setRes(768,576);

		HRESULT hr=m_pControl->Run();
		if(FAILED(hr))
		{
			throw CDShowException("Failed to start filter graph",hr);
		}
	}
}

void CDShowGraph::stop()
{
	HRESULT hr=m_pControl->Stop();
	if(FAILED(hr))
	{
		throw CDShowException("Failed to stop filter graph",hr);
	}
}

CDShowCaptureDevice* CDShowGraph::getCaptureDevice()
{
	if(m_pSource==NULL)
	{
		throw CDShowException("No capture device");
	}
	return m_pSource;
};

void CDShowGraph::getCurrentMediatype(AM_MEDIA_TYPE *pmt)
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

void CDShowGraph::showRendererProperies(HWND hParent)
{
	USES_CONVERSION;
	CAUUID pages;
	HRESULT hr;

	if(m_renderer==NULL)
	{
		return;
	}

	CComQIPtr<ISpecifyPropertyPages> pSProp;
	CComQIPtr<IUnknown,&IID_IUnknown> pUnk;
	
	pUnk=m_renderer;
	pSProp=m_renderer;
	hr=pSProp->GetPages(&pages);
	if(FAILED(hr))
	{
		//FIXME
	}

	hr=OleCreatePropertyFrame(hParent,0,0,A2OLE("DirectShow Renderer"),1,&pUnk.p,pages.cElems,pages.pElems,MAKELCID(MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),SORT_DEFAULT),0,NULL);
	if(FAILED(hr))
	{
		//FIXME
	}
	
	CoTaskMemFree(pages.pElems);
}

void CDShowGraph::setRes(long x,long y)
{
	if(m_renderer==NULL)
	{
		return;
	}

	if(m_pStreamCfg==NULL)
	{
		findStreamConfig();
	}

	int count=0;
	int size=0;
	HRESULT hr=m_pStreamCfg->GetNumberOfCapabilities(&count,&size);
	if(FAILED(hr))
	{
		throw CDShowException("",hr);
	}
	AM_MEDIA_TYPE *mt;

	hr=m_pStreamCfg->GetFormat(&mt);
	if(FAILED(hr))
	{
		
	}
	BITMAPINFOHEADER *bmi=NULL;
	if(mt->formattype==FORMAT_VideoInfo)
	{
		VIDEOINFOHEADER *videoInfo=(VIDEOINFOHEADER*)mt->pbFormat;
		bmi=&(videoInfo->bmiHeader);
	}
	else if(mt->formattype==FORMAT_VideoInfo2)
	{
		VIDEOINFOHEADER2 *videoInfo2=(VIDEOINFOHEADER2*)mt->pbFormat;
		bmi=&(videoInfo2->bmiHeader);
	}
	bmi->biWidth=x;
	bmi->biHeight=y;
	bmi->biSizeImage=x*y*bmi->biBitCount/8;
	
	hr=m_pStreamCfg->SetFormat(mt);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to change resolution",hr);
	}
}

void CDShowGraph::findStreamConfig()
{
	CDShowPinEnum rendPins(m_renderer,PINDIR_INPUT);
	CComPtr<IPin> inPin;

	inPin=rendPins.next();
	if(inPin==NULL)
	{
		//oops, no pin
	}

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

#endif