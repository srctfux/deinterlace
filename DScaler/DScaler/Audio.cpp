/////////////////////////////////////////////////////////////////////////////
// $Id: Audio.cpp,v 1.40 2003-10-27 10:39:50 adcockj Exp $
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
// Change Log
//
// Date          Developer             Changes
//
// 11 Aug 2000   John Adcock           Moved Audio Functions in here
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 08 Jan 2001   John Adcock           Maybe fixed crashing bug
//
// 26 Feb 2001   Hermes Conrad         Sound Fixes
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.39  2003/07/29 13:33:06  atnak
// Overhauled mixer code
//
// Revision 1.38  2003/07/17 05:59:00  atnak
// A quick fix for non-unmuting muted hardware problem
//
// Revision 1.37  2003/06/26 09:07:58  adcockj
// Added patch for plopping from Arjan Zipp
//
// Revision 1.36  2003/02/06 00:37:28  robmuller
// Changed log level to prevent rattle when changing channels.
//
// Revision 1.35  2003/01/29 18:24:12  adcockj
// Added logging for mute calls
//
// Revision 1.34  2002/12/14 01:46:15  atnak
// Added thread id check so that SetTimer isn't called on YUVOutThread
//
// Revision 1.33  2002/12/13 20:35:12  tobbej
// added new asssert to audio_unmute (SetTimer shoud not be called from output thread)
//
// Revision 1.32  2002/12/13 02:50:48  atnak
// Added error protection to Anti-Plop unmute delay timer
//
// Revision 1.31  2002/12/09 00:32:15  atnak
// Added new muting stuff
//
// Revision 1.30  2002/12/07 23:05:46  atnak
// New Audio_Mute() and Audio_Unmute() functions for a new muting.
//
// Revision 1.29  2002/12/07 16:06:54  adcockj
// Tidy up muting code
//
// Revision 1.28  2002/12/07 15:59:06  adcockj
// Modified mute behaviour
//
// Revision 1.27  2002/10/18 03:35:01  flibuste2
// Fixed Audio_IsMuted()
// and fixed bSystemInMust variable state
// (was only updated on a change notification)
//
// Revision 1.26  2002/10/17 05:09:27  flibuste2
// Added Audio_IsMuted()
// Returns the current system mute status
//
// Revision 1.25  2002/09/28 13:31:41  kooiman
// Added sender object to events and added setting flag to treesettingsgeneric.
//
// Revision 1.24  2002/09/26 16:35:20  kooiman
// Volume event support.
//
// Revision 1.23  2002/04/13 21:52:39  laurentg
// Management of no current source
//
// Revision 1.22  2001/11/25 01:58:34  ittarnavsky
// initial checkin of the new I2C code
//
// Revision 1.21  2001/11/23 10:49:16  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.20  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.19  2001/11/02 16:33:07  adcockj
// Removed conflict tags
//
// Revision 1.18  2001/11/02 16:30:06  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.17  2001/08/30 10:08:24  adcockj
// Removed variable
//
// Revision 1.16.2.4  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.16.2.3  2001/08/17 16:35:13  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
// Revision 1.16.2.2  2001/08/14 16:41:36  adcockj
// Renamed driver
// Got to compile with new class based card
//
// Revision 1.16.2.1  2001/08/14 09:40:19  adcockj
// Interim version of code for multiple card support
//
// Revision 1.16  2001/08/05 16:31:55  adcockj
// Removed Fake MSP code
//
// Revision 1.15  2001/08/02 18:08:17  adcockj
// Made all logging code use new levels
//
// Revision 1.14  2001/08/02 07:45:10  adcockj
// Fixed problem with stereo settings
//
// Revision 1.13  2001/07/16 18:07:50  adcockj
// Added Optimisation parameter to ini file saving
//
// Revision 1.12  2001/07/13 18:13:24  adcockj
// Changed Mute to not be persisted and to work properly
//
// Revision 1.11  2001/07/13 16:14:55  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.10  2001/07/13 07:04:43  adcockj
// Attemp 1 at fixing MSP muting
//
// Revision 1.9  2001/07/12 16:16:39  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file Audio.cpp Audio Functions
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "audio.h"
#include "Status.h"
#include "MixerDev.h"
#include "Providers.h"
#include "DebugLog.h"

CRITICAL_SECTION AudioMuteCriticalSection;
DWORD dwTimerProcThreadId = 0;
BYTE AudioMuteStatus = 0;
BOOL bUserMute = FALSE;

VOID CALLBACK AudioUnmuteDelayTimerProc(HWND hwnd, UINT, UINT idTimer, DWORD);

//  IMPORTANT: Audio_Mute() and Audio_Unmute() are for internal
//  use only.  It should be placed in pairs.  For user prompted
//  muting, use Audio_SetUserMute().


void Initialize_Mute()
{
    // This critical section isn't needed but may
    // be in future if Audio_Mute/Unmute are going
    // to be used asynchronously.
    InitializeCriticalSection(&AudioMuteCriticalSection);

    // Set the timer proc to only be called on this thread
    dwTimerProcThreadId = GetCurrentThreadId();

    Audio_Mute();

    // Again for user mute if it's initially set
    if(bUserMute == TRUE)
    {
        Audio_Mute();
    }
}


void Audio_Mute(DWORD PostMuteDelay)
{
    EnterCriticalSection(&AudioMuteCriticalSection);

    // Multi-state muting and the handling PostMuteDelay
    // and PreMuteDelay within these two simple
    // asynchronous safe functions alleviates the need
    // for complex and messy checks elsewhere in the program.
    if(++AudioMuteStatus == 1)
    {
	    if(Mixer_IsEnabled())
	    {
		    Mixer_Mute();
	    }

        if(!Mixer_IsNoHardwareMute() && Providers_GetCurrentSource())
        {
            Providers_GetCurrentSource()->Mute();
        }

	    EventCollector->RaiseEvent(NULL, EVENT_MUTE, 0, 1);

        if(PostMuteDelay > 0)
        {
            Sleep(PostMuteDelay);
        }
    }

    LeaveCriticalSection(&AudioMuteCriticalSection);

    LOG(2, " Mute Called Status on Exit %d", AudioMuteStatus);
}


void Audio_Unmute(DWORD PreUnmuteDelay)
{
    EnterCriticalSection(&AudioMuteCriticalSection);

    if(AudioMuteStatus > 0)
    {
        if(PreUnmuteDelay > 0)
        {
            // This timer, together with the multi-state audio
            // mute structure, automatically finds the longest
            // delay time for us if there are multiple unmutes
            // around the same time.
            if(GetCurrentThreadId() != dwTimerProcThreadId ||
                !SetTimer(NULL, NULL, PreUnmuteDelay, AudioUnmuteDelayTimerProc))
            {
                // Timer creation failed so do the unmute now.
                PreUnmuteDelay = 0;
            }
        }
        if(PreUnmuteDelay == 0)
        {
            if(--AudioMuteStatus == 0)
            {
                // Always unmute the hardware contrary to how it's
				// done for mute, because the hardware needs to be
				// unmuted from its initial mute state.
                if(Providers_GetCurrentSource())
                {
                    Providers_GetCurrentSource()->UnMute();
                }

				if(Mixer_IsEnabled())
		        {
			        Mixer_UnMute();
		        }	

                EventCollector->RaiseEvent(NULL, EVENT_MUTE, 1, 0);
            }
        }
    }

    LeaveCriticalSection(&AudioMuteCriticalSection);
    LOG(2, " UnMute Called Status on Exit %d", AudioMuteStatus);
}


BOOL Audio_IsMute()
{
    return (AudioMuteStatus > 0);
}


VOID CALLBACK AudioUnmuteDelayTimerProc(HWND hwnd, UINT, UINT idTimer, DWORD)
{
    KillTimer(hwnd, idTimer);
    Audio_Unmute(0UL);
}


void Audio_SetUserMute(BOOL bMute)
{
    if(bUserMute != bMute)
    {
        bUserMute = bMute;
        if(bMute == TRUE)
        {
            Audio_Mute();
        }
        else
        {
            Audio_Unmute();
        }
    }
}


BOOL Audio_GetUserMute()
{
    return bUserMute;
}


BOOL UserMute_OnChange(long NewValue)
{
    Audio_SetUserMute(NewValue);
	return FALSE;
}


////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING AudioSettings[AUDIO_SETTING_LASTONE] =
{
    {
        "System in Mute", ONOFF, 0, (long*)&bUserMute,
        FALSE, 0, 1, 1, 1, 
        NULL,
        "Audio", "Mute", UserMute_OnChange,
    },
};

SETTING* Audio_GetSetting(AUDIO_SETTING Setting)
{
    if(Setting > -1 && Setting < AUDIO_SETTING_LASTONE)
    {
        return &(AudioSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void Audio_ReadSettingsFromIni()
{
    int i;
    for(i = 0; i < AUDIO_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(AudioSettings[i]));
    }
}

void Audio_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < AUDIO_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(AudioSettings[i]), bOptimizeFileAccess);
    }
}

void Audio_SetMenu(HMENU hMenu)
{
    CheckMenuItemBool(hMenu, IDM_MUTE, bUserMute);
}

