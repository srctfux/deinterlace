/////////////////////////////////////////////////////////////////////////////
// $Id: DSRendAboutPage.cpp,v 1.1 2002-11-14 19:02:09 tobbej Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Torbj�rn Jansson.  All rights reserved.
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
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DSRendAboutPage.cpp Implementation of CDSRendAboutPage
 */

#include "stdafx.h"
#include "DSRend.h"
#include "DSRendAboutPage.h"

/////////////////////////////////////////////////////////////////////////////
// CDSRendAboutPage

HRESULT CDSRendAboutPage::Activate(HWND hWndParent,LPCRECT pRect,BOOL bModal)
{
	ATLTRACE(_T("%s(%d) : CDSRendAboutPage::Activate\n"),__FILE__,__LINE__);
	HRESULT hr=IPropertyPageImpl<CDSRendAboutPage>::Activate(hWndParent,pRect,bModal);

	if(SUCCEEDED(hr))
	{
		char filename[300];
		GetModuleFileName(_Module.GetModuleInstance(),filename,sizeof(filename));
		DWORD dwVerInfoSize=GetFileVersionInfoSize(filename,NULL);
		if(dwVerInfoSize!=0)
		{
			BYTE *VerInfoBuffer=NULL;
			
			PSTR ProdVersion=NULL;
			UINT ProdVersionLen=0;
			VerInfoBuffer=(BYTE*)malloc(dwVerInfoSize);
			GetFileVersionInfo(filename,0,dwVerInfoSize,VerInfoBuffer);
			
			if(VerQueryValue(VerInfoBuffer,"\\StringFileInfo\\040904B0\\ProductVersion",(void**)&ProdVersion,&ProdVersionLen))
			{
				SetDlgItemText(IDC_ABOUT_VERSION,ProdVersion);
			}
			SetDlgItemText(IDC_ABOUT_COMPILEDATE,__TIMESTAMP__);
			free(VerInfoBuffer);
		}
	}
	return hr;
}

HRESULT CDSRendAboutPage::Apply()
{
	ATLTRACE(_T("CDSRendAboutPage::Apply\n"));
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

/*LRESULT CDSRendAboutPage::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	WORD wWidth=LOWORD(lParam);
	WORD wHeight=HIWORD(lParam);
	
	HWND hWnd=GetDlgItem(IDC_ABOUT_LINE1);
	RECT ItemRect;
	::GetWindowRect(hWnd,&ItemRect);
	ScreenToClient(&ItemRect);
	::MoveWindow(hWnd,ItemRect.left,ItemRect.top,wWidth,ItemRect.bottom-ItemRect.top,TRUE);

	hWnd=GetDlgItem(IDC_ABOUT_LINE2);
	::GetWindowRect(hWnd,&ItemRect);
	ScreenToClient(&ItemRect);
	::MoveWindow(hWnd,ItemRect.left,ItemRect.top,wWidth,ItemRect.bottom-ItemRect.top,TRUE);

	hWnd=GetDlgItem(IDC_ABOUT_LINE3);
	::GetWindowRect(hWnd,&ItemRect);
	ScreenToClient(&ItemRect);
	::MoveWindow(hWnd,ItemRect.left,ItemRect.top,wWidth,ItemRect.bottom-ItemRect.top,TRUE);
	return 0;
}*/