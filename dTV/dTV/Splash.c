/////////////////////////////////////////////////////////////////////////////
// Splash.h
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
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
/////////////////////////////////////////////////////////////////////////////
//
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 11 Jan 2001   John Adcock           Split into separate file
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Splash.h"
#include "dTV.h"
#define DOLOGGING
#include "DebugLog.h"

HWND SplashWnd = NULL;

BOOL APIENTRY SplashProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{

	switch (message)
	{
	case WM_INITDIALOG:
		InvalidateRect(hDlg, NULL, TRUE);
		SetTimer(hDlg, 2, 5000, NULL);
		return (TRUE);

	case WM_TIMER:
		if (wParam == 2)
		{
			SplashWnd  = NULL;
			EndDialog(hDlg, 0);
		}

		return (FALSE);
	}
	return (FALSE);
	UNREFERENCED_PARAMETER(lParam);
}

void ShowSpashScreen()
{
	SplashWnd = CreateDialog(hInst, "SPLASHBOX", NULL, SplashProc);
	SetWindowPos(SplashWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOCOPYBITS | SWP_NOSIZE);
}


void AddSplashTextLine(const char* szText)
{
	if(SplashWnd)
	{
		int nItem;
		nItem = ListBox_AddString(GetDlgItem(SplashWnd, IDC_LIST1), szText);
		ListBox_SetCurSel(GetDlgItem(SplashWnd, IDC_LIST1), nItem);
		InvalidateRect(GetDlgItem(SplashWnd, IDC_LIST1), NULL, TRUE);
		Sleep(20);
	}
	LOG(szText);
}