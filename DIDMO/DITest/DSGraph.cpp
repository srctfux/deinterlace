/////////////////////////////////////////////////////////////////////////////
// $Id: DSGraph.cpp,v 1.3 2001-09-19 17:45:14 tobbej Exp $
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
// Revision 1.2  2001/08/08 16:13:28  tobbej
// added a few comments
//
// Revision 1.1  2001/08/08 16:01:04  tobbej
// new test app for playing movie files thru dmo filter
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DITest.h"
#include "DSGraph.h"

#include "debug.h"
#include "DeinterlaceDMO_i.c"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDSGraph::CDSGraph(HWND parentWnd)
:m_dwRegister(0),m_parentWnd(parentWnd)
{
	HRESULT hr;
	initGraph();
	createOverlayMixer();
	setupVideoWindow();
	createDIFilter();

	//the template operator= will call queryinterface on pGraf to get a IMediaControl pointer
	m_pMediaControl=m_pGraph;
	m_pEvent=m_pGraph;

	//this will make it posibel to control the graph via graphedit
	hr=AddToRot(m_pGraph,&m_dwRegister);
}

CDSGraph::~CDSGraph()
{
	if(m_dwRegister!=0)
	{
		RemoveFromRot(m_dwRegister);
		m_dwRegister=0;
	}
}

void CDSGraph::showDIProperties()
{
	USES_CONVERSION;
	CAUUID pages;
	HRESULT hr;

	if(m_pDIFilter==NULL)
		return;

	CComQIPtr<ISpecifyPropertyPages> pSProp;
	CComQIPtr<IUnknown,&IID_IUnknown> pUnk;
		pUnk=m_pDIFilter;
	pSProp=m_pDIFilter;
	hr=pSProp->GetPages(&pages);
	
	hr=OleCreatePropertyFrame(m_parentWnd,0,0,A2OLE("Deinterlace settings"),1,&pUnk.p,pages.cElems,pages.pElems,MAKELCID(MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL),SORT_DEFAULT),0,NULL);
	
	CoTaskMemFree(pages.pElems);
}

void CDSGraph::initGraph()
{
	if(FAILED(m_pGraph.CoCreateInstance(CLSID_FilterGraph)))
	{
		AfxMessageBox("failed to create graph builde");
		return;
	}
	if(FAILED(m_pBuilder.CoCreateInstance(CLSID_CaptureGraphBuilder2)))
	{
		AfxMessageBox("failed to create capturegraph builder");
		return;
	}
	//connect CaptureGraphBuilder to graph
	m_pBuilder->SetFiltergraph(m_pGraph);
}

void CDSGraph::createOverlayMixer()
{
	HRESULT hr;
	//create overlay mixer and add it to the graph
	m_pOverlay.CoCreateInstance(CLSID_OverlayMixer);
	m_pGraph->AddFilter(m_pOverlay,L"Overlay Mixer");

	CComPtr<IPin> pPin;
	hr=m_pBuilder->FindPin(m_pOverlay,PINDIR_OUTPUT,NULL,NULL,TRUE,0,&pPin);
	if(FAILED(hr))
	{
		AfxMessageBox("Failed to find output pin on overlay mixer");
		return;
	}
	//render this pin so dshow connects a video window to overlaymixer
	hr=m_pGraph->Render(pPin);
	if(FAILED(hr))
	{
		AfxMessageBox("Failed to render output pin of overlay mixer");
		return;
	}
}

void CDSGraph::createDIFilter()
{
	HRESULT hr;
	//create and insert deinterlace dmo filter in graf
	/*hr=m_pDIFilter.CoCreateInstance(CLSID_DMOWrapperFilter);
	if(FAILED(hr))
	{
		AfxMessageBox("failed to create deinterlace dmo");
		return;
	}
	else
	{
		CComQIPtr<IDMOWrapperFilter> pWrap;
		pWrap=m_pDIFilter;
		hr=pWrap->Init(CLSID_Deinterlace,CLSID_VideoCompressorCategory);
		if(FAILED(hr))
		{
			AfxMessageBox("failed to init dmo wrapper filter");
			return;
		}
		
	}*/
	hr=m_pDIFilter.CoCreateInstance(CLSID_DIDMOWrapper);
	if(FAILED(hr))
	{
		AfxMessageBox("failed to create deinterlace filter");
		return;
	}
	hr=m_pGraph->AddFilter(m_pDIFilter,L"Deinterlace filter");
}

void CDSGraph::openFile(CString filename)
{
	USES_CONVERSION;
	HRESULT hr;
	//insert specified file in graph
	hr=m_pGraph->AddSourceFilter(A2W(filename),L"Source",&m_pSrc);
	if(FAILED(hr))
	{
		AfxMessageBox("failed to add file");
		return;
	}

	//build graf, first call will render pSrc thru pDIFilter and displays on pOverlay
	//second renderstream renders the audio if any
	hr=m_pBuilder->RenderStream(NULL,NULL,m_pSrc,m_pDIFilter,m_pOverlay);
	hr=m_pBuilder->RenderStream(NULL,&MEDIATYPE_Audio,m_pSrc,NULL,NULL);
}

void CDSGraph::play()
{
	if(m_pMediaControl==NULL)
		return;
	m_pMediaControl->Run();
}

void CDSGraph::pause()
{
	if(m_pMediaControl==NULL)
		return;
	m_pMediaControl->Pause();
}

void CDSGraph::stop()
{
	if(m_pMediaControl==NULL)
		return;
	m_pMediaControl->Stop();
}

void CDSGraph::setupVideoWindow()
{
	//get IVideoWindow interface
	if(FAILED(m_pGraph.QueryInterface(&m_pVidWin)))
	{}
	
	//setup videowindow to be a child window
	m_pVidWin->put_AutoShow(OAFALSE);
	m_pVidWin->put_Owner((OAHWND)m_parentWnd);
	m_pVidWin->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS);
	m_pVidWin->put_Visible(OATRUE);
}

void CDSGraph::resizeVideoWindow(long left, long top, long width, long height)
{
	if(m_pVidWin!=NULL)
	{
		m_pVidWin->SetWindowPosition(left,top,width,height);
	}
}

void CDSGraph::forwardWndMsg(HWND hwnd,UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	/*
	this function is used to forward important messages to IVideoWindow
	by forwarding WM_SYSCOLORCHANGE and WM_DISPLAYCHANGE the program survives display mode changes
	of course the overlay mixer must support the new resolution and color depth
	*/
	if(m_pVidWin!=NULL)
	{
		HRESULT hr=m_pVidWin->NotifyOwnerMessage((OAHWND)hwnd,uMsg,wParam,lParam);
	}
}
