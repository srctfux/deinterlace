/////////////////////////////////////////////////////////////////////////////
// OSD.c
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
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 8 Nov 2000    Michael Eskin         Initial onscreen display
//
// 28 Nov 2000   Mark Rejhon           Reorganization and visual improvements
//
// NOTICE FROM MARK: This code will probably be rewritten, but keeping 
// this code neat and architecturally well organized, will maximize code 
// recyclability.   There is a need for multiple independent OSD elements,
// such as persistent "MUTE" / "UNMUTE" with separate channel number that
// dissappears.   Perhaps some kind of a linked list of OSD's can be 
// maintained.   Keep a future multi-OSD architecture in mind when deciding
// to expand this code.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OSD.h"
#include "AspectRatio.h"
#include "Other.h"

char		gszOSD[512] = "";

//---------------------------------------------------------------------------
// Put up the current channel number
void OSD_ShowText(HWND hWnd,char *szText)
{
	if (strlen(szText))
	{
		strncpy(gszOSD, szText, sizeof(gszOSD));
		OSD_Redraw(hWnd);
		SetTimer(hWnd, OSD_TIMER_ID, OSD_TIMER_DELAY, NULL);
	}
	else
	{
		// If OSD message is blank, kill previous OSD message
		OSD_Clear(hWnd);
	}
}

//---------------------------------------------------------------------------
// Clear currently displayed OSD
void OSD_Clear(HWND hWnd)
{
	KillTimer(hWnd, OSD_TIMER_ID);
	lstrcpy(gszOSD, "");
	InvalidateRect(hWnd, NULL, FALSE);
}

//---------------------------------------------------------------------------
// OSD Redrawing code.  Can be called from a paint event.
void OSD_Redraw(HWND hWnd)
{
	HDC			hdc;
	HFONT		hTmp, hOSDfont;
	int			nLen, nFontsize;
	int			nXpos, nYpos;
	RECT		winRect;
	TEXTMETRIC	tmOSDFont;
	SIZE		sizeText;

	nLen = strlen(gszOSD);
	if (nLen)
	{
		InvalidateRect(hWnd,NULL,FALSE);
		PaintColorkey(hWnd, TRUE);
		GetWindowRect(hWnd,&winRect);
		nFontsize = (winRect.bottom - winRect.top) / 10;

		// Get the current window size
		hOSDfont = CreateFont(nFontsize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, VARIABLE_PITCH | FF_SWISS, OSD_FONT);
		if (!hOSDfont)
		{
			hOSDfont = CreateFont(nFontsize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, VARIABLE_PITCH | FF_SWISS, "Arial");
			if (!hOSDfont)
			{
				hOSDfont = CreateFont(nFontsize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, VARIABLE_PITCH | FF_SWISS, "");
			}
		}
		if (!hOSDfont) ErrorBox("Failed To Create OSD Font");

		hdc = GetDC(hWnd);
		if (hdc)
		{
			hTmp = SelectObject(hdc, hOSDfont);
			if (hTmp)
			{
				SetBkMode(hdc, TRANSPARENT);
				GetTextMetrics(hdc, &tmOSDFont);
				GetTextExtentPoint32(hdc, gszOSD, strlen(gszOSD), &sizeText);

				nXpos = 8 * (winRect.right - winRect.left) / 9 - sizeText.cx;
				nYpos = sizeText.cy;

				// Draw OSD outline
				SetTextColor(hdc, OSD_COLOR_OUTLINE);
				TextOut(hdc, nXpos - 2, nYpos, gszOSD, strlen(gszOSD));
				TextOut(hdc, nXpos + 2, nYpos, gszOSD, strlen(gszOSD));
				TextOut(hdc, nXpos, nYpos - 2, gszOSD, strlen(gszOSD));
				TextOut(hdc, nXpos, nYpos + 2, gszOSD, strlen(gszOSD));

				// Draw OSD text
				SetTextColor(hdc, OSD_COLOR_FILL);
				TextOut(hdc, nXpos, nYpos, gszOSD, strlen(gszOSD));
				
				SelectObject(hdc, hTmp);
				DeleteObject(hOSDfont);
			}
			ReleaseDC(hWnd, hdc);			
		}
	}
}
