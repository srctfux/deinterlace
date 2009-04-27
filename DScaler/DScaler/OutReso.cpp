/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 Laurent Garnier  All rights reserved.
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
//
/////////////////////////////////////////////////////////////////////////////
//
// This module uses code from Kristian Trenskow provided as DScaler patch
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file OutReso.cpp Change output resolution functions
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "DScaler.h"
#include "OutReso.h"
#include "Setting.h"
#include "IOutput.h"
#include "DebugLog.h"
#include "Providers.h"


typedef struct
{
    BOOL    bSwitchScreen;
    int        intResWidth;
    int        intResHeight;
    int        intResDepth;
    int        intResFreq;
    BOOL    bSupported;
    // Menu related items
    int        intMenuPixelPos;
    int        intMenuDepthPos;
} sResolution;


static sResolution resSettings[] = {
    //  Do Switch    Width    Height    Depth    Frequency    Supported
    {    FALSE,        0,        0,        0,        0,            TRUE,        0,    0    },
    {    FALSE,        0,        0,        0,        0,            FALSE,        1,    0    },
    {    TRUE,        640,    480,    16,        60,            FALSE,        0,    0    },
    {    TRUE,        640,    480,    16,        72,            FALSE,        0,    0    },
    {    TRUE,        640,    480,    16,        75,            FALSE,        0,    0    },
    {    TRUE,        640,    480,    16,        100,        FALSE,        0,    0    },
    {    TRUE,        640,    480,    16,        120,        FALSE,        0,    0    },
    {    TRUE,        640,    480,    32,        60,            FALSE,        0,    0    },
    {    TRUE,        640,    480,    32,        72,            FALSE,        0,    0    },
    {    TRUE,        640,    480,    32,        75,            FALSE,        0,    0    },
    {    TRUE,        640,    480,    32,        100,        FALSE,        0,    0    },
    {    TRUE,        640,    480,    32,        120,        FALSE,        0,    0    },
    {    TRUE,        720,    480,    16,        60,            FALSE,        0,    0    },
    {    TRUE,        720,    480,    16,        72,            FALSE,        0,    0    },
    {    TRUE,        720,    480,    16,        75,            FALSE,        0,    0    },
    {    TRUE,        720,    480,    16,        100,        FALSE,        0,    0    },
    {    TRUE,        720,    480,    16,        120,        FALSE,        0,    0    },
    {    TRUE,        720,    480,    32,        60,            FALSE,        0,    0    },
    {    TRUE,        720,    480,    32,        72,            FALSE,        0,    0    },
    {    TRUE,        720,    480,    32,        75,            FALSE,        0,    0    },
    {    TRUE,        720,    480,    32,        100,        FALSE,        0,    0    },
    {    TRUE,        720,    480,    32,        120,        FALSE,        0,    0    },
    {    TRUE,        854,    480,    16,        60,            FALSE,        0,    0    },
    {    TRUE,        854,    480,    16,        72,            FALSE,        0,    0    },
    {    TRUE,        854,    480,    16,        75,            FALSE,        0,    0    },
    {    TRUE,        854,    480,    16,        100,        FALSE,        0,    0    },
    {    TRUE,        854,    480,    16,        120,        FALSE,        0,    0    },
    {    TRUE,        854,    480,    32,        60,            FALSE,        0,    0    },
    {    TRUE,        854,    480,    32,        72,            FALSE,        0,    0    },
    {    TRUE,        854,    480,    32,        75,            FALSE,        0,    0    },
    {    TRUE,        854,    480,    32,        100,        FALSE,        0,    0    },
    {    TRUE,        854,    480,    32,        120,        FALSE,        0,    0    },
    {    TRUE,        720,    576,    16,        60,            FALSE,        0,    0    },
    {    TRUE,        720,    576,    16,        72,            FALSE,        0,    0    },
    {    TRUE,        720,    576,    16,        75,            FALSE,        0,    0    },
    {    TRUE,        720,    576,    16,        100,        FALSE,        0,    0    },
    {    TRUE,        720,    576,    16,        120,        FALSE,        0,    0    },
    {    TRUE,        720,    576,    32,        60,            FALSE,        0,    0    },
    {    TRUE,        720,    576,    32,        72,            FALSE,        0,    0    },
    {    TRUE,        720,    576,    32,        75,            FALSE,        0,    0    },
    {    TRUE,        720,    576,    32,        100,        FALSE,        0,    0    },
    {    TRUE,        720,    576,    32,        120,        FALSE,        0,    0    },
    {    TRUE,        768,    576,    16,        60,            FALSE,        0,    0    },
    {    TRUE,        768,    576,    16,        72,            FALSE,        0,    0    },
    {    TRUE,        768,    576,    16,        75,            FALSE,        0,    0    },
    {    TRUE,        768,    576,    16,        100,        FALSE,        0,    0    },
    {    TRUE,        768,    576,    16,        120,        FALSE,        0,    0    },
    {    TRUE,        768,    576,    32,        60,            FALSE,        0,    0    },
    {    TRUE,        768,    576,    32,        72,            FALSE,        0,    0    },
    {    TRUE,        768,    576,    32,        75,            FALSE,        0,    0    },
    {    TRUE,        768,    576,    32,        100,        FALSE,        0,    0    },
    {    TRUE,        768,    576,    32,        120,        FALSE,        0,    0    },
    {    TRUE,        1024,    576,    16,        60,            FALSE,        0,    0    },
    {    TRUE,        1024,    576,    16,        72,            FALSE,        0,    0    },
    {    TRUE,        1024,    576,    16,        75,            FALSE,        0,    0    },
    {    TRUE,        1024,    576,    16,        100,        FALSE,        0,    0    },
    {    TRUE,        1024,    576,    16,        120,        FALSE,        0,    0    },
    {    TRUE,        1024,    576,    32,        60,            FALSE,        0,    0    },
    {    TRUE,        1024,    576,    32,        72,            FALSE,        0,    0    },
    {    TRUE,        1024,    576,    32,        75,            FALSE,        0,    0    },
    {    TRUE,        1024,    576,    32,        100,        FALSE,        0,    0    },
    {    TRUE,        1024,    576,    32,        120,        FALSE,        0,    0    },
    {    TRUE,        800,    600,    16,        60,            FALSE,        0,    0    },
    {    TRUE,        800,    600,    16,        72,            FALSE,        0,    0    },
    {    TRUE,        800,    600,    16,        75,            FALSE,        0,    0    },
    {    TRUE,        800,    600,    16,        100,        FALSE,        0,    0    },
    {    TRUE,        800,    600,    16,        120,        FALSE,        0,    0    },
    {    TRUE,        800,    600,    32,        60,            FALSE,        0,    0    },
    {    TRUE,        800,    600,    32,        72,            FALSE,        0,    0    },
    {    TRUE,        800,    600,    32,        75,            FALSE,        0,    0    },
    {    TRUE,        800,    600,    32,        100,        FALSE,        0,    0    },
    {    TRUE,        800,    600,    32,        120,        FALSE,        0,    0    },
    {    TRUE,        1280,    720,    16,        60,            FALSE,        0,    0    },
    {    TRUE,        1280,    720,    16,        72,            FALSE,        0,    0    },
    {    TRUE,        1280,    720,    16,        75,            FALSE,        0,    0    },
    {    TRUE,        1280,    720,    16,        100,        FALSE,        0,    0    },
    {    TRUE,        1280,    720,    16,        120,        FALSE,        0,    0    },
    {    TRUE,        1280,    720,    32,        60,            FALSE,        0,    0    },
    {    TRUE,        1280,    720,    32,        72,            FALSE,        0,    0    },
    {    TRUE,        1280,    720,    32,        75,            FALSE,        0,    0    },
    {    TRUE,        1280,    720,    32,        100,        FALSE,        0,    0    },
    {    TRUE,        1280,    720,    32,        120,        FALSE,        0,    0    },
    {    TRUE,        1024,    768,    16,        60,            FALSE,        0,    0    },
    {    TRUE,        1024,    768,    16,        72,            FALSE,        0,    0    },
    {    TRUE,        1024,    768,    16,        75,            FALSE,        0,    0    },
    {    TRUE,        1024,    768,    16,        100,        FALSE,        0,    0    },
    {    TRUE,        1024,    768,    16,        120,        FALSE,        0,    0    },
    {    TRUE,        1024,    768,    32,        60,            FALSE,        0,    0    },
    {    TRUE,        1024,    768,    32,        72,            FALSE,        0,    0    },
    {    TRUE,        1024,    768,    32,        75,            FALSE,        0,    0    },
    {    TRUE,        1024,    768,    32,        100,        FALSE,        0,    0    },
    {    TRUE,        1024,    768,    32,        120,        FALSE,        0,    0    },
};


int OutputReso = 0;
SettingStringValue PStrip576i;
SettingStringValue PStrip480i;


/////////////////////////////////////////////////////////////////////////////
// Start of Menus related code
/////////////////////////////////////////////////////////////////////////////

void OutReso_UpdateMenu(HMENU hMenu)
{
    HMENU    hMenuReso;
    HMENU   hMenuPixel;
    HMENU   hMenuDepth;

    int        lastWidth = 1;
    int        lastHeight = 1;
    int        lastDepth = 1;
    char    szTmp[20] = "\0";

    int        i, j, n, pixel = 2, depth = 0;

    hMenuReso = GetOutResoSubmenu();
    if (hMenuReso == NULL)
    {
        return;
    }

    // Add "Do nothing" and seperator

    AppendMenu(hMenuReso, MF_STRING, IDM_OUTPUTRESO, "Don't change");
    AppendMenu(hMenuReso, MF_SEPARATOR, 0, NULL);
    j = 1;

    // Add "Use PowerStrip resolution"

    AppendMenu(hMenuReso, MF_STRING, IDM_OUTPUTRESO + 1, "Use PowerStrip resolution");
    resSettings[1].bSupported = TRUE;
    pixel++;
    j++;

    // Add in menus only supported display settings

    n = sizeof (resSettings) / sizeof (resSettings[0]);
    for (i=2; i < n ; i++)
    {
        if (resSettings[i].bSwitchScreen)
        {
            DEVMODE dm;
            dm.dmSize = sizeof(DEVMODE);
            dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
            dm.dmPelsWidth = resSettings[i].intResWidth;
            dm.dmPelsHeight = resSettings[i].intResHeight;
            dm.dmBitsPerPel = resSettings[i].intResDepth;
            dm.dmDisplayFrequency = resSettings[i].intResFreq;
            if (ChangeDisplaySettings(&dm, CDS_TEST) != DISP_CHANGE_SUCCESSFUL)
            {
                resSettings[i].bSupported = FALSE;
            }
            else
            {
                resSettings[i].bSupported = TRUE;
            }
        }

        if (resSettings[i].bSupported)
        {
            UINT Flags(MF_STRING);

            if (lastWidth != resSettings[i].intResWidth  ||  lastHeight != resSettings[i].intResHeight)
            {
                sprintf(szTmp, "%dx%d", resSettings[i].intResWidth, resSettings[i].intResHeight);
                hMenuPixel = CreateMenu();
                InsertMenu(hMenuReso, -1, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT) hMenuPixel, szTmp);
                pixel++;
                depth = 0;
                lastDepth = 0;
            }

            if (lastDepth != resSettings[i].intResDepth)
            {
                sprintf(szTmp, "%d bit", resSettings[i].intResDepth);
                hMenuDepth = CreateMenu();
                InsertMenu(hMenuPixel, -1, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT) hMenuDepth, szTmp);
                depth++;
            }

            sprintf(szTmp, "%d Hz", resSettings[i].intResFreq);
            AppendMenu(hMenuDepth, Flags, IDM_OUTPUTRESO + j, szTmp);
            j++;

            lastWidth = resSettings[i].intResWidth;
            lastHeight = resSettings[i].intResHeight;
            lastDepth = resSettings[i].intResDepth;

            resSettings[i].intMenuPixelPos = pixel - 1;
            resSettings[i].intMenuDepthPos = depth - 1;

            // Stop as soon as MAX_NUMBER_RESO is reached
            if (j >= MAX_NUMBER_RESO)
            {
                break;
            }
        }
    }

    // Update the value for the RESOFULLSCREEN setting to "none"
    // if the current value is greater than the number of items in menu
    if (Setting_GetValue(WM_DSCALER_GETVALUE, RESOFULLSCREEN) > (j-1))
    {
        Setting_SetValue(WM_DSCALER_GETVALUE, RESOFULLSCREEN, 0);
    }
}


void OutReso_SetMenu(HMENU hMenu)
{
    HMENU   hMenuReso;
    int     i, j, n, selected;

    hMenuReso = GetOutResoSubmenu();
    if (hMenuReso == NULL)
    {
        return;
    }

    n = sizeof (resSettings) / sizeof (resSettings[0]);
    for (i=0,j=0; i < n ; i++)
    {
        if (resSettings[i].bSupported)
        {
            CheckMenuItem(hMenuReso, resSettings[i].intMenuPixelPos, MF_UNCHECKED | MF_BYPOSITION);
            HMENU hMenuPixel = GetSubMenu(hMenuReso, resSettings[i].intMenuPixelPos);
            CheckMenuItem(hMenuPixel, resSettings[i].intMenuDepthPos, MF_UNCHECKED | MF_BYPOSITION);
            CheckMenuItem(hMenuReso, IDM_OUTPUTRESO + j, MF_UNCHECKED);
            if (j==OutputReso)
            {
                selected = i;
            }

            j++;
            // Stop as soon as MAX_NUMBER_RESO is reached
            if (j >= MAX_NUMBER_RESO)
            {
                break;
            }
        }
    }

    CheckMenuItem(hMenuReso, IDM_OUTPUTRESO + OutputReso, MF_CHECKED);
    if (OutputReso>1)
    {
        CheckMenuItem(hMenuReso, resSettings[selected].intMenuPixelPos, MF_CHECKED | MF_BYPOSITION);
        HMENU hMenuPixel = GetSubMenu(hMenuReso, resSettings[selected].intMenuPixelPos);
        CheckMenuItem(hMenuPixel, resSettings[selected].intMenuDepthPos, MF_CHECKED | MF_BYPOSITION);
    }
}


BOOL ProcessOutResoSelection(HWND hWnd, WORD wMenuID)
{
    if (wMenuID >= IDM_OUTPUTRESO && wMenuID < (IDM_OUTPUTRESO + MAX_NUMBER_RESO))
    {
        OutputReso = wMenuID - IDM_OUTPUTRESO;
        return TRUE;
    }
    return FALSE;
}

void OutReso_Change(HWND hWnd, HWND hPSWnd, BOOL bUseRegistrySettings, BOOL bCaptureRunning, LPSTR lTimingString, BOOL bApplyPStripTimingString)
{
    if (OutputReso == 0)
        return;

    if(!hPSWnd && (OutputReso == 1))
        return;

    // If PowerStrip has been found, use it
    if ( hPSWnd
      && ( (OutputReso == 1)
        || ( (OutputReso > 1) && bApplyPStripTimingString) ) )
    {
        BOOL changeRes = FALSE;

        // Get the actual PStrip timing string
        ATOM pActualPStripTimingString = (ATOM)SendMessage(hPSWnd, UM_GETPSTRIPTIMING, 0, 0);
        LPSTR lActualPStripTimingString = new char[PSTRIP_TIMING_STRING_SIZE];
        GlobalGetAtomName(pActualPStripTimingString, lActualPStripTimingString, PSTRIP_TIMING_STRING_SIZE);
        GlobalDeleteAtom(pActualPStripTimingString);

        ATOM aPStripTimingATOM = NULL;
        if((lTimingString != NULL) && (bApplyPStripTimingString))
        {
            if(lTimingString != lActualPStripTimingString)
            {
                aPStripTimingATOM = GlobalAddAtom(lTimingString);
                changeRes = TRUE;
            }
        }
        else if (Providers_GetCurrentSource())
        {
            // Get the video format
            eVideoFormat videoFormat = Providers_GetCurrentSource()->GetFormat();

            // 576i_50Hz and 576i_60Hz
            if((videoFormat == VIDEOFORMAT_PAL_B) || (videoFormat == VIDEOFORMAT_PAL_D) || (videoFormat == VIDEOFORMAT_PAL_G) || (videoFormat == VIDEOFORMAT_PAL_H)
                || (videoFormat == VIDEOFORMAT_PAL_I) || (videoFormat == VIDEOFORMAT_PAL_M) || (videoFormat == VIDEOFORMAT_PAL_N)
                || (videoFormat == VIDEOFORMAT_PAL_60) || (videoFormat == VIDEOFORMAT_PAL_N_COMBO))
            {
                if(PStrip576i && _stricmp(PStrip576i, lActualPStripTimingString) != 0)
                {
                    aPStripTimingATOM = GlobalAddAtom(PStrip576i);
                    changeRes = TRUE;
                }
            }
            // 480i_50Hz and 480i_60Hz
            else if((videoFormat == VIDEOFORMAT_NTSC_M) || (videoFormat == VIDEOFORMAT_NTSC_M_Japan) || (videoFormat == VIDEOFORMAT_NTSC_50))
            {
                if(PStrip480i && _stricmp(PStrip480i, lActualPStripTimingString) != 0)
                {
                    aPStripTimingATOM = GlobalAddAtom(PStrip480i);
                    changeRes = TRUE;
                }
            }
        }

        if(changeRes == TRUE)
        {
            BOOL bOverlay = GetActiveOutput()->OverlayActive();

            // Stop the overlay (and the capture)
            if (bOverlay)
            {
                if (bCaptureRunning)
                {
                    Overlay_Stop(hWnd);
                }
                else
                {
                    GetActiveOutput()->Overlay_Destroy();
                }
            }

            // Apply the PowerStrip timing string
            // If the PostMessage is successfull, the Atom is automatically deleted
            if(aPStripTimingATOM != NULL)
            {
                if(!SendMessage(hPSWnd, UM_SETPSTRIPTIMING, 0, aPStripTimingATOM))
                {
                    GlobalDeleteAtom(aPStripTimingATOM);
                }
            }

            // Restart the overlay (and the capture)
            if (bOverlay)
            {
                if (bCaptureRunning)
                {
                    Overlay_Start(hWnd);
                }
                else
                {
                    GetActiveOutput()->Overlay_Create();
                }
            }
        }
        delete lActualPStripTimingString;
    }
    else
    {
        DEVMODE dm;
        DEVMODE dm_cur;
        int     i, idx, n;

        n = sizeof (resSettings) / sizeof (resSettings[0]);
        for (idx=0,i=0; idx < n ; idx++)
        {
            if (resSettings[idx].bSupported)
            {
                if (i == OutputReso)
                {
                    break;
                }
                i++;
            }
        }

        if (resSettings[idx].bSwitchScreen)
        {
            dm.dmSize = sizeof(DEVMODE);
            dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
            if (bUseRegistrySettings)
            {
                // Get the display settings from registry
                EnumDisplaySettings(NULL, ENUM_REGISTRY_SETTINGS, &dm);
            }
            else
            {
                // Use the display settings defined by the user
                dm.dmPelsWidth = resSettings[idx].intResWidth;
                dm.dmPelsHeight = resSettings[idx].intResHeight;
                dm.dmBitsPerPel = resSettings[idx].intResDepth;
                dm.dmDisplayFrequency = resSettings[idx].intResFreq;
            }

            // Change display settings only if different from current
            dm_cur.dmSize = sizeof(DEVMODE);
            dm_cur.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
            EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm_cur);
            if ( (dm.dmPelsWidth != dm_cur.dmPelsWidth)
              || (dm.dmPelsHeight != dm_cur.dmPelsHeight)
              || (dm.dmBitsPerPel != dm_cur.dmBitsPerPel)
              || (dm.dmDisplayFrequency != dm_cur.dmDisplayFrequency))
            {
                BOOL bOverlay = GetActiveOutput()->OverlayActive();

                // Stop the overlay (and the capture)
                if (bOverlay)
                {
                    if (bCaptureRunning)
                    {
                        Overlay_Stop(hWnd);
                    }
                    else
                    {
                        GetActiveOutput()->Overlay_Destroy();
                    }
                }

    //            ShowWindow(hWnd, SW_HIDE);
                ChangeDisplaySettings(&dm, 0);
    //            ShowWindow(hWnd, SW_SHOW);

                // Restart the overlay (and the capture)
                if (bOverlay)
                {
                    if (bCaptureRunning)
                    {
                        Overlay_Start(hWnd);
                    }
                    else
                    {
                        GetActiveOutput()->Overlay_Create();
                    }
                }
            }
        }
    }
}