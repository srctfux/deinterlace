/////////////////////////////////////////////////////////////////////////////
// $Id: ParseCardIniDlg.h,v 1.1 2005-10-28 16:43:13 to_see Exp $
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 Torsten Seeboth. All rights reserved.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
//////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARSECARDINIDLG_H__65DA7234_6548_47E7_A523_CBBA70880C09__INCLUDED_)
#define AFX_PARSECARDINIDLG_H__65DA7234_6548_47E7_A523_CBBA70880C09__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CParseCardIniDlg : public CDialog
{
public:
	CParseCardIniDlg(CWnd* pParent = NULL);	// Standard-Konstruktor
	~CParseCardIniDlg();

	//{{AFX_DATA(CParseCardIniDlg)
	enum { IDD = IDD_PARSECARDINI_DIALOG };
	CButton	    m_ctrlSortByName;
	CListBox    m_ctrlListError;
	CTreeCtrl	m_ctrlTreeCard;
	BOOL	    m_bSortByName;
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CParseCardIniDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV-Unterst�tzung
	//}}AFX_VIRTUAL

protected:
	HICON m_hIcon;

	//{{AFX_MSG(CParseCardIniDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBtnOpenIni();
	afx_msg void OnChkSort();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void       StartParseCardIni(BOOL bSort);

private:
    CString    m_LastIniFile;
	CRect      m_GripperRect;
	CPoint     m_MinMaxInfo;
	CImageList m_ImageList;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ f�gt unmittelbar vor der vorhergehenden Zeile zus�tzliche Deklarationen ein.

#endif // !defined(AFX_PARSECARDINIDLG_H__65DA7234_6548_47E7_A523_CBBA70880C09__INCLUDED_)
