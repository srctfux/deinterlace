/////////////////////////////////////////////////////////////////////////////
// $Id: CX2388xSource.h,v 1.16 2003-01-07 23:27:02 laurentg Exp $
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
// This code is based on a version of dTV modified by Michael Eskin and
// others at Connexant.  Those parts are probably (c) Connexant 2002
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __CX2388XSOURCE_H___
#define __CX2388XSOURCE_H___

#include "Source.h"
#include "CX2388xCard.h"
#include "HardwareMemory.h"
#include "Setting.h"
#include "CX2388x_Defines.h"
/// \todo get rid of dependencies below here
#include "Other.h"

/** The source controls a specific CCX2388xCard to provide interlaced video
*/
class CCX2388xSource : public CSource
{
    DECLARE_CLASS_SETTINGS(CCX2388xSource);
public:
    CCX2388xSource(CCX2388xCard* pCard, CContigMemory* RiscDMAMem, CUserMemory* DisplayDMAMem[5], CUserMemory* VBIDMAMem[5], LPCSTR IniSection);
    ~CCX2388xSource();
    void Start();
    void Stop();
    void Reset();
    void GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming);
    BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);
    CCX2388xCard* GetCard();
    LPCSTR GetStatus();
    ISetting* GetVolume() {return m_Volume;};
    ISetting* GetBalance() {return m_Balance;};
    void Mute();
    void UnMute();
    ISetting* GetBrightness();
    ISetting* GetContrast();
    ISetting* GetHue();
    ISetting* GetSaturation();
    ISetting* GetSaturationU();
    ISetting* GetSaturationV();
    ISetting* GetTopOverscan();
    ISetting* GetBottomOverscan();
    ISetting* GetLeftOverscan();
    ISetting* GetRightOverscan();
    eVideoFormat GetFormat();
    void SetFormat(eVideoFormat NewFormat);
    BOOL IsInTunerMode();
    int GetWidth();
    int GetHeight();
    void UpdateMenu() {return;};
    void SetMenu(HMENU hMenu);
  
    CTreeSettingsPage* GetTreeSettingsPage();

    void HandleTimerMessages(int TimerId);
    BOOL SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat);
    BOOL IsVideoPresent();
    void DecodeVBI(TDeinterlaceInfo* pInfo);
    LPCSTR GetMenuLabel();
    BOOL OpenMediaFile(LPCSTR FileName, BOOL NewPlayList) {return FALSE;};
    BOOL IsAccessAllowed() {return TRUE;};
    void SetOverscan();
    BOOL HasSquarePixels() {return FALSE;};
    void ChangeSettingsBasedOnHW(int ProcessorSpeed, int TradeOff);
    void SavePerChannelSetup(int Start);
    LPCSTR IDString() { return m_IDString.c_str(); }

    int  NumInputs(eSourceInputType InputType);
    BOOL SetInput(eSourceInputType InputType, int Nr);
    int GetInput(eSourceInputType InputType);
    const char* GetInputName(eSourceInputType InputType, int Nr);
    BOOL InputHasTuner(eSourceInputType InputType, int Nr);

    virtual void OnEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp);

    ITuner* GetTuner();

private:
    virtual void CreateSettings(LPCSTR IniSection);

    void CreateRiscCode(BOOL bCaptureVBI);
    
    static BOOL APIENTRY SelectCardProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

    void GetNextFieldNormal(TDeinterlaceInfo* pInfo);
    void GetNextFieldAccurate(TDeinterlaceInfo* pInfo);
    void GetNextFieldNormalProg(TDeinterlaceInfo* pInfo);
    void GetNextFieldAccurateProg(TDeinterlaceInfo* pInfo);
    void SmartSleep(TDeinterlaceInfo* pInfo, BOOL bRunningLate);

    void SetupCard();
    void ChangeTVSettingsBasedOnTuner();
    void ChangeDefaultsForVideoFormat();
    void ChangeDefaultsForVideoInput();
    void SaveSettings(WORD ChangedSetup);
    void LoadSettings(WORD ChangedSetup);
    void SetupPictureStructures();
    void GetIniSectionName(char* pBuffer, WORD IniSectionMask);
    void ChangeDefaultsForSetup(WORD Setup);
    void SetupSettings();

    void ChangeChannelSectionNames();

    ISetting* GetCurrentAudioSetting();

    void InitializeUI();

private:
    CCX2388xCard*  m_pCard;
    BYTE*        m_pDisplay[5];
    CUserMemory* m_DisplayDMAMem[5];
    BYTE*        m_pVBILines[5];
    CUserMemory* m_VBIDMAMem[5];
    DWORD        m_RiscBasePhysical; 
    DWORD        m_RiscBasePhysicalVBI; 
    DWORD*       m_RiscBaseLinear;
    long         m_BytesPerRISCField;
    TPicture     m_EvenFields[5];
    TPicture     m_OddFields[5];
    long         m_CurrentX;
    long         m_CurrentY;
    long         m_CurrentVBILines;
    BOOL         m_IsFieldOdd;
    BOOL         m_InSaturationUpdate;
    int          m_CurrentChannel;    
    std::string  m_ChannelSubSection;
    BOOL         m_SettingsByChannelStarted;
    int          m_NumFields;
    HINSTANCE    m_hCX2388xResourceInst;


    std::string  m_Section;
    std::string  m_IDString;

    CSliderSetting* m_CardType;
    CYesNoSetting* m_bSavePerInput;
    CYesNoSetting* m_bSavePerFormat;
    CYesNoSetting* m_bSavePerChannel;
    CSliderSetting* m_CustomPixelWidth;

    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, Brightness);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, Contrast);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, Hue);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, Saturation);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, SaturationU);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, SaturationV);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, TopOverscan);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, BottomOverscan);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, LeftOverscan);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, RightOverscan);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, TunerType);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, VideoSource);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, VideoFormat);
    DEFINE_YESNO_CALLBACK_SETTING(CCX2388xSource, IsVideoProgressive);
	DEFINE_YESNO_CALLBACK_SETTING(CCX2388xSource, FLIFilmDetect);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, HDelay);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, VDelay);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, EatLinesAtTop);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, Sharpness);
	DEFINE_YESNO_CALLBACK_SETTING(CCX2388xSource, LumaAGC);
	DEFINE_YESNO_CALLBACK_SETTING(CCX2388xSource, ChromaAGC);
	DEFINE_YESNO_CALLBACK_SETTING(CCX2388xSource, FastSubcarrierLock);
	DEFINE_YESNO_CALLBACK_SETTING(CCX2388xSource, WhiteCrush);
	DEFINE_YESNO_CALLBACK_SETTING(CCX2388xSource, LowColorRemoval);
    DEFINE_LIST_CALLBACK_SETTING(CCX2388xSource, CombFilter);
	DEFINE_YESNO_CALLBACK_SETTING(CCX2388xSource, FullLumaRange);
	DEFINE_LIST_CALLBACK_SETTING(CCX2388xSource, Remodulation);
	DEFINE_LIST_CALLBACK_SETTING(CCX2388xSource, Chroma2HComb);
	DEFINE_LIST_CALLBACK_SETTING(CCX2388xSource, ForceRemodExcessChroma);
	DEFINE_LIST_CALLBACK_SETTING(CCX2388xSource, IFXInterpolation);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, CombRange);
	DEFINE_LIST_CALLBACK_SETTING(CCX2388xSource, SecondChromaDemod);
	DEFINE_LIST_CALLBACK_SETTING(CCX2388xSource, ThirdChromaDemod);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, PixelWidth);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, WhiteCrushUp);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, WhiteCrushDown);
	DEFINE_LIST_CALLBACK_SETTING(CCX2388xSource, WhiteCrushMajorityPoint);
	DEFINE_YESNO_CALLBACK_SETTING(CCX2388xSource, WhiteCrushPerFrame);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, Volume);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, Balance);
	DEFINE_LIST_CALLBACK_SETTING(CCX2388xSource, AudioStandard);
	DEFINE_LIST_CALLBACK_SETTING(CCX2388xSource, StereoType);

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


};




#endif