/////////////////////////////////////////////////////////////////////////////
// $Id: DSRendQualityPage.cpp,v 1.1.1.1 2002-02-03 10:52:53 tobbej Exp $
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
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DSRendQualityPage.cpp Implementation of CDSRendQualityPage
 */
#include "StdAfx.h"
#include "DSRend.h"
#include "DSRendQualityPage.h"

#define IDT_TIMER 1

HRESULT CDSRendQualityPage::FinalConstruct()
{
	ATLTRACE(_T("%s(%d) : CDSRendQualityPage::FinalConstruct\n"),__FILE__,__LINE__);
	return S_OK;
}

HRESULT CDSRendQualityPage::FinalRelease()
{
	ATLTRACE(_T("%s(%d) : CDSRendQualityPage::FinalRelease\n"),__FILE__,__LINE__);
	return S_OK;
}

HRESULT CDSRendQualityPage::Activate(HWND hWndParent,LPCRECT pRect,BOOL bModal)
{
	ATLTRACE(_T("%s(%d) : CDSRendQualityPage::Activate\n"),__FILE__,__LINE__);
	HRESULT hr=IPropertyPageImpl<CDSRendQualityPage>::Activate(hWndParent,pRect,bModal);

	if(m_hWnd!=NULL)
	{
		SetTimer(IDT_TIMER,500);
		updateDialog();
	}

	return hr;
}

HRESULT CDSRendQualityPage::Deactivate()
{
	ATLTRACE(_T("%s(%d) : CDSRendQualityPage::Deactivate\n"),__FILE__,__LINE__);
	HRESULT hr=IPropertyPageImpl<CDSRendQualityPage>::Deactivate();
	if(m_hWnd!=NULL)
	{
		KillTimer(IDT_TIMER);
	}
	return hr;
}

HRESULT CDSRendQualityPage::Apply()
{
	ATLTRACE(_T("%s(%d) : CDSRendQualityPage::Apply\n"),__FILE__,__LINE__);
	for (UINT i = 0; i < m_nObjects; i++)
	{
		// Do something interesting here
		// ICircCtl* pCirc;
		// m_ppUnk[i]->QueryInterface(IID_ICircCtl, (void**)&pCirc);
		// pCirc->put_Caption(CComBSTR("something special"));
		// pCirc->Release();
	}
	m_bDirty = FALSE;
	return S_OK;
}

LRESULT CDSRendQualityPage::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ATLTRACE(_T("%s(%d) : CDSRendQualityPage::OnTimer\n"),__FILE__,__LINE__);
	if(wParam==IDT_TIMER)
	{
		updateDialog();
		return TRUE;
	}
	return 0;
}

LRESULT CDSRendQualityPage::OnClickedReset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ATLTRACE(_T("%s(%d) : CDSRendQualityPage::OnClickedReset\n"),__FILE__,__LINE__);
	if(m_pQuality!=NULL)
	{
		m_pQuality->get_FramesDrawn(&m_qualDrawnStart);
		m_pQuality->get_FramesDroppedInRenderer(&m_qualDroppedStart);
	}
	if(m_pUpstreamDF!=NULL)
	{
		m_pUpstreamDF->GetNumDropped(&m_UpStreamDroppedStart);
		m_pUpstreamDF->GetNumNotDropped(&m_UpStreamDrawnStart);
	}
	return 0;
}

LRESULT CDSRendQualityPage::OnClickedRestore(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ATLTRACE(_T("%s(%d) : CDSRendQualityPage::OnClickedRestore\n"),__FILE__,__LINE__);
	m_qualDrawnStart=0;
	m_qualDroppedStart=0;
	m_UpStreamDrawnStart=0;
	m_UpStreamDroppedStart=0;
	return 0;
}

HRESULT CDSRendQualityPage::SetObjects( ULONG nObjects, IUnknown** ppUnk )
{
	ATLTRACE(_T("%s(%d) : CDSRendQualityPage::SetObjects\n"),__FILE__,__LINE__);
	if(ppUnk==NULL)
		return E_POINTER;
	if(nObjects!=1)
		return E_FAIL;

	HRESULT hr=(*ppUnk)->QueryInterface(IID_IQualProp,(void **)&m_pQuality);
	if(FAILED(hr))
	{
		return E_NOINTERFACE;
	}
	return IPropertyPageImpl<CDSRendQualityPage>::SetObjects(nObjects,ppUnk);
}

void CDSRendQualityPage::findUpstreamFilter()
{
	ATLTRACE(_T("%s(%d) : CDSRendQualityPage::findUpstreamFilter\n"),__FILE__,__LINE__);
	ATLASSERT(m_pQuality!=NULL);
	CComPtr<IBaseFilter> pRend;

	//find the filter we are attatched to
	HRESULT hr=m_pQuality->QueryInterface(IID_IBaseFilter,(void **)&pRend);
	if(FAILED(hr))
	{
		return;
	}
	
	//get the pin enumerator
	CComPtr<IEnumPins> pPins;
	hr=pRend->EnumPins(&pPins);
	if(FAILED(hr))
	{
		return;
	}
	
	//find the first pin
	CComPtr<IPin> pInPin;
	ULONG cFetched=0;
	hr=pPins->Next(1,&pInPin,NULL);
	if(FAILED(hr) || pInPin==NULL)
	{
		return;
	}
	
	//find the upstream pin that pInPin is connected to
	CComPtr<IPin> pOutPin;
	hr=pInPin->ConnectedTo(&pOutPin);
	if(FAILED(hr))
	{
		return;
	}

	//and finaly find the IAMDroppedFrames interface that we were looking for
	PIN_INFO pinInfo;
	hr=pOutPin->QueryPinInfo(&pinInfo);
	if(FAILED(hr))
	{
		return;
	}
	if(pinInfo.dir==PINDIR_OUTPUT && pinInfo.pFilter!=NULL)
	{
		m_pUpstreamDF=NULL;
		hr=pinInfo.pFilter->QueryInterface(IID_IAMDroppedFrames,(void **)&m_pUpstreamDF);
		
		//hmm, this is a bit strange...
		//acording to the documentation the IAMDroppedFrames shoud be on the filter itself, not the pin
		if(FAILED(hr))
		{
			hr=pOutPin->QueryInterface(IID_IAMDroppedFrames,(void **)&m_pUpstreamDF);
		}
		pinInfo.pFilter->Release();
		pinInfo.pFilter=NULL;
	}
}

void CDSRendQualityPage::updateDialog()
{
	ATLTRACE(_T("%s(%d) : CDSRendQualityPage::updateDialog\n"),__FILE__,__LINE__);
	int frameRate=-1;
	int syncOffset=-1;
	int devSyncOffset=-1;
	int framesDrawn=-1;
	int framesDropped=-1;
	int jitter=-1;
	TCHAR buffer[50];
	
	ATLASSERT(m_pQuality!=NULL);

	HRESULT hr=m_pQuality->get_AvgFrameRate(&frameRate);
	hr=m_pQuality->get_AvgSyncOffset(&syncOffset);
	hr=m_pQuality->get_DevSyncOffset(&devSyncOffset);
	hr=m_pQuality->get_FramesDrawn(&framesDrawn);
	hr=m_pQuality->get_FramesDroppedInRenderer(&framesDropped);
	hr=m_pQuality->get_Jitter(&jitter);

	wsprintf(buffer,TEXT("%d.%02d"),frameRate/100,frameRate%100);
	SetDlgItemText(IDC_AVG_FRAME_RATE,buffer);

	wsprintf(buffer,TEXT("%d"),syncOffset);
	SetDlgItemText(IDC_AVG_SYNC_OFFSET,buffer);
	
	wsprintf(buffer,TEXT("%d"),devSyncOffset);
	SetDlgItemText(IDC_STDDEV_SYNC_OFFSET,buffer);
	
	wsprintf(buffer,TEXT("%d"),framesDrawn-m_qualDrawnStart);
	SetDlgItemText(IDC_FRAMES_DRAWN,buffer);
	
	wsprintf(buffer,TEXT("%d"),framesDropped-m_qualDroppedStart);
	SetDlgItemText(IDC_FRAMES_DROPPED,buffer);
	
	wsprintf(buffer,TEXT("%d"),jitter);
	SetDlgItemText(IDC_JITTER,buffer);

	if(m_pUpstreamDF==NULL)
	{
		findUpstreamFilter();
	}
	
	if(m_pUpstreamDF!=NULL)
	{
		HWND hWnd;
		hWnd=GetDlgItem(IDC_UPSTREAM_GRP);
		::EnableWindow(hWnd,TRUE);
		hWnd=GetDlgItem(IDC_UPSTREAM_LBL1);
		::EnableWindow(hWnd,TRUE);
		hWnd=GetDlgItem(IDC_UPSTREAM_LBL2);
		::EnableWindow(hWnd,TRUE);
		hWnd=GetDlgItem(IDC_UPSTREAM_LBL3);
		::EnableWindow(hWnd,TRUE);
		hWnd=GetDlgItem(IDC_UPSTREAM_AVGSIZE);
		::EnableWindow(hWnd,TRUE);
		hWnd=GetDlgItem(IDC_UPSTREAM_DROPPED);
		::EnableWindow(hWnd,TRUE);
		hWnd=GetDlgItem(IDC_UPSTREAM_NOTDROPPED);
		::EnableWindow(hWnd,TRUE);

		long tmp;
		HRESULT hr=m_pUpstreamDF->GetNumDropped(&tmp);
		if(FAILED(hr))
		{
			SetDlgItemText(IDC_UPSTREAM_DROPPED,_T("?"));
		}
		else
		{
			SetDlgItemInt(IDC_UPSTREAM_DROPPED,tmp-m_UpStreamDroppedStart,FALSE);
		}
		
		hr=m_pUpstreamDF->GetNumNotDropped(&tmp);
		if(FAILED(hr))
		{
			SetDlgItemText(IDC_UPSTREAM_NOTDROPPED,_T("?"));
		}
		else
		{
			SetDlgItemInt(IDC_UPSTREAM_NOTDROPPED,tmp-m_UpStreamDrawnStart,FALSE);
		}
		hr=m_pUpstreamDF->GetAverageFrameSize(&tmp);
		if(FAILED(hr))
		{
			SetDlgItemText(IDC_UPSTREAM_AVGSIZE,_T("?"));
		}
		else
		{
			SetDlgItemInt(IDC_UPSTREAM_AVGSIZE,tmp,FALSE);
		}

	}
	else
	{
		HWND hWnd;
		hWnd=GetDlgItem(IDC_UPSTREAM_GRP);
		::EnableWindow(hWnd,FALSE);
		hWnd=GetDlgItem(IDC_UPSTREAM_LBL1);
		::EnableWindow(hWnd,FALSE);
		hWnd=GetDlgItem(IDC_UPSTREAM_LBL2);
		::EnableWindow(hWnd,FALSE);
		hWnd=GetDlgItem(IDC_UPSTREAM_LBL3);
		::EnableWindow(hWnd,FALSE);
		hWnd=GetDlgItem(IDC_UPSTREAM_AVGSIZE);
		::EnableWindow(hWnd,FALSE);
		hWnd=GetDlgItem(IDC_UPSTREAM_DROPPED);
		::EnableWindow(hWnd,FALSE);
		hWnd=GetDlgItem(IDC_UPSTREAM_NOTDROPPED);
		::EnableWindow(hWnd,FALSE);
	}
}
