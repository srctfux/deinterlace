/////////////////////////////////////////////////////////////////////////////
// Settings.c
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
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
//                                     so that we don't ever restart in film mode
//                                     when we're doing autodetect  
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 08 Jan 2001   John Adcock           Made all ini file reads use initial values
//                                     of variables rather than hardcoded values here
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "settings.h"
#include "tuner.h"
#include "audio.h"
#include "bt848.h"
#include "vbi.h"
#include "OutThreads.h"
#include "deinterlace.h"
#include "AspectRatio.h"
#include "DebugLog.h"
#include "FLT_TNoise.h"
#include "MixerDev.h"
#include "dTV.h"
#include "ProgramList.h"
#include "DI_Adaptive.h"
#include "DI_BlendedClip.h"
#include "DI_BobAndWeave.h"
#include "DI_TwoFrame.h"
#include "DI_Greedy.h"
#include "other.h"
#include "FD_50Hz.h"
#include "FD_60Hz.h"
#include "FD_Common.h"
#include "slider.h"
#include "Splash.h"
#include "OSD.h"
#include "TVCards.h"
#include "VideoSettings.h"

char szIniFile[MAX_PATH] = "dTV.ini";

void SetIniFileForSettings(LPSTR Name)
{
	GetCurrentDirectory(MAX_PATH, szIniFile);
	if (*Name == 0)			// add parm TRB 12/00
	{
		strcat(szIniFile, "\\dTV.ini");
	}
	else
	{
		strcat(szIniFile, "\\");
		strcat(szIniFile, Name);
	}
}

void LoadSettingsFromIni()
{
	char szKey[128];
	int i;

	// TVCard setting smust be called first as this modifys some defaults for the
	// other settings, but we need to be able to override the defaults.
	TVCard_ReadSettingsFromIni();

	// Read in rest of settings from each source files read method
	Aspect_ReadSettingsFromIni();
	BT848_ReadSettingsFromIni();
	dTV_ReadSettingsFromIni();
	OutThreads_ReadSettingsFromIni();
	Other_ReadSettingsFromIni();
	FD50_ReadSettingsFromIni();
	FD60_ReadSettingsFromIni();
	FD_Common_ReadSettingsFromIni();
	DI_Adaptive_ReadSettingsFromIni();
	DI_BobWeave_ReadSettingsFromIni();
	DI_BlendedClip_ReadSettingsFromIni();
	DI_TwoFrame_ReadSettingsFromIni();
	Deinterlace_ReadSettingsFromIni();
	FLT_TNoise_ReadSettingsFromIni();
	VideoSettings_ReadSettingsFromIni();

	VBI_Flags = 0;
	if(GetPrivateProfileInt("VBI", "VT", 0, szIniFile) != 0)
	{
		VBI_Flags += VBI_VT;
	}
	if(GetPrivateProfileInt("VBI", "VPS", 0, szIniFile) != 0)
	{
		VBI_Flags += VBI_VPS;
	}
	if(GetPrivateProfileInt("VBI", "CC", 0, szIniFile) != 0)
	{
		VBI_Flags += VBI_CC;
	}

	GetPrivateProfileString("Files", "DebugLogFilename", DebugLogFilename, DebugLogFilename, MAX_PATH, szIniFile);
	DebugLogEnabled = GetPrivateProfileInt("Files", "DebugLogEnabled", DebugLogEnabled, szIniFile);

	Capture_VBI = (GetPrivateProfileInt("Show", "CaptureVBI", Capture_VBI, szIniFile) != 0);  
	CurrentProgramm = GetPrivateProfileInt("Show", "LastProgram", CurrentProgramm, szIniFile);

	AudioSource = GetPrivateProfileInt("Sound", "AudioSource", AudioSource, szIniFile);
	System_In_Mute = (GetPrivateProfileInt("Sound", "System_In_Mute", System_In_Mute, szIniFile) != 0);
		
	bSaveSettings = (GetPrivateProfileInt("Show", "SaveSettings", CountryCode, szIniFile) != 0);
	CountryCode = GetPrivateProfileInt("Show", "CountryCode", CountryCode, szIniFile);

	for(i = 0; i < 9; i++)
	{
		sprintf(szKey, "Color%d", i + 1);
		VTColourTable[i] = GetPrivateProfileInt("VT", szKey, VTColourTable[i], szIniFile);
	}
	
	MSPMode = GetPrivateProfileInt("MSP", "MSPMode", MSPMode, szIniFile);	
	MSPMajorMode = GetPrivateProfileInt("MSP", "MSPMajorMode", MSPMajorMode, szIniFile);	
	MSPMinorMode = GetPrivateProfileInt("MSP", "MSPMinorMode", MSPMinorMode, szIniFile);	
	MSPStereo = GetPrivateProfileInt("MSP", "MSPStereo", MSPStereo, szIniFile);	
	AutoStereoSelect = (GetPrivateProfileInt("MSP", "MSPAutoStereo", AutoStereoSelect, szIniFile) != 0);	
	InitialVolume = GetPrivateProfileInt("MSP", "Volume", InitialVolume, szIniFile);	
	InitialSpatial = GetPrivateProfileInt("MSP", "Spatial", InitialSpatial, szIniFile);	
	InitialLoudness = GetPrivateProfileInt("MSP", "Loudness", InitialLoudness, szIniFile);	
	InitialBass = GetPrivateProfileInt("MSP", "Bass", InitialBass, szIniFile);	
	InitialTreble = GetPrivateProfileInt("MSP", "Treble", InitialTreble, szIniFile);	
	InitialBalance = GetPrivateProfileInt("MSP", "Balance", InitialBalance, szIniFile);	
	InitialSuperBass = (GetPrivateProfileInt("MSP", "SuperBass", InitialSuperBass, szIniFile) != 0);	

	for(i = 0; i < 5; i++)
	{
		sprintf(szKey, "Equalizer%d", i + 1);
		InitialEqualizer[i] = GetPrivateProfileInt("MSP", szKey, 0, szIniFile);
	}

	USE_MIXER = (GetPrivateProfileInt("Mixer", "UseMixer", USE_MIXER, szIniFile) != 0);	
	MIXER_LINKER_KANAL = GetPrivateProfileInt("Mixer", "VolLeftChannel", MIXER_LINKER_KANAL, szIniFile);
	MIXER_RECHTER_KANAL = GetPrivateProfileInt("Mixer", "VolRightChannel", MIXER_RECHTER_KANAL, szIniFile); 

	Volume.SoundSystem = GetPrivateProfileInt("Mixer", "VolumeSoundSystem", Volume.SoundSystem, szIniFile);
	Volume.Destination = GetPrivateProfileInt("Mixer", "VolumeDestination", Volume.Destination, szIniFile);
	Volume.Connection = GetPrivateProfileInt("Mixer", "VolumeConnection", Volume.Connection, szIniFile);
	Volume.Control = GetPrivateProfileInt("Mixer", "VolumeControl", Volume.Control, szIniFile);

	Mute.SoundSystem = GetPrivateProfileInt("Mixer", "MuteSoundSystem", Mute.SoundSystem, szIniFile);
	Mute.Destination = GetPrivateProfileInt("Mixer", "MuteDestination", Mute.Destination, szIniFile);
	Mute.Connection = GetPrivateProfileInt("Mixer", "MuteConnection", Mute.Connection, szIniFile);
	Mute.Control = GetPrivateProfileInt("Mixer", "MuteControl", Mute.Control, szIniFile);

	for(i = 0; i < 64; i++)
	{
		sprintf(szKey, "MixerSettings%d", i + 1);
		MixerLoad[i].MixerAccess.SoundSystem = GetPrivateProfileInt(szKey, "SoundSystem", -1, szIniFile);
		if(MixerLoad[i].MixerAccess.SoundSystem == -1)
		{
			MixerLoad[i].MixerAccess.Destination = 0;
			MixerLoad[i].MixerAccess.Connection = 0;
			MixerLoad[i].MixerAccess.Control = 0;
			MixerLoad[i].MixerValues.Kanal1 = 0;
			MixerLoad[i].MixerValues.Kanal2 = 0;
			MixerLoad[i].MixerValues.Kanal3 = 0;
			MixerLoad[i].MixerValues.Kanal4 = 0;
		}
		else
		{
			MixerLoad[i].MixerAccess.Destination = GetPrivateProfileInt(szKey, "Destination", 0, szIniFile);
			MixerLoad[i].MixerAccess.Connection = GetPrivateProfileInt(szKey, "Connection", 0, szIniFile);
			MixerLoad[i].MixerAccess.Control = GetPrivateProfileInt(szKey, "Control", 0, szIniFile);
			MixerLoad[i].MixerValues.Kanal1 = GetPrivateProfileInt(szKey, "Channel1", 0, szIniFile);
			MixerLoad[i].MixerValues.Kanal2 = GetPrivateProfileInt(szKey, "Channel2", 0, szIniFile);
			MixerLoad[i].MixerValues.Kanal3 = GetPrivateProfileInt(szKey, "Channel3", 0, szIniFile);
			MixerLoad[i].MixerValues.Kanal4 = GetPrivateProfileInt(szKey, "Channel4", 0, szIniFile);
		}
	}
}

LONG Settings_HandleSettingMsgs(HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
	switch(message)
	{
		case WM_ASPECT_GETVALUE:
			return Setting_GetValue(Aspect_GetSetting((ASPECT_SETTING)wParam));
			break;
		case WM_BT848_GETVALUE:		
			return Setting_GetValue(BT848_GetSetting((BT848_SETTING)wParam));
			break;
		case WM_DTV_GETVALUE:		
			return Setting_GetValue(dTV_GetSetting((DTV_SETTING)wParam));
			break;
		case WM_OUTHREADS_GETVALUE:
			return Setting_GetValue(OutThreads_GetSetting((OUTTHREADS_SETTING)wParam));
			break;
		case WM_OTHER_GETVALUE:		
			return Setting_GetValue(Other_GetSetting((OTHER_SETTING)wParam));
			break;
		case WM_FD50_GETVALUE:		
			return Setting_GetValue(FD50_GetSetting((FD50_SETTING)wParam));
			break;
		case WM_FD60_GETVALUE:		
			return Setting_GetValue(FD60_GetSetting((FD60_SETTING)wParam));
			break;
		case WM_FD_COMMON_GETVALUE:
			return Setting_GetValue(FD_Common_GetSetting((FD_COMMON_SETTING)wParam));
			break;
		case WM_DI_ADAPTIVE_GETVALUE:	
			return Setting_GetValue(DI_Adaptive_GetSetting((DI_ADAPTIVE_SETTING)wParam));
			break;
		case WM_DI_BOBWEAVE_GETVALUE:	
			return Setting_GetValue(DI_BobWeave_GetSetting((DI_BOBWEAVE_SETTING)wParam));
			break;
		case WM_DI_BLENDEDCLIP_GETVALUE:
			return Setting_GetValue(DI_BlendedClip_GetSetting((DI_BLENDEDCLIP_SETTING)wParam));
			break;
		case WM_DI_GREEDY_GETVALUE:
			return Setting_GetValue(DI_Greedy_GetSetting((DI_GREEDY_SETTING)wParam));
			break;
		case WM_DI_TWOFRAME_GETVALUE:	
			return Setting_GetValue(DI_TwoFrame_GetSetting((DI_TWOFRAME_SETTING)wParam));
			break;
		case WM_DEINTERLACE_GETVALUE:	
			return Setting_GetValue(Deinterlace_GetSetting((DEINTERLACE_SETTING)wParam));
			break;
		case WM_FLT_TNOISE_GETVALUE:		
			return Setting_GetValue(FLT_TNoise_GetSetting((FLT_TNOISE_SETTING)wParam));
			break;
		case WM_TVCARD_GETVALUE:		
			return Setting_GetValue(TVCard_GetSetting((TVCARD_SETTING)wParam));
			break;
		case WM_VIDEOSETTINGS_GETVALUE:		
			return Setting_GetValue(VideoSettings_GetSetting((VIDEOSETTINGS_SETTING)wParam));
			break;

		case WM_ASPECT_SETVALUE:
			Setting_SetValue(Aspect_GetSetting((ASPECT_SETTING)wParam), lParam);
			break;
		case WM_BT848_SETVALUE:		
			Setting_SetValue(BT848_GetSetting((BT848_SETTING)wParam), lParam);
			break;
		case WM_DTV_SETVALUE:		
			Setting_SetValue(dTV_GetSetting((DTV_SETTING)wParam), lParam);
			break;
		case WM_OUTHREADS_SETVALUE:
			Setting_SetValue(OutThreads_GetSetting((OUTTHREADS_SETTING)wParam), lParam);
			break;
		case WM_OTHER_SETVALUE:		
			Setting_SetValue(Other_GetSetting((OTHER_SETTING)wParam), lParam);
			break;
		case WM_FD50_SETVALUE:		
			Setting_SetValue(FD50_GetSetting((FD50_SETTING)wParam), lParam);
			break;
		case WM_FD60_SETVALUE:		
			Setting_SetValue(FD60_GetSetting((FD60_SETTING)wParam), lParam);
			break;
		case WM_FD_COMMON_SETVALUE:
			Setting_SetValue(FD_Common_GetSetting((FD_COMMON_SETTING)wParam), lParam);
			break;
		case WM_DI_ADAPTIVE_SETVALUE:	
			Setting_SetValue(DI_Adaptive_GetSetting((DI_ADAPTIVE_SETTING)wParam), lParam);
			break;
		case WM_DI_BOBWEAVE_SETVALUE:	
			Setting_SetValue(DI_BobWeave_GetSetting((DI_BOBWEAVE_SETTING)wParam), lParam);
			break;
		case WM_DI_BLENDEDCLIP_SETVALUE:
			Setting_SetValue(DI_BlendedClip_GetSetting((DI_BLENDEDCLIP_SETTING)wParam), lParam);
			break;
		case WM_DI_GREEDY_SETVALUE:
			Setting_SetValue(DI_Greedy_GetSetting((DI_GREEDY_SETTING)wParam), lParam);
			break;
		case WM_DI_TWOFRAME_SETVALUE:	
			Setting_SetValue(DI_TwoFrame_GetSetting((DI_TWOFRAME_SETTING)wParam), lParam);
			break;
		case WM_DEINTERLACE_SETVALUE:	
			Setting_SetValue(Deinterlace_GetSetting((DEINTERLACE_SETTING)wParam), lParam);
			break;
		case WM_FLT_TNOISE_SETVALUE:		
			Setting_SetValue(FLT_TNoise_GetSetting((FLT_TNOISE_SETTING)wParam), lParam);
			break;
		case WM_TVCARD_SETVALUE:		
			Setting_SetValue(TVCard_GetSetting((TVCARD_SETTING)wParam), lParam);
			break;
		case WM_VIDEOSETTINGS_SETVALUE:		
			Setting_SetValue(VideoSettings_GetSetting((VIDEOSETTINGS_SETTING)wParam), lParam);
			break;

		case WM_ASPECT_CHANGEVALUE:
			Setting_ChangeValue(Aspect_GetSetting((ASPECT_SETTING)wParam), lParam);
			break;
		case WM_BT848_CHANGEVALUE:		
			Setting_ChangeValue(BT848_GetSetting((BT848_SETTING)wParam), lParam);
			break;
		case WM_DTV_CHANGEVALUE:		
			Setting_ChangeValue(dTV_GetSetting((DTV_SETTING)wParam), lParam);
			break;
		case WM_OUTHREADS_CHANGEVALUE:
			Setting_ChangeValue(OutThreads_GetSetting((OUTTHREADS_SETTING)wParam), lParam);
			break;
		case WM_OTHER_CHANGEVALUE:		
			Setting_ChangeValue(Other_GetSetting((OTHER_SETTING)wParam), lParam);
			break;
		case WM_FD50_CHANGEVALUE:		
			Setting_ChangeValue(FD50_GetSetting((FD50_SETTING)wParam), lParam);
			break;
		case WM_FD60_CHANGEVALUE:		
			Setting_ChangeValue(FD60_GetSetting((FD60_SETTING)wParam), lParam);
			break;
		case WM_FD_COMMON_CHANGEVALUE:
			Setting_ChangeValue(FD_Common_GetSetting((FD_COMMON_SETTING)wParam), lParam);
			break;
		case WM_DI_ADAPTIVE_CHANGEVALUE:	
			Setting_ChangeValue(DI_Adaptive_GetSetting((DI_ADAPTIVE_SETTING)wParam), lParam);
			break;
		case WM_DI_BOBWEAVE_CHANGEVALUE:	
			Setting_ChangeValue(DI_BobWeave_GetSetting((DI_BOBWEAVE_SETTING)wParam), lParam);
			break;
		case WM_DI_BLENDEDCLIP_CHANGEVALUE:
			Setting_ChangeValue(DI_BlendedClip_GetSetting((DI_BLENDEDCLIP_SETTING)wParam), lParam);
			break;
		case WM_DI_GREEDY_CHANGEVALUE:
			Setting_ChangeValue(DI_Greedy_GetSetting((DI_GREEDY_SETTING)wParam), lParam);
			break;
		case WM_DI_TWOFRAME_CHANGEVALUE:	
			Setting_ChangeValue(DI_TwoFrame_GetSetting((DI_TWOFRAME_SETTING)wParam), lParam);
			break;
		case WM_DEINTERLACE_CHANGEVALUE:	
			Setting_ChangeValue(Deinterlace_GetSetting((DEINTERLACE_SETTING)wParam), lParam);
			break;
		case WM_FLT_TNOISE_CHANGEVALUE:		
			Setting_ChangeValue(FLT_TNoise_GetSetting((FLT_TNOISE_SETTING)wParam), lParam);
			break;
		case WM_TVCARD_CHANGEVALUE:		
			Setting_ChangeValue(TVCard_GetSetting((TVCARD_SETTING)wParam), lParam);
			break;
		case WM_VIDEOSETTINGS_CHANGEVALUE:		
			Setting_ChangeValue(VideoSettings_GetSetting((VIDEOSETTINGS_SETTING)wParam), lParam);
			break;
		
		default:
			break;
	}

    // Updates the menu checkbox settings
	SetMenuAnalog();

    // Set the configuration file autosave timer.
    // We use an autosave timer so that when the user has finished
    // making adjustments and at least a small delay has occured,
    // that the DTV.INI file is properly up to date, even if 
    // the system crashes or system is turned off abruptly.
    KillTimer(hWnd, TIMER_AUTOSAVE);
    SetTimer(hWnd, TIMER_AUTOSAVE, TIMER_AUTOSAVE_MS, NULL);

	return DefWindowProc(hWnd, message, wParam, lParam);
}


void WriteSettingsToIni()
{
	char szKey[128];
	int i;

	Aspect_WriteSettingsToIni();
	BT848_WriteSettingsToIni();
	dTV_WriteSettingsToIni();
	OutThreads_WriteSettingsToIni();
	Other_WriteSettingsToIni();
	FD50_WriteSettingsToIni();
	FD60_WriteSettingsToIni();
	FD_Common_WriteSettingsToIni();
	DI_Adaptive_WriteSettingsToIni();
	DI_BobWeave_WriteSettingsToIni();
	DI_BlendedClip_WriteSettingsToIni();
	DI_TwoFrame_WriteSettingsToIni();
	Deinterlace_WriteSettingsToIni();
	FLT_TNoise_WriteSettingsToIni();
	TVCard_WriteSettingsToIni();
	VideoSettings_WriteSettingsToIni();

	WritePrivateProfileInt("VBI", "VT", VBI_Flags & VBI_VT, szIniFile);
	WritePrivateProfileInt("VBI", "VPS", VBI_Flags & VBI_VPS, szIniFile);
	WritePrivateProfileInt("VBI", "CC", VBI_Flags & VBI_CC, szIniFile);

	WritePrivateProfileString("Files", "DebugLogFilename", DebugLogFilename, szIniFile);
	WritePrivateProfileInt("Files", "DebugLogEnabled", DebugLogEnabled, szIniFile);

	WritePrivateProfileInt("Show", "CaptureVBI", Capture_VBI, szIniFile);
	WritePrivateProfileInt("Show", "LastProgram", CurrentProgramm, szIniFile);

	WritePrivateProfileInt("Sound", "AudioSource", AudioSource, szIniFile);
	WritePrivateProfileInt("Sound", "System_In_Mute", System_In_Mute, szIniFile);	

	WritePrivateProfileInt("Show", "SaveSettings", bSaveSettings, szIniFile);
	WritePrivateProfileInt("Show", "CountryCode", CountryCode, szIniFile);

	for(i = 0; i < 9; i++)
	{
		sprintf(szKey, "Color%d", i + 1);
		WritePrivateProfileInt("VT", szKey, VTColourTable[i], szIniFile);
	}
	
	WritePrivateProfileInt("MSP", "MSPMode", MSPMode, szIniFile);	
	WritePrivateProfileInt("MSP", "MSPMajorMode", MSPMajorMode, szIniFile);	
	WritePrivateProfileInt("MSP", "MSPMinorMode", MSPMinorMode, szIniFile);	
	WritePrivateProfileInt("MSP", "MSPStereo", MSPStereo, szIniFile);	
	WritePrivateProfileInt("MSP", "MSPAutoStereo", AutoStereoSelect, szIniFile);	
	WritePrivateProfileInt("MSP", "Volume", InitialVolume, szIniFile);	
	WritePrivateProfileInt("MSP", "Spatial", InitialSpatial, szIniFile);	
	WritePrivateProfileInt("MSP", "Loudness", InitialLoudness, szIniFile);	
	WritePrivateProfileInt("MSP", "Bass", InitialBass, szIniFile);	
	WritePrivateProfileInt("MSP", "Treble", InitialTreble, szIniFile);	
	WritePrivateProfileInt("MSP", "Balance", InitialBalance, szIniFile);	
	WritePrivateProfileInt("MSP", "SuperBass", InitialSuperBass, szIniFile);	

	for(i = 0; i < 5; i++)
	{
		sprintf(szKey, "Equalizer%d", i + 1);
		WritePrivateProfileInt("MSP", szKey, InitialEqualizer[i], szIniFile);
	}

	WritePrivateProfileInt("Mixer", "UseMixer", USE_MIXER, szIniFile);	
	WritePrivateProfileInt("Mixer", "VolLeftChannel", MIXER_LINKER_KANAL, szIniFile);
	WritePrivateProfileInt("Mixer", "VolRightChannel", MIXER_RECHTER_KANAL, szIniFile); 

	WritePrivateProfileInt("Mixer", "VolumeSoundSystem", Volume.SoundSystem, szIniFile);
	WritePrivateProfileInt("Mixer", "VolumeDestination", Volume.Destination, szIniFile);
	WritePrivateProfileInt("Mixer", "VolumeConnection", Volume.Connection, szIniFile);
	WritePrivateProfileInt("Mixer", "VolumeControl", Volume.Control, szIniFile);

	WritePrivateProfileInt("Mixer", "MuteSoundSystem", Mute.SoundSystem, szIniFile);
	WritePrivateProfileInt("Mixer", "MuteDestination", Mute.Destination, szIniFile);
	WritePrivateProfileInt("Mixer", "MuteConnection", Mute.Connection, szIniFile);
	WritePrivateProfileInt("Mixer", "MuteControl", Mute.Control, szIniFile);

	for(i = 0; i < 64; i++)
	{
		sprintf(szKey, "MixerSettings%d", i + 1);
		if(MixerLoad[i].MixerAccess.SoundSystem != -1)
		{
			WritePrivateProfileInt(szKey, "SoundSystem", MixerLoad[i].MixerAccess.SoundSystem, szIniFile);
			WritePrivateProfileInt(szKey, "Destination", MixerLoad[i].MixerAccess.Destination, szIniFile);
			WritePrivateProfileInt(szKey, "Connection", MixerLoad[i].MixerAccess.Connection, szIniFile);
			WritePrivateProfileInt(szKey, "Control", MixerLoad[i].MixerAccess.Control, szIniFile);
			WritePrivateProfileInt(szKey, "Channel1", MixerLoad[i].MixerValues.Kanal1, szIniFile);
			WritePrivateProfileInt(szKey, "Channel2", MixerLoad[i].MixerValues.Kanal2, szIniFile);
			WritePrivateProfileInt(szKey, "Channel3", MixerLoad[i].MixerValues.Kanal3, szIniFile);
			WritePrivateProfileInt(szKey, "Channel4", MixerLoad[i].MixerValues.Kanal4, szIniFile);
		}
	}
}

void WritePrivateProfileInt(LPCTSTR lpAppName,  LPCTSTR lpKeyName,  int nValue, LPCTSTR lpFileName)
{
	char szValue[128];
	sprintf(szValue, "%d", nValue);
	WritePrivateProfileString(lpAppName,  lpKeyName,  szValue, lpFileName);
}

// Start of new UI code
// Not to be used yet

void SetControlVisibility(HWND hDlg, int ControlID, BOOL IsVisible)
{
	if(!IsVisible)
	{
		ShowWindow(GetDlgItem(hDlg, ControlID), SW_HIDE);
	}
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
		return (BOOL) *pSetting->pValue;
		break;
	case ITEMFROMLIST:
	case SLIDER:
	case NUMBER:
		return *pSetting->pValue;
		break;
	default:
		return 0;
	}
}

BOOL Setting_SetValue(SETTING* pSetting, long Value)
{
	long NewValue;
	if(pSetting == NULL)
	{
		return FALSE;
	}

	switch(pSetting->Type)
	{
	case YESNO:
		 NewValue = (Value != 0);
		break;
	case ITEMFROMLIST:
	case SLIDER:
	case NUMBER:
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
	
	if(pSetting->pfnOnChange != NULL)
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
	Slider_SetRangeMax(hSlider, pSetting->MaxValue);
	Slider_SetRangeMin(hSlider, pSetting->MinValue);
	Slider_SetPageSize(hSlider, pSetting->StepValue);
	Slider_SetLineSize(hSlider, 1);
	Slider_SetTic(hSlider, pSetting->Default);
	Setting_SetControlValue(pSetting, hSlider);
}

void Setting_SetControlValue(SETTING* pSetting, HWND hControl)
{
	char szBuffer[15];

	switch(pSetting->Type)
	{
	case YESNO:
		Button_SetCheck(hControl, *pSetting->pValue);
		break;

	case ITEMFROMLIST:
		ComboBox_SetCurSel(hControl, *pSetting->pValue);
		break;

	case SLIDER:
		Slider_SetPos(hControl, *pSetting->pValue);
		break;
	
	case NUMBER:
		Edit_SetText(hControl, _itoa(*pSetting->pValue, szBuffer, 10));
		break;
	default:
		break;
	}
}

BOOL Setting_SetFromControl(SETTING* pSetting, HWND hControl)
{
	long nValue;
	char szBuffer[15];

	switch(pSetting->Type)
	{
	case YESNO:
		nValue = (Button_GetCheck(hControl) != 0);
		break;

	case ITEMFROMLIST:
		nValue = ComboBox_GetCurSel(hControl);
		break;

	case SLIDER:
		nValue = Slider_GetPos(hControl);
		break;
	
	case NUMBER:
		Edit_GetText(hControl, szBuffer, 15);
		nValue = atoi(szBuffer);
		break;
	default:
		break;
	}
	return Setting_SetValue(pSetting, nValue);
}

HWND Setting_CreateControl(SETTING* pSetting, HWND hDlg, int* VertPos)
{
	HWND hNewControl = NULL;
	
	switch(pSetting->Type)
	{
	case YESNO:
		break;
	case ITEMFROMLIST:
		break;
	case SLIDER:
		break;
	case NUMBER:
		break;
	default:
		break;
	}
	*VertPos += 20;
	return hNewControl;
}

void Setting_SetupControl(SETTING* pSetting, HWND hControl)
{
	int i;

	switch(pSetting->Type)
	{
	case ITEMFROMLIST:
		i = 0;
		while(pSetting->pszList[i] != NULL)
		{
			ComboBox_InsertString(hControl, -1, pSetting->pszList[i]);
		}
		break;

	case SLIDER:
		Setting_SetupSlider(pSetting, hControl);
		break;
	default:
		break;
	}
	Setting_SetControlValue(pSetting, hControl);
}

void Setting_ReadFromIni(SETTING* pSetting)
{
	long nValue;

	if(pSetting->szIniSection != NULL)
	{
		nValue = GetPrivateProfileInt(pSetting->szIniSection, pSetting->szIniEntry, pSetting->MinValue - 100, szIniFile);
		if(nValue < pSetting->MinValue)
		{
			nValue = pSetting->Default;
		}
		*pSetting->pValue = nValue;
		pSetting->OriginalValue = *pSetting->pValue;
	}
}

void Setting_WriteToIni(SETTING* pSetting)
{
	if(pSetting->szIniSection != NULL)
	{
//		if(pSetting->OriginalValue != *pSetting->pValue)
		{
//			if(pSetting->Default != *pSetting->pValue)
			{
				WritePrivateProfileInt(pSetting->szIniSection, pSetting->szIniEntry, *pSetting->pValue, szIniFile);
			}
//			else
//			{
//				WritePrivateProfileInt(pSetting->szIniSection, pSetting->szIniEntry, pSetting->MinValue - 100, szIniFile);
//			}
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
	case SLIDER:
	case NUMBER:
		sprintf(szBuffer, "%s %d", pSetting->szDisplayName, *(pSetting->pValue));
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
int GetCurrentAdjustmentStepCount()
{
    static DWORD        dwLastTick = 0;
    static DWORD        dwFirstTick = 0;
    static DWORD        dwTaps = 0;
    DWORD               dwTick;
    DWORD               dwElapsedSinceLastCall;
    DWORD               dwElapsedSinceFirstTick;
    int                 nStepCount;

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
        nStep = GetCurrentAdjustmentStepCount();
		Setting_SetValue(pSetting, *pSetting->pValue + nStep);
    }
}

void Setting_Down(SETTING* pSetting)
{
    int nStep = 0;

    if (*pSetting->pValue > pSetting->MinValue)
    {
        nStep = GetCurrentAdjustmentStepCount();
		Setting_SetValue(pSetting, *pSetting->pValue - nStep);
    }
}

void Setting_ChangeValue(SETTING* pSetting, eCHANGEVALUE NewValue)
{
	if(pSetting == NULL)
	{
		return;
	}
	switch(NewValue)
	{
	case DISPLAYVALUE:
		Setting_OSDShow(pSetting, hWnd);
		break;
	case INCREMENTVALUE:
		Setting_Up(pSetting);
		Setting_OSDShow(pSetting, hWnd);
		break;
	case DECREMENTVALUE:
		Setting_Down(pSetting);
		Setting_OSDShow(pSetting, hWnd);
		break;
	case RESETVALUE:
		Setting_SetDefault(pSetting);
		Setting_OSDShow(pSetting, hWnd);
		break;
	case TOGGLEBOOL:
		if(pSetting->Type == YESNO)
		{
			Setting_SetValue(pSetting, !Setting_GetValue(pSetting));
			Setting_OSDShow(pSetting, hWnd);
		}
	case INCREMENTVALUE_SILENT:
		Setting_Up(pSetting);
		break;
	case DECREMENTVALUE_SILENT:
		Setting_Down(pSetting);
		break;
	case RESETVALUE_SILENT:
		Setting_SetDefault(pSetting);
		break;
	case TOGGLEBOOL_SILENT:
		if(pSetting->Type == YESNO)
		{
			Setting_SetValue(pSetting, !Setting_GetValue(pSetting));
		}
	default:
		break;
	}
}

BOOL APIENTRY UISubMenuProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	static UI_SUBMENU* pSubMenu;
	int i;
	HWND Controls[8];
	long PrevValues[8];
	int VertPos = 0;

	switch (message)
	{
	case WM_INITDIALOG:
		pSubMenu = (UI_SUBMENU*)lParam;
		SetWindowText(hDlg, pSubMenu->szDisplayName);
		for(i = 0;i < 8;i++)
		{
			Controls[i] = Setting_CreateControl(pSubMenu->Elements[i], hDlg, &VertPos);
			Setting_SetupControl(pSubMenu->Elements[i], Controls[i]);
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC1 + i), pSubMenu->Elements[i]->szDisplayName);
			PrevValues[i] = Setting_GetValue(pSubMenu->Elements[i]);
		}
		break;

	case WM_COMMAND:
		switch LOWORD(wParam)
		{
		case IDOK:
			EndDialog(hDlg, TRUE);
			break;

		case IDCANCEL:
			// revert to old value
			for(i = 0;i < 8 && pSubMenu->Elements[i] != NULL;i++)
			{
				Setting_SetValue(pSubMenu->Elements[i], PrevValues[i]);
			}
			EndDialog(hDlg, FALSE);
			break;

		case IDDEFAULTS:
			// revert to default values
			for(i = 0;i < 8 && pSubMenu->Elements[i] != NULL;i++)
			{
				*pSubMenu->Elements[i]->pValue = pSubMenu->Elements[i]->Default;
				Setting_SetControlValue(pSubMenu->Elements[i], Controls[i]);
			}
			break;

		case IDC_CHECK1:
		case IDC_CHECK2:
		case IDC_CHECK3:
		case IDC_CHECK4:
		case IDC_CHECK5:
		case IDC_CHECK6:
		case IDC_CHECK7:
		case IDC_CHECK8:
			i = LOWORD(wParam) -  IDC_CHECK1;
			Setting_SetFromControl(pSubMenu->Elements[i], Controls[i]);
			break;
		case WM_VSCROLL:
		case WM_HSCROLL:
			for(i = 0; i < 8; i++)
			{
				if((HWND)lParam == Controls[i])
				{
					Setting_SetFromControl(pSubMenu->Elements[i], Controls[i]);
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
	return FALSE;
}

void DisplayUISubMenuAsDialog(UI_SUBMENU* pSubMenu)
{
	DialogBoxParam(hInst, "UI_SUB_MENU", hWnd, UISubMenuProc, (LPARAM)pSubMenu);
}

