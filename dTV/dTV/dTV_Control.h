/////////////////////////////////////////////////////////////////////////////
// dTV_Control.h
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
//
// This header file is free software; you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
//
// This header file is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
// License for more details.
//
/////////////////////////////////////////////////////////////////////////////
//
// Notes for writers of external apps
//
// To use control dTV using an external app you can use SendMessage to
// perform both commands and to get/set all the settings
//
// To perform commands use the WM_COMMAND message
// e.g. SendMessage(hWndDTV, WM_COMMAND, IDM_OSD_SHOW, 0);
//
// To get settings use the appropriate WM_XXX_GETVALUE
// e.g. Brightness = SendMessage(hWndDTV, WM_BT848_GETVALUE, BRIGHTNESS, 0);
//
// To set settings use the appropriate WM_XXX_SETVALUE
// e.g. SendMessage(hWndDTV, WM_BT848_SETVALUE, HUE, NewHueValue);
//
// To up settings use the appropriate WM_XXX_CHANGEVALUE
// e.g. SendMessage(hWndDTV, WM_BT848_CHANGEVALUE, HUE, INCREMENTVALUE);
//
// To down settings use the appropriate WM_XXX_CHANGEVALUE
// e.g. SendMessage(hWndDTV, WM_BT848_CHANGEVALUE, HUE, DECREMENTVALUE);
//
// To show settings use the appropriate WM_XXX_CHANGEVALUE
// e.g. SendMessage(hWndDTV, WM_BT848_CHANGEVALUE, HUE, DISPLAYVALUE);
//
// To reset settings to default use the appropriate WM_XXX_CHANGEVALUE
// e.g. SendMessage(hWndDTV, WM_BT848_CHANGEVALUE, HUE, RESETVALUE);
//
// To do the above operation without using the OSD use the ????VALUE_SILENT
// e.g. SendMessage(hWndDTV, WM_BT848_CHANGEVALUE, HUE, INCREMENTVALUE_SILENT);
//
// The dTV window handle can be obtained using
// hWndDTV = FindWindow("dTV", NULL);
//
/////////////////////////////////////////////////////////////////////////////
//
// Notes for dTV developers
//
// This is the place to add settings for any new file you create
// You should also update the LoadSettingsFromIni & SaveSettingsToIni
// functions in Settings.h so that your setttings get loaded
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 16 Jan 2001   John Adcock           Moved all parts that could be used to
//                                     Control dTV externally to this file
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __DTV_CONTROL_H___
#define __DTV_CONTROL_H___

/////////////////////////////////////////////////////////////////////////////
// Control Messages passed using WM_COMMAND
/////////////////////////////////////////////////////////////////////////////
#ifdef DTV_EXTERNAL

#define IDM_VT_RESET                    261
#define IDM_RESET                       280
#define IDM_TAKESTILL                   485
#define IDM_OVERLAY_STOP                590
#define IDM_OVERLAY_START               591
#define IDM_HIDE_OSD                    592
#define IDM_SHOW_OSD                    593
// IDM_SET_OSD_TEXT the lParam must be the handle of a global atom
#define IDM_SET_OSD_TEXT                594
#define IDM_SASPECT_COMPUTE             731

#endif


/////////////////////////////////////////////////////////////////////////////
// Constants for WM_????_CHANGEVALUE messages
/////////////////////////////////////////////////////////////////////////////
typedef enum
{
	DISPLAYVALUE = 0,
	INCREMENTVALUE,
	DECREMENTVALUE,
	RESETVALUE,
	TOGGLEBOOL,
	INCREMENTVALUE_SILENT,
	DECREMENTVALUE_SILENT,
	RESETVALUE_SILENT,
	TOGGLEBOOL_SILENT,
} eCHANGEVALUE;

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in AspectRatio.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
	OVERSCAN,
	SOURCE_ASPECT,
	CUSTOM_SOURCE_ASPECT,
	TARGET_ASPECT,
	CUSTOM_TARGET_ASPECT,
	ASPECT_MODE,
	LUMINANCETHRESHOLD,
	IGNORENONBLACKPIXELS,
	AUTODETECTASPECT,
	ZOOMINFRAMECOUNT,
	ASPECTHISTORYTIME,
	ASPECTCONSISTENCYTIME,
	VERTICALPOS,
	HORIZONTALPOS,
	ASPECT_SETTING_LASTONE,
} ASPECT_SETTING;

#define WM_ASPECT_GETVALUE			(WM_USER + 1)
#define WM_ASPECT_SETVALUE			(WM_USER + 101)
#define WM_ASPECT_CHANGEVALUE		(WM_USER + 201)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in Bt848.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
	BRIGHTNESS = 0,
	CONTRAST,
	HUE,
	SATURATION,
	SATURATIONU,
	SATURATIONV,
	BDELAY,
	BTAGCDISABLE,
	BTCRUSH,
	BTEVENCHROMAAGC,
	BTODDCHROMAAGC,
	BTEVENLUMAPEAK,
	BTODDLUMAPEAK,
	BTFULLLUMARANGE,
	BTEVENLUMADEC,
	BTODDLUMADEC,
	BTEVENCOMB,
	BTODDCOMB,
	BTCOLORBARS,
	BTGAMMACORRECTION,
	BTCORING,
	BTHORFILTER,
	BTVERTFILTER,
	BTCOLORKILL,
	BTWHITECRUSHUP,
	BTWHITECRUSHDOWN,
	CURRENTX,
	CUSTOMPIXELWIDTH,
	VIDEOSOURCE,
	TVFORMAT,
	HDELAY,
	VDELAY,
	BT848_SETTING_LASTONE,
} BT848_SETTING;

#define WM_BT848_GETVALUE			(WM_USER + 2)
#define WM_BT848_SETVALUE			(WM_USER + 102)
#define WM_BT848_CHANGEVALUE		(WM_USER + 202)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in dTV.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
	STARTLEFT = 0,
	STARTTOP,
	STARTWIDTH,
	STARTHEIGHT,
	ALWAYSONTOP,
	DISPLAYSPLASHSCREEN,
	ISFULLSCREEN,
	FORCEFULLSCREEN,
	SHOWSTATUSBAR,
	SHOWMENU,
	WINDOWPROCESSOR,
	THREADPROCESSOR,
	WINDOWPRIORITY,
	THREADPRIORITY,
	DTV_SETTING_LASTONE,
} DTV_SETTING;

#define WM_DTV_GETVALUE				(WM_USER + 3)
#define WM_DTV_SETVALUE				(WM_USER + 103)
#define WM_DTV_CHANGEVALUE			(WM_USER + 203)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in OutThreads.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
	HURRYWHENLATE = 0,
	WAITFORFLIP,
	DOACCURATEFLIPS,
	SLEEPINTERVAL,
	AUTODETECT,
	PULLDOWNMODE,
	REFRESHRATE,
	WAITFORVSYNC,
	OUTTHREADS_SETTING_LASTONE,
} OUTTHREADS_SETTING;

#define WM_OUTHREADS_GETVALUE		(WM_USER + 4)
#define WM_OUTHREADS_SETVALUE		(WM_USER + 104)
#define WM_OUTHREADS_CHANGEVALUE	(WM_USER + 204)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in Other.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
	BACKBUFFERS = 0,
	OVERLAYCOLOR,
	OTHER_SETTING_LASTONE,
} OTHER_SETTING;

#define WM_OTHER_GETVALUE			(WM_USER + 5)
#define WM_OTHER_SETVALUE			(WM_USER + 105)
#define WM_OTHER_CHANGEVALUE		(WM_USER + 205)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in FD_50Hz.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
	PULLDOWNTHRESHHOLDLOW = 0,
	PULLDOWNTHRESHHOLDHIGH,
	PALFILMFALLBACKMODE,
	PALFILMREPEATCOUNT,
	PALFILMREPEATCOUNT2,
	FD50_SETTING_LASTONE,
} FD50_SETTING;

#define WM_FD50_GETVALUE			(WM_USER + 6)
#define WM_FD50_SETVALUE			(WM_USER + 106)
#define WM_FD50_CHANGEVALUE			(WM_USER + 206)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in FD_50Hz.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
	NTSCFILMFALLBACKMODE = 0,
	NTSCPULLDOWNREPEATCOUNT,
	NTSCPULLDOWNREPEATCOUNT2,
	THRESHOLD32PULLDOWN,
	THRESHOLDPULLDOWNMISMATCH,
	THRESHOLDPULLDOWNCOMB,
	FALLBACKTOVIDEO,
	PULLDOWNSWITCHINTERVAL,
	PULLDOWNSWITCHMAX,
	FD60_SETTING_LASTONE,
} FD60_SETTING;

#define WM_FD60_GETVALUE			(WM_USER + 7)
#define WM_FD60_SETVALUE			(WM_USER + 107)
#define WM_FD60_CHANGEVALUE			(WM_USER + 207)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in FD_Common.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
	BITSHIFT = 0,
	EDGEDETECT,
	JAGGIETHRESHOLD,
	DIFFTHRESHOLD,
	FD_COMMON_SETTING_LASTONE,
} FD_COMMON_SETTING;

#define WM_FD_COMMON_GETVALUE		(WM_USER + 8)
#define WM_FD_COMMON_SETVALUE		(WM_USER + 108)
#define WM_FD_COMMON_CHANGEVALUE	(WM_USER + 208)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in DI_Adaptive.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
	LOWMOTIONFIELDCOUNT = 0,
	STATICIMAGEFIELDCOUNT,
	STATICIMAGEMODE,
	LOWMOTIONMODE,
	HIGHMOTIONMODE,
	ADAPTIVETHRESH32PULLDOWN,
	ADAPTIVETHRESHMISMATCH,
	DI_ADAPTIVE_SETTING_LASTONE,
} DI_ADAPTIVE_SETTING;

#define WM_DI_ADAPTIVE_GETVALUE		(WM_USER + 9)
#define WM_DI_ADAPTIVE_SETVALUE		(WM_USER + 109)
#define WM_DI_ADAPTIVE_CHANGEVALUE	(WM_USER + 209)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in DI_BobAndWeave.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
	WEAVEEDGEDETECT = 0,
	WEAVEJAGGIETHRESHOLD,
	TEMPORALTOLERANCE,
	SPATIALTOLERANCE,
	SIMILARITYTHRESHOLD,
	DI_BOBWEAVE_SETTING_LASTONE,
} DI_BOBWEAVE_SETTING;

#define WM_DI_BOBWEAVE_GETVALUE		(WM_USER + 10)
#define WM_DI_BOBWEAVE_SETVALUE		(WM_USER + 110)
#define WM_DI_BOBWEAVE_CHANGEVALUE	(WM_USER + 210)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in DI_BlendedClip.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
	BLCMINIMUMCLIP = 0,
	BLCPIXELMOTIONSENSE,
	BLCRECENTMOTIONSENSE,
	BLCMOTIONAVGPERIOD,
	BLCPIXELCOMBSENSE,
	BLCRECENTCOMBSENSE,
	BLCCOMBAVGPERIOD,
	BLCHIGHCOMBSKIP,
	BLCLOWMOTIONSKIP,
	BLCVERTICALSMOOTHING,
	BLCUSEINTERPBOB,
	BLCBLENDCHROMA,
	BLCSHOWCONTROLS,
	DI_BLENDEDCLIP_SETTING_LASTONE,
} DI_BLENDEDCLIP_SETTING;

#define WM_DI_BLENDEDCLIP_GETVALUE		(WM_USER + 11)
#define WM_DI_BLENDEDCLIP_SETVALUE		(WM_USER + 111)
#define WM_DI_BLENDEDCLIP_CHANGEVALUE	(WM_USER + 211)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in DI_TwoFrame.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
	TWOFRAMESPATIALTOLERANCE = 0,
	TWOFRAMETEMPORALTOLERANCE,
	DI_TWOFRAME_SETTING_LASTONE,
} DI_TWOFRAME_SETTING;

#define WM_DI_TWOFRAME_GETVALUE		(WM_USER + 12)
#define WM_DI_TWOFRAME_SETVALUE		(WM_USER + 112)
#define WM_DI_TWOFRAME_CHANGEVALUE	(WM_USER + 212)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in Deinterlace.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
	DEINTERLACE_SETTING_LASTONE = 0,
} DEINTERLACE_SETTING;

#define WM_DEINTERLACE_GETVALUE		(WM_USER + 13)
#define WM_DEINTERLACE_SETVALUE		(WM_USER + 113)
#define WM_DEINTERLACE_CHANGEVALUE	(WM_USER + 213)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in FLT_TNoise.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
	USETEMPORALNOISEFILTER = 0,
	TEMPORALLUMINANCETHRESHOLD,
	TEMPORALCHROMATHRESHOLD,
	FLT_TNOISE_SETTING_LASTONE,
} FLT_TNOISE_SETTING;

#define WM_FLT_TNOISE_GETVALUE		(WM_USER + 14)
#define WM_FLT_TNOISE_SETVALUE		(WM_USER + 114)
#define WM_FLT_TNOISE_CHANGEVALUE	(WM_USER + 214)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in Greedy.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
	GREEDYMAXCOMB = 0,
	DI_GREEDY_SETTING_LASTONE,
} DI_GREEDY_SETTING;
#define WM_DI_GREEDY_GETVALUE		(WM_USER + 15)
#define WM_DI_GREEDY_SETVALUE		(WM_USER + 115)
#define WM_DI_GREEDY_CHANGEVALUE	(WM_USER + 215)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in TVCards.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
	CURRENTCARDTYPE = 0,
	CURRENTTUNERTYPE,
	PROCESSORSPEED,
	TRADEOFF,
	TVCARD_SETTING_LASTONE,
} TVCARD_SETTING;
#define WM_TVCARD_GETVALUE		(WM_USER + 16)
#define WM_TVCARD_SETVALUE		(WM_USER + 116)
#define WM_TVCARD_CHANGEVALUE	(WM_USER + 216)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in VideoSettings.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
	SAVEPERINPUT = 0,
	SAVEPERFORMAT,
	VIDEOSETTINGS_SETTING_LASTONE,
} VIDEOSETTINGS_SETTING;
#define WM_VIDEOSETTINGS_GETVALUE		(WM_USER + 17)
#define WM_VIDEOSETTINGS_SETVALUE		(WM_USER + 117)
#define WM_VIDEOSETTINGS_CHANGEVALUE	(WM_USER + 217)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in OSD.c
/////////////////////////////////////////////////////////////////////////////
typedef enum
{
	OSDB_TRANSPARENT = 0,
	OSDB_BLOCK,
	OSDB_SHADED,
	OSDBACK_LASTONE,
} eOSDBackground;

typedef enum
{
	OSD_OUTLINECOLOR,
	OSD_TEXTCOLOR,
	OSD_PERCENTAGESIZE,
	OSD_ANTIALIAS,
	OSD_BACKGROUND,
	OSD_OUTLINE,
	OSD_SETTING_LASTONE,
} OSD_SETTING;
#define WM_OSD_GETVALUE		(WM_USER + 18)
#define WM_OSD_SETVALUE		(WM_USER + 118)
#define WM_OSD_CHANGEVALUE	(WM_USER + 218)

#endif