/////////////////////////////////////////////////////////////////////////////
// globals.c
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
//
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Original Release
//                                     Translated most code from German
//                                     Combined Header files
//                                     Cut out all decoding
//                                     Cut out digital hardware stuff
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "globals.h"
#include "bt848.h"

int InitialVolume = 1000;
char InitialBalance = 0x00;
char InitialLoudness = 0x00;
char InitialBass = 0x00;
char InitialTreble = 0x00;
BOOL InitialSuperBass = FALSE;
char InitialEqualizer[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
BOOL MSPToneControl = FALSE;
char InitialSpatial = 0x00;

HWND ShowPDCInfo=NULL;
HWND ShowVTInfo=NULL;
HWND ShowVPSInfo=NULL;


struct TTVSetting TVSettings[10] =
{
	/* PAL-BDGHI */
	{ 768, 576, 1135, 0x7f, 0x72, (BT848_IFORM_PAL_BDGHI|BT848_IFORM_XT1),
	    944, 768, 186, 922, 0x1e, 0, TRUE, 511},
	/* NTSC Square Pixel */ 
	{ 640, 480,  910, 0x68, 0x5d, (BT848_IFORM_NTSC|BT848_IFORM_XT0),
	    780, 640, 135, 754, 0x1a, 0, FALSE, 400},
	/* SECAM */
	{ 922, 576, 1135, 0x7f, 0xa0, (BT848_IFORM_SECAM|BT848_IFORM_XT1),
	    1135, 922, 186, 922, 0x1c, 0, TRUE, 511},
	/* PAL-M */
	{ 754, 480,  910, 0x70, 0x5d, (BT848_IFORM_PAL_M|BT848_IFORM_XT0),
	    910, 754, 135, 754, 0x1a, 0, FALSE, 400},
	/* PAL-N */
	{ 922, 576, 1135, 0x7f, 0x72, (BT848_IFORM_PAL_N|BT848_IFORM_XT1),
	    1135, 922, 186, 922, 0x1c, 0, TRUE, 400},
	/* NTSC Japan*/
	{ 754, 480,  910, 0x70, 0x5d, (BT848_IFORM_NTSC_JAP|BT848_IFORM_XT0),
	    910, 754, 135, 754, 0x1a, 0, FALSE, 400},
	/* PAL Full Pixel */
	{ 922, 576, 1135, 0x7f, 0x72, (BT848_IFORM_PAL_BDGHI|BT848_IFORM_XT1),
	    1135, 922, 186, 922, 0x1c, 0 , TRUE, 511},
	/* NTSC Full Pixel */
	{ 754, 480,  910, 0x70, 0x5d, (BT848_IFORM_NTSC|BT848_IFORM_XT0),
	    910, 754, 135, 754, 0x1a, 0, FALSE, 400},
	/* NTSC CCIR601 */ 
	{ 720, 480,  910, 0x68, 0x5d, (BT848_IFORM_NTSC|BT848_IFORM_XT0),
	    858, 720, 135, 754, 0x1a, 0, FALSE, 400},
};


unsigned int ManuellAudio[8] =
{ 
	0x0000, 
	0x0000, 
	0x0000, 
	0x0000, 
	0x0000,
	0x0000,
	0x0000,
	0x0000
};


BOOL System_In_Mute=FALSE;

BOOL bSaveSettings = FALSE;

char CurrentDir[256];

HMIXER hMixer=NULL;

BOOL bIsOddField = FALSE;

struct TMixerAccess Volume={-1,0,0,0};
struct TMixerAccess Mute={-1,0,0,0};

struct TMixerLoad MixerLoad[64];

struct TLNB LNB = 
{
	FALSE,
	10700, 12750, 9750, 10600, 11700, TRUE, TRUE,
	10700, 12750, 9750, 10600, 11700, TRUE, TRUE,
	10700, 12750, 9750, 10600, 11700, TRUE, TRUE,
	10700, 12750, 9750, 10600, 11700, TRUE, TRUE,
};

HWND hWnd = NULL;
HANDLE hInst = NULL;

int emsizex = 754;
int emsizey = 521;
int emstartx = 10;
int emstarty = 10;

int pgsizex = -1;
int pgsizey = -1;
int pgstartx = -1;
int pgstarty = -1;

BOOL bAlwaysOnTop = FALSE;

BOOL bDisplayStatusBar = TRUE;

PMemStruct Risc_dma;
PMemStruct Vbi_dma[5];
PMemStruct Display_dma[5];
PMemStruct Burst_dma[5];

HANDLE Bt848Device=NULL;

int TVFormat;
int AudioSource = 4;
int VideoSource = 1;
int CountryCode=0;
int TVTYPE = 0;

/// VideoText
unsigned short VTColourTable[9] =
{
	0,		//Black
	31744,	//Red
	992,	//Green
	32736,	//Yellow
	31,		//Blue
	15375,	//Invisible
	15871,	//Cyan
	32767,	//White 
	32767,	//Transparent
};

char InitialHue = 0x00;
char InitialBrightness = 0x00;
int InitialContrast = 0xd8;
int InitialSaturationU = 0xfe;
int InitialSaturationV = 0xb4;
int InitialOverscan = 4;

BOOL Has_MSP=FALSE;

BOOL Capture_VBI = TRUE;

LPDIRECTDRAW lpDD = NULL;

int VBI_Flags=3;

struct TProgramm Programm[MAXPROGS+1];

HFONT currFont = NULL;

BOOL VD_RAW=FALSE;


struct SOTREC SOTInfoRec;

unsigned short UTPages[12];
unsigned short UTCount=0;

ePULLDOWNMODES gPulldownMode = VIDEO_MODE;

long PulldownThresholdLow = 100;
long PulldownThresholdHigh = 7000;
long PulldownRepeatCount = 5;

char szBTVPluginName[MAX_PATH];

BOOL bUseBTVPlugin = FALSE;

BTV_V1_PARAMS BTVParams = 
{
	NULL,
	NULL,
	NULL,
	NULL,
	0,
	0,
	0,
	0,
	1,
	0,
	0,
};

BOOL bIsFullScreen = FALSE;

char BTVendorID[10];
char BTDeviceID[10];
char MSPVersion[16];

