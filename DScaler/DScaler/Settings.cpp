/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
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

/**
 * @file Settings.cpp Settings Functions
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "Settings.h"
#include "Audio.h"
#include "VBI.h"
#include "OutThreads.h"
#include "deinterlace.h"
#include "AspectRatio.h"
#include "DebugLog.h"
#include "MixerDev.h"
#include "DScaler.h"
#include "ProgramList.h"
#include "OverlayOutput.h"
#include "D3D9Output.h"
#include "FD_50Hz.h"
#include "FD_60Hz.h"
#include "FD_Common.h"
#include "FD_Prog.h"
#include "Slider.h"
#include "Splash.h"
#include "OSD.h"
#include "Filter.h"
#include "FieldTiming.h"
#include "VBI_VideoText.h"
#include "Providers.h"
#include "Calibration.h"
#include "StillSource.h"
#include "SettingsPerChannel.h"
#include "SettingsMaster.h"
#include "TimeShift.h"
#include "EPG.h"

using namespace std;

char szIniFile[MAX_PATH] = "DScaler.ini";

void SetIniFileForSettings(LPSTR Name)
{
    GetCurrentDirectory(MAX_PATH, szIniFile);
    if (*Name == 0)         // add parm TRB 12/00
    {
        strcat(szIniFile, "\\DScaler.ini");
    }
    else
    {
        strcat(szIniFile, "\\");
        strcat(szIniFile, Name);
    }
}

LPCSTR GetIniFileForSettings()
{
    return szIniFile;
}

// this function removes any comments, leading spaces and tabs, trailing spaces, tabs,
// carriage returns and new line characters
LPTSTR CleanUpLine(LPTSTR lpString)
{
    int i = 0;
    char *string = lpString;

    if(string == NULL || string[0] == '\0')
    {
        return lpString;
    }

    // remove leading spaces and tabs
    while(string[0] == ' ' || string[0] == '\t')
    {
        string++;
    }

    strcpy(lpString, string);

    // remove any comments
    string = strchr(lpString, ';');
    if(string != NULL)
    {
        *string = '\0';
    }

    // remove trailing spaces, tabs, newline and carriage return
    i = strlen(lpString) - 1;
    while(i >= 0 && (lpString[i] == ' ' || lpString[i] == '\t' || lpString[i] == '\r') ||
          lpString[i] == '\n')
    {
        lpString[i--] = '\0';
    }
    return lpString;
}

// this function returns TRUE if lpString contains a valid section name ('[blahblah]')
BOOL IsSectionName(LPCTSTR lpString)
{
    char localcopy[2560];

    if(lpString == NULL)
    {
        return FALSE;
    }

    strncpy(localcopy, lpString, sizeof(localcopy));

    CleanUpLine(localcopy);

    if(strlen(localcopy) < 3)
    {
        return FALSE;
    }
    return (localcopy[0] == '[' && localcopy[strlen(localcopy) - 1] == ']');
}

#define TEMPFILENAME "ThisFileCanBeRemoved.tmp"

// This function adds an empty line before each new section in the ini file to greatly
// enhance readability.
void BeautifyIniFile(LPCTSTR lpIniFileName)
{
    FILE* IniFile = NULL;
    FILE* TempFile = NULL;
    char szTempFile[MAX_PATH] = "";
    char buf[2560] = "";
    char lastline[2560] = "";
    BOOL IOError = FALSE;

    buf[2550] = '\0';

    GetCurrentDirectory(MAX_PATH, szTempFile);
    strcat(szTempFile, "\\");
    strcat(szTempFile, TEMPFILENAME);

    TempFile = fopen(szTempFile, "w");
    if(TempFile == NULL)
    {
        LOG(1, "BeautifyIniFile: Error opening temp file for writing.");
        return;
    }
    else
    {
        IniFile = fopen(lpIniFileName, "r");
        if(IniFile == NULL)
        {
            fclose(TempFile);
            remove(szTempFile);
            LOG(1, "BeautifyIniFile: Error opening ini file for reading.");
            return;
        }
        else
        {
            while(!feof(IniFile))
            {
                if(fgets(buf, 2550, IniFile) == NULL)
                {
                    if(feof(IniFile))
                    {
                        break;
                    }
                    else
                    {
                        IOError = TRUE;
                        LOG(1, "BeautifyIniFile: Error reading ini file.");
                    }
                }
                if(IsSectionName(buf) && lastline[0] != '\0')
                {
                    if(fputs("\n", TempFile) < 0)
                    {
                        IOError = TRUE;
                        LOG(1, "BeautifyIniFile: Error writing temp file (#1).");
                    }
                }
                if(fputs(buf, TempFile) < 0)
                {
                    IOError = TRUE;
                    LOG(1, "BeautifyIniFile: Error writing temp file (#2).");
                }
                if(IOError)
                {
                    fclose(IniFile);
                    fclose(TempFile);
                    remove(szTempFile);
                    return;
                }
                strncpy(lastline, buf, sizeof(lastline));
                CleanUpLine(lastline);
            }
        }
        fclose(IniFile);
        fclose(TempFile);
    }
    remove(lpIniFileName);

    // RM Sept 28 2006: The following while loop and Sleep statement should not be needed.
    // It was added because Antivir Personal Edition Classic was causing problems when
    // the win32 file heuristic of the Antivir Guard was set to the medium level.
    // Antivir causes a delay in removing the ini file which causes the rename to fail.
    //
    int i = 0;
    while(rename(szTempFile, lpIniFileName) != 0)
    {
        Sleep(1);
        if(i++ > 100)
        {
            LOG(1, "BeautifyIniFile: rename failed.");
            break;
        }
    }
    if(i != 0)
    {
        LOG(1, "BeautifyIniFile: Sleep needed %d time(s) before rename.", i);
    }
}

void WritePrivateProfileInt(LPCTSTR lpAppName,  LPCTSTR lpKeyName,  int nValue, LPCTSTR lpFileName)
{
    char szValue[128];
    sprintf(szValue, "%d", nValue);
    WritePrivateProfileString(lpAppName,  lpKeyName,  szValue, lpFileName);
    LOG(2, " WritePrivateProfileInt %s %s Value %s", lpAppName, lpKeyName, szValue);
}


SmartPtr<CSimpleSetting> GetSetting(long GetValueMsg, long SettingIndex)
{
    SmartPtr<CSettingsHolder> Holder(SettingsMaster->FindMsgHolder(GetValueMsg));
    if(Holder)
    {
        SmartPtr<CSimpleSetting> Setting(Holder->GetSetting(SettingIndex));
        if(Setting)
        {
            return Setting;
        }
        else
        {
            throw std::logic_error("Can't find setting");
        }
    }
    else
    {
        throw std::logic_error("Can't find setting");
    }
}

long Setting_GetValue(long GetValueMsg, long SettingIndex)
{
    return GetSetting(GetValueMsg, SettingIndex)->GetValueAsMessage();
}

void Setting_SetValue(long GetValueMsg, long SettingIndex, long Value)
{
    GetSetting(GetValueMsg, SettingIndex)->SetValueFromMessage(Value);
}

void Setting_SetDefault(long GetValueMsg, long SettingIndex)
{
    GetSetting(GetValueMsg, SettingIndex)->ChangeValue(RESET);
}

void Setting_ChangeDefault(long GetValueMsg, long SettingIndex, long Default)
{
    GetSetting(GetValueMsg, SettingIndex)->ChangeDefault(Default);
}

void Setting_SetupSlider(long GetValueMsg, long SettingIndex, HWND hSlider)
{
    GetSetting(GetValueMsg, SettingIndex)->SetupControl(hSlider);
}

void Setting_SetControlValue(long GetValueMsg, long SettingIndex, HWND hControl)
{
    GetSetting(GetValueMsg, SettingIndex)->SetControlValue(hControl);
}

void Setting_SetFromControl(long GetValueMsg, long SettingIndex, HWND hControl)
{
    GetSetting(GetValueMsg, SettingIndex)->SetFromControl(hControl);
}

void Setting_OSDShow(long GetValueMsg, long SettingIndex, HWND hWnd)
{
    GetSetting(GetValueMsg, SettingIndex)->ChangeValue(DISPLAY);
}

void Setting_Up(long GetValueMsg, long SettingIndex)
{
    GetSetting(GetValueMsg, SettingIndex)->ChangeValue(ADJUSTUP);
}

void Setting_Down(long GetValueMsg, long SettingIndex)
{
    GetSetting(GetValueMsg, SettingIndex)->ChangeValue(ADJUSTDOWN);
}

//---------------------------------------------------------------------------
// This function allows for accelerated slider adjustments
// For example, adjusting Contrast or Brightness faster the longer
// you hold down the adjustment key.
int GetCurrentAdjustmentStepCount(void* pSetting)
{
    static DWORD        dwLastTick = 0;
    static DWORD        dwFirstTick = 0;
    static DWORD        dwTaps = 0;
    DWORD               dwTick;
    DWORD               dwElapsedSinceLastCall;
    DWORD               dwElapsedSinceFirstTick;
    int                 nStepCount;
    static void*     LastSetting = NULL;

    if(LastSetting != pSetting)
    {
        dwLastTick = 0;
        dwFirstTick = 0;
        dwTaps = 0;
        LastSetting = pSetting;
    }

    dwTick = GetTickCount();
    dwElapsedSinceLastCall = dwTick - dwLastTick;
    dwElapsedSinceFirstTick = dwTick - dwFirstTick;

    if ((dwTaps < ADJ_MINIMUM_REPEAT_BEFORE_ACCEL) &&
        (dwElapsedSinceLastCall < ADJ_BUTTON_REPRESS_REPEAT_DELAY))
    {
        // Ensure that the button or keypress is repeated or tapped
        // a minimum number of times before acceleration begins
        dwFirstTick = dwTick;
        nStepCount = 1;
    }
    if (dwElapsedSinceLastCall < ADJ_KEYB_TYPEMATIC_REPEAT_DELAY)
    {
        // This occurs if the end-user is holding down a keyboard key.
        // The longer the time has elapsed since the keyboard key has
        // been held down, the bigger the adjustment steps become, up to a maximum.
        nStepCount = 1 + (dwElapsedSinceFirstTick / ADJ_KEYB_TYPEMATIC_ACCEL_STEP);
        if (nStepCount > ADJ_KEYB_TYPEMATIC_MAX_STEP)
        {
            nStepCount = ADJ_KEYB_TYPEMATIC_MAX_STEP;
        }
    }
    else if (dwElapsedSinceLastCall < ADJ_BUTTON_REPRESS_REPEAT_DELAY)
    {
        // This occurs if the end-user is tapping a button repeatedly
        // such as on a handheld remote control, when a universal remote
        // is programmed with a keypress.  Most remotes cannot repeat
        // a keypress automatically, so the end user must tap the key.
        // The longer the time has elapsed since the first button press,
        // the bigger the adjustment steps become, up to a maximum.
        nStepCount = 1 + (dwElapsedSinceFirstTick / ADJ_BUTTON_REPRESS_ACCEL_STEP);
        if (nStepCount > ADJ_BUTTON_REPRESS_MAX_STEP)
        {
            nStepCount = ADJ_BUTTON_REPRESS_MAX_STEP;
        }
    }
    else
    {
        // The keypress or button press is no longer consecutive,
        // so reset adjustment step.
        dwFirstTick = dwTick;
        nStepCount = 1;
        dwTaps = 0;
    }
    dwTaps++;
    dwLastTick = dwTick;
    return nStepCount;
}
