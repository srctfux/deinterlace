/////////////////////////////////////////////////////////////////////////////
// ErrorBox.c
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
// Change Log
//
// Date          Developer             Changes
//
// 11 Aug 2000   John Adcock           Better support for error messages
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

extern HWND hWnd;

void _ErrorBox(HWND hwndParent, LPCSTR szFile, int Line, LPCSTR szMessage)
{
	char szDispMessage[1024];

	_snprintf(szDispMessage,1024, "%s\nThe error occured in %s at line %d", szMessage, szFile, Line);
	if(hwndParent == NULL)
	{
		MessageBox(hWnd, szDispMessage, "dTV Error", MB_ICONSTOP | MB_OK); 
	}
	else
	{
		MessageBox(hwndParent, szDispMessage, "dTV Error", MB_ICONSTOP | MB_OK); 
	}
}


