/////////////////////////////////////////////////////////////////////////////
// $Id: TSOptionsDlg.h,v 1.2 2001-07-24 12:24:25 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2001   Eric Schmidt          Original Release.
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.1  2001/07/23 20:52:07  ericschmidt
// Added TimeShift class.  Original Release.  Got record and playback code working.
//
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSOPTIONSDLG_H__C76FD300_76CF_11D5_A1E2_AD9180F1D520__INCLUDED_)
#define AFX_TSOPTIONSDLG_H__C76FD300_76CF_11D5_A1E2_AD9180F1D520__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

// Declare this for friend declaration below.
class TimeShift;

/////////////////////////////////////////////////////////////////////////////
// CTSOptionsDlg dialog

class CTSOptionsDlg : public CDialog
{
    friend TimeShift;

public:
// Construction
	CTSOptionsDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTSOptionsDlg)
	enum { IDD = IDD_TSOPTIONS };
	CComboBox	m_WaveOutComboBox;
	CComboBox	m_WaveInComboBox;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTSOptionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTSOptionsDlg)
	afx_msg void OnButtonCompression();
	afx_msg void OnButtonOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TSOPTIONSDLG_H__C76FD300_76CF_11D5_A1E2_AD9180F1D520__INCLUDED_)
