/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file TreeSettingsDlg.h header file for tree settings dialog
 */

#if !defined(AFX_TREESETTINGSDLG_H__80CDEE5F_F683_4BEB_9028_5EE2ACEE51F6__INCLUDED_)
#define AFX_TREESETTINGSDLG_H__80CDEE5F_F683_4BEB_9028_5EE2ACEE51F6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TreeSettingsDlg.h : header file
//

#include <vector>
#include "Setting.h"
#include "TreeSettingsPage.h"
#include "..\DScalerRes\resource.h"
#include "GradientStatic.h"

#define FILTER_SETTINGS_MASK        0x01
#define DEINTERLACE_SETTINGS_MASK   0x02
#define ADVANCED_SETTINGS_MASK      0x04
#define ALL_SETTINGS_MASK            0x08


/**
 * Tree based settings dialog.
 * @see CTreeSettingsPage
 */
class CTreeSettingsDlg : public CDialog
{
// Construction
public:
    CTreeSettingsDlg(CString caption,CWnd* pParent = NULL);   // standard constructor

    /**
     * Adds a new page to the tree.
     *
     * @param pPage pointer to the page
     * @param parent parent node in the tree.
     * note that the parent must already have been added with a call to AddPage
     * @return integer used when adding new pages as a child of this one
     */
    int AddPage(CTreeSettingsPage *pPage,int parent=-1,int imageIndex=0,int imageIndexSelected=0);

    bool ShowPage(int iPage);

    /**
     * @param iStartPage page that will be shown initialy
     */
    void SetStartPage(int iStartPage) {m_iStartPage=iStartPage;};
    static void ShowTreeSettingsDlg(int iSettingsMask);

// Dialog Data
    //{{AFX_DATA(CTreeSettingsDlg)
    enum { IDD = IDD_TREESETTING };
    CTreeCtrl    m_tree;
    CGradientStatic    m_PageHeader;
    //}}AFX_DATA


// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CTreeSettingsDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CTreeSettingsDlg)
    virtual void OnOK();
    virtual void OnCancel();
    afx_msg void OnSelchangingTree(NMHDR* pNMHDR, LRESULT* pResult);
    virtual BOOL OnInitDialog();
    afx_msg void OnHelpBtn();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnPaint();
    afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
    afx_msg LRESULT OnNcHitTest(CPoint point);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    ///internal class. used for storing information about one page
    class CPageInfo
    {
    public:
        CTreeSettingsPage *m_pPage;
        int m_parent;
        HTREEITEM m_hTreeItem;
        int m_imageIndex;
        int m_imageIndexSelected;
    };
    ///vector of all pages
    std::vector<CPageInfo> m_pages;
    CString m_settingsDlgCaption;
    ///current page
    int m_iCurrentPage;
    ///start page
    int m_iStartPage;
    //image list
    CImageList m_ImageList;

    CFont m_StaticFont;
    CRect m_GripperRect;
    CPoint m_MinMaxInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TREESETTINGSDLG_H__80CDEE5F_F683_4BEB_9028_5EE2ACEE51F6__INCLUDED_)
