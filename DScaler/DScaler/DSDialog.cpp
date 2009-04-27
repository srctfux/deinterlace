////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009 John Adcock.  All rights reserved.
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
 * @file Dialog.cpp
 */

#include "stdafx.h"
#include "DSDialog.h"
#include "DScaler.h"

using namespace std;


BOOL CALLBACK CDSDialog::MasterModalDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    if(message == WM_INITDIALOG)
    {
        SetWindowLong(hDlg, DWL_USER, (LONG)(CDSDialog*)lParam);
        SetWindowContextHelpId(hDlg, ((CDSDialog*)lParam)->m_HelpID);
    }
    CDSDialog* myDialogObject = (CDSDialog*)GetWindowLong(hDlg, DWL_USER);
    if(myDialogObject)
    {
        if(message != WM_HELP)
        {
            return myDialogObject->DialogProc(hDlg, message, wParam, lParam);
        }
        else
        {
            return SendMessage(GetMainWnd(), message, wParam, lParam);
        }
    }
    else
    {
        return FALSE;
    }
}


CDSDialog::CDSDialog(LPCSTR ResourceId) :
    m_ResourceId(ResourceId)
{
}

CDSDialog::~CDSDialog()
{
}

INT_PTR CDSDialog::DoModal(HWND hParent)
{
    return DialogBoxParam(hResourceInst, m_ResourceId, hParent, MasterModalDialogProc, (LPARAM)this);
}

HWND CDSDialog::Create(HWND hParent)
{
    return CreateDialogParam(hResourceInst, m_ResourceId, hParent, MasterModalDialogProc, (LPARAM)this);
}

void CDSDialog::Destroy(HWND hDlg)
{
    DestroyWindow(hDlg);
}

string GetDlgItemString(HWND hDlg, int id)
{
    HWND hWnd = GetDlgItem(hDlg, id);
    int TextSize = GetWindowTextLength(hWnd);
    if(TextSize > 0)
    {
        vector<char> Buffer(TextSize + 1);
        GetWindowText(hWnd, &Buffer[0], TextSize + 1);
        return &Buffer[0];
    }
    else
    {
        return "";
    }
}

void CDSDialog::SetHelpID(int HelpID)
{
    m_HelpID = HelpID;
}

int GetDlgItemInt(HWND hDlg, int id)
{
    string Value(GetDlgItemString(hDlg, id));
    return FromString<int>(Value);
}

void SetDlgItemInt(HWND hDlg, int id, int Value)
{
    string ValueAsString(ToString(Value));
    SetWindowText(GetDlgItem(hDlg, id), ValueAsString.c_str());
}

