/////////////////////////////////////////////////////////////////////////////
// $Id: DeinterlaceProperties.h,v 1.2 2001-09-19 17:50:07 tobbej Exp $
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
// Revision 1.1  2001/08/08 15:37:02  tobbej
// moved dmo filter to new directory
//
// Revision 1.2  2001/08/07 20:22:35  tobbej
// added new button in propertypage to show plugin ui
// fixed Activate function
//
// Revision 1.1.1.1  2001/07/30 16:14:44  tobbej
// initial import of new dmo filter
//
//
//////////////////////////////////////////////////////////////////////////////


#ifndef __DEINTERLACEPROPERTIES_H_
#define __DEINTERLACEPROPERTIES_H_

#include "resource.h"       // main symbols

EXTERN_C const CLSID CLSID_DeinterlaceProperties;

/////////////////////////////////////////////////////////////////////////////
// CDeinterlaceProperties
class ATL_NO_VTABLE CDeinterlaceProperties :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDeinterlaceProperties, &CLSID_DeinterlaceProperties>,
	public IPropertyPageImpl<CDeinterlaceProperties>,
	public CDialogImpl<CDeinterlaceProperties>
{
public:
	CDeinterlaceProperties() 
	{
		m_dwTitleID = IDS_TITLEDeinterlaceProperties;
		m_dwHelpFileID = IDS_HELPFILEDeinterlaceProperties;
		m_dwDocStringID = IDS_DOCSTRINGDeinterlaceProperties;
	}

	enum {IDD = IDD_DEINTERLACEPROPERTIES};

DECLARE_REGISTRY_RESOURCEID(IDR_DEINTERLACEPROPERTIES)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDeinterlaceProperties) 
	COM_INTERFACE_ENTRY(IPropertyPage)
END_COM_MAP()

BEGIN_MSG_MAP(CDeinterlaceProperties)
	CHAIN_MSG_MAP(IPropertyPageImpl<CDeinterlaceProperties>)
	COMMAND_HANDLER(IDC_PLUGIN_LOAD, BN_CLICKED, OnPluginLoad)
	COMMAND_HANDLER(IDC_PLUGIN_UNLOAD, BN_CLICKED, OnPluginUnload)
	COMMAND_HANDLER(IDC_PLUGIN_BROWSE, BN_CLICKED, OnPluginBrowse)
	COMMAND_HANDLER(IDC_PLUGIN_SHOWUI, BN_CLICKED, OnPluginShowUI)
	COMMAND_HANDLER(IDC_PLUGIN_MODE, CBN_SELENDOK, OnPluginModeChange)
	COMMAND_HANDLER(IDC_PLUGIN_MODE, CBN_SELCHANGE, OnPluginModeUpdLasMode)
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	STDMETHOD(SetObjects)(ULONG nObjects, IUnknown** ppUnk)
	{
		ATLTRACE("%s(%d) : CDeinterlaceProperties::SetObjects\n",__FILE__,__LINE__);
		if(nObjects==1)
		{
			CComQIPtr<IDeinterlace> pDoc(ppUnk[0]);
			if(pDoc!=NULL)
				return IPropertyPageImpl<CDeinterlaceProperties>::SetObjects(nObjects, ppUnk);
		}
		return E_INVALIDARG;
	}
	
	STDMETHOD(Apply)(void)
	{
		ATLTRACE("%s(%d) : CDeinterlaceProperties::Apply\n",__FILE__,__LINE__);
		
		if (!m_ppUnk)
			return E_UNEXPECTED;
		
		/*if(!m_bDirty)
			return E_UNEXPECTED;		
		m_bDirty = FALSE;*/

		return S_OK;
	}
	STDMETHOD(Activate)( HWND hWndParent, LPCRECT pRect, BOOL bModal )
	{
		HRESULT hr=IPropertyPageImpl<CDeinterlaceProperties>::Activate(hWndParent,pRect,bModal);
		UpdateUI();
		return hr;
	}
	
private:
	void UpdateUI();
	LRESULT OnPluginLoad(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPluginUnload(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPluginBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPluginShowUI(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPluginModeChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPluginModeUpdLasMode(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT m_lastMode;
};

#endif //__DEINTERLACEPROPERTIES_H_
