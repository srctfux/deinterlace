/////////////////////////////////////////////////////////////////////////////
// $Id: DeinterlaceSettings.h,v 1.1 2001-08-08 15:37:02 tobbej Exp $
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
// Revision 1.2  2001/08/07 20:22:35  tobbej
// added new button in propertypage to show plugin ui
// fixed Activate function
//
// Revision 1.1.1.1  2001/07/30 16:14:44  tobbej
// initial import of new dmo filter
//
//
//////////////////////////////////////////////////////////////////////////////


#ifndef __DEINTERLACESETTINGS_H_
#define __DEINTERLACESETTINGS_H_

#include "resource.h"       // main symbols

#include <commctrl.h>		//UDM_SETRANGE32

EXTERN_C const CLSID CLSID_DeinterlaceSettings;

/////////////////////////////////////////////////////////////////////////////
// CDeinterlaceSettings
class ATL_NO_VTABLE CDeinterlaceSettings :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CDeinterlaceSettings, &CLSID_DeinterlaceSettings>,
	public IPropertyPageImpl<CDeinterlaceSettings>,
	public CDialogImpl<CDeinterlaceSettings>
{
public:
	CDeinterlaceSettings() 
	{
		m_dwTitleID = IDS_TITLEDeinterlaceSettings;
		m_dwHelpFileID = IDS_HELPFILEDeinterlaceSettings;
		m_dwDocStringID = IDS_DOCSTRINGDeinterlaceSettings;
	}

	enum {IDD = IDD_DEINTERLACESETTINGS};

DECLARE_REGISTRY_RESOURCEID(IDR_DEINTERLACESETTINGS)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDeinterlaceSettings) 
	COM_INTERFACE_ENTRY(IPropertyPage)
END_COM_MAP()

BEGIN_MSG_MAP(CDeinterlaceSettings)
	CHAIN_MSG_MAP(IPropertyPageImpl<CDeinterlaceSettings>)
	COMMAND_HANDLER(IDC_SETTINGS_LIST, LBN_SELCHANGE, OnSelChangeList)
	COMMAND_HANDLER(IDC_SETTINGS_EDIT, EN_CHANGE, OnChangeEdit)
	MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
	COMMAND_HANDLER(IDC_SETTINGS_DEFAULT, BN_CLICKED, OnSettingsDefault)
	COMMAND_HANDLER(IDC_SETTINGS_CHECK, BN_CLICKED, OnClickedCheck)
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	STDMETHOD(SetObjects)(ULONG nObjects, IUnknown** ppUnk)
	{
		ATLTRACE("%s(%d) : CDeinterlaceSettings::SetObjects\n",__FILE__,__LINE__);
		
		//make sure we only get one object
		if(nObjects==1)
		{
			CComQIPtr<IDeinterlace> pDoc(ppUnk[0]);
			if(pDoc!=NULL)
				return IPropertyPageImpl<CDeinterlaceSettings>::SetObjects(nObjects, ppUnk);
		}
		return E_INVALIDARG;
	}
	
	STDMETHOD(Apply)(void)
	{
		ATLTRACE(_T("%s(%d) : CDeinterlaceSettings::Apply\n"),__FILE__,__LINE__);
		if (!m_ppUnk)
			return E_UNEXPECTED;
		/*if(!m_bDirty)
			return E_UNEXPECTED;
		m_bDirty = FALSE;*/

		return S_OK;
	}
	
	STDMETHOD(Activate)( HWND hWndParent, LPCRECT pRect, BOOL bModal )
	{
		CComQIPtr<IDeinterlace> pDI(m_ppUnk[0]);
		HRESULT hr=IPropertyPageImpl<CDeinterlaceSettings>::Activate(hWndParent,pRect,bModal);
		
		long count;
		if(SUCCEEDED(pDI->GetSettingCount(&count)))
		{
			HWND hList=GetDlgItem(IDC_SETTINGS_LIST);
			SendMessage(hList,LB_RESETCONTENT,0,0);
			for(int i=0;i<count;i++)
			{
				DISETTING setting;
				if(SUCCEEDED(pDI->GetSetting(i,&setting)))
				{
					LRESULT index=SendMessage(hList,LB_ADDSTRING,0,(LPARAM)setting.szDisplayName);
					SendMessage(hList,LB_SETITEMDATA,index,i);
				}
			}
			//select first item
			if(SendMessage(hList,LB_SETCURSEL,0,0)!=LB_ERR)
			{
				BOOL dummy;
				OnSelChangeList(0,0,hList,dummy);
			}
		}
		return hr;
	}
private:
	long getSelectedSettingIndex();
	void UpdateControls();
	LRESULT OnSelChangeList(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnChangeEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSettingsDefault(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClickedCheck(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};

#endif //__DEINTERLACESETTINGS_H_
