/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Source.h,v 1.61 2003-10-10 11:15:12 laurentg Exp $
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
//
// $Log: not supported by cvs2svn $
// Revision 1.60  2003/08/15 18:20:26  laurentg
// Save in the source if it is the first setup
//
// Revision 1.59  2003/08/11 20:45:55  laurentg
// Method to know if the source can be controled through a media player
//
// Revision 1.58  2003/07/22 22:31:36  laurentg
// Correct handling of pause (P key) for video file playing
//
// Revision 1.57  2003/07/05 10:55:57  laurentg
// New method SetWidth
//
// Revision 1.56  2003/01/19 19:45:12  laurentg
// GetInitialHeight must return the height of the input picture
//
// Revision 1.55  2003/01/19 11:09:10  laurentg
// New methods GetInitialWidth and GetInitialHeight to store the initial size before resizing in DScaler (for stills)
//
// Revision 1.54  2003/01/18 13:55:43  laurentg
// New methods GetHDelay and GetVDelay
//
// Revision 1.53  2003/01/18 10:52:11  laurentg
// SetOverscan renamed SetAspectRatioData
// Unnecessary call to SetOverscan deleted
// Overscan setting specific to calibration deleted
//
// Revision 1.52  2003/01/16 13:30:49  adcockj
// Fixes for various settings problems reported by Laurent 15/Jan/2003
//
// Revision 1.51  2003/01/10 17:37:45  adcockj
// Interrim Check in of Settings rewrite
//  - Removed SETTINGSEX structures and flags
//  - Removed Seperate settings per channel code
//  - Removed Settings flags
//  - Cut away some unused features
//
// Revision 1.50  2003/01/08 19:59:35  laurentg
// Analogue Blanking setting by source
//
// Revision 1.49  2003/01/07 23:27:01  laurentg
// New overscan settings
//
// Revision 1.48  2002/11/07 20:33:16  adcockj
// Promoted ACPI functions so that state management works properly
//
// Revision 1.47  2002/11/05 14:36:22  adcockj
// Remove bt848 advanced flags dialog, now replaced by generic one
//
// Revision 1.46  2002/10/31 03:10:55  atnak
// Changed CSource::GetTreeSettingsPage to return CTreeSettingsPage*
//
// Revision 1.45  2002/10/29 03:05:48  atnak
// Added a virtual GetTreeSettingsPage() to all CSources
//
// Revision 1.44  2002/10/22 04:08:50  flibuste2
// -- Modified CSource to include virtual ITuner* GetTuner();
// -- Modified HasTuner() and GetTunerId() when relevant
//
// Revision 1.43  2002/10/15 18:31:44  kooiman
// Added stereo detect interval for continuous scanning for stereo mode.
//
// Revision 1.42  2002/10/11 21:44:40  ittarnavsky
// added rcs/cvs log tag
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __BT848SOURCE_H___
#define __BT848SOURCE_H___

#include "Source.h"
#include "BT848Card.h"
#include "HardwareMemory.h"
#include "Setting.h"
#include "Bt848_Defines.h"
/// \todo get rid of dependencies below here
#include "Other.h"

/** The source controls a specific CBT848Card to provide interlaced video
*/
class CBT848Source : public CSource
{
    DECLARE_CLASS_SETTINGS(CBT848Source);
public:
    CBT848Source(CBT848Card* pBT848Card, CContigMemory* RiscDMAMem, CUserMemory* DisplayDMAMem[5], CUserMemory* VBIDMAMem[5], LPCSTR IniSection, LPCSTR ChipName, int DeviceIndex);
    ~CBT848Source();
    virtual void OnEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp);
    void Start();
    void Stop();
    void Reset();
    void GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming);
    BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);
    CBT848Card* GetBT848Card();
    LPCSTR GetStatus();
    ISetting* GetVolume();
    ISetting* GetBalance();
    void Mute();
    void UnMute();
    ISetting* GetBrightness();
    ISetting* GetContrast();
    ISetting* GetHue();
    ISetting* GetSaturation();
    ISetting* GetSaturationU();
    ISetting* GetSaturationV();
    ISetting* GetAnalogueBlanking() {return NULL;};
    ISetting* GetTopOverscan();
    ISetting* GetBottomOverscan();
    ISetting* GetLeftOverscan();
    ISetting* GetRightOverscan();
	ISetting* GetHDelay();
	ISetting* GetVDelay();
    /// Gets the current field being processed by the card
    int GetRISCPosAsInt();
	
    eVideoFormat GetFormat();
    void SetFormat(eVideoFormat NewFormat);
    BOOL IsInTunerMode();
    int GetInitialWidth() {return GetWidth();};
    int GetInitialHeight() {return GetHeight() / 2;};
    int GetWidth();
    int GetHeight();
    void SetWidth(int w);
    void UpdateMenu() {return;};
    void SetMenu(HMENU hMenu);
    
    void HandleTimerMessages(int TimerId);
    BOOL SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat);
    BOOL IsVideoPresent();
    void DecodeVBI(TDeinterlaceInfo* pInfo);
    LPCSTR GetMenuLabel();
    BOOL OpenMediaFile(LPCSTR FileName, BOOL NewPlayList) {return FALSE;};
    BOOL IsAccessAllowed() {return TRUE;};
    void SetAspectRatioData();
    BOOL HasSquarePixels() {return FALSE;};
    void ChangeSettingsBasedOnHW(int ProcessorSpeed, int TradeOff);
    void ChannelChange(int PreChange,int OldChannel,int NewChannel);
    int GetDeviceIndex();
    const char* GetChipName();
    LPCSTR IDString() { return m_IDString.c_str(); }
    int  NumInputs(eSourceInputType InputType);
    BOOL SetInput(eSourceInputType InputType, int Nr);
    int GetInput(eSourceInputType InputType);
    const char* GetInputName(eSourceInputType InputType, int Nr);
    BOOL InputHasTuner(eSourceInputType InputType, int Nr);

    ITuner* GetTuner();

    CTreeSettingsPage* GetTreeSettingsPage();
    
    void SetSourceAsCurrent();

    void Pause() {return;};
    void UnPause() {return;};

	BOOL HasMediaControl() {return FALSE;};

    BOOL IsAudioMixerAccessAllowed() {return TRUE;};

	BOOL IsInitialSetup() {return m_InitialSetup;};

private:
    virtual void CreateSettings(LPCSTR IniSection);

    void CreateRiscCode(BOOL bCaptureVBI);
    
    static BOOL APIENTRY SelectCardProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
    static BOOL APIENTRY AudioSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
    static BOOL APIENTRY AudioStandardManualProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);	
    
    void GetNextFieldNormal(TDeinterlaceInfo* pInfo);
    void GetNextFieldAccurate(TDeinterlaceInfo* pInfo);
    void SmartSleep(TDeinterlaceInfo* pInfo, BOOL bRunningLate);

    void SetupCard();
    void ChangeTVSettingsBasedOnTuner();
    void InitAudio();
    void InitAudioControls();

    ISetting* GetCurrentAudioSetting();

    //static void StaticAudioStandardDetected(void *pThis, int What, long Value);	
    void AudioStandardDetected(long Standard);
    void SupportedSoundChannelsDetected(eSupportedSoundChannels supported);

private:
    CBT848Card*  m_pBT848Card;
    BYTE*        m_pDisplay[5];
    BYTE*        m_pVBILines[5];
    CUserMemory* m_VBIDMAMem[5];
    CUserMemory* m_DisplayDMAMem[5];
    DWORD        m_RiscBasePhysical; 
    DWORD*       m_RiscBaseLinear;
    long         m_BytesPerRISCField;
    TPicture     m_EvenFields[5];
    TPicture     m_OddFields[5];
    long         m_CurrentX;
    long         m_CurrentY;
    long         m_CurrentVBILines;
    BOOL         m_IsFieldOdd;
    BOOL         m_InSaturationUpdate;
    std::string  m_ChannelSubSection;
    std::string  m_ChipName;
    int          m_DeviceIndex;

    std::string  m_Section;
    std::string  m_IDString;
    int          m_DetectingAudioStandard;
    BOOL         m_InitAudioControls;
    int          m_KeepDetectingStereo;

    CSliderSetting* m_CustomPixelWidth;
    CYesNoSetting* m_ReversePolarity;
    CSliderSetting* m_CardType;

	BOOL		 m_InitialSetup;

    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Brightness);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Contrast);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Hue);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Saturation);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, SaturationU);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, SaturationV);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, TopOverscan);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, BottomOverscan);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, LeftOverscan);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, RightOverscan);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, BDelay);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, HDelay);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, VDelay);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, BtWhiteCrushUp);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, BtWhiteCrushDown);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, PixelWidth);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, VideoSource);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, VideoFormat);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, TunerType);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtAgcDisable);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtCrush);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtEvenChromaAGC);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtOddChromaAGC);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtEvenLumaPeak);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtOddLumaPeak);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtFullLumaRange);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtEvenLumaDec);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtOddLumaDec);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtEvenComb);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtOddComb);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtColorBars);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtGammaCorrection);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtVertFilter);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtHorFilter);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtCoring);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtColorKill);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Volume);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Bass);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Treble);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Balance);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, AutoStereoSelect);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, AudioSource1);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, AudioSource2);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, AudioSource3);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, AudioSource4);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, AudioSource5);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, AudioSource6);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, AudioChannel);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, UseInputPin1);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, UseEqualizer);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, EqualizerBand1);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, EqualizerBand2);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, EqualizerBand3);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, EqualizerBand4);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, EqualizerBand5);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, AudioLoudness);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, AudioSuperbass);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, AudioSpatialEffect);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, AudioAutoVolumeCorrection);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, AudioStandardDetect);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, AudioStandardDetectInterval);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, AudioStandardManual);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, AudioStandardMajorCarrier);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, AudioStandardMinorCarrier);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, AudioStandardInStatusBar);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, MSP34xxFlags);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, AutoStereoDetectInterval);

protected:
    void ChangeDefaultsForVideoFormat(BOOL bDontSetValue);
    void ChangeDefaultsForVideoInput(BOOL bDontSetValue) {};
    void ChangeDefaultsForAudioInput(BOOL bDontSetValue) {};
};




#endif