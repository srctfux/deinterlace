/////////////////////////////////////////////////////////////////////////////
// $Id: TreeSettingsPage.cpp,v 1.2 2002-05-09 17:20:15 tobbej Exp $
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
//
// $Log: not supported by cvs2svn $
// Revision 1.1  2002/04/24 19:04:01  tobbej
// new treebased settings dialog
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file TreeSettingsPage.cpp implementation file for CTreeSettingsPage
 */

#include "stdafx.h"
#include "TreeSettingsPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTreeSettingsPage dialog


CTreeSettingsPage::CTreeSettingsPage(CString name,UINT nIDTemplate)
	: CDialog(nIDTemplate, NULL),m_dlgID(nIDTemplate),m_name(name)
{
	//{{AFX_DATA_INIT(CTreeSettingsPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTreeSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTreeSettingsPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTreeSettingsPage, CDialog)
	//{{AFX_MSG_MAP(CTreeSettingsPage)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTreeSettingsPage message handlers

BOOL CTreeSettingsPage::PreTranslateMessage(MSG* pMsg) 
{
	//dont allow escape key to be processed by the page
	//the escape key will normaly close the dialog
	if((pMsg->message==WM_KEYDOWN) && (pMsg->wParam==VK_ESCAPE))
	{
		return TRUE;
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CTreeSettingsPage::GetMinSize(int &width,int &height)
{
	width=0;
	height=0;
}

void CTreeSettingsPage::OnOK()
{
	EndDialog(IDOK);
}

void CTreeSettingsPage::OnCancel()
{
	EndDialog(IDCANCEL);
}

bool CTreeSettingsPage::OnKillActive()
{
	return true;
}

bool CTreeSettingsPage::OnSetActive()
{
	return true;
}

bool CTreeSettingsPage::OnQueryCancel()
{
	return true;
}
