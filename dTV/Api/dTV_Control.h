/////////////////////////////////////////////////////////////////////////////
// dTV_Control.h
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
// This header file is free software; you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details
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
// hWndDTV = FindWindow(DSCALER_APPNAME, NULL);
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
// 20 Feb 2001   Michael Samblanet     Added new values for Aspect Control
//                                     (bounce & clipping modes)
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __DTV_CONTROL_H___
#define __DTV_CONTROL_H___

#define DSCALER_APPNAME "dScaler"

/////////////////////////////////////////////////////////////////////////////
// Control Messages passed using WM_COMMAND
/////////////////////////////////////////////////////////////////////////////
#ifdef DTV_EXTERNAL

#define IDM_SAVE_SETTINGS_NOW           100

#define IDM_VT_RESET                    261
#define IDM_RESET                       280
#define IDM_TAKESTILL                   485
#define IDM_OVERLAY_STOP                590
#define IDM_OVERLAY_START               591
#define IDM_HIDE_OSD                    592
#define IDM_SHOW_OSD                    593
// IDM_SET_OSD_TEXT the lParam must be the handle of a global atom
#define IDM_SET_OSD_TEXT                594
#define IDM_FAST_REPAINT                599
#define IDM_ASPECT_FULLSCREEN           701
#define IDM_ASPECT_LETTERBOX            702
#define IDM_ASPECT_ANAMORPHIC           703

// Messages for the Source Aspect Ratio Menu
#define IDM_SASPECT_0                   710
#define IDM_SASPECT_133                 711
#define IDM_SASPECT_166                 712
#define IDM_SASPECT_178                 713
#define IDM_SASPECT_185                 714
#define IDM_SASPECT_200                 715
#define IDM_SASPECT_235                 716
#define IDM_SASPECT_166A                720
#define IDM_SASPECT_178A                721
#define IDM_SASPECT_185A                722
#define IDM_SASPECT_200A                723
#define IDM_SASPECT_235A                724
#define IDM_SASPECT_COMPUTE             731

// Messages for the Video Format Menu
#define IDM_TYPEFORMAT_PAL              1120
#define IDM_TYPEFORMAT_NTSC             1121
#define IDM_TYPEFORMAT_SECAM            1122
#define IDM_TYPEFORMAT_PAL_M            1123
#define IDM_TYPEFORMAT_PAL_N            1124
#define IDM_TYPEFORMAT_NTSC_JAPAN       1125
#define IDM_TYPEFORMAT_PAL60            1126

// Messages for the Input Select Menu
#define IDM_SOURCE_TUNER                1089
#define IDM_SOURCE_COMPOSITE            1090
#define IDM_SOURCE_SVIDEO               1091
#define IDM_SOURCE_OTHER1               1092
#define IDM_SOURCE_OTHER2               1093
#define IDM_SOURCE_COMPVIASVIDEO        1094
#define IDM_SOURCE_CCIR656_1            1095
#define IDM_SOURCE_CCIR656_2            1096
#define IDM_SOURCE_CCIR656_3            1097
#define IDM_SOURCE_CCIR656_4            1098

// Messages for the Screen Aspect Ratio Menu
#define IDM_TASPECT_0                   740
#define IDM_TASPECT_133                 741
#define IDM_TASPECT_166                 742
#define IDM_TASPECT_178                 743
#define IDM_TASPECT_185                 744
#define IDM_TASPECT_200                 745
#define IDM_TASPECT_235                 746

// Indexes for the Video Deinterlace Algorithms
// to select a deinterlace method send a WM_COMMAND message
// with the relevant index added to this message
// e.g. To switch to adaptive
// SendMessage(hWndDTV, WM_COMMAND, IDM_FIRST_DEINTMETHOD + INDEX_ADAPTIVE, 0);
#define IDM_FIRST_DEINTMETHOD             1900

/////////////////////////////////////////////////////////////////////////////
// For setting of certain values
//
typedef enum
{
	SOURCE_TUNER = 0,
	SOURCE_COMPOSITE,
	SOURCE_SVIDEO,
	SOURCE_OTHER1,
	SOURCE_OTHER2,
	SOURCE_COMPVIASVIDEO,
	SOURCE_CCIR656_1,
	SOURCE_CCIR656_2,
	SOURCE_CCIR656_3,
	SOURCE_CCIR656_4,
} VIDEOSOURCETYPE;

typedef enum
{
	FORMAT_PAL_BDGHI = 0,
	FORMAT_NTSC,
	FORMAT_SECAM,
	FORMAT_PAL_M,
	FORMAT_PAL_N,
	FORMAT_NTSC_J,
	FORMAT_PAL60,
    FORMAT_LASTONE,
} VIDEOFORMAT;

#endif

// Indexes for the Video Deinterlace Algorithms
#define INDEX_VIDEO_BOB                   0
#define INDEX_VIDEO_WEAVE                 1
#define INDEX_VIDEO_2FRAME                2
#define INDEX_WEAVE                       3
#define INDEX_BOB                         4
#define INDEX_SCALER_BOB                  5
#define INDEX_EVEN_ONLY                   13
#define INDEX_ODD_ONLY                    14
#define INDEX_BLENDED_CLIP                15
#define INDEX_ADAPTIVE                    16
#define INDEX_VIDEO_GREEDY                17
#define INDEX_VIDEO_GREEDY2FRAME          18

/////////////////////////////////////////////////////////////////////////////
// Allow callers to convert a WM code to do other operations
/////////////////////////////////////////////////////////////////////////////

#define WM_CONVERT_TO_GETVALUE(x)     (((x - WM_USER) % 100) + WM_USER)
#define WM_CONVERT_TO_SETVALUE(x)    ((((x - WM_USER) % 100) + 100) + WM_USER)
#define WM_CONVERT_TO_CHANGEVALUE(x) ((((x - WM_USER) % 100) + 200) + WM_USER)


/////////////////////////////////////////////////////////////////////////////
// Constants for WM_????_CHANGEVALUE messages
/////////////////////////////////////////////////////////////////////////////
typedef enum
{
	DISPLAY = 0,          // Display OSD value.
	ADJUSTUP,             // Increase value, with acceleration [display OSD]
	ADJUSTDOWN,           // Decrease value, with acceleration [display OSD]
	INCREMENT,            // Increase value by 1 [display OSD]
	DECREMENT,            // Decrease value by 1 [display OSD]
	RESET,                // Reset value to default [display OSD]
	TOGGLEBOOL,           // Toggle a boolean setting [display OSD]
	ADJUSTUP_SILENT,      // Same, but no OSD
	ADJUSTDOWN_SILENT,    // Same, but no OSD
	INCREMENT_SILENT,     // Same, but no OSD
	DECREMENT_SILENT,     // Same, but no OSD
	RESET_SILENT,         // Same, but no OSD
	TOGGLEBOOL_SILENT,    // Same, but no OSD
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
	CLIPPING,
	BOUNCE,
	BOUNCEPERIOD,
	DEFERSETOVERLAY,
	BOUNCETIMERPERIOD,
	ORBIT,
	ORBITPERIODX,
	ORBITPERIODY,
	ORBITSIZE,
	ORBITTIMERPERIOD,
	AUTOSIZEWINDOW,
	SKIPPIXELS,
	aaaa1,
	bbbbb1,
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
	AUTOHIDECURSOR,
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
	PULLDOWNMODE_DEPRECATED,
	REFRESHRATE,
	WAITFORVSYNC,
	REVERSEPOLARITY,
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
// Control settings contained in DI_VideoBob.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
	WEAVEEDGEDETECT = 0,
	WEAVEJAGGIETHRESHOLD,
	DI_VIDEOBOB_SETTING_LASTONE,
} DI_VIDEOBOB_SETTING;

#define WM_DI_VIDEOBOB_GETVALUE		(WM_USER + 10)
#define WM_DI_VIDEOBOB_SETVALUE		(WM_USER + 110)
#define WM_DI_VIDEOBOB_CHANGEVALUE	(WM_USER + 210)

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
// Control settings contained in DI_Greedy.c
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
// Control settings contained in FLT_TNoise.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
	TEMPORALLUMINANCETHRESHOLD = 0,
	TEMPORALCHROMATHRESHOLD,
	USETEMPORALNOISEFILTER,
	FLT_TNOISE_SETTING_LASTONE,
} FLT_TNOISE_SETTING;

#define WM_FLT_TNOISE_GETVALUE		(WM_USER + 14)
#define WM_FLT_TNOISE_SETVALUE		(WM_USER + 114)
#define WM_FLT_TNOISE_CHANGEVALUE	(WM_USER + 214)

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
	SAVETVFORMATPERINPUT,
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
	OSD_OUTLINECOLOR = 0,
	OSD_TEXTCOLOR,
	OSD_PERCENTAGESIZE,
	OSD_PERCENTAGESMALLSIZE,
	OSD_ANTIALIAS,
	OSD_BACKGROUND,
	OSD_OUTLINE,
	OSD_AUTOHIDE_SCREEN,
	OSD_SETTING_LASTONE,
} OSD_SETTING;
#define WM_OSD_GETVALUE		(WM_USER + 18)
#define WM_OSD_SETVALUE		(WM_USER + 118)
#define WM_OSD_CHANGEVALUE	(WM_USER + 218)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in FLT_Gamma.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
	GAMMAVALUE = 0,
	USESTOREDTABLE,
	USEGAMMAFILTER,
	FLT_GAMMA_SETTING_LASTONE,
} FLT_GAMMA_SETTING;

#define WM_FLT_GAMMA_GETVALUE		(WM_USER + 20)
#define WM_FLT_GAMMA_SETVALUE		(WM_USER + 120)
#define WM_FLT_GAMMA_CHANGEVALUE	(WM_USER + 220)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in VBI.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
	CAPTURE_VBI = 0,
	CLOSEDCAPTIONMODE,
	DOTELETEXT,
	DOVPS,
	DOWSS,
	VBI_SETTING_LASTONE,
} VBI_SETTING;

#define WM_VBI_GETVALUE		(WM_USER + 21)
#define WM_VBI_SETVALUE		(WM_USER + 121)
#define WM_VBI_CHANGEVALUE	(WM_USER + 221)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in DI_Greedy2Frame.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
	GREEDYTWOFRAMETHRESHOLD = 0,
	DI_GREEDY2FRAME_SETTING_LASTONE,
} DI_GREEDY2FRAME_SETTING;

#define WM_DI_GREEDY2FRAME_GETVALUE		(WM_USER + 22)
#define WM_DI_GREEDY2FRAME_SETVALUE		(WM_USER + 122)
#define WM_DI_GREEDY2FRAME_CHANGEVALUE	(WM_USER + 222)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in DI_VideoWeave.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
	TEMPORALTOLERANCE = 0,
	SPATIALTOLERANCE,
	SIMILARITYTHRESHOLD,
	DI_VIDEOWEAVE_SETTING_LASTONE,
} DI_VIDEOWEAVE_SETTING;

#define WM_DI_VIDEOWEAVE_GETVALUE		(WM_USER + 23)
#define WM_DI_VIDEOWEAVE_SETVALUE		(WM_USER + 123)
#define WM_DI_VIDEOWEAVE_CHANGEVALUE	(WM_USER + 223)

/////////////////////////////////////////////////////////////////////////////
// Control settings contained in FLT_LinearCorrection.c
/////////////////////////////////////////////////////////////////////////////

typedef enum
{
	USELINEARCORRFILTER = 0,
	DOONLYMASKING,
	MASKTYPE,
	MASKPARAM1,
	MASKPARAM2,
	MASKPARAM3,
	MASKPARAM4,
	FLT_LINEAR_CORR_SETTING_LASTONE,
} FLT_LINEAR_CORR_SETTING;

#define WM_FLT_LINEAR_CORR_GETVALUE		(WM_USER + 24)
#define WM_FLT_LINEAR_CORR_SETVALUE		(WM_USER + 124)
#define WM_FLT_LINEAR_CORR_CHANGEVALUE	(WM_USER + 224)

#endif
