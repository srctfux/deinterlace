/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Source.cpp,v 1.71 2002-09-22 17:47:04 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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
// Revision 1.70  2002/09/21 08:28:04  kooiman
// Preparations for fm radio accidentally slipped in. Disabled it till it works.
//
// Revision 1.69  2002/09/20 19:19:07  kooiman
// Force call to audiostandard detect onchange.
//
// Revision 1.68  2002/09/16 20:08:21  adcockj
// fixed format detect for cx2388x
//
// Revision 1.67  2002/09/16 19:34:18  adcockj
// Fix for auto format change
//
// Revision 1.66  2002/09/16 14:37:36  kooiman
// Added stereo autodetection.
//
// Revision 1.65  2002/09/15 15:57:27  kooiman
// Added Audio standard support.
//
// Revision 1.64  2002/09/15 14:20:37  adcockj
// Fixed timing problems for cx2388x chips
//
// Revision 1.63  2002/09/12 21:55:23  ittarnavsky
// Removed references to HasMSP and UseInputPin1
//
// Revision 1.62  2002/09/07 20:54:50  kooiman
// Added equalizer, loudness, spatial effects for MSP34xx
//
// Revision 1.61  2002/09/02 19:07:21  kooiman
// Added BT848 advanced settings to advanced settings dialog
//
// Revision 1.60  2002/08/27 22:02:32  kooiman
// Added Get/Set input for video and audio for all sources. Added source input change notification.
//
// Revision 1.59  2002/08/26 18:25:09  adcockj
// Fixed problem with PAL/NTSC detection
//
// Revision 1.58  2002/08/19 18:58:24  adcockj
// Changed video defaults
//
// Revision 1.57  2002/08/15 14:16:18  kooiman
// Cleaner settings per channel implementation
//
// Revision 1.56  2002/08/13 21:21:24  kooiman
// Improved settings per channel to account for source and input changes.
//
// Revision 1.55  2002/08/13 21:04:42  kooiman
// Add IDString() to Sources for identification purposes.
//
// Revision 1.54  2002/08/12 22:42:28  kooiman
// Fixed small spelling error.
//
// Revision 1.53  2002/08/12 19:54:27  laurentg
// Selection of video card to adjust DScaler settings
//
// Revision 1.52  2002/08/11 16:56:35  laurentg
// More information displayed in the title of the BT card setup dialog box
//
// Revision 1.51  2002/08/11 14:16:54  laurentg
// Disable Cancel button when the select card is displayed at startup
//
// Revision 1.50  2002/08/11 12:08:24  laurentg
// Cut BT Card setup and general hardware setup in two different windows
//
// Revision 1.49  2002/08/09 13:33:24  laurentg
// Processor speed and trade off settings moved from BT source settings to DScaler settings
//
// Revision 1.48  2002/08/08 21:15:07  kooiman
// Fix settings per channel timing issue.
//
// Revision 1.47  2002/08/08 12:35:39  kooiman
// Better channel settings support for BT848 settings.
//
// Revision 1.46  2002/08/07 21:53:04  adcockj
// Removed todo item
//
// Revision 1.45  2002/08/05 13:25:17  kooiman
// Added BT volume to save by channel settings.
//
// Revision 1.44  2002/08/05 12:05:28  kooiman
// Added support for per channel settings.
//
// Revision 1.43  2002/07/02 20:00:07  adcockj
// New setting for MSP input pin selection
//
// Revision 1.42  2002/06/22 15:00:22  laurentg
// New vertical flip mode
//
// Revision 1.41  2002/06/16 18:54:59  robmuller
// ACPI powersafe support.
//
// Revision 1.40  2002/06/05 20:53:49  adcockj
// Default changes and settings fixes
//
// Revision 1.39  2002/04/15 22:50:08  laurentg
// Change again the available formats for still saving
// Automatic switch to "square pixels" AR mode when needed
//
// Revision 1.38  2002/04/10 07:14:50  adcockj
// Fixed crash on saving settings
//
// Revision 1.37  2002/04/07 10:37:53  adcockj
// Made audio source work per input
//
// Revision 1.36  2002/03/12 21:10:04  robmuller
// Corrected error in TradeOff setting.
//
// Revision 1.35  2002/03/04 20:44:49  adcockj
// Reversed incorrect changed
//
// Revision 1.33  2002/02/23 16:41:09  laurentg
// Set timer TIMER_MSP only if current card has a MSP
//
// Revision 1.32  2002/02/23 00:30:47  laurentg
// NotifySizeChange
//
// Revision 1.31  2002/02/19 16:03:36  tobbej
// removed CurrentX and CurrentY
// added new member in CSource, NotifySizeChange
//
// Revision 1.30  2002/02/17 20:32:34  laurentg
// Audio input display suppressed from the OSD main screen
// GetStatus modified to display the video input name in OSD main screen even when there is no signal
//
// Revision 1.29  2002/02/17 18:45:08  laurentg
// At the first hardware setup, select the correct audio input
//
// Revision 1.28  2002/02/17 17:46:59  laurentg
// Mute the audio when switching to another card
//
// Revision 1.27  2002/02/10 21:34:31  laurentg
// Default value for "Save Settings By Format" is now ON
//
// Revision 1.26  2002/02/09 14:46:05  laurentg
// OSD main screen updated to display the correct input name (or channel)
// OSD main screen updated to display only activated filters
// Menu label for the BT848 providers now displays the name of the card
//
// Revision 1.25  2002/02/09 02:44:56  laurentg
// Overscan now stored in a setting of the source
//
// Revision 1.24  2002/02/08 19:27:18  adcockj
// Fixed problems with video settings dialog
//
// Revision 1.23  2002/01/26 17:54:48  laurentg
// Bug correction regarding pixel width updates
//
// Revision 1.22  2002/01/24 00:00:13  robmuller
// Added bOptimizeFileAccess flag to WriteToIni from the settings classes.
//
// Revision 1.21  2002/01/21 14:33:17  robmuller
// Fixed: setting wrong audio input in tuner mode in VideoSourceOnChange().
//
// Revision 1.20  2002/01/17 22:22:06  robmuller
// Added member function GetTunerId().
//
// Revision 1.19  2002/01/13 12:47:58  adcockj
// Fix for pixel width and format change
//
// Revision 1.18  2001/12/22 13:18:04  adcockj
// Tuner bugfixes
//
// Revision 1.17  2001/12/19 19:24:45  ittarnavsky
// prepended SOUNDCHANNEL_ to all members of the eSoundChannel enum
//
// Revision 1.16  2001/12/18 13:12:11  adcockj
// Interim check-in for redesign of card specific settings
//
// Revision 1.15  2001/12/16 10:14:16  laurentg
// Calculation of used fields restored
//
// Revision 1.14  2001/12/05 21:45:10  ittarnavsky
// added changes for the AudioDecoder and AudioControls support
//
// Revision 1.13  2001/12/03 19:33:59  adcockj
// Bug fixes for settings and memory
//
// Revision 1.12  2001/12/03 17:27:56  adcockj
// SECAM NICAM patch from Quenotte
//
// Revision 1.11  2001/11/29 17:30:51  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.10  2001/11/29 14:04:06  adcockj
// Added Javadoc comments
//
// Revision 1.9  2001/11/25 01:58:34  ittarnavsky
// initial checkin of the new I2C code
//
// Revision 1.8  2001/11/23 10:49:16  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.7  2001/11/22 22:27:00  adcockj
// Bug Fixes
//
// Revision 1.6  2001/11/22 13:32:03  adcockj
// Finished changes caused by changes to TDeinterlaceInfo - Compiles
//
// Revision 1.5  2001/11/21 15:21:39  adcockj
// Renamed DEINTERLACE_INFO to TDeinterlaceInfo in line with standards
// Changed TDeinterlaceInfo structure to have history of pictures.
//
// Revision 1.4  2001/11/21 12:32:11  adcockj
// Renamed CInterlacedSource to CSource in preparation for changes to DEINTERLACE_INFO
//
// Revision 1.3  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.2  2001/11/02 16:30:07  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.1.2.11  2001/08/23 16:04:57  adcockj
// Improvements to dynamic menus to remove requirement that they are not empty
//
// Revision 1.1.2.10  2001/08/22 18:38:31  adcockj
// Fixed Recursive bug
//
// Revision 1.1.2.9  2001/08/22 11:12:48  adcockj
// Added VBI support
//
// Revision 1.1.2.8  2001/08/22 10:40:58  adcockj
// Added basic tuner support
// Fixed recusive bug
//
// Revision 1.1.2.7  2001/08/21 16:42:16  adcockj
// Per format/input settings and ini file fixes
//
// Revision 1.1.2.6  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.1.2.5  2001/08/19 14:43:47  adcockj
// Fixed memory leaks
//
// Revision 1.1.2.4  2001/08/18 17:09:30  adcockj
// Got to compile, still lots to do...
//
// Revision 1.1.2.3  2001/08/17 16:35:14  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
// Revision 1.1.2.2  2001/08/16 06:43:34  adcockj
// moved more stuff into the new file (deonsn't compile)
//
// Revision 1.1.2.1  2001/08/15 14:44:05  adcockj
// Starting to put some flesh onto the new structure
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "BT848Source.h"
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
#include "BT848_Defines.h"
#include "FD_60Hz.h"
#include "FD_50Hz.h"
#include "DebugLog.h"
#include "AspectRatio.h"
#include "SettingsPerChannel.h"

extern long EnableCancelButton;

void BT848_OnSetup(void *pThis, int Start)
{
   if (pThis != NULL)
   {
      ((CBT848Source*)pThis)->SavePerChannelSetup(Start);
   }
}


CBT848Source::CBT848Source(CBT848Card* pBT848Card, CContigMemory* RiscDMAMem, CUserMemory* DisplayDMAMem[5], CUserMemory* VBIDMAMem[5], LPCSTR IniSection, LPCSTR ChipName, int DeviceIndex) :
    CSource(WM_BT848_GETVALUE, IDC_BT848),
    m_pBT848Card(pBT848Card),
    m_CurrentX(720),
    m_CurrentY(480),
    m_CurrentVBILines(19),
    m_Section(IniSection),
    m_IsFieldOdd(FALSE),
    m_InSaturationUpdate(FALSE),
    m_CurrentChannel(-1),
    m_SettingsByChannelStarted(FALSE),
    m_ChipName(ChipName),
    m_DeviceIndex(DeviceIndex),
    m_DetectingAudioStandard(0)
{
    m_IDString = IniSection;
    CreateSettings(IniSection);

    m_InitialACPIStatus = m_pBT848Card->GetACPIStatus();
    // if the BT878 is powered down we need to power it up
    if(m_InitialACPIStatus != 0)
    {
        m_pBT848Card->SetACPIStatus(0);
    }

    SettingsPerChannel_RegisterOnSetup(this, BT848_OnSetup);
    Channel_Register_Change_Notification(this, CBT848Source::StaticChannelChange);

    ReadFromIni();
    ChangeSectionNamesForInput();
    ChangeDefaultsForInput();
    LoadInputSettings();

    m_RiscBaseLinear = (DWORD*)RiscDMAMem->GetUserPointer();
    m_RiscBasePhysical = RiscDMAMem->TranslateToPhysical(m_RiscBaseLinear, 83968, NULL);
    for(int i(0); i < 5; ++i)
    {
        m_pDisplay[i] = (BYTE*)DisplayDMAMem[i]->GetUserPointer();
        m_DisplayDMAMem[i] = DisplayDMAMem[i];
        m_pVBILines[i] = (BYTE*)VBIDMAMem[i]->GetUserPointer();
        m_VBIDMAMem[i] = VBIDMAMem[i];
    }

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
    SetupCard();
    Reset();

    NotifyInputChange(0, VIDEOINPUT, -1, m_VideoSource->GetValue());
}

CBT848Source::~CBT848Source()
{
    Channel_UnRegister_Change_Notification(this, CBT848Source::StaticChannelChange);
    BT848_OnSetup(this, 0);
    // if the BT878 was not in D0 state we restore the original ACPI power state
    if(m_InitialACPIStatus != 0)
    {
        m_pBT848Card->SetACPIStatus(m_InitialACPIStatus);
    }

    KillTimer(hWnd, TIMER_MSP);
    delete m_pBT848Card;
}


void CBT848Source::CreateSettings(LPCSTR IniSection)
{
    m_Brightness = new CBrightnessSetting(this, "Brightness", DEFAULT_BRIGHTNESS_NTSC, -128, 127, IniSection);
    m_Settings.push_back(m_Brightness);

    m_Contrast = new CContrastSetting(this, "Contrast", DEFAULT_CONTRAST_NTSC, 0, 511, IniSection);
    m_Settings.push_back(m_Contrast);

    m_Hue = new CHueSetting(this, "Hue", DEFAULT_HUE_NTSC, -128, 127, IniSection);
    m_Settings.push_back(m_Hue);

    m_Saturation = new CSaturationSetting(this, "Saturation", (DEFAULT_SAT_V_NTSC + DEFAULT_SAT_U_NTSC) / 2, 0, 511, IniSection);
    m_Settings.push_back(m_Saturation);

    m_SaturationU = new CSaturationUSetting(this, "Blue Saturation", DEFAULT_SAT_U_NTSC, 0, 511, IniSection);
    m_Settings.push_back(m_SaturationU);

    m_SaturationV = new CSaturationVSetting(this, "Red Saturation", DEFAULT_SAT_V_NTSC, 0, 511, IniSection);
    m_Settings.push_back(m_SaturationV);

    m_Overscan = new COverscanSetting(this, "Overscan", DEFAULT_OVERSCAN_NTSC, 0, 150, IniSection);
    m_Settings.push_back(m_Overscan);

    m_BDelay = new CBDelaySetting(this, "Macrovision Timing", 0, 0, 255, IniSection);
    m_Settings.push_back(m_BDelay);

    m_BtAgcDisable = new CBtAgcDisableSetting(this, "AGC Disable", FALSE, IniSection);
    m_Settings.push_back(m_BtAgcDisable);

    m_BtCrush = new CBtCrushSetting(this, "Crush", TRUE, IniSection);
    m_Settings.push_back(m_BtCrush);

    m_BtEvenChromaAGC = new CBtEvenChromaAGCSetting(this, "Even Chroma AGC", TRUE, IniSection);
    m_Settings.push_back(m_BtEvenChromaAGC);

    m_BtOddChromaAGC = new CBtOddChromaAGCSetting(this, "Odd Chroma AGC", TRUE, IniSection);
    m_Settings.push_back(m_BtOddChromaAGC);

    m_BtEvenLumaPeak = new CBtEvenLumaPeakSetting(this, "Even Luma Peak", FALSE, IniSection);
    m_Settings.push_back(m_BtEvenLumaPeak);

    m_BtOddLumaPeak = new CBtOddLumaPeakSetting(this, "Odd Luma Peak", FALSE, IniSection);
    m_Settings.push_back(m_BtOddLumaPeak);

    m_BtFullLumaRange = new CBtFullLumaRangeSetting(this, "Full Luma Range", FALSE, IniSection);
    m_Settings.push_back(m_BtFullLumaRange);

    m_BtEvenLumaDec = new CBtEvenLumaDecSetting(this, "Even Luma Dec", FALSE, IniSection);
    m_Settings.push_back(m_BtEvenLumaDec);

    m_BtOddLumaDec = new CBtOddLumaDecSetting(this, "Odd Luma Dec", FALSE, IniSection);
    m_Settings.push_back(m_BtOddLumaDec);

    m_BtEvenComb = new CBtEvenCombSetting(this, "Even Comb", TRUE, IniSection);
    m_Settings.push_back(m_BtEvenComb);

    m_BtOddComb = new CBtOddCombSetting(this, "Odd Comb", TRUE, IniSection);
    m_Settings.push_back(m_BtOddComb);

    m_BtColorBars = new CBtColorBarsSetting(this, "Color Bars", FALSE, IniSection);
    m_Settings.push_back(m_BtColorBars);

    m_BtGammaCorrection = new CBtGammaCorrectionSetting(this, "Gamma Correction", FALSE, IniSection);
    m_Settings.push_back(m_BtGammaCorrection);

    m_BtCoring = new CBtCoringSetting(this, "Coring", FALSE, IniSection);
    m_Settings.push_back(m_BtCoring);

    m_BtHorFilter = new CBtHorFilterSetting(this, "Horizontal Filter", FALSE, IniSection);
    m_Settings.push_back(m_BtHorFilter);

    m_BtVertFilter = new CBtVertFilterSetting(this, "Vertical Filter", FALSE, IniSection);
    m_Settings.push_back(m_BtVertFilter);

    m_BtColorKill = new CBtColorKillSetting(this, "Color Kill", FALSE, IniSection);
    m_Settings.push_back(m_BtColorKill);

    m_BtWhiteCrushUp = new CBtWhiteCrushUpSetting(this, "White Crush Upper", 0xCF, 0, 255, IniSection);
    m_Settings.push_back(m_BtWhiteCrushUp);

    m_BtWhiteCrushDown = new CBtWhiteCrushDownSetting(this, "White Crush Lower", 0x7F, 0, 255, IniSection);
    m_Settings.push_back(m_BtWhiteCrushDown);

    m_PixelWidth = new CPixelWidthSetting(this, "Sharpness", 720, 120, DSCALER_MAX_WIDTH, IniSection);
    m_PixelWidth->SetStepValue(2);
    m_Settings.push_back(m_PixelWidth);

    m_CustomPixelWidth = new CSliderSetting("Custom Pixel Width", 750, 120, DSCALER_MAX_WIDTH, IniSection, "CustomPixelWidth");
    m_CustomPixelWidth->SetStepValue(2);
    m_Settings.push_back(m_CustomPixelWidth);

    m_VideoSource = new CVideoSourceSetting(this, "Video Source", 0, 0, 6, IniSection);
    m_Settings.push_back(m_VideoSource);

    m_VideoFormat = new CVideoFormatSetting(this, "Video Format", VIDEOFORMAT_NTSC_M, 0, VIDEOFORMAT_LASTONE - 1, IniSection);
    m_Settings.push_back(m_VideoFormat);

    m_HDelay = new CHDelaySetting(this, "Horizontal Delay", 0, 0, 255, IniSection);
    m_Settings.push_back(m_HDelay);

    m_VDelay = new CVDelaySetting(this, "Vertical Delay", 0, 0, 255, IniSection);
    m_VDelay->SetStepValue(2);
    m_Settings.push_back(m_VDelay);

    m_ReversePolarity = new CYesNoSetting("Reverse Polarity", FALSE, IniSection, "ReversePolarity");
    m_Settings.push_back(m_ReversePolarity);

    m_CardType = new CSliderSetting("Card Type", TVCARD_UNKNOWN, TVCARD_UNKNOWN, TVCARD_LASTONE - 1, IniSection, "CardType");
    m_Settings.push_back(m_CardType);

    m_TunerType = new CTunerTypeSetting(this, "Tuner Type", TUNER_ABSENT, TUNER_ABSENT, TUNER_LASTONE - 1, IniSection);
    m_Settings.push_back(m_TunerType);

    m_AudioSource1 = new CAudioSource1Setting(this, "Audio Source 1", AUDIOINPUT_MUTE, AUDIOINPUT_TUNER, AUDIOINPUT_STEREO, IniSection);
    m_Settings.push_back(m_AudioSource1);

    m_AudioChannel = new CAudioChannelSetting(this, "Audio Channel", SOUNDCHANNEL_STEREO, SOUNDCHANNEL_MONO, SOUNDCHANNEL_LANGUAGE2, IniSection);
    m_Settings.push_back(m_AudioChannel);

    m_AutoStereoSelect = new CAutoStereoSelectSetting(this, "Auto Stereo Select", FALSE, IniSection);
    m_Settings.push_back(m_AutoStereoSelect);

    m_Volume = new CVolumeSetting(this, "Volume", 900, 0, 1000, IniSection);
    m_Volume->SetStepValue(20);
    m_Settings.push_back(m_Volume);

    m_Bass = new CBassSetting(this, "Bass", 0, -96, 127, IniSection);
    m_Settings.push_back(m_Bass);

    m_Treble = new CTrebleSetting(this, "Treble", 0, -96, 127, IniSection);
    m_Settings.push_back(m_Treble);

    m_Balance = new CBalanceSetting(this, "Balance", 0, -127, 127, IniSection);
    m_Settings.push_back(m_Balance);

    m_bSavePerInput = new CYesNoSetting("Save Per Input", FALSE, IniSection, "SavePerInput");
    m_Settings.push_back(m_bSavePerInput);
    
    m_bSavePerFormat = new CYesNoSetting("Save Per Format", TRUE, IniSection, "SavePerFormat");
    m_Settings.push_back(m_bSavePerFormat);
    
    m_AudioSource2 = new CAudioSource2Setting(this, "Audio Source 2", AUDIOINPUT_MUTE, AUDIOINPUT_TUNER, AUDIOINPUT_STEREO, IniSection);
    m_Settings.push_back(m_AudioSource2);

    m_AudioSource3 = new CAudioSource3Setting(this, "Audio Source 3", AUDIOINPUT_MUTE, AUDIOINPUT_TUNER, AUDIOINPUT_STEREO, IniSection);
    m_Settings.push_back(m_AudioSource3);

    m_AudioSource4 = new CAudioSource4Setting(this, "Audio Source 4", AUDIOINPUT_MUTE, AUDIOINPUT_TUNER, AUDIOINPUT_STEREO, IniSection);
    m_Settings.push_back(m_AudioSource4);

    m_AudioSource5 = new CAudioSource5Setting(this, "Audio Source 5", AUDIOINPUT_MUTE, AUDIOINPUT_TUNER, AUDIOINPUT_STEREO, IniSection);
    m_Settings.push_back(m_AudioSource5);

    m_AudioSource6 = new CAudioSource6Setting(this, "Audio Source 6", AUDIOINPUT_MUTE, AUDIOINPUT_TUNER, AUDIOINPUT_STEREO, IniSection);
    m_Settings.push_back(m_AudioSource6);

    m_UseInputPin1 = new CUseInputPin1Setting(this, "Use MSP Input Pin 1", FALSE, IniSection);
    m_Settings.push_back(m_UseInputPin1);

    m_UseEqualizer = new CUseEqualizerSetting(this, "Use equalizer", FALSE, IniSection);
    m_Settings.push_back(m_UseEqualizer);

    m_EqualizerBand1 = new CEqualizerBand1Setting(this, "Equalizer band 1", 0, -96, 96, IniSection);
    m_Settings.push_back(m_EqualizerBand1);

    m_EqualizerBand2 = new CEqualizerBand2Setting(this, "Equalizer band 2", 0, -96, 96, IniSection);
    m_Settings.push_back(m_EqualizerBand2);

    m_EqualizerBand3 = new CEqualizerBand3Setting(this, "Equalizer band 3", 0, -96, 96, IniSection);
    m_Settings.push_back(m_EqualizerBand3);

    m_EqualizerBand4 = new CEqualizerBand4Setting(this, "Equalizer band 4", 0, -96, 96, IniSection);
    m_Settings.push_back(m_EqualizerBand4);

    m_EqualizerBand5 = new CEqualizerBand5Setting(this, "Equalizer band 5", 0, -96, 96, IniSection);
    m_Settings.push_back(m_EqualizerBand5);

    m_AudioLoudness = new CAudioLoudnessSetting(this, "Loudness", 0, 0, 255, IniSection);
    m_Settings.push_back(m_AudioLoudness);

    m_AudioSuperbass = new CAudioSuperbassSetting(this, "Super Bass", FALSE, IniSection);
    m_Settings.push_back(m_AudioSuperbass);

    m_AudioSpatialEffect = new CAudioSpatialEffectSetting(this, "Spatial Effect", 0, -128, 127, IniSection);
    m_Settings.push_back(m_AudioSpatialEffect);

    m_AudioAutoVolumeCorrection = new CAudioAutoVolumeCorrectionSetting(this, "Automatic Volume Correction", 0, 0, 60*1000, IniSection);
    m_Settings.push_back(m_AudioAutoVolumeCorrection);

	m_AudioStandardDetect = new CAudioStandardDetectSetting(this, "Audio Standard Detect", 0, 0, 4, IniSection);
    m_Settings.push_back(m_AudioStandardDetect);
    
    m_AudioStandardDetectInterval = new CAudioStandardDetectIntervalSetting(this, "Audio Standard Detect Interval (ms)", 200, 0, 10000, IniSection);
    m_Settings.push_back(m_AudioStandardDetectInterval);

	m_AudioStandardManual = new CAudioStandardManualSetting(this, "Audio Standard Manual", 0, 0, 0x7ff-1, IniSection);
    m_Settings.push_back(m_AudioStandardManual);

	m_AudioStandardMajorCarrier = new CAudioStandardMajorCarrierSetting(this, "Audio Standard Major carrier", 0, 0, 0x7ffffffL, IniSection);
    m_Settings.push_back(m_AudioStandardMajorCarrier);

	m_AudioStandardMinorCarrier = new CAudioStandardMinorCarrierSetting(this, "Audio Standard Minor carrier", 0, 0, 0x7ffffffL, IniSection);
    m_Settings.push_back(m_AudioStandardMinorCarrier);

    ReadFromIni();
}


void CBT848Source::Start()
{
    m_pBT848Card->StopCapture();
    CreateRiscCode(bCaptureVBI);
    m_pBT848Card->StartCapture(bCaptureVBI);
    m_pBT848Card->SetDMA(TRUE);
    Audio_Unmute();
    Timing_Reset();
    NotifySizeChange();
    // \todo: FIXME check to see if we need this timer
    {
        SetTimer(hWnd, TIMER_MSP, TIMER_MSP_MS, NULL);
    }
    NotifySquarePixelsCheck();
}

void CBT848Source::Reset()
{
    m_pBT848Card->ResetHardware(m_RiscBasePhysical);
    m_pBT848Card->SetVideoSource(m_VideoSource->GetValue());
    if (m_BDelay->GetValue() != 0)
    {
        // BDELAY override from .ini file
        m_pBT848Card->SetBDelay(m_BDelay->GetValue());
    }

    m_pBT848Card->SetBrightness(m_Brightness->GetValue());
    m_pBT848Card->SetContrast(m_Contrast->GetValue());
    m_pBT848Card->SetHue(m_Hue->GetValue());
    m_pBT848Card->SetSaturationU(m_SaturationU->GetValue());
    m_pBT848Card->SetSaturationV(m_SaturationV->GetValue());
    m_pBT848Card->SetEvenLumaDec(m_BtEvenLumaDec->GetValue());
    m_pBT848Card->SetOddLumaDec(m_BtOddLumaDec->GetValue());
    m_pBT848Card->SetEvenChromaAGC(m_BtEvenChromaAGC->GetValue());
    m_pBT848Card->SetOddChromaAGC(m_BtOddChromaAGC->GetValue());
    m_pBT848Card->SetEvenLumaPeak(m_BtEvenLumaPeak->GetValue());
    m_pBT848Card->SetOddLumaPeak(m_BtOddLumaPeak->GetValue());
    m_pBT848Card->SetColorKill(m_BtColorKill->GetValue());
    m_pBT848Card->SetHorFilter(m_BtHorFilter->GetValue());
    m_pBT848Card->SetVertFilter(m_BtVertFilter->GetValue());
    m_pBT848Card->SetFullLumaRange(m_BtFullLumaRange->GetValue());
    m_pBT848Card->SetCoring(m_BtCoring->GetValue());
    m_pBT848Card->SetEvenComb(m_BtEvenComb->GetValue());
    m_pBT848Card->SetOddComb(m_BtOddComb->GetValue());
    m_pBT848Card->SetAgcDisable(m_BtAgcDisable->GetValue());
    m_pBT848Card->SetCrush(m_BtCrush->GetValue());
    m_pBT848Card->SetColorBars(m_BtColorBars->GetValue());
    m_pBT848Card->SetGammaCorrection(m_BtGammaCorrection->GetValue());
    m_pBT848Card->SetWhiteCrushUp(m_BtWhiteCrushUp->GetValue());
    m_pBT848Card->SetWhiteCrushDown(m_BtWhiteCrushDown->GetValue());

    m_CurrentX = m_PixelWidth->GetValue();
    m_pBT848Card->SetGeoSize(
                                m_VideoSource->GetValue(), 
                                (eVideoFormat)m_VideoFormat->GetValue(), 
                                m_CurrentX, 
                                m_CurrentY, 
                                m_CurrentVBILines,
                                m_VDelay->GetValue(), 
                                m_HDelay->GetValue()
                            );
    
    NotifySizeChange();
    
    m_pBT848Card->SetAudioSource((eAudioInput)GetCurrentAudioSetting()->GetValue());
    //m_AudioStandardDetect->SetValue(m_AudioStandardDetect->GetValue());
    AudioStandardDetectOnChange(m_AudioStandardDetect->GetValue(),m_AudioStandardDetect->GetValue());
    m_pBT848Card->SetAudioChannel((eSoundChannel)m_AudioChannel->GetValue()); // FIXME, (m_UseInputPin1->GetValue() != 0));
}


void CBT848Source::CreateRiscCode(BOOL bCaptureVBI)
{
    DWORD* pRiscCode;
    WORD nField;
    WORD nLine;
    LPBYTE pUser;
    DWORD pPhysical;
    DWORD GotBytesPerLine;
    DWORD BytesPerLine = 0;

    pRiscCode = (DWORD*)m_RiscBaseLinear;
    // we create the RISC code for 10 fields
    // the first one (0) is even
    // last one (9) is odd
    for(nField = 0; nField < 10; nField++)
    {
        // First we sync onto either the odd or even field
        if(nField & 1)
        {
            *(pRiscCode++) = (DWORD) (BT848_RISC_SYNC | BT848_RISC_RESYNC | BT848_FIFO_STATUS_VRO);
        }
        else
        {
            *(pRiscCode++) = (DWORD) (BT848_RISC_SYNC | BT848_RISC_RESYNC | BT848_FIFO_STATUS_VRE);
        }
        *(pRiscCode++) = 0;

        // Create VBI code of required
        if (bCaptureVBI)
        {
            *(pRiscCode++) = (DWORD) (BT848_RISC_SYNC | BT848_FIFO_STATUS_FM1);
            *(pRiscCode++) = 0;

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
                *(pRiscCode++) = BT848_RISC_WRITE | BT848_RISC_SOL | BT848_RISC_EOL | VBI_SPL;
                *(pRiscCode++) = pPhysical;
                pUser += 2048;
            }
        }

        *(pRiscCode++) = (DWORD) (BT848_RISC_SYNC | BT848_FIFO_STATUS_FM1);
        *(pRiscCode++) = 0;


        // work out the position of the first line
        // first line is line zero an even line
        pUser = m_pDisplay[nField / 2];
        if(nField & 1)
        {
            pUser += 2048;
        }
        BytesPerLine = m_CurrentX * 2;
        for (nLine = 0; nLine < m_CurrentY / 2; nLine++)
        {

            pPhysical = m_DisplayDMAMem[nField / 2]->TranslateToPhysical(pUser, BytesPerLine, &GotBytesPerLine);
            if(pPhysical == 0 || BytesPerLine > GotBytesPerLine)
            {
                return;
            }
            *(pRiscCode++) = BT848_RISC_WRITE | BT848_RISC_SOL | BT848_RISC_EOL | BytesPerLine;
            *(pRiscCode++) = pPhysical;
            // since we are doing all the lines of the same
            // polarity at the same time we skip two lines
            pUser += 4096;
        }
    }

    m_BytesPerRISCField = ((long)pRiscCode - (long)m_RiscBaseLinear) / 10;
    *(pRiscCode++) = BT848_RISC_JUMP;
    *(pRiscCode++) = m_RiscBasePhysical;

    m_pBT848Card->SetRISCStartAddress(m_RiscBasePhysical);
}


void CBT848Source::Stop()
{
    // stop capture
    m_pBT848Card->StopCapture();
    Audio_Mute();
    // \todo: FIXME check to see if we need this timer
    {
        KillTimer(hWnd, TIMER_MSP);
    }
}

void CBT848Source::GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming)
{
    static long RepeatCount = 0;

    if(AccurateTiming)
    {
        GetNextFieldAccurate(pInfo);
    }
    else
    {
        GetNextFieldNormal(pInfo);
    }

    if (!pInfo->bRunningLate)
    {
    }

    Shift_Picture_History(pInfo);
    if(m_IsFieldOdd)
    {
        if(m_ReversePolarity->GetValue() == FALSE)
        {
            Replace_Picture_In_History(pInfo, 0, &m_OddFields[pInfo->CurrentFrame]);
        }
        else
        {
            Replace_Picture_In_History(pInfo, 0, &m_EvenFields[pInfo->CurrentFrame]);
        }
    }
    else
    {
        if(m_ReversePolarity->GetValue() == FALSE)
        {
            Replace_Picture_In_History(pInfo, 0, &m_EvenFields[pInfo->CurrentFrame]);
        }
        else
        {
            Replace_Picture_In_History(pInfo, 0, &m_OddFields[(pInfo->CurrentFrame + 4) % 5]);
        }
    }

    pInfo->LineLength = m_CurrentX * 2;
    pInfo->FrameWidth = m_CurrentX;
    pInfo->FrameHeight = m_CurrentY;
    pInfo->FieldHeight = m_CurrentY / 2;
    pInfo->InputPitch = 4096;

    Timing_IncrementUsedFields();

    // auto input detect
    Timimg_AutoFormatDetect(pInfo, 10);
}

int CBT848Source::GetWidth()
{
    return m_CurrentX;
}

int CBT848Source::GetHeight()
{
    return m_CurrentY;
}


CBT848Card* CBT848Source::GetBT848Card()
{
    return m_pBT848Card;
}

LPCSTR CBT848Source::GetStatus()
{
    static LPCSTR pRetVal = "";
    if (IsInTunerMode())
    {
        if (*VT_GetStation() != 0x00)
        {
            pRetVal = VT_GetStation();
        }
        else if (VPSLastName[0] != 0x00)
        {
            pRetVal = VPSLastName;
        }
        else
        {
            pRetVal = Channel_GetName();
        }
    }
    else
    {
        pRetVal = m_pBT848Card->GetInputName(m_VideoSource->GetValue());
    }
    return pRetVal;
}

int CBT848Source::GetRISCPosAsInt()
{
    int CurrentPos = 10;
    while(CurrentPos > 9)
    {
        DWORD CurrentRiscPos = m_pBT848Card->GetRISCPos();
        CurrentPos = (CurrentRiscPos - m_RiscBasePhysical) / m_BytesPerRISCField;
    }

    return CurrentPos;
}

eVideoFormat CBT848Source::GetFormat()
{
    return (eVideoFormat)m_VideoFormat->GetValue();
}

void CBT848Source::SetFormat(eVideoFormat NewFormat)
{
    PostMessage(hWnd, WM_BT848_SETVALUE, TVFORMAT, NewFormat);
}


ISetting* CBT848Source::GetBrightness()
{
    return m_Brightness;
}

ISetting* CBT848Source::GetContrast()
{
    return m_Contrast;
}

ISetting* CBT848Source::GetHue()
{
    return m_Hue;
}

ISetting* CBT848Source::GetSaturation()
{
    return m_Saturation;
}

ISetting* CBT848Source::GetSaturationU()
{
    return m_SaturationU;
}

ISetting* CBT848Source::GetSaturationV()
{
    return m_SaturationV;
}

ISetting* CBT848Source::GetOverscan()
{
    return m_Overscan;
}

void CBT848Source::BtAgcDisableOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetAgcDisable(NewValue);
}

void CBT848Source::BtCrushOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetCrush(NewValue);
}

void CBT848Source::BtEvenChromaAGCOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetEvenChromaAGC(NewValue);
}

void CBT848Source::BtOddChromaAGCOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetOddChromaAGC(NewValue);
}

void CBT848Source::BtEvenLumaPeakOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetEvenLumaPeak(NewValue);
}

void CBT848Source::BtOddLumaPeakOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetOddLumaPeak(NewValue);
}

void CBT848Source::BtFullLumaRangeOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetFullLumaRange(NewValue);
}

void CBT848Source::BtEvenLumaDecOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetEvenLumaDec(NewValue);
}

void CBT848Source::BtOddLumaDecOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetOddLumaDec(NewValue);
}

void CBT848Source::BtEvenCombOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetEvenComb(NewValue);
}

void CBT848Source::BtOddCombOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetOddComb(NewValue);
}

void CBT848Source::BtColorBarsOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetColorBars(NewValue);
}

void CBT848Source::BtGammaCorrectionOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetGammaCorrection(NewValue);
}

void CBT848Source::BtVertFilterOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetVertFilter(NewValue);
}

void CBT848Source::BtHorFilterOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetHorFilter(NewValue);
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
void CBT848Source::GetNextFieldNormal(TDeinterlaceInfo* pInfo)
{
    BOOL bSlept = FALSE;
    int NewPos;
    int Diff;
    int OldPos = (pInfo->CurrentFrame * 2 + m_IsFieldOdd + 1) % 10;

    while(OldPos == (NewPos = GetRISCPosAsInt()))
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
        Free_Picture_History(pInfo);
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

void CBT848Source::GetNextFieldAccurate(TDeinterlaceInfo* pInfo)
{
    BOOL bSlept = FALSE;
    int NewPos;
    int Diff;
    int OldPos = (pInfo->CurrentFrame * 2 + m_IsFieldOdd + 1) % 10;
    
    while(OldPos == (NewPos = GetRISCPosAsInt()))
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
        Free_Picture_History(pInfo);
        pInfo->bMissedFrame = TRUE;
        Timing_AddDroppedFields(Diff - 1);
        LOG(2, " Dropped Frame");
        Timing_Reset();
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
    
    // we've just got a new field
    // we are going to time the odd to odd
    // input frequency
    if(m_IsFieldOdd)
    {
        Timing_UpdateRunningAverage(pInfo, 2);
    }

    Timing_SmartSleep(pInfo, pInfo->bRunningLate, bSlept);
}

void CBT848Source::VideoSourceOnChange(long NewValue, long OldValue)
{
    NotifyInputChange(1, VIDEOINPUT, OldValue, NewValue);

    Stop_Capture();
    Audio_Mute();
    SaveInputSettings(TRUE);
    LoadInputSettings();
    Reset();

    NotifyInputChange(0, VIDEOINPUT, OldValue, NewValue);

    // set up sound
    if(m_pBT848Card->IsInputATuner(NewValue))
    {
        Channel_SetCurrent();
    }
    Audio_Unmute();
    Start_Capture();
}

void CBT848Source::VideoFormatOnChange(long NewValue, long OldValue)
{
    Stop_Capture();
    SaveInputSettings(TRUE);
    LoadInputSettings();
    Reset();
    Start_Capture();
}

void CBT848Source::PixelWidthOnChange(long NewValue, long OldValue)
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
    m_pBT848Card->SetGeoSize(
                                m_VideoSource->GetValue(), 
                                (eVideoFormat)m_VideoFormat->GetValue(), 
                                m_CurrentX, 
                                m_CurrentY, 
                                m_CurrentVBILines,
                                m_VDelay->GetValue(), 
                                m_HDelay->GetValue()
                            );
    
    NotifySizeChange();

    Start_Capture();
}

void CBT848Source::HDelayOnChange(long NewValue, long OldValue)
{
    Stop_Capture();
    m_pBT848Card->SetGeoSize(
                                m_VideoSource->GetValue(), 
                                (eVideoFormat)m_VideoFormat->GetValue(), 
                                m_CurrentX, 
                                m_CurrentY, 
                                m_CurrentVBILines,
                                m_VDelay->GetValue(), 
                                m_HDelay->GetValue()
                            );
    Start_Capture();
}

void CBT848Source::VDelayOnChange(long NewValue, long OldValue)
{
    Stop_Capture();
    m_pBT848Card->SetGeoSize(
                                m_VideoSource->GetValue(), 
                                (eVideoFormat)m_VideoFormat->GetValue(), 
                                m_CurrentX, 
                                m_CurrentY, 
                                m_CurrentVBILines,
                                m_VDelay->GetValue(), 
                                m_HDelay->GetValue()
                            );
    Start_Capture();
}

void CBT848Source::BrightnessOnChange(long Brightness, long OldValue)
{
    m_pBT848Card->SetBrightness(Brightness);
}

void CBT848Source::BtWhiteCrushUpOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetWhiteCrushUp(NewValue);
}

void CBT848Source::BtWhiteCrushDownOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetWhiteCrushDown(NewValue);
}

void CBT848Source::BtCoringOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetCoring(NewValue);
}

void CBT848Source::BtColorKillOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetColorKill(NewValue);
}

void CBT848Source::BDelayOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetBDelay(NewValue);
}

void CBT848Source::HueOnChange(long Hue, long OldValue)
{
    m_pBT848Card->SetHue(Hue);
}

void CBT848Source::ContrastOnChange(long Contrast, long OldValue)
{
    m_pBT848Card->SetContrast(Contrast);
}

void CBT848Source::SaturationUOnChange(long SatU, long OldValue)
{
    m_pBT848Card->SetSaturationU(SatU);
    if(m_InSaturationUpdate == FALSE)
    {
        m_InSaturationUpdate = TRUE;
        m_Saturation->SetValue((SatU + m_SaturationV->GetValue()) / 2);
        m_Saturation->SetMin(abs(SatU - m_SaturationV->GetValue()) / 2);
        m_Saturation->SetMax(511 - abs(SatU - m_SaturationV->GetValue()) / 2);
        m_InSaturationUpdate = FALSE;
    }
}

void CBT848Source::SaturationVOnChange(long SatV, long OldValue)
{
    m_pBT848Card->SetSaturationV(SatV);
    if(m_InSaturationUpdate == FALSE)
    {
        m_InSaturationUpdate = TRUE;
        m_Saturation->SetValue((SatV + m_SaturationU->GetValue()) / 2);
        m_Saturation->SetMin(abs(SatV - m_SaturationU->GetValue()) / 2);
        m_Saturation->SetMax(511 - abs(SatV - m_SaturationU->GetValue()) / 2);
        m_InSaturationUpdate = FALSE;
    }
}


void CBT848Source::SaturationOnChange(long Sat, long OldValue)
{
    if(m_InSaturationUpdate == FALSE)
    {
        m_InSaturationUpdate = TRUE;
        long NewSaturationU = m_SaturationU->GetValue() + (Sat - OldValue);
        long NewSaturationV = m_SaturationV->GetValue() + (Sat - OldValue);
        m_SaturationU->SetValue(NewSaturationU);
        m_SaturationV->SetValue(NewSaturationV);
        m_Saturation->SetMin(abs(m_SaturationV->GetValue() - m_SaturationU->GetValue()) / 2);
        m_Saturation->SetMax(511 - abs(m_SaturationV->GetValue() - m_SaturationU->GetValue()) / 2);
        m_InSaturationUpdate = FALSE;
    }
}

void CBT848Source::OverscanOnChange(long Overscan, long OldValue)
{
    AspectSettings.InitialOverscan = Overscan;
    WorkoutOverlaySize(TRUE);
}

void CBT848Source::TunerTypeOnChange(long TunerId, long OldValue)
{
    m_pBT848Card->InitTuner((eTunerId)TunerId);
}

BOOL CBT848Source::IsInTunerMode()
{
    return m_pBT848Card->IsInputATuner(m_VideoSource->GetValue());
}


void CBT848Source::SetupCard()
{
    if(m_CardType->GetValue() == TVCARD_UNKNOWN)
    {
        // try to detect the card
        m_CardType->SetValue(m_pBT848Card->AutoDetectCardType());
        m_TunerType->SetValue(m_pBT848Card->AutoDetectTuner((eTVCardId)m_CardType->GetValue()));

        // then display the hardware setup dialog
        EnableCancelButton = 0;
        DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_SELECTCARD), hWnd, (DLGPROC) SelectCardProc, (LPARAM)this);
        EnableCancelButton = 1;

        if(m_TunerType->GetValue() != TUNER_ABSENT)
        {
            m_AudioSource1->SetValue(AUDIOINPUT_TUNER);
        }
        else
        {
            m_AudioSource1->SetValue(AUDIOINPUT_EXTERNAL);
        }
    }
    m_pBT848Card->SetCardType(m_CardType->GetValue());
    m_pBT848Card->InitTuner((eTunerId)m_TunerType->GetValue());
    m_pBT848Card->InitAudio();
}

void CBT848Source::ChangeSettingsBasedOnHW(int ProcessorSpeed, int TradeOff)
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

void CBT848Source::ChangeTVSettingsBasedOnTuner()
{
    // default the TVTYPE dependant on the Tuner selected
    // should be OK most of the time
    if(m_TunerType->GetValue() != TUNER_ABSENT)
    {
        eVideoFormat videoFormat = m_pBT848Card->GetTuner()->GetDefaultVideoFormat();
        m_VideoFormat->ChangeDefault(videoFormat);
    }
}

BOOL CBT848Source::HasTuner()
{
    if(m_TunerType->GetValue() != TUNER_ABSENT)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


BOOL CBT848Source::SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat)
{
    /*
    //Doesn't work yet
    if(VideoFormat == VIDEOFORMAT_IS_FM_RADIO)
    {
        return m_pBT848Card->GetTuner()->SetRadioFrequency(FrequencyId);
    }
    */
    if(VideoFormat == VIDEOFORMAT_LASTONE)
    {
        VideoFormat = m_pBT848Card->GetTuner()->GetDefaultVideoFormat();
    }
    if(VideoFormat != m_VideoFormat->GetValue())
    {
        m_VideoFormat->SetValue(VideoFormat);
        
        //m_AudioStandardDetect->SetValue(m_AudioStandardDetect->GetValue());
        AudioStandardDetectOnChange(m_AudioStandardDetect->GetValue(),m_AudioStandardDetect->GetValue());
    }
    return m_pBT848Card->GetTuner()->SetTVFrequency(FrequencyId, VideoFormat);
}

BOOL CBT848Source::IsVideoPresent()
{
    return m_pBT848Card->IsVideoPresent();
}


void CBT848Source::DecodeVBI(TDeinterlaceInfo* pInfo)
{
    int nLineTarget;
    BYTE* pVBI = (LPBYTE) m_pVBILines[(pInfo->CurrentFrame + 4) % 5];
    if (m_IsFieldOdd)
    {
        pVBI += m_CurrentVBILines * 2048;
    }
    for (nLineTarget = 0; nLineTarget < m_CurrentVBILines ; nLineTarget++)
    {
       VBI_DecodeLine(pVBI + nLineTarget * 2048, nLineTarget, m_IsFieldOdd);
    }
}

eTunerId CBT848Source::GetTunerId()
{
    return m_pBT848Card->GetTuner()->GetTunerId();
}

LPCSTR CBT848Source::GetMenuLabel()
{
    return m_pBT848Card->GetCardName(m_pBT848Card->GetCardType());
}

void CBT848Source::SetOverscan()
{
    AspectSettings.InitialOverscan = m_Overscan->GetValue();
}


void CBT848Source::StaticChannelChange(void *pThis, int PreChange,int OldChannel,int NewChannel)
{
    if (pThis != NULL)
    {
        ((CBT848Source*)pThis)->ChannelChange(PreChange, OldChannel, NewChannel);
    }
}
void CBT848Source::ChannelChange(int PreChange, int OldChannel, int NewChannel)
{
    if (!PreChange && (m_AudioStandardDetect->GetValue()==3))
    {
        //m_AudioStandardDetect->SetValue(m_AudioStandardDetect->GetValue());
        AudioStandardDetectOnChange(m_AudioStandardDetect->GetValue(),m_AudioStandardDetect->GetValue());
    } 
    else if ((!PreChange) && m_AutoStereoSelect->GetValue())
    {      
        m_pBT848Card->DetectAudioStandard(m_AudioStandardDetectInterval->GetValue(), 2, this, StaticAudioStandardDetected);         
    }
    ///\todo schedule to occur after audio standard / stereo detect
    if (!PreChange && (m_AudioAutoVolumeCorrection->GetValue() > 0))
    {
       //Turn off & on after channel change
       long nDecayTimeIndex = m_AudioAutoVolumeCorrection->GetValue();
       m_AudioAutoVolumeCorrection->SetValue(0);
       m_AudioAutoVolumeCorrection->SetValue(nDecayTimeIndex);
    }
}

void CBT848Source::SavePerChannelSetup(int Start)
{
    if (Start&1)
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

int CBT848Source::GetDeviceIndex()
{
    return m_DeviceIndex;
}

const char* CBT848Source::GetChipName()
{
    return m_ChipName.c_str();
}



int  CBT848Source::NumInputs(eSourceInputType InputType)
{
  if (InputType == VIDEOINPUT)
  {
      return m_pBT848Card->GetNumInputs();      
  }
  else if (InputType == AUDIOINPUT)
  {
      return m_pBT848Card->GetNumAudioInputs();      
  }
  return 0;
}

BOOL CBT848Source::SetInput(eSourceInputType InputType, int Nr)
{
  if (InputType == VIDEOINPUT)
  {
      m_VideoSource->SetValue(Nr);
      return TRUE;
  }
  else if (InputType == AUDIOINPUT)
  {      
      m_pBT848Card->SetAudioSource((eAudioInput)Nr);          
      return TRUE;      
  }
  return FALSE;
}

int CBT848Source::GetInput(eSourceInputType InputType)
{
  if (InputType == VIDEOINPUT)
  {
      return m_VideoSource->GetValue();
  }
  else if (InputType == AUDIOINPUT)
  {
      return m_pBT848Card->GetAudioInput();    
  }
  return -1;
}

const char* CBT848Source::GetInputName(eSourceInputType InputType, int Nr)
{
  if (InputType == VIDEOINPUT)
  {
      if ((Nr>=0) && (Nr < m_pBT848Card->GetNumInputs()) )
      {
          return m_pBT848Card->GetInputName(Nr);
      }
  } 
  else if (InputType == AUDIOINPUT)
  {      
      return m_pBT848Card->GetAudioInputName((eAudioInput)Nr);
  }
  return NULL;
}

BOOL CBT848Source::InputHasTuner(eSourceInputType InputType, int Nr)
{
  if (InputType == VIDEOINPUT)
  {
    if(m_TunerType->GetValue() != TUNER_ABSENT)
    {
        return m_pBT848Card->IsInputATuner(Nr);
    }
    else
    {
        return FALSE;
    }
  }
  return FALSE;
}

CTreeSettingsGeneric* CBT848Source::BT848_GetTreeSettingsPage()
{
    vector <CSimpleSetting*>vSettingsList;

    vSettingsList.push_back(m_BDelay);
    vSettingsList.push_back(m_BtAgcDisable);
    vSettingsList.push_back(m_BtCrush);
    vSettingsList.push_back(m_BtEvenChromaAGC);
    vSettingsList.push_back(m_BtOddChromaAGC);
    vSettingsList.push_back(m_BtEvenLumaPeak);
    vSettingsList.push_back(m_BtOddLumaPeak);
    vSettingsList.push_back(m_BtFullLumaRange);
    vSettingsList.push_back(m_BtEvenLumaDec);
    vSettingsList.push_back(m_BtOddLumaDec);
    vSettingsList.push_back(m_BtEvenComb);
    vSettingsList.push_back(m_BtOddComb);
    vSettingsList.push_back(m_BtGammaCorrection);
    vSettingsList.push_back(m_BtCoring);
    vSettingsList.push_back(m_BtHorFilter);
    vSettingsList.push_back(m_BtVertFilter);
    vSettingsList.push_back(m_BtColorKill);
    vSettingsList.push_back(m_BtWhiteCrushUp);
    vSettingsList.push_back(m_BtWhiteCrushDown);
    vSettingsList.push_back(m_CustomPixelWidth);
    vSettingsList.push_back(m_HDelay);
    vSettingsList.push_back(m_VDelay);
    vSettingsList.push_back(m_ReversePolarity);

    return new CTreeSettingsGeneric("BT8x8 Advanced",vSettingsList);
}

