/////////////////////////////////////////////////////////////////////////////
// $Id: DeinterlaceProperties.cpp,v 1.1 2001-08-08 15:37:02 tobbej Exp $
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
// Revision 1.3  2001/08/07 20:22:35  tobbej
// added new button in propertypage to show plugin ui
// fixed Activate function
//
// Revision 1.2  2001/08/07 17:43:52  tobbej
// fixed NULL pointer crash in property page
//
// Revision 1.1.1.1  2001/07/30 16:14:44  tobbej
// initial import of new dmo filter
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DeinterlaceDMO.h"
#include "DeinterlaceProperties.h"

/////////////////////////////////////////////////////////////////////////////
// CDeinterlaceProperties


void CDeinterlaceProperties::UpdateUI()
{
	ATLTRACE("%s(%d) : UpdateUI\n",__FILE__,__LINE__);
	CComQIPtr<IDeinterlace,&IID_IDeinterlace> pDI(m_ppUnk[0]);
	
	if(SUCCEEDED(pDI->IsPluginLoaded()))
	{
		SetDlgItemText(IDC_PLUGIN_STATUS,_T("Loaded"));
	}
	else
	{
		SetDlgItemText(IDC_PLUGIN_STATUS,_T("Not loaded"));
	}
	
	unsigned char *name;
	if(SUCCEEDED(pDI->GetPluginName(&name)))
	{
		SetDlgItemText(IDC_PLUGIN_NAME,(char *)name);
	}
	else
	{
		SetDlgItemText(IDC_PLUGIN_NAME,"");
	}
	
	BOOL hasUI=FALSE;
	pDI->PluginHasUI(&hasUI);
	::EnableWindow(GetDlgItem(IDC_PLUGIN_SHOWUI),hasUI);
	
}

LRESULT CDeinterlaceProperties::OnPluginUnload(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ATLTRACE("%s(%d) : OnPluginUnload\n",__FILE__,__LINE__);
	USES_CONVERSION;
	CComQIPtr<IDeinterlace,&IID_IDeinterlace> pDI(m_ppUnk[0]);

	if(FAILED(pDI->UnloadPlugin()))
	{
		CComPtr<IErrorInfo> pError;
		CComBSTR strError;
		GetErrorInfo(0, &pError);
		pError->GetDescription(&strError);
		MessageBox(OLE2T(strError), _T("Error"), MB_ICONEXCLAMATION);
		return 0;
	}
	
	UpdateUI();
	return 0;
}

LRESULT CDeinterlaceProperties::OnPluginLoad(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ATLTRACE("%s(%d) : OnPluginLoad\n",__FILE__,__LINE__);
	USES_CONVERSION;
	HRESULT hr;
	CComQIPtr<IDeinterlace,&IID_IDeinterlace> pDI(m_ppUnk[0]);

	TCHAR strFileName[255];
	GetDlgItemText(IDC_PLUGIN_FILENAME,strFileName,255);
	hr=pDI->LoadPlugin(strFileName);
	if(FAILED(hr))
	{
		CComPtr<IErrorInfo> pError;
		CComBSTR strError;
		GetErrorInfo(0, &pError);
		if(pError!=NULL)
		{
			pError->GetDescription(&strError);
			MessageBox(OLE2T(strError), _T("Error"), MB_ICONEXCLAMATION);
		}
		else
		{
			MessageBox("Unknown error","Error",MB_ICONEXCLAMATION);
		}
		return 0;
	}

	UpdateUI();
	return 0;
}

LRESULT CDeinterlaceProperties::OnPluginBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	OPENFILENAME ofn;		// common dialog box structure
	TCHAR szFile[260];		// buffer for file name
	
	szFile[0]=0;
	GetDlgItemText(IDC_PLUGIN_FILENAME,szFile,260);

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = _T("Plugins (*.dll)\0*.dll\0All (*.*)\0*.*\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle=_T("Select plugin");
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box.
	if(GetOpenFileName(&ofn)==TRUE)
	{
		SetDlgItemText(IDC_PLUGIN_FILENAME,ofn.lpstrFile);
		BOOL dummy;
		OnPluginLoad(0,0,NULL,dummy);
	}
	
	return 0;
}

LRESULT CDeinterlaceProperties::OnPluginShowUI(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	ATLTRACE("%s(%d) : OnPluginShowUI\n",__FILE__,__LINE__);
	CComQIPtr<IDeinterlace,&IID_IDeinterlace> pDI(m_ppUnk[0]);
	
	if(FAILED(pDI->ShowPluginUI((long*)m_hWnd)))
	{
		MessageBox(_T("Failed to show plugin UI"),_T("Error"));
	}

	return 0;
}