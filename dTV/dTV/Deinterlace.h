/////////////////////////////////////////////////////////////////////////////
// deinterlace.h
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
// 24 Jul 2000   John Adcock           Put all my deinterlacing code into this
//                                     file
//
// 05 Jan 2001   John Adcock           Added flip frequencies to DeintMethods
//
// 07 Jan 2001   John Adcock           Added Adaptive deinterlacing method
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __DEINTERLACE_H___
#define __DEINTERLACE_H___

#include "settings.h"

// Get Hold of the Deinterlace.c file settings
SETTING* Deinterlace_GetSetting(DEINTERLACE_SETTING Setting);
void Deinterlace_ReadSettingsFromIni();
void Deinterlace_WriteSettingsToIni();
void Deinterlace_SetMenu(HMENU hMenu);



// Deinterlace modes.  Since these modes are referred to by number in the
// INI file, it's desirable to keep the numbers consistent between releases.
// Otherwise users will end up in the wrong modes when they upgrade.  If
// you renumber or add/remove modes, be sure to update inifile.htm, which
// documents the mode IDs!
typedef enum
{
	VIDEO_MODE_BOB = 0,
	VIDEO_MODE_WEAVE = 1,
	VIDEO_MODE_2FRAME = 2,
	SIMPLE_WEAVE = 3,
	SIMPLE_BOB = 4,
	SCALER_BOB = 5,
	FILM_22_PULLDOWN_ODD = 6,
	FILM_22_PULLDOWN_EVEN = 7,
	FILM_32_PULLDOWN_0 = 8,
	FILM_32_PULLDOWN_1 = 9,
	FILM_32_PULLDOWN_2 = 10,
	FILM_32_PULLDOWN_3 = 11,
	FILM_32_PULLDOWN_4 = 12,
	EVEN_ONLY = 13,
	ODD_ONLY = 14,
	BLENDED_CLIP = 15,
	ADAPTIVE = 16,
	GREEDY = 17,
	PULLDOWNMODES_LAST_ONE = 18
} ePULLDOWNMODES;

#define MAX_FIELD_HISTORY 5

/////////////////////////////////////////////////////////////////////////////
// Describes the inputs to a deinterlacing algorithm.  Some of these values
// are also available in global variables, but they're duplicated here in
// anticipation of eventually supporting deinterlacing plugins.
typedef struct {
	// Data from the most recent several odd and even fields, from newest
	// to oldest, i.e., OddLines[0] is always the most recent odd field.
	// Pointers are NULL if the field in question isn't valid, e.g. because
	// the program just started or a field was skipped.
	short **OddLines[MAX_FIELD_HISTORY];
	short **EvenLines[MAX_FIELD_HISTORY];

	// Current overlay buffer pointer.
	BYTE *Overlay;

	// The part of the overlay that we actually show
	RECT SourceRect;

	// True if the most recent field is an odd one; false if it was even.
	BOOL IsOdd;

	// which frame are we on
	int CurrentFrame;

	// Overlay pitch (number of bytes between scanlines).
	DWORD OverlayPitch;

	// Number of bytes of actual data in each scanline.  May be less than
	// OverlayPitch since the overlay's scanlines might have alignment
	// requirements.  Generally equal to FrameWidth * 2.
	DWORD LineLength;

	// Number of pixels in each scanline.
	int FrameWidth;

	// Number of scanlines per frame.
	int FrameHeight;

	// Number of scanlines per field.  FrameHeight / 2, mostly for
	// cleanliness so we don't have to keep dividing FrameHeight by 2.
	int FieldHeight;

	// Results from the NTSC Field compare
	long FieldDiff;
	// Results of the PAL mode deinterlace detect
	long CombFactor;

} DEINTERLACE_INFO;

// Deinterlace functions return true if the overlay is ready to be displayed.
typedef BOOL (DEINTERLACE_FUNC)(DEINTERLACE_INFO *info);

typedef struct
{
	// What to display when selected
	char* szName;
	// What to display when used in adaptive mode (NULL to use szName)
	char* szAdaptiveName;
	// Do we need to shrink the overlay by half
	BOOL bIsHalfHeight;
	// Is this a film mode
	BOOL bIsFilmMode;
    // Pointer to Algorithm function (cannot be NULL)
    DEINTERLACE_FUNC* pfnAlgorithm;
	// flip frequency in 50Hz mode
	unsigned long FrameRate50Hz;
	// flip frequency in 60Hz mode
	unsigned long FrameRate60Hz;
} DEINTERLACE_METHOD;

extern DEINTERLACE_METHOD DeintMethods[PULLDOWNMODES_LAST_ONE];

void memcpyMMX(void *Dest, void *Src, size_t nBytes);

#ifdef USE_SSE
void memcpySSE(void *Dest, void *Src, size_t nBytes);
#endif

extern char* DeintModeNames[PULLDOWNMODES_LAST_ONE];

#endif
