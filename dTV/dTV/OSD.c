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
#include "Status.h"

//---------------------------------------------------------------------------
// Global OSD Information structure
OSD_INFO    grOSD = {""};
BOOL        bOverride = FALSE;

//---------------------------------------------------------------------------
// Display specified OSD text with autohide
void OSD_ShowText(HWND hWnd, LPCTSTR szText, double dfSize)
{
    if (bOverride) return;
	if (strlen(szText))
	{
		HDC hDC;
		hDC = GetDC(hWnd);

        grOSD.dfSize = dfSize;
        grOSD.dfXpos = 0.9;
        grOSD.dfYpos = 0.1;
		strncpy(grOSD.szText, szText, sizeof(grOSD.szText));
        
		OSD_Redraw(hWnd, hDC);
		ReleaseDC(hWnd, hDC);
		SetTimer(hWnd, OSD_TIMER_ID, OSD_TIMER_DELAY, NULL);
	}
	else
	{
		// If OSD message is blank, kill previous OSD message
		OSD_Clear(hWnd);
	}
	StatusBar_Repaint();
}

//---------------------------------------------------------------------------
// Displayed specified OSD text without autohide timer.
// Stays on screen until a new OSD message replaces current OSD message.
void OSD_ShowTextPersistent(HWND hWnd, LPCTSTR szText, double dfSize)
{
    if (bOverride) return;
	KillTimer(hWnd, OSD_TIMER_ID);
	if (strlen(szText))
	{
		HDC hDC;
		hDC = GetDC(hWnd);
        grOSD.dfSize = dfSize;
        grOSD.dfXpos = 0.9;
        grOSD.dfYpos = 0.1;
		strncpy(grOSD.szText, szText, sizeof(grOSD.szText));
        OSD_Redraw(hWnd, hDC);
		ReleaseDC(hWnd, hDC);
	}
	else
	{
		// If OSD message is blank, kill previous OSD message
		OSD_Clear(hWnd);
	}
	StatusBar_Repaint();
}

//---------------------------------------------------------------------------
// Override all previous OSD text, and force this current OSD text
// to override all other OSD text showings (done by the above functions).
// This is useful for external programs to override dTV's own OSD text
// for its own controls.
void OSD_ShowTextOverride(HWND hWnd, LPCTSTR szText, double dfSize)
{
    bOverride = FALSE;
    OSD_ShowText(hWnd, szText, dfSize);
    bOverride = TRUE;
}

//---------------------------------------------------------------------------
// Clear currently displayed OSD
void OSD_Clear(HWND hWnd)
{
	KillTimer(hWnd, OSD_TIMER_ID);
    bOverride = FALSE;
	lstrcpy(grOSD.szText, "");
	InvalidateRect(hWnd, NULL, FALSE);
}

//---------------------------------------------------------------------------
// OSD Redrawing code.  Can be called from a paint event.
void OSD_Redraw(HWND hWnd, HDC hDC)
{
	HFONT		hTmp, hOSDfont;
	int			nLen, nFontsize;
	int			nXpos, nYpos;
    int         nXWinSize, nYWinSize;
	RECT		winRect;
	TEXTMETRIC	tmOSDFont;
	SIZE		sizeText;

	nLen = strlen(grOSD.szText);
	if (nLen && hDC != NULL)
	{
        if (grOSD.dfSize == 0) grOSD.dfSize = OSD_DEFAULT_SIZE_PERC;

		GetClientRect(hWnd,&winRect);
		PaintColorkey(hWnd, TRUE, hDC, &winRect);
		nFontsize = (int)((double)(winRect.bottom - winRect.top) * (grOSD.dfSize / 100.00));

		// Set specified font
		hOSDfont = CreateFont(nFontsize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, VARIABLE_PITCH | FF_SWISS, OSD_FONT);
		if (!hOSDfont)
		{
			// Fallback to Arial
			hOSDfont = CreateFont(nFontsize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, VARIABLE_PITCH | FF_SWISS, "Arial");
			if (!hOSDfont)
			{
				// Otherwise, fallback to any available font
				hOSDfont = CreateFont(nFontsize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, VARIABLE_PITCH | FF_SWISS, "");
			}
		}
		if (!hOSDfont) ErrorBox("Failed To Create OSD Font");

		hTmp = SelectObject(hDC, hOSDfont);
		if (hTmp)
		{
			SetBkMode(hDC, TRANSPARENT);
			GetTextMetrics(hDC, &tmOSDFont);
			GetTextExtentPoint32(hDC, grOSD.szText, strlen(grOSD.szText), &sizeText);

            nXWinSize = winRect.right  - winRect.left;
            nYWinSize = winRect.bottom - winRect.top;

			nXpos = (int)((double)nXWinSize * grOSD.dfXpos) - sizeText.cx;
			nYpos = (int)((double)nYWinSize * grOSD.dfYpos);

			// Draw OSD outline
			SetTextColor(hDC, OSD_COLOR_OUTLINE);
			TextOut(hDC, nXpos - 2, nYpos, grOSD.szText, strlen(grOSD.szText));
			TextOut(hDC, nXpos + 2, nYpos, grOSD.szText, strlen(grOSD.szText));
			TextOut(hDC, nXpos, nYpos - 2, grOSD.szText, strlen(grOSD.szText));
			TextOut(hDC, nXpos, nYpos + 2, grOSD.szText, strlen(grOSD.szText));

			// Draw OSD text
			SetTextColor(hDC, OSD_COLOR_FILL);
			TextOut(hDC, nXpos, nYpos, grOSD.szText, strlen(grOSD.szText));
			
			SelectObject(hDC, hTmp);
			DeleteObject(hOSDfont);
		}
	}
}
