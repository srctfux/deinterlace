/////////////////////////////////////////////////////////////////////////////
// $Id: ChildView.cpp,v 1.1 2001-08-08 16:01:04 tobbej Exp $
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

#include "stdafx.h"
#include "DITest.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildView

CChildView::CChildView()
:m_pGraph(NULL)
{
}

CChildView::~CChildView()
{
	if(m_pGraph!=NULL)
	{
		delete m_pGraph;
		m_pGraph=NULL;
	}
}


BEGIN_MESSAGE_MAP(CChildView,CWnd )
	//{{AFX_MSG_MAP(CChildView)
	ON_WM_PAINT()
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_PROPERTIES, OnFileProperties)
	ON_COMMAND(ID_CONTROLS_PLAY, OnControlsPlay)
	ON_COMMAND(ID_CONTROLS_PAUSE, OnControlsPause)
	ON_COMMAND(ID_CONTROLS_STOP, OnControlsStop)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	ON_WM_SYSCOLORCHANGE()
	ON_MESSAGE(WM_DISPLAYCHANGE,OnDisplayChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	// Do not call CWnd::OnPaint() for painting messages
}


void CChildView::OnFileOpen() 
{
	CFileDialog fileDlg(TRUE);
	CString filename;
	if(fileDlg.DoModal()==IDOK)
	{
		filename=fileDlg.GetFileName();
		if(m_pGraph!=NULL)
		{
			delete m_pGraph;
			m_pGraph=NULL;
		}
		m_pGraph=new CDSGraph(m_hWnd);
		m_pGraph->openFile(filename);
		
		//make sure that the initial size of videowindow is correct
		CRect rect;
		GetClientRect(&rect);
		m_pGraph->resizeVideoWindow(0,0,rect.Width(),rect.Height());
	}
	else
		return;
}

void CChildView::OnFileProperties() 
{
	if(m_pGraph!=NULL)
	{
		m_pGraph->showDIProperties();
	}
}

void CChildView::OnControlsPlay() 
{
	if(m_pGraph!=NULL)
	{
		m_pGraph->play();
	}
	
}

void CChildView::OnControlsPause() 
{
	if(m_pGraph!=NULL)
	{
		m_pGraph->pause();
	}
	
}

void CChildView::OnControlsStop() 
{
	if(m_pGraph!=NULL)
	{
		m_pGraph->stop();
	}	
}

BOOL CChildView::OnEraseBkgnd(CDC* pDC) 
{
	if(m_pGraph==NULL)
		return CWnd ::OnEraseBkgnd(pDC);
	else
		return FALSE;
}

void CChildView::OnSize(UINT nType, int cx, int cy) 
{
	CWnd ::OnSize(nType, cx, cy);
	
	if(m_pGraph!=NULL)
	{
		m_pGraph->resizeVideoWindow(0,0,cx,cy);
	}
	
}

void CChildView::OnFileClose() 
{
	if(m_pGraph!=NULL)
	{
		delete m_pGraph;
		m_pGraph=NULL;
	}
	
}

void CChildView::OnSysColorChange() 
{
	CWnd ::OnSysColorChange();
	
	if(m_pGraph!=NULL)
		m_pGraph->forwardWndMsg(m_hWnd,WM_SYSCOLORCHANGE,0,0);
	
}

void CChildView::OnDisplayChange(WPARAM wParam, LPARAM lParam)
{
	if(m_pGraph!=NULL)
		m_pGraph->forwardWndMsg(m_hWnd,WM_DISPLAYCHANGE,wParam,lParam);
}
