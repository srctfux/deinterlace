/////////////////////////////////////////////////////////////////////////////
// $Id: CX2388xSource_UI.cpp,v 1.43 2004-03-07 12:20:12 to_see Exp $
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
// Revision 1.42  2004/02/29 19:41:45  to_see
// new Submenu's in CX Card for Audio Channel and Audio Standard
// new AutoMute entry
//
// Revision 1.41  2004/02/27 20:51:00  to_see
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
// Revision 1.40  2004/02/05 21:47:52  to_see
// Starting/Stopping connexant-drivers while dscaler is running.
// To Enable/Disable it, go to Settings->Advanced Settings->
// CX2388X Advanced->Stopping Conexxant driver while Dscaler is running.
//
// This enables sound on my card without to go to windows control panel.
//
// Revision 1.39  2003/10/27 10:39:51  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.38  2003/07/18 09:41:23  adcockj
// Added PDI input to holo3d (doesn't yet work)
//
// Revision 1.37  2003/06/01 14:48:33  adcockj
// Fixed possible bug spotted by Laurent
//
// Revision 1.36  2003/03/23 10:42:21  laurentg
// Avoid to switch to an unknown video input when using 000...
//
// Revision 1.35  2003/03/23 10:25:23  laurentg
// Use video input name as icon tips when not in tuner mode
//
// Revision 1.34  2003/01/27 22:04:09  laurentg
// First step to merge setup hardware and hardware info dialog boxes
// CPU flag information moved in the general hardware dialog box
// Hardware info dialog box available for CX2388x
//
// Revision 1.33  2003/01/25 23:43:15  laurentg
// Default video settings for SECAM
//
// Revision 1.32  2003/01/21 14:42:14  adcockj
// Changed PAL defaults and added place for SECAM defaults
//
// Revision 1.31  2003/01/19 10:39:56  laurentg
// Disable selection of the tuner input when no tuner has been selected in the card setup dialog box
//
// Revision 1.30  2003/01/18 12:10:47  laurentg
// Avoid double display in OSD (ADJUSTDOWN_SILENT and ADJUSTUP_SILENT instead of (ADJUSTDOWN and ADJUSTUP)
//
// Revision 1.29  2003/01/16 14:21:49  adcockj
// Added analogue blanking to advanced settings
//
// Revision 1.28  2003/01/16 13:30:49  adcockj
// Fixes for various settings problems reported by Laurent 15/Jan/2003
//
// Revision 1.27  2003/01/15 15:54:22  adcockj
// Fixed some keyboard focus issues
//
// Revision 1.26  2003/01/10 17:37:55  adcockj
// Interrim Check in of Settings rewrite
//  - Removed SETTINGSEX structures and flags
//  - Removed Seperate settings per channel code
//  - Removed Settings flags
//  - Cut away some unused features
//
// Revision 1.25  2003/01/07 23:27:02  laurentg
// New overscan settings
//
// Revision 1.24  2003/01/04 16:54:39  adcockj
// Disabled format menu when in tuner mode
//
// Revision 1.23  2002/12/31 13:21:22  adcockj
// Fixes for SetDefault Problems (needs testing)
//
// Revision 1.22  2002/12/23 17:22:10  adcockj
// Settings fixes
//
// Revision 1.21  2002/12/10 14:53:17  adcockj
// Sound fixes for cx2388x
//
// Revision 1.20  2002/12/04 16:18:25  adcockj
// Added White crush to settings dialog
//
// Revision 1.19  2002/12/04 15:54:09  adcockj
// Hacky fix for settings by channel code with mutiple cards
//
// Revision 1.18  2002/12/04 15:20:08  adcockj
// Fixed accedental test code check in
//
// Revision 1.17  2002/12/04 15:15:24  adcockj
// Checked in test code by accident
//
// Revision 1.16  2002/12/03 16:34:13  adcockj
// Corrected channel settings groupings
//
// Revision 1.15  2002/12/03 13:28:23  adcockj
// Corrected per channel settings code
//
// Revision 1.14  2002/12/03 07:56:31  adcockj
// Fixed some problems with settings not saving
//
// Revision 1.13  2002/11/28 18:06:32  adcockj
// Changed text for progressive mode
//
// Revision 1.12  2002/11/13 10:34:36  adcockj
// Improved pixel width support
//
// Revision 1.11  2002/11/12 15:22:50  adcockj
// Made new flag settings have default setting
// Added pixel width for CX2388x cards
//
// Revision 1.10  2002/11/12 11:33:07  adcockj
// Fixed OSD
//
// Revision 1.9  2002/11/09 20:53:46  laurentg
// New CX2388x settings
//
// Revision 1.8  2002/11/09 00:22:23  laurentg
// New settings for CX2388x chip
//
// Revision 1.7  2002/11/08 10:37:46  adcockj
// Added UI for Holo3d Settings
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
// Revision 1.3  2002/10/31 03:10:55  atnak
// Changed CSource::GetTreeSettingsPage to return CTreeSettingsPage*
//
// Revision 1.2  2002/10/29 22:00:30  adcockj
// Added EatlLinesAtTop setting for SDI on holo3d
//
// Revision 1.1  2002/10/29 11:05:28  adcockj
// Renamed CT2388x to CX2388x
//
// 
// CVS Log while file was called CT2388xSource_UI.cpp
//
// Revision 1.8  2002/10/26 17:51:52  adcockj
// Simplified hide cusror code and removed PreShowDialogOrMenu & PostShowDialogOrMenu
//
// Revision 1.7  2002/10/21 19:08:09  adcockj
// Added support for keyboard h/v delay
//
// Revision 1.6  2002/10/21 07:19:33  adcockj
// Preliminary Support for PixelView XCapture
//
// Revision 1.5  2002/09/29 16:16:21  adcockj
// Holo3d imrprovements
//
// Revision 1.4  2002/09/29 13:56:30  adcockj
// Fixed some cursor hide problems
//
// Revision 1.3  2002/09/25 15:11:12  adcockj
// Preliminary code for format specific support for settings per channel
//
// Revision 1.2  2002/09/22 17:47:04  adcockj
// Fixes for holo3d
//
// Revision 1.1  2002/09/11 18:19:38  adcockj
// Prelimainary support for CX2388x based cards
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file CX2388xSource.cpp CCX2388xSource Implementation (UI)
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "CX2388xSource.h"
#include "CX2388x_Defines.h"
#include "DScaler.h"
#include "Providers.h"
#include "OutThreads.h"
#include "AspectRatio.h"
#include "DebugLog.h"
#include "SettingsPerChannel.h"

extern const char *TunerNames[TUNER_LASTONE];
extern long EnableCancelButton;

BOOL APIENTRY CCX2388xSource::SelectCardProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    int i;
    int nIndex;
    char buf[128];
    static CCX2388xSource* pThis;
    CCX2388xCard* pCard = NULL;
    char szCardId[9] = "n/a     ";
    char szVendorId[9] = "n/a ";
    char szDeviceId[9] = "n/a ";
    DWORD dwCardId(0);

    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CCX2388xSource*)lParam;
        sprintf(buf, "Setup card %s", pThis->IDString());
        SetWindowText(hDlg, buf);
        Button_Enable(GetDlgItem(hDlg, IDCANCEL), EnableCancelButton);
        SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_RESETCONTENT, 0, 0);
        for(i = 0; i < CX2388xCARD_LASTONE; i++)
        {
            int nIndex;
            nIndex = SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_ADDSTRING, 0, (LONG)pThis->m_pCard->GetCardName((eCX2388xCardId)i));
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

		pCard = pThis->GetCard();
        SetDlgItemText(hDlg, IDC_BT_CHIP_TYPE, "CX2388x");
        sprintf(szVendorId,"%04X", pCard->GetVendorId());
        SetDlgItemText(hDlg, IDC_BT_VENDOR_ID, szVendorId);
        sprintf(szDeviceId,"%04X", pCard->GetDeviceId());
        SetDlgItemText(hDlg, IDC_BT_DEVICE_ID, szDeviceId);
        dwCardId = pCard->GetSubSystemId();
        if(dwCardId != 0 && dwCardId != 0xffffffff)
        {
            sprintf(szCardId,"%8X", dwCardId);
        }
        SetDlgItemText(hDlg, IDC_AUTODECTECTID, szCardId);

        return TRUE;
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            i = SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_GETCURSEL, 0, 0);
            pThis->m_TunerType->SetValue(ComboBox_GetItemData(GetDlgItem(hDlg, IDC_TUNERSELECT), i));

            i =  SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_GETCURSEL, 0, 0);
            pThis->m_CardType->SetValue(ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CARDSSELECT), i));
			WriteSettingsToIni(TRUE);
            EndDialog(hDlg, TRUE);
            break;
        case IDCANCEL:
            EndDialog(hDlg, TRUE);
            break;
        case IDC_CARDSSELECT:
            i = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_CARDSSELECT));
            i = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CARDSSELECT), i);                        
            i = pThis->m_pCard->AutoDetectTuner((eCX2388xCardId)i);
            for (nIndex = 0; nIndex < TUNER_LASTONE; nIndex++)
            {   
              if (ComboBox_GetItemData(GetDlgItem(hDlg, IDC_TUNERSELECT), nIndex) == i)
              {          
                 ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_TUNERSELECT), nIndex);
              }
            }
            break;
        case IDC_AUTODETECT:
            {
                eCX2388xCardId CardId = pThis->m_pCard->AutoDetectCardType();
                eTunerId TunerId = pThis->m_pCard->AutoDetectTuner(CardId);
                
                SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_RESETCONTENT, 0, 0);
                for(i = 0; i < CX2388xCARD_LASTONE; i++)
                {
                    int nIndex;
                    nIndex = SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_ADDSTRING, 0, (LONG)pThis->m_pCard->GetCardName((eCX2388xCardId)i));
                    SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_SETITEMDATA, nIndex, i);
                    if(i == CardId)
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
                SetFocus(hDlg);
                // Update the tuner combobox after the SetFocus
                // because SetFocus modifies this combobox
                for (nIndex = 0; nIndex < TUNER_LASTONE; nIndex++)
                {
                    i = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_TUNERSELECT), nIndex);
                    if (i == TunerId)
                    {          
                        SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_SETCURSEL, nIndex, 0);
                    }
                }
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

void CCX2388xSource::SetMenu(HMENU hMenu)
{
    int i;
    MENUITEMINFO MenuItemInfo;
    char Buffer[265];

    // set up the input menu
    for(i = 0;i < m_pCard->GetNumInputs(); ++i)
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
        sprintf(Buffer, "%s\tCtrl+Alt+F%d",m_pCard->GetInputName(i), i + 1);
        MenuItemInfo.cch = strlen(Buffer);
        SetMenuItemInfo(m_hMenu, IDM_SOURCE_INPUT1 + i, FALSE, &MenuItemInfo);
        
        // enable the menu and check it appropriately
        //EnableMenuItem(m_hMenu, IDM_SOURCE_INPUT1 + i, MF_ENABLED);
		EnableMenuItem(m_hMenu, IDM_SOURCE_INPUT1 + i, (m_TunerType->GetValue() == TUNER_ABSENT && m_pCard->IsInputATuner(i)) ? MF_GRAYED : MF_ENABLED);
        CheckMenuItemBool(m_hMenu, IDM_SOURCE_INPUT1 + i, (m_VideoSource->GetValue() == i));
	}
    
    while(i < CT_INPUTS_PER_CARD)
    {
        EnableMenuItem(m_hMenu, IDM_SOURCE_INPUT1 + i, MF_GRAYED);
        ++i;
    }

    BOOL DoneWidth = FALSE;

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

    // grey out formats if in Tuner mode as the format is saved
    // in the channel settings
    EnableMenuItemBool(m_hMenu, IDM_TYPEFORMAT_0, !IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_TYPEFORMAT_1, !IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_TYPEFORMAT_2, !IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_TYPEFORMAT_3, !IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_TYPEFORMAT_4, !IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_TYPEFORMAT_5, !IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_TYPEFORMAT_6, !IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_TYPEFORMAT_7, !IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_TYPEFORMAT_8, !IsInTunerMode());

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

	if(m_CardType->GetValue() == CX2388xCARD_HOLO3D)
	{
		CheckMenuItemBool(m_hMenu, IDM_PROGRESSIVE, m_IsVideoProgressive->GetValue());
		CheckMenuItemBool(m_hMenu, IDM_FLI_FILMDETECT, m_FLIFilmDetect->GetValue());
        EnableMenuItem(m_hMenu, IDM_PROGRESSIVE, MF_ENABLED);
        EnableMenuItem(m_hMenu, IDM_FLI_FILMDETECT, MF_ENABLED);
	}
	else
	{
        EnableMenuItem(m_hMenu, IDM_PROGRESSIVE, MF_GRAYED);
        EnableMenuItem(m_hMenu, IDM_FLI_FILMDETECT, MF_GRAYED);
	}
    
	CheckMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_MONO,			(m_StereoType->GetValue() == STEREOTYPE_MONO  ));
	CheckMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_STEREO,			(m_StereoType->GetValue() == STEREOTYPE_STEREO));
	CheckMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_LANGUAGE1,		(m_StereoType->GetValue() == STEREOTYPE_ALT1  ));
	CheckMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_LANGUAGE2,		(m_StereoType->GetValue() == STEREOTYPE_ALT2  ));
	CheckMenuItemBool(m_hMenu, IDM_AUTOSTEREO,					(m_StereoType->GetValue() == STEREOTYPE_AUTO  ));
	EnableMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_MONO,			IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_STEREO,		IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_LANGUAGE1,		IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_SOUNDCHANNEL_LANGUAGE2,		IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_AUTOSTEREO,					IsInTunerMode());

	CheckMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_AUTO,		(m_AudioStandard->GetValue() == AUDIO_STANDARD_AUTO			));
	CheckMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_A2,		(m_AudioStandard->GetValue() == AUDIO_STANDARD_A2			));
	CheckMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_BTSC,		(m_AudioStandard->GetValue() == AUDIO_STANDARD_BTSC			));
	CheckMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_BTSC_SAP,	(m_AudioStandard->GetValue() == AUDIO_STANDARD_BTSC_SAP		));
	CheckMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_EIAJ,		(m_AudioStandard->GetValue() == AUDIO_STANDARD_EIAJ			));
	CheckMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_FM,		(m_AudioStandard->GetValue() == AUDIO_STANDARD_FM			));
	CheckMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_NICAM,		(m_AudioStandard->GetValue() == AUDIO_STANDARD_NICAM		));
	EnableMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_AUTO,		IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_A2,		IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_BTSC,		IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_BTSC_SAP,	IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_EIAJ,		IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_FM,		IsInTunerMode());
    EnableMenuItemBool(m_hMenu, IDM_CX2388X_AUDIO_STD_NICAM,	IsInTunerMode());
}

BOOL CCX2388xSource::HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
{
    switch(LOWORD(wParam))
    {
        case IDM_SETUPCARD:
            Stop_Capture();
            DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_SELECTCARD), hWnd, (DLGPROC) SelectCardProc, (LPARAM)this);
            m_pCard->SetCardType(m_CardType->GetValue());
            m_pCard->InitTuner((eTunerId)m_TunerType->GetValue());
            Start_Capture();
            break;

        case IDM_HWINFO:
            DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_HWINFO), hWnd, CCX2388xCard::ChipSettingProc, (LPARAM)m_pCard);
            break;

        case IDM_SOURCE_INPUT1:
        case IDM_SOURCE_INPUT2:
        case IDM_SOURCE_INPUT3:
        case IDM_SOURCE_INPUT4:
        case IDM_SOURCE_INPUT5:
        case IDM_SOURCE_INPUT6:
        case IDM_SOURCE_INPUT7:
        case IDM_SOURCE_INPUT8:
        case IDM_SOURCE_INPUT9:
            {
                int nValue = LOWORD(wParam) - IDM_SOURCE_INPUT1;
				if (nValue < m_pCard->GetNumInputs())
				{
					if (m_TunerType->GetValue() != TUNER_ABSENT || !m_pCard->IsInputATuner(nValue))
					{
						ShowText(hWnd, m_pCard->GetInputName(nValue));
						SetTrayTip(m_pCard->GetInputName(nValue));
						m_VideoSource->SetValue(nValue);
						SendMessage(hWnd, WM_COMMAND, IDM_VT_RESET, 0);
					}
				}
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
    
        case IDM_PROGRESSIVE:
            m_IsVideoProgressive->SetValue(!m_IsVideoProgressive->GetValue());
            if(m_IsVideoProgressive->GetValue())
            {
                ShowText(hWnd, "Using Faroudja Deinterlacing");
            }
            else
            {
                ShowText(hWnd, "Using DScaler Deinterlacing");
            }
            break;

		case IDM_FLI_FILMDETECT:
            m_FLIFilmDetect->SetValue(!m_FLIFilmDetect->GetValue());
            if(m_FLIFilmDetect->GetValue())
            {
                ShowText(hWnd, "FLI2200 Film Detection - On");
            }
            else
            {
                ShowText(hWnd, "FLI2200 Film Detection - Off");
            }
            break;

        case IDM_HDELAY_PLUS:
            m_HDelay->ChangeValue(ADJUSTUP_SILENT);
            SendMessage(hWnd, WM_COMMAND, IDM_HDELAY_CURRENT, 0);
            break;

        case IDM_HDELAY_MINUS:
            m_HDelay->ChangeValue(ADJUSTDOWN_SILENT);
            SendMessage(hWnd, WM_COMMAND, IDM_HDELAY_CURRENT, 0);
            break;

        case IDM_HDELAY_CURRENT:
            m_HDelay->OSDShow();
            break;

        case IDM_VDELAY_PLUS:
            m_VDelay->ChangeValue(ADJUSTUP_SILENT);
            SendMessage(hWnd, WM_COMMAND, IDM_VDELAY_CURRENT, 0);
            break;

        case IDM_VDELAY_MINUS:
            m_VDelay->ChangeValue(ADJUSTDOWN_SILENT);
            SendMessage(hWnd, WM_COMMAND, IDM_VDELAY_CURRENT, 0);
            break;

        case IDM_VDELAY_CURRENT:
            m_VDelay->OSDShow();
            break;

        case IDM_DSVIDEO_STANDARD_0:
            // "Custom Settings ..." menu
            if (m_hCX2388xResourceInst != NULL)
            {
                m_pCard->ShowRegisterSettingsDialog(m_hCX2388xResourceInst);
            }
            else
            {
                ShowText(hWnd, "CX2388xRes.dll not loaded");
            }
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

        case IDM_PIXELWIDTH_PLUS:
            m_PixelWidth->ChangeValue(ADJUSTUP_SILENT);
            SendMessage(hWnd, WM_COMMAND, IDM_PIXELWIDTH_CURRENT, 0);
            break;

        case IDM_PIXELWIDTH_MINUS:
            m_PixelWidth->ChangeValue(ADJUSTDOWN_SILENT);
            SendMessage(hWnd, WM_COMMAND, IDM_PIXELWIDTH_CURRENT, 0);
            break;

        case IDM_PIXELWIDTH_CURRENT:
            m_PixelWidth->OSDShow();
            break;

        case IDM_SOUNDCHANNEL_MONO:
            m_StereoType->SetValue((eCX2388xStereoType)STEREOTYPE_MONO);
            break;
        
		case IDM_SOUNDCHANNEL_STEREO:
            m_StereoType->SetValue((eCX2388xStereoType)STEREOTYPE_STEREO);
            break;
        
		case IDM_SOUNDCHANNEL_LANGUAGE1:
            m_StereoType->SetValue((eCX2388xStereoType)STEREOTYPE_ALT1);
            break;
        
		case IDM_SOUNDCHANNEL_LANGUAGE2:
            m_StereoType->SetValue((eCX2388xStereoType)STEREOTYPE_ALT2);
            break;
	
		case IDM_AUTOSTEREO:
            m_StereoType->SetValue((eCX2388xStereoType)STEREOTYPE_AUTO);
            break;
		
		case IDM_CX2388X_AUDIO_STD_AUTO:
			m_AudioStandard->SetValue((eCX2388xAudioStandard)AUDIO_STANDARD_AUTO);
			break;
	
		case IDM_CX2388X_AUDIO_STD_A2:
			m_AudioStandard->SetValue((eCX2388xAudioStandard)AUDIO_STANDARD_A2);
			break;

		case IDM_CX2388X_AUDIO_STD_BTSC:
			m_AudioStandard->SetValue((eCX2388xAudioStandard)AUDIO_STANDARD_BTSC);
			break;
		
		case IDM_CX2388X_AUDIO_STD_BTSC_SAP:
			m_AudioStandard->SetValue((eCX2388xAudioStandard)AUDIO_STANDARD_BTSC_SAP);
			break;

		case IDM_CX2388X_AUDIO_STD_EIAJ:
			m_AudioStandard->SetValue((eCX2388xAudioStandard)AUDIO_STANDARD_EIAJ);
			break;
		
		case IDM_CX2388X_AUDIO_STD_FM:
			m_AudioStandard->SetValue((eCX2388xAudioStandard)AUDIO_STANDARD_FM);
			break;

		case IDM_CX2388X_AUDIO_STD_NICAM:
			m_AudioStandard->SetValue((eCX2388xAudioStandard)AUDIO_STANDARD_NICAM);
			break;
		
		default:
            return FALSE;
            break;
    }
    return TRUE;
}

void CCX2388xSource::ChangeDefaultsForVideoInput(BOOL bDontSetValue)
{
    if(m_CardType->GetValue() != CX2388xCARD_HOLO3D)
    {
        m_Brightness->ChangeDefault(128, bDontSetValue);
        m_Contrast->ChangeDefault(0x39, bDontSetValue);
        m_Hue->ChangeDefault(128, bDontSetValue);
        m_Saturation->ChangeDefault((0x7f + 0x5A) / 2, bDontSetValue);
        m_SaturationU->ChangeDefault(0x7f, bDontSetValue);
        m_SaturationV->ChangeDefault(0x5A, bDontSetValue);
        m_IsVideoProgressive->ChangeDefault(FALSE, bDontSetValue);
    }
    else
    {
        m_Brightness->ChangeDefault(128, bDontSetValue);
        m_Contrast->ChangeDefault(128, bDontSetValue);
        m_Hue->ChangeDefault(128, bDontSetValue);
        m_Saturation->ChangeDefault(128, bDontSetValue);
        m_SaturationU->ChangeDefault(128, bDontSetValue);
        m_SaturationV->ChangeDefault(128, bDontSetValue);
        m_IsVideoProgressive->ChangeDefault(TRUE, bDontSetValue);
    }
}

void CCX2388xSource::ChangeDefaultsForVideoFormat(BOOL bDontSetValue)
{
    eVideoFormat format = GetFormat();
    if(IsNTSCVideoFormat(format))
    {
        m_TopOverscan->ChangeDefault(DEFAULT_OVERSCAN_NTSC, bDontSetValue);
        m_BottomOverscan->ChangeDefault(DEFAULT_OVERSCAN_NTSC, bDontSetValue);
        m_LeftOverscan->ChangeDefault(DEFAULT_OVERSCAN_NTSC, bDontSetValue);
        m_RightOverscan->ChangeDefault(DEFAULT_OVERSCAN_NTSC, bDontSetValue);
    }
    else if(IsSECAMVideoFormat(format))
    {
        // Suggested colour values (PixelView XCapture)
        m_Saturation->ChangeDefault(57, bDontSetValue);
        m_SaturationU->ChangeDefault(64, bDontSetValue);
        m_SaturationV->ChangeDefault(51, bDontSetValue);

        m_TopOverscan->ChangeDefault(DEFAULT_OVERSCAN_PAL, bDontSetValue);
        m_BottomOverscan->ChangeDefault(DEFAULT_OVERSCAN_PAL, bDontSetValue);
        m_LeftOverscan->ChangeDefault(DEFAULT_OVERSCAN_PAL, bDontSetValue);
        m_RightOverscan->ChangeDefault(DEFAULT_OVERSCAN_PAL, bDontSetValue);
    }
    else
    {
        // Laurent's suggested colour values
        m_Saturation->ChangeDefault(133, bDontSetValue);
        m_SaturationU->ChangeDefault(154, bDontSetValue);
        m_SaturationV->ChangeDefault(112, bDontSetValue);
        // \todo add in correct default video values for PAL

        m_TopOverscan->ChangeDefault(DEFAULT_OVERSCAN_PAL, bDontSetValue);
        m_BottomOverscan->ChangeDefault(DEFAULT_OVERSCAN_PAL, bDontSetValue);
        m_LeftOverscan->ChangeDefault(DEFAULT_OVERSCAN_PAL, bDontSetValue);
        m_RightOverscan->ChangeDefault(DEFAULT_OVERSCAN_PAL, bDontSetValue);
    }
}

CTreeSettingsPage* CCX2388xSource::GetTreeSettingsPage()
{
    vector <CSimpleSetting*>vSettingsList;

    vSettingsList.push_back(m_HDelay);
    vSettingsList.push_back(m_VDelay);
    vSettingsList.push_back(m_AnalogueBlanking);
    vSettingsList.push_back(m_ConexxantStartStopDriver);
    if(m_CardType->GetValue() == CX2388xCARD_HOLO3D)
    {
        vSettingsList.push_back(m_EatLinesAtTop);
        vSettingsList.push_back(m_Sharpness);
    }
    else
    {
        vSettingsList.push_back(m_LumaAGC);
        vSettingsList.push_back(m_ChromaAGC);
        vSettingsList.push_back(m_FastSubcarrierLock);
        vSettingsList.push_back(m_WhiteCrush);
        vSettingsList.push_back(m_LowColorRemoval);
        vSettingsList.push_back(m_CombFilter);
        vSettingsList.push_back(m_FullLumaRange);
        vSettingsList.push_back(m_Remodulation);
        vSettingsList.push_back(m_Chroma2HComb);
        vSettingsList.push_back(m_ForceRemodExcessChroma);
        vSettingsList.push_back(m_IFXInterpolation);
        vSettingsList.push_back(m_CombRange);
        vSettingsList.push_back(m_SecondChromaDemod);
        vSettingsList.push_back(m_ThirdChromaDemod);
        vSettingsList.push_back(m_WhiteCrushUp);
        vSettingsList.push_back(m_WhiteCrushDown);
        vSettingsList.push_back(m_WhiteCrushMajorityPoint);
        vSettingsList.push_back(m_WhiteCrushPerFrame);
        vSettingsList.push_back(m_Volume);
        vSettingsList.push_back(m_Balance);
        vSettingsList.push_back(m_AudioStandard);
        vSettingsList.push_back(m_StereoType);
        vSettingsList.push_back(m_AutoMute);
    }

    return new CTreeSettingsGeneric("CX2388x Advanced",vSettingsList);
}

void CCX2388xSource::InitializeUI()
{
    MENUITEMINFO    MenuItemInfo;
    HMENU           hSubMenu;
    LPSTR           pMenuName;

    m_hCX2388xResourceInst = LoadLibrary("CX2388xRes.dll");

    if(m_hCX2388xResourceInst != NULL)
    {
        hSubMenu = GetSubMenu(m_hMenu, 0);

        // Set up two separators with the Custom Settings ...
        // menu in between before listing the standards.
        MenuItemInfo.cbSize = sizeof(MenuItemInfo);
        MenuItemInfo.fMask = MIIM_TYPE;
        MenuItemInfo.fType = MFT_SEPARATOR;

        pMenuName = "Custom Settings ...";
        MenuItemInfo.fMask = MIIM_TYPE | MIIM_ID;
        MenuItemInfo.fType = MFT_STRING;
        MenuItemInfo.dwTypeData = pMenuName;
        MenuItemInfo.cch = strlen(pMenuName);
        MenuItemInfo.wID = IDM_DSVIDEO_STANDARD_0;
        InsertMenuItem(hSubMenu, 5, TRUE, &MenuItemInfo);
    }
}
