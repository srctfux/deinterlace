/////////////////////////////////////////////////////////////////////////////
// $Id: Settings.cpp,v 1.37 2002-08-08 12:13:23 kooiman Exp $
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
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           changed to windows Ini file functions
//
//  3 Nov 2000   Michael Eskin         Added override of initial BDELAY setting
//               Conexant Systems      by adding non-zero InitialBDelay in .ini
//                                     File. Changed NTSC defaults to 0x5C
//
// 21 Dec 2000   John Adcock           Added function to setup ini file name
//
// 26 Dec 2000   Eric Schmidt          Fixed remember-last-channel-on-startup.
//
// 05 Jan 2001   John Adcock           Added GetRefreshRate function
//                                     Added DoAccurateFlips parameter
//                                     Added gPALFilmFallbackMode setting
//
// 07 Jan 2001   John Adcock           Added gNTSCFilmFallbackMode setting
//                                     Changed saving of gPulldownMode
//                                     so that we don't ever restart in film Mode
//                                     when we're doing autodetect  
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 08 Jan 2001   John Adcock           Made all ini file reads use initial values
//                                     of variables rather than hardcoded values here
//
// 26 Dec 2001   Eric Schmidt          Added Custom Channel Order.
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.36  2002/08/07 09:54:52  kooiman
// Added 'save per channel' settings.
//
// Revision 1.35  2002/08/06 18:31:10  kooiman
// Bit more flexibility.
//
// Revision 1.34  2002/06/13 11:43:55  robmuller
// Settings at default value that did not exist in the ini file were not written to the ini file.
//
// Revision 1.33  2002/06/13 10:40:37  robmuller
// Made anti plop mute delay configurable.
//
// Revision 1.32  2002/06/12 18:41:11  robmuller
// Fixed duplicating lines in dscaler.ini.
//
// Revision 1.31  2002/06/10 23:56:28  robmuller
// Add an empty line before each new section in the ini file.
//
// Revision 1.30  2002/02/27 20:47:21  laurentg
// Still settings
//
// Revision 1.29  2002/01/24 00:00:13  robmuller
// Added bOptimizeFileAccess flag to WriteToIni from the settings classes.
//
// Revision 1.28  2002/01/18 15:39:46  robmuller
// Prevent unnecessary actions taken when Setting_SetValue() is called with an unchanged value.
//
// Revision 1.27  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.26  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.25  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.24  2001/09/25 22:31:53  laurentg
// New control settings added for calibration
//
// Revision 1.23.2.5  2001/08/21 16:42:16  adcockj
// Per format/input settings and ini file fixes
//
// Revision 1.23.2.4  2001/08/21 09:43:01  adcockj
// Brought branch up to date with latest code fixes
//
// Revision 1.23.2.3  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.23.2.2  2001/08/17 16:35:14  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
// Revision 1.23.2.1  2001/08/14 16:41:37  adcockj
// Renamed driver
// Got to compile with new class based card
//
// Revision 1.23  2001/08/03 09:52:42  adcockj
// Added range checking on settings and fixed setting with out of range errors
//
// Revision 1.22  2001/07/28 13:24:40  adcockj
// Added UI for Overlay Controls and fixed issues with SettingsDlg
//
// Revision 1.21  2001/07/16 18:07:50  adcockj
// Added Optimisation parameter to ini file saving
//
// Revision 1.20  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.19  2001/07/12 19:24:35  adcockj
// Fixes for vertical sliders
//
// Revision 1.18  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

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
#include "other.h"
#include "FD_50Hz.h"
#include "FD_60Hz.h"
#include "FD_Common.h"
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

typedef SETTING* (__cdecl GENERICGETSETTING)(long SettingIndex);
typedef void (__cdecl GENERICREADSETTINGS)();
typedef void (__cdecl GENERICWRITESETTINGS)(BOOL);

typedef struct
{
    UINT GetValueMessage;
    GENERICGETSETTING* pfnGetSetting;
    GENERICREADSETTINGS* pfnReadSettings;
    GENERICWRITESETTINGS* pfnWriteSettings;
} TFileWithSettings;

TFileWithSettings Settings[] =
{
    {
        WM_ASPECT_GETVALUE,
        (GENERICGETSETTING*)Aspect_GetSetting,
        Aspect_ReadSettingsFromIni,
        Aspect_WriteSettingsToIni,
    },
    {
        WM_DSCALER_GETVALUE,
        (GENERICGETSETTING*)DScaler_GetSetting,
        DScaler_ReadSettingsFromIni,
        DScaler_WriteSettingsToIni,
    },
    {
        WM_OUTTHREADS_GETVALUE,
        (GENERICGETSETTING*)OutThreads_GetSetting,
        OutThreads_ReadSettingsFromIni,
        OutThreads_WriteSettingsToIni,
    },
    {
        WM_OTHER_GETVALUE,
        (GENERICGETSETTING*)Other_GetSetting,
        Other_ReadSettingsFromIni,
        Other_WriteSettingsToIni,
    },
    {
        WM_FD50_GETVALUE,
        (GENERICGETSETTING*)FD50_GetSetting,
        FD50_ReadSettingsFromIni,
        FD50_WriteSettingsToIni,
    },
    {
        WM_FD60_GETVALUE,
        (GENERICGETSETTING*)FD60_GetSetting,
        FD60_ReadSettingsFromIni,
        FD60_WriteSettingsToIni,
    },
    {
        WM_FD_COMMON_GETVALUE,
        (GENERICGETSETTING*)FD_Common_GetSetting,
        FD_Common_ReadSettingsFromIni,
        FD_Common_WriteSettingsToIni,
    },
    {
        WM_OSD_GETVALUE,
        (GENERICGETSETTING*)OSD_GetSetting,
        OSD_ReadSettingsFromIni,
        OSD_WriteSettingsToIni,
    },
    {
        WM_VBI_GETVALUE,
        (GENERICGETSETTING*)VBI_GetSetting,
        VBI_ReadSettingsFromIni,
        VBI_WriteSettingsToIni,
    },
    {
        WM_TIMING_GETVALUE,
        (GENERICGETSETTING*)Timing_GetSetting,
        Timing_ReadSettingsFromIni,
        Timing_WriteSettingsToIni,
    },
    {
        WM_MIXERDEV_GETVALUE,
        (GENERICGETSETTING*)MixerDev_GetSetting,
        MixerDev_ReadSettingsFromIni,
        MixerDev_WriteSettingsToIni,
    },
    {
        WM_CHANNELS_GETVALUE,
        (GENERICGETSETTING*)Channels_GetSetting,
        Channels_ReadSettingsFromIni,
        Channels_WriteSettingsToIni,
    },
    {
        WM_AUDIO_GETVALUE,
        (GENERICGETSETTING*)Audio_GetSetting,
        Audio_ReadSettingsFromIni,
        Audio_WriteSettingsToIni,
    },
    {
        WM_DEBUG_GETVALUE,
        (GENERICGETSETTING*)Debug_GetSetting,
        Debug_ReadSettingsFromIni,
        Debug_WriteSettingsToIni,
    },
    {
        WM_VT_GETVALUE,
        (GENERICGETSETTING*)VT_GetSetting,
        VT_ReadSettingsFromIni,
        VT_WriteSettingsToIni,
    },
    {
        WM_CALIBR_GETVALUE,
        (GENERICGETSETTING*)Calibr_GetSetting,
        Calibr_ReadSettingsFromIni,
        Calibr_WriteSettingsToIni,
    },
    {
        WM_STILL_GETVALUE,
        (GENERICGETSETTING*)Still_GetSetting,
        Still_ReadSettingsFromIni,
        Still_WriteSettingsToIni,
    },
    {
        WM_ANTIPLOP_GETVALUE,
        (GENERICGETSETTING*)AntiPlop_GetSetting,
        AntiPlop_ReadSettingsFromIni,
        AntiPlop_WriteSettingsToIni,
    },
    {
        WM_SETTINGSPERCHANNEL_GETVALUE,
        (GENERICGETSETTING*)SettingsPerChannel_GetSetting,
        SettingsPerChannel_ReadSettingsFromIni,
        SettingsPerChannel_WriteSettingsToIni,
    },
};

#define NUMSETTINGS (sizeof(Settings)/ sizeof(TFileWithSettings))

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

void LoadSettingsFromIni()
{
    for(int i(0); i < NUMSETTINGS; ++i)
    {
        Settings[i].pfnReadSettings();
    }
    Providers_ReadFromIni();       
}

LONG Settings_HandleSettingMsgs(HWND hWnd, UINT message, UINT wParam, LONG lParam, BOOL* bDone)
{
    LONG RetVal = 0;
    *bDone = FALSE;
    int i;

    for(i = 0; (*bDone == FALSE) && (i < NUMSETTINGS); ++i)
    {
        if(message == Settings[i].GetValueMessage)
        {
            RetVal =  Setting_GetValue(Settings[i].pfnGetSetting(wParam));
            *bDone = TRUE;
        }
        else if(message == Settings[i].GetValueMessage + 100)
        {
            Setting_SetValue(Settings[i].pfnGetSetting(wParam), lParam);
            *bDone = TRUE;
        }
        else if(message == Settings[i].GetValueMessage + 200)
        {
            Setting_ChangeValue(Settings[i].pfnGetSetting(wParam), (eCHANGEVALUE)lParam);
            *bDone = TRUE;
        }
    }
    return RetVal;
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
    char localcopy[256];

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
    char buf[256] = "";
    char lastline[256] = "";
    BOOL IOError = FALSE;

    buf[255] = '\0';

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
                if(fgets(buf, 255, IniFile) == NULL)
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
    rename(szTempFile, lpIniFileName);
}

void WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    // Restore all channel specific parameters temporarily to their default values
    SettingsPerChannel_ToDefaultState(TRUE);

    for(int i(0); i < NUMSETTINGS; ++i)
    {
        Settings[i].pfnWriteSettings(bOptimizeFileAccess);
    }

    Deinterlace_WriteSettingsToIni(bOptimizeFileAccess);
    Filter_WriteSettingsToIni(bOptimizeFileAccess);
    Providers_WriteToIni(bOptimizeFileAccess);

    // Restore all channel specific parameters temporarily to their default values
    SettingsPerChannel_ToDefaultState(FALSE);
    SettingsPerChannel_WriteSettings(NULL, -1, bOptimizeFileAccess);

    // These two lines flushes current INI file to disk (in case of abrupt poweroff shortly afterwards)
    WritePrivateProfileString(NULL, NULL, NULL, szIniFile);

    // Disk cache flushing:
    // The below doesn't seem to be needed and only seem to 
    // cause the video to freeze for a full second.
    //_flushall(); 

    BeautifyIniFile(szIniFile);
}

void WritePrivateProfileInt(LPCTSTR lpAppName,  LPCTSTR lpKeyName,  int nValue, LPCTSTR lpFileName)
{
    char szValue[128];
    sprintf(szValue, "%d", nValue);
    WritePrivateProfileString(lpAppName,  lpKeyName,  szValue, lpFileName);
}

long Setting_GetValue(SETTING* pSetting)
{
    if(pSetting == NULL)
    {
        return -1;
    }
    switch(pSetting->Type)
    {
    case YESNO:
    case ONOFF:
        return (BOOL) *pSetting->pValue;
        break;
    case ITEMFROMLIST:
    case SLIDER:
        return *pSetting->pValue;
        break;
    default:
        return 0;
    }
}

BOOL Setting_SetValue(SETTING* pSetting, long Value, int iForceOnChange)
{
    long NewValue;
    if(pSetting == NULL)
    {
        return FALSE;
    }

    switch(pSetting->Type)
    {
    case YESNO:
    case ONOFF:
         NewValue = (Value != 0);
        break;
    case ITEMFROMLIST:
    case SLIDER:
        if(Value > pSetting->MaxValue)
        {
            NewValue = pSetting->MaxValue;
        }
        else if(Value < pSetting->MinValue)
        {
            NewValue = pSetting->MinValue;
        }
        else
        {
            NewValue = Value;
        }
        break;
    default:
        return FALSE;
        break;
    }
    
    // If no action is needed, bail out early. This prevents the long delays when
    // pSetting->pfnOnChange() takes a while.
    if ((*pSetting->pValue == NewValue) && (iForceOnChange!=1))
    {
        return FALSE;
    }

    if ((pSetting->pfnOnChange != NULL) && (iForceOnChange>=0))
    {
        return pSetting->pfnOnChange(NewValue); 
    }
    else
    {
        *pSetting->pValue = NewValue;
        return FALSE;
    }
}


void Setting_SetDefault(SETTING* pSetting)
{
    Setting_SetValue(pSetting, pSetting->Default);
}

void Setting_SetupSlider(SETTING* pSetting, HWND hSlider)
{
    Slider_ClearTicks(hSlider, TRUE);
    Slider_SetRangeMax(hSlider, pSetting->MaxValue - pSetting->MinValue);
    Slider_SetRangeMin(hSlider, 0);
    Slider_SetPageSize(hSlider, pSetting->StepValue);
    Slider_SetLineSize(hSlider, pSetting->StepValue);
    if(GetWindowLong(hSlider, GWL_STYLE) & TBS_VERT)
    {
        Slider_SetTic(hSlider, pSetting->MaxValue - pSetting->Default);
    }
    else
    {
        Slider_SetTic(hSlider, pSetting->Default - pSetting->MinValue);
    }
    Setting_SetControlValue(pSetting, hSlider);
}

void Setting_SetControlValue(SETTING* pSetting, HWND hControl)
{
    switch(pSetting->Type)
    {
    case YESNO:
    case ONOFF:
        Button_SetCheck(hControl, *pSetting->pValue);
        break;

    case ITEMFROMLIST:
        ComboBox_SetCurSel(hControl, *pSetting->pValue);
        break;

    case SLIDER:
        if(GetWindowLong(hControl, GWL_STYLE) & TBS_VERT)
        {
            Slider_SetPos(hControl, pSetting->MaxValue - *pSetting->pValue);
        }
        else
        {
            Slider_SetPos(hControl, *pSetting->pValue - pSetting->MinValue);
        }
        break;
    default:
        break;
    }
}

BOOL Setting_SetFromControl(SETTING* pSetting, HWND hControl)
{
    long nValue;

    switch(pSetting->Type)
    {
    case YESNO:
    case ONOFF:
        nValue = (Button_GetCheck(hControl) == BST_CHECKED);
        break;

    case ITEMFROMLIST:
        nValue = ComboBox_GetCurSel(hControl);
        break;

    case SLIDER:
        nValue = Slider_GetPos(hControl);
        if(GetWindowLong(hControl, GWL_STYLE) & TBS_VERT)
        {
            nValue = pSetting->MaxValue - nValue;
        }
        else
        {
            nValue = nValue + pSetting->MinValue;
        }
        break;
    
    default:
        break;
    }
    return Setting_SetValue(pSetting, nValue);
}

BOOL Setting_ReadFromIni(SETTING* pSetting, BOOL bDontSetDefault)
{
    long nValue;
    BOOL IsSettingInIniFile = TRUE;

    if(pSetting->szIniSection != NULL)
    {
        nValue = GetPrivateProfileInt(pSetting->szIniSection, pSetting->szIniEntry, pSetting->MinValue - 100, szIniFile);
        if(nValue == pSetting->MinValue - 100)
        {
            nValue = pSetting->Default;            
            IsSettingInIniFile = FALSE;
        }
        if(nValue < pSetting->MinValue)
        {
            LOG(1, "%s %s Was out of range - %d is too low", pSetting->szIniSection, pSetting->szIniEntry, nValue);
            nValue = pSetting->MinValue;
        }
        if(nValue > pSetting->MaxValue)
        {
            LOG(1, "%s %s Was out of range - %d is too high", pSetting->szIniSection, pSetting->szIniEntry, nValue);
            nValue = pSetting->MaxValue;
        }
        if (IsSettingInIniFile || !bDontSetDefault)
        {
            *pSetting->pValue = nValue;
        }
        if(IsSettingInIniFile)
        {
            pSetting->LastSavedValue = *pSetting->pValue;
        }
        else
        {
            pSetting->LastSavedValue = pSetting->MinValue - 100;
        }
    }
    else
    {
        return FALSE;
    }
    return IsSettingInIniFile;
}

void Setting_WriteToIni(SETTING* pSetting, BOOL bOptimizeFileAccess)
{
    if(pSetting->szIniSection != NULL)
    {
		if(!bOptimizeFileAccess || pSetting->LastSavedValue != *pSetting->pValue)
		{
	        WritePrivateProfileInt(pSetting->szIniSection, pSetting->szIniEntry, *pSetting->pValue, szIniFile);
	        pSetting->LastSavedValue = *pSetting->pValue;
		}
    }
}

void Setting_OSDShow(SETTING* pSetting, HWND hWnd)
{
    char szBuffer[1024] = "Unexpected Display Error";

    switch(pSetting->Type)
    {
    case ITEMFROMLIST:
        sprintf(szBuffer, "%s %s", pSetting->szDisplayName, pSetting->pszList[*(pSetting->pValue)]);
        break;
    case YESNO:
        sprintf(szBuffer, "%s %s", pSetting->szDisplayName, *(pSetting->pValue)?"YES":"NO");
        break;
    case ONOFF:
        sprintf(szBuffer, "%s %s", pSetting->szDisplayName, *(pSetting->pValue)?"ON":"OFF");
        break;
    case SLIDER:
        if(pSetting->OSDDivider == 1)
        {
            sprintf(szBuffer, "%s %d", pSetting->szDisplayName, *(pSetting->pValue));
        }
        else if(pSetting->OSDDivider == 8)
        {
            sprintf(szBuffer, "%s %.3f", pSetting->szDisplayName, (float)*(pSetting->pValue) / (float)pSetting->OSDDivider);
        }
        else
        {
            sprintf(szBuffer, "%s %.*f", pSetting->szDisplayName, (int)log10(pSetting->OSDDivider), (float)*(pSetting->pValue) / (float)pSetting->OSDDivider);
        }
        break;
    default:
        break;
    }
    OSD_ShowText(hWnd, szBuffer, 0);
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

void Setting_SetSection(SETTING* pSetting, LPSTR NewValue)
{
    if(pSetting != NULL)
    {
        pSetting->szIniSection = NewValue;
    }
}

void Setting_Up(SETTING* pSetting)
{
    int nStep = 0;

    if (*pSetting->pValue < pSetting->MaxValue)
    {
        nStep = GetCurrentAdjustmentStepCount(pSetting) * pSetting->StepValue;
        Setting_SetValue(pSetting, *pSetting->pValue + nStep);
    }
}

void Setting_Down(SETTING* pSetting)
{
    int nStep = 0;

    if (*pSetting->pValue > pSetting->MinValue)
    {
        nStep = GetCurrentAdjustmentStepCount(pSetting) * pSetting->StepValue;
        Setting_SetValue(pSetting, *pSetting->pValue - nStep);
    }
}

void Setting_ChangeDefault(SETTING* pSetting, long Default)
{
    pSetting->Default = Default;
    *pSetting->pValue = Default;
}

void Setting_ChangeValue(SETTING* pSetting, eCHANGEVALUE NewValue)
{
    if(pSetting == NULL)
    {
        return;
    }
    switch(NewValue)
    {
    case DISPLAY:
        Setting_OSDShow(pSetting, hWnd);
        break;
    case ADJUSTUP:
        Setting_Up(pSetting);
        Setting_OSDShow(pSetting, hWnd);
        break;
    case ADJUSTDOWN:
        Setting_Down(pSetting);
        Setting_OSDShow(pSetting, hWnd);
        break;
    case INCREMENT:
        Setting_SetValue(pSetting, Setting_GetValue(pSetting) + pSetting->StepValue);
        Setting_OSDShow(pSetting, hWnd);
        break;
    case DECREMENT:
        Setting_SetValue(pSetting, Setting_GetValue(pSetting) - pSetting->StepValue);
        Setting_OSDShow(pSetting, hWnd);
        break;
    case RESET:
        Setting_SetDefault(pSetting);
        Setting_OSDShow(pSetting, hWnd);
        break;
    case TOGGLEBOOL:
        if(pSetting->Type == YESNO || pSetting->Type == ONOFF)
        {
            Setting_SetValue(pSetting, !Setting_GetValue(pSetting));
            Setting_OSDShow(pSetting, hWnd);
        }
        break;
    case ADJUSTUP_SILENT:
        Setting_Up(pSetting);
        break;
    case ADJUSTDOWN_SILENT:
        Setting_Down(pSetting);
        break;
    case INCREMENT_SILENT:
        Setting_SetValue(pSetting, Setting_GetValue(pSetting) + pSetting->StepValue);
        break;
    case DECREMENT_SILENT:
        Setting_SetValue(pSetting, Setting_GetValue(pSetting) - pSetting->StepValue);
        break;
    case RESET_SILENT:
        Setting_SetDefault(pSetting);
        break;
    case TOGGLEBOOL_SILENT:
        if(pSetting->Type == YESNO || pSetting->Type == ONOFF)
        {
            Setting_SetValue(pSetting, !Setting_GetValue(pSetting));
        }
        break;
    default:
        break;
    }
}

