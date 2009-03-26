/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
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
 * @file deinterlace.h deinterlace Header file
 */
 
#ifndef __DEINTERLACE_H___
#define __DEINTERLACE_H___

#include "settings.h"
#include "DS_Deinterlace.h"


// Get Hold of the Deinterlace.c file settings
void Deinterlace_SetMenu(HMENU hMenu);
void GetDeinterlaceSettings(std::vector< SmartPtr<CSettingsHolder> >& Holders,std::vector< std::string >& Names);



// Deinterlace Modes.  Since these Modes are referred to by number in the
// INI file, it's desirable to keep the numbers consistent between releases.
// Otherwise users will end up in the wrong Modes when they upgrade.  If
// you renumber or add/remove Modes, be sure to update inifile.htm, which
// documents the Mode IDs!
enum eFilmPulldownMode
{
    FILM_22_PULLDOWN_ODD = 0,
    FILM_22_PULLDOWN_EVEN = 1,
    FILM_32_PULLDOWN_0 = 2,
    FILM_32_PULLDOWN_1 = 3,
    FILM_32_PULLDOWN_2 = 4,
    FILM_32_PULLDOWN_3 = 5,
    FILM_32_PULLDOWN_4 = 6,
    FILM_32_PULLDOWN_COMB = 7,
    FILM_22_PULLDOWN_COMB = 8,
    FILMPULLDOWNMODES_LAST_ONE = 9
};

enum eProgressivePulldownMode
{
    PROG_22_PULLDOWN_ODD = 0,
    PROG_22_PULLDOWN_EVEN,
    PROG_32_PULLDOWN_0,
    PROG_32_PULLDOWN_1,
    PROG_32_PULLDOWN_2,
    PROG_32_PULLDOWN_3,
    PROG_32_PULLDOWN_4,
    NORMAL_PROGRESSIVE,
    PROGPULLDOWNMODES_LAST_ONE
};


#define MAX_FIELD_HISTORY 5

BOOL LoadDeinterlacePlugins();
void UnloadDeinterlacePlugins();

void ResetDeinterlaceStats();

DEINTERLACE_METHOD* GetCurrentDeintMethod();
DEINTERLACE_METHOD* GetProgressiveMethod(int Mode);
DEINTERLACE_METHOD* GetVideoDeintMethod(int Mode);
DEINTERLACE_METHOD* GetVideoDeintIndex(int Index);
DEINTERLACE_METHOD* GetFilmDeintMethod(eFilmPulldownMode Mode);
BOOL ProcessDeinterlaceSelection(HWND hWnd, WORD wMenuID);

BOOL IsProgressiveMode();
BOOL IsFilmMode();
eFilmPulldownMode GetFilmMode();
eProgressivePulldownMode GetProgMode();
BOOL InHalfHeightMode();
BOOL SetProgressiveMode();
BOOL UnsetProgressiveMode();
void SetFilmDeinterlaceMode(eFilmPulldownMode Mode);
void SetVideoDeinterlaceIndex(int index);
void SetVideoDeinterlaceMode(int Mode);
char* GetDeinterlaceModeName();
void IncrementDeinterlaceMode();
void DecrementDeinterlaceMode();
void PrepareDeinterlaceMode();

void ShowVideoModeUI();

BOOL CopyFrame(TDeinterlaceInfo* pInfo);

extern const char* DeinterlaceNames[100];

// Statistics
extern long nInitialTicks;
extern long nLastTicks;
extern long nTotalDeintModeChanges;

#endif
