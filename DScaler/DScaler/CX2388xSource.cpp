/////////////////////////////////////////////////////////////////////////////
// $Id: CX2388xSource.cpp,v 1.20 2002-12-23 17:22:10 adcockj Exp $
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

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "CX2388xSource.h"
#include "DScaler.h"
#include "VBI.h"
#include "VBI_VideoText.h"
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
#include "SettingsPerChannel.h"
#include "Providers.h"

extern long EnableCancelButton;

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

void CX2388x_OnSetup(void *pThis, int Start)
{
   if (pThis != NULL)
   {
      ((CCX2388xSource*)pThis)->SavePerChannelSetup(Start);
   }
}


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
    m_CurrentChannel(-1),
    m_SettingsByChannelStarted(FALSE),
    m_NumFields(10),
    m_hCX2388xResourceInst(NULL),
    m_SettingsSetup(NULL)
{
    CreateSettings(IniSection);

    SettingsPerChannel_RegisterOnSetup(this, CX2388x_OnSetup);
    
    eEventType EventList[] = {EVENT_CHANNEL_PRECHANGE,EVENT_CHANNEL_CHANGE,EVENT_ENDOFLIST};
    EventCollector->Register(this, EventList);
    
    ReadFromIni();

    m_RiscBaseLinear = (DWORD*)RiscDMAMem->GetUserPointer();
    m_RiscBasePhysical = RiscDMAMem->TranslateToPhysical(m_RiscBaseLinear, 83968, NULL);
    for(int i(0); i < 5; ++i)
    {
        m_pDisplay[i] = (BYTE*)DisplayDMAMem[i]->GetUserPointer();
        m_DisplayDMAMem[i] = DisplayDMAMem[i];
        m_pVBILines[i] = (BYTE*)VBIDMAMem[i]->GetUserPointer();
        m_VBIDMAMem[i] = VBIDMAMem[i];
    }

    SetupCard();

    LoadSettings(SETUP_CHANGE_ANY);
    
    InitializeUI();

    Reset();

    EventCollector->RaiseEvent(this, EVENT_VIDEOINPUT_CHANGE, -1, m_VideoSource->GetValue());
    EventCollector->RaiseEvent(this, EVENT_VIDEOFORMAT_CHANGE, -1, m_VideoFormat->GetValue());
    EventCollector->RaiseEvent(this, EVENT_VOLUME, 0, m_Volume->GetValue());
}

CCX2388xSource::~CCX2388xSource()
{
    EventCollector->Unregister(this);
    CX2388x_OnSetup(this, 0);

    if (m_SettingsSetup != NULL)
    {
        delete [] m_SettingsSetup;
    }

    delete m_pCard;
}

void CCX2388xSource::OnEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp)
{
    if (pEventObject != (CEventObject*)this)
    {
        return;
    }
    if (Event == EVENT_CHANNEL_CHANGE)
    {
    }
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
    CSettingGroup *pCX2388xGroup = GetSettingsGroup("CX2388x","CX2388x","CX Card");
    CSettingGroup *pVideoGroup = pCX2388xGroup->GetGroup("Video","Video");
    CSettingGroup *pH3DGroup = pCX2388xGroup->GetGroup("H3D","H3D");
    CSettingGroup *pAudioGroup = pCX2388xGroup->GetGroup("Audio","Audio");

    eSettingFlags FlagsAll = (eSettingFlags)(SETTINGFLAG_PER_SOURCE|SETTINGFLAG_ALLOW_PER_VIDEOINPUT|SETTINGFLAG_ALLOW_PER_VIDEOFORMAT|SETTINGFLAG_ALLOW_PER_CHANNEL|SETTINGFLAG_ONCHANGE_ALL);

    m_Brightness = new CBrightnessSetting(this, "Brightness", 128, 0, 255, IniSection, pVideoGroup, FlagsAll);
    m_Settings.push_back(m_Brightness);

    m_Contrast = new CContrastSetting(this, "Contrast", 128, 0, 255, IniSection, pVideoGroup, FlagsAll);
    m_Settings.push_back(m_Contrast);

    m_Hue = new CHueSetting(this, "Hue", 128, 0, 255, IniSection, pVideoGroup, FlagsAll);
    m_Settings.push_back(m_Hue);

    m_Saturation = new CSaturationSetting(this, "Saturation", 128, 0, 255, IniSection, pVideoGroup, FlagsAll);
    m_Settings.push_back(m_Saturation);

    m_SaturationU = new CSaturationUSetting(this, "Blue Saturation", 128, 0, 255, IniSection, pVideoGroup, FlagsAll);
    m_Settings.push_back(m_SaturationU);

    m_SaturationV = new CSaturationVSetting(this, "Red Saturation", 128, 0, 255, IniSection, pVideoGroup, FlagsAll);
    m_Settings.push_back(m_SaturationV);

    m_Overscan = new COverscanSetting(this, "Overscan", DEFAULT_OVERSCAN_NTSC, 0, 150, IniSection, pVideoGroup, FlagsAll);
    m_Settings.push_back(m_Overscan);

    m_VideoSource = new CVideoSourceSetting(this, "Video Source", 0, 0, 7, IniSection);
    m_Settings.push_back(m_VideoSource);

    m_VideoFormat = new CVideoFormatSetting(this, "Video Format", VIDEOFORMAT_NTSC_M, 0, VIDEOFORMAT_LASTONE - 1, IniSection);
    m_Settings.push_back(m_VideoFormat);

    m_CardType = new CSliderSetting("Card Type", CX2388xCARD_UNKNOWN, CX2388xCARD_UNKNOWN, CX2388xCARD_LASTONE - 1, IniSection, "CardType");
    m_Settings.push_back(m_CardType);

    m_TunerType = new CTunerTypeSetting(this, "Tuner Type", TUNER_ABSENT, TUNER_ABSENT, TUNER_LASTONE - 1, IniSection);
    m_Settings.push_back(m_TunerType);

    m_bSavePerInput = new CYesNoSetting("Save Per Input", FALSE, IniSection, "SavePerInput");
    m_Settings.push_back(m_bSavePerInput);
    
    m_bSavePerFormat = new CYesNoSetting("Save Per Format", TRUE, IniSection, "SavePerFormat");
    m_Settings.push_back(m_bSavePerFormat);
    
    m_bSavePerChannel = new CYesNoSetting("Save Per Channel", FALSE, IniSection, "SavePerChannel");
    m_Settings.push_back(m_bSavePerChannel);

    m_IsVideoProgressive = new CIsVideoProgressiveSetting(this, "Is Video Progressive", FALSE, IniSection, pH3DGroup, FlagsAll);
    m_Settings.push_back(m_IsVideoProgressive);

    m_FLIFilmDetect = new CFLIFilmDetectSetting(this, "FLI Film Detect", TRUE, IniSection, pH3DGroup, FlagsAll);
    m_Settings.push_back(m_FLIFilmDetect);

    m_HDelay = new CHDelaySetting(this, "Horizontal Delay", 0, 0, 255, IniSection, pVideoGroup, FlagsAll);
    m_Settings.push_back(m_HDelay);

    m_VDelay = new CVDelaySetting(this, "Vertical Delay", 0, 0, 255, IniSection, pVideoGroup, FlagsAll);
    m_VDelay->SetStepValue(4);
    m_Settings.push_back(m_VDelay);

    m_EatLinesAtTop = new CEatLinesAtTopSetting(this, "Eat Lines At Top", 12, 0, 100, IniSection, pH3DGroup, FlagsAll);
    m_Settings.push_back(m_EatLinesAtTop);
    
    m_Sharpness = new CSharpnessSetting(this, "Sharpness", 0, -8, 7, IniSection, pH3DGroup, FlagsAll);
    m_Settings.push_back(m_Sharpness);

    m_LumaAGC = new CLumaAGCSetting(this, "Luma AGC", FALSE, IniSection, pCX2388xGroup, FlagsAll);
    m_Settings.push_back(m_LumaAGC);

    m_ChromaAGC = new CChromaAGCSetting(this, "Chroma AGC", FALSE, IniSection, pCX2388xGroup, FlagsAll);
    m_Settings.push_back(m_ChromaAGC);

    m_FastSubcarrierLock = new CFastSubcarrierLockSetting(this, "Fast Subcarrier Lock", FALSE, IniSection, pCX2388xGroup, FlagsAll);
    m_Settings.push_back(m_FastSubcarrierLock);

    m_WhiteCrush = new CWhiteCrushSetting(this, "White Crush", FALSE, IniSection, pCX2388xGroup, FlagsAll);
    m_Settings.push_back(m_WhiteCrush);

    m_LowColorRemoval = new CLowColorRemovalSetting(this, "Low Color Removal", FALSE, IniSection, pCX2388xGroup, FlagsAll);
    m_Settings.push_back(m_LowColorRemoval);

    m_CombFilter = new CCombFilterSetting(this, "Comb Filter", CCX2388xCard::COMBFILTER_DEFAULT, CCX2388xCard::COMBFILTER_FULL, IniSection, CombFilterSzList, pCX2388xGroup, FlagsAll);
    m_Settings.push_back(m_CombFilter);

    m_FullLumaRange = new CFullLumaRangeSetting(this, "Full Luma Range", TRUE, IniSection, pCX2388xGroup, FlagsAll);
    m_Settings.push_back(m_FullLumaRange);

    m_Remodulation = new CRemodulationSetting(this, "Remodulation", CCX2388xCard::FLAG_DEFAULT, CCX2388xCard::FLAG_ON, IniSection, DefaultOffOnSzList, pCX2388xGroup, FlagsAll);
    m_Settings.push_back(m_Remodulation);

    m_Chroma2HComb = new CChroma2HCombSetting(this, "Chroma 2H Comb", CCX2388xCard::FLAG_DEFAULT, CCX2388xCard::FLAG_ON, IniSection, DefaultOffOnSzList, pCX2388xGroup, FlagsAll);
    m_Settings.push_back(m_Chroma2HComb);

    m_ForceRemodExcessChroma = new CForceRemodExcessChromaSetting(this, "Force Remodulation of Excess Chroma", CCX2388xCard::FLAG_DEFAULT, CCX2388xCard::FLAG_ON, IniSection, DefaultOffOnSzList, pCX2388xGroup, FlagsAll);
    m_Settings.push_back(m_ForceRemodExcessChroma);

    m_IFXInterpolation = new CIFXInterpolationSetting(this, "IFX Interpolation", CCX2388xCard::FLAG_DEFAULT, CCX2388xCard::FLAG_ON, IniSection, DefaultOffOnSzList, pCX2388xGroup, FlagsAll);
    m_Settings.push_back(m_IFXInterpolation);

    m_CombRange = new CCombRangeSetting(this, "Adaptative Comb Filter Threshold", 0x01f, 0, 0x3ff, IniSection, pVideoGroup, FlagsAll);
    m_Settings.push_back(m_CombRange);

    m_SecondChromaDemod = new CSecondChromaDemodSetting(this, "Second Chroma Demodulation", CCX2388xCard::FLAG_DEFAULT, CCX2388xCard::FLAG_ON, IniSection, DefaultOffOnSzList, pCX2388xGroup, FlagsAll);
    m_Settings.push_back(m_SecondChromaDemod);

    m_ThirdChromaDemod = new CThirdChromaDemodSetting(this, "Third Chroma Demodulation", CCX2388xCard::FLAG_DEFAULT, CCX2388xCard::FLAG_ON, IniSection, DefaultOffOnSzList, pCX2388xGroup, FlagsAll);
    m_Settings.push_back(m_ThirdChromaDemod);

    m_PixelWidth = new CPixelWidthSetting(this, "Sharpness", 720, 120, DSCALER_MAX_WIDTH, IniSection, pCX2388xGroup, FlagsAll);
    m_PixelWidth->SetStepValue(2);
    m_Settings.push_back(m_PixelWidth);

    m_CustomPixelWidth = new CSliderSetting("Custom Pixel Width", 750, 120, DSCALER_MAX_WIDTH, IniSection, "CustomPixelWidth", pCX2388xGroup, FlagsAll);
    m_CustomPixelWidth->SetStepValue(2);
    m_Settings.push_back(m_CustomPixelWidth);

    m_WhiteCrushUp = new CWhiteCrushUpSetting(this, "White Crush Up", 15, 0, 63, IniSection, pCX2388xGroup, FlagsAll);
    m_Settings.push_back(m_WhiteCrushUp);

    m_WhiteCrushDown = new CWhiteCrushDownSetting(this, "White Crush Down", 63, 0, 63, IniSection, pCX2388xGroup, FlagsAll);
    m_Settings.push_back(m_WhiteCrushDown);

    m_WhiteCrushMajorityPoint = new CWhiteCrushMajorityPointSetting(this, "White Crush Majority Point", CCX2388xCard::MAJSEL_AUTOMATIC, CCX2388xCard::MAJSEL_AUTOMATIC, IniSection, WhiteCrushMajorityPointList, pCX2388xGroup, FlagsAll);
    m_Settings.push_back(m_WhiteCrushMajorityPoint);

    m_WhiteCrushPerFrame = new CWhiteCrushPerFrameSetting(this, "White Crush Per Frame", TRUE, IniSection, pCX2388xGroup, FlagsAll);
    m_Settings.push_back(m_WhiteCrushPerFrame);

    m_Volume = new CVolumeSetting(this, "Volume", 900, 0, 1000, IniSection, pAudioGroup, FlagsAll);
    m_Volume->SetStepValue(20);
    m_Settings.push_back(m_Volume);

    m_Balance = new CBalanceSetting(this, "Balance", 0, -127, 127, IniSection, pAudioGroup, FlagsAll);
    m_Settings.push_back(m_Balance);

    m_AudioStandard = new CAudioStandardSetting(this, "Audio Standard", CCX2388xCard::AUDIO_STANDARD_AUTO, CCX2388xCard::AUDIO_STANDARD_FM, IniSection, AudioStandardList, pAudioGroup, FlagsAll);
    m_Settings.push_back(m_AudioStandard);

    m_StereoType = new CStereoTypeSetting(this, "Stereo Type", CCX2388xCard::STEREOTYPE_AUTO, CCX2388xCard::STEREOTYPE_ALT2, IniSection, StereoTypeList, pAudioGroup, FlagsAll);
    m_Settings.push_back(m_StereoType);

#ifdef _DEBUG    
    if (CX2388X_SETTING_LASTONE != m_Settings.size())
    {
        LOGD("Number of settings in CX2388X source is not equal to the number of settings in DS_Control.h");
        LOGD("DS_Control.h or CX2388xSource.cpp are probably not in sync with eachother.");
    }
#endif

    SetupSettings();
}

void CCX2388xSource::SetupSettings()
{
    #define PER_VIDEOINPUT      SETUP_PER_VIDEOINPUT
    #define PER_VIDEOFORMAT     SETUP_PER_VIDEOFORMAT
    #define PER_AUDIOINPUT      SETUP_PER_AUDIOINPUT
    #define PER_CHANNEL         SETUP_PER_CHANNEL
    
    TSettingsSetup SettingsSetupHolo3d[] =
    {
        { m_VideoSource,            SETUP_CHANGE_VIDEOINPUT },
        { m_VideoFormat,            SETUP_CHANGE_VIDEOFORMAT | PER_VIDEOINPUT | PER_CHANNEL },
        { m_Brightness,             PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_Contrast,               PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_Saturation,             PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_Hue,                    PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_Overscan,               PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_FLIFilmDetect,          PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_Sharpness,              PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_IsVideoProgressive,     PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { NULL,                     0 }
    };

    TSettingsSetup SettingsSetupNormal[] =
    {
        { m_VideoSource,            SETUP_CHANGE_VIDEOINPUT },
        { m_VideoFormat,            SETUP_CHANGE_VIDEOFORMAT | PER_VIDEOINPUT | PER_CHANNEL },
        { m_Brightness,             PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_Contrast,               PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_Saturation,             PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_Hue,                    PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_Overscan,               PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_LumaAGC,                PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_ChromaAGC,              PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_FastSubcarrierLock,     PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_WhiteCrush,             PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_LowColorRemoval,        PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_CombFilter,             PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_FullLumaRange,          PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_Remodulation,           PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_Chroma2HComb,           PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_ForceRemodExcessChroma, PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_IFXInterpolation,       PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_CombRange,              PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_SecondChromaDemod,      PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_ThirdChromaDemod,       PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_WhiteCrushUp,           PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_WhiteCrushDown,         PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_WhiteCrushMajorityPoint,PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_WhiteCrushPerFrame,     PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_Volume,                 PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_Balance,                PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_AudioStandard,          PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_StereoType,             PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { NULL,                     0 }
    };

    #undef PER_VIDEOINPUT
    #undef PER_VIDEOFORMAT
    #undef PER_AUDIOINPUT
    #undef PER_CHANNEL

    if(m_CardType->GetValue() == CX2388xCARD_HOLO3D)
    {
        WORD ListCount = sizeof(SettingsSetupHolo3d)/sizeof(TSettingsSetup);

        m_SettingsSetup = new TSettingsSetup[ListCount];
        memcpy(m_SettingsSetup, SettingsSetupHolo3d, sizeof(SettingsSetupHolo3d));
    }
    else
    {
        WORD ListCount = sizeof(SettingsSetupNormal)/sizeof(TSettingsSetup);

        m_SettingsSetup = new TSettingsSetup[ListCount];
        memcpy(m_SettingsSetup, SettingsSetupNormal, sizeof(SettingsSetupNormal));
    }
}


void CCX2388xSource::Start()
{
    m_pCard->StopCapture();
    CreateRiscCode(bCaptureVBI && (m_CurrentVBILines > 0));
    // only capture VBI if we are expecting them
    m_pCard->StartCapture(bCaptureVBI && (m_CurrentVBILines > 0));
    Timing_Reset();
    NotifySizeChange();
    NotifySquarePixelsCheck();
}

void CCX2388xSource::Reset()
{
    m_pCard->ResetHardware();
    m_pCard->SetVideoSource(m_VideoSource->GetValue());

    m_pCard->SetContrastBrightness(m_Contrast->GetValue(), m_Brightness->GetValue());
    m_pCard->SetHue(m_Hue->GetValue());
    m_pCard->SetSaturationU(m_SaturationU->GetValue());
    m_pCard->SetSaturationV(m_SaturationV->GetValue());

    m_CurrentX = m_PixelWidth->GetValue();
    m_pCard->SetGeoSize(
                            m_VideoSource->GetValue(), 
                            (eVideoFormat)m_VideoFormat->GetValue(), 
                            m_CurrentX, 
                            m_CurrentY, 
                            m_CurrentVBILines,
                            m_VDelay->GetValue(), 
                            m_HDelay->GetValue(),
                            m_IsVideoProgressive->GetValue()
                        );

    if(IsInTunerMode())
    {
        m_pCard->AudioInit(
                            m_VideoSource->GetValue(), 
                            (eVideoFormat)m_VideoFormat->GetValue(), 
                            (CCX2388xCard::eAudioStandard)m_AudioStandard->GetValue(),
                            (CCX2388xCard::eStereoType)m_StereoType->GetValue()
                          );
    }

    if(m_CardType->GetValue() == CX2388xCARD_HOLO3D)
    {
        m_pCard->SetFLIFilmDetect(m_FLIFilmDetect->GetValue());
        m_pCard->SetSharpness(m_Sharpness->GetValue());
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
        m_pCard->SetWhiteCrushUp(m_WhiteCrushUp->GetValue());
        m_pCard->SetWhiteCrushDown(m_WhiteCrushDown->GetValue());
        m_pCard->SetWhiteCrushMajorityPoint((CCX2388xCard::eWhiteCrushMajSel)m_WhiteCrushMajorityPoint->GetValue());
        m_pCard->SetWhiteCrushPerFrame(m_WhiteCrushPerFrame->GetValue());
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
        if(m_CardType->GetValue() == CX2388xCARD_HOLO3D)
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
    // stop capture
    m_pCard->StopCapture();
}

void CCX2388xSource::GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming)
{
    static long RepeatCount = 0;

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


CCX2388xCard* CCX2388xSource::GetCard()
{
    return m_pCard;
}

LPCSTR CCX2388xSource::GetStatus()
{
    static LPCSTR pRetVal = "";
    pRetVal = m_pCard->GetInputName(m_VideoSource->GetValue());
    return pRetVal;
}

eVideoFormat CCX2388xSource::GetFormat()
{
    return (eVideoFormat)m_VideoFormat->GetValue();
}

void CCX2388xSource::SetFormat(eVideoFormat NewFormat)
{
    PostMessage(hWnd, WM_CX2388X_SETVALUE, CX2388XTVFORMAT, NewFormat);
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
    if(m_CardType->GetValue() != CX2388xCARD_HOLO3D)
    {
        return m_SaturationU;
    }
    else
    {
        return NULL;
    }
}

ISetting* CCX2388xSource::GetSaturationV()
{
    if(m_CardType->GetValue() != CX2388xCARD_HOLO3D)
    {
        return m_SaturationV;
    }
    else
    {
        return NULL;
    }
}

ISetting* CCX2388xSource::GetOverscan()
{
    return m_Overscan;
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
    int NewPos;
    int Diff;
    int OldPos = (pInfo->CurrentFrame * 2 + m_IsFieldOdd + 1) % 10;

    while(OldPos == (NewPos = m_pCard->GetRISCPos()))
    {
        // need to sleep more often
        // so that we don't take total control of machine
        // in normal operation
        Timing_SmartSleep(pInfo, FALSE, bSlept);
        pInfo->bRunningLate = FALSE;            // if we waited then we are not late
    }

    Diff = (10 + NewPos - OldPos) % 10;
    if(Diff > 1)
    {
        // delete all history
        ClearPictureHistory(pInfo);
        pInfo->bMissedFrame = TRUE;
        Timing_AddDroppedFields(Diff - 1);
        LOG(2, " Dropped Frame");
    }
    else
    {
        pInfo->bMissedFrame = FALSE;
        if (pInfo->bRunningLate)
        {
            Timing_AddDroppedFields(1);
            LOG(2, "Running Late");
        }
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
    int NewPos;
    int Diff;
    int OldPos = (pInfo->CurrentFrame + 1) % m_NumFields;

    while(OldPos == (NewPos = m_pCard->GetRISCPos()))
    {
        // need to sleep more often
        // so that we don't take total control of machine
        // in normal operation
        Timing_SmartSleep(pInfo, FALSE, bSlept);
        pInfo->bRunningLate = FALSE;            // if we waited then we are not late
    }

    Diff = (m_NumFields + NewPos - OldPos) % m_NumFields;
    if(Diff > 1)
    {
        // delete all history
        ClearPictureHistory(pInfo);
        pInfo->bMissedFrame = TRUE;
        Timing_AddDroppedFields(Diff - 1);
        LOG(2, " Dropped Frame");
    }
    else
    {
        pInfo->bMissedFrame = FALSE;
        if (pInfo->bRunningLate)
        {
            Timing_AddDroppedFields(1);
            LOG(2, "Running Late");
        }
    }

    pInfo->CurrentFrame = (NewPos + m_NumFields - 1) % m_NumFields;
}

void CCX2388xSource::GetNextFieldAccurate(TDeinterlaceInfo* pInfo)
{
    BOOL bSlept = FALSE;
    int NewPos;
    int Diff;
    int OldPos = (pInfo->CurrentFrame * 2 + m_IsFieldOdd + 1) % 10;
    static int FieldCount(0);
    
    while(OldPos == (NewPos = m_pCard->GetRISCPos()))
    {
        pInfo->bRunningLate = FALSE;            // if we waited then we are not late
    }

    Diff = (10 + NewPos - OldPos) % 10;
    if(Diff == 1)
    {
    }
    else if(Diff == 2) 
    {
        NewPos = (OldPos + 1) % 10;
        Timing_SetFlipAdjustFlag(TRUE);
        LOG(2, " Slightly late");
    }
    else if(Diff == 3) 
    {
        NewPos = (OldPos + 1) % 10;
        Timing_SetFlipAdjustFlag(TRUE);
        LOG(2, " Very late");
    }
    else
    {
        // delete all history
        ClearPictureHistory(pInfo);
        pInfo->bMissedFrame = TRUE;
        Timing_AddDroppedFields(Diff - 1);
        LOG(2, " Dropped Frame");
        Timing_Reset();
        FieldCount = 0;
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
    
    FieldCount += Diff;
    // do input frequency on cleanish field changes only
    if(Diff == 1 && FieldCount > 1)
    {
        Timing_UpdateRunningAverage(pInfo, FieldCount);
        FieldCount = 0;
    }

    Timing_SmartSleep(pInfo, pInfo->bRunningLate, bSlept);
}


void CCX2388xSource::GetNextFieldAccurateProg(TDeinterlaceInfo* pInfo)
{
    BOOL bSlept = FALSE;
    int NewPos;
    int Diff;
    int OldPos = (pInfo->CurrentFrame + 1) % m_NumFields;
    static int FieldCount(0);
    
    while(OldPos == (NewPos = m_pCard->GetRISCPos()))
    {
        pInfo->bRunningLate = FALSE;            // if we waited then we are not late
    }


    Diff = (m_NumFields + NewPos - OldPos) % m_NumFields;
    if(Diff == 1)
    {
    }
    else if(Diff == 2) 
    {
        NewPos = (OldPos + 1) % m_NumFields;
        Timing_SetFlipAdjustFlag(TRUE);
        LOG(2, " Slightly late");
    }
    else if(Diff == 3) 
    {
        NewPos = (OldPos + 1) % m_NumFields;
        Timing_SetFlipAdjustFlag(TRUE);
        LOG(2, " Very late");
    }
    else
    {
        // delete all history
        ClearPictureHistory(pInfo);
        pInfo->bMissedFrame = TRUE;
        Timing_AddDroppedFields(Diff - 1);
        LOG(2, " Dropped Frame");
        Timing_Reset();
        FieldCount = 0;
    }

    pInfo->CurrentFrame = (NewPos + m_NumFields - 1) % m_NumFields;
    
    FieldCount += Diff;
    // do input frequency on cleanish field changes only
    if(Diff == 1 && FieldCount > 1)
    {
        Timing_UpdateRunningAverage(pInfo, FieldCount);
        FieldCount = 0;
    }

    Timing_SmartSleep(pInfo, pInfo->bRunningLate, bSlept);
}

void CCX2388xSource::VideoSourceOnChange(long NewValue, long OldValue)
{
    EventCollector->RaiseEvent(this, EVENT_VIDEOINPUT_PRECHANGE, OldValue, NewValue);

    Stop_Capture();

    Audio_Mute(PreSwitchMuteDelay);

    SaveSettings(SETUP_CHANGE_VIDEOINPUT);
    LoadSettings(SETUP_CHANGE_VIDEOINPUT);
    Reset();

    EventCollector->RaiseEvent(this, EVENT_VIDEOINPUT_CHANGE, OldValue, NewValue);

    // set up sound
    if(m_pCard->IsInputATuner(NewValue))
    {
        Channel_SetCurrent();
        Audio_Unmute(PostSwitchMuteDelay);
    }
    Start_Capture();
}

void CCX2388xSource::VideoFormatOnChange(long NewValue, long OldValue)
{
    EventCollector->RaiseEvent(this, EVENT_VIDEOFORMAT_PRECHANGE, OldValue, NewValue);
    Stop_Capture();
    SaveSettings(SETUP_CHANGE_VIDEOFORMAT);
    LoadSettings(SETUP_CHANGE_VIDEOFORMAT);
    EventCollector->RaiseEvent(this, EVENT_VIDEOFORMAT_CHANGE, OldValue, NewValue);
    Reset();
    Start_Capture();
}

void CCX2388xSource::BrightnessOnChange(long Brightness, long OldValue)
{
    m_pCard->SetContrastBrightness(m_Contrast->GetValue(), Brightness);
}

void CCX2388xSource::HueOnChange(long Hue, long OldValue)
{
    m_pCard->SetHue(Hue);
}

void CCX2388xSource::ContrastOnChange(long Contrast, long OldValue)
{
    m_pCard->SetContrastBrightness(Contrast, m_Brightness->GetValue());
}

void CCX2388xSource::SaturationUOnChange(long SatU, long OldValue)
{
    m_pCard->SetSaturationU(SatU);
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
    m_pCard->SetSaturationV(SatV);
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

void CCX2388xSource::OverscanOnChange(long Overscan, long OldValue)
{
    AspectSettings.InitialOverscan = Overscan;
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
    if(m_CardType->GetValue() == CX2388xCARD_UNKNOWN)
    {
        // try to detect the card
        m_CardType->SetValue(m_pCard->AutoDetectCardType());
        m_TunerType->SetValue(m_pCard->AutoDetectTuner((eCX2388xCardId)m_CardType->GetValue()));

        // then display the hardware setup dialog
        EnableCancelButton = 0;
        DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_SELECTCARD), hWnd, (DLGPROC) SelectCardProc, (LPARAM)this);
        EnableCancelButton = 1;
    }
    m_pCard->SetCardType(m_CardType->GetValue());
    m_pCard->InitTuner((eTunerId)m_TunerType->GetValue());

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

void CCX2388xSource::ChangeTVSettingsBasedOnTuner()
{
    // default the TVTYPE dependant on the Tuner selected
    // should be OK most of the time
    if(m_TunerType->GetValue() != TUNER_ABSENT)
    {
        eVideoFormat videoFormat = m_pCard->GetTuner()->GetDefaultVideoFormat();
        m_VideoFormat->ChangeDefault(videoFormat);
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
    return m_pCard->GetTuner()->SetTVFrequency(FrequencyId, VideoFormat);
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

void CCX2388xSource::SetOverscan()
{
    AspectSettings.InitialOverscan = m_Overscan->GetValue();
}

void CCX2388xSource::SavePerChannelSetup(int Start)
{
    if (Start)
    {
        m_SettingsByChannelStarted = TRUE;
        ChangeChannelSectionNames();
    }    
    else
    {
        m_SettingsByChannelStarted = FALSE;
        if (m_ChannelSubSection.size() > 0)
        {
            SettingsPerChannel_UnregisterSection(m_ChannelSubSection.c_str());
        }
    }
}

void CCX2388xSource::HandleTimerMessages(int TimerId)
{
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
    m_pCard->SetWhiteCrushUp(NewValue);
}

void CCX2388xSource::WhiteCrushDownOnChange(long NewValue, long OldValue)
{
    m_pCard->SetWhiteCrushDown(NewValue);
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
    if(m_CardType->GetValue() == CX2388xCARD_HOLO3D)
    {
        m_pCard->SetFLIFilmDetect(NewValue);
    }
}


void CCX2388xSource::HDelayOnChange(long NewValue, long OldValue)
{
    Stop_Capture();
    m_pCard->SetGeoSize(
                            m_VideoSource->GetValue(), 
                            (eVideoFormat)m_VideoFormat->GetValue(), 
                            m_CurrentX, 
                            m_CurrentY, 
                            m_CurrentVBILines,
                            m_VDelay->GetValue(), 
                            m_HDelay->GetValue(),
                            m_IsVideoProgressive->GetValue()
                        );
    Start_Capture();
}

void CCX2388xSource::VDelayOnChange(long NewValue, long OldValue)
{
    Stop_Capture();
    m_pCard->SetGeoSize(
                            m_VideoSource->GetValue(), 
                            (eVideoFormat)m_VideoFormat->GetValue(), 
                            m_CurrentX, 
                            m_CurrentY, 
                            m_CurrentVBILines,
                            m_VDelay->GetValue(), 
                            m_HDelay->GetValue(),
                            m_IsVideoProgressive->GetValue()
                        );
    Start_Capture();
}

void CCX2388xSource::EatLinesAtTopOnChange(long NewValue, long OldValue)
{
    Stop_Capture();
    Start_Capture();
}

void CCX2388xSource::SharpnessOnChange(long NewValue, long OldValue)
{
    m_pCard->SetSharpness(NewValue);
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
    m_CurrentX = NewValue;
    m_pCard->SetGeoSize(
                            m_VideoSource->GetValue(), 
                            (eVideoFormat)m_VideoFormat->GetValue(), 
                            m_CurrentX, 
                            m_CurrentY, 
                            m_CurrentVBILines,
                            m_VDelay->GetValue(), 
                            m_HDelay->GetValue(),
                            m_IsVideoProgressive->GetValue()
                        );
    
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
