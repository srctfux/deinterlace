/////////////////////////////////////////////////////////////////////////////
// $Id: TreeSettingsDlg.cpp,v 1.1 2002-04-24 19:04:00 tobbej Exp $
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
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file TreeSettingsDlg.cpp implementation file for tree settings dialog
 */

#include "stdafx.h"
#include "TreeSettingsDlg.h"

#include <afxpriv.h>	//WM_COMMANDHELP

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CTreeSettingsDlg::CTreeSettingsDlg(CString caption,CWnd* pParent /*=NULL*/)
	: CDialog(CTreeSettingsDlg::IDD, pParent),
	m_settingsDlgCaption(caption),
	m_iCurrentPage(-1),
	m_iStartPage(-1)
{
	//{{AFX_DATA_INIT(CTreeSettingsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTreeSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTreeSettingsDlg)
	DDX_Control(pDX, IDC_TREESETTINGS_TREE, m_tree);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTreeSettingsDlg, CDialog)
	//{{AFX_MSG_MAP(CTreeSettingsDlg)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREESETTINGS_TREE, OnSelchangedTree)
	ON_BN_CLICKED(IDC_HELPBTN, OnHelpBtn)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTreeSettingsDlg message handlers

void CTreeSettingsDlg::OnOK() 
{
	for(int i=0;i<m_pages.size();i++)
	{
		CTreeSettingsPage *pPage=m_pages[i].m_pPage;
		//check if the page has been created
		if(::IsWindow(pPage->m_hWnd))
		{
			if(!pPage->UpdateData())
			{
				//show the page that failed
				if(m_iCurrentPage!=i)
				{
					ShowPage(i);
				}
				return;
			}
		}
	}
	for(i=0;i<m_pages.size();i++)
	{
		CTreeSettingsPage *pPage=m_pages[i].m_pPage;
		//check if the page has been created
		if(::IsWindow(pPage->m_hWnd))
		{
			pPage->OnOK();
		}
	}
	
	CDialog::OnOK();
}

void CTreeSettingsDlg::OnCancel() 
{
	for(int i=0;i<m_pages.size();i++)
	{
		CTreeSettingsPage *pPage=m_pages[i].m_pPage;
		if(!pPage->OnQueryCancel())
		{
			//maybe also call ShowPage(i) so the page that aborted the cancel is shown
			return;
		}
	}

	for(i=0;i<m_pages.size();i++)
	{
		CTreeSettingsPage *pPage=m_pages[i].m_pPage;
		
		//call OnCancel only on pages that has been created (selected atleast once)
		if(::IsWindow(pPage->m_hWnd))
		{
			pPage->OnCancel();
		}
	}
	
	CDialog::OnCancel();
}

void CTreeSettingsDlg::OnSelchangedTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	*pResult = 0;
	int iPage=pNMTreeView->itemNew.lParam;
	if(m_iCurrentPage!=iPage)
	{
		if(!ShowPage(iPage))
		{
			*pResult=TRUE;
		}
	}
}

BOOL CTreeSettingsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetWindowText(m_settingsDlgCaption);

	for(int i=0;i<m_pages.size();i++)
	{
		HTREEITEM hParent=TVI_ROOT;
		if(m_pages[i].m_parent>=0)
		{

			if(m_pages[i].m_parent<m_pages.size())
			{
				hParent=m_pages[m_pages[i].m_parent].m_hTreeItem;
			}
		}
		HTREEITEM hNode=m_tree.InsertItem(m_pages[i].m_pPage->GetName(),0,0,hParent);
		m_tree.SetItemState(hNode,TVIS_EXPANDED,TVIS_EXPANDED);
		m_tree.SetItemData(hNode,i);
		m_pages[i].m_hTreeItem=hNode;
	}

	//show the first page
	if(m_iStartPage>=0 && m_iStartPage<m_pages.size())
	{
		ShowPage(m_iStartPage);
	}
	else
	{
		ShowPage(0);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

int CTreeSettingsDlg::AddPage(CTreeSettingsPage *pPage,int parent)
{
	CPageInfo newPage;
	newPage.m_pPage=pPage;
	newPage.m_parent=parent;
	newPage.m_hTreeItem=NULL;
	m_pages.push_back(newPage);
	return m_pages.size()-1;
}

bool CTreeSettingsDlg::ShowPage(int iPage)
{
	if(iPage<0 || iPage>=m_pages.size())
	{
		return false;
	}
	CRect pageSize;
	CWnd *pPageFrame=GetDlgItem(IDC_TREESETTINGS_PAGEFRAME);
	ASSERT(pPageFrame!=NULL);
	pPageFrame->GetWindowRect(&pageSize);
	ScreenToClient(&pageSize);

	CTreeSettingsPage *pNewPage=m_pages[iPage].m_pPage;
	//create the new page if nessesary
	if(!::IsWindow(pNewPage->m_hWnd))
	{
		if(!pNewPage->Create(pNewPage->GetDlgID(),this))
		{
			//create failed
			return false;
		}
		pNewPage->MoveWindow(pageSize);
	}
	//can the new page be activated?
	if(!pNewPage->OnSetActive())
	{
		return false;
	}
	
	//deactivate current page
	CTreeSettingsPage *pCurrentPage=NULL;
	if(m_iCurrentPage>=0 && m_iCurrentPage<m_pages.size())
	{
		pCurrentPage=m_pages[m_iCurrentPage].m_pPage;
		if(pCurrentPage->OnKillActive())
		{
			pCurrentPage->ShowWindow(SW_HIDE);
		}
		else
		{
			return false;
		}
	}
	
	//show the new page
	pNewPage->ShowWindow(SW_SHOWNOACTIVATE);
	m_iCurrentPage=iPage;
	
	//make sure the new page is properly positioned
	CRect rect;
	GetClientRect(&rect);
	PostMessage(WM_SIZE,SIZE_RESTORED,MAKELPARAM(rect.Width(),rect.Height()));
	return true;
}

void CTreeSettingsDlg::OnHelpBtn() 
{
	//send WM_COMMANDHELP (mfc internal message) to the currently active page
	if(m_iCurrentPage>=0 && m_iCurrentPage<m_pages.size())
	{
		m_pages[m_iCurrentPage].m_pPage->SendMessage(WM_COMMANDHELP);
	}
	
}

void CTreeSettingsDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	if(m_iCurrentPage>=0 && m_iCurrentPage<m_pages.size())
	{
		CRect rect;
		int height;
		int width;

		//active page
		CWnd *pPageFrame=GetDlgItem(IDC_TREESETTINGS_PAGEFRAME);
		pPageFrame->GetWindowRect(&rect);
		ScreenToClient(&rect);
		rect.right=cx-10;
		rect.bottom=cy-50;
		pPageFrame->MoveWindow(rect,FALSE);
		m_pages[m_iCurrentPage].m_pPage->MoveWindow(rect,FALSE);

		//line
		CRect line;
		CWnd *pLine=GetDlgItem(IDC_TREESETTINGS_LINE);
		pLine->GetWindowRect(&line);
		ScreenToClient(&line);
		height=line.Height();
		rect.top=cy-40-height;
		rect.bottom=rect.top+height;
		pLine->MoveWindow(&rect,FALSE);

		
		//tree
		m_tree.GetWindowRect(&rect);
		ScreenToClient(&rect);
		rect.bottom=cy-40;
		m_tree.MoveWindow(&rect,FALSE);
		
		//buttons
		CWnd *pOkBtn=GetDlgItem(IDOK);
		CWnd *pCancelBtn=GetDlgItem(IDCANCEL);
		CWnd *pHelpBtn=GetDlgItem(IDC_HELPBTN);

		pHelpBtn->GetWindowRect(&rect);
		ScreenToClient(&rect);
		width=rect.Width();
		height=rect.Height();

		rect.right=cx-10;
		rect.left=rect.right-width;
		rect.top=cy-30;
		rect.bottom=rect.top+height;
		pHelpBtn->MoveWindow(&rect,FALSE);

		rect.right=rect.left-5;
		rect.left=rect.right-width;
		pCancelBtn->MoveWindow(&rect,FALSE);

		rect.right=rect.left-5;
		rect.left=rect.right-width;
		pOkBtn->MoveWindow(&rect,FALSE);
		InvalidateRect(NULL);
	}
	
}
