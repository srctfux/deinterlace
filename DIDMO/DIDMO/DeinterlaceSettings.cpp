/////////////////////////////////////////////////////////////////////////////
// $Id: DeinterlaceSettings.cpp,v 1.1 2001-08-08 15:37:02 tobbej Exp $
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
// Revision 1.2  2001/08/06 17:47:19  tobbej
// fixed default button to update controls properly
// fixed displaying of first setting after the page has been activated
//
// Revision 1.1.1.1  2001/07/30 16:14:44  tobbej
// initial import of new dmo filter
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DeinterlaceDMO.h"
#include "DeinterlaceSettings.h"

/////////////////////////////////////////////////////////////////////////////
// CDeinterlaceSettings

LRESULT CDeinterlaceSettings::OnSelChangeList(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CComQIPtr<IDeinterlace> pDI(m_ppUnk[0]);
	
	long index=getSelectedSettingIndex();

	DISETTING setting;
	long value;

	if(SUCCEEDED(pDI->GetSetting(index,&setting)))
	{
		pDI->get_SettingValue(index,&value);

		switch(setting.Type)
		{
		case DISETTING::DISETTING_TYPE::YESNO:
		case DISETTING::DISETTING_TYPE::ONOFF:
			::ShowWindow(GetDlgItem(IDC_SETTINGS_DEFAULT),SW_SHOWNA);
			::ShowWindow(GetDlgItem(IDC_SETTINGS_CHECK),SW_SHOWNA);
			::ShowWindow(GetDlgItem(IDC_SETTINGS_EDIT),SW_HIDE);
			::ShowWindow(GetDlgItem(IDC_SETTINGS_SLIDER),SW_HIDE);
			::ShowWindow(GetDlgItem(IDC_SETTINGS_SPIN),SW_HIDE);
			::ShowWindow(GetDlgItem(IDC_SETTINGS_COMBO),SW_HIDE);
			break;

		case DISETTING::DISETTING_TYPE::SLIDER:
			::ShowWindow(GetDlgItem(IDC_SETTINGS_DEFAULT),SW_SHOWNA);
			::ShowWindow(GetDlgItem(IDC_SETTINGS_CHECK),SW_HIDE);
			::ShowWindow(GetDlgItem(IDC_SETTINGS_EDIT),SW_SHOWNA);

			if(setting.MaxValue <= UD_MAXVAL && setting.MinValue >= UD_MINVAL)
				::ShowWindow(GetDlgItem(IDC_SETTINGS_SLIDER),SW_SHOWNA);
			else
				::ShowWindow(GetDlgItem(IDC_SETTINGS_SLIDER),SW_HIDE);
			
			::ShowWindow(GetDlgItem(IDC_SETTINGS_SPIN),SW_SHOWNA);
			::ShowWindow(GetDlgItem(IDC_SETTINGS_COMBO),SW_HIDE);
			break;

		case DISETTING::DISETTING_TYPE::ITEMFROMLIST:
			::ShowWindow(GetDlgItem(IDC_SETTINGS_DEFAULT),SW_SHOWNA);
			::ShowWindow(GetDlgItem(IDC_SETTINGS_CHECK),SW_HIDE);
			::ShowWindow(GetDlgItem(IDC_SETTINGS_EDIT),SW_HIDE);
			::ShowWindow(GetDlgItem(IDC_SETTINGS_SLIDER),SW_HIDE);
			::ShowWindow(GetDlgItem(IDC_SETTINGS_SPIN),SW_HIDE);
			::ShowWindow(GetDlgItem(IDC_SETTINGS_COMBO),SW_SHOWNA);
			break;

		default:
			::ShowWindow(GetDlgItem(IDC_SETTINGS_DEFAULT),SW_HIDE);
			::ShowWindow(GetDlgItem(IDC_SETTINGS_CHECK),SW_HIDE);
			::ShowWindow(GetDlgItem(IDC_SETTINGS_EDIT),SW_HIDE);
			::ShowWindow(GetDlgItem(IDC_SETTINGS_SLIDER),SW_HIDE);
			::ShowWindow(GetDlgItem(IDC_SETTINGS_SPIN),SW_HIDE);
			::ShowWindow(GetDlgItem(IDC_SETTINGS_COMBO),SW_HIDE);
		}
		UpdateControls();
	}

	return 0;
}

LRESULT CDeinterlaceSettings::OnChangeEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CComQIPtr<IDeinterlace> pDI(m_ppUnk[0]);
	
	TCHAR buffer[50];
	long index=getSelectedSettingIndex();
	if(index==LB_ERR)
		return 0;
	::GetWindowText(hWndCtl,buffer,50);
	
	if(SUCCEEDED(pDI->put_SettingValue(index,atol(buffer))))
		UpdateControls();

	return 0;
}

LRESULT CDeinterlaceSettings::OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CComQIPtr<IDeinterlace> pDI(m_ppUnk[0]);
	long index=getSelectedSettingIndex();
	if(index==LB_ERR)
		return 0;
	HWND hSlider=GetDlgItem(IDC_SETTINGS_SLIDER);
	if(hSlider==NULL)
		return 0;
	
	long pos=SendMessage(hSlider,TBM_GETPOS,0,0);
	if(SUCCEEDED(pDI->put_SettingValue(index,pos)))
		UpdateControls();

	return 0;
}

LRESULT CDeinterlaceSettings::OnClickedCheck(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CComQIPtr<IDeinterlace> pDI(m_ppUnk[0]);
	long index=getSelectedSettingIndex();
	
	if(index==LB_ERR)
		return 0;
	
	if(SUCCEEDED(pDI->put_SettingValue(index,IsDlgButtonChecked(IDC_SETTINGS_CHECK))))
		UpdateControls();
	return 0;
}

LRESULT CDeinterlaceSettings::OnSettingsDefault(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CComQIPtr<IDeinterlace> pDI(m_ppUnk[0]);
	long index=getSelectedSettingIndex();
	DISETTING setting;

	if(index==LB_ERR)
		return 0;
	
	if(SUCCEEDED(pDI->GetSetting(index,&setting)))
	{
		if(SUCCEEDED(pDI->put_SettingValue(index,setting.Default)))
			UpdateControls();
	}
	
	return 0;
}

void CDeinterlaceSettings::UpdateControls()
{
	static bool bInUpdate = false;
	if(bInUpdate)
		return;
	bInUpdate=true;

	CComQIPtr<IDeinterlace> pDI(m_ppUnk[0]);
	long index=getSelectedSettingIndex();
	long value;
	DISETTING setting;

	if(FAILED(pDI->GetSetting(index,&setting)))
		return;
	if(FAILED(pDI->get_SettingValue(index,&value)))
		return;

	HWND hCheck=GetDlgItem(IDC_SETTINGS_CHECK);
	HWND hEdit=GetDlgItem(IDC_SETTINGS_EDIT);
	HWND hSpin=GetDlgItem(IDC_SETTINGS_SPIN);
	HWND hSlider=GetDlgItem(IDC_SETTINGS_SLIDER);
	HWND hCombo=GetDlgItem(IDC_SETTINGS_COMBO);

	if(::GetWindowLong(hSpin,GWL_STYLE) & WS_VISIBLE)
	{
		SendMessage(hSpin,UDM_SETRANGE32,setting.MinValue,setting.MaxValue);
	}

	if(::GetWindowLong(hEdit,GWL_STYLE) & WS_VISIBLE)
	{
		TCHAR buffer[50];
		wsprintf(buffer,_T("%ld"),value);
		::SetWindowText(hEdit,buffer);
	}

	if(::GetWindowLong(hCheck,GWL_STYLE) & WS_VISIBLE)
	{
		CheckDlgButton(IDC_SETTINGS_CHECK,value);
		::SetWindowText(hCheck,reinterpret_cast<LPTSTR>(setting.szDisplayName));
	}
	
	if(::GetWindowLong(hSlider,GWL_STYLE) & WS_VISIBLE)
	{
	    SendMessage(hSlider,TBM_CLEARTICS,(WPARAM)TRUE,(LPARAM)0);
		SendMessage(hSlider,TBM_SETRANGEMAX,(WPARAM)TRUE,(LPARAM)setting.MaxValue);
		SendMessage(hSlider,TBM_SETRANGEMIN,(WPARAM)TRUE,(LPARAM)setting.MinValue);		
		SendMessage(hSlider,TBM_SETPAGESIZE,(WPARAM)0,(LPARAM)setting.StepValue);
		SendMessage(hSlider,TBM_SETLINESIZE,(WPARAM)0,(LPARAM)setting.StepValue);
		SendMessage(hSlider,TBM_SETTIC,(WPARAM)0,(LPARAM)setting.Default);
		SendMessage(hSlider,TBM_SETPOS,(WPARAM)TRUE,(LPARAM)value);
	}

	if(::GetWindowLong(hCombo,GWL_STYLE) & WS_VISIBLE)
	{
		SendMessage(hCombo,CB_RESETCONTENT,0,0);
	}
	bInUpdate=false;
}

long CDeinterlaceSettings::getSelectedSettingIndex()
{
	HWND hList=GetDlgItem(IDC_SETTINGS_LIST);
	LPARAM cursel=SendMessage(hList,LB_GETCURSEL,0,0);
	if(cursel==LB_ERR)
		return LB_ERR;

	LPARAM index=SendMessage(hList,LB_GETITEMDATA,cursel,0);
	if(index==LB_ERR)
		return LB_ERR;
	else
		return index;
	
}
