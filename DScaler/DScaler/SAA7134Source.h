/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Source.h,v 1.25 2002-11-07 20:33:17 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Atsushi Nakagawa.  All rights reserved.
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
// This software was based on BT848Source.cpp.  Those portions are
// Copyright (c) 2001 John Adcock.
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 09 Sep 2002   Atsushi Nakagawa      Initial Release
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.24  2002/11/07 18:54:20  atnak
// Redid getting next field -- fixes some issues
//
// Revision 1.23  2002/10/31 05:39:02  atnak
// Added SoundChannel change event for toolbar
//
// Revision 1.22  2002/10/31 05:02:55  atnak
// Settings cleanup and audio tweaks
//
// Revision 1.21  2002/10/31 03:10:55  atnak
// Changed CSource::GetTreeSettingsPage to return CTreeSettingsPage*
//
// Revision 1.20  2002/10/29 03:07:18  atnak
// Added SAA713x TreeSettings Page
//
// Revision 1.19  2002/10/28 11:10:11  atnak
// Various changes and revamp to settings
//
// Revision 1.18  2002/10/26 04:42:50  atnak
// Added AGC config and automatic volume leveling control
//
// Revision 1.17  2002/10/23 17:05:18  atnak
// Added variable VBI sample rate scaling
//
// Revision 1.16  2002/10/22 04:08:50  flibuste2
// -- Modified CSource to include virtual ITuner* GetTuner();
// -- Modified HasTuner() and GetTunerId() when relevant
//
// Revision 1.15  2002/10/20 07:41:04  atnak
// custom audio standard setup + etc
//
// Revision 1.14  2002/10/12 20:03:12  atnak
// added half second wait for DecodeVBI() after channel change
//
// Revision 1.13  2002/10/12 01:37:28  atnak
// fixes negative dropped frames bug
//
// Revision 1.12  2002/10/08 20:35:39  atnak
// whitepeak, colorpeak, comb filter UI options
//
// Revision 1.11  2002/10/08 12:30:38  atnak
// tweaks and fixes
//
// Revision 1.10  2002/10/06 09:49:19  atnak
// Smarter GetNextField sleeping
//
// Revision 1.9  2002/10/04 23:40:46  atnak
// proper support for audio channels mono,stereo,lang1,lang2 added
//
// Revision 1.8  2002/10/03 23:36:22  atnak
// Various changes (major): VideoStandard, AudioStandard, CSAA7134Common, cleanups, tweaks etc,
//
// Revision 1.7  2002/09/28 13:33:04  kooiman
// Added sender object to events and added setting flag to treesettingsgeneric.
//
// Revision 1.6  2002/09/26 11:33:42  kooiman
// Use event collector
//
// Revision 1.5  2002/09/16 17:52:34  atnak
// Support for SAA7134Res.dll dialogs
//
// Revision 1.4  2002/09/14 19:40:48  atnak
// various changes
//
// Revision 1.3  2002/09/10 12:24:03  atnak
// changed some UI stuff
//
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __SAA7134SOURCE_H___
#define __SAA7134SOURCE_H___

#include "Source.h"
#include "SAA7134Card.h"
#include "HardwareMemory.h"
#include "Setting.h"
#include "SAA7134_Defines.h"
/// \todo get rid of dependencies below here
#include "Other.h"

/** The source controls a specific CSAA7134Card to provide interlaced video
*/
class CSAA7134Source : public CSource,
                       public CSAA7134Common
{
    DECLARE_CLASS_SETTINGS(CSAA7134Source);
public:
    CSAA7134Source(CSAA7134Card* pSAA7134Card, CContigMemory* PagelistDMAMem[kMAX_PAGETABLES], CUserMemory* DisplayDMAMem[kMAX_FRAMEBUFFERS], CUserMemory* VBIDMAMem[kMAX_FRAMEBUFFERS], LPCSTR IniSection, LPCSTR ChipName, int DeviceIndex);
    ~CSAA7134Source();

    void Start();
    void Stop();
    void Reset();

    void Mute();
    void UnMute();

    void GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming);
    void DecodeVBI(TDeinterlaceInfo* pInfo);

    BOOL SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat);

    LPCSTR GetStatus();
    BOOL IsInTunerMode();
    BOOL IsVideoPresent();

    ITuner* GetTuner();

    void SetFormat(eVideoFormat NewFormat);
    eVideoFormat GetFormat();

    void SetOverscan();

    int GetWidth();
    int GetHeight();

    int GetDeviceIndex();
    const char* GetChipName();
    LPCSTR IDString() { return m_IDString.c_str(); }

    int NumInputs(eSourceInputType InputType);
    BOOL SetInput(eSourceInputType InputType, int Nr);
    int GetInput(eSourceInputType InputType);
    const char* GetInputName(eSourceInputType InputType, int Nr);
    BOOL InputHasTuner(eSourceInputType InputType, int Nr);

    ISetting* GetVolume();
    ISetting* GetBalance();

    ISetting* GetBrightness();
    ISetting* GetContrast();
    ISetting* GetHue();
    ISetting* GetSaturation();
    ISetting* GetSaturationU();
    ISetting* GetSaturationV();
    ISetting* GetOverscan();

    LPCSTR GetMenuLabel();
    void SetMenu(HMENU hMenu);
    void UpdateMenu() {return;};

    virtual void OnEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp);

    BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);
    void HandleTimerMessages(int TimerId);

    void SavePerChannelSetup(int Start);
    void ChangeSettingsBasedOnHW(int ProcessorSpeed, int TradeOff);

    BOOL OpenMediaFile(LPCSTR FileName, BOOL NewPlayList) {return FALSE;};
    BOOL IsAccessAllowed() {return TRUE;};
    BOOL HasSquarePixels() {return FALSE;};

    CTreeSettingsPage* CSAA7134Source::GetTreeSettingsPage();

private:
    virtual void CreateSettings(LPCSTR IniSection);

    void SetupCard();

    void SetupVideoSource();
    void SetupAudioSource();
    void SetupVideoStandard();
    void SetupAudioStandard();
    void UpdateAudioStatus();

    void SetupDMAMemory();
    DWORD CreatePageTable(CUserMemory* pDMAMemory, DWORD nPagesWanted, LPDWORD pPageTable);
    
    TPicture* GetFieldBuffer(TFieldID FieldID);
    void GetFrameIndex(TFieldID FieldID, BYTE* pFrameIndex, BOOL* pIsFieldOdd);

    void GiveNextField(TDeinterlaceInfo* pInfo, TPicture* picture);
    void GetNextFieldNormal(TDeinterlaceInfo* pInfo);
    void GetNextFieldAccurate(TDeinterlaceInfo* pInfo);

    BOOL PollForNextField(TFieldID* pNextFieldID, int* pFieldDistance, BOOL bSmartSleep);

    void InitializeSmartSleep();
    void PerformSmartSleep(ULONGLONG* pPerformanceTick, ULONGLONG* pTimePassed);
    void UpdateSmartSleep(BOOL bRecalculate, ULONGLONG LastTick, ULONGLONG TimePassed);

    void InitializeUI();
    void CleanupUI();

    static BOOL APIENTRY SelectCardProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
    static BOOL APIENTRY RegisterEditProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
    static BOOL APIENTRY AudioStandardProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
    static BOOL APIENTRY OtherEditProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

    void SetupSettings();
    void SaveSettings(WORD ChangedSetup);
    void LoadSettings(WORD ChangedSetup);
    void ChangeDefaultsForSetup(WORD ChangedSetup);
    void ChangeDefaultsForVideoInput();
    void ChangeDefaultsForVideoFormat();
    void ChangeDefaultsForAudioInput();
    void ChangeTVSettingsBasedOnTuner();

    void GetIniSectionName(char* pBuffer, WORD IniSectionMask);
    void ChangeChannelSectionNames();


protected:
    enum eSettingsSetup
    {

        SETUP_SINGLE                = 0UL,
        SETUP_NONE                  = 0UL,
        SETUP_PER_VIDEOINPUT        = 1 << 0,
        SETUP_PER_VIDEOFORMAT       = 1 << 1,
        SETUP_PER_AUDIOINPUT        = 1 << 2,
        SETUP_PER_CHANNEL           = 1 << 3,
        SETUP_CHANGE_VIDEOINPUT     = 1 << 4,
        SETUP_CHANGE_VIDEOFORMAT    = 1 << 5,
        SETUP_CHANGE_AUDIOINPUT     = 1 << 6,
        SETUP_CHANGE_CHANNEL        = 1 << 7,
        SETUP_CHANGE_ANY            = 0x00F0,
    };

    typedef struct
    {
        CSimpleSetting*    Setting;
        DWORD               Setup;
        
    } TSettingsSetup;

    TSettingsSetup* m_SettingsSetup;

    HINSTANCE       m_hSAA7134ResourceInst;

private:
    CSAA7134Card*   m_pSAA7134Card;
    BYTE*           m_pDisplay[kMAX_FRAMEBUFFERS];
    BYTE*           m_pVBILines[kMAX_FRAMEBUFFERS];
    CUserMemory*    m_VBIDMAMem[kMAX_FRAMEBUFFERS];
    CUserMemory*    m_DisplayDMAMem[kMAX_FRAMEBUFFERS];
    DWORD           m_DisplayPageTablePhysical[kMAX_VID_PAGETABLES];
    DWORD*          m_DisplayPageTableLinear[kMAX_VID_PAGETABLES];
    DWORD           m_VBIPageTablePhysical[kMAX_VBI_PAGETABLES];
    DWORD*          m_VBIPageTableLinear[kMAX_VBI_PAGETABLES];
    TPicture        m_EvenFields[kMAX_FRAMEBUFFERS];
    TPicture        m_OddFields[kMAX_FRAMEBUFFERS];
    long            m_CurrentX;
    long            m_CurrentY;
    long            m_CurrentVBILines;
    BOOL            m_InSaturationUpdate;
    int             m_CurrentChannel;    
    std::string     m_ChannelSubSection;
    BOOL            m_SettingsByChannelStarted;
    std::string     m_ChipName;
    int             m_DeviceIndex;

    std::string     m_Section;
    std::string     m_IDString;

    eAudioChannel   m_DetectedAudioChannel;

    DWORD           m_ChannelChangeTick;

    ULONGLONG       m_PerformanceFrequency;
    // Used to time field to field tick delay
    ULONGLONG       m_LastFieldPerformanceCount;
    // Used to store current field to field microsecond delay
    ULONGLONG       m_MinimumFieldDelay;

    // The field the card is currently processing
    TFieldID        m_ProcessingFieldID;
    // The field that is currently in use
    TFieldID        m_CurrentFieldID;

    BOOL            m_bSelectCardCancelButton;

    CSliderSetting* m_CustomPixelWidth;
    CYesNoSetting*  m_ReversePolarity;
    CYesNoSetting*  m_VBIDebugOverlay;
    CSliderSetting* m_CardType;
    CYesNoSetting*  m_bSavePerInput;
    CYesNoSetting*  m_bSavePerFormat;
    CYesNoSetting*  m_bSavePerChannel;

    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, Brightness);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, Contrast);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, Hue);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, Saturation);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, Overscan);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, PixelWidth);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, VideoSource);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, VideoFormat);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, TunerType);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, HDelay);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, VDelay);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, HPLLMode);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, Volume);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, Bass);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, Treble);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, Balance);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioSource);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioStandard);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioChannel);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioSampleRate);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, VBIUpscaleDivisor);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, GainControlLevel);
    DEFINE_YESNO_CALLBACK_SETTING(CSAA7134Source,  AutomaticGainControl);
    DEFINE_YESNO_CALLBACK_SETTING(CSAA7134Source,  VideoMirror);
    DEFINE_YESNO_CALLBACK_SETTING(CSAA7134Source,  AutoStereoSelect);
    DEFINE_YESNO_CALLBACK_SETTING(CSAA7134Source,  WhitePeak);
    DEFINE_YESNO_CALLBACK_SETTING(CSAA7134Source,  ColorPeak);
    DEFINE_YESNO_CALLBACK_SETTING(CSAA7134Source,  AdaptiveCombFilter);
    DEFINE_YESNO_CALLBACK_SETTING(CSAA7134Source,  CustomAudioStandard);
    DEFINE_LIST_CALLBACK_SETTING(CSAA7134Source,   AutomaticVolumeLevel);

    // These settings are only effective when CustomAudioStandard is set
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioMajorCarrier);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioMinorCarrier);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioMajorCarrierMode);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioMinorCarrierMode);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioCh1FMDeemph);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioCh2FMDeemph);
};


#endif

