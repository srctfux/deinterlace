/////////////////////////////////////////////////////////////////////////////
// $Id: TreeSettingsGeneric.h,v 1.5 2002-09-26 11:33:42 kooiman Exp $
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
// 22 Jun 2001   Torbj�rn Jansson      Initial release
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file TreeSettingsGeneric.h Header file for CTreeSettingsGeneric class
 */

#if !defined(AFX_TREESETTINGSDLG_H__AF9102F2_06B5_43B0_8DD7_C3CFFA8DCCDB__INCLUDED_)
#define AFX_TREESETTINGSDLG_H__AF9102F2_06B5_43B0_8DD7_C3CFFA8DCCDB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DS_ApiCommon.h"
#include "HSListBox.h"
#include "TreeSettingsPage.h"
#include "Settings.h"
#include "Setting.h"
#include "..\DScalerRes\resource.h"

class CSimpleSetting;

/**
 * Generic settings page for tree based settings dialog.
 * This dialog is almost the same as CSettingsDlg but is modified to work as
 * a page is CTreeSettingsDlg
 *
 * @see CHSListBox
 * @see CTreeSettingsDlg
 */
class CTreeSettingsGeneric : public CTreeSettingsPage
{
// Construction
public:

    /**
	 * @param name name used in the tree
	 * @param settings pointer to array of SETTING
     * @param count size of settings array
	 */
    CTreeSettingsGeneric(CString name,SETTING* settings,long count);
    CTreeSettingsGeneric(CString name,vector<CSimpleSetting*> settings); 
    ~CTreeSettingsGeneric();

// Dialog Data
    //{{AFX_DATA(CTreeSettingsGeneric)
    enum { IDD = IDD_TREESETTINGS_GENERIC };
    CComboBox   m_Combo;
    CButton m_CheckBox;
    CSpinButtonCtrl m_Spin;
    CSliderCtrl m_Slider;
    CEdit   m_Edit;
    CButton m_DefaultButton;
    CHSListBox  m_ListBox;
    //}}AFX_DATA

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CTreeSettingsGeneric)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CTreeSettingsGeneric)
    virtual BOOL OnInitDialog();
    afx_msg void OnSelchangeList();
    afx_msg void OnChangeEdit();
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnSettingsDefault();
    afx_msg void OnCheckClick();
    afx_msg void OnSelchangeChoosefromlist();
    afx_msg void OnDeltaposSettingsSpin(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
	void OnOK();
private:
    void UpdateControls();
    long m_SettingsCount;
    vector<SETTING*> m_Settings;
    vector<CSimpleSetting*> m_CSettings;
    long m_CurrentSetting;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTINGSDLG_H__AF9102F2_06B5_43B0_8DD7_C3CFFA8DCCDB__INCLUDED_)
