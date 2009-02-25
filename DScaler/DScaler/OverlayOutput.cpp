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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.10  2007/12/14 19:31:47  adcockj
// Fixes for Vista crashing
// Consistent exception handling as references
//
// Revision 1.9  2007/07/27 00:49:03  robmuller
// Reduce cpu usage (and hopefully dropped frames) when waiting for the vsync.
//
// Revision 1.8  2007/02/19 14:48:50  adcockj
// Fixed various issues with d3d9 code and settings
//
// Revision 1.7  2007/02/19 10:13:45  adcockj
// Fixes for Critical thread and RECT issuesin D3D9 and overlay code
//
// Revision 1.6  2007/02/19 03:09:21  robmuller
// Fix: overlay settings in Advanced Settings dialog did not work.
//
// Revision 1.5  2007/02/18 20:16:12  robmuller
// Applied coding standards.
//
// Revision 1.4  2007/02/18 15:00:37  robmuller
// Added CVS log.
//
//
//
/////////////////////////////////////////////////////////////////////////////
//
// The name of this file used to be "other.cpp".
// The revision log of "other.cpp" follows:
//
//
// Revision 1.76  2006/09/24 03:47:19  robmuller
// Stop potential endless loop.
//
// Revision 1.75  2006/09/24 03:14:05  robmuller
// Change number of backbuffers in real-time.
//
// Revision 1.74  2005/03/23 14:20:58  adcockj
// Test fix for threading issues
//
// Revision 1.73  2005/03/11 17:16:40  adcockj
// ifdefed out stuff that required newer sdk
//
// Revision 1.72  2005/03/11 13:03:08  adcockj
// handle MCE remote buttons for teletext
//
// Revision 1.71  2005/03/10 22:30:54  adcockj
// fixed some bugs with MCE remote support
//
// Revision 1.70  2005/03/10 17:40:40  adcockj
// first go at adding MCE remote support
//
// Revision 1.69  2004/05/06 15:00:43  adcockj
// Fix for errors on ctrl-alt-delete (Bug-947656)
//
// Revision 1.68  2004/05/02 14:09:32  atnak
// Fixed possible problem of overlay colour getting dithered with < 32bit colour
//
// Revision 1.67  2003/10/27 10:39:52  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.66  2003/07/18 09:38:00  adcockj
// Added some unused test code for flip timing
//
// Revision 1.65  2003/05/31 11:38:17  laurentg
// Load dynamic functions earlier to have splash screen on correct screen and to be able to start in full screen mode on the second monitor
//
// Revision 1.64  2003/04/15 13:06:27  adcockj
// Corrections for NT 4 compatable Multi-Monitor code
//
// Revision 1.63  2003/04/08 14:17:23  adcockj
// Used dynamic calling of new multi-monitor functions to allow running on NT4
//
// Revision 1.62  2003/03/29 13:36:36  laurentg
// Allow the display of DScaler to monitors other than the primary
//
// Revision 1.61  2003/03/16 18:29:20  laurentg
// New multiple frames feature
//
// Revision 1.60  2003/03/05 13:54:55  adcockj
// Use optimized memcpy in output filter copy
//
// Revision 1.59  2003/02/17 11:39:00  adcockj
// Added group flags for setting per channel on more settings
//
// Revision 1.58  2003/01/26 18:03:04  laurentg
// Problem with initial values for overlay settings fixed
//
// Revision 1.57  2003/01/25 20:54:29  robmuller
// Another bug fix from Kevin Radke.
//
// Revision 1.56  2003/01/25 17:50:37  robmuller
// Bug fix from Kevin Radke.
//
// Revision 1.55  2003/01/24 01:55:18  atnak
// OSD + Teletext conflict fix, offscreen buffering for OSD and Teletext,
// got rid of the pink overlay colorkey for Teletext.
//
// Revision 1.54  2003/01/11 15:22:26  adcockj
// Interim Checkin of setting code rewrite
//  - Remove CSettingsGroupList class
//  - Fixed bugs in format switching
//  - Some new CSettingGroup code
//
// Revision 1.53  2003/01/10 17:38:10  adcockj
// Interrim Check in of Settings rewrite
//  - Removed SETTINGSEX structures and flags
//  - Removed Seperate settings per channel code
//  - Removed Settings flags
//  - Cut away some unused features
//
// Revision 1.52  2003/01/02 20:41:46  adcockj
// Fixed very silly problem with new code
//
// Revision 1.51  2003/01/02 19:03:07  adcockj
// Removed extra Surface and replaced with memory buffer due to lack of blt support
//  and alignment problems
//
// Revision 1.50  2003/01/02 17:27:05  adcockj
// Improvements to extra surface code
//
// Revision 1.49  2003/01/02 16:22:09  adcockj
// Preliminary code to support output plugins properly
//
// Revision 1.48  2003/01/01 20:56:46  atnak
// Updates for various VideoText changes
//
// Revision 1.47  2002/10/16 16:08:54  tobbej
// spelling error
//
// Revision 1.46  2002/09/26 16:26:26  adcockj
// Fix for some old cards that do not support waitinf for flip
//
// Revision 1.45  2002/08/15 14:16:18  kooiman
// Cleaner settings per channel implementation
//
// Revision 1.44  2002/08/09 08:31:15  kooiman
// Changed default value for settings in the channel setting list.
//
// Revision 1.43  2002/08/08 12:49:33  kooiman
// Added Overlay settings to settings per channel.
//
// Revision 1.42  2002/08/07 21:53:04  adcockj
// Removed todo item
//
// Revision 1.41  2002/07/19 10:04:26  laurentg
// LOG in double suppressed
//
// Revision 1.40  2002/07/03 00:41:14  laurentg
// Add the setting "Use Overlay Controls" in the Change Settings dialog box
//
// Revision 1.39  2002/06/16 10:05:09  adcockj
// Commented out primary suface locks awaiting tests
//
// Revision 1.38  2002/06/13 12:10:22  adcockj
// Move to new Setings dialog for filers, video deint and advanced settings
//
// Revision 1.37  2002/04/27 11:46:28  laurentg
// Error messge in UpdateOverlay restored
//
// Revision 1.36  2002/04/13 21:52:40  laurentg
// Management of no current source
//
// Revision 1.35  2002/03/21 10:26:00  robmuller
// Added CanDoOverlayColorControl().
//
// Revision 1.34  2002/02/09 13:04:09  laurentg
// Propose in "Other settings" UI only the overlay settings which cannot be setup in another dialog box
//
// Revision 1.33  2002/01/26 18:04:29  laurentg
// Locking and unlocking the overlay and not the overlay back buffer when taking stills
//
// Revision 1.32  2002/01/19 11:56:26  robmuller
// Fixed overlay error 80004001 on startup with 3dfx cards.
//
// Revision 1.31  2002/01/12 16:57:02  adcockj
// POssible fix for 80070057 error on create
//
// Revision 1.30  2001/12/16 17:04:37  adcockj
// Debug Log improvements
//
// Revision 1.29  2001/11/28 16:04:50  adcockj
// Major reorganization of STill support
//
// Revision 1.28  2001/11/26 13:02:27  adcockj
// Bug Fixes and standards changes
//
// Revision 1.27  2001/11/25 21:29:50  laurentg
// Take still, Open file, Close file callbacks updated
//
// Revision 1.26  2001/11/25 10:41:26  laurentg
// TIFF code moved from Other.cpp to TiffSource.c + still capture updated
//
// Revision 1.25  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.24  2001/11/21 15:21:39  adcockj
// Renamed DEINTERLACE_INFO to TDeinterlaceInfo in line with standards
// Changed TDeinterlaceInfo structure to have history of pictures.
//
// Revision 1.23  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.22  2001/10/30 20:54:28  koreth
// Don't retry fetching physical overlay color if it's supposed to be zero.
//
// Revision 1.21  2001/10/17 11:46:11  adcockj
// Bug fixes
//
// Revision 1.20  2001/09/21 20:47:12  laurentg
// SaveStill modified to return the name of the written file
// Name of the file added in the OSD text when doing a snapshot
//
// Revision 1.19  2001/09/19 10:05:50  adcockj
// Updated feature help
//
// Revision 1.18  2001/09/09 17:41:08  adcockj
// Fixed bug in sharpness code
//
// Revision 1.17  2001/09/05 21:05:29  adcockj
// Bug Fixes for new overlay code
//
// Revision 1.16  2001/09/05 15:07:48  adcockj
// Wrapped overlay calls with critical section
// Updated Loging
//
// Revision 1.15  2001/09/02 14:17:51  adcockj
// Improved teletext code
//
// Revision 1.14  2001/08/13 18:54:55  adcockj
// Tidied up surface blanking code
//
// Revision 1.13  2001/08/11 11:17:57  adcockj
// Fixed problems when using zero backbuffers
//
// Revision 1.12  2001/08/02 16:43:05  adcockj
// Added Debug level to LOG function
//
// Revision 1.11  2001/07/28 13:24:40  adcockj
// Added UI for Overlay Controls and fixed issues with SettingsDlg
//
// Revision 1.10  2001/07/27 12:30:09  adcockj
// Added Overlay Color controls (Thanks to Muljadi Budiman)
//
// Revision 1.9  2001/07/16 18:07:50  adcockj
// Added Optimisation parameter to ini file saving
//
// Revision 1.8  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.7  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "overlayoutput.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "DScaler.h"
#include "OutThreads.h"
#include "VBI_VideoText.h"
#include "ErrorBox.h"
#include "Splash.h"
#include "DebugLog.h"
#include "AspectRatio.h"
#include "SettingsPerChannel.h"
//#include <dxerr9.h>

// cope with older DX header files
#if !defined(DDFLIP_DONOTWAIT)
    #define DDFLIP_DONOTWAIT 0
#endif

// the instance of the overlay object
COverlayOutput OverlayOutputInstance;

//-----------------------------------------------------------------------------
// Callback function used by DirectDrawEnumerateEx to find all monitors
BOOL WINAPI COverlayOutput::DDEnumCallbackEx(GUID* pGuid, LPTSTR pszDesc, LPTSTR pszDriverName,
							 VOID* pContext, HMONITOR hMonitor )
{
	MONITORINFO MonInfo;
	LPDIRECTDRAW lpDD;
	
	if (NbMonitors == MAX_MONITORS)
		return DDENUMRET_CANCEL;

	// DirectDrawEnumerateEx returns hMonitor = NULL on single monitor configuration 
	// and both NULL and non-NULL value for the primary monitor in multiple monitors context !
	// However MonitorFromWindow/Rect functions always return non-NULL HMONITOR handles
	// so we need to replace the NULL handle in single monitor context with the non-NULL value
	if (hMonitor == NULL)
	{
		hMonitor = OverlayOutputInstance.lpMonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
	}	
	LOG(2, "Monitor %d %s %s", hMonitor, pszDesc, pszDriverName);

	// and therefore test if we found again the same monitor !
	for (int i = 0; i < NbMonitors; i++)
	{
		if (Monitors[i].hMon == hMonitor)
		{
			LOG(2, "Monitor alrady listed");
			return DDENUMRET_OK;
		}
	}

	MonInfo.cbSize = sizeof(MONITORINFO);
	if (OverlayOutputInstance.lpGetMonitorInfoA(hMonitor, &MonInfo))
	{
		if (SUCCEEDED(OverlayOutputInstance.lpDirectDrawCreate(pGuid, &lpDD, NULL)))
		{
			Monitors[NbMonitors].hMon = hMonitor;
			Monitors[NbMonitors].lpDD = lpDD;
			NbMonitors++;
			LOG(1, "Monitor %d (%d %d %d %d)", NbMonitors, MonInfo.rcMonitor.left, MonInfo.rcMonitor.right, MonInfo.rcMonitor.top, MonInfo.rcMonitor.bottom);
		}
	}
	
    return DDENUMRET_OK; // Keep enumerating
}

//-----------------------------------------------------------------------------
BOOL COverlayOutput::ListMonitors(HWND hWnd)
{
    BOOL RetVal = TRUE;

	HINSTANCE h = LoadLibrary("ddraw.dll");

    // If ddraw.dll doesn't exist in the search path,
    // then DirectX probably isn't installed, so fail.
    if(h != NULL)
    {
	    // Retrieve the function from the DDL
        LPDIRECTDRAWENUMERATEEX lpDDEnumEx;
        lpDDEnumEx = (LPDIRECTDRAWENUMERATEEX) GetProcAddress(h,"DirectDrawEnumerateExA");
        if (lpDDEnumEx)
	    {
		    // If the function is there, call it to enumerate all display 
		    // devices attached to the desktop
		    if(lpDDEnumEx(DDEnumCallbackEx, NULL, DDENUM_ATTACHEDSECONDARYDEVICES) != DD_OK)
            {
                RetVal = FALSE;
            }
	    }
        else
        {
            RetVal = FALSE;
        }

        // If the library was loaded by calling LoadLibrary(),
        // then you must use FreeLibrary() to let go of it.
        FreeLibrary(h);
	}
    else
    {
        RetVal = FALSE;
    }
	return RetVal;
}

void COverlayOutput::LoadDynamicFunctions()
{
    // we've got to load these functions dynamically 
    // so that we continue to run on NT 4
	hUserLib = LoadLibrary("user32.dll");
    lpMonitorFromWindow = (HMONITOR (WINAPI *)( IN HWND hwnd, IN DWORD dwFlags)) GetProcAddress(hUserLib, "MonitorFromWindow");
    lpGetMonitorInfoA = (BOOL (WINAPI *)( IN HMONITOR hMonitor, OUT LPMONITORINFO lpmi)) GetProcAddress(hUserLib, "GetMonitorInfoA");

    hDDrawLib = LoadLibrary("ddraw.dll");
    lpDirectDrawCreate = (HRESULT (WINAPI* )( GUID FAR *, LPDIRECTDRAW FAR *, IUnknown FAR * )) GetProcAddress(hDDrawLib, "DirectDrawCreate");
}

//-----------------------------------------------------------------------------
LPDIRECTDRAW COverlayOutput::GetCurrentDD(HWND hWnd)
{
    // if we can't see these new functions or we fail to list the moitors just use a normal DD create
    if(lpMonitorFromWindow == NULL || lpGetMonitorInfoA == NULL || ListMonitors(hWnd) == FALSE)
    {
        if (FAILED(lpDirectDrawCreate(NULL, &lpDD, NULL)))
        {
            ErrorBox("DirectDrawCreate failed");
            return (FALSE);
        }
        return lpDD;
    }

    // got the list of monitors then get the DD context from the current one
	HMONITOR hMonitor = lpMonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);

	for (int i=0 ; i<NbMonitors ; i++)
	{
		if (Monitors[i].hMon == hMonitor)
		{
			return Monitors[i].lpDD;
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------
void COverlayOutput::SetCurrentMonitor(HWND hWnd)
{
    if(lpMonitorFromWindow != NULL)
    {
    	hCurrentMon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
    }
}

//-----------------------------------------------------------------------------
void COverlayOutput::CheckChangeMonitor(HWND hWnd)
{
    if(lpMonitorFromWindow == NULL)
    {
    	return;
    }

	HMONITOR hMon = lpMonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);

	if (hCurrentMon == NULL)
	{
		return;
	}

	if (hMon != hCurrentMon)
	{
		hCurrentMon = hMon;
		Overlay_Stop(hWnd);
		if (lpDD != NULL)
		{
			Overlay_Destroy();
			lpDD = NULL;
		}
		if (InitDD(hWnd))
		{
			Overlay_Start(hWnd);
		}
	}
}

//-----------------------------------------------------------------------------
// Tells whether or not video overlay color control is possible
BOOL COverlayOutput::CanDoOverlayColorControl()
{
    return (pDDColorControl != NULL);
}

//-----------------------------------------------------------------------------
// Tells whether or not video overlay is active
BOOL COverlayOutput::OverlayActive()
{
    return (lpDDOverlay != NULL);
}

//-----------------------------------------------------------------------------
// Blank out video overlay
void COverlayOutput::Overlay_Clean()
{
    unsigned int nPixel;
    unsigned int nLine;
    HRESULT ddrval;
    DDSURFACEDESC SurfaceDesc;
    unsigned short* pPixel;

    EnterCriticalSection(&hDDCritSect);
    if (lpDDOverlay != NULL)
    {
        memset(&SurfaceDesc, 0x00, sizeof(SurfaceDesc));
        SurfaceDesc.dwSize = sizeof(SurfaceDesc);

        ddrval = lpDDOverlay->Lock(NULL, &SurfaceDesc, DDLOCK_WAIT, NULL);

        for (nLine = 0; nLine < SurfaceDesc.dwHeight; nLine++)
        {
            pPixel = (unsigned short*)((BYTE*)SurfaceDesc.lpSurface + nLine * SurfaceDesc.lPitch);
            for (nPixel = 0; nPixel < SurfaceDesc.dwWidth; ++nPixel)
            {
                *pPixel = 0x8000;
                pPixel++;
            }
        }
        ddrval = lpDDOverlay->Unlock(SurfaceDesc.lpSurface);
    }
    if (lpDDOverlayBack != NULL)
    {
        memset(&SurfaceDesc, 0x00, sizeof(SurfaceDesc));
        SurfaceDesc.dwSize = sizeof(SurfaceDesc);

        ddrval = lpDDOverlayBack->Lock(NULL, &SurfaceDesc, DDLOCK_WAIT, NULL);

        for (nLine = 0; nLine < SurfaceDesc.dwHeight; nLine++)
        {
            pPixel = (unsigned short*)((BYTE*)SurfaceDesc.lpSurface + nLine * SurfaceDesc.lPitch);
            for (nPixel = 0; nPixel < SurfaceDesc.dwWidth; ++nPixel)
            {
                *pPixel = 0x8000;
                pPixel++;
            }
        }
        ddrval = lpDDOverlayBack->Unlock(SurfaceDesc.lpSurface);
    }
    LeaveCriticalSection(&hDDCritSect);
}


//-----------------------------------------------------------------------------
// Update video overlay with new rectangle
BOOL COverlayOutput::Overlay_Update(LPRECT pSrcRect, LPRECT pDestRect, DWORD dwFlags)
{
    HRESULT     ddrval;
    DDOVERLAYFX DDOverlayFX;
    int i(0);

    if ((lpDD == NULL) || (lpDDSurface == NULL) || (lpDDOverlay == NULL))
    {
        return FALSE;
    }

    EnterCriticalSection(&hDDCritSect);

    memset(&DDOverlayFX, 0x00, sizeof(DDOverlayFX));
    DDOverlayFX.dwSize = sizeof(DDOverlayFX);

    if (pSrcRect == NULL)
    {
        ////////////////////////////////
        // we are trying to hide overlay
        ////////////////////////////////
        ddrval = lpDDOverlay->UpdateOverlay(NULL, lpDDSurface, NULL, dwFlags, &DDOverlayFX);
        // if another device has requested exclusive access then we
        // can get the no hardware error, just wait a bit and try again
        while(ddrval == DDERR_NOOVERLAYHW)
        {
            Sleep(100);
            ddrval = lpDDOverlay->UpdateOverlay(NULL, lpDDSurface, NULL, dwFlags, &DDOverlayFX);
        }
        // just return if we get this here
        // all DDERR_SURFACELOST will be handled by
        // the main processing loop
        if(ddrval == DDERR_SURFACELOST)
        {
            LeaveCriticalSection(&hDDCritSect);
            return FALSE;
        }
        // 3dfx cards fail with DDERR_UNSUPPORTED if an overlay is hidden before it is shown.
        // we just ignore that error.
        if (FAILED(ddrval) && ddrval != DDERR_CURRENTLYNOTAVAIL &&
            !(ddrval == DDERR_UNSUPPORTED && dwFlags == DDOVER_HIDE))
        {
            // 2001-01-06 John Adcock
            // Now show return code
            char szErrorMsg[200];
            sprintf(szErrorMsg, "Error %x calling UpdateOverlay (Hide)", ddrval);
            ErrorBox(szErrorMsg);
            LeaveCriticalSection(&hDDCritSect);
            return FALSE;
        }
    }
    else
    {
        ////////////////////////////////
        // we are trying to show overlay
        ////////////////////////////////
        if(bCanDoColorKey)
        {
            dwFlags |= DDOVER_KEYDESTOVERRIDE;

			COLORREF overlayColour;
			HDC hDC;

			if (SUCCEEDED(lpDDSurface->GetDC(&hDC)))
			{
				overlayColour = Overlay_GetCorrectedColor(hDC);
				lpDDSurface->ReleaseDC(hDC);
			}
			else
			{
				overlayColour = Overlay_GetColor();
			}

            DWORD dwPhysicalOverlayColor = Overlay_ColorMatch(lpDDSurface, overlayColour);
            if (dwPhysicalOverlayColor == 0 && overlayColour != 0)      // sometimes we glitch and can't get the Value
            {
                LOG(3, "Physical overlay color is zero!  Retrying.");
                dwPhysicalOverlayColor = Overlay_ColorMatch(lpDDSurface, overlayColour);
            }
            LOG(3, "Physical overlay color is %x", dwPhysicalOverlayColor);

            VT_SetOverlayColour(overlayColour);

            DDOverlayFX.dckDestColorkey.dwColorSpaceHighValue = dwPhysicalOverlayColor;
            DDOverlayFX.dckDestColorkey.dwColorSpaceLowValue = dwPhysicalOverlayColor;
        }
        if(bCanDoBob && bAllowBobMode)
        {
            dwFlags |= DDOVER_BOB;
        }

        ddrval = lpDDOverlay->UpdateOverlay(pSrcRect, lpDDSurface, pDestRect, dwFlags, &DDOverlayFX);
		
        // if another device has requested exclusive access then we
        // can get the no hardware error, just wait a bit and try again
        while(ddrval == DDERR_NOOVERLAYHW)
        {
            Sleep(100);
            ddrval = lpDDOverlay->UpdateOverlay(pSrcRect, lpDDSurface, pDestRect, dwFlags, &DDOverlayFX);
        }
        // just return if we get this here
        // all DDERR_SURFACELOST will be handled by
        // the main processing loop
        if(ddrval == DDERR_SURFACELOST)
        {
            LeaveCriticalSection(&hDDCritSect);
            return FALSE;
        }
        // we get unsupported error here for mpact2 cards
        // so cope with this by not trying to update
        // the color key Value and just hoping it works
        // with the existing one (black used to work)
        if(ddrval == DDERR_UNSUPPORTED)
        {
            DDCOLORKEY ColorKey;

            LOG(1, "Got unsupported error from Overlay Update");
            ddrval = lpDDOverlay->GetColorKey(DDCKEY_DESTOVERLAY, &ColorKey);
            if(SUCCEEDED(ddrval))
            {
                g_OverlayColor = ColorKey.dwColorSpaceHighValue;
                LOG(1, "Reset overlay color to %x", g_OverlayColor);
            }
            dwFlags &= ~DDOVER_KEYDESTOVERRIDE;
            memset(&DDOverlayFX, 0x00, sizeof(DDOverlayFX));
            DDOverlayFX.dwSize = sizeof(DDOverlayFX);
            ddrval = lpDDOverlay->UpdateOverlay(pSrcRect, lpDDSurface, pDestRect, dwFlags, &DDOverlayFX);
        }
        // on some cards the driver returns DDERR_OUTOFCAPS, if the wrong original
        // is not at the fron, so flip round until it is
        // but check that we don't go into an infinite loop
        while(ddrval == DDERR_OUTOFCAPS && i < 5)
        {
            ddrval = lpDDOverlay->Flip(NULL, 0); 
            ddrval = lpDDOverlay->UpdateOverlay(pSrcRect, lpDDSurface, pDestRect, dwFlags, &DDOverlayFX);
            ++i;
        }

        if (FAILED(ddrval))
        {
            if ((pDestRect->top < pDestRect->bottom) && (pDestRect->left < pDestRect->right))
            {
                // 2000-10-29 Added by Mark Rejhon
                // Display error message only if rectangle dimensions are positive.
                // Negative rectangle dimensions are frequently caused by the user
                // resizing the window smaller than the video size.
                // 2001-01-06 John Adcock
                // Now show return code
                char szErrorMsg[200];
                sprintf(szErrorMsg, "Error %x in UpdateOverlay", ddrval);
                ErrorBox(szErrorMsg);
            }
            LeaveCriticalSection(&hDDCritSect);
            return FALSE;
        }
        
        // update the controls
        if(bUseOverlayControls)
        {
           Overlay_SetColorControls();
        }
    }
    LeaveCriticalSection(&hDDCritSect);
    return TRUE;
}


void COverlayOutput::Overlay_ResetColorControls()
{
    if (pDDColorControl == NULL)
    {
        return;
    }

    if(OriginalColorControls.dwFlags != 0)
    {
        HRESULT ddrval = pDDColorControl->SetColorControls(&OriginalColorControls);
		// if we have lost the surface e.g Ctrl-alt-del
		// just carry on
        if(ddrval != DDERR_SURFACELOST && FAILED(ddrval))
        {
            char szErrorMsg[200];
            sprintf(szErrorMsg, "Error %x in SetColorControls()", ddrval);
            ErrorBox(szErrorMsg);
        }
    }
}

void COverlayOutput::Overlay_SetColorControls()
{
    DDCOLORCONTROL sColorControl;

    if (pDDColorControl == NULL)
    {
        return;
    }

    sColorControl.dwSize = sizeof(DDCOLORCONTROL);
    if (SUCCEEDED(pDDColorControl->GetColorControls(&sColorControl)))
    {
        if(sColorControl.dwFlags != 0)
        {
            // Needed on the nVidia
            if(OriginalColorControls.dwFlags == 0)
            {
                memcpy(&OriginalColorControls, &sColorControl, sizeof(DDCOLORCONTROL));
                if(!(sColorControl.dwFlags & DDCOLOR_BRIGHTNESS))
                {
                    OtherSettings[OVERLAYBRIGHTNESS].szDisplayName = NULL;
                }
                if(!(sColorControl.dwFlags & DDCOLOR_CONTRAST))
                {
                    OtherSettings[OVERLAYCONTRAST].szDisplayName = NULL;
                }
                if(!(sColorControl.dwFlags & DDCOLOR_HUE))
                {
                    OtherSettings[OVERLAYHUE].szDisplayName = NULL;
                }
                if(!(sColorControl.dwFlags & DDCOLOR_SATURATION))
                {
                    OtherSettings[OVERLAYSATURATION].szDisplayName = NULL;
                }
                if(!(sColorControl.dwFlags & DDCOLOR_SHARPNESS))
                {
                    OtherSettings[OVERLAYSHARPNESS].szDisplayName = NULL;
                }
                if(!(sColorControl.dwFlags & DDCOLOR_GAMMA))
                {
                    OtherSettings[OVERLAYGAMMA].szDisplayName = NULL;
                }

            }

            sColorControl.lBrightness = OverlayBrightness * 10;
            sColorControl.lContrast = OverlayContrast * 100;
            sColorControl.lHue = OverlayHue;
            sColorControl.lSaturation = OverlaySaturation * 100;
            sColorControl.lGamma = OverlayGamma;
            sColorControl.lSharpness = OverlaySharpness;

            if(bUseOverlayControls)
            {
                HRESULT ddrval = pDDColorControl->SetColorControls(&sColorControl);
                if (FAILED(ddrval))
                {
                    char szErrorMsg[200];
                    sprintf(szErrorMsg, "Error %x in SetColorControls()", ddrval);
                    ErrorBox(szErrorMsg);
                }
            }
        }
    }
    else
    {
        ErrorBox("Cannot get color control");
    }
}


//-----------------------------------------------------------------------------
// Create new video overlay
BOOL COverlayOutput::Overlay_Create()
{
    DDSURFACEDESC SurfaceDesc;
    DDPIXELFORMAT PixelFormat;
    HRESULT ddrval;
    DDSCAPS caps;
    int minBuffers, maxBuffers, numBuffers;
    char msg[500];
    int loopcounter = 0;

    if (lpDDOverlay)
    {
        return FALSE;
    }

    EnterCriticalSection(&hDDCritSect);

    // 2000-10-31 Moved by Mark Rejhon
    // Attempt to create primary surface before overlay, in this module,
    // because we may have destroyed the primary surface during a computer 
    // resolution change.
    memset(&SurfaceDesc, 0x00, sizeof(SurfaceDesc));
    SurfaceDesc.dwSize = sizeof(SurfaceDesc);
    SurfaceDesc.dwFlags = DDSD_CAPS;
    SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    ddrval = lpDD->CreateSurface(&SurfaceDesc, &lpDDSurface, NULL);

	// handle the case where we are in a funny in between state
	// by looping, this happens sometimes with Ctrl-alt-delete on
	// some graphics cards.
	while(ddrval == DDERR_UNSUPPORTEDMODE && loopcounter++ < 20)
	{
		Sleep(100);
	    ddrval = lpDD->CreateSurface(&SurfaceDesc, &lpDDSurface, NULL);
	}
    if (FAILED(ddrval))
    {
        sprintf(msg, "Error creating primary surface: %x", ddrval);
        RealErrorBox(msg);
        LeaveCriticalSection(&hDDCritSect);
        return (FALSE);
    }

    memset(&PixelFormat, 0x00, sizeof(PixelFormat));
    PixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    if (bIsRGB)
    {
        PixelFormat.dwFlags = DDPF_RGB;
        PixelFormat.dwRGBBitCount = 16;
        PixelFormat.dwRBitMask = 0xf800;
        PixelFormat.dwGBitMask = 0x07e0;
        PixelFormat.dwBBitMask = 0x001f;
    }
    else
    {
        PixelFormat.dwFlags = DDPF_FOURCC;
        PixelFormat.dwFourCC = MAKEFOURCC('Y', 'U', 'Y', '2');
        PixelFormat.dwYUVBitCount = 16;
    }

    memset(&SurfaceDesc, 0x00, sizeof(SurfaceDesc));
    SurfaceDesc.dwSize = sizeof(SurfaceDesc);
    SurfaceDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_BACKBUFFERCOUNT;
    SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY | DDSCAPS_FLIP | DDSCAPS_COMPLEX;

    // create a surface big enough to hold the largest resolution supported
    // this ensures that we can always have enough space to allow
    // Mode changes without recreating the overlay
    SurfaceDesc.dwWidth = DSCALER_MAX_WIDTH;
    SurfaceDesc.dwHeight = DSCALER_MAX_HEIGHT;
    SurfaceDesc.ddpfPixelFormat = PixelFormat;

    // If the user specified a particular back buffer Count, use it.  Otherwise
    // try triple buffering and drop down to double buffering, then single
    // buffering, if the card doesn't have enough memory.
    minBuffers = BackBuffers >= 0 ? BackBuffers : 0;
    maxBuffers = BackBuffers >= 0 ? BackBuffers : 2;

    for (numBuffers = maxBuffers; numBuffers >= minBuffers; numBuffers--)
    {
        SurfaceDesc.dwBackBufferCount = numBuffers;
        // if we get down to zero back buffers then the flags are different
        if(numBuffers == 0)
        {
            SurfaceDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
            SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY;
        }
        ddrval = lpDD->CreateSurface(&SurfaceDesc, &lpDDOverlay, NULL);

		// stop trying if we succeeded
		// or if we are told there is not enough memory
		// maybe E_INVALIDARG means that the card doesn't support that many
		// buffers so loop if we get that too
        if (SUCCEEDED(ddrval) || (ddrval != DDERR_OUTOFVIDEOMEMORY && ddrval != E_INVALIDARG))
		{
            break;
		}
    }

    if (FAILED(ddrval))
    {
        lpDDOverlay = NULL;
    }

    if (numBuffers < minBuffers)
    {
        // Not enough video memory.  Display different messages depending
        // on what we tried to allocate, since it may mean different things.

        if (minBuffers == 0)
        {
            // We tried single-buffering and it didn't work.
            RealErrorBox("Your video card doesn't have enough overlay\n"
                     "memory for a TV picture.  Try lowering your\n"
                     "color depth or screen resolution.  Rebooting\n"
                     "may help in some cases if memory is being\n"
                     "used by something else on your system.");
            LeaveCriticalSection(&hDDCritSect);
            return (FALSE);
        }
        else
        {
            // We didn't get down to single-buffering, meaning the user
            // specified a back buffer Count.
            sprintf(msg, "Your video card doesn't have enough overlay\n"
                         "memory for %d back buffers.  If you've used\n"
                         "that many back buffers before, you may need\n"
                         "to reboot.  Otherwise try lowering your screen\n"
                         "resolution or color depth, or try setting\n"
                         "BackBuffers=-1 in DScaler.ini to allow DScaler to\n"
                         "decide how many back buffers it can allocate.",
                    BackBuffers);
            RealErrorBox(msg);
            LeaveCriticalSection(&hDDCritSect);
            return (FALSE);
        }
    }

    if (FAILED(ddrval))
    {
        switch (ddrval) 
        {
        case DDERR_NOOVERLAYHW:
            RealErrorBox("Your video card doesn't appear to support\n"
                     "overlays, which DScaler requires.");
            LeaveCriticalSection(&hDDCritSect);
            return (FALSE);

            // Any other interesting error codes?
        }
        
        sprintf(msg, "Can't create overlay surface: %x", ddrval);
        RealErrorBox(msg);
        LeaveCriticalSection(&hDDCritSect);
        return FALSE;
    }

    sprintf(msg, "%d Back Buffers", numBuffers);
    AddSplashTextLine(msg);

    ddrval = lpDDOverlay->Lock(NULL, &SurfaceDesc, DDLOCK_WAIT, NULL);
    // sometimes in win98 we get weird error messages here
    // so we need to loop until it's OK or we get a surface lost message
    while(ddrval == DDERR_NOOVERLAYHW || ddrval == DDERR_SURFACEBUSY)
    {
        Sleep(100);
        ddrval = lpDDOverlay->Lock(NULL, &SurfaceDesc, DDLOCK_WAIT, NULL);
    }
    if(ddrval == DDERR_SURFACELOST)
    {
        ddrval = lpDD->CreateSurface(&SurfaceDesc, &lpDDOverlay, NULL);
        if (FAILED(ddrval))
        {
            sprintf(msg, "Lost overlay surface and can't recreate it: %x", ddrval);
            RealErrorBox(msg);
            lpDDOverlay = NULL;
            LeaveCriticalSection(&hDDCritSect);
            return FALSE;
        }
        ddrval = lpDDOverlay->Lock(NULL, &SurfaceDesc, DDLOCK_WAIT, NULL);
    }
    if (FAILED(ddrval))
    {
        char szErrorMsg[200];
        sprintf(szErrorMsg, "Error %x in Lock Surface", ddrval);
        RealErrorBox(szErrorMsg);
        LeaveCriticalSection(&hDDCritSect);
        return (FALSE);
    }

    ddrval = lpDDOverlay->Unlock(SurfaceDesc.lpSurface);
    if (FAILED(ddrval))
    {
        RealErrorBox("Can't Unlock Surface");
        LeaveCriticalSection(&hDDCritSect);
        return (FALSE);
    }

    if(numBuffers != 0)
    {
        memset(&caps, 0, sizeof(caps));
        caps.dwCaps = DDSCAPS_BACKBUFFER;
        ddrval = lpDDOverlay->GetAttachedSurface(&caps, &lpDDOverlayBack);
        if (FAILED(ddrval))
        {
            RealErrorBox("Can't create Overlay Back Surface");
            lpDDOverlayBack = NULL;
            LeaveCriticalSection(&hDDCritSect);
            return (FALSE);
        }
        else
        {
            ddrval = lpDDOverlayBack->Lock(NULL, &SurfaceDesc, DDLOCK_WAIT, NULL);
            if (FAILED(ddrval))
            {
                RealErrorBox("Can't Lock Back Surface");
                LeaveCriticalSection(&hDDCritSect);
                return (FALSE);
            }
            ddrval = DDERR_WASSTILLDRAWING;

            ddrval = lpDDOverlayBack->Unlock(SurfaceDesc.lpSurface);
            if (FAILED(ddrval))
            {
                RealErrorBox("Can't Unlock Back Surface");
                LeaveCriticalSection(&hDDCritSect);
                return (FALSE);
            }
        }
    }
    else
    {
        lpDDOverlayBack = lpDDOverlay;
        lpDDOverlayBack->AddRef();
    }

    // New Code - Getting the DD color control
    ddrval = lpDDOverlay->QueryInterface(IID_IDirectDrawColorControl, (void **) &pDDColorControl);
    if(SUCCEEDED(ddrval))
    {
		OriginalColorControls.dwSize = sizeof(DDCOLORCONTROL);
        ddrval = pDDColorControl->GetColorControls(&OriginalColorControls);
        if(SUCCEEDED(ddrval))
        {
            LOG(3, "OriginalColorControls %d Brightness %d", OriginalColorControls.dwFlags & DDCOLOR_BRIGHTNESS, OriginalColorControls.lBrightness);
            LOG(3, "OriginalColorControls %d Contrast %d", OriginalColorControls.dwFlags & DDCOLOR_CONTRAST, OriginalColorControls.lContrast);
            LOG(3, "OriginalColorControls %d Hue %d", OriginalColorControls.dwFlags & DDCOLOR_HUE, OriginalColorControls.lHue);
            LOG(3, "OriginalColorControls %d Saturation %d", OriginalColorControls.dwFlags & DDCOLOR_SATURATION, OriginalColorControls.lSaturation);
            LOG(3, "OriginalColorControls %d Gamma %d", OriginalColorControls.dwFlags & DDCOLOR_SHARPNESS, OriginalColorControls.lGamma);
            LOG(3, "OriginalColorControls %d Sharpness %d", OriginalColorControls.dwFlags & DDCOLOR_GAMMA, OriginalColorControls.lSharpness);
            if(bUseOverlayControls)
            {
               Overlay_SetColorControls();
            }
        }
        else
        {
            pDDColorControl->Release();
			pDDColorControl = NULL;
        }
    }
    else
    {
		pDDColorControl = NULL;
    }


    // try to create a memory buffer
    // that we can use if any output filters are switched
    // on.  This is required because reading and writing back to 
    // video memory is very slow
    lpExtraMemoryForFilters = (BYTE*)malloc(DSCALER_MAX_WIDTH * DSCALER_MAX_HEIGHT * 2 + 16);
    if(lpExtraMemoryForFilters == NULL)
    {
       // if we can't do a system memory buffer it's not the end of the
       // world it just means that any output filters will run very slow
       LOG(1, "Couldn't create additional buffer for output filters");
       lpExtraMemoryForFilters = NULL;
    }

    LeaveCriticalSection(&hDDCritSect);

    // overlay clean is wrapped in a critcal section already
    Overlay_Clean();

// some code to work out how long a flip is
// could be used by new JT code
#ifdef _NOT_YET_USED

    int i = 0; 
    OutputTicksPerFrame = 0;
    while(i < 5)
    {
        HRESULT hr = lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
        ULONGLONG ticks1;
        ULONGLONG ticks2;

        QueryPerformanceCounter((PLARGE_INTEGER)&ticks1);

        for(int j = 0; j < 10; ++j)
        {
            hr = lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
        }
        QueryPerformanceCounter((PLARGE_INTEGER)&ticks2);
        ULONG Ticks = (ticks2 - ticks1);
        // if we get the same answer to within 1% twice in a row
        // we're done, this should avoid problems with pre-emption
        if(OutputTicksPerFrame * 99 < Ticks * 10 && 
            OutputTicksPerFrame * 101 > Ticks * 10)
        {
            break;
        }
        OutputTicksPerFrame = Ticks / 10;
        ++i;
    }

    if(i == 5)
    {
        LOG(1, "Trouble getting lock on Ticks per frame");
    }

#endif

    return (TRUE);
}


//-----------------------------------------------------------------------------
// Name: DDColorMatch()
// Desc: Convert a RGB color to a pysical color.
//       We do this by leting GDI SetPixel() do the color matching
//       then we lock the memory and see what it got mapped to.
//-----------------------------------------------------------------------------
DWORD COverlayOutput::Overlay_ColorMatch(LPDIRECTDRAWSURFACE pdds, COLORREF rgb)
{
    COLORREF rgbT;
    HDC hdc;
    DWORD dw = CLR_INVALID;
    DDSURFACEDESC SurfaceDesc;
    HRESULT hres;

    //
    //  Use GDI SetPixel to color match for us
    //
    hres = pdds->GetDC(&hdc);
    if (SUCCEEDED(hres))
    {
        rgbT = GetPixel(hdc, 0, 0);     // Save current pixel Value
        SetPixel(hdc, 0, 0, rgb);       // Set our Value
        pdds->ReleaseDC(hdc);
    }
    //
    // Now lock the surface so we can read back the converted color
    //
    SurfaceDesc.dwSize = sizeof(SurfaceDesc);
    hres = pdds->Lock(NULL, &SurfaceDesc, DDLOCK_WAIT, NULL);
    if (SUCCEEDED(hres))
    {
        if(SurfaceDesc.ddpfPixelFormat.dwRGBBitCount <= 8)
        {
            dw = *(BYTE*)SurfaceDesc.lpSurface;
        }
        else
        {
            dw = *(DWORD*)SurfaceDesc.lpSurface;                 // Get DWORD
            if (SurfaceDesc.ddpfPixelFormat.dwRGBBitCount < 32)
            {
                dw &= (1 << SurfaceDesc.ddpfPixelFormat.dwRGBBitCount) - 1;  // Mask it to bpp
            }
        }
        pdds->Unlock(NULL);
    }

    //
    //  Now put the color that was there back.
    //
    hres = pdds->GetDC(&hdc);
    if (SUCCEEDED(hres))
    {
        SetPixel(hdc, 0, 0, rgbT);
        pdds->ReleaseDC(hdc);
    }
    return dw;
}

//-----------------------------------------------------------------------------
// Deinitialize video overlay
//
// 2000-10-31 Added by Mark Rejhon
// Provide a way to destroy the video overlay and primary, which should
// be done right before a computer resolution change.
// 
BOOL COverlayOutput::Overlay_Destroy()
{
    EnterCriticalSection(&hDDCritSect);

    // reset overlay control to previous settings so that
    // DVD's will look OK
    if(pDDColorControl != NULL)
    {
        Overlay_ResetColorControls();
        pDDColorControl->Release();
        pDDColorControl = NULL;

    }

    // Now destroy the Extra Surface
    if (lpExtraMemoryForFilters != NULL)
    {
        free(lpExtraMemoryForFilters);
        lpExtraMemoryForFilters = NULL;
    }
   
    // Now destroy the Back Overlay
    if (lpDDOverlayBack != NULL)
    {
        lpDDOverlayBack->Release();
        lpDDOverlayBack = NULL;
    }

    // Now destroy the main Overlay
    if (lpDDOverlay != NULL)
    {
        // Destroy the video overlays
        lpDDOverlay->Release();
        lpDDOverlay = NULL;
    }

    // Now destroy the primary surface
    if (lpDDSurface != NULL) 
    {
        lpDDSurface->Release();
        lpDDSurface = NULL;
    }
    LeaveCriticalSection(&hDDCritSect);
    return TRUE;
}


COLORREF COverlayOutput::Overlay_GetColor()
{
    return g_OverlayColor;
}

COLORREF COverlayOutput::Overlay_GetCorrectedColor(HDC hDC)
{
	COLORREF nearest = GetNearestColor(hDC, g_OverlayColor);
	return (nearest == CLR_INVALID) ? g_OverlayColor : nearest;
}

BOOL COverlayOutput::Overlay_Lock_Extra_Buffer(TDeinterlaceInfo* pInfo)
{
    if(lpExtraMemoryForFilters == NULL)
    {
        LOG(1, "Extra Buffer has been deleted");
        return FALSE;
    }

    pInfo->OverlayPitch = DSCALER_MAX_WIDTH * 2;
    // get back some memory aligned on 16 byte boundary for SSE
    pInfo->Overlay = lpExtraMemoryForFilters + (16 - ((DWORD)lpExtraMemoryForFilters % 16));
    return TRUE;
}

//
// Add a function to Lock the overlay surface and update some Info from it.
// We always lock and write to the back buffer.
// Flipping takes care of the proper buffer addresses.
// Some of this Info can change each time.  
// We also check to see if we still need to Flip because the
// non-waiting last flip failed.  If so, try it one more time,
// then give up.  Tom Barry 10/26/00
//
BOOL COverlayOutput::Overlay_Lock_Back_Buffer(TDeinterlaceInfo* pInfo, BOOL bUseExtraBuffer)
{
    if(bUseExtraBuffer && lpExtraMemoryForFilters != NULL)
    {
        return Overlay_Lock_Extra_Buffer(pInfo);
    }
    if(lpDDOverlay == NULL || lpDDOverlayBack == NULL)
    {
        LOG(1, "Overlay has been deleted");
        return FALSE;
    }

    EnterCriticalSection(&hDDCritSect);

    HRESULT         ddrval;
    static DWORD    dwFlags = DDLOCK_WAIT | DDLOCK_NOSYSLOCK;
    DDSURFACEDESC   SurfaceDesc;

    if (FlipResult == DDERR_WASSTILLDRAWING)             // prev flip was busy?
    {
        ddrval = lpDDOverlay->Flip(NULL, DDFLIP_DONOTWAIT);  
        if(ddrval == DDERR_SURFACELOST)
        {
            LOG(1, "Flip before lock failed");
            LeaveCriticalSection(&hDDCritSect);
            return FALSE;
        }
        FlipResult = 0;                 // but no time to try any more
    }

    memset(&SurfaceDesc, 0x00, sizeof(SurfaceDesc));
    SurfaceDesc.dwSize = sizeof(SurfaceDesc);
    while(ddrval = lpDDOverlayBack->Lock(NULL, &SurfaceDesc, dwFlags & !DDLOCK_WAIT, NULL) == DDERR_WASSTILLDRAWING)
	{
        // RM 7-27-7: We are probably waiting for the vertical sync. 
        // To prevent spending useless cpu cycles waiting for the vsync we release control
        // so other applications can do their thing. This not only reduces cpu usage but 
        // hopefully it reduces frame dropping as well since there is never such a large
        // amount of time to spare then at this moment.
		Sleep(1);
	}

    // fix suggested by christoph for NT 4.0 sp6
    if(ddrval == E_INVALIDARG && (dwFlags & DDLOCK_NOSYSLOCK))
    {
        //remove flag
        ddrval = lpDDOverlayBack->Lock(NULL, &SurfaceDesc, DDLOCK_WAIT, NULL);
        if( SUCCEEDED(ddrval) )
        {
            //remember for next time
            dwFlags = DDLOCK_WAIT;
            LOG(1, "Switched to not using NOSYSLOCK");
        }
    }

    if(FAILED(ddrval))
    {
        LOG(1, "Lock failed %8x", ddrval);
        LeaveCriticalSection(&hDDCritSect);
        return FALSE;
    }

    pInfo->OverlayPitch = SurfaceDesc.lPitch;         // Set new pitch, may change
    pInfo->Overlay = (BYTE*)SurfaceDesc.lpSurface;
	if(pInfo->Overlay == NULL)
	{
		LeaveCriticalSection(&hDDCritSect);
        return FALSE;
	}

    // stay in critical section
    return TRUE;
}

BOOL COverlayOutput::Overlay_Lock(TDeinterlaceInfo* pInfo)
{
    if(lpDDOverlay == NULL || lpDDOverlayBack == NULL)
    {
        LOG(1, "Overlay has been deleted");
        return FALSE;
    }

    EnterCriticalSection(&hDDCritSect);

    HRESULT         ddrval;
    static DWORD    dwFlags = DDLOCK_WAIT | DDLOCK_NOSYSLOCK;
    DDSURFACEDESC   SurfaceDesc;

    if (FlipResult == DDERR_WASSTILLDRAWING)             // prev flip was busy?
    {
        ddrval = lpDDOverlay->Flip(NULL, DDFLIP_DONOTWAIT);  
        if(ddrval == DDERR_SURFACELOST)
        {
            LOG(1, "Flip before lock failed");
            LeaveCriticalSection(&hDDCritSect);
            return FALSE;
        }
        FlipResult = 0;                 // but no time to try any more
    }

    memset(&SurfaceDesc, 0x00, sizeof(SurfaceDesc));
    SurfaceDesc.dwSize = sizeof(SurfaceDesc);
    ddrval = lpDDOverlay->Lock(NULL, &SurfaceDesc, dwFlags, NULL);

    // fix suggested by christoph for NT 4.0 sp6
    if(ddrval == E_INVALIDARG && (dwFlags & DDLOCK_NOSYSLOCK))
    {
        //remove flag
        ddrval = lpDDOverlay->Lock(NULL, &SurfaceDesc, DDLOCK_WAIT, NULL);
        if( SUCCEEDED(ddrval) )
        {
            //remember for next time
            dwFlags = DDLOCK_WAIT;
            LOG(1, "Switched to not using NOSYSLOCK");
        }
    }

    if(FAILED(ddrval))
    {
        LOG(1, "Lock failed %8x", ddrval);
        LeaveCriticalSection(&hDDCritSect);
        return FALSE;
    }

    pInfo->OverlayPitch = SurfaceDesc.lPitch;         // Set new pitch, may change
    pInfo->Overlay = (BYTE*)SurfaceDesc.lpSurface;
	if(pInfo->Overlay == NULL)
	{
		LeaveCriticalSection(&hDDCritSect);
        return FALSE;
	}

	
    // stay in critical section
    return TRUE;
}

BOOL COverlayOutput::Overlay_Unlock_Back_Buffer(BOOL bUseExtraBuffer)
{
    if(bUseExtraBuffer && lpExtraMemoryForFilters != NULL)
    {
        return TRUE;
    }
    // make sure we always release the critical section
    if(lpDDOverlayBack == NULL)
    {
        LOG(1, "Overlay has been deleted");
        LeaveCriticalSection(&hDDCritSect);
        return FALSE;
    }

    // we are already in critical section
    HRESULT ddrval = lpDDOverlayBack->Unlock(NULL);
    BOOL RetVal = TRUE;
    if(FAILED(ddrval))
    {
        if(ddrval != DDERR_SURFACELOST)
        {
            LOG(1, "Unexpected failure in Unlock %8x", ddrval);
        }
        RetVal = FALSE;
    }
    LeaveCriticalSection(&hDDCritSect);
    return RetVal;
}

BOOL COverlayOutput::Overlay_Unlock()
{
    // make sure we always leave the critical section
    if(lpDDOverlay == NULL)
    {
        LOG(1, "Overlay has been deleted");
        LeaveCriticalSection(&hDDCritSect);
        return FALSE;
    }

    // we are already in critical section
    HRESULT ddrval = lpDDOverlay->Unlock(NULL);
    BOOL RetVal = TRUE;
    if(FAILED(ddrval))
    {
        if(ddrval != DDERR_SURFACELOST)
        {
            LOG(1, "Unexpected failure in Unlock %8x", ddrval);
        }
        RetVal = FALSE;
    }
    LeaveCriticalSection(&hDDCritSect);
    return RetVal;
}

void COverlayOutput::Overlay_Copy_External(BYTE* lpExternalMemoryBuffer, int ExternalPitch, TDeinterlaceInfo* pInfo)
{
    BYTE* FromPtr = lpExternalMemoryBuffer + (16 - ((DWORD)lpExternalMemoryBuffer % 16));
    long FromPitch = ExternalPitch;
    // careful as we need to ensure this is always unlocked
    if(Overlay_Lock_Back_Buffer(pInfo, FALSE))
    {
        BYTE* ToPtr = pInfo->Overlay;

        for(int i(0) ; i < pInfo->FrameHeight; ++i)
        {
            pInfo->pMemcpy(ToPtr, FromPtr, pInfo->LineLength);
            FromPtr += FromPitch;
            ToPtr += pInfo->OverlayPitch;
        }
        _asm
        {
            emms
        }

        Overlay_Unlock_Back_Buffer(FALSE);
    }
}

void COverlayOutput::Overlay_Copy_Extra(TDeinterlaceInfo* pInfo)
{
    Overlay_Lock_Extra_Buffer(pInfo);
    BYTE* FromPtr = pInfo->Overlay;
    long FromPitch = pInfo->OverlayPitch;
    // careful as we need to ensure this is always unlocked
    if(Overlay_Lock_Back_Buffer(pInfo, FALSE))
    {
        BYTE* ToPtr = pInfo->Overlay;

        for(int i(0) ; i < pInfo->FrameHeight; ++i)
        {
            pInfo->pMemcpy(ToPtr, FromPtr, pInfo->LineLength);
            FromPtr += FromPitch;
            ToPtr += pInfo->OverlayPitch;
        }
        _asm
        {
            emms
        }

        Overlay_Unlock_Back_Buffer(FALSE);
    }
}

BOOL COverlayOutput::Overlay_Flip(DWORD FlipFlag, BOOL bUseExtraBuffer, BYTE* lpExternalMemoryBuffer, int ExternalPitch, TDeinterlaceInfo* pInfo)
{
    if(lpDDOverlay == NULL)
    {
        LOG(1, "Overlay has been deleted");
        return FALSE;
    }

    if(bUseExtraBuffer && lpExternalMemoryBuffer != NULL)
    {
        Overlay_Copy_External(lpExternalMemoryBuffer, ExternalPitch, pInfo);
    }

    // if we have been using the extra surface then we need to copy
    // the picture onto the overlay
    else if(bUseExtraBuffer && lpExtraMemoryForFilters != NULL)
    {
        Overlay_Copy_Extra(pInfo);
    }


	

    EnterCriticalSection(&hDDCritSect);

    BOOL RetVal = TRUE;
    FlipResult = lpDDOverlay->Flip(NULL, FlipFlag); 
    if(FAILED(FlipResult))
    {
        // cope with card's that don't support waiting for the flip
        if(FlipFlag != DDFLIP_WAIT && FlipResult == DDERR_INVALIDPARAMS)
        {
            FlipResult = lpDDOverlay->Flip(NULL, DDFLIP_WAIT); 
            if(SUCCEEDED(FlipResult))
            {
                PostMessageToMainWindow(WM_OUTTHREADS_SETVALUE, WAITFORFLIP, 1);
            }
        }
        if(FlipResult != DDERR_WASSTILLDRAWING && 
            FlipResult != DDERR_SURFACELOST &&
            FlipResult != DDERR_NOTFLIPPABLE)
        {
            LOG(1, "Surface Flip failed %8x", FlipResult);
        }
        // return OK if we get DDERR_WASSTILLDRAWING
        // we'll do the flip next time
        // also if we get DDERR_NOTFLIPPABLE
        // as this probably means the overlay has been hiden
        RetVal = (FlipResult == DDERR_WASSTILLDRAWING) || (FlipResult == DDERR_NOTFLIPPABLE);
    }
    LeaveCriticalSection(&hDDCritSect);
    return RetVal;
}

//-----------------------------------------------------------------------------
// Support for RGB surface
void COverlayOutput::Overlay_SetRGB(BOOL IsRGB)
{
    bIsRGB = IsRGB;
}

BOOL COverlayOutput::Overlay_GetRGB()
{
    return bIsRGB;
}

//-----------------------------------------------------------------------------
// Initialize DirectDraw
BOOL COverlayOutput::InitDD(HWND hWnd)
{
    HRESULT ddrval;
    DDCAPS DriverCaps;

	lpDD = GetCurrentDD(hWnd);

    // can we use Overlay ??
    memset(&DriverCaps, 0x00, sizeof(DriverCaps));
    DriverCaps.dwSize = sizeof(DriverCaps);
    ddrval = lpDD->GetCaps(&DriverCaps, NULL);

    if (SUCCEEDED(ddrval))
    {
        if (DriverCaps.dwCaps & DDCAPS_OVERLAY)
        {
            if (!(DriverCaps.dwCaps & DDCAPS_OVERLAYSTRETCH))
            {
                ErrorBox("Can't Strech Overlay");
                return FALSE;
            }

            bCanDoColorKey = ((DriverCaps.dwCKeyCaps & DDCKEYCAPS_DESTOVERLAY) > 0);

            if (DriverCaps.dwCaps & DDCAPS_ALIGNSIZESRC)
            {
                SrcSizeAlign = DriverCaps.dwAlignSizeSrc;
            }
            else
            {
                SrcSizeAlign = 1;
            }

            if (DriverCaps.dwCaps & DDCAPS_ALIGNSIZEDEST)
            {
                DestSizeAlign = DriverCaps.dwAlignSizeDest;
            }
            else
            {
                DestSizeAlign = 1;
            }
            bCanDoBob = ((DriverCaps.dwCaps2 & DDCAPS2_CANFLIPODDEVEN) != 0);
            bCanDoFlipInterval = ((DriverCaps.dwCaps2 & DDCAPS2_FLIPINTERVAL) != 0);
        }
        else
        {
            ErrorBox("Can't Use Overlay");
            return (FALSE);
        }
    }

    ddrval = lpDD->SetCooperativeLevel(hWnd, DDSCL_NORMAL);

    if (FAILED(ddrval))
    {
        ErrorBox("SetCooperativeLevel failed");
        return (FALSE);
    }

    return TRUE;
}


//-----------------------------------------------------------------------------
// Deinitialize DirectDraw
void COverlayOutput::ExitDD(void)
{
    if (lpDD != NULL)
    {
        Overlay_Destroy();
        if(lpMonitorFromWindow != NULL)
        {
	        for (int i=0 ; i<NbMonitors ; i++)
	        {
                if(Monitors[i].lpDD!=NULL)
                {
		            Monitors[i].lpDD->Release();
                }
                Monitors[i].lpDD = NULL;
                Monitors[i].hMon = 0;
	        }
        }
        else
        {
            lpDD->Release();
        }
        lpDD = NULL;
    }
}

IOutput::OUTPUTTYPES COverlayOutput::Type() 
{
	return OUT_OVERLAY;
}

BOOL COverlayOutput::Overlay_ColorKey_OnChange(long NewValue)
{
    OverlayOutputInstance.g_OverlayColor = (COLORREF)NewValue;
    if(GetActiveOutput()->Type() == OUT_OVERLAY)
    {
        WorkoutOverlaySize(TRUE);
    }
    return FALSE;
}

BOOL COverlayOutput::Overlay_Brightness_OnChange(long NewValue)
{
    OverlayOutputInstance.OverlayBrightness = NewValue;   
    if(GetActiveOutput()->Type() == OUT_OVERLAY)
    {
        GetActiveOutput()->Overlay_SetColorControls();
    }
    return FALSE;
}

BOOL COverlayOutput::Overlay_Contrast_OnChange(long NewValue)
{
    OverlayOutputInstance.OverlayContrast = NewValue;   
    if(GetActiveOutput()->Type() == OUT_OVERLAY)
    {
        GetActiveOutput()->Overlay_SetColorControls();   
    }
    return FALSE;
}

BOOL COverlayOutput::Overlay_Hue_OnChange(long NewValue)
{
    OverlayOutputInstance.OverlayHue = NewValue;   
    if(GetActiveOutput()->Type() == OUT_OVERLAY)
    {
        GetActiveOutput()->Overlay_SetColorControls();      
    }
    return FALSE;
}

BOOL COverlayOutput::Overlay_Saturation_OnChange(long NewValue)
{
    OverlayOutputInstance.OverlaySaturation = NewValue;   
    if(GetActiveOutput()->Type() == OUT_OVERLAY)
    {
        GetActiveOutput()->Overlay_SetColorControls();      
    }
    return FALSE;
}

BOOL COverlayOutput::Overlay_Gamma_OnChange(long NewValue)
{
    OverlayOutputInstance.OverlayGamma = NewValue;   
    if(GetActiveOutput()->Type() == OUT_OVERLAY)
    {
        GetActiveOutput()->Overlay_SetColorControls();      
    }
    return FALSE;
}

BOOL COverlayOutput::Overlay_Sharpness_OnChange(long NewValue)
{
    OverlayOutputInstance.OverlaySharpness = NewValue;   
    if(GetActiveOutput()->Type() == OUT_OVERLAY)
    {
        GetActiveOutput()->Overlay_SetColorControls();      
    }
    return FALSE;
}

BOOL COverlayOutput::Overlay_UseControls_OnChange(long NewValue)
{
	OverlayOutputInstance.bUseOverlayControls = NewValue;
    if(GetActiveOutput()->Type() == OUT_OVERLAY)
    {
        if(OverlayOutputInstance.bUseOverlayControls)
        {
           GetActiveOutput()->Overlay_SetColorControls();
        }
        else
        {
           OverlayOutputInstance.Overlay_ResetColorControls();
        }
    }
    return FALSE;
}

BOOL COverlayOutput::Overlay_AllowBobMode_OnChange(long NewValue)
{
    OverlayOutputInstance.bAllowBobMode = NewValue;
    if(GetActiveOutput()->Type() == OUT_OVERLAY)
    {
        Overlay_Stop(GetMainWnd());
        Overlay_Start(GetMainWnd());
    }
    return FALSE;
}

BOOL COverlayOutput::Overlay_BackBuffers_OnChange(long NewValue)
{
    OverlayOutputInstance.BackBuffers = NewValue;
    if(GetActiveOutput()->Type() == OUT_OVERLAY)
    {
        Overlay_Stop(GetMainWnd());
        Overlay_Start(GetMainWnd());
    }
    return FALSE;
}

void COverlayOutput::WaitForVerticalBlank()
{
	if (lpDD != NULL)
	{
		lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
	}
}


void COverlayOutput::InitOtherSettings()
{    
    const SETTING LocalOtherSettings[OTHER_SETTING_LASTONE] =
	{
		{
			"Back Buffers", SLIDER, 0, (long*)&BackBuffers,
			-1, -1, 2, 1, 1,
			NULL,
			"Overlay", "BackBuffers", Overlay_BackBuffers_OnChange,
		},
		{
			"Overlay Colorkey", SLIDER, 0, (long*)&g_OverlayColor,
			RGB(32,16,16), 0, RGB(255,255,255), 1, 1,
			NULL,
			"Overlay", "OverlayColor", Overlay_ColorKey_OnChange,
		},
		{
			"Use Overlay Controls", ONOFF, 0, (long*)&bUseOverlayControls,
			FALSE, 0, 1, 1, 1,
			NULL,
			"Overlay", "UseOverlayControls", Overlay_UseControls_OnChange,
		},
		// DirectX docs say brightness value should be 0 to 10,000
		// which would mean a range of 0 to 1000.
		// however nVidia cards seem to have a range of -200 to 200
		// so the curreent range allows for this and leaves a lage margin
		// of error
		{
			"Overlay Brightness", SLIDER, 0, (long*)&OverlayBrightness,
			75, -1000, 1000, 5, 10,
			NULL,
			"Overlay", "OverlayBrightness", Overlay_Brightness_OnChange,
		},
		{
			"Overlay Contrast", SLIDER, 0, (long*)&OverlayContrast,
			100, 0, 200, 1, 1,
			NULL,
			"Overlay", "OverlayContrast", Overlay_Contrast_OnChange,
		},
		{
			"Overlay Hue", SLIDER, 0, (long*)&OverlayHue,
			0, -180, 180, 1, 1,
			NULL,
			"Overlay", "OverlayHue", Overlay_Hue_OnChange,
		},
		{
			"Overlay Saturation", SLIDER, 0, (long*)&OverlaySaturation,
			100, 0, 200, 1, 1,
			NULL,
			"Overlay", "OverlaySaturation", Overlay_Saturation_OnChange,
		},
		{
			"Overlay Gamma", SLIDER, 0, (long*)&OverlayGamma,
			1, 1, 500, 1, 1,
			NULL,
			"Overlay", "OverlayGamma", Overlay_Gamma_OnChange,
		},
		{
			"Overlay Sharpness", SLIDER, 0, (long*)&OverlaySharpness,
			5, 0, 10, 1, 1,
			NULL,
			"Overlay", "OverlaySharpness", Overlay_Sharpness_OnChange,
		},
		{
			"Allow Bob Mode", ONOFF, 0, (long*)&bAllowBobMode,
			FALSE, 0, 1, 1, 1,
			NULL,
			"Overlay", "AllowBobMode", Overlay_AllowBobMode_OnChange,
		},
	};
	memmove(OtherSettings, LocalOtherSettings, sizeof(OtherSettings));
}

COverlayOutput::COverlayOutput(void)
{
	lpDD=NULL;
	lpDDSurface=NULL;
	lpDDOverlay=NULL;
	lpDDOverlayBack=NULL;
	lpExtraMemoryForFilters=NULL;
	g_OverlayColor = RGB(32, 16, 16);
	bCanColorKey=FALSE;
	BackBuffers=-1;
	bCanDoBob=false;
	bCanDoFlipInterval=false;
	bCanDoColorKey=false;
	pDDColorControl=NULL;
	bUseOverlayControls=FALSE;
	bAllowBobMode=FALSE;
	OutputTicksPerFrame=0;
	OverlayBrightness=75;
	OverlayContrast = 100;
	OverlayHue = 0;
	OverlaySaturation = 100;
	OverlayGamma = 1;
	OverlaySharpness = 5;
	FlipResult=0;
	DestSizeAlign=1;
	SrcSizeAlign=1;
    m_bSettingInitialized = false;
	bIsRGB = FALSE;
    
    LoadDynamicFunctions();

    InitializeCriticalSection(&hDDCritSect);
}

COverlayOutput::~COverlayOutput(void)
{
    FreeLibrary(hUserLib);
    FreeLibrary(hDDrawLib);
    DeleteCriticalSection(&hDDCritSect);
}


SETTING* Overlay_GetSetting(OTHER_SETTING Setting)
{
    if(Setting > -1 && Setting < OTHER_SETTING_LASTONE)
    {
        return &(OverlayOutputInstance.GetOtherSettings()[Setting]);
    }
    else
    {
        return NULL;
    }
}

CSettingsHolderStandAlone OverlaySettingsHolder;

void Overlay_ReadSettingsFromIni()
{
    CSettingGroup *pOverlayGroup = OverlaySettingsHolder.GetSettingsGroup("Overlay", SETTING_BY_CHANNEL | SETTING_BY_FORMAT | SETTING_BY_INPUT, FALSE);

    OverlaySettingsHolder.AddSetting(&OverlayOutputInstance.GetOtherSettings()[OVERLAYBRIGHTNESS], pOverlayGroup);
    OverlaySettingsHolder.AddSetting(&OverlayOutputInstance.GetOtherSettings()[OVERLAYCONTRAST], pOverlayGroup);
    OverlaySettingsHolder.AddSetting(&OverlayOutputInstance.GetOtherSettings()[OVERLAYHUE], pOverlayGroup);
    OverlaySettingsHolder.AddSetting(&OverlayOutputInstance.GetOtherSettings()[OVERLAYSATURATION], pOverlayGroup);
    OverlaySettingsHolder.AddSetting(&OverlayOutputInstance.GetOtherSettings()[OVERLAYGAMMA], pOverlayGroup);
    OverlaySettingsHolder.AddSetting(&OverlayOutputInstance.GetOtherSettings()[OVERLAYSHARPNESS], pOverlayGroup);

    OverlaySettingsHolder.AddSetting(&OverlayOutputInstance.GetOtherSettings()[BACKBUFFERS]);
    OverlaySettingsHolder.AddSetting(&OverlayOutputInstance.GetOtherSettings()[OVERLAYCOLOR]);
    OverlaySettingsHolder.AddSetting(&OverlayOutputInstance.GetOtherSettings()[USEOVERLAYCONTROLS]);
    OverlaySettingsHolder.AddSetting(&OverlayOutputInstance.GetOtherSettings()[ALLOWBOBMODE]);

#ifdef _DEBUG
    if (OTHER_SETTING_LASTONE != OverlaySettingsHolder.GetNumSettings())
    {
        LOGD("Number of settings in Overlay source is not equal to the number of settings in DS_Control.h");
        LOGD("DS_Control.h or Other.cpp are probably not in sync with each other.");
    }
#endif

    OverlaySettingsHolder.DisableOnChange();
    OverlaySettingsHolder.ReadFromIni();
    OverlaySettingsHolder.EnableOnChange();
}

void Overlay_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    OverlaySettingsHolder.WriteToIni(bOptimizeFileAccess);
}

CTreeSettingsGeneric* Overlay_GetTreeSettingsPage()
{
    return new CTreeSettingsGeneric("Overlay Settings", OverlayOutputInstance.GetOtherSettings(), OVERLAYBRIGHTNESS);
}
