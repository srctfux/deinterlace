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
// 23 Feb 2001   Michael Samblanet     Calculate OSD rect so we do not 
//                                     invalidate entire display to erase
//
// 24 Feb 2001   Michael Samblanet     Moved rect into OSDInfo structure
//                                     Should improve compatability with coming
//                                     OSD changes
//
// 25 Feb 2001   Laurent Garnier       Added management of multiple OSD texts
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
OSD_INFO    grOSD[OSD_MAX_TEXT];
int         NbText = 0;
BOOL        bOverride = FALSE;

//---------------------------------------------------------------------------
// Return the number of texts currently defined for OSD
int OSD_GetNbText()
{
	return (NbText);
}

//---------------------------------------------------------------------------
// Clean the list of texts for OSD
void OSD_ClearAllTexts()
{
	NbText = 0;
}

//---------------------------------------------------------------------------
// Add a new text to the list of texts for OSD
void OSD_AddText(LPCTSTR szText, double dfSize, long textColor, OSD_TEXT_XPOS textXpos, double dfXpos, double dfYpos)
{
	if ( (strlen(szText) == 0) || (NbText >= OSD_MAX_TEXT) )
	{
		return;
	}

	if (dfSize == 0)
	{
        grOSD[NbText].dfSize = DefaultSizePerc;
	}
	else
	{
		grOSD[NbText].dfSize = dfSize;
	}
	if (textColor == 0)
	{
		grOSD[NbText].textColor = TextColor;
	}
	else
	{
		grOSD[NbText].textColor = textColor;
	}
	grOSD[NbText].textXpos = textXpos;
	grOSD[NbText].dfXpos = dfXpos;
	grOSD[NbText].dfYpos = dfYpos;
	strncpy(grOSD[NbText].szText, szText, sizeof(grOSD[NbText].szText));

	NbText++;
}

//---------------------------------------------------------------------------
// Display defined OSD texts
void OSD_Show(HWND hWnd, BOOL persistent)
{
    RECT		winRect;
	HDC         hDC;

	if (bOverride) return;
	if (persistent == TRUE)
		KillTimer(hWnd, OSD_TIMER_ID);
	hDC = GetDC(hWnd);
	GetClientRect(hWnd,&winRect);
	PaintColorkey(hWnd, TRUE, hDC, &winRect);
	OSD_Redraw(hWnd, hDC);
	ReleaseDC(hWnd, hDC);
	if (persistent == FALSE)
		SetTimer(hWnd, OSD_TIMER_ID, OSD_TIMER_DELAY, NULL);
	StatusBar_Repaint();
}

//---------------------------------------------------------------------------
// Display specified OSD text with autohide
void OSD_ShowText(HWND hWnd, LPCTSTR szText, double dfSize)
{
    if (bOverride) return;
	if (strlen(szText))
	{
		OSD_ClearAllTexts();
		OSD_AddText(szText, dfSize, 0, OSD_XPOS_RIGHT, 0.9, 0.1);
		OSD_Show(hWnd, FALSE);
	}
	else
	{
		// If OSD message is blank, kill previous OSD message
		OSD_Clear(hWnd);
	}
}

//---------------------------------------------------------------------------
// Displayed specified OSD text without autohide timer.
// Stays on screen until a new OSD message replaces current OSD message.
void OSD_ShowTextPersistent(HWND hWnd, LPCTSTR szText, double dfSize)
{
    if (bOverride) return;
	if (strlen(szText))
	{
		OSD_ClearAllTexts();
		OSD_AddText(szText, dfSize, 0, OSD_XPOS_RIGHT, 0.9, 0.1);
		OSD_Show(hWnd, TRUE);
	}
	else
	{
		// If OSD message is blank, kill previous OSD message
		OSD_Clear(hWnd);
	}
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
	int	i;

	KillTimer(hWnd, OSD_TIMER_ID);
    bOverride = FALSE;
	for (i = 0 ; i < NbText ; i++)
	{
		InvalidateRect(hWnd, &(grOSD[i].currentRect), FALSE);
	}
	OSD_ClearAllTexts();
	StatusBar_Repaint();
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
	TEXTMETRIC	tmOSDFont;
	SIZE		sizeText;
   	RECT		winRect;
	DWORD       dwQuality = 0;
	int			i;

	nLen = strlen(grOSD[0].szText);
	if (nLen && hDC != NULL)
	{
		for (i = 0 ; i < NbText ; i++)
		{

		// LG 02/25/2001 This line is no more needed
		// if (grOSD[i].dfSize == 0) grOSD[i].dfSize = DefaultSizePerc;

	    GetClientRect(hWnd,&winRect);
		nFontsize = (int)((double)(winRect.bottom - winRect.top) * (grOSD[i].dfSize / 100.00));

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
		hOSDfontOutline = CreateFont(nFontsize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, szCurrentFont);

		if (!hOSDfontOutline) hOSDfontOutline = hOSDfont;

		hTmp = SelectObject(hDC, hOSDfontOutline);
		if (hTmp)
		{
			GetTextMetrics(hDC, &tmOSDFont);
			GetTextExtentPoint32(hDC, grOSD[i].szText, strlen(grOSD[i].szText), &sizeText);

            nXWinSize = winRect.right  - winRect.left;
            nYWinSize = winRect.bottom - winRect.top;

			switch (grOSD[i].textXpos)
			{
			case OSD_XPOS_RIGHT:
				nXpos = (int)((double)nXWinSize * grOSD[i].dfXpos) - sizeText.cx;
				break;
			case OSD_XPOS_CENTER:
				nXpos = (int)((double)nXWinSize * grOSD[i].dfXpos - (double)sizeText.cx / 2.0);
				break;
			case OSD_XPOS_LEFT:
			default:
				nXpos = (int)((double)nXWinSize * grOSD[i].dfXpos);
				break;
			}

			nYpos = (int)((double)nYWinSize * grOSD[i].dfYpos);

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
				TextOut(hDC, nXpos - 2, nYpos, grOSD[i].szText, strlen(grOSD[i].szText));
				TextOut(hDC, nXpos + 2, nYpos, grOSD[i].szText, strlen(grOSD[i].szText));
				TextOut(hDC, nXpos, nYpos - 2, grOSD[i].szText, strlen(grOSD[i].szText));
				TextOut(hDC, nXpos, nYpos + 2, grOSD[i].szText, strlen(grOSD[i].szText));
				TextOut(hDC, nXpos - 1, nYpos - 1, grOSD[i].szText, strlen(grOSD[i].szText));
				TextOut(hDC, nXpos + 1, nYpos - 1, grOSD[i].szText, strlen(grOSD[i].szText));
				TextOut(hDC, nXpos - 1, nYpos + 1, grOSD[i].szText, strlen(grOSD[i].szText));
				TextOut(hDC, nXpos + 1, nYpos + 1, grOSD[i].szText, strlen(grOSD[i].szText));
			}

			// Draw OSD text
			if (SelectObject(hDC, hOSDfont))
			{
				SetTextColor(hDC, grOSD[i].textColor);
				SetBkColor(hDC, OutlineColor);
				TextOut(hDC, nXpos, nYpos, grOSD[i].szText, strlen(grOSD[i].szText));

				{   // MRS 2-23-01 Calculate rectnagle for the entire OSD 
					// so we do not invalidate the entire window to remove it.
					SIZE sz;
					GetTextExtentExPoint(hDC, grOSD[i].szText, strlen(grOSD[i].szText), 
											32000, NULL, NULL, &sz);
					grOSD[i].currentRect.left = nXpos-4; if (grOSD[i].currentRect.left < 0) grOSD[i].currentRect.left = 0;
					grOSD[i].currentRect.right = nXpos + sz.cx + 4;
					grOSD[i].currentRect.top = nYpos-4; if (grOSD[i].currentRect.top < 0) grOSD[i].currentRect.top = 0;
					grOSD[i].currentRect.bottom = nYpos + sz.cy + 4;
				}
			}

			SelectObject(hDC, hTmp);
			DeleteObject(hOSDfont);
			DeleteObject(hOSDfontOutline);

			}			
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
		 RGB(0,0,0), 0, RGB(255,255,255), 1, 1,
		 NULL,
		"OSD", "OutlineColor", NULL,
	},
	{
		"OSD Text Color", NUMBER, 0, &TextColor,
		 RGB(0,255,0), 0, RGB(255,255,255), 1, 1,
		 NULL,
		"OSD", "TextColor", NULL,
	},
	{
		"OSD Default Size", NUMBER, 0, &DefaultSizePerc,
		 10, 0, 100, 1, 1,
		 NULL,
		"OSD", "DefaultSizePerc", NULL,
	},
	{
		"OSD Anti Alias", ONOFF, 0, &bAntiAlias,
		 TRUE, 0, 1, 1, 1,
		 NULL,
		"OSD", "AntiAlias", NULL,
	},
	{
		"OSD Background", NUMBER, 0, &Background,
		 OSDB_TRANSPARENT, 0, OSDBACK_LASTONE - 1, 1, 1,
		 NULL,
		"OSD", "Background", NULL,
	},
	{
		"OSD Outline Text", ONOFF, 0, &bOutline,
		 TRUE, 0,  1, 1, 1,
		 NULL,
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
