/////////////////////////////////////////////////////////////////////////////
// $id$
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
// This software was based on BT848Souce_UI.cpp.  Those portions are
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
// $log$
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "SAA7134Source.h"
#include "SAA7134_Defines.h"
#include "DScaler.h"
#include "OutThreads.h"
#include "AspectRatio.h"
#include "DebugLog.h"
#include "SettingsPerChannel.h"

extern const char *TunerNames[TUNER_LASTONE];


// SAA7134: This file still needs loads of work


BOOL APIENTRY CSAA7134Source::AudioSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
/*    static int  TVolume;
    static char TBalance;
    static char TBass;
    static char TTreble;
    static CSAA7134Source* pThis;

    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CSAA7134Source*)lParam;

        TVolume = pThis->m_Volume->GetValue();
        TBass = pThis->m_Bass->GetValue();
        TTreble = pThis->m_Treble->GetValue();
        TBalance = pThis->m_Balance->GetValue();

        SetDlgItemInt(hDlg, IDC_D1, TVolume, FALSE);
        SetDlgItemInt(hDlg, IDC_D4, TBass, TRUE);
        SetDlgItemInt(hDlg, IDC_D5, TTreble, TRUE);
        SetDlgItemInt(hDlg, IDC_D6, TBalance, TRUE);

        pThis->m_Volume->SetupControl(GetDlgItem(hDlg, IDC_SLIDER1));
        pThis->m_Bass->SetupControl(GetDlgItem(hDlg, IDC_SLIDER4));
        pThis->m_Treble->SetupControl(GetDlgItem(hDlg, IDC_SLIDER5));
        pThis->m_Balance->SetupControl(GetDlgItem(hDlg, IDC_SLIDER6));
        break;

    case WM_VSCROLL:
    case WM_HSCROLL:
        if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER1))
        {
            pThis->m_Volume->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D1, pThis->m_Volume->GetValue(), TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER4))
        {
            pThis->m_Bass->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D4, pThis->m_Bass->GetValue(), TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER5))
        {
            pThis->m_Treble->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D5, pThis->m_Treble->GetValue(), TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER6))
        {
            pThis->m_Balance->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D6, pThis->m_Balance->GetValue(), TRUE);
        }
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            WriteSettingsToIni(TRUE);
            EndDialog(hDlg, TRUE);
            break;
        
        case IDCANCEL:
            pThis->Mute();
            pThis->m_Volume->SetValue(TVolume);
            pThis->m_Bass->SetValue(TBass);
            pThis->m_Treble->SetValue(TTreble);
            pThis->m_Balance->SetValue(TBalance);
            pThis->UnMute();
            EndDialog(hDlg, TRUE);
            break;
        
        case IDC_DEFAULT:
            pThis->Mute();
            pThis->m_Volume->SetDefault();
            pThis->m_Bass->SetDefault();
            pThis->m_Treble->SetDefault();
            pThis->m_Balance->SetDefault();
            SetDlgItemInt(hDlg, IDC_D1, pThis->m_Volume->GetValue(), FALSE);
            SetDlgItemInt(hDlg, IDC_D4, pThis->m_Bass->GetValue(), TRUE);
            SetDlgItemInt(hDlg, IDC_D5, pThis->m_Treble->GetValue(), TRUE);
            SetDlgItemInt(hDlg, IDC_D6, pThis->m_Balance->GetValue(), TRUE);

            pThis->m_Volume->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER1));
            pThis->m_Bass->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER4));
            pThis->m_Treble->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER5));
            pThis->m_Balance->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER6));
            pThis->UnMute();
            break;
        default:
            break;

        }
        break;
    }*/
    return (FALSE);
}


BOOL APIENTRY CSAA7134Source::SelectCardProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    int i;
    int nIndex;
    char buf[128];
    static long OrigTuner;
    static CSAA7134Source* pThis;

    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CSAA7134Source*)lParam;
        sprintf(buf, "Setup card %u with chip %s", pThis->GetDeviceIndex() + 1, pThis->GetChipName());
        SetWindowText(hDlg, buf);
//      Button_Enable(GetDlgItem(hDlg, IDCANCEL), EnableCancelButton);
        SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_RESETCONTENT, 0, 0);
        for(i = 0; i < pThis->m_pSAA7134Card->GetMaxCards(); i++)
        {
            int nIndex;
            nIndex = SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_ADDSTRING, 0, (LONG)pThis->m_pSAA7134Card->GetCardName((eTVCardId)i));
            SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_SETITEMDATA, nIndex, i);
            if(i == pThis->m_CardType->GetValue())
            {
                SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_SETCURSEL, nIndex, 0);
            }
        }

        SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_RESETCONTENT, 0, 0);
        for(i = 0; i < TUNER_LASTONE; i++)
        {
            nIndex = SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_ADDSTRING, 0, (LONG)TunerNames[i]);
            SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_SETITEMDATA, nIndex, i);
        }

        OrigTuner = pThis->m_TunerType->GetValue();
        SetFocus(hDlg);
        // Update the tuner combobox after the SetFocus
        // because SetFocus modifies this combobox
        for (nIndex = 0; nIndex < TUNER_LASTONE; nIndex++)
        {
          i = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_TUNERSELECT), nIndex);
          if (i == pThis->m_TunerType->GetValue() )
          {          
            SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_SETCURSEL, nIndex, 0);
          }
        }
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            i = SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_GETCURSEL, 0, 0);
            pThis->m_TunerType->SetValue(ComboBox_GetItemData(GetDlgItem(hDlg, IDC_TUNERSELECT), i));

            i =  SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_GETCURSEL, 0, 0);
            pThis->m_CardType->SetValue(ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CARDSSELECT), i));
            if(OrigTuner != pThis->m_TunerType->GetValue())
            {
                pThis->ChangeTVSettingsBasedOnTuner();
            }
            WriteSettingsToIni(TRUE);
            EndDialog(hDlg, TRUE);
            break;
        case IDCANCEL:
            EndDialog(hDlg, TRUE);
            break;
        case IDC_CARDSSELECT:
            i = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_CARDSSELECT));
            i = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CARDSSELECT), i);                        
            i = pThis->m_pSAA7134Card->AutoDetectTuner((eTVCardId)i);
            for (nIndex = 0; nIndex < TUNER_LASTONE; nIndex++)
            {   
              if (ComboBox_GetItemData(GetDlgItem(hDlg, IDC_TUNERSELECT), nIndex) == i)
              {
                 ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_TUNERSELECT), nIndex);
              }
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return (FALSE);
}

void CSAA7134Source::SetMenu(HMENU hMenu)
{
    int i;
    MENUITEMINFO MenuItemInfo;
    char Buffer[265];

    // set up the input menu
    for(i = 0;i < m_pSAA7134Card->GetNumInputs(); ++i)
    {
        // reset the menu info structure
        memset(&MenuItemInfo, 0, sizeof(MenuItemInfo));
        MenuItemInfo.cbSize = sizeof(MenuItemInfo);
        MenuItemInfo.fMask = MIIM_TYPE;

        // get the size of the string
        GetMenuItemInfo(m_hMenu, IDM_SOURCE_INPUT1 + i, FALSE, &MenuItemInfo);
        // set the buffer and get the current string
        MenuItemInfo.dwTypeData = Buffer;
        GetMenuItemInfo(m_hMenu, IDM_SOURCE_INPUT1 + i, FALSE, &MenuItemInfo);
        // create the new string and correct the menu
        sprintf(Buffer, "%s\tCtrl+Alt+F%d",m_pSAA7134Card->GetInputName(i), i + 1);
        MenuItemInfo.cch = strlen(Buffer);
        SetMenuItemInfo(m_hMenu, IDM_SOURCE_INPUT1 + i, FALSE, &MenuItemInfo);
        
        // enable the menu and check it appropriately
        EnableMenuItem(m_hMenu, IDM_SOURCE_INPUT1 + i, MF_ENABLED);
        CheckMenuItemBool(m_hMenu, IDM_SOURCE_INPUT1 + i, (m_VideoSource->GetValue() == i));
    }
    
    while(i < INPUTS_PER_CARD)
    {
        EnableMenuItem(m_hMenu, IDM_SOURCE_INPUT1 + i, MF_GRAYED);
        ++i;
    }

    BOOL DoneWidth = FALSE;

    EnableMenuItemBool(m_hMenu, IDM_AUDIOSETTINGS1, FALSE);

    EnableMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_768, GetTVFormat((eVideoFormat)m_VideoFormat->GetValue())->wHActivex1 >= 768);

    CheckMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_768, (m_PixelWidth->GetValue() == 768));
    DoneWidth |= (m_PixelWidth->GetValue() == 768);
    CheckMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_754, (m_PixelWidth->GetValue() == 754));
    DoneWidth |= (m_PixelWidth->GetValue() == 754);
    CheckMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_720, (m_PixelWidth->GetValue() == 720));
    DoneWidth |= (m_PixelWidth->GetValue() == 720);
    CheckMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_640, (m_PixelWidth->GetValue() == 640));
    DoneWidth |= (m_PixelWidth->GetValue() == 640);
    CheckMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_480, (m_PixelWidth->GetValue() == 480));
    DoneWidth |= (m_PixelWidth->GetValue() == 480);
    CheckMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_384, (m_PixelWidth->GetValue() == 384));
    DoneWidth |= (m_PixelWidth->GetValue() == 384);
    CheckMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_320, (m_PixelWidth->GetValue() == 320));
    DoneWidth |= (m_PixelWidth->GetValue() == 320);
    CheckMenuItemBool(m_hMenu, IDM_SETTINGS_PIXELWIDTH_CUSTOM, !DoneWidth);

    eVideoFormat videoFormat = (eVideoFormat)m_VideoFormat->GetValue();
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_0, (IsPALVideoFormat(videoFormat) 
                                                    && videoFormat != VIDEOFORMAT_PAL_M
                                                    && videoFormat != VIDEOFORMAT_PAL_N
                                                    && videoFormat != VIDEOFORMAT_PAL_60
                                                    && videoFormat != VIDEOFORMAT_PAL_N_COMBO
                                                    ));
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_1, (videoFormat == VIDEOFORMAT_NTSC_M));
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_2, (IsSECAMVideoFormat(videoFormat)));
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_3, (videoFormat == VIDEOFORMAT_PAL_M));
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_4, (videoFormat == VIDEOFORMAT_PAL_N));
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_5, (videoFormat == VIDEOFORMAT_NTSC_M_Japan));
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_6, (videoFormat == VIDEOFORMAT_PAL_60));
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_7, (videoFormat == VIDEOFORMAT_NTSC_50));
    CheckMenuItemBool(m_hMenu, IDM_TYPEFORMAT_8, (videoFormat == VIDEOFORMAT_PAL_N_COMBO));

    CheckMenuItemBool(m_hMenu, IDM_AUDIO_0, (GetCurrentAudioSetting()->GetValue() == 0));
    CheckMenuItemBool(m_hMenu, IDM_AUDIO_1, (GetCurrentAudioSetting()->GetValue() == 1));
    CheckMenuItemBool(m_hMenu, IDM_AUDIO_2, (GetCurrentAudioSetting()->GetValue() == 2));
    CheckMenuItemBool(m_hMenu, IDM_AUDIO_3, (GetCurrentAudioSetting()->GetValue() == 3));
    CheckMenuItemBool(m_hMenu, IDM_AUDIO_4, (GetCurrentAudioSetting()->GetValue() == 4));
    CheckMenuItemBool(m_hMenu, IDM_AUDIO_5, (GetCurrentAudioSetting()->GetValue() == 5));

    CheckMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_MONO, (m_AudioChannel->GetValue() == 1));
    CheckMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_STEREO, (m_AudioChannel->GetValue() == 2));
    CheckMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_LANGUAGE1, (m_AudioChannel->GetValue() == 3));
    CheckMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_LANGUAGE2, (m_AudioChannel->GetValue() == 4));

    CheckMenuItemBool(m_hMenu, IDM_AUTOSTEREO, m_AutoStereoSelect->GetValue());

    CheckMenuItemBool(m_hMenu, IDM_SAVE_BY_FORMAT, m_bSavePerFormat->GetValue());
    CheckMenuItemBool(m_hMenu, IDM_SAVE_BY_INPUT, m_bSavePerInput->GetValue());
}

BOOL APIENTRY CSAA7134Source::AdvVideoSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
/*    static CSAA7134Source* pThis = NULL;


    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CSAA7134Source*) lParam;
        break;

    case WM_MOUSEMOVE:
        return (FALSE);

    case WM_COMMAND:

        switch LOWORD(wParam)
        {
        case IDCANCEL:                      //  Restore settings
            EndDialog(hDlg, TRUE);
            break;

        case IDOK:                          // Is Done
            WriteSettingsToIni(TRUE);
            EndDialog(hDlg, TRUE);
            break;

        }
        break;
    }*/
    return (FALSE);
}



BOOL CSAA7134Source::HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
{
    switch(LOWORD(wParam))
    {
        case IDM_SETUPCARD:
            Stop_Capture();
            DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_SELECTCARD), hWnd, (DLGPROC) SelectCardProc, (LPARAM)this);
            m_pSAA7134Card->SetCardType(m_CardType->GetValue());
            m_pSAA7134Card->InitTuner((eTunerId)m_TunerType->GetValue());
            m_pSAA7134Card->InitAudio();
            ChangeTVSettingsBasedOnCard();
            Start_Capture();
            break;

        case IDM_BDELAY_PLUS:
            break;

        case IDM_BDELAY_MINUS:
            break;

        case IDM_BDELAY_CURRENT:
            break;

        case IDM_HDELAY_PLUS:
            m_HDelay->ChangeValue(ADJUSTUP);
            SendMessage(hWnd, WM_COMMAND, IDM_HDELAY_CURRENT, 0);
            break;

        case IDM_HDELAY_MINUS:
            m_HDelay->ChangeValue(ADJUSTDOWN);
            SendMessage(hWnd, WM_COMMAND, IDM_HDELAY_CURRENT, 0);
            break;

        case IDM_HDELAY_CURRENT:
            m_HDelay->OSDShow();
            break;

        case IDM_VDELAY_PLUS:
            m_VDelay->ChangeValue(ADJUSTUP);
            SendMessage(hWnd, WM_COMMAND, IDM_VDELAY_CURRENT, 0);
            break;

        case IDM_VDELAY_MINUS:
            m_VDelay->ChangeValue(ADJUSTDOWN);
            SendMessage(hWnd, WM_COMMAND, IDM_VDELAY_CURRENT, 0);
            break;

        case IDM_VDELAY_CURRENT:
            m_VDelay->OSDShow();
            break;

        case IDM_PIXELWIDTH_PLUS:
            m_PixelWidth->ChangeValue(ADJUSTUP);
            SendMessage(hWnd, WM_COMMAND, IDM_PIXELWIDTH_CURRENT, 0);
            break;

        case IDM_PIXELWIDTH_MINUS:
            m_PixelWidth->ChangeValue(ADJUSTDOWN);
            SendMessage(hWnd, WM_COMMAND, IDM_PIXELWIDTH_CURRENT, 0);
            break;

        case IDM_PIXELWIDTH_CURRENT:
            m_PixelWidth->OSDShow();
            break;

        case IDM_SOUNDCHANNEL_MONO:
            m_AudioChannel->SetValue(SOUNDCHANNEL_MONO);
            break;
        case IDM_SOUNDCHANNEL_STEREO:
            m_AudioChannel->SetValue(SOUNDCHANNEL_STEREO);
            break;
        case IDM_SOUNDCHANNEL_LANGUAGE1:
            m_AudioChannel->SetValue(SOUNDCHANNEL_LANGUAGE1);
            break;
        case IDM_SOUNDCHANNEL_LANGUAGE2:
            m_AudioChannel->SetValue(SOUNDCHANNEL_LANGUAGE2);
            break;

        case IDM_AUTOSTEREO:
            m_AutoStereoSelect->SetValue(!m_AutoStereoSelect->GetValue());
            break;

        case IDM_USEINPUTPIN1:
            break;

        case IDM_AUDIO_0:
        case IDM_AUDIO_1:
        case IDM_AUDIO_2:
            GetCurrentAudioSetting()->SetValue((LOWORD(wParam) - IDM_AUDIO_0));
            switch (GetCurrentAudioSetting()->GetValue())
            {
            case AUDIOINPUTLINE_TUNER:     
                ShowText(hWnd, "Audio Input - Tuner");     
                break;
            case AUDIOINPUTLINE_LINE1: 
                ShowText(hWnd, "Audio Input - Line 1"); 
                break;
            case AUDIOINPUTLINE_LINE2:  
                ShowText(hWnd, "Audio Input - Line 2");  
                break;
            }
            break;

        case IDM_AUDIOSTANDARD_1:
        case IDM_AUDIOSTANDARD_2:
        case IDM_AUDIOSTANDARD_3:
        case IDM_AUDIOSTANDARD_4:
        case IDM_AUDIOSTANDARD_5:
        case IDM_AUDIOSTANDARD_6:
        case IDM_AUDIOSTANDARD_7:
        case IDM_AUDIOSTANDARD_8:
        case IDM_AUDIOSTANDARD_9:
        case IDM_AUDIOSTANDARD_10:
            {
                int nValue = LOWORD(wParam) - IDM_AUDIOSTANDARD_1;
                ShowText(hWnd, m_pSAA7134Card->GetAudioStandardName((eAudioStandard)nValue));
                m_AudioStandard->SetValue(nValue);
            }
            break;

        case IDM_SOURCE_INPUT1:
        case IDM_SOURCE_INPUT2:
        case IDM_SOURCE_INPUT3:
        case IDM_SOURCE_INPUT4:
        case IDM_SOURCE_INPUT5:
        case IDM_SOURCE_INPUT6:
        case IDM_SOURCE_INPUT7:
            {
                int nValue = LOWORD(wParam) - IDM_SOURCE_INPUT1;
                ShowText(hWnd, m_pSAA7134Card->GetInputName(nValue));
                m_VideoSource->SetValue(nValue);
                SendMessage(hWnd, WM_COMMAND, IDM_VT_RESET, 0);
            }
            break;
            
        case IDM_HWINFO:
            DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_HWINFO), hWnd, CSAA7134Card::ChipSettingProc, (LPARAM)m_pSAA7134Card);
            break;

        case IDM_AUDIOSETTINGS:
            break;

        case IDM_AUDIOSETTINGS1:
            break;

        case IDM_ADV_VIDEOSETTINGS:
//            DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_ADV_VIDEOSETTINGS), hWnd, AdvVideoSettingProc, (LPARAM)this);
            break;

        // Video format (NTSC, PAL, etc)
        case IDM_TYPEFORMAT_0:
            m_VideoFormat->SetValue(VIDEOFORMAT_PAL_B);
            ShowText(hWnd, GetStatus());
            break;
        case IDM_TYPEFORMAT_1:
            m_VideoFormat->SetValue(VIDEOFORMAT_NTSC_M);
            ShowText(hWnd, GetStatus());
            break;
        case IDM_TYPEFORMAT_2:
            m_VideoFormat->SetValue(VIDEOFORMAT_SECAM_B);
            ShowText(hWnd, GetStatus());
            break;
        case IDM_TYPEFORMAT_3:
            m_VideoFormat->SetValue(VIDEOFORMAT_PAL_M);
            ShowText(hWnd, GetStatus());
            break;
        case IDM_TYPEFORMAT_4:
            m_VideoFormat->SetValue(VIDEOFORMAT_PAL_N);
            ShowText(hWnd, GetStatus());
            break;
        case IDM_TYPEFORMAT_5:
            m_VideoFormat->SetValue(VIDEOFORMAT_NTSC_M_Japan);
            ShowText(hWnd, GetStatus());
            break;
        case IDM_TYPEFORMAT_6:
            m_VideoFormat->SetValue(VIDEOFORMAT_PAL_60);
            ShowText(hWnd, GetStatus());
            break;
        case IDM_TYPEFORMAT_7:
            m_VideoFormat->SetValue(VIDEOFORMAT_NTSC_50);
            ShowText(hWnd, GetStatus());
            break;
        case IDM_TYPEFORMAT_8:
            m_VideoFormat->SetValue(VIDEOFORMAT_PAL_N_COMBO);
            ShowText(hWnd, GetStatus());
            break;

        case IDM_SETTINGS_PIXELWIDTH_768:
            m_PixelWidth->SetValue(768);
            break;

        case IDM_SETTINGS_PIXELWIDTH_754:
            m_PixelWidth->SetValue(754);
            break;

        case IDM_SETTINGS_PIXELWIDTH_720:
            m_PixelWidth->SetValue(720);
            break;
    
        case IDM_SETTINGS_PIXELWIDTH_640:
            m_PixelWidth->SetValue(640);
            break;
    
        case IDM_SETTINGS_PIXELWIDTH_480:
            m_PixelWidth->SetValue(480);
            break;
    
        case IDM_SETTINGS_PIXELWIDTH_384:
            m_PixelWidth->SetValue(384);
            break;
    
        case IDM_SETTINGS_PIXELWIDTH_320:
            m_PixelWidth->SetValue(320);
            break;
    
        case IDM_SETTINGS_PIXELWIDTH_CUSTOM:
            m_PixelWidth->SetValue(m_CustomPixelWidth->GetValue());
            break;

        case IDM_SAVE_BY_FORMAT:
            SaveInputSettings(TRUE);
            m_bSavePerFormat->SetValue(!m_bSavePerFormat->GetValue());
            LoadInputSettings();
            break;

        case IDM_SAVE_BY_INPUT:
            SaveInputSettings(TRUE);
            m_bSavePerInput->SetValue(!m_bSavePerInput->GetValue());
            LoadInputSettings();
            break;

        case IDM_SAVE_BY_CHANNEL:
            // Gone           
            break;        
        default:
            return FALSE;
            break;
    }
    return TRUE;
}

void CSAA7134Source::ChangeSectionNamesForInput()
{
    int Input = -1;
    int Format = -1;

    if(m_bSavePerInput->GetValue())
    {
        Input = m_VideoSource->GetValue();
    }

    if(m_bSavePerFormat->GetValue())
    {
        Format = m_VideoFormat->GetValue();
    }

    if(Input == -1 && Format == -1)
    {
        m_Brightness->SetSection(m_Section.c_str());
        m_Contrast->SetSection(m_Section.c_str());
        m_Hue->SetSection(m_Section.c_str());
        m_Saturation->SetSection(m_Section.c_str());
//      m_SaturationU->SetSection(m_Section.c_str());
//      m_SaturationV->SetSection(m_Section.c_str());
        m_Overscan->SetSection(m_Section.c_str());
//      m_BDelay->SetSection(m_Section.c_str());
    }
    else
    {
        char szSection[128];
        sprintf(szSection, "%s_%d_%d", m_Section.c_str(), Input, Format);
        m_Brightness->SetSection(szSection);
        m_Contrast->SetSection(szSection);
        m_Hue->SetSection(szSection);
        m_Saturation->SetSection(szSection);
//      m_SaturationU->SetSection(szSection);
//      m_SaturationV->SetSection(szSection);
        m_Overscan->SetSection(szSection);
//      m_BDelay->SetSection(szSection);
    }

    ChangeDefaultsForInput();
}

void CSAA7134Source::ChangeDefaultsForInput()
{
    eVideoFormat format = GetFormat();
    if(IsNTSCVideoFormat(format))
    {
        m_Overscan->ChangeDefault(DEFAULT_OVERSCAN_NTSC);
    }
    else
    {
        m_Overscan->ChangeDefault(DEFAULT_OVERSCAN_PAL);
    }
    
/*    // set up defaults fro position parameters
    if(m_bSavePerFormat->GetValue())
    {
        m_BDelay->ChangeDefault(GetTVFormat((eVideoFormat)m_VideoFormat->GetValue())->bDelayB);
    }
    else
    {
        m_BDelay->ChangeDefault(0);
    }*/
}


void CSAA7134Source::LoadInputSettings()
{
    ChangeDefaultsForInput();
    
    if (!SettingsPerChannel())
    {
        ChangeSectionNamesForInput();
        m_Brightness->ReadFromIni();
        m_Contrast->ReadFromIni();
        m_Hue->ReadFromIni();
        m_Saturation->ReadFromIni();
//        m_SaturationU->ReadFromIni();
//        m_SaturationV->ReadFromIni();
        m_Overscan->ReadFromIni();
//        m_BDelay->ReadFromIni();
    }

    ChangeChannelSectionNames();
}

void CSAA7134Source::SaveInputSettings(BOOL bOptimizeFileAccess)
{
    if (!SettingsPerChannel())
    {        
        m_Brightness->WriteToIni(bOptimizeFileAccess);
        m_Contrast->WriteToIni(bOptimizeFileAccess);
        m_Hue->WriteToIni(bOptimizeFileAccess);
        m_Saturation->WriteToIni(bOptimizeFileAccess);
//        m_SaturationU->WriteToIni(bOptimizeFileAccess);
//        m_SaturationV->WriteToIni(bOptimizeFileAccess);
        m_Overscan->WriteToIni(bOptimizeFileAccess);
//        m_BDelay->WriteToIni(bOptimizeFileAccess);
    }
}


void CSAA7134Source::ChangeChannelSectionNames()
{    
/*    if (!m_SettingsByChannelStarted)
    {
        return;
    }

    std::string sOldSection = m_ChannelSubSection;
    
    int Input = -1;
    int Format = -1;
    
    if(m_bSavePerInput->GetValue())
    {
        Input = m_VideoSource->GetValue();
    }
    if(m_bSavePerFormat->GetValue())
    {
        Format = m_VideoFormat->GetValue();
    }

    if(Input != -1 || Format != -1)
    {
        char szSection[100];
        sprintf(szSection, "%s_%d_%d", m_Section.c_str(), Input, Format);
        m_ChannelSubSection = szSection;     
    } 
    else
    {
        m_ChannelSubSection = m_Section;
    }
    if (sOldSection != m_ChannelSubSection)
    {
        if (sOldSection.size() > 0)
        {            
            if (m_CurrentChannel >=0)
            {            
                SettingsPerChannel_SaveChannelSettings(sOldSection.c_str(), m_VideoSource->GetValue(), m_CurrentChannel);
            }        
            SettingsPerChannel_UnregisterSection(sOldSection.c_str());
        }
    
        SettingsPerChannel_RegisterSetSection(m_ChannelSubSection.c_str());
        SettingsPerChannel_RegisterSetting("Brightness","BT8x8 - Brightness",TRUE, m_Brightness);
        SettingsPerChannel_RegisterSetting("Hue","BT8x8 - Hue",TRUE, m_Hue);            
        SettingsPerChannel_RegisterSetting("Contrast","BT8x8 - Contrast",TRUE, m_Contrast);
        
        SettingsPerChannel_RegisterSetting("Saturation","BT8x8 - Saturation",TRUE, m_Saturation);
        SettingsPerChannel_RegisterSetting("Saturation","BT8x8 - Saturation",TRUE, m_SaturationU);
        SettingsPerChannel_RegisterSetting("Saturation","BT8x8 - Saturation",TRUE, m_SaturationV);
    
        SettingsPerChannel_RegisterSetting("Overscan","BT8x8 - Overscan",TRUE, m_Overscan);
        
        SettingsPerChannel_RegisterSetting("AudioChannel","BT8x8 - Audio Channel",TRUE, m_AudioChannel);
        SettingsPerChannel_RegisterSetting("AutoStereoSelect","BT8x8 - Auto Stereo Select",TRUE, m_AutoStereoSelect);
        
        SettingsPerChannel_RegisterSetting("Volume","BT8x8 - Volume",TRUE, m_Volume);            
        SettingsPerChannel_RegisterSetting("Balance","BT8x8 - Balance",TRUE, m_Balance);
        SettingsPerChannel_RegisterSetting("BassTreble","BT8x8 - Bass & Treble",TRUE);            
        SettingsPerChannel_RegisterSetting("BassTreble","BT8x8 - Bass & Treble",TRUE, m_Bass);            
        SettingsPerChannel_RegisterSetting("BassTreble","BT8x8 - Bass & Treble",TRUE, m_Treble);        
     
        SettingsPerChannel_RegisterSetting("SAA7134Delays","BT8x8 - H/V/B Delay",FALSE, m_HDelay);
        SettingsPerChannel_RegisterSetting("SAA7134Delays","BT8x8 - H/V/B Delay",FALSE, m_VDelay);
        SettingsPerChannel_RegisterSetting("SAA7134Delays","BT8x8 - H/V/B Delay",FALSE, m_BDelay);
    }*/
}
