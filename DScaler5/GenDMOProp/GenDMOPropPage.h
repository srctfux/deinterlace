///////////////////////////////////////////////////////////////////////////////
// $Id: GenDMOPropPage.h,v 1.2 2003-05-01 12:34:41 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
// GenDMOProp.dll - Generic DirectShow property page using IMediaParams
// Copyright (c) 2003 John Adcock
///////////////////////////////////////////////////////////////////////////////
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
///////////////////////////////////////////////////////////////////////////////

#ifndef __GENDMOPROPPAGE_H_
#define __GENDMOPROPPAGE_H_

#include "resource.h"       // main symbols

EXTERN_C const CLSID CLSID_GenDMOPropPage;

/////////////////////////////////////////////////////////////////////////////
// CGenDMOPropPage
class ATL_NO_VTABLE CGenDMOPropPage :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CGenDMOPropPage, &CLSID_GenDMOPropPage>,
	public IPropertyPageImpl<CGenDMOPropPage>,
	public CDialogImpl<CGenDMOPropPage>
{
public:
	CGenDMOPropPage();

	enum {IDD = IDD_GENDMOPROPPAGE};

DECLARE_REGISTRY_RESOURCEID(IDR_GENDMOPROPPAGE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CGenDMOPropPage) 
	COM_INTERFACE_ENTRY(IPropertyPage)
END_COM_MAP()

BEGIN_MSG_MAP(CGenDMOPropPage)
	CHAIN_MSG_MAP(IPropertyPageImpl<CGenDMOPropPage>)
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	STDMETHOD(Apply)(void);
    STDMETHOD(SetObjects)(ULONG cObjects,IUnknown **ppUnk);

private:
    CComQIPtr<IMediaParamInfo> m_MediaParamInfo;
    CComQIPtr<IMediaParams> m_MediaParams;
    long m_CountParams;
    float* m_Values;
};

#endif //__GENDMOPROPPAGE_H_
