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
// 03 Mar 2001   Laurent Garnier       Added functions OSD_ShowInfosScreen
//                                     and OSD_GetLineYpos
//
// 10 Mar 2001   Laurent Garnier       Status bar height taken into account when
//                                     calculating texts placement
//
// 18 Mar 2001   Laurent Garnier       Added multiple screens feature
//                                     Added specific screen for WSS data decoding
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

#include "bt848.h"
#include "ProgramList.h"
#include "Audio.h"
#include "MixerDev.h"
#include "OutThreads.h"
#include "FD_60Hz.h"
#include "Filter.h"
#include "Dialogs.h"
#include "dTV.h"
#include "VBI_WSSdecode.h"

#define	OSD_SCREEN_1	1
#define	OSD_SCREEN_2	2
#define	OSD_SCREEN_3	3
#define	OSD_SCREEN_4	4
#define	OSD_SCREEN_5	5
#define	OSD_SCREEN_6	6
#define	OSD_SCREEN_7	7
#define	OSD_SCREEN_8	8
#define	OSD_SCREEN_9	9
#define	OSD_SCREEN_10	10

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
struct {
	int		screen_id;	// OSD screen identifier
	BOOL	active;		// Screen to take into account or not
} ActiveScreens[] = {
	{	OSD_SCREEN_1,	TRUE	},
	{	OSD_SCREEN_2,	TRUE	},
	{	OSD_SCREEN_3,	FALSE	},
	{	OSD_SCREEN_4,	FALSE	},
	{	OSD_SCREEN_5,	FALSE	},
	{	OSD_SCREEN_6,	FALSE	},
	{	OSD_SCREEN_7,	FALSE	},
	{	OSD_SCREEN_8,	FALSE	},
	{	OSD_SCREEN_9,	FALSE	},
	{	OSD_SCREEN_10,	FALSE	},
};
int	IdxCurrentScreen = -1;	// index of the current displayed OSD screen


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
		IdxCurrentScreen = -1;
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
		IdxCurrentScreen = -1;
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
	IdxCurrentScreen = -1;
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
	    GetClientRect(hWnd,&winRect);
		nXWinSize = winRect.right  - winRect.left;
		nYWinSize = winRect.bottom - winRect.top;
		if (IsStatusBarVisible())
		{
			nYWinSize -= StatusBar_Height();
		}

		for (i = 0 ; i < NbText ; i++)
		{

		// LG 02/25/2001 This line is no more needed
		// if (grOSD[i].dfSize == 0) grOSD[i].dfSize = DefaultSizePerc;

		nFontsize = (int)((double)nYWinSize * (grOSD[i].dfSize / 100.00));

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

//---------------------------------------------------------------------------
// Calculate vertical position of line in OSD screen
//    Return value between 0 (top) and 1 (bottom)
//    Use line number > 0 if reference is top
//    Use line number < 0 if reference is bottom
//    dfMargin is a percent of screen height/width and value must be between 0 and 1
//    dfSize is a percent of screen height and value must be between 0 and 100
double OSD_GetLineYpos (int nLine, double dfMargin, double dfSize)
{
	double	dfY;
	double	dfH = dfSize / 100;

	// Line number 0 has no sense
	if (nLine == 0)	return (0);

	if (nLine > 0)
	{
		dfY = dfMargin + ((double)nLine - 1) * dfH;
	}
	else
	{
		dfY = 1 - dfMargin + (double)nLine * dfH;
	}

	// Line outside screen
	if ( (dfY < 0) || ((dfY + dfH) > 1) )
	{
		dfY = 0;
	}

	return (dfY);
}

//---------------------------------------------------------------------------
// Display multiple informations on scrren
void OSD_ShowInfosScreen(HWND hWnd, double dfSize)
{
	double	dfMargin = 0.05;	// 5% of screen height/width
	char	szInfo[64];
	int		nLine;
	int		NbScreens;			// number of OSD scrrens
	int		NbActiveScreens;	// number of active OSD screens
	int		IdxScreen;
	int		i, idx;

	// determine which screen to display
	NbScreens = sizeof (ActiveScreens) / sizeof (ActiveScreens[0]);
	IdxScreen = IdxCurrentScreen + 1;
	if (IdxScreen >= NbScreens)
		IdxScreen = 0;
	IdxCurrentScreen = -1;
	for (i = IdxScreen ; i < (IdxScreen+NbScreens) ; i++)
	{
		idx = i % NbScreens;
		if (ActiveScreens[idx].active)
		{
			NbActiveScreens++;
			if (IdxCurrentScreen == -1)
				IdxCurrentScreen = idx;
		}
	}
	// Case : no OSD screen
	if (IdxCurrentScreen == -1)
		return;

	OSD_ClearAllTexts();

	switch (ActiveScreens[IdxCurrentScreen].screen_id)
	{
	// GENERAL SCREEN
	case OSD_SCREEN_1:
		if (dfSize == 0)	dfSize = 4;	// 4% of screen height

		// dTV version
		OSD_AddText(GetProductNameAndVersion(), dfSize, 0, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (1, dfMargin, dfSize));

		// Channel
		nLine = 2;
		if (Setting_GetValue(BT848_GetSetting(VIDEOSOURCE)) == SOURCE_TUNER)
		{
			OSD_AddText(Programm[CurrentProgramm].Name, dfSize, 0, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine, dfMargin, dfSize));
			nLine++;
		}

		// Video input + video format
		switch (Setting_GetValue(BT848_GetSetting(VIDEOSOURCE)))
		{
		case SOURCE_TUNER:
			strcpy(szInfo, "Video : Tuner ");
			break;
		case SOURCE_COMPOSITE:
			strcpy(szInfo, "Video : Composite ");
			break;
		case SOURCE_SVIDEO:
			strcpy(szInfo, "Video : S-Video ");
			break;
		case SOURCE_OTHER1:
			strcpy(szInfo, "Video : Other 1 ");
			break;
		case SOURCE_OTHER2:
			strcpy(szInfo, "Video : Other 2 ");
			break;
		case SOURCE_COMPVIASVIDEO:
			strcpy(szInfo, "Video : Composite via S-Video ");
			break;
		case SOURCE_CCIR656_1:
			strcpy(szInfo, "Video : CCIR656 1 ");
			break;
		case SOURCE_CCIR656_2:
			strcpy(szInfo, "Video : CCIR656 2 ");
			break;
		case SOURCE_CCIR656_3:
			strcpy(szInfo, "Video : CCIR656 3 ");
			break;
		case SOURCE_CCIR656_4:
			strcpy(szInfo, "Video : CCIR656 4 ");
			break;
		default:
			strcpy(szInfo, "Video : Unknown ");
			break;
		}
		strcat(szInfo, BT848_GetTVFormat()->szDesc);
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, dfSize));

		// Audio input + muting
		switch (AudioSource) {
		case 0:
			strcpy(szInfo, "Audio : Tuner");
			break;
		case 1:
			strcpy(szInfo, "Audio : MSP/Radio");
			break;
		case 2:
			strcpy(szInfo, "Audio : External");
			break;
		case 3:
			strcpy(szInfo, "Audio : Internal");
			break;
		case 4:
			strcpy(szInfo, "Audio : Disabled");
			break;
		case 5:
			strcpy(szInfo, "Audio : Stereo");
			break;
		default:
			strcpy(szInfo, "Audio : Unknown");
			break;
		}
		if (System_In_Mute == TRUE)
		{
			strcat (szInfo, " - MUTE");
		}
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, dfSize));

		// Pixel width
		sprintf (szInfo, "Pixel width : %u", Setting_GetValue(BT848_GetSetting(CURRENTX)));
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, dfSize));

		// Source ratio
		sprintf(szInfo, "Source %.3f", (double)Setting_GetValue(Aspect_GetSetting(SOURCE_ASPECT)) / 1000.0);

		if (strlen(szInfo) > 0)
		{
			if ( (Setting_GetValue(Aspect_GetSetting(ASPECT_MODE)) == 1)
			  && (Setting_GetValue(Aspect_GetSetting(SOURCE_ASPECT)) != 1333) )
			{
				strcat(szInfo, " Letterboxed");
			}
			else if (Setting_GetValue(Aspect_GetSetting(ASPECT_MODE)) == 2)
			{
			strcat(szInfo, " Anamorphic");
			}
			OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, dfSize));
		}

		// Display ratio
		sprintf(szInfo, "Display %.3f", (double)Setting_GetValue(Aspect_GetSetting(TARGET_ASPECT)) / 1000.0);
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, dfSize));

		// Video settings
		nLine = 2;
		sprintf (szInfo, "Brightness : %03u", Setting_GetValue(BT848_GetSetting(BRIGHTNESS)));
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, dfSize));
		sprintf (szInfo, "Contrast : %03u", Setting_GetValue(BT848_GetSetting(CONTRAST)));
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, dfSize));
		sprintf (szInfo, "Hue : %03u", Setting_GetValue(BT848_GetSetting(HUE)));
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, dfSize));
		sprintf (szInfo, "Color : %03u", Setting_GetValue(BT848_GetSetting(SATURATION)));
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, dfSize));
		sprintf (szInfo, "Color U : %03u", Setting_GetValue(BT848_GetSetting(SATURATIONU)));
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, dfSize));
		sprintf (szInfo, "Color V : %03u", Setting_GetValue(BT848_GetSetting(SATURATIONV)));
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, dfSize));

		// Deinterlace mode
		nLine = -1;
		if (Setting_GetValue(FD60_GetSetting(FALLBACKTOVIDEO)))
		{
			strcpy(szInfo, "Fallback on Bad Pulldown ON");
		}
		else
		{
			strcpy(szInfo, "Fallback on Bad Pulldown OFF");
		}
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine--, dfMargin, dfSize));
		if (Setting_GetValue(OutThreads_GetSetting(AUTODETECT)))
		{
			strcpy(szInfo, "Auto Pulldown Detect ON");
		}
		else
		{
			strcpy(szInfo, "Auto Pulldown Detect OFF");
		}
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine--, dfMargin, dfSize));
		OSD_AddText(DeinterlaceModeName(-1), dfSize, 0, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine--, dfMargin, dfSize));

		// Filters
		nLine = -1;
		if (Setting_GetValue(Filter_GetSetting(USETEMPORALNOISEFILTER)))
		{
			strcpy(szInfo, "Noise Filter ON");
		}
		else
		{
			strcpy(szInfo, "Noise Filter OFF");
		}
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine--, dfMargin, dfSize));
		if (Setting_GetValue(Filter_GetSetting(USEGAMMAFILTER)))
		{
			strcpy(szInfo, "Gamma Filter ON");
		}
		else
		{
			strcpy(szInfo, "Gamma Filter OFF");
		}
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine--, dfMargin, dfSize));
		break;

	// WSS DATA DECODING SCREEN
	case OSD_SCREEN_2:
		if (dfSize == 0)	dfSize = 4;	// 6% of screen height

		// Title
		OSD_AddText("WSS data decoding", 8, RGB(255,150,150), OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (1, dfMargin, 10));

		nLine = 3;

		OSD_AddText("Status", dfSize, RGB(150,150,255), OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, dfSize));

		sprintf (szInfo, "Decoding errors : %d", WSSNbDecodeErr);
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, dfSize));
		sprintf (szInfo, "Decoding OK : %d", WSSNbDecodeOk);
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, dfSize));
		if ((WSSNbDecodeErr+WSSNbDecodeOk) > 0)
		{
			sprintf (szInfo, "Last decode status : %s", WSSDecodeOk ? "OK" : "ERROR");		
			OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, dfSize));
		}

		if (WSSNbDecodeOk > 1)
		{

		nLine++;

		OSD_AddText("Data", dfSize, RGB(150,150,255), OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, dfSize));

		// WSS data
		if (WSSAspectMode == 1)
			strcpy (szInfo, "Aspect mode : non anamorphic");
		else if (WSSAspectMode == 2)
			strcpy (szInfo, "Aspect mode : anamorphic");
		else
			strcpy (szInfo, "Aspect mode : undefined");
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, dfSize));
		if (WSSAspectRatio > 0)
			sprintf (szInfo, "Aspect ratio : %.2f", WSSAspectRatio / 1000.0);
		else
			strcpy (szInfo, "Aspect ratio : undefined");
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, dfSize));
		sprintf (szInfo, "Mode : %s", WSSFilmMode ? "film mode" : "camera mode");		
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, dfSize));
		sprintf (szInfo, "Color encoding : %s", WSSColorPlus ? "Motion Adaptative ColorPlus" : "normal Pal");
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, dfSize));
		sprintf (szInfo, "Helper signals : %s", WSSHelperSignals ? "yes" : "no");		
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, dfSize));
		sprintf (szInfo, "Teletext subtitles : %s", WSSTeletextSubtitle ? "yes" : "no");		
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, dfSize));
		switch (WSSOpenSubtitles)
		{
		case WSS625_SUBTITLE_NO:
			strcpy (szInfo, "Open subtitles : no");
			break;
		case WSS625_SUBTITLE_INSIDE:
			strcpy (szInfo, "Open subtitles : inside active picture");
			break;
		case WSS625_SUBTITLE_OUTSIDE:
			strcpy (szInfo, "Open subtitles : outside active picture");
			break;
		default:
			strcpy (szInfo, "Open subtitles : ???");
			break;
		}
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, dfSize));
		sprintf (szInfo, "Surround sound : %s", WSSSurroundSound ? "yes" : "no");		
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, dfSize));
		sprintf (szInfo, "Copyright asserted : %s", WSSCopyrightAsserted ? "yes" : "no");		
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, dfSize));
		sprintf (szInfo, "Copy protection : %s", WSSCopyProtection ? "yes" : "no");		
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, dfSize));

		nLine++;

		OSD_AddText("Debug", dfSize, RGB(150,150,255), OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, dfSize));

		// Debug informations
		sprintf (szInfo, "Average start position : %d", WSSAvgPos);
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, dfSize));
		sprintf (szInfo, "Minimum start position : %d", WSSMinPos);
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, dfSize));
		sprintf (szInfo, "Maximum start position : %d", WSSMaxPos);
		OSD_AddText(szInfo, dfSize, 0, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, dfSize));
		}
		break;

	case OSD_SCREEN_3:
		break;

	case OSD_SCREEN_4:
		break;

	case OSD_SCREEN_5:
		break;

	case OSD_SCREEN_6:
		break;

	case OSD_SCREEN_7:
		break;

	case OSD_SCREEN_8:
		break;

	case OSD_SCREEN_9:
		break;

	case OSD_SCREEN_10:
		break;

	default:
		break;
	}

	OSD_Show(hWnd, FALSE);
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
