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


char szFontName[128] = "Arial";
long OutlineColor = RGB(0,0,0);
long TextColor = RGB(0,255,0);
long DefaultSizePerc = 10;
BOOL bAntiAlias = TRUE;
BOOL bOutline = TRUE;
eOSDBackground Background;


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
	CHAR		szCurrentFont[64];
	HFONT		hTmp, hOSDfont, hOSDfontOutline;
	int			nLen, nFontsize;
	int			nXpos, nYpos;
    int         nXWinSize, nYWinSize;
	RECT		winRect;
	TEXTMETRIC	tmOSDFont;
	SIZE		sizeText;
	DWORD       dwQuality = 0;

	nLen = strlen(grOSD.szText);
	if (nLen && hDC != NULL)
	{
        if (grOSD.dfSize == 0) grOSD.dfSize = DefaultSizePerc;

		GetClientRect(hWnd,&winRect);
		PaintColorkey(hWnd, TRUE, hDC, &winRect);
		nFontsize = (int)((double)(winRect.bottom - winRect.top) * (grOSD.dfSize / 100.00));

		// Set specified font
		if(bAntiAlias)
		{
			dwQuality = ANTIALIASED_QUALITY;
		}

		strcpy(szCurrentFont, szFontName);
		hOSDfont = CreateFont(nFontsize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, dwQuality, DEFAULT_PITCH | FF_DONTCARE, szFontName);
		if (!hOSDfont)
		{
			// Fallback to Arial
			strcpy(szCurrentFont, "Arial");
			hOSDfont = CreateFont(nFontsize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, dwQuality, VARIABLE_PITCH | FF_SWISS, szCurrentFont);
			if (!hOSDfont)
			{
				// Otherwise, fallback to any available font
				strcpy(szCurrentFont, "");
				hOSDfont = CreateFont(nFontsize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, dwQuality, VARIABLE_PITCH | FF_SWISS, szCurrentFont);
			}
		}
		if (!hOSDfont) ErrorBox("Failed To Create OSD Font");
		hOSDfontOutline = CreateFont(nFontsize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_PITCH | FF_DONTCARE, szCurrentFont);

		hTmp = SelectObject(hDC, hOSDfontOutline);
		if (hTmp)
		{
			GetTextMetrics(hDC, &tmOSDFont);
			GetTextExtentPoint32(hDC, grOSD.szText, strlen(grOSD.szText), &sizeText);

            nXWinSize = winRect.right  - winRect.left;
            nYWinSize = winRect.bottom - winRect.top;

			nXpos = (int)((double)nXWinSize * grOSD.dfXpos) - sizeText.cx;
			nYpos = (int)((double)nYWinSize * grOSD.dfYpos);

			// Draw the requested background for the text
			switch(Background)
			{
			case OSDB_TRANSPARENT:
				SetBkMode(hDC, TRANSPARENT);
				SetBkColor(hDC, OutlineColor);
				break;
			
			case OSDB_BLOCK:
				SetBkMode(hDC, OPAQUE);
				SetBkColor(hDC, OutlineColor);
				break;
			
			case OSDB_SHADED:
				{
					HBRUSH hBrush;
					HBRUSH hBrushOld;
					HBITMAP hBM;
					WORD bBrushBits[8] = {0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, };
					SetBkMode(hDC, TRANSPARENT);
					SetTextColor(hDC, OutlineColor);
					SetBkColor(hDC, Overlay_GetColor());
                    hBM = CreateBitmap(8, 8, 1, 1, (LPBYTE)bBrushBits); 
                    hBrush = CreatePatternBrush(hBM); 
					hBrushOld = SelectObject(hDC, hBrush);
					if(bOutline)
					{
						PatBlt(hDC, nXpos - 2, nYpos - 2, sizeText.cx + 4, sizeText.cy + 4, PATCOPY);
					}
					else
					{
						PatBlt(hDC, nXpos, nYpos, sizeText.cx, sizeText.cy, PATCOPY);
					}
					SelectObject(hDC, hBrushOld);
					DeleteObject(hBrush);
					DeleteObject(hBM);
				}
				break;
			default:
				break;
			}

			if(bOutline)
			{
				// Draw OSD outline if required
				SetTextColor(hDC, OutlineColor);
				TextOut(hDC, nXpos - 2, nYpos, grOSD.szText, strlen(grOSD.szText));
				TextOut(hDC, nXpos + 2, nYpos, grOSD.szText, strlen(grOSD.szText));
				TextOut(hDC, nXpos, nYpos - 2, grOSD.szText, strlen(grOSD.szText));
				TextOut(hDC, nXpos, nYpos + 2, grOSD.szText, strlen(grOSD.szText));
				TextOut(hDC, nXpos - 1, nYpos - 1, grOSD.szText, strlen(grOSD.szText));
				TextOut(hDC, nXpos + 1, nYpos - 1, grOSD.szText, strlen(grOSD.szText));
				TextOut(hDC, nXpos - 1, nYpos + 1, grOSD.szText, strlen(grOSD.szText));
				TextOut(hDC, nXpos + 1, nYpos + 1, grOSD.szText, strlen(grOSD.szText));
			}

			// Draw OSD text
			if (SelectObject(hDC, hOSDfont))
			{
				SetTextColor(hDC, TextColor);
				SetBkColor(hDC, OutlineColor);
				TextOut(hDC, nXpos, nYpos, grOSD.szText, strlen(grOSD.szText));
			}
			
			SelectObject(hDC, hTmp);
			DeleteObject(hOSDfont);
			DeleteObject(hOSDfontOutline);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////

SETTING OSDSettings[OSD_SETTING_LASTONE] =
{
	{
		"OSD Outline Color", NUMBER, 0, &OutlineColor,
		 RGB(0,0,0), 0, RGB(255,255,255), 1, NULL,
		"OSD", "OutlineColor", NULL,
	},
	{
		"OSD Text Color", NUMBER, 0, &TextColor,
		 RGB(0,255,0), 0, RGB(255,255,255), 1, NULL,
		"OSD", "TextColor", NULL,
	},
	{
		"OSD Default Size", NUMBER, 0, &DefaultSizePerc,
		 10, 0, 100, 1, NULL,
		"OSD", "DefaultSizePerc", NULL,
	},
	{
		"OSD Anti Alias", ONOFF, 0, &bAntiAlias,
		 TRUE, 0, 1, 1, NULL,
		"OSD", "AntiAlias", NULL,
	},
	{
		"OSD Background", NUMBER, 0, &Background,
		 OSDB_TRANSPARENT, 0, OSDBACK_LASTONE - 1, 1, NULL,
		"OSD", "Background", NULL,
	},
	{
		"OSD Outline Text", ONOFF, 0, &bOutline,
		 TRUE, 0,  1, 1, NULL,
		"OSD", "Outline", NULL,
	},
};


SETTING* OSD_GetSetting(OSD_SETTING Setting)
{
	if(Setting > -1 && Setting < OSD_SETTING_LASTONE)
	{
		return &(OSDSettings[Setting]);
	}
	else
	{
		return NULL;
	}
}

void OSD_ReadSettingsFromIni()
{
	int i;
	for(i = 0; i < OSD_SETTING_LASTONE; i++)
	{
		Setting_ReadFromIni(&(OSDSettings[i]));
	}
	GetPrivateProfileString("OSD", "FontName", "Arial", szFontName, sizeof(szFontName) , GetIniFileForSettings());
}

void OSD_WriteSettingsToIni()
{
	int i;
	for(i = 0; i < OSD_SETTING_LASTONE; i++)
	{
		Setting_WriteToIni(&(OSDSettings[i]));
	}
	WritePrivateProfileString("OSD", "FontName", szFontName, GetIniFileForSettings());
}
