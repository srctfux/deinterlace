/////////////////////////////////////////////////////////////////////////////
// $Id: HardwareSettings.cpp,v 1.1 2002-08-11 12:14:02 laurentg Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Laurent Garnier.  All rights reserved.
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
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "DScaler.h"
#include "OutThreads.h"
#include "FD_50Hz.h"
#include "FD_60Hz.h"
#include "DebugLog.h"
#include "Providers.h"

static void ChangeSettingsBasedOnHW(int ProcessorSpeed, int TradeOff)
{
    // now do defaults based on the processor speed selected
    if(ProcessorSpeed == 1 && TradeOff == 0)
    {
        // User has selected 300-500 MHz and low judder
        Setting_ChangeDefault(OutThreads_GetSetting(WAITFORFLIP), TRUE);
        Setting_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), FALSE);
        Setting_ChangeDefault(OutThreads_GetSetting(AUTODETECT), TRUE);
        Setting_ChangeDefault(FD60_GetSetting(NTSCFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
        Setting_ChangeDefault(FD50_GetSetting(PALFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
    }
    else if(ProcessorSpeed == 1 && TradeOff == 1)
    {
        // User has selected 300-500 MHz and best picture
        Setting_ChangeDefault(OutThreads_GetSetting(WAITFORFLIP), FALSE);
        Setting_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), FALSE);
        Setting_ChangeDefault(OutThreads_GetSetting(AUTODETECT), TRUE);
        Setting_ChangeDefault(FD60_GetSetting(NTSCFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
        Setting_ChangeDefault(FD50_GetSetting(PALFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
    }
    else if(ProcessorSpeed == 2 && TradeOff == 0)
    {
        // User has selected below 300 MHz and low judder
        Setting_ChangeDefault(OutThreads_GetSetting(WAITFORFLIP), TRUE);
        Setting_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), FALSE);
        Setting_ChangeDefault(OutThreads_GetSetting(AUTODETECT), FALSE);
        Setting_ChangeDefault(FD60_GetSetting(NTSCFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
        Setting_ChangeDefault(FD50_GetSetting(PALFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
    }
    else if(ProcessorSpeed == 2 && TradeOff == 1)
    {
        // User has selected below 300 MHz and best picture
        Setting_ChangeDefault(OutThreads_GetSetting(WAITFORFLIP), FALSE);
        Setting_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), FALSE);
        Setting_ChangeDefault(OutThreads_GetSetting(AUTODETECT), FALSE);
        Setting_ChangeDefault(FD60_GetSetting(NTSCFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
        Setting_ChangeDefault(FD50_GetSetting(PALFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
    }
    else
    {
        // user has fast processor use best defaults
        Setting_ChangeDefault(OutThreads_GetSetting(WAITFORFLIP), TRUE);
        Setting_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), TRUE);
        Setting_ChangeDefault(OutThreads_GetSetting(AUTODETECT), TRUE);
        Setting_ChangeDefault(FD60_GetSetting(NTSCFILMFALLBACKMODE), INDEX_VIDEO_GREEDYH);
        Setting_ChangeDefault(FD50_GetSetting(PALFILMFALLBACKMODE), INDEX_VIDEO_GREEDYH);
    }

    Providers_ChangeSettingsBasedOnHW(Setting_GetValue(DScaler_GetSetting(PROCESSORSPEED)), Setting_GetValue(DScaler_GetSetting(TRADEOFF)));
}

BOOL APIENTRY HardwareSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
//    static long OrigProcessorSpeed;
//    static long OrigTradeOff;

    switch (message)
    {
    case WM_INITDIALOG:
        SendMessage(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED), CB_ADDSTRING, 0, (LONG)"Above 500 MHz");
        SendMessage(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED), CB_ADDSTRING, 0, (LONG)"300 - 500 MHz");
        SendMessage(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED), CB_ADDSTRING, 0, (LONG)"Below 300 MHz");
        SendMessage(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED), CB_SETCURSEL, Setting_GetValue(DScaler_GetSetting(PROCESSORSPEED)), 0);
        SendMessage(GetDlgItem(hDlg, IDC_TRADEOFF), CB_ADDSTRING, 0, (LONG)"Show all frames - Lowest judder");
        SendMessage(GetDlgItem(hDlg, IDC_TRADEOFF), CB_ADDSTRING, 0, (LONG)"Best picture quality");
        SendMessage(GetDlgItem(hDlg, IDC_TRADEOFF), CB_SETCURSEL, Setting_GetValue(DScaler_GetSetting(TRADEOFF)), 0);
//        OrigProcessorSpeed = Setting_GetValue(DScaler_GetSetting(PROCESSORSPEED));
//        OrigTradeOff = Setting_GetValue(DScaler_GetSetting(TRADEOFF));
        SetFocus(hDlg);
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            Setting_SetValue(DScaler_GetSetting(PROCESSORSPEED), ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED)));
            Setting_SetValue(DScaler_GetSetting(TRADEOFF), ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_TRADEOFF)));
//            if(OrigProcessorSpeed != Setting_GetValue(DScaler_GetSetting(PROCESSORSPEED)) || 
//                OrigTradeOff != Setting_GetValue(DScaler_GetSetting(TRADEOFF)))
//            {
                ChangeSettingsBasedOnHW(Setting_GetValue(DScaler_GetSetting(PROCESSORSPEED)), Setting_GetValue(DScaler_GetSetting(TRADEOFF)));
//            }
			WriteSettingsToIni(FALSE);
            EndDialog(hDlg, TRUE);
            break;
        case IDCANCEL:
            EndDialog(hDlg, TRUE);
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
