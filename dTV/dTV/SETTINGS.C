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
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "settings.h"
#include "tuner.h"
#include "audio.h"
#include "bt848.h"
#include "vbi.h"
#include "bTVPlugin.h"
#include "OutThreads.h"
#include "deinterlace.h"
#include "AspectRatio.h"
#include "DebugLog.h"

// MRS 9-2-00
// Added variable in dTV.c to track which aspect mode we are currently in
// Use aspect * 1000 (1.66 = 1660, 2.35 = 2350, etc)
// Declared in DTV.C
extern int source_aspect, target_aspect, aspect_mode, custom_source_aspect, custom_target_aspect;
// END MRS 9-2-00


void LoadSettingsFromIni(LPSTR Name)
{
	char szIniFile[MAX_PATH];
	char szKey[128];
	int i;

	GetCurrentDirectory(MAX_PATH, szIniFile);
	strcat(szIniFile, "\\dTV.ini");

	emstartx = GetPrivateProfileInt("MainWindow", "StartLeft", 10, szIniFile);
	emstarty = GetPrivateProfileInt("MainWindow", "StartTop", 10, szIniFile);
	emsizex = GetPrivateProfileInt("MainWindow", "StartWidth", 754, szIniFile);
	emsizey = GetPrivateProfileInt("MainWindow", "StartHeight", 521, szIniFile);
	bAlwaysOnTop = (GetPrivateProfileInt("MainWindow", "AlwaysOnTop", 0, szIniFile) != 0);

	pgstartx = GetPrivateProfileInt("ProgList", "StartLeft", -1, szIniFile);
	pgstarty = GetPrivateProfileInt("ProgList", "StartTop", -1, szIniFile);
	pgsizex = GetPrivateProfileInt("ProgList", "StartWidth", -1, szIniFile);
	pgsizey = GetPrivateProfileInt("ProgList", "StartHeight", -1, szIniFile);
	for(i = 0; i < 15; i++)
	{
		sprintf(szKey, "LISTFIELD_%d_ID", i + 1);
		ButtonList[i].FeldId = GetPrivateProfileInt("ProgList", szKey, 0, szIniFile);
		sprintf(szKey, "LISTFIELD_%d_LENGTH", i + 1);
		ButtonList[i].x = GetPrivateProfileInt("ProgList", szKey, 0, szIniFile);
	}

	PriorClassId = GetPrivateProfileInt("Threads", "ProcessPriority", 0, szIniFile);
	ThreadClassId = GetPrivateProfileInt("Threads", "ThreadPriority", 1, szIniFile);
	MainProcessor = GetPrivateProfileInt("Threads", "WindowProcessor", 0, szIniFile);
	DecodeProcessor = GetPrivateProfileInt("Threads", "DecodeProcessor", 0, szIniFile);

	bDisplayStatusBar = (GetPrivateProfileInt("Show", "StatusBar", 1, szIniFile) != 0);
	Show_Menu = (GetPrivateProfileInt("Show", "Menu", 1, szIniFile) != 0);

	PulldownThresholdLow = GetPrivateProfileInt("Pulldown", "PulldownThresholdLow", PulldownThresholdLow, szIniFile);
	PulldownThresholdHigh = GetPrivateProfileInt("Pulldown", "PulldownThresholdHigh", PulldownThresholdHigh, szIniFile);
	PulldownRepeatCount = GetPrivateProfileInt("Pulldown", "PulldownRepeatCount", PulldownRepeatCount, szIniFile);
	PulldownRepeatCount2 = GetPrivateProfileInt("Pulldown", "PulldownRepeatCount2", PulldownRepeatCount2, szIniFile);
	Threshold32Pulldown  = GetPrivateProfileInt("Pulldown", "Threshold32Pulldown", Threshold32Pulldown, szIniFile);
	ThresholdPulldownMismatch  = GetPrivateProfileInt("Pulldown", "ThresholdPulldownMismatch", ThresholdPulldownMismatch, szIniFile);
	ThresholdPulldownComb  = GetPrivateProfileInt("Pulldown", "ThresholdPulldownComb", ThresholdPulldownComb, szIniFile);
	bAutoDetectMode = (GetPrivateProfileInt("Pulldown", "bAutoDetectMode", bAutoDetectMode, szIniFile) != 0);
	bFallbackToVideo = (GetPrivateProfileInt("Pulldown", "bFallbackToVideo", bFallbackToVideo, szIniFile) != 0);
	BitShift = GetPrivateProfileInt("Pulldown", "BitShift", BitShift, szIniFile);
	DiffThreshold = GetPrivateProfileInt("Pulldown", "DiffThreshold", DiffThreshold, szIniFile);
	PulldownSwitchInterval = GetPrivateProfileInt("Pulldown", "PulldownSwitchInterval", PulldownSwitchInterval, szIniFile);
	PulldownSwitchMax = GetPrivateProfileInt("Pulldown", "PulldownSwitchMax", PulldownSwitchMax, szIniFile);

	EdgeDetect = GetPrivateProfileInt("Deinterlace", "EdgeDetect", EdgeDetect, szIniFile);
	JaggieThreshold = GetPrivateProfileInt("Deinterlace", "JaggieThreshold", JaggieThreshold, szIniFile);

	VBI_Flags = 0;
	if(GetPrivateProfileInt("VBI", "VT", 0, szIniFile) != 0)
	{
		VBI_Flags += VBI_VT;
	}
	if(GetPrivateProfileInt("VBI", "IC", 0, szIniFile) != 0)
	{
		VBI_Flags += VBI_IC;
	}
	if(GetPrivateProfileInt("VBI", "VPS", 0, szIniFile) != 0)
	{
		VBI_Flags += VBI_VPS;
	}
	if(GetPrivateProfileInt("VBI", "VD", 0, szIniFile) != 0)
	{
		VBI_Flags += VBI_VD;
	}
	if(GetPrivateProfileInt("VBI", "CC", 0, szIniFile) != 0)
	{
		VBI_Flags += VBI_CC;
	}

	i = GetCurrentDirectory(sizeof(IC_BASE_DIR), IC_BASE_DIR);
	strcat(IC_BASE_DIR, "\\I_Cast");
	GetPrivateProfileString("Files", "VBIDir", IC_BASE_DIR, IC_BASE_DIR, MAX_PATH, szIniFile);

	i = GetCurrentDirectory(sizeof(VD_DIR), VD_DIR);
	strcat(VD_DIR, "\\VideoDat");
	GetPrivateProfileString("Files", "VDDir", VD_DIR, VD_DIR, MAX_PATH, szIniFile);
	GetPrivateProfileString("Files", "VDFilename", "VD-RAW.Dat", VDat.RawName, MAX_PATH, szIniFile);
	VD_RAW = (GetPrivateProfileInt("Files", "VDRaw", 0, szIniFile) != 0);
	GetPrivateProfileString("Files", "DebugLogFilename", DebugLogFilename, DebugLogFilename, MAX_PATH, szIniFile);
	DebugLogEnabled = GetPrivateProfileInt("Files", "DebugLogEnabled", DebugLogEnabled, szIniFile);

	CardType = GetPrivateProfileInt("Hardware", "CardType", TVCARD_UNKNOWN, szIniFile);
	VideoSource = GetPrivateProfileInt("Hardware", "VideoSource", 1, szIniFile);
	TunerType = GetPrivateProfileInt("Hardware", "TunerType", TUNER_ABSENT, szIniFile); 
	TVTYPE = GetPrivateProfileInt("Hardware", "TVType", -1, szIniFile); 
	InitialHue = GetPrivateProfileInt("Hardware", "InitialHue", 0, szIniFile); 
	InitialContrast = GetPrivateProfileInt("Hardware", "InitialContrast", 0xd8, szIniFile); 
	InitialBrightness = GetPrivateProfileInt("Hardware", "InitialBrightness", 0, szIniFile); 
	InitialSaturationU = GetPrivateProfileInt("Hardware", "InitialSaturationU", 0xfe, szIniFile); 
	InitialSaturationV = GetPrivateProfileInt("Hardware", "InitialSaturationV", 0xb4, szIniFile); 
	InitialOverscan = GetPrivateProfileInt("Hardware", "InitialOverscan", 4, szIniFile); 

	ManuellAudio[0] = GetPrivateProfileInt("Hardware", "GPIO_OUT_EN", 0, szIniFile); 
	ManuellAudio[1] = GetPrivateProfileInt("Hardware", "GPIO_DATA_TUNER", 0, szIniFile);  
	ManuellAudio[2] = GetPrivateProfileInt("Hardware", "GPIO_DATA_RADIO", 0, szIniFile);  
	ManuellAudio[3] = GetPrivateProfileInt("Hardware", "GPIO_DATA_EXTERN", 0, szIniFile);  
	ManuellAudio[4] = GetPrivateProfileInt("Hardware", "GPIO_DATA_INTERN", 0, szIniFile);  
	ManuellAudio[5] = GetPrivateProfileInt("Hardware", "GPIO_DATA_OUT", 0, szIniFile);  
	ManuellAudio[6] = GetPrivateProfileInt("Hardware", "GPIO_DATA_IN", 0, szIniFile);  
	ManuellAudio[7] = GetPrivateProfileInt("Hardware", "GPIO_REG_INP", 0, szIniFile);  

	//Capture_VBI = (GetPrivateProfileInt("Show", "CaptureVBI", 0, szIniFile) != 0);  
	InitialProg = GetPrivateProfileInt("Show", "LastProgram", 0, szIniFile);

	LNB.Diseq = (GetPrivateProfileInt("Sound", "DiscEqu", 0, szIniFile) != 0);  

	for(i = 0; i < 4; i++)
	{
		sprintf(szKey, "LNB%d_MinFreq", i + 1);
		LNB.Anschluss[i].MinFreq = GetPrivateProfileInt("Sound", szKey, LNB.Anschluss[i].MinFreq, szIniFile);
		sprintf(szKey, "LNB%d_MaxFreq", i + 1);
		LNB.Anschluss[i].MaxFreq = GetPrivateProfileInt("Sound", szKey, LNB.Anschluss[i].MaxFreq, szIniFile);
		sprintf(szKey, "LNB%d_LofLow", i + 1);
		LNB.Anschluss[i].LofLow = GetPrivateProfileInt("Sound", szKey, LNB.Anschluss[i].LofLow, szIniFile);
		sprintf(szKey, "LNB%d_LofHigh", i + 1);
		LNB.Anschluss[i].LofHigh = GetPrivateProfileInt("Sound", szKey, LNB.Anschluss[i].LofHigh, szIniFile);
		sprintf(szKey, "LNB%d_SwitchFreq", i + 1);
		LNB.Anschluss[i].SwitchFreq = GetPrivateProfileInt("Sound", szKey, LNB.Anschluss[i].SwitchFreq, szIniFile);
		sprintf(szKey, "LNB%d_Power", i + 1);
		LNB.Anschluss[i].Power = (GetPrivateProfileInt("Sound", szKey, LNB.Anschluss[i].Power, szIniFile) != 0);
		sprintf(szKey, "LNB%d_Switch22khz", i + 1);
		LNB.Anschluss[i].Switch22khz = (GetPrivateProfileInt("Sound", szKey, LNB.Anschluss[i].Switch22khz, szIniFile) != 0);
		sprintf(szKey, "LNB%d_SwitchFreq", i + 1);
		LNB.Anschluss[i].SwitchFreq = GetPrivateProfileInt("Sound", szKey, LNB.Anschluss[i].SwitchFreq, szIniFile);
		sprintf(szKey, "LNB%d_Use", i + 1);
		LNB.Anschluss[i].Use = (GetPrivateProfileInt("Sound", szKey, LNB.Anschluss[i].Use, szIniFile) != 0);
	}
	
	bSaveSettings = (GetPrivateProfileInt("Show", "SaveSettings", 1, szIniFile) != 0);
	CountryCode = GetPrivateProfileInt("Show", "CountryCode", 1, szIniFile);

	i = GetCurrentDirectory(sizeof(VT_BASE_DIR), VT_BASE_DIR);
	strcat(VT_BASE_DIR, "\\VideoText");
	GetPrivateProfileString("VT", "VTDir", VT_BASE_DIR, VT_BASE_DIR, MAX_PATH, szIniFile);
	VT_ALWAYS_EXPORT = (GetPrivateProfileInt("VT", "VTAlwayExport", 0, szIniFile) != 0);
	VT_NL = (GetPrivateProfileInt("VT", "VT_NL", 0, szIniFile) != 0);
	VT_HEADERS = (GetPrivateProfileInt("VT", "VT_Headers", 1, szIniFile) != 0);
	VT_STRIPPED = (GetPrivateProfileInt("VT", "VT_Stripped", 1, szIniFile) != 0);
	VT_REVEAL = (GetPrivateProfileInt("VT", "VT_Reveal", 0, szIniFile) != 0);

	for(i = 0; i < 9; i++)
	{
		sprintf(szKey, "Color%d", i + 1);
		VTColourTable[i] = GetPrivateProfileInt("VT", szKey, VTColourTable[i], szIniFile);
	}
	
	MSPMode = GetPrivateProfileInt("MSP", "MSPMode", 3, szIniFile);	
	MSPMajorMode = GetPrivateProfileInt("MSP", "MSPMajorMode", 0, szIniFile);	
	MSPMinorMode = GetPrivateProfileInt("MSP", "MSPMinorMode", 0, szIniFile);	
	MSPStereo = GetPrivateProfileInt("MSP", "MSPStereo", 0, szIniFile);	
	AutoStereoSelect = (GetPrivateProfileInt("MSP", "MSPAutoStereo", 1, szIniFile) != 0);	
	InitialVolume = GetPrivateProfileInt("MSP", "Volume", 1000, szIniFile);	
	InitialSpatial = GetPrivateProfileInt("MSP", "Spatial", 0, szIniFile);	
	InitialLoudness = GetPrivateProfileInt("MSP", "Loudness", 0, szIniFile);	
	InitialBass = GetPrivateProfileInt("MSP", "Bass", 0, szIniFile);	
	InitialTreble = GetPrivateProfileInt("MSP", "Treble", 0, szIniFile);	
	InitialBalance = GetPrivateProfileInt("MSP", "Balance", 0, szIniFile);	
	InitialSuperBass = (GetPrivateProfileInt("MSP", "SuperBass", 0, szIniFile) != 0);	

	for(i = 0; i < 5; i++)
	{
		sprintf(szKey, "Equalizer%d", i + 1);
		InitialEqualizer[i] = GetPrivateProfileInt("MSP", szKey, 0, szIniFile);
	}

	USE_MIXER = (GetPrivateProfileInt("Mixer", "UseMixer", 0, szIniFile) != 0);	
	MIXER_LINKER_KANAL = GetPrivateProfileInt("Mixer", "VolLeftChannel", -1, szIniFile);
	MIXER_RECHTER_KANAL = GetPrivateProfileInt("Mixer", "VolRightChannel", -1, szIniFile); 

	Volume.SoundSystem = GetPrivateProfileInt("Mixer", "VolumeSoundSystem", -1, szIniFile);
	Volume.Destination = GetPrivateProfileInt("Mixer", "VolumeDestination", 0, szIniFile);
	Volume.Connection = GetPrivateProfileInt("Mixer", "VolumeConnection", 0, szIniFile);
	Volume.Control = GetPrivateProfileInt("Mixer", "VolumeControl", 0, szIniFile);

	Mute.SoundSystem = GetPrivateProfileInt("Mixer", "MuteSoundSystem", -1, szIniFile);
	Mute.Destination = GetPrivateProfileInt("Mixer", "MuteDestination", 0, szIniFile);
	Mute.Connection = GetPrivateProfileInt("Mixer", "MuteConnection", 0, szIniFile);
	Mute.Control = GetPrivateProfileInt("Mixer", "MuteControl", 0, szIniFile);

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

	GetPrivateProfileString("BTVPlugin", "Name", "bDeinterlace.dll", szBTVPluginName, MAX_PATH, szIniFile);
	BTVParams.Level1 = GetPrivateProfileInt("BTVPlugin", "Level1", BTVParams.Level1, szIniFile);
	BTVParams.Level2 = GetPrivateProfileInt("BTVPlugin", "Level2", BTVParams.Level2, szIniFile);
	BTVParams.x = GetPrivateProfileInt("BTVPlugin", "x", BTVParams.x, szIniFile);
	BTVParams.y = GetPrivateProfileInt("BTVPlugin", "y", BTVParams.y, szIniFile);
	BTVParams.UseKey1 = GetPrivateProfileInt("BTVPlugin", "UseKey1", BTVParams.UseKey1, szIniFile);
	BTVParams.UseKey2 = GetPrivateProfileInt("BTVPlugin", "UseKey2", BTVParams.UseKey2, szIniFile);

	bIsFullScreen = (GetPrivateProfileInt("MainWindow", "bIsFullScreen", bIsFullScreen, szIniFile) != 0);

	// MRS 9/2/00
	// Load Aspect Mode from INI- using strings would be more elegant long-term
	source_aspect = GetPrivateProfileInt("ASPECT", "SourceAspect", 0, szIniFile);
	custom_source_aspect = GetPrivateProfileInt("ASPECT", "CustomSourceAspect", 0, szIniFile);
	target_aspect = GetPrivateProfileInt("ASPECT", "ScreenAspect", 0, szIniFile);
	custom_target_aspect = GetPrivateProfileInt("ASPECT", "CustomScreenAspect", 0, szIniFile);
	aspect_mode = GetPrivateProfileInt("ASPECT", "Mode", 0, szIniFile);
	// END MRS 9/2/00
	LuminanceThreshold = GetPrivateProfileInt("ASPECT", "LuminanceThreshold", LuminanceThreshold, szIniFile);
	IgnoreNonBlackPixels = GetPrivateProfileInt("ASPECT", "IgnoreNonBlackPixels", IgnoreNonBlackPixels, szIniFile);
	AutoDetectAspect = GetPrivateProfileInt("ASPECT", "AutoDetectAspect", AutoDetectAspect, szIniFile);
	ZoomInFrameCount = GetPrivateProfileInt("ASPECT", "ZoomInFrameCount", ZoomInFrameCount, szIniFile);
}

void WriteSettingsToIni()
{
	char szIniFile[MAX_PATH];
	char szKey[128];
	int i;

	GetCurrentDirectory(MAX_PATH, szIniFile);
	strcat(szIniFile, "\\dTV.ini");

	WritePrivateProfileInt("MainWindow", "bIsFullScreen", bIsFullScreen, szIniFile);
	if(!bIsFullScreen)
	{
		WritePrivateProfileInt("MainWindow", "StartLeft", emstartx, szIniFile);
		WritePrivateProfileInt("MainWindow", "StartTop", emstarty, szIniFile);
		WritePrivateProfileInt("MainWindow", "StartWidth", emsizex, szIniFile);
		WritePrivateProfileInt("MainWindow", "StartHeight", emsizey, szIniFile);
	}
	WritePrivateProfileInt("MainWindow", "AlwaysOnTop", bAlwaysOnTop, szIniFile);

	WritePrivateProfileInt("ProgList", "StartLeft", pgstartx, szIniFile);
	WritePrivateProfileInt("ProgList", "StartTop", pgstarty, szIniFile);
	WritePrivateProfileInt("ProgList", "StartWidth", pgsizex, szIniFile);
	WritePrivateProfileInt("ProgList", "StartHeight", pgsizey, szIniFile);
	for(i = 0; i < 15; i++)
	{
		sprintf(szKey, "LISTFIELD_%d_ID", i + 1);
		WritePrivateProfileInt("ProgList", szKey, ButtonList[i].FeldId, szIniFile);
		sprintf(szKey, "LISTFIELD_%d_LENGTH", i + 1);
		WritePrivateProfileInt("ProgList", szKey, ButtonList[i].x, szIniFile);
	}

	WritePrivateProfileInt("Threads", "ProcessPriority", PriorClassId, szIniFile);
	WritePrivateProfileInt("Threads", "ThreadPriority", ThreadClassId, szIniFile);
	WritePrivateProfileInt("Threads", "WindowProcessor", MainProcessor, szIniFile);
	WritePrivateProfileInt("Threads", "DecodeProcessor", DecodeProcessor, szIniFile);

	WritePrivateProfileInt("Pulldown", "PulldownThresholdLow", PulldownThresholdLow, szIniFile);
	WritePrivateProfileInt("Pulldown", "PulldownThresholdHigh", PulldownThresholdHigh, szIniFile);
	WritePrivateProfileInt("Pulldown", "PulldownRepeatCount", PulldownRepeatCount, szIniFile);
	WritePrivateProfileInt("Pulldown", "PulldownRepeatCount2", PulldownRepeatCount2, szIniFile);
	WritePrivateProfileInt("Pulldown", "Threshold32Pulldown", Threshold32Pulldown, szIniFile);
	WritePrivateProfileInt("Pulldown", "ThresholdPulldownMismatch", ThresholdPulldownMismatch, szIniFile);
	WritePrivateProfileInt("Pulldown", "ThresholdPulldownComb", ThresholdPulldownComb, szIniFile);
	WritePrivateProfileInt("Pulldown", "bAutoDetectMode", bAutoDetectMode, szIniFile);
	WritePrivateProfileInt("Pulldown", "bFallbackToVideo", bFallbackToVideo, szIniFile);
	WritePrivateProfileInt("Pulldown", "BitShift", BitShift, szIniFile);
	WritePrivateProfileInt("Pulldown", "DiffThreshold", DiffThreshold, szIniFile);
	WritePrivateProfileInt("Pulldown", "PulldownSwitchInterval", PulldownSwitchInterval, szIniFile);
	WritePrivateProfileInt("Pulldown", "PulldownSwitchMax", PulldownSwitchMax, szIniFile);

	WritePrivateProfileInt("Deinterlace", "EdgeDetect", EdgeDetect, szIniFile);
	WritePrivateProfileInt("Deinterlace", "JaggieThreshold", JaggieThreshold, szIniFile);


	WritePrivateProfileInt("Show", "StatusBar", bDisplayStatusBar, szIniFile);
	WritePrivateProfileInt("Show", "Menu", Show_Menu, szIniFile);

	WritePrivateProfileInt("VBI", "VT", VBI_Flags & VBI_VT, szIniFile);
	WritePrivateProfileInt("VBI", "IC", VBI_Flags & VBI_IC, szIniFile);
	WritePrivateProfileInt("VBI", "VPS", VBI_Flags & VBI_VPS, szIniFile);
	WritePrivateProfileInt("VBI", "VD", VBI_Flags & VBI_VD, szIniFile);
	WritePrivateProfileInt("VBI", "CC", VBI_Flags & VBI_CC, szIniFile);

	WritePrivateProfileString("Files", "VBIDir", IC_BASE_DIR, szIniFile);
	WritePrivateProfileString("Files", "VDDir", VD_DIR, szIniFile);
	WritePrivateProfileString("Files", "VDFilename", VDat.RawName, szIniFile);
	WritePrivateProfileInt("Files", "VDRaw", VD_RAW, szIniFile);
	WritePrivateProfileString("Files", "DebugLogFilename", DebugLogFilename, szIniFile);
	WritePrivateProfileInt("Files", "DebugLogEnabled", DebugLogEnabled, szIniFile);

	WritePrivateProfileInt("Hardware", "CardType", CardType, szIniFile);
	WritePrivateProfileInt("Hardware", "VideoSource", VideoSource, szIniFile);
	WritePrivateProfileInt("Hardware", "TunerType", TunerType, szIniFile); 
	WritePrivateProfileInt("Hardware", "TVType", TVTYPE, szIniFile); 
	WritePrivateProfileInt("Hardware", "InitialHue", InitialHue, szIniFile); 
	WritePrivateProfileInt("Hardware", "InitialContrast", InitialContrast, szIniFile); 
	WritePrivateProfileInt("Hardware", "InitialBrightness", InitialBrightness, szIniFile); 
	WritePrivateProfileInt("Hardware", "InitialSaturationU", InitialSaturationU, szIniFile); 
	WritePrivateProfileInt("Hardware", "InitialSaturationV", InitialSaturationV, szIniFile); 
	WritePrivateProfileInt("Hardware", "InitialOverscan", InitialOverscan, szIniFile); 

	WritePrivateProfileInt("Hardware", "GPIO_OUT_EN", ManuellAudio[0], szIniFile); 
	WritePrivateProfileInt("Hardware", "GPIO_DATA_TUNER", ManuellAudio[1], szIniFile);  
	WritePrivateProfileInt("Hardware", "GPIO_DATA_RADIO", ManuellAudio[2], szIniFile);  
	WritePrivateProfileInt("Hardware", "GPIO_DATA_EXTERN", ManuellAudio[3], szIniFile);  
	WritePrivateProfileInt("Hardware", "GPIO_DATA_INTERN", ManuellAudio[4], szIniFile);  
	WritePrivateProfileInt("Hardware", "GPIO_DATA_OUT", ManuellAudio[5], szIniFile);  
	WritePrivateProfileInt("Hardware", "GPIO_DATA_IN", ManuellAudio[6], szIniFile);  
	WritePrivateProfileInt("Hardware", "GPIO_REG_INP", ManuellAudio[7], szIniFile);  

	WritePrivateProfileInt("Show", "CaptureVBI", Capture_VBI, szIniFile);
	WritePrivateProfileInt("Show", "LastProgram", InitialProg, szIniFile);

	WritePrivateProfileInt("Sound", "DiscEqu", LNB.Diseq, szIniFile);

	for(i = 0; i < 4; i++)
	{
		sprintf(szKey, "LNB%d_MinFreq", i + 1);
		WritePrivateProfileInt("Sound", szKey, LNB.Anschluss[i].MinFreq, szIniFile);
		sprintf(szKey, "LNB%d_MaxFreq", i + 1);
		WritePrivateProfileInt("Sound", szKey, LNB.Anschluss[i].MaxFreq, szIniFile);
		sprintf(szKey, "LNB%d_LofLow", i + 1);
		WritePrivateProfileInt("Sound", szKey, LNB.Anschluss[i].LofLow, szIniFile);
		sprintf(szKey, "LNB%d_LofHigh", i + 1);
		WritePrivateProfileInt("Sound", szKey, LNB.Anschluss[i].LofHigh, szIniFile);
		sprintf(szKey, "LNB%d_SwitchFreq", i + 1);
		WritePrivateProfileInt("Sound", szKey, LNB.Anschluss[i].SwitchFreq, szIniFile);
		sprintf(szKey, "LNB%d_Power", i + 1);
		WritePrivateProfileInt("Sound", szKey, LNB.Anschluss[i].Power, szIniFile);
		sprintf(szKey, "LNB%d_Switch22khz", i + 1);
		WritePrivateProfileInt("Sound", szKey, LNB.Anschluss[i].Switch22khz, szIniFile);
		sprintf(szKey, "LNB%d_SwitchFreq", i + 1);
		WritePrivateProfileInt("Sound", szKey, LNB.Anschluss[i].SwitchFreq, szIniFile);
		sprintf(szKey, "LNB%d_Use", i + 1);
		WritePrivateProfileInt("Sound", szKey, LNB.Anschluss[i].Use, szIniFile);
	}
	
	WritePrivateProfileInt("Show", "SaveSettings", bSaveSettings, szIniFile);
	WritePrivateProfileInt("Show", "CountryCode", CountryCode, szIniFile);

	WritePrivateProfileString("VT", "VTDir", VT_BASE_DIR, szIniFile);
	WritePrivateProfileInt("VT", "VTAlwayExport", VT_ALWAYS_EXPORT, szIniFile);
	WritePrivateProfileInt("VT", "VT_NL", VT_NL, szIniFile);
	WritePrivateProfileInt("VT", "VT_Headers", VT_HEADERS, szIniFile);
	WritePrivateProfileInt("VT", "VT_Stripped", VT_STRIPPED, szIniFile);
	WritePrivateProfileInt("VT", "VT_Reveal", VT_REVEAL, szIniFile);

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

	WritePrivateProfileString("BTVPlugin", "Name", szBTVPluginName, szIniFile);
	WritePrivateProfileInt("BTVPlugin", "Level1", BTVParams.Level1, szIniFile);
	WritePrivateProfileInt("BTVPlugin", "Level2", BTVParams.Level2, szIniFile);
	WritePrivateProfileInt("BTVPlugin", "x", BTVParams.x, szIniFile);
	WritePrivateProfileInt("BTVPlugin", "y", BTVParams.y, szIniFile);
	WritePrivateProfileInt("BTVPlugin", "UseKey1", BTVParams.UseKey1, szIniFile);
	WritePrivateProfileInt("BTVPlugin", "UseKey2", BTVParams.UseKey2, szIniFile);

	// MRS 9/2/00
	// Save Aspect Mode from INI- using strings would be more elegant long-term
	WritePrivateProfileInt("ASPECT", "SourceAspect", source_aspect, szIniFile);
	WritePrivateProfileInt("ASPECT", "ScreenAspect", target_aspect, szIniFile);
	WritePrivateProfileInt("ASPECT", "Mode", aspect_mode, szIniFile);
	// END MRS 9/2/00
	WritePrivateProfileInt("ASPECT", "LuminanceThreshold", LuminanceThreshold, szIniFile);
	WritePrivateProfileInt("ASPECT", "IgnoreNonBlackPixels", IgnoreNonBlackPixels, szIniFile);
	WritePrivateProfileInt("ASPECT", "AutoDetectAspect", AutoDetectAspect, szIniFile);
	WritePrivateProfileInt("ASPECT", "ZoomInFrameCount", ZoomInFrameCount, szIniFile);
}

void WritePrivateProfileInt(LPCTSTR lpAppName,  LPCTSTR lpKeyName,  int nValue, LPCTSTR lpFileName)
{
	char szValue[128];
	sprintf(szValue, "%d", nValue);
	WritePrivateProfileString(lpAppName,  lpKeyName,  szValue, lpFileName);
}
