/////////////////////////////////////////////////////////////////////////////
// $Id: DSGraph.h,v 1.1 2001-08-08 16:01:04 tobbej Exp $
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

#if !defined(AFX_DSGRAPH_H__91894738_B9A0_478C_A369_6BF95933FD76__INCLUDED_)
#define AFX_DSGRAPH_H__91894738_B9A0_478C_A369_6BF95933FD76__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDSGraph  
{
public:
	CDSGraph(HWND parentWnd);
	virtual ~CDSGraph();
	
	//forward window message to videowindow
	void forwardWndMsg(HWND hwnd,UINT uMsg, WPARAM wParam,LPARAM lParam);
	
	//resize videowindow
	void resizeVideoWindow(long left,long top,long width,long height);
	
	//controls to start/stop graph
	void stop();
	void pause();
	void play();
	
	//loads a file in filter graph, doesnt start playing of graph
	void openFile(CString filename);
	
	//shows propertypage for deinterlace plugin
	void showDIProperties();

private:
	void setupVideoWindow();
	void createDIFilter();
	void createOverlayMixer();
	void initGraph();
	CComPtr<IGraphBuilder> m_pGraph;
	CComPtr<ICaptureGraphBuilder2> m_pBuilder;
	CComQIPtr<IMediaControl> m_pMediaControl;
	CComQIPtr<IMediaEvent> m_pEvent;
	CComPtr<IVideoWindow> m_pVidWin;
	
	//source filter (file)
	CComPtr<IBaseFilter> m_pSrc;
	CComQIPtr<IBaseFilter> m_pDIFilter;

	//overlay mixer
	CComPtr<IBaseFilter> m_pOverlay;
	
	DWORD m_dwRegister;
	HWND m_parentWnd;
};

#endif // !defined(AFX_DSGRAPH_H__91894738_B9A0_478C_A369_6BF95933FD76__INCLUDED_)
