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
#include "DI_BlendedClip.h"
#include "other.h"
#include "FD_50Hz.h"
#include "FD_60Hz.h"
#include "FD_Common.h"
#include "slider.h"
#include "Splash.h"
#include "OSD.h"

// MRS 9-2-00
// Added variable in dTV.c to track which aspect mode we are currently in
// Use aspect * 1000 (1.66 = 1660, 2.35 = 2350, etc)
// Declared in DTV.C
extern int source_aspect, target_aspect, aspect_mode, custom_source_aspect, custom_target_aspect;
// END MRS 9-2-00

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

	// Read in settings from each source files read method
	Aspect_ReadSetttingsFromIni();
	BT848_ReadSetttingsFromIni();

	emstartx = GetPrivateProfileInt("MainWindow", "StartLeft", 10, szIniFile);
	emstarty = GetPrivateProfileInt("MainWindow", "StartTop", 10, szIniFile);
	emsizex = GetPrivateProfileInt("MainWindow", "StartWidth", 754, szIniFile);
	emsizey = GetPrivateProfileInt("MainWindow", "StartHeight", 521, szIniFile);

	bAlwaysOnTop = (GetPrivateProfileInt("MainWindow", "AlwaysOnTop", bAlwaysOnTop, szIniFile) != 0);
	bDisplaySplashScreen = (GetPrivateProfileInt("MainWindow", "DisplaySplashScreen", bDisplaySplashScreen, szIniFile) != 0);
	bIsFullScreen = (GetPrivateProfileInt("MainWindow", "bIsFullScreen", bIsFullScreen, szIniFile) != 0);

	// 2000-10-30 Added by Mark Rejhon
	// This is for situations where it is desirable for dTV to always start up in full screen
	// even if dTV was exited during windowed mode on the last time
	if (GetPrivateProfileInt("MainWindow", "AlwaysForceFullScreen", 0, szIniFile) != 0)
	{
		bIsFullScreen = TRUE;
	}

	PriorClassId = GetPrivateProfileInt("Threads", "ProcessPriority", PriorClassId, szIniFile);
	ThreadClassId = GetPrivateProfileInt("Threads", "ThreadPriority", ThreadClassId, szIniFile);
	MainProcessor = GetPrivateProfileInt("Threads", "WindowProcessor", MainProcessor, szIniFile);
	DecodeProcessor = GetPrivateProfileInt("Threads", "DecodeProcessor", DecodeProcessor, szIniFile);

	// Added new performance related parms to Threads group - TRB 10/28/00
	Hurry_When_Late = (GetPrivateProfileInt("Threads", "Hurry_When_Late", Hurry_When_Late, szIniFile) != 0);
	Wait_For_Flip = (GetPrivateProfileInt("Threads", "Wait_For_Flip", Wait_For_Flip, szIniFile) != 0);
	DoAccurateFlips = (GetPrivateProfileInt("Threads", "DoAccurateFlips", DoAccurateFlips, szIniFile) != 0);
	Sleep_Interval = GetPrivateProfileInt("Threads", "Sleep_Interval", Sleep_Interval, szIniFile);

    // Mark Rejhon 01/01/01 - New Overlay section and OverlayColor setting
	Back_Buffers = GetPrivateProfileInt("Overlay", "Back_Buffers", Back_Buffers, szIniFile);
	OverlayColor = GetPrivateProfileInt("Overlay", "OverlayColor", OverlayColor, szIniFile);

	bDisplayStatusBar = (GetPrivateProfileInt("Show", "StatusBar", bDisplayStatusBar, szIniFile) != 0);
	Show_Menu = (GetPrivateProfileInt("Show", "Menu", Show_Menu, szIniFile) != 0);

	PulldownThresholdLow = GetPrivateProfileInt("Pulldown", "PulldownThresholdLow", PulldownThresholdLow, szIniFile);
	PulldownThresholdHigh = GetPrivateProfileInt("Pulldown", "PulldownThresholdHigh", PulldownThresholdHigh, szIniFile);
	PulldownRepeatCount = GetPrivateProfileInt("Pulldown", "PulldownRepeatCount", PulldownRepeatCount, szIniFile);
	PulldownRepeatCount2 = GetPrivateProfileInt("Pulldown", "PulldownRepeatCount2", PulldownRepeatCount2, szIniFile);
	Threshold32Pulldown  = GetPrivateProfileInt("Pulldown", "Threshold32Pulldown", Threshold32Pulldown, szIniFile);
	ThresholdPulldownMismatch  = GetPrivateProfileInt("Pulldown", "ThresholdPulldownMismatch", ThresholdPulldownMismatch, szIniFile);
	ThresholdPulldownComb  = GetPrivateProfileInt("Pulldown", "ThresholdPulldownComb", ThresholdPulldownComb, szIniFile);
	bFallbackToVideo = (GetPrivateProfileInt("Pulldown", "bFallbackToVideo", bFallbackToVideo, szIniFile) != 0);
	BitShift = GetPrivateProfileInt("Pulldown", "BitShift", BitShift, szIniFile);
	DiffThreshold = GetPrivateProfileInt("Pulldown", "DiffThreshold", DiffThreshold, szIniFile);
	PulldownSwitchInterval = GetPrivateProfileInt("Pulldown", "PulldownSwitchInterval", PulldownSwitchInterval, szIniFile);
	PulldownSwitchMax = GetPrivateProfileInt("Pulldown", "PulldownSwitchMax", PulldownSwitchMax, szIniFile);

	StaticImageFieldCount = GetPrivateProfileInt("Pulldown", "StaticImageFieldCount", StaticImageFieldCount, szIniFile);
	LowMotionFieldCount = GetPrivateProfileInt("Pulldown", "LowMotionFieldCount", LowMotionFieldCount, szIniFile);
	StaticImageMode = GetPrivateProfileInt("Pulldown", "StaticImageMode", StaticImageMode, szIniFile);
	LowMotionMode = GetPrivateProfileInt("Pulldown", "LowMotionMode", LowMotionMode, szIniFile);
	HighMotionMode = GetPrivateProfileInt("Pulldown", "HighMotionMode", HighMotionMode, szIniFile);

	// JA 02/01/2001
	// use SetDeinterlaceFunction so that Half height gets set correctly
	SetDeinterlaceMode(GetPrivateProfileInt("Deinterlace", "DeinterlaceMode", gPulldownMode, szIniFile));
	// JA added film fallback modes
	gPALFilmFallbackMode = GetPrivateProfileInt("Deinterlace", "PALFilmFallbackMode", gPALFilmFallbackMode, szIniFile);
	gNTSCFilmFallbackMode = GetPrivateProfileInt("Deinterlace", "NTSCFilmFallbackMode", gNTSCFilmFallbackMode, szIniFile);
	bAutoDetectMode = (GetPrivateProfileInt("Pulldown", "bAutoDetectMode", bAutoDetectMode, szIniFile) != 0);

	EdgeDetect = GetPrivateProfileInt("Deinterlace", "EdgeDetect", EdgeDetect, szIniFile);
	JaggieThreshold = GetPrivateProfileInt("Deinterlace", "JaggieThreshold", JaggieThreshold, szIniFile);
	SpatialTolerance = GetPrivateProfileInt("Deinterlace", "SpatialTolerance", SpatialTolerance, szIniFile);
	TemporalTolerance = GetPrivateProfileInt("Deinterlace", "TemporalTolerance", TemporalTolerance, szIniFile);
	SimilarityThreshold = GetPrivateProfileInt("Deinterlace", "SimilarityThreshold", SimilarityThreshold, szIniFile);
	
// Deinterlace settings for Blended Clip only
	BlcMinimumClip  = GetPrivateProfileInt("Deinterlace", "BlcMinimumClip", BlcMinimumClip , szIniFile);
	BlcPixelMotionSense = GetPrivateProfileInt("Deinterlace", "BlcPixelMotionSense", BlcPixelMotionSense  , szIniFile);
	BlcRecentMotionSense  = GetPrivateProfileInt("Deinterlace", "BlcRecentMotionSense", BlcRecentMotionSense , szIniFile);
	BlcMotionAvgPeriod  = GetPrivateProfileInt("Deinterlace", "BlcMotionAvgPeriod", BlcMotionAvgPeriod , szIniFile);
	BlcPixelCombSense  = GetPrivateProfileInt("Deinterlace", "BlcPixelCombSense", BlcPixelCombSense , szIniFile);
	BlcRecentCombSense  = GetPrivateProfileInt("Deinterlace", "BlcRecentCombSense", BlcRecentCombSense  , szIniFile);
	BlcCombAvgPeriod  = GetPrivateProfileInt("Deinterlace", "BlcCombAvgPeriod", BlcCombAvgPeriod , szIniFile);
	BlcHighCombSkip  = GetPrivateProfileInt("Deinterlace", "BlcHighCombSkip", BlcHighCombSkip , szIniFile);
	BlcLowMotionSkip  = GetPrivateProfileInt("Deinterlace", "BlcLowMotionSkip", BlcLowMotionSkip , szIniFile);
	BlcVerticalSmoothing  = GetPrivateProfileInt("Deinterlace", "BlcVerticalSmoothing", BlcVerticalSmoothing , szIniFile);
	BlcUseInterpBob = GetPrivateProfileInt("Deinterlace", "BlcUseInterpBob", BlcUseInterpBob , szIniFile);
	BlcBlendChroma  = GetPrivateProfileInt("Deinterlace", "BlcBlendChroma", BlcBlendChroma , szIniFile);
	BlcShowControls  = GetPrivateProfileInt("Deinterlace", "BlcShowControls", BlcShowControls , szIniFile);

	TemporalLuminanceThreshold = GetPrivateProfileInt("NoiseFilter", "TemporalLuminanceThreshold", TemporalLuminanceThreshold, szIniFile);
	TemporalChromaThreshold = GetPrivateProfileInt("NoiseFilter", "TemporalChromaThreshold", TemporalChromaThreshold, szIniFile);
	UseTemporalNoiseFilter = GetPrivateProfileInt("NoiseFilter", "UseTemporalNoiseFilter", UseTemporalNoiseFilter, szIniFile);

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

	CardType = GetPrivateProfileInt("Hardware", "CardType", TVCARD_UNKNOWN, szIniFile);
	VideoSource = GetPrivateProfileInt("Hardware", "VideoSource", VideoSource, szIniFile);
	TunerType = GetPrivateProfileInt("Hardware", "TunerType", TUNER_ABSENT, szIniFile); 
	TVTYPE = GetPrivateProfileInt("Hardware", "TVType", -1, szIniFile); 

	// MAE 2 Nov 2000 - Start of change for Macrovision fix
	InitialBDelay = GetPrivateProfileInt("Hardware", "InitialBDelay", InitialBDelay, szIniFile);
	// MAE 2 Nov 2000 - End of change for Macrovision fix

	// TRB 1218/00 - Add some Adv Video Settings to ini file
	BtAgcDisable = 
		GetPrivateProfileInt("Hardware", "BtAgcDisable", BtAgcDisable >> 4, szIniFile) << 4;
	BtCrush = GetPrivateProfileInt("Hardware", "BtCrush", BtCrush, szIniFile);
	BtEvenChromaAGC = 
		GetPrivateProfileInt("Hardware", "BtEvenChromaAGC", BtEvenChromaAGC >> 6, szIniFile) << 6;
	BtOddChromaAGC = 
		GetPrivateProfileInt("Hardware", "BtOddChromaAGC", BtOddChromaAGC >> 6, szIniFile) << 6;
	BtEvenLumaPeak = 
		GetPrivateProfileInt("Hardware", "BtEvenLumaPeak", BtEvenLumaPeak >> 7, szIniFile) << 7;
	BtOddLumaPeak = 
		GetPrivateProfileInt("Hardware", "BtOddLumaPeak", BtOddLumaPeak >> 7, szIniFile) << 7;
	BtFullLumaRange = 
		GetPrivateProfileInt("Hardware", "BtFullLumaRange", BtFullLumaRange >> 7, szIniFile) << 7;
	BtEvenLumaDec = 
		GetPrivateProfileInt("Hardware", "BtEvenLumaDec", BtEvenLumaDec >> 5, szIniFile) << 5;
	BtOddLumaDec = 
		GetPrivateProfileInt("Hardware", "BtOddLumaDec", BtOddLumaDec >> 5, szIniFile) << 5;
	BtEvenComb = 
		GetPrivateProfileInt("Hardware", "BtEvenComb", BtEvenComb >> 6, szIniFile) << 6;
	BtOddComb = 
		GetPrivateProfileInt("Hardware", "BtOddComb", BtOddComb >> 6, szIniFile) << 6;
	BtColorBars = 
		GetPrivateProfileInt("Hardware", "BtColorBars", BtColorBars >> 6, szIniFile) << 6;
	BtGammaCorrection = 
		GetPrivateProfileInt("Hardware", "BtGammaCorrection", BtGammaCorrection >> 4, szIniFile) << 4;
	BtCoring = GetPrivateProfileInt("Hardware", "BtCoring", BtCoring >> 5, szIniFile) << 5;
	BtHorFilter =
		GetPrivateProfileInt("Hardware", "BtHorFilter", BtHorFilter >> 3, szIniFile) << 3;
	BtVertFilter = GetPrivateProfileInt("Hardware", "BtVertFilter", BtVertFilter, szIniFile);
	BtColorKill = GetPrivateProfileInt("Hardware", "BtColorKill", BtColorKill >> 5, szIniFile) << 5;
	BtWhiteCrushUp = GetPrivateProfileInt("Hardware", "BtWhiteCrushUp", BtWhiteCrushUp, szIniFile);
	BtWhiteCrushDown = GetPrivateProfileInt("Hardware", "BtWhiteCrushDown", BtWhiteCrushDown, szIniFile);

	Capture_VBI = (GetPrivateProfileInt("Show", "CaptureVBI", Capture_VBI, szIniFile) != 0);  
	CurrentProgramm = GetPrivateProfileInt("Show", "LastProgram", CurrentProgramm, szIniFile);

	AudioSource = GetPrivateProfileInt("Sound", "AudioSource", AudioSource, szIniFile);
	
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

	// MRS 9/2/00
	// Load Aspect Mode from INI- using strings would be more elegant long-term
	source_aspect = GetPrivateProfileInt("ASPECT", "SourceAspect", source_aspect, szIniFile);
	custom_source_aspect = GetPrivateProfileInt("ASPECT", "CustomSourceAspect", custom_source_aspect, szIniFile);
	target_aspect = GetPrivateProfileInt("ASPECT", "ScreenAspect", target_aspect, szIniFile);
	custom_target_aspect = GetPrivateProfileInt("ASPECT", "CustomScreenAspect", custom_target_aspect, szIniFile);
	aspect_mode = GetPrivateProfileInt("ASPECT", "Mode", aspect_mode, szIniFile);
	// END MRS 9/2/00
	LuminanceThreshold = GetPrivateProfileInt("ASPECT", "LuminanceThreshold", LuminanceThreshold, szIniFile);
	IgnoreNonBlackPixels = GetPrivateProfileInt("ASPECT", "IgnoreNonBlackPixels", IgnoreNonBlackPixels, szIniFile);
	AutoDetectAspect = GetPrivateProfileInt("ASPECT", "AutoDetectAspect", AutoDetectAspect, szIniFile);
	ZoomInFrameCount = GetPrivateProfileInt("ASPECT", "ZoomInFrameCount", ZoomInFrameCount, szIniFile);
	AspectHistoryTime = GetPrivateProfileInt("ASPECT", "AspectHistoryTime", AspectHistoryTime, szIniFile);
	AspectConsistencyTime = GetPrivateProfileInt("ASPECT", "AspectConsistencyTime", AspectConsistencyTime, szIniFile);
	VerticalPos = GetPrivateProfileInt("ASPECT", "VerticalPos", VerticalPos, szIniFile);
	HorizontalPos = GetPrivateProfileInt("ASPECT", "HorizontalPos", HorizontalPos, szIniFile);
}

void WriteSettingsToIni()
{
	char szKey[128];
	int i;

	Aspect_WriteSetttingsToIni();
	BT848_WriteSetttingsToIni();

	WritePrivateProfileInt("MainWindow", "AlwaysOnTop", bAlwaysOnTop, szIniFile);
	WritePrivateProfileInt("MainWindow", "DisplaySplashScreen", bDisplaySplashScreen, szIniFile);
	WritePrivateProfileInt("MainWindow", "bIsFullScreen", bIsFullScreen, szIniFile);
	if(!bIsFullScreen)
	{
		WritePrivateProfileInt("MainWindow", "StartLeft", emstartx, szIniFile);
		WritePrivateProfileInt("MainWindow", "StartTop", emstarty, szIniFile);
		WritePrivateProfileInt("MainWindow", "StartWidth", emsizex, szIniFile);
		WritePrivateProfileInt("MainWindow", "StartHeight", emsizey, szIniFile);
	}

	WritePrivateProfileInt("Threads", "ProcessPriority", PriorClassId, szIniFile);
	WritePrivateProfileInt("Threads", "ThreadPriority", ThreadClassId, szIniFile);
	WritePrivateProfileInt("Threads", "WindowProcessor", MainProcessor, szIniFile);
	WritePrivateProfileInt("Threads", "DecodeProcessor", DecodeProcessor, szIniFile);

	// Added new performance related parms to Threads group - TRB 10/28/00
	WritePrivateProfileInt("Threads", "Hurry_When_Late", Hurry_When_Late, szIniFile);
	WritePrivateProfileInt("Threads", "Wait_For_Flip", Wait_For_Flip, szIniFile);
	WritePrivateProfileInt("Threads", "DoAccurateFlips", DoAccurateFlips, szIniFile);
	WritePrivateProfileInt("Threads", "Sleep_Interval", Sleep_Interval, szIniFile);

    // Mark Rejhon 01/01/01 - New Overlay section and OverlayColor setting
	WritePrivateProfileInt("Overlay", "Back_Buffers", Back_Buffers, szIniFile);
	WritePrivateProfileInt("Overlay", "OverlayColor", OverlayColor, szIniFile);
	
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
	WritePrivateProfileInt("Pulldown", "StaticImageFieldCount", StaticImageFieldCount, szIniFile);
	WritePrivateProfileInt("Pulldown", "LowMotionFieldCount", LowMotionFieldCount, szIniFile);
	WritePrivateProfileInt("Pulldown", "StaticImageMode", StaticImageMode, szIniFile);
	WritePrivateProfileInt("Pulldown", "LowMotionMode", LowMotionMode, szIniFile);
	WritePrivateProfileInt("Pulldown", "HighMotionMode", HighMotionMode, szIniFile);

	// JA 07/01/2001
	// if we are in autodect mode we don't want to save the current film mode
	// so save the current fallback mode instead.
	if(bAutoDetectMode && DeintMethods[gPulldownMode].bIsFilmMode)
	{
		if(TVSettings[TVTYPE].Is25fps)
		{
			WritePrivateProfileInt("Deinterlace", "DeinterlaceMode", gPALFilmFallbackMode, szIniFile);
		}
		else
		{
			WritePrivateProfileInt("Deinterlace", "DeinterlaceMode", gNTSCFilmFallbackMode, szIniFile);
		}
	}
	else
	{
		WritePrivateProfileInt("Deinterlace", "DeinterlaceMode", gPulldownMode, szIniFile);
	}
	// JA added film fallback modes
	WritePrivateProfileInt("Deinterlace", "PALFilmFallbackMode", gPALFilmFallbackMode, szIniFile);
	WritePrivateProfileInt("Deinterlace", "NTSCFilmFallbackMode", gNTSCFilmFallbackMode, szIniFile);

	WritePrivateProfileInt("Deinterlace", "EdgeDetect", EdgeDetect, szIniFile);
	WritePrivateProfileInt("Deinterlace", "JaggieThreshold", JaggieThreshold, szIniFile);
	WritePrivateProfileInt("Deinterlace", "SpatialTolerance", SpatialTolerance, szIniFile);
	WritePrivateProfileInt("Deinterlace", "TemporalTolerance", TemporalTolerance, szIniFile);
	WritePrivateProfileInt("Deinterlace", "SimilarityThreshold", SimilarityThreshold, szIniFile);

// Deinterlace settings for Blended Clip only
	WritePrivateProfileInt("Deinterlace", "BlcMinimumClip", BlcMinimumClip , szIniFile);
	WritePrivateProfileInt("Deinterlace", "BlcPixelMotionSense", BlcPixelMotionSense  , szIniFile);
	WritePrivateProfileInt("Deinterlace", "BlcRecentMotionSense", BlcRecentMotionSense , szIniFile);
	WritePrivateProfileInt("Deinterlace", "BlcMotionAvgPeriod", BlcMotionAvgPeriod , szIniFile);
	WritePrivateProfileInt("Deinterlace", "BlcPixelCombSense", BlcPixelCombSense , szIniFile);
	WritePrivateProfileInt("Deinterlace", "BlcRecentCombSense", BlcRecentCombSense  , szIniFile);
	WritePrivateProfileInt("Deinterlace", "BlcCombAvgPeriod", BlcCombAvgPeriod , szIniFile);
	WritePrivateProfileInt("Deinterlace", "BlcHighCombSkip", BlcHighCombSkip , szIniFile);
	WritePrivateProfileInt("Deinterlace", "BlcLowMotionSkip", BlcLowMotionSkip , szIniFile);
	WritePrivateProfileInt("Deinterlace", "BlcVerticalSmoothing", BlcVerticalSmoothing , szIniFile);
	WritePrivateProfileInt("Deinterlace", "BlcUseInterpBob", BlcUseInterpBob , szIniFile);
	WritePrivateProfileInt("Deinterlace", "BlcBlendChroma", BlcBlendChroma , szIniFile);
	WritePrivateProfileInt("Deinterlace", "BlcShowControls", BlcShowControls , szIniFile);

	WritePrivateProfileInt("NoiseFilter", "TemporalLuminanceThreshold", TemporalLuminanceThreshold, szIniFile);
	WritePrivateProfileInt("NoiseFilter", "TemporalChromaThreshold", TemporalChromaThreshold, szIniFile);
	WritePrivateProfileInt("NoiseFilter", "UseTemporalNoiseFilter", UseTemporalNoiseFilter, szIniFile);

	WritePrivateProfileInt("Show", "StatusBar", bDisplayStatusBar, szIniFile);
	WritePrivateProfileInt("Show", "Menu", Show_Menu, szIniFile);

	WritePrivateProfileInt("VBI", "VT", VBI_Flags & VBI_VT, szIniFile);
	WritePrivateProfileInt("VBI", "VPS", VBI_Flags & VBI_VPS, szIniFile);
	WritePrivateProfileInt("VBI", "CC", VBI_Flags & VBI_CC, szIniFile);

	WritePrivateProfileString("Files", "DebugLogFilename", DebugLogFilename, szIniFile);
	WritePrivateProfileInt("Files", "DebugLogEnabled", DebugLogEnabled, szIniFile);

	WritePrivateProfileInt("Hardware", "CardType", CardType, szIniFile);
	WritePrivateProfileInt("Hardware", "VideoSource", VideoSource, szIniFile);
	WritePrivateProfileInt("Hardware", "TunerType", TunerType, szIniFile); 
	WritePrivateProfileInt("Hardware", "TVType", TVTYPE, szIniFile); 

// MAE 2 Nov 2000 - Start of change for Macrovision fix
	WritePrivateProfileInt("Hardware", "InitialBDelay", InitialBDelay, szIniFile); 
// MAE 2 Nov 2000 - End of change for Macrovision fix

// TRB 1218/00 - Add some Adv Video Settings to ini file
	WritePrivateProfileInt("Hardware", "BtAgcDisable", BtAgcDisable >> 4, szIniFile);
	WritePrivateProfileInt("Hardware", "BtCrush", BtCrush, szIniFile);
	WritePrivateProfileInt("Hardware", "BtEvenChromaAGC", BtEvenChromaAGC >> 6, szIniFile);
	WritePrivateProfileInt("Hardware", "BtOddChromaAGC", BtOddChromaAGC >> 6, szIniFile);
	WritePrivateProfileInt("Hardware", "BtEvenLumaPeak", BtEvenLumaPeak >> 7, szIniFile);
	WritePrivateProfileInt("Hardware", "BtOddLumaPeak", BtOddLumaPeak >> 7, szIniFile);
	WritePrivateProfileInt("Hardware", "BtFullLumaRange", BtFullLumaRange >> 7, szIniFile);
	WritePrivateProfileInt("Hardware", "BtEvenLumaDec", BtEvenLumaDec >> 5, szIniFile);
	WritePrivateProfileInt("Hardware", "BtOddLumaDec", BtOddLumaDec >> 5, szIniFile);
	WritePrivateProfileInt("Hardware", "BtEvenComb", BtEvenComb >> 6, szIniFile);
	WritePrivateProfileInt("Hardware", "BtOddComb", BtOddComb >> 6, szIniFile);
	WritePrivateProfileInt("Hardware", "BtColorBars", BtColorBars >> 6, szIniFile);
	WritePrivateProfileInt("Hardware", "BtGammaCorrection", BtGammaCorrection >> 4, szIniFile);
	WritePrivateProfileInt("Hardware", "BtCoring", BtCoring >> 5, szIniFile);
	WritePrivateProfileInt("Hardware", "BtHorFilter", BtHorFilter >> 3, szIniFile);
	WritePrivateProfileInt("Hardware", "BtVertFilter", BtVertFilter, szIniFile);
	WritePrivateProfileInt("Hardware", "BtColorKill", BtColorKill >> 5, szIniFile);
	WritePrivateProfileInt("Hardware", "BtWhiteCrushUp", BtWhiteCrushUp, szIniFile);
	WritePrivateProfileInt("Hardware", "BtWhiteCrushDown", BtWhiteCrushDown, szIniFile);

	WritePrivateProfileInt("Show", "CaptureVBI", Capture_VBI, szIniFile);
	WritePrivateProfileInt("Show", "LastProgram", CurrentProgramm, szIniFile);

	WritePrivateProfileInt("Sound", "AudioSource", AudioSource, szIniFile);
	
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
	WritePrivateProfileInt("ASPECT", "AspectHistoryTime", AspectHistoryTime, szIniFile);
	WritePrivateProfileInt("ASPECT", "AspectConsistencyTime", AspectConsistencyTime, szIniFile);
	WritePrivateProfileInt("ASPECT", "VerticalPos", VerticalPos, szIniFile);
	WritePrivateProfileInt("ASPECT", "HorizontalPos", HorizontalPos, szIniFile);
}

void WritePrivateProfileInt(LPCTSTR lpAppName,  LPCTSTR lpKeyName,  int nValue, LPCTSTR lpFileName)
{
	char szValue[128];
	sprintf(szValue, "%d", nValue);
	WritePrivateProfileString(lpAppName,  lpKeyName,  szValue, lpFileName);
}

DWORD GetRefreshRate()
{
	return GetPrivateProfileInt("Pulldown", "RefreshRate", 0, szIniFile);	
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
	switch(pSetting->Type)
	{
	case YESNO:
		*pSetting->pValue = (Value != 0);
		break;
	case ITEMFROMLIST:
	case SLIDER:
	case NUMBER:
		if(Value > pSetting->MaxValue)
		{
			*pSetting->pValue = pSetting->MaxValue;
		}
		else if(Value < pSetting->MinValue)
		{
			*pSetting->pValue = pSetting->MinValue;
		}
		else
		{
			*pSetting->pValue = Value;
		}
		break;
	default:
		return FALSE;
		break;
	}
	
	if(pSetting->pfnOnChange != NULL)
	{
		return pSetting->pfnOnChange(Value);
	}
	else
	{
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
		Setting_SetValue(pSetting, nValue);
		pSetting->bHasChanged = FALSE;
	}
}

void Setting_WriteToIni(SETTING* pSetting)
{
	if(pSetting->szIniSection != NULL)
	{
		if(pSetting->bHasChanged)
		{
			if(pSetting->Default != *pSetting->pValue)
			{
				WritePrivateProfileInt(pSetting->szIniSection, pSetting->szIniEntry, *pSetting->pValue, szIniFile);
			}
			else
			{
				WritePrivateProfileInt(pSetting->szIniSection, pSetting->szIniEntry, pSetting->MinValue - 100, szIniFile);
			}
		}
	}
}


void Setting_OSDShow(SETTING* pSetting, HWND hWnd)
{
	char szBuffer[1024] = "Unexpected Display Error";

	switch(pSetting->Type)
	{
	case ITEMFROMLIST:
		sprintf(szBuffer, "%s - %s", pSetting->szDisplayName, pSetting->pszList[*(pSetting->pValue)]);
		break;
	case YESNO:
		sprintf(szBuffer, "%s - %s", pSetting->szDisplayName, *(pSetting->pValue)?"YES":"NO");
		break;
	case SLIDER:
	case NUMBER:
		sprintf(szBuffer, "%s - %d", pSetting->szDisplayName, *(pSetting->pValue));
		break;
	default:
		break;
	}
	OSD_ShowText(hWnd, szBuffer, 0);
}

void Setting_Up(SETTING* pSetting)
{
}

void Setting_Down(SETTING* pSetting)
{
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

