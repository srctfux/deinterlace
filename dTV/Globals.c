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

#include <windows.h>       /* required for all Windows applications */
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <process.h>       /* for _beginthread                      */
#include <stdlib.h>        /* atoi                                  */
#include <io.h>         
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ddraw.h>

#include "resource.h"
#include "defines.h"
#include "structs.h"
#include "globals.h"

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
	    944, 768, 186, 922, 0x20, 0 },
	/* NTSC Square Pixel */ 
	{ 640, 480,  910, 0x68, 0x5d, (BT848_IFORM_NTSC|BT848_IFORM_XT0),
	    780, 640, 135, 754, 0x1a, 0},
	/* SECAM */
	{ 922, 576, 1135, 0x7f, 0xa0, (BT848_IFORM_SECAM|BT848_IFORM_XT1),
	    1135, 922, 186, 922, 0x20, 0 },
	/* PAL-M */
	{ 754, 480,  910, 0x70, 0x5d, (BT848_IFORM_PAL_M|BT848_IFORM_XT0),
	    910, 754, 135, 754, 0x1a, 0},
	/* PAL-N */
	{ 922, 576, 1135, 0x7f, 0x72, (BT848_IFORM_PAL_N|BT848_IFORM_XT1),
	    1135, 922, 186, 922, 0x20, 0 },
	/* NTSC Japan*/
	{ 754, 480,  910, 0x70, 0x5d, (BT848_IFORM_NTSC_JAP|BT848_IFORM_XT0),
	    910, 754, 135, 754, 0x1a, 0},
	/* PAL Full Pixel */
	{ 922, 576, 1135, 0x7f, 0x72, (BT848_IFORM_PAL_BDGHI|BT848_IFORM_XT1),
	    1135, 922, 186, 922, 0x20, 0 },
	/* PAL CCIR601 */
	/* NTSC Full Pixel */
	{ 754, 480,  910, 0x70, 0x5d, (BT848_IFORM_NTSC|BT848_IFORM_XT0),
	    910, 754, 135, 754, 0x1a, 0},
	/* NTSC CCIR601 */ 
	{ 720, 480,  910, 0x68, 0x5d, (BT848_IFORM_NTSC|BT848_IFORM_XT0),
	    858, 720, 135, 754, 0x1a, 0},
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


struct TTunerType Tuners[9] =
{
    { 2244/*16*140.25*/,7412/*16*463.25*/,0x02,0x04,0x01,0x8e,0xc2,623},
	{ 2244/*16*140.25*/,7412/*16*463.25*/,0xa0,0x90,0x30,0x8e,0xc0,623},
	{ 2516/*16*157.25*/,7220/*16*451.25*/,0xA0,0x90,0x30,0x8e,0xc0,732},
	{ 2692/*16*168.25*/,7156/*16*447.25*/,0xA7,0x97,0x37,0x8e,0xc0,623},
	{ 0        ,0        ,0x00,0x00,0x00,0x00,0x00,000},
	{ 2692/*16*168.25*/,7156/*16*447.25*/,0xA0,0x90,0x30,0x8e,0xc0,623},
	{ 2516/*16*157.25*/,7412/*16*463.25*/,0x02,0x04,0x01,0x8e,0xc2,732},
	{ 2720/*16*170.00*/,7200/*16*450.00*/,0xa0,0x90,0x30,0x8e,0xc2,623},
	{ 0        ,0        ,0x00,0x00,0x00,0x00,0x00,000},

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

BOOL USETUNER=TRUE;
BOOL USECARD=TRUE;

int TVFormat;
int ColourFormat = 4;
int AudioSource = 4;
int VideoSource = 1;
int CardType = 0;
int CountryCode=0;
int TunerType = 0;
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

unsigned char InitialHue=0x00;
unsigned char InitialBrightness=0x00;
unsigned char InitialIFORM=0x00;
int InitialContrast=0xd8;
int InitialSaturationU=0xfe;
int InitialSaturationV=0xb4;
int InitialLow=45;

BOOL Has_MSP=FALSE;

BOOL Capture_Video = TRUE;
BOOL Capture_VBI = TRUE;
BOOL USE_DX_LOCK = FALSE;

LPDIRECTDRAW lpDD = NULL;

BOOL Toggle_WithOut_Frame = FALSE;

int VBI_Flags=3;

struct TProgramm Programm[MAXPROGS+1];

HFONT currFont = NULL;

BOOL VD_RAW=FALSE;

struct TChannels Channels;

struct SOTREC SOTInfoRec;

unsigned short UTPages[12];
unsigned short UTCount=0;

ePULLDOWNMODES gPulldownMode = VIDEO_MODE;