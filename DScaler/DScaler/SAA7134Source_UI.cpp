/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Source_UI.cpp,v 1.12 2002-10-20 07:41:04 atnak Exp $
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
// $Log: not supported by cvs2svn $
// Revision 1.11  2002/10/16 11:38:46  atnak
// cleaned up audio standard stuff
//
// Revision 1.10  2002/10/08 20:35:39  atnak
// whitepeak, colorpeak, comb filter UI options
//
// Revision 1.9  2002/10/04 23:40:46  atnak
// proper support for audio channels mono,stereo,lang1,lang2 added
//
// Revision 1.8  2002/10/03 23:31:50  atnak
// Changes for HPLLMode menu and other various minor
//
// Revision 1.7  2002/09/29 13:56:30  adcockj
// Fixed some cursor hide problems
//
// Revision 1.6  2002/09/25 15:11:12  adcockj
// Preliminary code for format specific support for settings per channel
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

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "..\DScalerResDbg\SAA7134Res\resource.h"
#include "resource.h"
#include "SAA7134Source.h"
#include "SAA7134_Defines.h"
#include "DScaler.h"
#include "OutThreads.h"
#include "AspectRatio.h"
#include "DebugLog.h"
#include "SettingsPerChannel.h"
#include "Slider.h"

// This identifies the position of the audio standard
// sub menu in the saa7134 menu
#define SAA7134MENU_AUDIOSTANDARD_POS       4

// The ID used here has to be one not already used by
// the saa7134 menu and has to have sufficient IDs following
// for AUDIOSTANDARD_LASTONE IDs
#define SAA7134MENU_AUDIOSTANDARD_START     IDM_DSVIDEO_STANDARD_0


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern const char *TunerNames[TUNER_LASTONE];


// SAA7134: This file still needs loads of work


void CSAA7134Source::InitializeUI()
{
    MENUITEMINFO    MenuItemInfo;
    HMENU           hSubMenu;
    LPSTR           pMenuName;

    m_hSAA7134ResourceInst = LoadLibrary("SAA7134Res.dll");

    if (m_pSAA7134Card->GetDeviceId() != 0x7130)
    {
        hSubMenu = GetSubMenu(m_hMenu, 0);
        hSubMenu = GetSubMenu(hSubMenu, SAA7134MENU_AUDIOSTANDARD_POS);

        // Set up two separators with the Custom Settings ...
        // menu in between before listing the standards.
        MenuItemInfo.cbSize = sizeof (MenuItemInfo);
        MenuItemInfo.fMask = MIIM_TYPE;
        MenuItemInfo.fType = MFT_SEPARATOR;

        InsertMenuItem(hSubMenu, 0, TRUE, &MenuItemInfo);
        InsertMenuItem(hSubMenu, 0, TRUE, &MenuItemInfo);

        pMenuName = "Custom Settings ...";
        MenuItemInfo.fMask = MIIM_TYPE | MIIM_ID;
        MenuItemInfo.fType = MFT_STRING;
        MenuItemInfo.dwTypeData = pMenuName;
        MenuItemInfo.cch = strlen(pMenuName);
        MenuItemInfo.wID = SAA7134MENU_AUDIOSTANDARD_START + AUDIOSTANDARD_LASTONE;
        InsertMenuItem(hSubMenu, 1, TRUE, &MenuItemInfo);

        for (int i(0); i < AUDIOSTANDARD_LASTONE; i++)
        {
            pMenuName = (LPSTR) m_pSAA7134Card->GetAudioStandardName((eAudioStandard)i);

            MenuItemInfo.fMask = MIIM_TYPE | MIIM_ID;
            MenuItemInfo.fType = MFT_STRING;
            MenuItemInfo.dwTypeData = pMenuName;
            MenuItemInfo.cch = strlen(pMenuName);
            MenuItemInfo.wID = SAA7134MENU_AUDIOSTANDARD_START + i;

            InsertMenuItem(hSubMenu, i, TRUE, &MenuItemInfo);
        }
    }
}


void CSAA7134Source::CleanupUI()
{
    if (m_hSAA7134ResourceInst != NULL)
    {
        FreeLibrary(m_hSAA7134ResourceInst);
    }
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
        Button_Enable(GetDlgItem(hDlg, IDCANCEL), pThis->m_bSelectCardCancelButton);
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

BOOL APIENTRY CSAA7134Source::RegisterEditProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    int i;
    char buf[128];
    static CSAA7134Source* pThis;
    static DWORD dwAddress;
    static BOOL bDisableChanges = TRUE;
    static BOOL bUnsafeRead = TRUE;
    BYTE Data;
    WORD LastChanged = 0;

    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CSAA7134Source*)lParam;
        sprintf(buf, "Edit Register on %s chip", pThis->GetChipName());
        SetWindowText(hDlg, buf);
        SendMessage(GetDlgItem(hDlg, IDC_REGISTERSELECT), CB_RESETCONTENT, 0, 0);
        for(i = 0; i < 0x400; i++)
        {
            int nIndex;
            sprintf(buf, "0x%03X", i);
            nIndex = SendMessage(GetDlgItem(hDlg, IDC_REGISTERSELECT), CB_ADDSTRING, 0, (LONG)buf);
            SendMessage(GetDlgItem(hDlg, IDC_REGISTERSELECT), CB_SETITEMDATA, nIndex, i);
            if (i == 0)
            {   
                SendMessage(GetDlgItem(hDlg, IDC_REGISTERSELECT), CB_SETCURSEL, nIndex, i);
            }
        }
        SetFocus(hDlg);
        RegisterEditProc(hDlg, WM_COMMAND, MAKELONG(IDC_REGISTERSELECT, 0L), lParam);
        break;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_REFRESH:
            if (!bUnsafeRead)
            {
                Data = pThis->m_pSAA7134Card->DirectGetByte(dwAddress);
                sprintf(buf, "%d", Data);
                SetDlgItemText(hDlg, IDC_VALUE_DEC, buf);
                sprintf(buf, "0x%02X", Data);
                SetDlgItemText(hDlg, IDC_VALUE_HEX, buf);
                SetDlgItemText(hDlg, IDC_STATIC_BIT0, Data & (1<<0) ? "1" : "0");
                SetDlgItemText(hDlg, IDC_STATIC_BIT1, Data & (1<<1) ? "1" : "0");
                SetDlgItemText(hDlg, IDC_STATIC_BIT2, Data & (1<<2) ? "1" : "0");
                SetDlgItemText(hDlg, IDC_STATIC_BIT3, Data & (1<<3) ? "1" : "0");
                SetDlgItemText(hDlg, IDC_STATIC_BIT4, Data & (1<<4) ? "1" : "0");
                SetDlgItemText(hDlg, IDC_STATIC_BIT5, Data & (1<<5) ? "1" : "0");
                SetDlgItemText(hDlg, IDC_STATIC_BIT6, Data & (1<<6) ? "1" : "0");
                SetDlgItemText(hDlg, IDC_STATIC_BIT7, Data & (1<<7) ? "1" : "0");
                CheckDlgButton(hDlg, IDC_BIT0, Data & (1<<0));
                CheckDlgButton(hDlg, IDC_BIT1, Data & (1<<1));
                CheckDlgButton(hDlg, IDC_BIT2, Data & (1<<2));
                CheckDlgButton(hDlg, IDC_BIT3, Data & (1<<3));
                CheckDlgButton(hDlg, IDC_BIT4, Data & (1<<4));
                CheckDlgButton(hDlg, IDC_BIT5, Data & (1<<5));
                CheckDlgButton(hDlg, IDC_BIT6, Data & (1<<6));
                CheckDlgButton(hDlg, IDC_BIT7, Data & (1<<7));
            }
            break;

        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg, TRUE);
            break;

        case IDC_REGISTERSELECT:
            i = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_REGISTERSELECT));
            dwAddress = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_REGISTERSELECT), i);
            if (dwAddress >= 0x400)
            {
                bUnsafeRead = TRUE;
                bDisableChanges = TRUE;
            }
            else
            {
                bUnsafeRead = FALSE;
                bDisableChanges = FALSE;
            }
            RegisterEditProc(hDlg, WM_COMMAND, MAKELONG(IDC_REFRESH, 0L), lParam);
            break;

        case IDC_BIT7:
        case IDC_BIT6:
        case IDC_BIT5:
        case IDC_BIT4:
        case IDC_BIT3:
        case IDC_BIT2:
        case IDC_BIT1:
        case IDC_BIT0:
            if (!bDisableChanges)
            {
                i = LOWORD(wParam) - IDC_BIT0;
                pThis->m_pSAA7134Card->DirectSetBit(dwAddress, i,
                    (BST_CHECKED == IsDlgButtonChecked(hDlg, LOWORD(wParam))));
                RegisterEditProc(hDlg, WM_COMMAND, MAKELONG(IDC_REFRESH, 0L), lParam);
            }
            else
            {
                LastChanged = LOWORD(wParam);
            }
        default:
            break;
        }
        break;
    default:
        break;
    }
    return (FALSE);
}


BOOL APIENTRY CSAA7134Source::AudioStandardProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    static CSAA7134Source*      pThis;
    static BOOL                 bLinked = FALSE;
    static eAudioStandard       AudioStandard;
    static DWORD                ScreenCarrier1;
    static DWORD                ScreenCarrier2;
    static eAudioCarrierMode    ScreenCarrier1Mode;
    static eAudioCarrierMode    ScreenCarrier2Mode;
    static eAudioFMDeemphasis   ScreenCh1FMDeemph;
    static eAudioFMDeemphasis   ScreenCh2FMDeemph;
    static DWORD                CustomCarrier1;
    static DWORD                CustomCarrier2;
    static eAudioCarrierMode    CustomCarrier1Mode;
    static eAudioCarrierMode    CustomCarrier2Mode;
    static eAudioFMDeemphasis   CustomCh1FMDeemph;
    static eAudioFMDeemphasis   CustomCh2FMDeemph;
    static WORD                 EditCarrierIndex;
    static BOOL                 EditingCarrier1 = TRUE;
    static BOOL                 EditingCarrier2 = TRUE;

    BOOL    UpdateCustom = FALSE;
    LPSTR   pString;
    int     nIndex;
    int     Number;
    char    buf[50];
    int     i;

    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CSAA7134Source*)lParam;    

        // AUDIOSTANDARD_LASTONE is used as "Custom" only in
        // CSAA7134SourceUI.cpp.  Here and in menu callback procs.

        CustomCarrier1        = pThis->m_AudioStandardCarrier1->GetValue();
        CustomCarrier2        = pThis->m_AudioStandardCarrier2->GetValue();
        CustomCarrier1Mode    = (eAudioCarrierMode)pThis->m_AudioStandardCarrier1Mode->GetValue();
        CustomCarrier2Mode    = (eAudioCarrierMode)pThis->m_AudioStandardCarrier2Mode->GetValue();
        CustomCh1FMDeemph     = (eAudioFMDeemphasis)pThis->m_AudioStandardCh1FMDeemph->GetValue();
        CustomCh2FMDeemph     = (eAudioFMDeemphasis)pThis->m_AudioStandardCh2FMDeemph->GetValue();

        if (pThis->m_CustomAudioStandard->GetValue())
        {
            AudioStandard = AUDIOSTANDARD_LASTONE;
            ScreenCarrier1     = CustomCarrier1;
            ScreenCarrier2     = CustomCarrier2;
            ScreenCh1FMDeemph  = CustomCh1FMDeemph;
            ScreenCh2FMDeemph  = CustomCh2FMDeemph;
            ScreenCarrier1Mode = CustomCarrier1Mode;
            ScreenCarrier2Mode = CustomCarrier2Mode;
        }
        else
        {
            AudioStandard = (eAudioStandard)pThis->m_AudioStandard->GetValue();
            ScreenCarrier1     = m_AudioStandards[AudioStandard].Carrier1;
            ScreenCarrier2     = m_AudioStandards[AudioStandard].Carrier2;
            ScreenCh1FMDeemph  = m_AudioStandards[AudioStandard].Ch1FMDeemphasis;
            ScreenCh2FMDeemph  = m_AudioStandards[AudioStandard].Ch2FMDeemphasis;
            ScreenCarrier1Mode = m_AudioStandards[AudioStandard].Carrier1Mode;
            ScreenCarrier2Mode = m_AudioStandards[AudioStandard].Carrier2Mode;
        }

        if (ScreenCarrier2Mode == AUDIOCHANNELMODE_NONE ||
            ScreenCarrier2Mode == AUDIOCHANNELMODE_BTSC ||
            ScreenCarrier2Mode == AUDIOCHANNELMODE_EIAJ)
        {
            ScreenCarrier2Mode = AUDIOCHANNELMODE_FM;
        }

        // Audio standard list (Plus 1 for "Custom")
        for (i = 0; i < AUDIOSTANDARD_LASTONE+1; i++) 
        {
            if (i == AUDIOSTANDARD_LASTONE)
            {
                pString = "Custom";
            }
            else
            {
                pString = (LPSTR) pThis->m_pSAA7134Card->GetAudioStandardName((eAudioStandard)i);
            }
            nIndex = SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_LIST), LB_ADDSTRING, 0, (LPARAM) pString);
            SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_LIST), LB_SETITEMDATA, nIndex, (LPARAM) i);

            if (i == AudioStandard)
            {
                SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_LIST), LB_SETCURSEL, i, 0);
            }
        }

        // Major/minor carrier list
        Number = pThis->GetMaxAudioCarrierNames();
        for (i = 0; i < Number; i++) 
        {
            sprintf(buf, "%g", ((float)m_AudioCarrierList[i]*12.288)/(1<<24));
            nIndex = SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_ADDSTRING, 0, (LPARAM)buf);
            SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_SETITEMDATA, nIndex, (LPARAM)m_AudioCarrierList[i]);

            nIndex = SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_ADDSTRING, 0, (LPARAM)buf);
            SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_SETITEMDATA, nIndex, (LPARAM)m_AudioCarrierList[i]);

            if (m_AudioCarrierList[i] == ScreenCarrier1)
            {
                SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_SETCURSEL, i, 0);
                EditingCarrier1 = FALSE;
            }

            if (m_AudioCarrierList[i] == ScreenCarrier2)
            {
                SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_SETCURSEL, i, 0);
                EditingCarrier2 = FALSE;
            }
        }

        EditCarrierIndex = Number;

        if (EditingCarrier1)
        {
            sprintf(buf, "%g", ((float)ScreenCarrier1*12.288)/(1<<24));
            SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_ADDSTRING, 0, (LPARAM)buf);
            SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_SETITEMDATA, EditCarrierIndex, ScreenCarrier1); 
            SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_SETCURSEL, EditCarrierIndex, 0);
        }

        if (EditingCarrier2)
        {
            sprintf(buf, "%g", ((float)ScreenCarrier2*12.288)/(1<<24));
            SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_ADDSTRING, 0, (LPARAM)buf);
            SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_SETITEMDATA, EditCarrierIndex, ScreenCarrier2); 
            SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_SETCURSEL, EditCarrierIndex, 0);
        }

        // Major/minor FM De-emphasis list
        Number = pThis->GetMaxFMDeemphasisNames();
        for (i = 0; i < Number; i++) 
        {
            // Enter the FM de-emphasis combo box items
            nIndex = SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJOR_DEMPH), CB_ADDSTRING, 0, (LPARAM)m_AudioFMDeemphasisName[i]);
            SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJOR_DEMPH), CB_SETITEMDATA, nIndex, (LPARAM)i);

            nIndex = SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINOR_DEMPH), CB_ADDSTRING, 0, (LPARAM)m_AudioFMDeemphasisName[i]);
            SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINOR_DEMPH), CB_SETITEMDATA, nIndex, (LPARAM)i);
        }

        // Select the right item in the FM de-emphasis list
        SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJOR_DEMPH), CB_SETCURSEL, ScreenCh1FMDeemph, 0);
        SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINOR_DEMPH), CB_SETCURSEL, ScreenCh2FMDeemph, 0);

        // Check the carrier mode radio buttons
        CheckDlgButton(hDlg, IDC_AUDIOSTANDARD_MAJOR_AM, ScreenCarrier1Mode == AUDIOCHANNELMODE_AM);
        CheckDlgButton(hDlg, IDC_AUDIOSTANDARD_MAJOR_FM, ScreenCarrier1Mode == AUDIOCHANNELMODE_FM);

        CheckDlgButton(hDlg, IDC_AUDIOSTANDARD_MINOR_AM, ScreenCarrier2Mode == AUDIOCHANNELMODE_AM);
        CheckDlgButton(hDlg, IDC_AUDIOSTANDARD_MINOR_FM, ScreenCarrier2Mode == AUDIOCHANNELMODE_FM || ScreenCarrier2Mode == AUDIOCHANNELMODE_FM_KOREA);
        CheckDlgButton(hDlg, IDC_AUDIOSTANDARD_MINOR_NICAM, ScreenCarrier2Mode == AUDIOCHANNELMODE_NICAM);

        CheckDlgButton(hDlg, IDC_AUDIOSTANDARD_KOREAN, ScreenCarrier2Mode == AUDIOCHANNELMODE_FM_KOREA);
        EnableWindow(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_KOREAN), ScreenCarrier2Mode == AUDIOCHANNELMODE_FM || ScreenCarrier2Mode == AUDIOCHANNELMODE_FM_KOREA);

        CheckDlgButton(hDlg, IDC_LINKED, bLinked);

        // Manual adjust sliders
        Slider_ClearTicks(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJOR_ADJUST), TRUE);
        Slider_SetRangeMax(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJOR_ADJUST), AUDIO_CARRIER_10_7);
        Slider_SetRangeMin(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJOR_ADJUST), AUDIO_CARRIER_4_5);
        Slider_SetPageSize(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJOR_ADJUST), 1);
        Slider_SetLineSize(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJOR_ADJUST), 1);
        Slider_SetTic(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJOR_ADJUST), 0);
        Slider_SetPos(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJOR_ADJUST), ScreenCarrier1);

        Slider_ClearTicks(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINOR_ADJUST), TRUE);
        Slider_SetRangeMax(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINOR_ADJUST), AUDIO_CARRIER_10_7);
        Slider_SetRangeMin(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINOR_ADJUST), AUDIO_CARRIER_4_5);
        Slider_SetPageSize(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINOR_ADJUST), 1);
        Slider_SetLineSize(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINOR_ADJUST), 1);
        Slider_SetTic(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINOR_ADJUST), 0);
        Slider_SetPos(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINOR_ADJUST), ScreenCarrier2);

        AudioStandardProc(hDlg, WM_TIMER, IDC_AUDIOSTANDARD_MANUAL_DETECTNOW, 0);
        SetTimer(hDlg, IDC_AUDIOSTANDARD_MANUAL_DETECTNOW, 200, NULL);

        // Focus the audio standards list
        SetFocus(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_LIST));
        break;

	case WM_TIMER:
		if (wParam == IDC_AUDIOSTANDARD_MANUAL_DETECTNOW)
		{
            pThis->m_pSAA7134Card->GetAudioDecoderStatus(buf, 50);
            SetDlgItemText(hDlg, IDC_AUDIOSTANDARD_STATUS, buf);
        }
		break;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_AUDIOSTANDARD_LIST:
            if (HIWORD(wParam) == LBN_SELCHANGE)
            {
                int nItem = SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_LIST), LB_GETCURSEL, 0, 0); 
                AudioStandard = (eAudioStandard)SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_LIST), LB_GETITEMDATA, nItem, 0); 

                if (AudioStandard == AUDIOSTANDARD_LASTONE)
                {
                    ScreenCarrier1      = CustomCarrier1;
                    ScreenCarrier2      = CustomCarrier2;
                    ScreenCh1FMDeemph   = CustomCh1FMDeemph;
                    ScreenCh2FMDeemph   = CustomCh2FMDeemph;
                    ScreenCarrier1Mode  = CustomCarrier1Mode;
                    ScreenCarrier2Mode  = CustomCarrier2Mode;
                }
                else
                {
                    ScreenCarrier1      = m_AudioStandards[AudioStandard].Carrier1;
                    ScreenCarrier2      = m_AudioStandards[AudioStandard].Carrier2;
                    ScreenCarrier1Mode  = m_AudioStandards[AudioStandard].Carrier1Mode;
                    ScreenCarrier2Mode  = m_AudioStandards[AudioStandard].Carrier2Mode;
                    ScreenCh1FMDeemph   = m_AudioStandards[AudioStandard].Ch1FMDeemphasis;
                    ScreenCh2FMDeemph   = m_AudioStandards[AudioStandard].Ch2FMDeemphasis;
                }

                if (ScreenCarrier2Mode == AUDIOCHANNELMODE_NONE ||
                    ScreenCarrier2Mode == AUDIOCHANNELMODE_BTSC ||
                    ScreenCarrier2Mode == AUDIOCHANNELMODE_EIAJ)
                {
                    ScreenCarrier2Mode = AUDIOCHANNELMODE_FM;
                }

                Number = pThis->GetMaxAudioCarrierNames();
                for (i = 0; i < Number; i++) 
                {
                    if (m_AudioCarrierList[i] == ScreenCarrier1)
                    {
                        SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_SETCURSEL, i, 0);
                    }
                    if (m_AudioCarrierList[i] == ScreenCarrier2)
                    {
                        SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_SETCURSEL, i, 0);
                    }
                }

                if (EditingCarrier1)
                {
                    EditingCarrier1 = FALSE;
                    SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_DELETESTRING, EditCarrierIndex, 0);
                }
                if (EditingCarrier2)
                {
                    EditingCarrier2 = FALSE;
                    SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_DELETESTRING, EditCarrierIndex, 0);
                }

                Slider_SetPos(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJOR_ADJUST), ScreenCarrier1);
                Slider_SetPos(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINOR_ADJUST), ScreenCarrier2);

                SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJOR_DEMPH), CB_SETCURSEL, ScreenCh1FMDeemph, 0);
                SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINOR_DEMPH), CB_SETCURSEL, ScreenCh2FMDeemph, 0);

                CheckDlgButton(hDlg, IDC_AUDIOSTANDARD_MAJOR_AM, ScreenCarrier1Mode == AUDIOCHANNELMODE_AM);
                CheckDlgButton(hDlg, IDC_AUDIOSTANDARD_MAJOR_FM, ScreenCarrier1Mode == AUDIOCHANNELMODE_FM);

                CheckDlgButton(hDlg, IDC_AUDIOSTANDARD_MINOR_AM, ScreenCarrier2Mode == AUDIOCHANNELMODE_AM);
                CheckDlgButton(hDlg, IDC_AUDIOSTANDARD_MINOR_FM, ScreenCarrier2Mode == AUDIOCHANNELMODE_FM || ScreenCarrier2Mode == AUDIOCHANNELMODE_FM_KOREA);
                CheckDlgButton(hDlg, IDC_AUDIOSTANDARD_MINOR_NICAM, ScreenCarrier2Mode == AUDIOCHANNELMODE_NICAM);

                CheckDlgButton(hDlg, IDC_AUDIOSTANDARD_KOREAN, ScreenCarrier2Mode == AUDIOCHANNELMODE_FM_KOREA);
                EnableWindow(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_KOREAN), ScreenCarrier2Mode == AUDIOCHANNELMODE_FM || ScreenCarrier2Mode == AUDIOCHANNELMODE_FM_KOREA);

                pThis->m_pSAA7134Card->SetAudioCarrier1Freq(ScreenCarrier1);
                pThis->m_pSAA7134Card->SetAudioCarrier2Freq(ScreenCarrier2);
                pThis->m_pSAA7134Card->SetAudioCarrier1Mode(ScreenCarrier1Mode);
                pThis->m_pSAA7134Card->SetAudioCarrier2Mode(ScreenCarrier2Mode);
                pThis->m_pSAA7134Card->SetCh1FMDeemphasis(ScreenCh1FMDeemph);
                pThis->m_pSAA7134Card->SetCh2FMDeemphasis(ScreenCh2FMDeemph);
            }
            break;

        case IDC_AUDIOSTANDARD_MAJORCARRIER:
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                int nItem = SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_GETCURSEL, 0, 0); 
                ScreenCarrier1 = SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_GETITEMDATA, nItem, 0); 

                if (EditingCarrier1)
                {
                    EditingCarrier1 = FALSE;
                    SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_DELETESTRING, EditCarrierIndex, 0);
                }

                Slider_SetPos(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJOR_ADJUST), ScreenCarrier1);

                pThis->m_pSAA7134Card->SetAudioCarrier1Freq(ScreenCarrier1);
                UpdateCustom = TRUE;
            }
            break;
            
        case IDC_AUDIOSTANDARD_MINORCARRIER:
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                int nItem = SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_GETCURSEL, 0, 0); 
                ScreenCarrier2 = SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_GETITEMDATA, nItem, 0);

                if (EditingCarrier2)
                {
                    EditingCarrier2 = FALSE;
                    SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_DELETESTRING, EditCarrierIndex, 0);
                }

                Slider_SetPos(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINOR_ADJUST), ScreenCarrier2);

                pThis->m_pSAA7134Card->SetAudioCarrier2Freq(ScreenCarrier2);
                UpdateCustom = TRUE;
            }
            break;

        case IDC_AUDIOSTANDARD_MAJOR_DEMPH:
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                int nItem = SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJOR_DEMPH), CB_GETCURSEL, 0, 0); 
                ScreenCh1FMDeemph = (eAudioFMDeemphasis)SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJOR_DEMPH), CB_GETITEMDATA, nItem, 0); 
                pThis->m_pSAA7134Card->SetCh1FMDeemphasis(ScreenCh1FMDeemph);
                UpdateCustom = TRUE;
            }
            break;

        case IDC_AUDIOSTANDARD_MINOR_DEMPH:
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                int nItem = SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINOR_DEMPH), CB_GETCURSEL, 0, 0); 
                ScreenCh2FMDeemph = (eAudioFMDeemphasis)SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINOR_DEMPH), CB_GETITEMDATA, nItem, 0); 
                pThis->m_pSAA7134Card->SetCh2FMDeemphasis(ScreenCh2FMDeemph);
                UpdateCustom = TRUE;
            }
            break;

        case IDC_AUDIOSTANDARD_MAJOR_AM:
            ScreenCarrier1Mode = AUDIOCHANNELMODE_AM;
            pThis->m_pSAA7134Card->SetAudioCarrier1Mode(ScreenCarrier1Mode);
            UpdateCustom = TRUE;
            break;


        case IDC_AUDIOSTANDARD_MAJOR_FM:
            ScreenCarrier1Mode = AUDIOCHANNELMODE_FM;
            pThis->m_pSAA7134Card->SetAudioCarrier1Mode(ScreenCarrier1Mode);
            UpdateCustom = TRUE;
            break;

        case IDC_AUDIOSTANDARD_MINOR_AM:
            ScreenCarrier2Mode = AUDIOCHANNELMODE_AM;
            pThis->m_pSAA7134Card->SetAudioCarrier2Mode(ScreenCarrier2Mode);
            CheckDlgButton(hDlg, IDC_AUDIOSTANDARD_KOREAN, FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_KOREAN), FALSE);
            UpdateCustom = TRUE;
            break;

        case IDC_AUDIOSTANDARD_MINOR_FM:
            ScreenCarrier2Mode = AUDIOCHANNELMODE_FM;
            pThis->m_pSAA7134Card->SetAudioCarrier2Mode(ScreenCarrier2Mode);
            EnableWindow(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_KOREAN), TRUE);
            UpdateCustom = TRUE;
            break;

        case IDC_AUDIOSTANDARD_MINOR_NICAM:
            ScreenCarrier2Mode = AUDIOCHANNELMODE_NICAM;
            pThis->m_pSAA7134Card->SetAudioCarrier2Mode(ScreenCarrier2Mode);
            CheckDlgButton(hDlg, IDC_AUDIOSTANDARD_KOREAN, FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_KOREAN), FALSE);
            UpdateCustom = TRUE;
            break;

        case IDC_AUDIOSTANDARD_KOREAN:
            if (IsDlgButtonChecked(hDlg, LOWORD(wParam)) == BST_CHECKED)
            {
                ScreenCarrier2Mode = AUDIOCHANNELMODE_FM_KOREA;
            }
            else
            {
                ScreenCarrier2Mode = AUDIOCHANNELMODE_FM;
            }
            pThis->m_pSAA7134Card->SetAudioCarrier2Mode(ScreenCarrier2Mode);
            UpdateCustom = TRUE;
            break;

        case IDC_LINKED:
            bLinked = (IsDlgButtonChecked(hDlg, LOWORD(wParam)) == BST_CHECKED);
            if (bLinked)
            {
                ScreenCarrier2 = ScreenCarrier1;
                Slider_SetPos(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINOR_ADJUST), ScreenCarrier2);

                if (EditingCarrier2)
                {
                    SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_DELETESTRING, EditCarrierIndex, 0);
                }
                EditingCarrier2 = TRUE;

                sprintf(buf, "%g", ((float)ScreenCarrier2*12.288)/(1<<24));
                SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_ADDSTRING, 0, (LPARAM)buf);
                SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_SETITEMDATA, EditCarrierIndex, ScreenCarrier2); 
                SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_SETCURSEL, EditCarrierIndex, 0);
                pThis->m_pSAA7134Card->SetAudioCarrier2Freq(ScreenCarrier2);
            }
            break;
        
        case IDCANCEL:
            if (pThis->m_CustomAudioStandard->GetValue())
            {
                // OnChange will set everything back for us
                pThis->m_CustomAudioStandard->SetValue(TRUE);
            }
            else if (AudioStandard != pThis->m_AudioStandard->GetValue())
            {
                pThis->m_AudioStandard->SetValue(pThis->m_AudioStandard->GetValue());
            }
            KillTimer(hDlg, IDC_AUDIOSTANDARD_MANUAL_DETECTNOW);
            EndDialog(hDlg, TRUE);
            break;

        case IDOK:
            if (AudioStandard != AUDIOSTANDARD_LASTONE)
            {
                // No need to OnChange because this dialog sets all the
                // card registers
                pThis->m_AudioStandard->SetValue(AudioStandard, ONCHANGE_NONE);
            }
            else
            {
                pThis->m_AudioStandardCarrier1->SetValue(ScreenCarrier1);
                pThis->m_AudioStandardCarrier2->SetValue(ScreenCarrier2);
                pThis->m_AudioStandardCarrier1Mode->SetValue(ScreenCarrier1Mode);
                pThis->m_AudioStandardCarrier2Mode->SetValue(ScreenCarrier2Mode);
                pThis->m_AudioStandardCh1FMDeemph->SetValue(ScreenCh1FMDeemph);
                pThis->m_AudioStandardCh2FMDeemph->SetValue(ScreenCh2FMDeemph);
                pThis->m_CustomAudioStandard->SetValue(TRUE);
            }
            KillTimer(hDlg, IDC_AUDIOSTANDARD_MANUAL_DETECTNOW);
            EndDialog(hDlg, TRUE);
            break;

        default:
            break;
        }
        break;

    case WM_VSCROLL:
    case WM_HSCROLL:
        if((HWND)lParam == GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJOR_ADJUST))
        {
            DWORD Carrier = Slider_GetPos(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJOR_ADJUST));
            if (Carrier != ScreenCarrier1)
            {
                ScreenCarrier1 = Carrier;

                if (EditingCarrier1)
                {
                    SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_DELETESTRING, EditCarrierIndex, 0);
                }
                EditingCarrier1 = TRUE;

                sprintf(buf, "%g", ((float)ScreenCarrier1*12.288)/(1<<24));
                SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_ADDSTRING, 0, (LPARAM)buf);
                SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_SETITEMDATA, EditCarrierIndex, ScreenCarrier1); 
                SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MAJORCARRIER), CB_SETCURSEL, EditCarrierIndex, 0);
                pThis->m_pSAA7134Card->SetAudioCarrier1Freq(ScreenCarrier1);
                UpdateCustom = TRUE;

                if (bLinked)
                {
                    ScreenCarrier2 = ScreenCarrier1;
                    Slider_SetPos(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINOR_ADJUST), ScreenCarrier2);

                    if (EditingCarrier2)
                    {
                        SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_DELETESTRING, EditCarrierIndex, 0);
                    }
                    EditingCarrier2 = TRUE;

                    sprintf(buf, "%g", ((float)ScreenCarrier2*12.288)/(1<<24));
                    SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_ADDSTRING, 0, (LPARAM)buf);
                    SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_SETITEMDATA, EditCarrierIndex, ScreenCarrier2); 
                    SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_SETCURSEL, EditCarrierIndex, 0);
                    pThis->m_pSAA7134Card->SetAudioCarrier2Freq(ScreenCarrier2);
                }
            }
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINOR_ADJUST))
        {
            DWORD Carrier = Slider_GetPos(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINOR_ADJUST));
            if (Carrier != ScreenCarrier2)
            {
                ScreenCarrier2 = Carrier;

                if (EditingCarrier2)
                {
                    SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_DELETESTRING, EditCarrierIndex, 0);
                }
                EditingCarrier2 = TRUE;

                sprintf(buf, "%g", ((float)ScreenCarrier2*12.288)/(1<<24));
                SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_ADDSTRING, 0, (LPARAM)buf);
                SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_SETITEMDATA, EditCarrierIndex, ScreenCarrier2); 
                SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_MINORCARRIER), CB_SETCURSEL, EditCarrierIndex, 0);
                pThis->m_pSAA7134Card->SetAudioCarrier2Freq(ScreenCarrier2);
                UpdateCustom = TRUE;

                if (bLinked)
                {
                    bLinked = FALSE;
                    CheckDlgButton(hDlg, IDC_LINKED, FALSE);
                }
            }
        }
        break;

    default:
        break;
    }

    if (UpdateCustom)
    {
        CustomCarrier1      = ScreenCarrier1;
        CustomCarrier2      = ScreenCarrier2;
        CustomCh1FMDeemph   = ScreenCh1FMDeemph;
        CustomCh2FMDeemph   = ScreenCh2FMDeemph;
        CustomCarrier1Mode  = ScreenCarrier1Mode;
        CustomCarrier2Mode  = ScreenCarrier2Mode;

        AudioStandard = AUDIOSTANDARD_LASTONE;
        SendMessage(GetDlgItem(hDlg, IDC_AUDIOSTANDARD_LIST), LB_SETCURSEL, AudioStandard, 0);
    }

    return (FALSE);
}


BOOL APIENTRY CSAA7134Source::OtherEditProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    static CSAA7134Source* pThis;

    static char LeftGain = 0;
    static char RightGain = 0;
    static char NicamGain = 0;
    static BOOL bLinked = TRUE;

    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CSAA7134Source*)lParam;

        LeftGain = pThis->m_pSAA7134Card->GetAudioLeftVolume();
        RightGain = pThis->m_pSAA7134Card->GetAudioRightVolume();
        NicamGain = pThis->m_pSAA7134Card->GetAudioNicamVolume();

        Slider_ClearTicks(GetDlgItem(hDlg, IDC_LEFT_SLIDER), TRUE);
        Slider_SetRangeMax(GetDlgItem(hDlg, IDC_LEFT_SLIDER), 15);
        Slider_SetRangeMin(GetDlgItem(hDlg, IDC_LEFT_SLIDER), -15);
        Slider_SetPageSize(GetDlgItem(hDlg, IDC_LEFT_SLIDER), 1);
        Slider_SetLineSize(GetDlgItem(hDlg, IDC_LEFT_SLIDER), 1);
        Slider_SetTic(GetDlgItem(hDlg, IDC_LEFT_SLIDER), 0);
        Slider_SetPos(GetDlgItem(hDlg, IDC_LEFT_SLIDER), LeftGain);
        SetDlgItemInt(hDlg, IDC_LEFT_EDIT, LeftGain, TRUE);

        Slider_ClearTicks(GetDlgItem(hDlg, IDC_RIGHT_SLIDER), TRUE);
        Slider_SetRangeMax(GetDlgItem(hDlg, IDC_RIGHT_SLIDER), 15);
        Slider_SetRangeMin(GetDlgItem(hDlg, IDC_RIGHT_SLIDER), -15);
        Slider_SetPageSize(GetDlgItem(hDlg, IDC_RIGHT_SLIDER), 1);
        Slider_SetLineSize(GetDlgItem(hDlg, IDC_RIGHT_SLIDER), 1);
        Slider_SetTic(GetDlgItem(hDlg, IDC_RIGHT_SLIDER), 0);
        Slider_SetPos(GetDlgItem(hDlg, IDC_RIGHT_SLIDER), RightGain);
        SetDlgItemInt(hDlg, IDC_RIGHT_EDIT, RightGain, TRUE);

        Slider_ClearTicks(GetDlgItem(hDlg, IDC_NICAM_SLIDER), TRUE);
        Slider_SetRangeMax(GetDlgItem(hDlg, IDC_NICAM_SLIDER), 15);
        Slider_SetRangeMin(GetDlgItem(hDlg, IDC_NICAM_SLIDER), -15);
        Slider_SetPageSize(GetDlgItem(hDlg, IDC_NICAM_SLIDER), 1);
        Slider_SetLineSize(GetDlgItem(hDlg, IDC_NICAM_SLIDER), 1);
        Slider_SetTic(GetDlgItem(hDlg, IDC_NICAM_SLIDER), 0);
        Slider_SetPos(GetDlgItem(hDlg, IDC_NICAM_SLIDER), NicamGain);
        SetDlgItemInt(hDlg, IDC_NICAM_EDIT, NicamGain, TRUE);

        CheckDlgButton(hDlg, IDC_LINKED_CHECK, bLinked);

        SetFocus(hDlg);
        break;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
/*        case IDC_CARRIER1_AM:
            pThis->m_pSAA7134Card->SetAudioCarrier1Mode(AUDIOCHANNELMODE_AM);
            break;

        case IDC_CARRIER1_FM:
            pThis->m_pSAA7134Card->SetAudioCarrier1Mode(AUDIOCHANNELMODE_FM);
            break;

        case IDC_CARRIER1_NICAM:
            pThis->m_pSAA7134Card->SetAudioCarrier1Mode(AUDIOCHANNELMODE_NICAM);
            break;

        case IDC_CARRIER2_AM:
            pThis->m_pSAA7134Card->SetAudioCarrier2Mode(AUDIOCHANNELMODE_AM);
            break;

        case IDC_CARRIER2_FM:
            pThis->m_pSAA7134Card->SetAudioCarrier2Mode(AUDIOCHANNELMODE_FM);
            break;

        case IDC_CARRIER2_NICAM:
            pThis->m_pSAA7134Card->SetAudioCarrier2Mode(AUDIOCHANNELMODE_NICAM);
            break;
*/
        case IDC_LINKED_CHECK:
            bLinked = (BST_CHECKED == IsDlgButtonChecked(hDlg, LOWORD(wParam)));
            break;

        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg, TRUE);
            break;

        default:
            break;
        }
        break;

    case WM_VSCROLL:
    case WM_HSCROLL:
        if((HWND)lParam == GetDlgItem(hDlg, IDC_LEFT_SLIDER))
        {
            LeftGain = Slider_GetPos(GetDlgItem(hDlg, IDC_LEFT_SLIDER));
            SetDlgItemInt(hDlg, IDC_LEFT_EDIT, LeftGain, TRUE);
            pThis->m_pSAA7134Card->SetAudioLeftVolume(LeftGain);

            if (bLinked)
            {
                RightGain = LeftGain;
                Slider_SetPos(GetDlgItem(hDlg, IDC_RIGHT_SLIDER), RightGain);
                SetDlgItemInt(hDlg, IDC_RIGHT_EDIT, RightGain, TRUE);
                pThis->m_pSAA7134Card->SetAudioRightVolume(RightGain);
            }
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_RIGHT_SLIDER))
        {
            RightGain = Slider_GetPos(GetDlgItem(hDlg, IDC_RIGHT_SLIDER));
            SetDlgItemInt(hDlg, IDC_RIGHT_EDIT, RightGain, TRUE);
            pThis->m_pSAA7134Card->SetAudioRightVolume(RightGain);

            if (bLinked)
            {
                LeftGain = RightGain;
                Slider_SetPos(GetDlgItem(hDlg, IDC_LEFT_SLIDER), LeftGain);
                SetDlgItemInt(hDlg, IDC_LEFT_EDIT, LeftGain, TRUE);
                pThis->m_pSAA7134Card->SetAudioLeftVolume(LeftGain);
            }
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_NICAM_SLIDER))
        {
            NicamGain = Slider_GetPos(GetDlgItem(hDlg, IDC_NICAM_SLIDER));
            SetDlgItemInt(hDlg, IDC_NICAM_EDIT, NicamGain, TRUE);
            pThis->m_pSAA7134Card->SetAudioNicamVolume(NicamGain);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_BLUR_SLIDER))
        {
        }
        break;

    default:
        break;
    }
    return (FALSE);
}


/*
BOOL APIENTRY CSAA7134Source::OtherEditProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    static CSAA7134Source* pThis;

    static char LeftGain = 0;
    static char RightGain = 0;
    static char NicamGain = 0;
    static BOOL bLinked = TRUE;

    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CSAA7134Source*)lParam;

        LeftGain = pThis->m_pSAA7134Card->GetAudioLeftVolume();
        RightGain = pThis->m_pSAA7134Card->GetAudioRightVolume();
        NicamGain = pThis->m_pSAA7134Card->GetAudioNicamVolume();

        Slider_ClearTicks(GetDlgItem(hDlg, IDC_LEFT_SLIDER), TRUE);
        Slider_SetRangeMax(GetDlgItem(hDlg, IDC_LEFT_SLIDER), 15);
        Slider_SetRangeMin(GetDlgItem(hDlg, IDC_LEFT_SLIDER), -16);
        Slider_SetPageSize(GetDlgItem(hDlg, IDC_LEFT_SLIDER), 1);
        Slider_SetLineSize(GetDlgItem(hDlg, IDC_LEFT_SLIDER), 1);
        Slider_SetTic(GetDlgItem(hDlg, IDC_LEFT_SLIDER), 0);
        Slider_SetPos(GetDlgItem(hDlg, IDC_LEFT_SLIDER), LeftGain);
        SetDlgItemInt(hDlg, IDC_LEFT_EDIT, LeftGain, TRUE);

        Slider_ClearTicks(GetDlgItem(hDlg, IDC_RIGHT_SLIDER), TRUE);
        Slider_SetRangeMax(GetDlgItem(hDlg, IDC_RIGHT_SLIDER), 15);
        Slider_SetRangeMin(GetDlgItem(hDlg, IDC_RIGHT_SLIDER), -16);
        Slider_SetPageSize(GetDlgItem(hDlg, IDC_RIGHT_SLIDER), 1);
        Slider_SetLineSize(GetDlgItem(hDlg, IDC_RIGHT_SLIDER), 1);
        Slider_SetTic(GetDlgItem(hDlg, IDC_RIGHT_SLIDER), 0);
        Slider_SetPos(GetDlgItem(hDlg, IDC_RIGHT_SLIDER), RightGain);
        SetDlgItemInt(hDlg, IDC_RIGHT_EDIT, RightGain, TRUE);

        Slider_ClearTicks(GetDlgItem(hDlg, IDC_NICAM_SLIDER), TRUE);
        Slider_SetRangeMax(GetDlgItem(hDlg, IDC_NICAM_SLIDER), 15);
        Slider_SetRangeMin(GetDlgItem(hDlg, IDC_NICAM_SLIDER), -16);
        Slider_SetPageSize(GetDlgItem(hDlg, IDC_NICAM_SLIDER), 1);
        Slider_SetLineSize(GetDlgItem(hDlg, IDC_NICAM_SLIDER), 1);
        Slider_SetTic(GetDlgItem(hDlg, IDC_NICAM_SLIDER), 0);
        Slider_SetPos(GetDlgItem(hDlg, IDC_NICAM_SLIDER), NicamGain);
        SetDlgItemInt(hDlg, IDC_NICAM_EDIT, NicamGain, TRUE);

        CheckDlgButton(hDlg, IDC_LINKED_CHECK, bLinked);

        SetFocus(hDlg);
        break;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_LINKED_CHECK:
            bLinked = (BST_CHECKED == IsDlgButtonChecked(hDlg, LOWORD(wParam)));
            break;

        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg, TRUE);
            break;

        default:
            break;
        }
        break;

    case WM_VSCROLL:
    case WM_HSCROLL:
        if((HWND)lParam == GetDlgItem(hDlg, IDC_LEFT_SLIDER))
        {
            LeftGain = Slider_GetPos(GetDlgItem(hDlg, IDC_LEFT_SLIDER));
            SetDlgItemInt(hDlg, IDC_LEFT_EDIT, LeftGain, TRUE);
            pThis->m_pSAA7134Card->SetAudioLeftVolume(LeftGain);

            if (bLinked)
            {
                RightGain = LeftGain;
                Slider_SetPos(GetDlgItem(hDlg, IDC_RIGHT_SLIDER), RightGain);
                SetDlgItemInt(hDlg, IDC_RIGHT_EDIT, RightGain, TRUE);
                pThis->m_pSAA7134Card->SetAudioRightVolume(RightGain);
            }
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_RIGHT_SLIDER))
        {
            RightGain = Slider_GetPos(GetDlgItem(hDlg, IDC_RIGHT_SLIDER));
            SetDlgItemInt(hDlg, IDC_RIGHT_EDIT, RightGain, TRUE);
            pThis->m_pSAA7134Card->SetAudioRightVolume(RightGain);

            if (bLinked)
            {
                LeftGain = RightGain;
                Slider_SetPos(GetDlgItem(hDlg, IDC_LEFT_SLIDER), LeftGain);
                SetDlgItemInt(hDlg, IDC_LEFT_EDIT, LeftGain, TRUE);
                pThis->m_pSAA7134Card->SetAudioLeftVolume(LeftGain);
            }
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_NICAM_SLIDER))
        {
            NicamGain = Slider_GetPos(GetDlgItem(hDlg, IDC_NICAM_SLIDER));
            SetDlgItemInt(hDlg, IDC_NICAM_EDIT, NicamGain, TRUE);
            pThis->m_pSAA7134Card->SetAudioNicamVolume(NicamGain);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_BLUR_SLIDER))
        {
        }
        break;

    default:
        break;
    }
    return (FALSE);
}
*/

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

    EnableMenuItemBool(m_hMenu, IDM_AUDIO_0, m_pSAA7134Card->GetDeviceId() != 0x7130);

    CheckMenuItemBool(m_hMenu, IDM_AUDIO_0, (GetCurrentAudioSetting()->GetValue() == 0));
    CheckMenuItemBool(m_hMenu, IDM_AUDIO_1, (GetCurrentAudioSetting()->GetValue() == 1));
    CheckMenuItemBool(m_hMenu, IDM_AUDIO_2, (GetCurrentAudioSetting()->GetValue() == 2));

    BOOL bDACActive = GetCurrentAudioSetting()->GetValue() == AUDIOINPUTSOURCE_DAC;

    // Plus 1 for the "Custom Settings ..." menu item
    for (i = 0; i < AUDIOSTANDARD_LASTONE+1; i++)
    {
        EnableMenuItemBool(m_hMenu, SAA7134MENU_AUDIOSTANDARD_START+i, bDACActive);
        CheckMenuItemBool(m_hMenu, SAA7134MENU_AUDIOSTANDARD_START+i, bDACActive &&
            (!m_CustomAudioStandard->GetValue()) && (m_AudioStandard->GetValue() == i));
    }

    if (m_CustomAudioStandard->GetValue())
    {
        CheckMenuItemBool(m_hMenu, SAA7134MENU_AUDIOSTANDARD_START+AUDIOSTANDARD_LASTONE, bDACActive && TRUE);
    }

    EnableMenuItemBool(m_hMenu, IDM_AUDIOSTANDARD_32000, !bDACActive);
    EnableMenuItemBool(m_hMenu, IDM_AUDIOSTANDARD_44100, !bDACActive);
    EnableMenuItemBool(m_hMenu, IDM_AUDIOSTANDARD_48000, !bDACActive);

    CheckMenuItemBool(m_hMenu, IDM_AUDIOSTANDARD_32000, bDACActive || m_AudioSampleRate->GetValue() == AUDIOSAMPLERATE_32000Hz);
    CheckMenuItemBool(m_hMenu, IDM_AUDIOSTANDARD_44100, !bDACActive && m_AudioSampleRate->GetValue() == AUDIOSAMPLERATE_44100Hz);
    CheckMenuItemBool(m_hMenu, IDM_AUDIOSTANDARD_48000, !bDACActive && m_AudioSampleRate->GetValue() == AUDIOSAMPLERATE_48000Hz);

    EnableMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_MONO, m_pSAA7134Card->IsAudioChannelDetected(AUDIOCHANNEL_MONO));
    EnableMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_STEREO, m_pSAA7134Card->IsAudioChannelDetected(AUDIOCHANNEL_STEREO));
    EnableMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_LANGUAGE1, m_pSAA7134Card->IsAudioChannelDetected(AUDIOCHANNEL_LANGUAGE1));
    EnableMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_LANGUAGE2, m_pSAA7134Card->IsAudioChannelDetected(AUDIOCHANNEL_LANGUAGE2));

    CheckMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_MONO, m_pSAA7134Card->GetAudioChannel() == AUDIOCHANNEL_MONO);
    CheckMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_STEREO, m_pSAA7134Card->GetAudioChannel() == AUDIOCHANNEL_STEREO);
    CheckMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_LANGUAGE1, m_pSAA7134Card->GetAudioChannel() == AUDIOCHANNEL_LANGUAGE1);
    CheckMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_LANGUAGE2, m_pSAA7134Card->GetAudioChannel() == AUDIOCHANNEL_LANGUAGE2);

    if (bDACActive)
    {
        EnableMenuItemBool(m_hMenu, IDM_AUTOSTEREO, TRUE);
        CheckMenuItemBool(m_hMenu, IDM_AUTOSTEREO, m_AutoStereoSelect->GetValue());
    }
    else
    {
        EnableMenuItemBool(m_hMenu, IDM_AUTOSTEREO, FALSE);
        CheckMenuItemBool(m_hMenu, IDM_AUTOSTEREO, FALSE);
    }
    
    CheckMenuItemBool(m_hMenu, IDM_SAA7134_HPLLMODE0, m_HPLLMode->GetValue() == 0);
    CheckMenuItemBool(m_hMenu, IDM_SAA7134_HPLLMODE1, m_HPLLMode->GetValue() == 1);
    CheckMenuItemBool(m_hMenu, IDM_SAA7134_HPLLMODE2, m_HPLLMode->GetValue() == 2);

    CheckMenuItemBool(m_hMenu, IDM_SAA7134_WHITEPEAK, m_WhitePeak->GetValue());
    CheckMenuItemBool(m_hMenu, IDM_SAA7134_COLORPEAK, m_ColorPeak->GetValue());
    CheckMenuItemBool(m_hMenu, IDM_SAA7134_COMBFILTER, m_AdaptiveCombFilter->GetValue());

    CheckMenuItemBool(m_hMenu, IDM_SAVE_BY_FORMAT, m_bSavePerFormat->GetValue());
    CheckMenuItemBool(m_hMenu, IDM_SAVE_BY_INPUT, m_bSavePerInput->GetValue());
}


BOOL CSAA7134Source::HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
{
    if (LOWORD(wParam) >= SAA7134MENU_AUDIOSTANDARD_START &&
        LOWORD(wParam) < SAA7134MENU_AUDIOSTANDARD_START + AUDIOSTANDARD_LASTONE)
    {
        // dynamic audio standards menu
        int nValue = LOWORD(wParam) - SAA7134MENU_AUDIOSTANDARD_START;
        ShowText(hWnd, m_pSAA7134Card->GetAudioStandardName((eAudioStandard)nValue));
        m_AudioStandard->SetValue(nValue);
        return TRUE;
    }
    else if (LOWORD(wParam) == SAA7134MENU_AUDIOSTANDARD_START + AUDIOSTANDARD_LASTONE)
    {
        // "Custom Settings ..." menu
        if (m_hSAA7134ResourceInst != NULL)
        {
            PreShowDialogOrMenu();
            DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_AUDIOSTANDARD_CUSTOM), hWnd, AudioStandardProc, (LPARAM)this);
            PostShowDialogOrMenu();
        }
        else
        {
            ShowText(hWnd, "SAA7134Res.dll not loaded");
        }
    }

    switch(LOWORD(wParam))
    {
        case IDM_SETUPCARD:
            Stop_Capture();
            {
                long OrigCard = m_CardType->GetValue();
                long OrigTuner = m_TunerType->GetValue();
                PreShowDialogOrMenu();
                DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_SELECTCARD), hWnd, (DLGPROC) SelectCardProc, (LPARAM)this);
                PostShowDialogOrMenu();
                if (m_CardType->GetValue() != OrigCard)
                {
                    m_pSAA7134Card->SetCardType(m_CardType->GetValue());
                    m_pSAA7134Card->InitAudio();
                    ChangeTVSettingsBasedOnCard();
                }
                if (m_TunerType->GetValue() != OrigTuner)
                {
                    m_pSAA7134Card->InitTuner((eTunerId)m_TunerType->GetValue());
                }   
            }
            Start_Capture();
            break;

        case IDM_BDELAY_PLUS:
            ShowText(hWnd, "BDelay Plus - Unsupported");
            break;

        case IDM_BDELAY_MINUS:
            ShowText(hWnd, "BDelay Minus - Unsupported");
            break;

        case IDM_BDELAY_CURRENT:
            ShowText(hWnd, "BDelay Current - Unsupported");
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
            m_AudioChannel->SetValue(AUDIOCHANNEL_MONO);
            break;
        case IDM_SOUNDCHANNEL_STEREO:
            m_AudioChannel->SetValue(AUDIOCHANNEL_STEREO);
            break;
        case IDM_SOUNDCHANNEL_LANGUAGE1:
            m_AudioChannel->SetValue(AUDIOCHANNEL_LANGUAGE1);
            break;
        case IDM_SOUNDCHANNEL_LANGUAGE2:
            m_AudioChannel->SetValue(AUDIOCHANNEL_LANGUAGE2);
            break;

        case IDM_AUTOSTEREO:
            m_AutoStereoSelect->SetValue(!m_AutoStereoSelect->GetValue());            
            break;

        case IDM_SAA7134_HPLLMODE0:
            ShowText(hWnd, "TV Mode");
            m_HPLLMode->SetValue(0);
            break;

        case IDM_SAA7134_HPLLMODE1:
            ShowText(hWnd, "VCR Mode");
            m_HPLLMode->SetValue(1);
            break;

        case IDM_SAA7134_HPLLMODE2:
            ShowText(hWnd, "Fast Tracking");
            m_HPLLMode->SetValue(2);
            break;

        case IDM_SAA7134_WHITEPEAK:
            m_WhitePeak->SetValue(!m_WhitePeak->GetValue());
            break;

        case IDM_SAA7134_COLORPEAK:
            m_ColorPeak->SetValue(!m_ColorPeak->GetValue());
            break;

        case IDM_SAA7134_COMBFILTER:
            m_AdaptiveCombFilter->SetValue(!m_AdaptiveCombFilter->GetValue());
            break;

        case IDM_AUDIO_0:
        case IDM_AUDIO_1:
        case IDM_AUDIO_2:
            GetCurrentAudioSetting()->SetValue((LOWORD(wParam) - IDM_AUDIO_0));
            switch (GetCurrentAudioSetting()->GetValue())
            {
            case AUDIOINPUTSOURCE_DAC:     
                ShowText(hWnd, "Audio Input - Tuner");
                break;
            case AUDIOINPUTSOURCE_LINE1: 
                ShowText(hWnd, "Audio Input - Line 1");
                break;
            case AUDIOINPUTSOURCE_LINE2:  
                ShowText(hWnd, "Audio Input - Line 2");
                break;
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

        case IDM_AUDIOSTANDARD_32000:
        case IDM_AUDIOSTANDARD_44100:
        case IDM_AUDIOSTANDARD_48000:
            {
                int nValue = LOWORD(wParam) - IDM_AUDIOSTANDARD_32000;
                m_AudioSampleRate->SetValue(nValue);
            }
            break;

        case IDM_HWINFO:
            PreShowDialogOrMenu();
            DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_HWINFO), hWnd, CSAA7134Card::ChipSettingProc, (LPARAM)m_pSAA7134Card);
            PostShowDialogOrMenu();
            break;

        case IDM_ADV_VIDEOSETTINGS:
            if (m_hSAA7134ResourceInst != NULL)
            {
                PreShowDialogOrMenu();
                DialogBoxParam(m_hSAA7134ResourceInst, "REGISTEREDIT", hWnd, RegisterEditProc, (LPARAM)this);
                PostShowDialogOrMenu();
            }
            else
            {
                ShowText(hWnd, "SAA7134Res.dll not loaded");
            }
            break;

        case IDM_AUDIOSETTINGS:
            if (m_hSAA7134ResourceInst != NULL)
            {
                PreShowDialogOrMenu();
                DialogBoxParam(m_hSAA7134ResourceInst, "OTHEREDIT", hWnd, OtherEditProc, (LPARAM)this);
                PostShowDialogOrMenu();
            }
            else
            {
                ShowText(hWnd, "SAA7134Res.dll not loaded");
            }
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
        m_Overscan->SetSection(m_Section.c_str());
    }
    else
    {
        char szSection[128];
        sprintf(szSection, "%s_%d_%d", m_Section.c_str(), Input, Format);
        m_Brightness->SetSection(szSection);
        m_Contrast->SetSection(szSection);
        m_Hue->SetSection(szSection);
        m_Saturation->SetSection(szSection);
        m_Overscan->SetSection(szSection);
    }

    ChangeDefaultsForInput();
}

void CSAA7134Source::ChangeDefaultsForInput()
{
    eVideoFormat format = GetFormat();
    if(IsPALVideoFormat(format))
    {
        m_Saturation->ChangeDefault(SAA7134_DEFAULT_PAL_SATURATION);
        m_Overscan->ChangeDefault(SAA7134_DEFAULT_PAL_OVERSCAN);
    }
    else if(IsNTSCVideoFormat(format))
    {
        m_Saturation->ChangeDefault(SAA7134_DEFAULT_NTSC_SATURATION);
        m_Overscan->ChangeDefault(SAA7134_DEFAULT_NTSC_OVERSCAN);
    }
    else
    {
        m_Saturation->ChangeDefault(SAA7134_DEFAULT_SATURATION);
        m_Overscan->ChangeDefault(SAA7134_DEFAULT_OVERSCAN);
    }
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
        m_Overscan->ReadFromIni();
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
        m_Overscan->WriteToIni(bOptimizeFileAccess);
    }
}


void CSAA7134Source::ChangeChannelSectionNames()
{    
      if (!m_SettingsByChannelStarted)
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
                SettingsPerChannel_SaveChannelSettings(sOldSection.c_str(), m_VideoSource->GetValue(), m_CurrentChannel, GetFormat());
            }        
            SettingsPerChannel_UnregisterSection(sOldSection.c_str());
        }
    
        SettingsPerChannel_RegisterSetSection(m_ChannelSubSection.c_str());
        SettingsPerChannel_RegisterSetting("Brightness", "SAA713x - Brightness",TRUE, m_Brightness);
        SettingsPerChannel_RegisterSetting("Hue", "SAA713x - Hue", TRUE, m_Hue);            
        SettingsPerChannel_RegisterSetting("Contrast", "SAA713x - Contrast", TRUE, m_Contrast);
        SettingsPerChannel_RegisterSetting("Saturation","SAA713x - Saturation",TRUE, m_Saturation);
        SettingsPerChannel_RegisterSetting("Overscan", "SAA713x - Overscan", TRUE, m_Overscan);
        
        SettingsPerChannel_RegisterSetting("AudioChannel", "SAA713x - Audio Channel", TRUE, m_AudioChannel);
        SettingsPerChannel_RegisterSetting("HPLLMode", "SAA713x - Miscellaneous", TRUE, m_HPLLMode);
        
        // SettingsPerChannel_RegisterSetting("Volume","SAA713x - Volume",TRUE, m_Volume);           
        // SettingsPerChannel_RegisterSetting("Balance","SAA713x - Balance",TRUE, m_Balance);
     
        SettingsPerChannel_RegisterSetting("Delays", "SAA713x - H/V Delay", FALSE, m_HDelay);
        SettingsPerChannel_RegisterSetting("Delays", "SAA713x - H/V Delay", FALSE, m_VDelay);
    }
}
