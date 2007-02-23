/////////////////////////////////////////////////////////////////////////////
// $Id: CX2388xSource.cpp,v 1.79 2007-02-23 17:46:35 to_see Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 John Adcock.  All rights reserved.
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
//
// This code is based on a version of dTV modified by Michael Eskin and
// others at Connexant.  Those parts are probably (c) Connexant 2002
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.78  2007/02/18 21:32:44  robmuller
// Added option to not compile cx2388x code.
//
// Revision 1.77  2007/02/18 21:15:31  robmuller
// Added option to not compile BT8x8 code.
//
// Revision 1.76  2005/08/15 19:17:48  to_see
// Corrected Logging in StartStopConexantDriver
//
// Revision 1.75  2005/07/26 22:19:31  laurentg
// Use the new function Channel_GetVBIName
//
// Revision 1.74  2005/03/29 13:07:00  adcockj
// Avoid tight loops in processing
//
// Revision 1.73  2005/03/23 14:20:36  adcockj
// Test fix for threading issues
//
// Revision 1.72  2005/03/11 14:54:39  adcockj
// Get rid of a load of compilation warnings in vs.net
//
// Revision 1.71  2004/12/25 22:40:18  to_see
// Changed the card list to an ini file
//
// Revision 1.70  2004/11/13 21:45:56  to_see
// - Some minor fixes
// - Added "Vertical Sync Detection" in CX2388x Advanced Settings, enabled by default.
//   It reduces dead lock problems dramaticaly if no video signal is present. Faster videosignal detection.
//
// Revision 1.69  2004/07/10 11:57:17  adcockj
// improved cx2388x driver coverage when disabling drivers
//
// Revision 1.68  2004/06/19 20:13:47  to_see
// Faster and better A2 Stereo Detection
//
// Revision 1.67  2004/06/01 20:04:51  to_see
// some minor audio fixes
//
// Revision 1.66  2004/05/21 18:35:59  to_see
// Bugfix: Moved StartStopConexantDriver code from CX2388xCard to CCX2388xSource that the driver is stoped before CCX2388xCard::InitTuner is called.
//
// More Loging when StartStopConexantDriver is not able to Stop the WDM-Driver when other SW uses the card Hardware.
//
// Revision 1.65  2004/04/21 20:20:19  to_see
// Bugfix for Nicam
//
// Revision 1.64  2004/03/07 12:20:12  to_see
// added 2 Cards
// working Nicam-Sound
// Submenus in CX-Card for Soundsettings
// Click in "Autodetect" in "Setup card CX2388x" is now working
// added "Automute if no Tunersignal" in CX2388x Advanced
//
// Revision 1.63  2004/02/29 19:41:45  to_see
// new Submenu's in CX Card for Audio Channel and Audio Standard
// new AutoMute entry
//
// Revision 1.62  2004/02/27 20:50:59  to_see
// -more logging in CCX2388xCard::StartStopConexxantDriver
// -handling for IDC_AUTODETECT in CX2388xSource_UI.cpp
// -renamed eAudioStandard to eCX2388xAudioStandard,
//  eStereoType to eCX2388xStereoType and moved from
//  cx2388xcard.h to cx2388x_defines.h
// -moved Audiodetecting from CX2388xCard_Audio.cpp
//  to CX2388xSource_Audio.cpp
// -CCX2388xCard::AutoDetectTuner read
//  at first from Registers
//
// Revision 1.61  2004/02/21 14:11:30  to_see
// more A2-code
//
// Revision 1.60  2004/02/15 21:18:16  to_see
// New A2 code.
//
// Revision 1.59  2004/02/06 08:01:21  adcockj
// Fixed a couple of minor issues with Torsten's changes
//
// Revision 1.58  2004/02/05 21:47:52  to_see
// Starting/Stopping connexant-drivers while dscaler is running.
// To Enable/Disable it, go to Settings->Advanced Settings->
// CX2388X Advanced->Stopping Conexxant driver while Dscaler is running.
//
// This enables sound on my card without to go to windows control panel.
//
// Revision 1.57  2003/11/06 19:36:58  adcockj
// Increase size of vertical delay
//
// Revision 1.56  2003/10/27 10:39:51  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.55  2003/10/03 10:44:27  laurentg
// GetStatus enhanced to get the channel name from teletext when in tuner mode
//
// Revision 1.54  2003/08/15 18:21:26  laurentg
// Save in the source if it is the first setup
//
// Revision 1.53  2003/07/05 10:55:57  laurentg
// New method SetWidth
//
// Revision 1.52  2003/06/14 14:29:21  laurentg
// Video format saved per video input for BT8x8 and CX2388x
//
// Revision 1.51  2003/05/30 12:21:20  laurentg
// Don't forget to notify video format change if necessary when switching source and video input of destination source is tuner
//
// Revision 1.50  2003/05/29 17:07:27  laurentg
// no message
//
// Revision 1.49  2003/03/09 19:48:28  laurentg
// Updated field statistics
//
// Revision 1.48  2003/03/09 11:35:24  laurentg
// Judder Terminator - input timing slightly updated
//
// Revision 1.47  2003/03/08 20:01:25  laurentg
// New setting "always sleep"
//
// Revision 1.46  2003/02/26 20:53:30  laurentg
// New timing setting MaxFieldShift
//
// Revision 1.45  2003/02/22 13:42:42  laurentg
// New counter to count fields runnign late
// Update input frequency on cleanish field changes only which means when the field is no running late
//
// Revision 1.44  2003/02/16 10:11:10  laurentg
// White Crush ON by default (to solve flickering problem with XCapture)
//
// Revision 1.43  2003/02/03 11:08:07  laurentg
// Don't do the VBI shift line for 60 Hz video formats
//
// Revision 1.42  2003/01/25 23:46:25  laurentg
// Reset after the loading of the new settings in VideoFormatOnChange
//
// Revision 1.41  2003/01/21 14:42:14  adcockj
// Changed PAL defaults and added place for SECAM defaults
//
// Revision 1.40  2003/01/18 13:55:43  laurentg
// New methods GetHDelay and GetVDelay
//
// Revision 1.39  2003/01/18 10:52:11  laurentg
// SetOverscan renamed SetAspectRatioData
// Unnecessary call to SetOverscan deleted
// Overscan setting specific to calibration deleted
//
// Revision 1.38  2003/01/16 14:21:17  adcockj
// Added mute call for non-tuner inputs
//
// Revision 1.37  2003/01/16 13:30:49  adcockj
// Fixes for various settings problems reported by Laurent 15/Jan/2003
//
// Revision 1.36  2003/01/13 19:00:49  adcockj
// put reset in correct place
//
// Revision 1.35  2003/01/13 17:46:48  adcockj
// HDelay and VDelay turned from absolute to adjustments
//
// Revision 1.34  2003/01/12 21:19:18  adcockj
// Added Settings per flags to groups
//
// Revision 1.33  2003/01/12 20:10:49  adcockj
// Put analogue blanking setting in properly
//
// Revision 1.32  2003/01/12 16:19:33  adcockj
// Added SettingsGroup activity setting
// Corrected event sequence and channel change behaviour
//
// Revision 1.31  2003/01/11 15:22:25  adcockj
// Interim Checkin of setting code rewrite
//  - Remove CSettingsGroupList class
//  - Fixed bugs in format switching
//  - Some new CSettingGroup code
//
// Revision 1.30  2003/01/11 12:53:57  adcockj
// Interim Check in of settings changes
//  - bug fixes for overlay settings changes
//  - Bug fixes for new settings changes
//  - disables settings per channel completely
//
// Revision 1.29  2003/01/10 17:51:59  adcockj
// Removed SettingFlags
//
// Revision 1.28  2003/01/10 17:37:52  adcockj
// Interrim Check in of Settings rewrite
//  - Removed SETTINGSEX structures and flags
//  - Removed Seperate settings per channel code
//  - Removed Settings flags
//  - Cut away some unused features
//
// Revision 1.27  2003/01/08 19:59:36  laurentg
// Analogue Blanking setting by source
//
// Revision 1.26  2003/01/07 23:27:02  laurentg
// New overscan settings
//
// Revision 1.25  2003/01/07 16:49:07  adcockj
// Changes to allow variable sampling rates for VBI
//
// Revision 1.24  2003/01/05 19:01:13  adcockj
// Made some changes to Laurent's last set of VBI fixes
//
// Revision 1.23  2003/01/05 18:35:45  laurentg
// Init function for VBI added
//
// Revision 1.22  2003/01/05 16:54:54  laurentg
// Updated parameters for VBI_DecodeLine
//
// Revision 1.21  2003/01/04 20:58:59  laurentg
// Shift of one line when dealing with VBI line for CX2388x
//
// Revision 1.20  2002/12/23 17:22:10  adcockj
// Settings fixes
//
// Revision 1.19  2002/12/10 14:53:16  adcockj
// Sound fixes for cx2388x
//
// Revision 1.18  2002/12/10 12:58:07  adcockj
// Removed NotifyInputChange and NotifyVideoFormatChange functions and replaced with
//  calls to EventCollector->RaiseEvent
//
// Revision 1.17  2002/12/04 17:43:49  adcockj
// Contrast and Brightness adjustments so that h3d card behaves in expected way
//
// Revision 1.16  2002/12/03 07:56:31  adcockj
// Fixed some problems with settings not saving
//
// Revision 1.15  2002/12/02 13:47:01  adcockj
// Allow fine control over white crush settings
//
// Revision 1.14  2002/11/13 10:34:36  adcockj
// Improved pixel width support
//
// Revision 1.13  2002/11/12 15:22:47  adcockj
// Made new flag settings have default setting
// Added pixel width for CX2388x cards
//
// Revision 1.12  2002/11/12 09:18:29  adcockj
// Correct default for EatLinesAtTop
//
// Revision 1.11  2002/11/10 12:48:16  laurentg
// Order in CombFilterSzList updated
//
// Revision 1.10  2002/11/09 20:53:46  laurentg
// New CX2388x settings
//
// Revision 1.9  2002/11/09 00:22:23  laurentg
// New settings for CX2388x chip
//
// Revision 1.8  2002/11/07 20:33:17  adcockj
// Promoted ACPI functions so that state management works properly
//
// Revision 1.7  2002/11/06 20:14:51  adcockj
// Centered pixels to work with my equipment
//
// Revision 1.6  2002/11/06 11:11:23  adcockj
// Added new Settings and applied Laurent's filter setup suggestions
//
// Revision 1.5  2002/11/03 15:54:10  adcockj
// Added cx2388x register tweaker support
//
// Revision 1.4  2002/10/31 14:47:20  adcockj
// Added Sharpness
//
// Revision 1.3  2002/10/29 22:36:41  adcockj
// VBI fixes (still doesn't work)
//
// Revision 1.2  2002/10/29 22:00:30  adcockj
// Added EatlLinesAtTop setting for SDI on holo3d
//
// Revision 1.1  2002/10/29 11:05:28  adcockj
// Renamed CT2388x to CX2388x
//
// 
// CVS Log while file was called CT2388xSource.cpp
//
// Revision 1.22  2002/10/26 17:51:52  adcockj
// Simplified hide cusror code and removed PreShowDialogOrMenu & PostShowDialogOrMenu
//
// Revision 1.21  2002/10/24 16:04:47  adcockj
// Another attempt to get VBI working
// Tidy up CMDS/Buffers code
//
// Revision 1.20  2002/10/23 20:26:53  adcockj
// Bug fixes for cx2388x
//
// Revision 1.19  2002/10/23 15:18:07  adcockj
// Added preliminary code for VBI
//
// Revision 1.18  2002/10/22 18:52:18  adcockj
// Added ASPI support
//
// Revision 1.17  2002/10/22 04:08:50  flibuste2
// -- Modified CSource to include virtual ITuner* GetTuner();
// -- Modified HasTuner() and GetTunerId() when relevant
//
// Revision 1.16  2002/10/21 16:07:26  adcockj
// Added H & V delay options for CX2388x cards
//
// Revision 1.15  2002/10/21 07:19:33  adcockj
// Preliminary Support for PixelView XCapture
//
// Revision 1.14  2002/10/17 13:31:37  adcockj
// Give Holo3d different menu and updated settings
//
// Revision 1.13  2002/10/02 19:02:06  adcockj
// Faster film mode change
//
// Revision 1.12  2002/10/02 10:55:46  kooiman
// Fixed C++ type casting for events.
//
// Revision 1.11  2002/09/29 16:16:21  adcockj
// Holo3d imrprovements
//
// Revision 1.10  2002/09/29 13:53:40  adcockj
// Ensure Correct History stored
//
// Revision 1.9  2002/09/29 10:14:14  adcockj
// Fixed problem with history in OutThreads
//
// Revision 1.8  2002/09/28 13:33:04  kooiman
// Added sender object to events and added setting flag to treesettingsgeneric.
//
// Revision 1.7  2002/09/26 11:33:42  kooiman
// Use event collector
//
// Revision 1.6  2002/09/25 15:11:12  adcockj
// Preliminary code for format specific support for settings per channel
//
// Revision 1.5  2002/09/22 17:47:04  adcockj
// Fixes for holo3d
//
// Revision 1.4  2002/09/16 20:08:21  adcockj
// fixed format detect for cx2388x
//
// Revision 1.3  2002/09/16 19:34:19  adcockj
// Fix for auto format change
//
// Revision 1.2  2002/09/15 14:20:38  adcockj
// Fixed timing problems for cx2388x chips
//
// Revision 1.1  2002/09/11 18:19:37  adcockj
// Prelimainary support for CX2388x based cards
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file CX2388xSource.cpp CCX2388xSource Implementation
 */

#include "stdafx.h"

#ifdef WANT_CX2388X_SUPPORT

#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "CX2388xSource.h"
#include "DScaler.h"
#include "VBI.h"
#include "VBI_VideoText.h"
#include "VBI_VPSdecode.h"
#include "Audio.h"
#include "VideoSettings.h"
#include "OutThreads.h"
#include "OSD.h"
#include "Status.h"
#include "FieldTiming.h"
#include "ProgramList.h"
#include "CX2388x_Defines.h"
#include "FD_60Hz.h"
#include "FD_50Hz.h"
#include "DebugLog.h"
#include "AspectRatio.h"
#include "SettingsMaster.h"
#include "SettingsPerChannel.h"
#include "Providers.h"
#include "SoundChannel.h"
#include <setupapi.h>	// for Start/Stopping driver
#include <devguid.h>	// for Start/Stopping driver, define GUID_DEVCLASS_MEDIA

extern long EnableCxCancelButton;

const char* CombFilterSzList[] =
{
    { "Default" 			},
    { "Off"					},
    { "Chroma comb only"    },
    { "Full Comb"			},
};

const char* DefaultOffOnSzList[] =
{
    { "Default"				},
    { "Force Off"           },
    { "Force On"			},
};

const char* WhiteCrushMajorityPointList[] =
{
    { "3/4"			},
    { "1/2"         },
    { "1/4"			},
    { "Automatic"   },
};

const char* AudioStandardList[] =
{
    { "Auto"        },
    { "BTSC"        },
    { "EIAJ"        },
    { "A2"			},
    { "BTSC-SAP"    },
    { "NICAM"       },
    { "FM"          },
};

const char* StereoTypeList[] =
{
    { "Auto"        },
    { "Stereo"      },
    { "Mono"		},
    { "Alt1"        },
    { "Alt2"        },
};

CCX2388xSource::CCX2388xSource(CCX2388xCard* pCard, CContigMemory* RiscDMAMem, CUserMemory* DisplayDMAMem[5], CUserMemory* VBIDMAMem[5], LPCSTR IniSection) :
    CSource(WM_CX2388X_GETVALUE, IDC_CX2388X),
    m_pCard(pCard),
    m_CurrentX(720),
    m_CurrentY(480),
    m_Section(IniSection),
    m_IDString(IniSection),
    m_CurrentVBILines(19),
    m_IsFieldOdd(FALSE),
    m_InSaturationUpdate(FALSE),
    m_NumFields(10),
    m_hCX2388xResourceInst(NULL),
	m_InitialSetup(FALSE),
	m_AutoDetectA2Counter(0),
	m_bDriverStoped(FALSE)
{
    CreateSettings(IniSection);

    DisableOnChange();

    eEventType EventList[] = {EVENT_CHANNEL_PRECHANGE,EVENT_CHANNEL_CHANGE,EVENT_ENDOFLIST};
    EventCollector->Register(this, EventList);
   

    m_RiscBaseLinear = (DWORD*)RiscDMAMem->GetUserPointer();
    m_RiscBasePhysical = RiscDMAMem->TranslateToPhysical(m_RiscBaseLinear, 83968, NULL);
    for(int i(0); i < 5; ++i)
    {
        m_pDisplay[i] = (BYTE*)DisplayDMAMem[i]->GetUserPointer();
        m_DisplayDMAMem[i] = DisplayDMAMem[i];
        m_pVBILines[i] = (BYTE*)VBIDMAMem[i]->GetUserPointer();
        m_VBIDMAMem[i] = VBIDMAMem[i];
    }

    // loads up core settings like card and tuner type
    ReadFromIni();
    
    SetupCard();

    InitializeUI();
}


CCX2388xSource::~CCX2388xSource()
{
	StopUpdateAudioStatus();

    // CX2388x reserves input -1 as the clean up indicator
    m_pCard->SetVideoSource(-1);

	EventCollector->Unregister(this);
    delete m_pCard;

	if(m_bDriverStoped == TRUE)
	{
		StartStopConexantDriver(DICS_ENABLE);
	}
}

void CCX2388xSource::SetSourceAsCurrent()
{
    // need to call up to parent to run register settings functions
    CSource::SetSourceAsCurrent();

    // tell the rest of DScaler what the setup is
    // A side effect of the Raise events is to tell the settings master
    // what our current setup is
    EventCollector->RaiseEvent(this, EVENT_VIDEOINPUT_CHANGE, -1, m_VideoSource->GetValue());
    EventCollector->RaiseEvent(this, EVENT_VIDEOFORMAT_CHANGE, -1, m_VideoFormat->GetValue());
    EventCollector->RaiseEvent(this, EVENT_VOLUME, 0, m_Volume->GetValue());

    int OldFormat = m_VideoFormat->GetValue();

    // reset the tuner
    // this will kick of the change channel event
    // which must happen after the video input event
    if(IsInTunerMode())
    {
        Channel_Reset();
    }
	else
	{
		// We read what is the video format saved for this video input
	    SettingsMaster->LoadOneSetting(m_VideoFormat);
	}

    // tell the world if the format has changed
    if(OldFormat != m_VideoFormat->GetValue())
    {
        EventCollector->RaiseEvent(this, EVENT_VIDEOFORMAT_PRECHANGE, OldFormat, m_VideoFormat->GetValue());
        EventCollector->RaiseEvent(this, EVENT_VIDEOFORMAT_CHANGE, OldFormat, m_VideoFormat->GetValue());

		// We save the video format attached to this video input
	    SettingsMaster->WriteOneSetting(m_VideoFormat);
    }

    // make sure the defaults are correct
    // but don't change the values
    ChangeDefaultsForSetup(SETUP_CHANGE_ANY, TRUE);

    SettingsMaster->LoadSettings();

    Reset();
}

void CCX2388xSource::OnEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp)
{
}

void CCX2388xSource::SetupPictureStructures()
{
    if(m_IsVideoProgressive->GetValue())
    {
        // Set up 5 sets of pointers to the start of pictures
        for (int j(0); j < 5; j++)
        {
            m_EvenFields[j].pData = m_pDisplay[j];
            m_EvenFields[j].Flags = PICTURE_PROGRESSIVE;
            m_EvenFields[j].IsFirstInSeries = FALSE;
        }
    }
    else
    {
        // Set up 5 sets of pointers to the start of odd and even lines
        for (int j(0); j < 5; j++)
        {
            m_OddFields[j].pData = m_pDisplay[j] + 2048;
            m_OddFields[j].Flags = PICTURE_INTERLACED_ODD;
            m_OddFields[j].IsFirstInSeries = FALSE;
            m_EvenFields[j].pData = m_pDisplay[j];
            m_EvenFields[j].Flags = PICTURE_INTERLACED_EVEN;
            m_EvenFields[j].IsFirstInSeries = FALSE;
        }
    }
}

void CCX2388xSource::CreateSettings(LPCSTR IniSection)
{
    CSettingGroup *pVideoFormatGroup = GetSettingsGroup("CX2388x - Video Format", SETTING_BY_INPUT, TRUE);
    CSettingGroup *pVideoGroup = GetSettingsGroup("CX2388x - Video", SETTING_BY_CHANNEL | SETTING_BY_FORMAT | SETTING_BY_INPUT, TRUE);
    CSettingGroup *pH3DGroup = GetSettingsGroup("CX2388x - H3D", SETTING_BY_FORMAT | SETTING_BY_INPUT);
    CSettingGroup *pAudioGroup = GetSettingsGroup("CX2388x - Audio", SETTING_BY_CHANNEL, FALSE);

    m_Brightness = new CBrightnessSetting(this, "Brightness", 128, 0, 255, IniSection, pVideoGroup);
    m_Settings.push_back(m_Brightness);

    m_Contrast = new CContrastSetting(this, "Contrast", 128, 0, 255, IniSection, pVideoGroup);
    m_Settings.push_back(m_Contrast);

    m_Hue = new CHueSetting(this, "Hue", 128, 0, 255, IniSection, pVideoGroup);
    m_Settings.push_back(m_Hue);

    m_Saturation = new CSaturationSetting(this, "Saturation", 128, 0, 255, IniSection, pVideoGroup);
    m_Settings.push_back(m_Saturation);

    m_SaturationU = new CSaturationUSetting(this, "Blue Saturation", 128, 0, 255, IniSection, pVideoGroup);
    m_Settings.push_back(m_SaturationU);

    m_SaturationV = new CSaturationVSetting(this, "Red Saturation", 128, 0, 255, IniSection, pVideoGroup);
    m_Settings.push_back(m_SaturationV);

    m_TopOverscan = new CTopOverscanSetting(this, "Overscan at Top", DEFAULT_OVERSCAN_NTSC, 0, 150, IniSection, pVideoGroup);
    m_Settings.push_back(m_TopOverscan);

    m_VideoSource = new CVideoSourceSetting(this, "Video Source", 0, 0, 7, IniSection);
    m_Settings.push_back(m_VideoSource);

    m_VideoFormat = new CVideoFormatSetting(this, "Video Format", VIDEOFORMAT_NTSC_M, 0, VIDEOFORMAT_LASTONE - 1, IniSection, pVideoFormatGroup);
    m_Settings.push_back(m_VideoFormat);

    m_CardType = new CSliderSetting("Card Type", CX2388xCARD_UNKNOWN, CX2388xCARD_UNKNOWN, m_pCard->GetMaxCards() - 1, IniSection, "CardType");
    m_Settings.push_back(m_CardType);

    m_TunerType = new CTunerTypeSetting(this, "Tuner Type", TUNER_ABSENT, TUNER_ABSENT, TUNER_LASTONE - 1, IniSection);
    m_Settings.push_back(m_TunerType);

    // save per input removed
    m_Settings.push_back(NULL);
    
    // save per format removed
    m_Settings.push_back(NULL);

    // save per channel removed
    m_Settings.push_back(NULL);

    m_IsVideoProgressive = new CIsVideoProgressiveSetting(this, "Is Video Progressive", FALSE, IniSection, pH3DGroup);
    m_Settings.push_back(m_IsVideoProgressive);

    m_FLIFilmDetect = new CFLIFilmDetectSetting(this, "FLI Film Detect", TRUE, IniSection, pH3DGroup);
    m_Settings.push_back(m_FLIFilmDetect);

    m_HDelay = new CHDelaySetting(this, "Horizontal Delay Adjustment", 0, -12, 12, IniSection, pVideoGroup);
    m_HDelay->SetStepValue(2);
    m_Settings.push_back(m_HDelay);

    m_VDelay = new CVDelaySetting(this, "Vertical Delay Adjustment", 0, -40, 40, IniSection, pVideoGroup);
    m_VDelay->SetStepValue(4);
    m_Settings.push_back(m_VDelay);

    m_EatLinesAtTop = new CEatLinesAtTopSetting(this, "Eat Lines At Top", 12, 0, 100, IniSection, pH3DGroup);
    m_Settings.push_back(m_EatLinesAtTop);
    
    m_Sharpness = new CSharpnessSetting(this, "Sharpness", 0, -8, 7, IniSection, pH3DGroup);
    m_Settings.push_back(m_Sharpness);

    m_LumaAGC = new CLumaAGCSetting(this, "Luma AGC", FALSE, IniSection, pVideoGroup);
    m_Settings.push_back(m_LumaAGC);

    m_ChromaAGC = new CChromaAGCSetting(this, "Chroma AGC", FALSE, IniSection, pVideoGroup);
    m_Settings.push_back(m_ChromaAGC);

    m_FastSubcarrierLock = new CFastSubcarrierLockSetting(this, "Fast Subcarrier Lock", FALSE, IniSection, pVideoGroup);
    m_Settings.push_back(m_FastSubcarrierLock);

    m_WhiteCrush = new CWhiteCrushSetting(this, "White Crush", TRUE, IniSection, pVideoGroup);
    m_Settings.push_back(m_WhiteCrush);

    m_LowColorRemoval = new CLowColorRemovalSetting(this, "Low Color Removal", FALSE, IniSection, pVideoGroup);
    m_Settings.push_back(m_LowColorRemoval);

    m_CombFilter = new CCombFilterSetting(this, "Comb Filter", CCX2388xCard::COMBFILTER_DEFAULT, CCX2388xCard::COMBFILTER_FULL, IniSection, CombFilterSzList, pVideoGroup);
    m_Settings.push_back(m_CombFilter);

    m_FullLumaRange = new CFullLumaRangeSetting(this, "Full Luma Range", FALSE, IniSection, pVideoGroup);
    m_Settings.push_back(m_FullLumaRange);

    m_Remodulation = new CRemodulationSetting(this, "Remodulation", CCX2388xCard::FLAG_DEFAULT, CCX2388xCard::FLAG_ON, IniSection, DefaultOffOnSzList, pVideoGroup);
    m_Settings.push_back(m_Remodulation);

    m_Chroma2HComb = new CChroma2HCombSetting(this, "Chroma 2H Comb", CCX2388xCard::FLAG_DEFAULT, CCX2388xCard::FLAG_ON, IniSection, DefaultOffOnSzList, pVideoGroup);
    m_Settings.push_back(m_Chroma2HComb);

    m_ForceRemodExcessChroma = new CForceRemodExcessChromaSetting(this, "Force Remodulation of Excess Chroma", CCX2388xCard::FLAG_DEFAULT, CCX2388xCard::FLAG_ON, IniSection, DefaultOffOnSzList, pVideoGroup);
    m_Settings.push_back(m_ForceRemodExcessChroma);

    m_IFXInterpolation = new CIFXInterpolationSetting(this, "IFX Interpolation", CCX2388xCard::FLAG_DEFAULT, CCX2388xCard::FLAG_ON, IniSection, DefaultOffOnSzList, pVideoGroup);
    m_Settings.push_back(m_IFXInterpolation);

    m_CombRange = new CCombRangeSetting(this, "Adaptative Comb Filter Threshold", 0x01f, 0, 0x3ff, IniSection, pVideoGroup);
    m_Settings.push_back(m_CombRange);

    m_SecondChromaDemod = new CSecondChromaDemodSetting(this, "Second Chroma Demodulation", CCX2388xCard::FLAG_DEFAULT, CCX2388xCard::FLAG_ON, IniSection, DefaultOffOnSzList, pVideoGroup);
    m_Settings.push_back(m_SecondChromaDemod);

    m_ThirdChromaDemod = new CThirdChromaDemodSetting(this, "Third Chroma Demodulation", CCX2388xCard::FLAG_DEFAULT, CCX2388xCard::FLAG_ON, IniSection, DefaultOffOnSzList, pVideoGroup);
    m_Settings.push_back(m_ThirdChromaDemod);

    m_PixelWidth = new CPixelWidthSetting(this, "Sharpness", 720, 120, DSCALER_MAX_WIDTH, IniSection, pVideoGroup);
    m_PixelWidth->SetStepValue(2);
    m_Settings.push_back(m_PixelWidth);

    m_CustomPixelWidth = new CSliderSetting("Custom Pixel Width", 750, 120, DSCALER_MAX_WIDTH, IniSection, "CustomPixelWidth", pVideoGroup);
    m_CustomPixelWidth->SetStepValue(2);
    m_Settings.push_back(m_CustomPixelWidth);

    m_WhiteCrushUp = new CWhiteCrushUpSetting(this, "White Crush Up", 15, 0, 63, IniSection, pVideoGroup);
    m_Settings.push_back(m_WhiteCrushUp);

    m_WhiteCrushDown = new CWhiteCrushDownSetting(this, "White Crush Down", 63, 0, 63, IniSection, pVideoGroup);
    m_Settings.push_back(m_WhiteCrushDown);

    m_WhiteCrushMajorityPoint = new CWhiteCrushMajorityPointSetting(this, "White Crush Majority Point", CCX2388xCard::MAJSEL_AUTOMATIC, CCX2388xCard::MAJSEL_AUTOMATIC, IniSection, WhiteCrushMajorityPointList, pVideoGroup);
    m_Settings.push_back(m_WhiteCrushMajorityPoint);

    m_WhiteCrushPerFrame = new CWhiteCrushPerFrameSetting(this, "White Crush Per Frame", TRUE, IniSection, pVideoGroup);
    m_Settings.push_back(m_WhiteCrushPerFrame);

    m_Volume = new CVolumeSetting(this, "Volume", 900, 0, 1000, IniSection, pAudioGroup);
    m_Volume->SetStepValue(20);
    m_Settings.push_back(m_Volume);

    m_Balance = new CBalanceSetting(this, "Balance", 0, -127, 127, IniSection, pAudioGroup);
    m_Settings.push_back(m_Balance);

    m_AudioStandard = new CAudioStandardSetting(this, "Audio Standard", AUDIO_STANDARD_AUTO, AUDIO_STANDARD_FM, IniSection, AudioStandardList, pAudioGroup);
    m_Settings.push_back(m_AudioStandard);

    m_StereoType = new CStereoTypeSetting(this, "Stereo Type", STEREOTYPE_AUTO, STEREOTYPE_ALT2, IniSection, StereoTypeList, pAudioGroup);
    m_Settings.push_back(m_StereoType);

    m_BottomOverscan = new CBottomOverscanSetting(this, "Overscan at Bottom", DEFAULT_OVERSCAN_NTSC, 0, 150, IniSection, pVideoGroup);
    m_Settings.push_back(m_BottomOverscan);

    m_LeftOverscan = new CLeftOverscanSetting(this, "Overscan at Left", DEFAULT_OVERSCAN_NTSC, 0, 150, IniSection, pVideoGroup);
    m_Settings.push_back(m_LeftOverscan);

    m_RightOverscan = new CRightOverscanSetting(this, "Overscan at Right", DEFAULT_OVERSCAN_NTSC, 0, 150, IniSection, pVideoGroup);
    m_Settings.push_back(m_RightOverscan);

    m_AnalogueBlanking = new CAnalogueBlankingSetting(this, "Analogue Blanking", FALSE, IniSection, pVideoGroup);
    m_Settings.push_back(m_AnalogueBlanking);

    m_ConexantStopDriver = new CConexantStopDriverSetting(this, "Stop Conexant driver while DScaler is running", FALSE, IniSection, pAudioGroup);
    m_Settings.push_back(m_ConexantStopDriver);

    m_AutoMute = new CAutoMuteSetting(this, "Automute if no Tunersignal", TRUE, IniSection, pVideoGroup);
    m_Settings.push_back(m_AutoMute);

    m_VerticalSyncDetection = new CVerticalSyncDetectionSetting(this, "Vertical Sync Detection", TRUE, IniSection, pVideoGroup);
    m_Settings.push_back(m_VerticalSyncDetection);

	m_CardName = new CStringSetting("Card Name", reinterpret_cast<long>(""), IniSection, "CardName");
	m_Settings.push_back(m_CardName);

#ifdef _DEBUG    
    if (CX2388X_SETTING_LASTONE != m_Settings.size())
    {
        LOGD("Number of settings in CX2388X source is not equal to the number of settings in DS_Control.h");
        LOGD("DS_Control.h or CX2388xSource.cpp are probably not in sync with eachother.");
    }
#endif
}

void CCX2388xSource::Start()
{
    m_pCard->StopCapture();
    CreateRiscCode(bCaptureVBI && (m_CurrentVBILines > 0));
    // only capture VBI if we are expecting them
    m_pCard->StartCapture(bCaptureVBI && (m_CurrentVBILines > 0));
    
	// This timer is used to update audiostatus & automute
	StartUpdateAudioStatus();
    
	Timing_Reset();
    NotifySizeChange();
    NotifySquarePixelsCheck();

    if(m_CurrentX == 720)
    {
    	VBI_Init_data(27.0);
    }
    else
    {
        // we use  only two different sampling rates which are based off the pal and ntsc fsc values
        VBI_Init_data(GetTVFormat((eVideoFormat)m_VideoFormat->GetValue())->Bt848VBISamplingFrequency);
    }
    EnableOnChange();
}

void CCX2388xSource::Reset()
{
    m_pCard->ResetHardware();
    m_pCard->SetVideoSource(m_VideoSource->GetValue());

    m_pCard->SetContrastBrightness((BYTE)m_Contrast->GetValue(), (BYTE)m_Brightness->GetValue());
    m_pCard->SetHue((BYTE)m_Hue->GetValue());
    m_pCard->SetSaturationU((BYTE)m_SaturationU->GetValue());
    m_pCard->SetSaturationV((BYTE)m_SaturationV->GetValue());

    m_CurrentX = m_PixelWidth->GetValue();
    m_pCard->SetGeoSize(
                            m_VideoSource->GetValue(), 
                            (eVideoFormat)m_VideoFormat->GetValue(), 
                            m_CurrentX, 
                            m_CurrentY, 
                            m_CurrentVBILines,
                            m_IsVideoProgressive->GetValue()
                        );
    
    m_pCard->SetHDelay(
                            m_VideoSource->GetValue(), 
                            (eVideoFormat)m_VideoFormat->GetValue(), 
                            m_CurrentX, 
                            m_HDelay->GetValue()
                      );
    
    m_pCard->SetVDelay(
                            m_VideoSource->GetValue(), 
                            (eVideoFormat)m_VideoFormat->GetValue(), 
                            m_CurrentX, 
                            m_VDelay->GetValue() 
                      );

    if(IsInTunerMode())
    {
        m_pCard->AudioInit(
                            m_VideoSource->GetValue(), 
                            (eVideoFormat)m_VideoFormat->GetValue(), 
                            (eCX2388xAudioStandard)m_AudioStandard->GetValue(),
                            (eCX2388xStereoType)m_StereoType->GetValue()
                          );
    }
    else
    {
        Mute();
    }

    if(m_pCard->IsThisCardH3D((eCX2388xCardId)m_CardType->GetValue()))
    {
        m_pCard->SetFLIFilmDetect(m_FLIFilmDetect->GetValue());
        m_pCard->SetSharpness((char)m_Sharpness->GetValue());
    }
    
    else
    {
		m_pCard->SetLumaAGC(m_LumaAGC->GetValue());
        m_pCard->SetChromaAGC(m_ChromaAGC->GetValue());
        m_pCard->SetFastSubcarrierLock(m_FastSubcarrierLock->GetValue());
        m_pCard->SetWhiteCrushEnable(m_WhiteCrush->GetValue());
        m_pCard->SetLowColorRemoval(m_LowColorRemoval->GetValue());
        m_pCard->SetCombFilter((CCX2388xCard::eCombFilter)(m_CombFilter->GetValue()));
        m_pCard->SetFullLumaRange(m_FullLumaRange->GetValue());
        m_pCard->SetRemodulation((CCX2388xCard::eFlagWithDefault)m_Remodulation->GetValue());
        m_pCard->SetChroma2HComb((CCX2388xCard::eFlagWithDefault)m_Chroma2HComb->GetValue());
        m_pCard->SetForceRemodExcessChroma((CCX2388xCard::eFlagWithDefault)m_ForceRemodExcessChroma->GetValue());
        m_pCard->SetIFXInterpolation((CCX2388xCard::eFlagWithDefault)m_IFXInterpolation->GetValue());
        m_pCard->SetCombRange(m_CombRange->GetValue());
        m_pCard->SetSecondChromaDemod((CCX2388xCard::eFlagWithDefault)m_SecondChromaDemod->GetValue());
        m_pCard->SetThirdChromaDemod((CCX2388xCard::eFlagWithDefault)m_ThirdChromaDemod->GetValue());
        m_pCard->SetWhiteCrushUp((BYTE)m_WhiteCrushUp->GetValue());
        m_pCard->SetWhiteCrushDown((BYTE)m_WhiteCrushDown->GetValue());
        m_pCard->SetWhiteCrushMajorityPoint((CCX2388xCard::eWhiteCrushMajSel)m_WhiteCrushMajorityPoint->GetValue());
        m_pCard->SetWhiteCrushPerFrame(m_WhiteCrushPerFrame->GetValue());
        m_pCard->SetVerticalSyncDetection(m_VerticalSyncDetection->GetValue());
    }
    NotifySizeChange();
}

void CCX2388xSource::CreateRiscCode(BOOL bCaptureVBI)
{
    DWORD *pRiscCode;    // For host memory version
    int nField;
    int nLine;
    LPBYTE pUser;
    DWORD pPhysical;
    DWORD GotBytesPerLine;
    DWORD BytesPerLine = 0;
    int NumLines;
    DWORD BytesToSkip;
    BOOL IsVideo480P = m_IsVideoProgressive->GetValue();

    SetupPictureStructures();

    pRiscCode = (DWORD*)m_RiscBaseLinear;

    // we create the RISC code for 10 fields
    // the first one (0) is even
    // last one (9) is odd
    if(IsVideo480P)
    {
        if(m_CurrentY == 576)
        {
            m_NumFields = 4;
        }
        else
        {
            m_NumFields = 5;
        }
    }
    else
    {
        m_NumFields = 10;
    }
    for (nField = 0; nField < m_NumFields; nField++)
    {
        DWORD Instruction(0);

        if(IsVideo480P)
        {
            Instruction = RISC_RESYNC;
        }
        else
        {
            // First we sync onto either the odd or even field (odd only for 480p)
            if (nField & 1)
            {
                Instruction = RISC_RESYNC_EVEN;
            }
            else
            {
                Instruction = RISC_RESYNC_ODD;
            }
        }

        // maintain counter that we use to tell us where we are
        // in the RISC code
        if(nField == 0)
        {
            Instruction |= RISC_CNT_RESET;
        }
        else
        {
            Instruction |= RISC_CNT_INC;
        }

        *(pRiscCode++) = Instruction;

        // work out the position of the first line
        // first line is line zero an even line
        if (IsVideo480P)
        {
            pUser = m_pDisplay[nField];
        }
        else
        {
            pUser = m_pDisplay[nField / 2];
            if(nField & 1)
            {
                pUser += 2048;
            }
        }
        BytesPerLine = m_CurrentX * 2;

        // if it's the holo3d and we're on the sdi input
        // in non progressive mode then we need to adjust 
        // for the image shift that seems to happen
        if(m_pCard->IsThisCardH3D((eCX2388xCardId)m_CardType->GetValue()))
        {
            if(m_VideoSource->GetValue() == 3 &&
                m_IsVideoProgressive->GetValue() == FALSE)
            {
                for (nLine = 0; nLine < m_EatLinesAtTop->GetValue(); nLine++)
                {
                    *(pRiscCode++) = RISC_SKIP | RISC_SOL | RISC_EOL | BytesPerLine;
                }
            }
        }

        //
        // For 480p, we will do the full frame in sequential order. For 480i, we
        // do one (even or odd) field at a time and skip over the interlaced lines
        //
        NumLines = (IsVideo480P ? m_CurrentY : (m_CurrentY / 2));
        BytesToSkip = (IsVideo480P ? 2048 : 4096);

        for (nLine = 0; nLine < NumLines; nLine++)
        {
            if (IsVideo480P)
            {
                pPhysical = m_DisplayDMAMem[nField]->TranslateToPhysical(pUser, BytesPerLine, &GotBytesPerLine);
            }
            else
            {
                pPhysical = m_DisplayDMAMem[nField / 2]->TranslateToPhysical(pUser, BytesPerLine, &GotBytesPerLine);
            }
            if(pPhysical == 0 || BytesPerLine > GotBytesPerLine)
            {
                ErrorBox("GetPhysicalAddress failed during RISC build");
                return;
            }
            *(pRiscCode++) = RISC_WRITE | RISC_SOL | RISC_EOL | BytesPerLine;
            *(pRiscCode++) = pPhysical;
            // since we are doing all the lines of the same
            // polarity at the same time we skip two lines
            pUser += BytesToSkip;
        }
    }

    m_BytesPerRISCField = ((DWORD)pRiscCode - (DWORD)m_RiscBaseLinear) / m_NumFields;

    *(pRiscCode++) = RISC_JUMP;

    *(pRiscCode++) = m_RiscBasePhysical; 

    m_pCard->SetRISCStartAddress(m_RiscBasePhysical);

    // attempt to do VBI
    // for this chip I think we need a seperate RISC program for VBI
    // so we'll tag the VBI program at the end
    if(bCaptureVBI == TRUE && IsVideo480P == FALSE)
    {
        // work out the physical start position of the VBI program
        m_RiscBasePhysicalVBI = m_RiscBasePhysical + ((DWORD)pRiscCode - (DWORD)m_RiscBaseLinear);

        m_NumFields = 10;
        for (nField = 0; nField < m_NumFields; nField++)
        {
            DWORD Instruction(0);

            // First we sync onto either the odd or even field
            if (nField & 1)
            {
                Instruction = RISC_RESYNC_EVEN;
            }
            else
            {
                Instruction = RISC_RESYNC_ODD;
            }

            *(pRiscCode++) = Instruction;

            // skip the first line
            // so that the line numbers tie up with those for
            // the bt848
			// It seems to be necessary only for 50 Hz video format
			if (GetTVFormat((eVideoFormat)m_VideoFormat->GetValue())->Is25fps)
			{
	            *(pRiscCode++) = RISC_SKIP | RISC_SOL | RISC_EOL | VBI_SPL;
			}

            pUser = m_pVBILines[nField / 2];
            if((nField & 1) == 1)
            {
                pUser += m_CurrentVBILines * 2048;
            }
            for (nLine = 0; nLine < m_CurrentVBILines; nLine++)
            {
                pPhysical = m_VBIDMAMem[nField / 2]->TranslateToPhysical(pUser, VBI_SPL, &GotBytesPerLine);
                if(pPhysical == 0 || VBI_SPL > GotBytesPerLine)
                {
                    return;
                }
                *(pRiscCode++) = RISC_WRITE | RISC_SOL | RISC_EOL | VBI_SPL;
                *(pRiscCode++) = pPhysical;
                pUser += 2048;
            }
        }

        // jump back to start
        *(pRiscCode++) = RISC_JUMP;
        *(pRiscCode++) = m_RiscBasePhysicalVBI; 

        m_pCard->SetRISCStartAddressVBI(m_RiscBasePhysicalVBI);
    }
}


void CCX2388xSource::Stop()
{
    // disable OnChange messages while video is stopped
    DisableOnChange();
    // stop capture
    m_pCard->StopCapture();
	StopUpdateAudioStatus();
}

void CCX2388xSource::GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming)
{
    if(m_IsVideoProgressive->GetValue())
    {
        if(AccurateTiming)
        {
            GetNextFieldAccurateProg(pInfo);
        }
        else
        {
            GetNextFieldNormalProg(pInfo);
        }
    }
    else
    {
        if(AccurateTiming)
        {
            GetNextFieldAccurate(pInfo);
        }
        else
        {
            GetNextFieldNormal(pInfo);
        }
    }

    ShiftPictureHistory(pInfo, m_NumFields);

    if(m_IsVideoProgressive->GetValue())
    {
        pInfo->PictureHistory[0] = &m_EvenFields[pInfo->CurrentFrame];

        pInfo->LineLength = m_CurrentX * 2;
        pInfo->FrameWidth = m_CurrentX;
        pInfo->FrameHeight = m_CurrentY;
        pInfo->FieldHeight = m_CurrentY;
        pInfo->InputPitch = 2048;

    }
    else
    {
        if(m_IsFieldOdd)
        {
            pInfo->PictureHistory[0] = &m_OddFields[pInfo->CurrentFrame];
        }
        else
        {
            pInfo->PictureHistory[0] = &m_EvenFields[pInfo->CurrentFrame];
        }

        pInfo->LineLength = m_CurrentX * 2;
        pInfo->FrameWidth = m_CurrentX;
        pInfo->FrameHeight = m_CurrentY;
        pInfo->FieldHeight = m_CurrentY / 2;
        pInfo->InputPitch = 4096;

    }

    Timing_IncrementUsedFields();

    // auto input detect
    Timimg_AutoFormatDetect(pInfo, m_NumFields);

}

int CCX2388xSource::GetWidth()
{
    return m_CurrentX;
}

int CCX2388xSource::GetHeight()
{
    return m_CurrentY;
}

void CCX2388xSource::SetWidth(int w)
{
	m_PixelWidth->SetValue(w);
}

CCX2388xCard* CCX2388xSource::GetCard()
{
    return m_pCard;
}

LPCSTR CCX2388xSource::GetStatus()
{
    static LPCSTR pRetVal = "";

    if (IsInTunerMode())
    {
        pRetVal = Channel_GetVBIName();

        if (*pRetVal == '\0')
        {
            pRetVal = Channel_GetName();
        }
    }
    else
    {
		pRetVal = m_pCard->GetInputName(m_VideoSource->GetValue());
    }

    return pRetVal;
}

eVideoFormat CCX2388xSource::GetFormat()
{
    return (eVideoFormat)m_VideoFormat->GetValue();
}

void CCX2388xSource::SetFormat(eVideoFormat NewFormat)
{
    PostMessageToMainWindow(WM_CX2388X_SETVALUE, CX2388XTVFORMAT, NewFormat);
}


ISetting* CCX2388xSource::GetBrightness()
{
    return m_Brightness;
}

ISetting* CCX2388xSource::GetContrast()
{
    return m_Contrast;
}

ISetting* CCX2388xSource::GetHue()
{
    return m_Hue;
}

ISetting* CCX2388xSource::GetSaturation()
{
    return m_Saturation;
}

ISetting* CCX2388xSource::GetSaturationU()
{
    if(m_pCard->IsThisCardH3D((eCX2388xCardId)m_CardType->GetValue()))
    {
        return NULL;
    }
    else
    {
        return m_SaturationU;
    }
}

ISetting* CCX2388xSource::GetSaturationV()
{
    if(m_pCard->IsThisCardH3D((eCX2388xCardId)m_CardType->GetValue()))
    {
        return NULL;
    }
    else
    {
        return m_SaturationV;
    }
}

ISetting* CCX2388xSource::GetAnalogueBlanking()
{
    if(m_CurrentX == 720)
    {
	    return m_AnalogueBlanking;
    }
	else
	{
	    return NULL;
	}
}

ISetting* CCX2388xSource::GetTopOverscan()
{
    return m_TopOverscan;
}

ISetting* CCX2388xSource::GetBottomOverscan()
{
    return m_BottomOverscan;
}

ISetting* CCX2388xSource::GetLeftOverscan()
{
    return m_LeftOverscan;
}

ISetting* CCX2388xSource::GetRightOverscan()
{
    return m_RightOverscan;
}

ISetting* CCX2388xSource::GetHDelay()
{
    return m_HDelay;
}

ISetting* CCX2388xSource::GetVDelay()
{
    return m_VDelay;
}

////////////////////////////////////////////////////////////////////////////////////
// The following function will continually check the position in the RISC code
// until it is  is different from what we already have.
// We know were we are so we set the current field to be the last one
// that has definitely finished.
//
// Added code here to use a user specified parameter for how long to sleep.  Note that
// windows timer tick resolution is really MUCH worse than 1 millesecond.  Who knows 
// what logic W98 really uses?
//
// Note also that sleep(0) just tells the operating system to dispatch some other
// task now if one is ready, not to sleep for zero seconds.  Since I've taken most
// of the unneeded waits out of other processing here Windows will eventually take 
// control away from us anyway, We might as well choose the best time to do it, without
// waiting more than needed. 
//
// Also added code to HurryWhenLate.  This checks if the new field is already here by
// the time we arrive.  If so, assume we are not keeping up with the BT chip and skip
// some later processing.  Skip displaying this field and use the CPU time gained to 
// get back here faster for the next one.  This should help us degrade gracefully on
// slower or heavily loaded systems but use all available time for processing a good
// picture when nothing else is running.  TRB 10/28/00
//
void CCX2388xSource::GetNextFieldNormal(TDeinterlaceInfo* pInfo)
{
    BOOL bSlept = FALSE;
	BOOL bLate = TRUE;
    int NewPos;
    int FieldDistance;
    int OldPos = (pInfo->CurrentFrame * 2 + m_IsFieldOdd + 1) % 10;
    DWORD StartTime = GetTickCount();

    while(OldPos == (NewPos = m_pCard->GetRISCPos()))
    {
        // need to sleep more often
        // so that we don't take total control of machine
        // in normal operation
        Timing_SmartSleep(pInfo, FALSE, bSlept);
        pInfo->bRunningLate = FALSE;            // if we waited then we are not late
        bLate = FALSE;							// if we waited then we are not late
        // check that we are not in a tight loop
        if(GetTickCount() > StartTime + 200)
        {
            break;
        }
    }
	if (bLate)
	{
		if (bAlwaysSleep)
		{
			Timing_SmartSleep(pInfo, pInfo->bRunningLate, bSlept);
		}
		Timing_IncrementNotWaitedFields();
	}

    FieldDistance = (10 + NewPos - OldPos) % 10;
    if(FieldDistance == 1)
    {
        pInfo->bMissedFrame = FALSE;
		if (bLate)
		{
            LOG(2, " Running late but right field");
			if (pInfo->bRunningLate)
			{
				Timing_AddDroppedFields(1);
			}
		}
    }
    else if (FieldDistance <= (MaxFieldShift+1))
    {
        NewPos = (OldPos + 1) % 10;
        pInfo->bMissedFrame = FALSE;
        Timing_AddLateFields(FieldDistance - 1);
        LOG(2, " Running late by %d fields", FieldDistance - 1);
    }
    else
    {
        // delete all history
        ClearPictureHistory(pInfo);
        pInfo->bMissedFrame = TRUE;
        Timing_AddDroppedFields(FieldDistance - 1);
        LOG(2, " Dropped %d Field(s)", FieldDistance - 1);
    }

    switch(NewPos)
    {
    case 0: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 4; break;
    case 1: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 0; break;
    case 2: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 0; break;
    case 3: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 1; break;
    case 4: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 1; break;
    case 5: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 2; break;
    case 6: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 2; break;
    case 7: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 3; break;
    case 8: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 3; break;
    case 9: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 4; break;
    }
}

////////////////////////////////////////////////////////////////////////////////////
// The following function will continually check the position in the RISC code
// until it is  is different from what we already have.
// We know were we are so we set the current field to be the last one
// that has definitely finished.
//
// Added code here to use a user specified parameter for how long to sleep.  Note that
// windows timer tick resolution is really MUCH worse than 1 millesecond.  Who knows 
// what logic W98 really uses?
//
// Note also that sleep(0) just tells the operating system to dispatch some other
// task now if one is ready, not to sleep for zero seconds.  Since I've taken most
// of the unneeded waits out of other processing here Windows will eventually take 
// control away from us anyway, We might as well choose the best time to do it, without
// waiting more than needed. 
//
// Also added code to HurryWhenLate.  This checks if the new field is already here by
// the time we arrive.  If so, assume we are not keeping up with the BT chip and skip
// some later processing.  Skip displaying this field and use the CPU time gained to 
// get back here faster for the next one.  This should help us degrade gracefully on
// slower or heavily loaded systems but use all available time for processing a good
// picture when nothing else is running.  TRB 10/28/00
//
void CCX2388xSource::GetNextFieldNormalProg(TDeinterlaceInfo* pInfo)
{
    BOOL bSlept = FALSE;
	BOOL bLate = TRUE;
    int NewPos;
    int FieldDistance;
    int OldPos = (pInfo->CurrentFrame + 1) % m_NumFields;
    DWORD StartTime = GetTickCount();

    while(OldPos == (NewPos = m_pCard->GetRISCPos()))
    {
        // need to sleep more often
        // so that we don't take total control of machine
        // in normal operation
        Timing_SmartSleep(pInfo, FALSE, bSlept);
        pInfo->bRunningLate = FALSE;            // if we waited then we are not late
        bLate = FALSE;							// if we waited then we are not late
        // check that we are not in a tight loop
        if(GetTickCount() > StartTime + 200)
        {
            break;
        }
    }
	if (bLate)
	{
		if (bAlwaysSleep)
		{
			Timing_SmartSleep(pInfo, pInfo->bRunningLate, bSlept);
		}
		Timing_IncrementNotWaitedFields();
	}

    FieldDistance = (m_NumFields + NewPos - OldPos) % m_NumFields;
    if(FieldDistance == 1)
    {
        pInfo->bMissedFrame = FALSE;
		if (bLate)
		{
            LOG(2, " Running late but right field");
			if (pInfo->bRunningLate)
			{
				Timing_AddDroppedFields(1);
			}
		}
    }
    else if (FieldDistance <= (MaxFieldShift+1))
    {
        NewPos = (OldPos + 1) % 10;
        pInfo->bMissedFrame = FALSE;
        Timing_AddLateFields(FieldDistance - 1);
        LOG(2, " Running late by %d fields", FieldDistance - 1);
    }
    else
    {
        // delete all history
        ClearPictureHistory(pInfo);
        pInfo->bMissedFrame = TRUE;
        Timing_AddDroppedFields(FieldDistance - 1);
        LOG(2, " Dropped %d Field(s)", FieldDistance - 1);
    }

    pInfo->CurrentFrame = (NewPos + m_NumFields - 1) % m_NumFields;
}

void CCX2388xSource::GetNextFieldAccurate(TDeinterlaceInfo* pInfo)
{
    BOOL bSlept = FALSE;
	BOOL bLate = TRUE;
    int NewPos;
    int FieldDistance;
    int OldPos = (pInfo->CurrentFrame * 2 + m_IsFieldOdd + 1) % 10;
    static int FieldCount(-1);
    DWORD StartTime = GetTickCount();
    
    while(OldPos == (NewPos = m_pCard->GetRISCPos()))
    {
        pInfo->bRunningLate = FALSE;            // if we waited then we are not late
        bLate = FALSE;							// if we waited then we are not late
        // check that we are not in a tight loop
        if(GetTickCount() > StartTime + 200)
        {
            break;
        }
    }
	if (bLate)
	{
		Timing_IncrementNotWaitedFields();
	}

    FieldDistance = (10 + NewPos - OldPos) % 10;
    if(FieldDistance == 1)
    {
        // No skipped fields, do nothing
		if (bLate)
		{
            LOG(2, " Running late but right field");
		}
    }
    else if (FieldDistance <= (MaxFieldShift+1))
    {
        NewPos = (OldPos + 1) % 10;
        Timing_SetFlipAdjustFlag(TRUE);
        Timing_AddLateFields(FieldDistance - 1);
        LOG(2, " Running late by %d fields", FieldDistance - 1);
    }
    else
    {
        // delete all history
        ClearPictureHistory(pInfo);
        pInfo->bMissedFrame = TRUE;
        Timing_AddDroppedFields(FieldDistance - 1);
        LOG(2, " Dropped %d Fields", FieldDistance - 1);
        Timing_Reset();
        FieldCount = -1;
    }

    switch(NewPos)
    {
    case 0: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 4; break;
    case 1: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 0; break;
    case 2: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 0; break;
    case 3: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 1; break;
    case 4: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 1; break;
    case 5: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 2; break;
    case 6: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 2; break;
    case 7: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 3; break;
    case 8: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 3; break;
    case 9: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 4; break;
    }
    
    // do input frequency on cleanish field changes only
	if (FieldCount != -1)
	{
	    FieldCount += FieldDistance;
	}
    if(FieldDistance == 1 && !bLate)
    {
	    if(FieldCount > 1)
		{
	        Timing_UpdateRunningAverage(pInfo, FieldCount);
	        FieldCount = 0;
		}
		else if (FieldCount == -1)
		{
	        FieldCount = 0;
		}
    }

	if (bAlwaysSleep || !bLate)
	{
	    Timing_SmartSleep(pInfo, pInfo->bRunningLate, bSlept);
	}
}


void CCX2388xSource::GetNextFieldAccurateProg(TDeinterlaceInfo* pInfo)
{
    BOOL bSlept = FALSE;
	BOOL bLate = TRUE;
    int NewPos;
    int FieldDistance;
    int OldPos = (pInfo->CurrentFrame + 1) % m_NumFields;
    static int FieldCount(-1);
    DWORD StartTime = GetTickCount();
    
    while(OldPos == (NewPos = m_pCard->GetRISCPos()))
    {
        pInfo->bRunningLate = FALSE;            // if we waited then we are not late
        bLate = FALSE;							// if we waited then we are not late
        // check that we are not in a tight loop
        if(GetTickCount() > StartTime + 200)
        {
            break;
        }
    }
	if (bLate)
	{
		Timing_IncrementNotWaitedFields();
	}

    FieldDistance = (m_NumFields + NewPos - OldPos) % m_NumFields;
    if(FieldDistance == 1)
    {
        // No skipped fields, do nothing
		if (bLate)
		{
            LOG(2, " Running late but right field");
		}
    }
    else if (FieldDistance <= (MaxFieldShift+1))
    {
        NewPos = (OldPos + 1) % m_NumFields;
        Timing_SetFlipAdjustFlag(TRUE);
        Timing_AddLateFields(FieldDistance - 1);
        LOG(2, " Running late by %d fields", FieldDistance - 1);
    }
    else
    {
        // delete all history
        ClearPictureHistory(pInfo);
        pInfo->bMissedFrame = TRUE;
        Timing_AddDroppedFields(FieldDistance - 1);
        LOG(2, " Dropped %d Fields", FieldDistance - 1);
        Timing_Reset();
        FieldCount = -1;
    }

    pInfo->CurrentFrame = (NewPos + m_NumFields - 1) % m_NumFields;
    
    // do input frequency on cleanish field changes only
	if (FieldCount != -1)
	{
	    FieldCount += FieldDistance;
	}
    if(FieldDistance == 1 && !bLate)
    {
	    if(FieldCount > 1)
		{
	        Timing_UpdateRunningAverage(pInfo, FieldCount);
	        FieldCount = 0;
		}
		else if (FieldCount == -1)
		{
	        FieldCount = 0;
		}
    }

	if (bAlwaysSleep || !bLate)
	{
	    Timing_SmartSleep(pInfo, pInfo->bRunningLate, bSlept);
	}
}

void CCX2388xSource::VideoSourceOnChange(long NewValue, long OldValue)
{
    Audio_Mute(PreSwitchMuteDelay);

    Stop_Capture();

    SettingsMaster->SaveSettings();

    // OK Capture is stopped so other onchange messages are
    // disabled so if anything that happens in those needs to be triggered
    // we have to manage that ourselves

    // here we have to watch for a format switch


    EventCollector->RaiseEvent(this, EVENT_VIDEOINPUT_PRECHANGE, OldValue, NewValue);
    EventCollector->RaiseEvent(this, EVENT_VIDEOINPUT_CHANGE, OldValue, NewValue);

    int OldFormat = m_VideoFormat->GetValue();
    
    // set up channel
    // this must happen after the VideoInput change is sent
    if(m_pCard->IsInputATuner(NewValue))
    {
        Channel_SetCurrent();
    }
	else
	{
		// We read what is the video format saved for this video input
	    SettingsMaster->LoadOneSetting(m_VideoFormat);
	}

    // tell the world if the format has changed
    if(OldFormat != m_VideoFormat->GetValue())
    {
        EventCollector->RaiseEvent(this, EVENT_VIDEOFORMAT_PRECHANGE, OldFormat, m_VideoFormat->GetValue());
        EventCollector->RaiseEvent(this, EVENT_VIDEOFORMAT_CHANGE, OldFormat, m_VideoFormat->GetValue());

		// We save the video format attached to this video input
	    SettingsMaster->WriteOneSetting(m_VideoFormat);
    }

    // make sure the defaults are correct
    // but don't change the values
    ChangeDefaultsForSetup(SETUP_CHANGE_ANY, TRUE);

    SettingsMaster->LoadSettings();

    // reset here when we have all the settings
    Reset();
    
    Audio_Unmute(PostSwitchMuteDelay);
    Start_Capture();
}

void CCX2388xSource::VideoFormatOnChange(long NewValue, long OldValue)
{
    Stop_Capture();

    SettingsMaster->SaveSettings();

    // OK Capture is stopped so other onchange messages are
    // disabled so if anything that happens in those needs to be triggered
    // we have to manage that ourselves

    EventCollector->RaiseEvent(this, EVENT_VIDEOFORMAT_PRECHANGE, OldValue, NewValue);   
    EventCollector->RaiseEvent(this, EVENT_VIDEOFORMAT_CHANGE, OldValue, NewValue);
    
    // make sure the defaults are correct
    // but don't change the values
    ChangeDefaultsForSetup(SETUP_CHANGE_ANY, TRUE);

    SettingsMaster->LoadSettings();

    // reset here when we have all the settings
    Reset();

    Start_Capture();
}

void CCX2388xSource::BrightnessOnChange(long Brightness, long OldValue)
{
    m_pCard->SetContrastBrightness((BYTE)m_Contrast->GetValue(), (BYTE)Brightness);
}

void CCX2388xSource::HueOnChange(long Hue, long OldValue)
{
    m_pCard->SetHue((BYTE)Hue);
}

void CCX2388xSource::ContrastOnChange(long Contrast, long OldValue)
{
    m_pCard->SetContrastBrightness((BYTE)Contrast, (BYTE)m_Brightness->GetValue());
}

void CCX2388xSource::SaturationUOnChange(long SatU, long OldValue)
{
    m_pCard->SetSaturationU((BYTE)SatU);
    if(m_InSaturationUpdate == FALSE)
    {
        m_InSaturationUpdate = TRUE;
        m_Saturation->SetValue((SatU + m_SaturationV->GetValue()) / 2);
        m_Saturation->SetMin(abs(SatU - m_SaturationV->GetValue()) / 2);
        m_Saturation->SetMax(255 - abs(SatU - m_SaturationV->GetValue()) / 2);
        m_InSaturationUpdate = FALSE;
    }
}

void CCX2388xSource::SaturationVOnChange(long SatV, long OldValue)
{
    m_pCard->SetSaturationV((BYTE)SatV);
    if(m_InSaturationUpdate == FALSE)
    {
        m_InSaturationUpdate = TRUE;
        m_Saturation->SetValue((SatV + m_SaturationU->GetValue()) / 2);
        m_Saturation->SetMin(abs(SatV - m_SaturationU->GetValue()) / 2);
        m_Saturation->SetMax(255 - abs(SatV - m_SaturationU->GetValue()) / 2);
        m_InSaturationUpdate = FALSE;
    }
}


void CCX2388xSource::SaturationOnChange(long Sat, long OldValue)
{
    if(m_InSaturationUpdate == FALSE)
    {
        m_InSaturationUpdate = TRUE;
        long NewSaturationU = m_SaturationU->GetValue() + (Sat - OldValue);
        long NewSaturationV = m_SaturationV->GetValue() + (Sat - OldValue);
        m_SaturationU->SetValue(NewSaturationU);
        m_SaturationV->SetValue(NewSaturationV);
        m_Saturation->SetMin(abs(m_SaturationV->GetValue() - m_SaturationU->GetValue()) / 2);
        m_Saturation->SetMax(255 - abs(m_SaturationV->GetValue() - m_SaturationU->GetValue()) / 2);
        m_InSaturationUpdate = FALSE;
    }
}

void CCX2388xSource::AnalogueBlankingOnChange(long NewValue, long OldValue)
{
    AspectSettings.bAnalogueBlanking = NewValue;
    WorkoutOverlaySize(TRUE);
}

void CCX2388xSource::TopOverscanOnChange(long Overscan, long OldValue)
{
    AspectSettings.InitialTopOverscan = Overscan;
    WorkoutOverlaySize(TRUE);
}

void CCX2388xSource::BottomOverscanOnChange(long Overscan, long OldValue)
{
    AspectSettings.InitialBottomOverscan = Overscan;
    WorkoutOverlaySize(TRUE);
}

void CCX2388xSource::LeftOverscanOnChange(long Overscan, long OldValue)
{
    AspectSettings.InitialLeftOverscan = Overscan;
    WorkoutOverlaySize(TRUE);
}

void CCX2388xSource::RightOverscanOnChange(long Overscan, long OldValue)
{
    AspectSettings.InitialRightOverscan = Overscan;
    WorkoutOverlaySize(TRUE);
}

void CCX2388xSource::TunerTypeOnChange(long TunerId, long OldValue)
{
    m_pCard->InitTuner((eTunerId)TunerId);
}

BOOL CCX2388xSource::IsInTunerMode()
{
    return m_pCard->IsInputATuner(m_VideoSource->GetValue());
}


void CCX2388xSource::SetupCard()
{
	if((m_ConexantStopDriver->GetValue() == TRUE) && (m_bDriverStoped == FALSE))
	{
		m_bDriverStoped = StartStopConexantDriver(DICS_DISABLE);
		if(m_bDriverStoped == TRUE)
		{
			m_pCard->ResetChip();
		}
	}

	// If the string card name is set, recalculate the card type based on
	// the given name.
    LPSTR cardName = reinterpret_cast<char*>(m_CardName->GetValue());
	if (*cardName != '\0')
	{
		m_CardType->SetValue(m_pCard->GetCardByName(cardName));
	}
	else
	{
		// Otherwise set the card name setting based on the card type for
		// future use.
		m_CardName->SetValue(reinterpret_cast<long>(
			m_pCard->GetCardName((eCX2388xCardId)m_CardType->GetValue())));
	}

    if(m_CardType->GetValue() == CX2388xCARD_UNKNOWN)
    {
		m_InitialSetup = TRUE;

        // try to detect the card
        m_CardType->SetValue(m_pCard->AutoDetectCardType());
        m_TunerType->SetValue(m_pCard->AutoDetectTuner((eCX2388xCardId)m_CardType->GetValue()));

        // then display the hardware setup dialog
        EnableCxCancelButton = 0;
        DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_SELECTCARD), GetMainWnd(), (DLGPROC) SelectCardProc, (LPARAM)this);
        EnableCxCancelButton = 1;
    }

    m_pCard->SetCardType(m_CardType->GetValue());
    m_pCard->InitTuner((eTunerId)m_TunerType->GetValue());

    // if the tuner has changed during this function
    // change the default format
    // but do so after the Tuner has been set on the card
    long OrigTuner = m_TunerType->GetValue();
    if(OrigTuner != m_TunerType->GetValue())
    {
        ChangeTVSettingsBasedOnTuner();
        // All the defaults should be set for NTSC
        // so in case we changed the format based on the tuner
        // reset here, actaully change the values too
        ChangeDefaultsForSetup(SETUP_CHANGE_ANY, FALSE);
    }

    // set up card specific menu
    DestroyMenu(m_hMenu);
    m_hMenu = m_pCard->GetCardSpecificMenu();
    Providers_UpdateMenu(m_hMenu);
}

void CCX2388xSource::ChangeSettingsBasedOnHW(int ProcessorSpeed, int TradeOff)
{
    // now do defaults based on the processor speed selected
    if(ProcessorSpeed == 0)
    {
        // User has selected below 300 MHz
        m_PixelWidth->ChangeDefault(640);
    }
    else if(ProcessorSpeed == 1)
    {
        // User has selected 300 MHz - 500 MHz
        m_PixelWidth->ChangeDefault(720);
    }
    else if(ProcessorSpeed == 2)
    {
        // User has selected 500 MHz - 1 GHz
        m_PixelWidth->ChangeDefault(720);
    }
    else
    {
        // user has fast processor use best defaults
        m_PixelWidth->ChangeDefault(720);
    }

}

/** ChangeTVSettingsBasedOnTuner
    This function only gets called when the tuner is set
    when the card is first found and all it does is set the default
    video format
*/
void CCX2388xSource::ChangeTVSettingsBasedOnTuner()
{
    // default the TVTYPE dependant on the Tuner selected
    // should be OK most of the time
    if(m_TunerType->GetValue() != TUNER_ABSENT)
    {
        // be a bit defensive here to avoid a possible
        // crash
        if(m_pCard->GetTuner() != NULL)
        {
            eVideoFormat videoFormat = m_pCard->GetTuner()->GetDefaultVideoFormat();
            m_VideoFormat->ChangeDefault(videoFormat);
        }
        else
        {
            LOG(1, " NULL Tuner in ChangeTVSettingsBasedOnTuner");
        }
    }
}


BOOL CCX2388xSource::SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat)
{
    if(VideoFormat == VIDEOFORMAT_LASTONE)
    {
        VideoFormat = m_pCard->GetTuner()->GetDefaultVideoFormat();
    }
    if(VideoFormat != m_VideoFormat->GetValue())
    {
        m_VideoFormat->SetValue(VideoFormat);
    }
	
	StopUpdateAudioStatus();
	
	BOOL bReturn = m_pCard->GetTuner()->SetTVFrequency(FrequencyId, VideoFormat);
	if(bReturn == TRUE)
	{
		// when switching from channel to channel the sound often hangs, 
		// so let's make an reset when AudioStandard is A2 or Nicam
		m_pCard->AudioInit(	m_VideoSource->GetValue(), 
			(eVideoFormat)m_VideoFormat->GetValue(), 
			(eCX2388xAudioStandard)m_AudioStandard->GetValue(),
			(eCX2388xStereoType)m_StereoType->GetValue() );
			
		StartUpdateAudioStatus();
	}

	return bReturn;
}

BOOL CCX2388xSource::IsVideoPresent()
{
    return m_pCard->IsVideoPresent();
}


void CCX2388xSource::DecodeVBI(TDeinterlaceInfo* pInfo)
{
    int nLineTarget;
    BYTE* pVBI = (LPBYTE) m_pVBILines[(pInfo->CurrentFrame + 4) % 5];
    if (m_IsFieldOdd)
    {
        pVBI += m_CurrentVBILines * 2048;
    }
    for (nLineTarget = 0; nLineTarget < m_CurrentVBILines; nLineTarget++)
    {
		VBI_DecodeLine(pVBI + nLineTarget * 2048, nLineTarget, m_IsFieldOdd);
    }
}


LPCSTR CCX2388xSource::GetMenuLabel()
{
    return m_pCard->GetCardName(m_pCard->GetCardType());
}

void CCX2388xSource::SetAspectRatioData()
{
    AspectSettings.InitialTopOverscan = m_TopOverscan->GetValue();
    AspectSettings.InitialBottomOverscan = m_BottomOverscan->GetValue();
    AspectSettings.InitialLeftOverscan = m_LeftOverscan->GetValue();
    AspectSettings.InitialRightOverscan = m_RightOverscan->GetValue();
    if(m_CurrentX == 720)
    {
	    AspectSettings.bAnalogueBlanking = m_AnalogueBlanking->GetValue();
    }
	else
	{
	    AspectSettings.bAnalogueBlanking = 0;
	}
}

void CCX2388xSource::HandleTimerMessages(int TimerId)
{
	UpdateAudioStatus();
}

void CCX2388xSource::IsVideoProgressiveOnChange(long NewValue, long OldValue)
{
    Stop_Capture();
    Reset();
    Start_Capture();
}

void CCX2388xSource::LumaAGCOnChange(long NewValue, long OldValue)
{
    m_pCard->SetLumaAGC(NewValue);
}

void CCX2388xSource::ChromaAGCOnChange(long NewValue, long OldValue)
{
    m_pCard->SetChromaAGC(NewValue);
}

void CCX2388xSource::FastSubcarrierLockOnChange(long NewValue, long OldValue)
{
    m_pCard->SetFastSubcarrierLock(NewValue);
}

void CCX2388xSource::WhiteCrushOnChange(long NewValue, long OldValue)
{
    m_pCard->SetWhiteCrushEnable(NewValue);
}

void CCX2388xSource::WhiteCrushUpOnChange(long NewValue, long OldValue)
{
    m_pCard->SetWhiteCrushUp((BYTE)NewValue);
}

void CCX2388xSource::WhiteCrushDownOnChange(long NewValue, long OldValue)
{
    m_pCard->SetWhiteCrushDown((BYTE)NewValue);
}

void CCX2388xSource::WhiteCrushMajorityPointOnChange(long NewValue, long OldValue)
{
    m_pCard->SetWhiteCrushMajorityPoint((CCX2388xCard::eWhiteCrushMajSel)NewValue);
}

void CCX2388xSource::WhiteCrushPerFrameOnChange(long NewValue, long OldValue)
{
    m_pCard->SetWhiteCrushPerFrame(NewValue);
}

void CCX2388xSource::LowColorRemovalOnChange(long NewValue, long OldValue)
{
    m_pCard->SetLowColorRemoval(NewValue);
}

void CCX2388xSource::CombFilterOnChange(long NewValue, long OldValue)
{
    m_pCard->SetCombFilter((CCX2388xCard::eCombFilter)NewValue);
}

void CCX2388xSource::FullLumaRangeOnChange(long NewValue, long OldValue)
{
    m_pCard->SetFullLumaRange(NewValue);
}

void CCX2388xSource::RemodulationOnChange(long NewValue, long OldValue)
{
    m_pCard->SetRemodulation((CCX2388xCard::eFlagWithDefault)NewValue);
}

void CCX2388xSource::Chroma2HCombOnChange(long NewValue, long OldValue)
{
    m_pCard->SetChroma2HComb((CCX2388xCard::eFlagWithDefault)NewValue);
}

void CCX2388xSource::ForceRemodExcessChromaOnChange(long NewValue, long OldValue)
{
    m_pCard->SetForceRemodExcessChroma((CCX2388xCard::eFlagWithDefault)NewValue);
}

void CCX2388xSource::IFXInterpolationOnChange(long NewValue, long OldValue)
{
    m_pCard->SetIFXInterpolation((CCX2388xCard::eFlagWithDefault)NewValue);
}

void CCX2388xSource::CombRangeOnChange(long NewValue, long OldValue)
{
    m_pCard->SetCombRange((CCX2388xCard::eFlagWithDefault)NewValue);
}

void CCX2388xSource::SecondChromaDemodOnChange(long NewValue, long OldValue)
{
    m_pCard->SetSecondChromaDemod((CCX2388xCard::eFlagWithDefault)NewValue);
}

void CCX2388xSource::ThirdChromaDemodOnChange(long NewValue, long OldValue)
{
    m_pCard->SetThirdChromaDemod((CCX2388xCard::eFlagWithDefault)NewValue);
}

void CCX2388xSource::FLIFilmDetectOnChange(long NewValue, long OldValue)
{
    if(m_pCard->IsThisCardH3D((eCX2388xCardId)m_CardType->GetValue()))
    {
        m_pCard->SetFLIFilmDetect(NewValue);
    }
}


void CCX2388xSource::HDelayOnChange(long NewValue, long OldValue)
{
    m_pCard->SetHDelay(
                            m_VideoSource->GetValue(), 
                            (eVideoFormat)m_VideoFormat->GetValue(), 
                            m_CurrentX, 
                            NewValue
                      );
}

void CCX2388xSource::VDelayOnChange(long NewValue, long OldValue)
{
    m_pCard->SetVDelay(
                            m_VideoSource->GetValue(), 
                            (eVideoFormat)m_VideoFormat->GetValue(), 
                            m_CurrentX, 
                            NewValue
                      );
}

void CCX2388xSource::EatLinesAtTopOnChange(long NewValue, long OldValue)
{
    Stop_Capture();
    Start_Capture();
}

void CCX2388xSource::SharpnessOnChange(long NewValue, long OldValue)
{
    m_pCard->SetSharpness((char)NewValue);
}


void CCX2388xSource::PixelWidthOnChange(long NewValue, long OldValue)
{
    if(NewValue != 768 &&
        NewValue != 754 &&
        NewValue != 720 &&
        NewValue != 640 &&
        NewValue != 384 &&
        NewValue != 320)
    {
        m_CustomPixelWidth->SetValue(NewValue);
    }
    Stop_Capture();

    Reset();
    
    NotifySizeChange();

    Start_Capture();
}


int  CCX2388xSource::NumInputs(eSourceInputType InputType)
{
  if (InputType == VIDEOINPUT)
  {
      return m_pCard->GetNumInputs();      
  }
  return 0;
}

BOOL CCX2388xSource::SetInput(eSourceInputType InputType, int Nr)
{
    if (InputType == VIDEOINPUT)
    {
        m_VideoSource->SetValue(Nr);
        return TRUE;
    }
    return FALSE;
}

int CCX2388xSource::GetInput(eSourceInputType InputType)
{
    if (InputType == VIDEOINPUT)
    {
        return m_VideoSource->GetValue();
    }
    return -1;
}

const char* CCX2388xSource::GetInputName(eSourceInputType InputType, int Nr)
{
    if (InputType == VIDEOINPUT)
    {
        if ((Nr>=0) && (Nr < m_pCard->GetNumInputs()) )
        {
            return m_pCard->GetInputName(Nr);
        }
    } 
    return NULL;
}

BOOL CCX2388xSource::InputHasTuner(eSourceInputType InputType, int Nr)
{
    if (InputType == VIDEOINPUT)
    {
        if(m_TunerType->GetValue() != TUNER_ABSENT)
        {
            return m_pCard->IsInputATuner(Nr);
        }
        else
        {
            return FALSE;
        }
    }
    return FALSE;
}

ITuner* CCX2388xSource::GetTuner()
{
    return m_pCard->GetTuner();
}

void CCX2388xSource::ConexantStopDriverOnChange(long NewValue,long OldValue)
{
}

void CCX2388xSource::AutoMuteOnChange(long NewValue,long OldValue)
{
	if(Audio_IsMute() && !Audio_GetUserMute())
	{
		Audio_Unmute();
	}
}

void CCX2388xSource::VerticalSyncDetectionOnChange(long NewValue,long OldValue)
{
    m_pCard->SetVerticalSyncDetection(NewValue);
}

///////////////////////////////////////////////////////////////////////
// In   NewState = DICS_DISABLE - disable  the device
//      NewState = DICS_ENABLE  - enable   the device
//
// Out  TRUE:   driver is found & stopped
//      FALSE:	an error occured
//
// based on DDK, see scr/setup/enable & scr/setup/devcon
///////////////////////////////////////////////////////////////////////
BOOL CCX2388xSource::StartStopConexantDriver(DWORD NewState)
{
	
    LOG(1, "CX2388x: WDM-Driver start search ...");
    
    // all CX2388x-Cards are having
	// VendorID = 0x14F1
	// DeviceID = 0x8800 (first sub-device)
    const char* pszCX2388X_HW_ID = "PCI\\VEN_14F1&DEV_88";
	
	// scan only Media-Classes
	HDEVINFO hDevInfo = SetupDiGetClassDevs((LPGUID)&GUID_DEVCLASS_MEDIA, NULL, NULL, DIGCF_PRESENT);
	if (hDevInfo == INVALID_HANDLE_VALUE)
    {
        LOG(0, "CX2388x: WDM-Driver search error - Media Class not found.");
        return FALSE;
    }
	
	SP_DEVINFO_DATA DeviceInfoData = {sizeof(SP_DEVINFO_DATA)};
	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	BOOL bFound = FALSE;
	for(int i=0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++)
	{
		DWORD	DataT		= NULL;
		LPTSTR	buffer		= NULL;
		DWORD	buffersize	= NULL;
        
		// see DDK src/setup/enable
		while (!SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData,	SPDRP_HARDWAREID,
												&DataT,	(PBYTE)buffer, buffersize, &buffersize))
        {
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				if (buffer)
				{
					LocalFree(buffer);
				}

				buffer = (LPTSTR)LocalAlloc(LPTR, buffersize);
			}

			else
			{
				return FALSE;
			}
		}

        LOG(2, buffer);

		if(strncmp(buffer, pszCX2388X_HW_ID, strlen(pszCX2388X_HW_ID)) == 0)
		{
            LOG(1,"CX2388x: WDM-Driver found.");
		    
		    // see DDK src/setup/devcon
		    SP_PROPCHANGE_PARAMS PropChangeParams = {sizeof(SP_CLASSINSTALL_HEADER)};
		    PropChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
		    PropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
		    PropChangeParams.Scope = DICS_FLAG_GLOBAL;
		    PropChangeParams.StateChange = NewState; 
		    PropChangeParams.HwProfile = 0;
    
		    if (!SetupDiSetClassInstallParams(hDevInfo,&DeviceInfoData,
			    (SP_CLASSINSTALL_HEADER *)&PropChangeParams,sizeof(PropChangeParams)))
		    {
			    LOG(0,"CX2388x: WDM-Driver unable to %s in DICS_FLAG_GLOBAL.", NewState == DICS_DISABLE ? "Stop" : "Start");
			    return FALSE;
		    }

		    PropChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
		    PropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
		    PropChangeParams.Scope = DICS_FLAG_CONFIGSPECIFIC;
		    PropChangeParams.StateChange = NewState; 
		    PropChangeParams.HwProfile = 0;

		    if (!SetupDiSetClassInstallParams(hDevInfo,&DeviceInfoData,
			    (SP_CLASSINSTALL_HEADER *)&PropChangeParams,sizeof(PropChangeParams))
			    || !SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,hDevInfo,&DeviceInfoData))
		    {
			    LOG(0,"CX2388x: WDM-Driver unable to %s in DICS_FLAG_CONFIGSPECIFIC.", NewState == DICS_DISABLE ? "Stop" : "Start");
			    return FALSE;
		    }
		    
		    LOG(1,"CX2388x: WDM-Driver %s.", NewState == DICS_DISABLE ? "Stop" : "Start");
			bFound = TRUE;
		}
		
		if(buffer)
		{
			LocalFree(buffer);
		}
	}

	if(bFound != TRUE)
	{
        LOG(1,"CX2388x: WDM-Driver not found.");
	}

	SetupDiDestroyDeviceInfoList(hDevInfo);
	
	return bFound;
}

#endif // WANT_CX2388X_SUPPORT
