/////////////////////////////////////////////////////////////////////////////
// AspectRatio.h
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
// Aspect ratio contrl was started by Michael Samblanet <mike@cardobe.com>
// Moved into separate module by Mark D Rejhon.  
//
// The purpose of this module is all the calculations and handling necessary
// to map the source image onto the destination display, even if they are
// different aspect ratios.
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 12 Sep 2000   Mark Rejhon           Centralized aspect ratio code
//                                     into separate module
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _ASPECT_H_
#define _ASPECT_H_ 1

#include "settings.h"

typedef enum
{
	OVERSCAN,
	SOURCE_ASPECT,
	CUSTOM_SOURCE_ASPECT,
	TARGET_ASPECT,
	CUSTOM_TARGET_ASPECT,
	ASPECT_MODE,
	LUMINANCETHRESHOLD,
	IGNORENONBLACKPIXELS,
	AUTODETECTASPECT,
	ZOOMINFRAMECOUNT,
	ASPECTHISTORYTIME,
	ASPECTCONSISTENCYTIME,
	VERTICALPOS,
	HORIZONTALPOS,
	ASPECT_SETTING_LASTONE,
} ASPECT_SETTING;

// Get Hold of the AspectRatio.c file settings
SETTING* Aspect_GetSetting(ASPECT_SETTING Setting);
void Aspect_ReadSettingsFromIni();
void Aspect_WriteSettingsToIni();

#define DEFAULT_OVERSCAN 4

#define ABS(x) ((x) < 0 ? -(x) : (x))

typedef enum
{
	VERT_POS_CENTRE = 0,
	VERT_POS_TOP,
	VERT_POS_BOTTOM,
} VERT_POS;

typedef enum
{
	HORZ_POS_CENTRE = 0,
	HORZ_POS_LEFT,
	HORZ_POS_RIGHT,
} HORZ_POS;

int     ProcessAspectRatioSelection(HWND hWnd, WORD wMenuID);
void    AspectRatio_SetMenu(HMENU hMenu);
void    WorkoutOverlaySize();
void    PaintColorkey(HWND hWnd, BOOL bEnable);
int		FindAspectRatio(short** EvenField, short** OddField);
void	AdjustAspectRatio(short** EvenField, short** OddField);
void	SetHalfHeight(int IsHalfHeight);
void	GetSourceRect(RECT *rect);

#endif