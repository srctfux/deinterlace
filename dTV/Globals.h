/////////////////////////////////////////////////////////////////////////////
// globals.h
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

#ifndef __GLOBALS_H___
#define __GLOBALS_H___

extern BITMAPINFO *VTCharSetLarge ;
extern BITMAPINFO *VTCharSetSmall;
extern BITMAPINFO *VTScreen[MAXVTDIALOG];
extern BOOL AutoStereoSelect;
extern BOOL bFilterBothRedBlue;
extern BOOL bIsOddField;
extern BOOL bNoBlue,bNoRed,bNoGreen;
extern BOOL bSaveSettings;
extern BOOL Capture_VBI;
extern BOOL Capture_Video;
extern BOOL Display_Status_Bar;
extern BOOL Has_MSP;
extern BOOL MSPToneControl;
extern BOOL System_In_Mute;
extern BOOL USE_MIXER;
extern BOOL USECARD;
extern BOOL USETUNER;
extern BOOL VD_RAW;
extern BOOL VT_ALWAYS_EXPORT;
extern BOOL VT_HEADERS;
extern BOOL VT_NL;
extern BOOL VT_REVEAL;
extern BOOL VT_STRIPPED;
extern BOOL VTLarge;
extern BYTE *pDisplay[5];
extern BYTE INIT_PLL;
extern BYTE VT_Header_Line[40];
extern char BTTyp[30];
extern char IC_BASE_DIR[128];
extern BOOL InitialSuperBass;
extern char InitialBalance;
extern char InitialBass;
extern char InitialEqualizer[5];
extern char InitialLoudness;
extern char InitialMixer[MAXPNAMELEN];
extern char InitialMixerConnect[MIXER_LONG_NAME_CHARS];
extern char InitialMixerLine[MIXER_LONG_NAME_CHARS];
extern char InitialSpatial;
extern char InitialTreble;
extern char MSPStatus[30];
extern char TunerStatus[30];
extern char VD_DIR[128];
extern char VPS_chname[9];
extern char VPS_lastname[9];
extern char VT_BASE_DIR[128];
extern HANDLE FilterEvent[MAXFILTER];
extern HANDLE hInst;
extern HMIXER hMixer;
extern HWND hWnd;
extern HWND hwndAudioField;
extern HWND hwndFPSField;
extern HWND hwndKeyField,hwndFPSField;
extern HWND hwndTextField, hwndPalField;
extern HWND ShowPDCInfo;
extern HWND ShowVPSInfo;
extern HWND ShowVTInfo;
extern HWND SplashWnd;
extern HWND hwndStatusBar;
extern int AnzahlProzessor;
extern int AusgabeProzessor;
extern int BytesOut;
extern int CardType;
extern int ColourFormat;
extern int CurrentCapture;
extern int CurrentConnect;
extern int CurrentLine;
extern int CurrentMixer;
extern int CurrentMode;
extern int CurrentPitch;
extern int CurrentProgramm;
extern int CurrentX;
extern int CurrentY;
extern int DeviceAnzahl;
extern int FirstWidthEven;
extern int FirstWidthOdd;
extern int InitialContrast;
extern int InitialLow;
extern int InitialProg;
extern int InitialSaturationU;
extern int InitialSaturationV;
extern int InitialVolume;
extern int LineFlag;
extern int MainProzessor;
extern int MasterTestzeilen;
extern int MIXER_LINKER_KANAL;
extern int MIXER_RECHTER_KANAL;
extern int MixerVolumeMax;
extern int MixerVolumeStep;
extern int MSPMajorMode;
extern int MSPMinorMode;
extern int MSPMode;
extern int MSPStereo;
extern int CurrentFrame;
extern int nPALplusData ;
extern int nPALplusZeile ;
extern int ProzessorMask;
extern int TunerType;
extern int TVTYPE;
extern int VBI_Flags;
extern int VBI_FPS;
extern int VBIProzessor;
extern int VideoSource;
extern int VT_Cache;
extern int VT_LATIN;
extern LPDIRECTDRAW lpDD;
extern LPDIRECTDRAWSURFACE lpDDOverlay;
extern LPDIRECTDRAWSURFACE lpDDOverlayBack;
extern LPDIRECTDRAWSURFACE lpDDSurface;
extern BYTE* lpOverlay;
extern BYTE* lpOverlayBack;
extern long OverlayPitch;
extern MIXERCAPS *MixerDev;
extern MIXERLINE *MixerLine;
extern short nLevelHigh;
extern short nLevelLow;
extern struct SOTREC SOTInfoRec;
extern struct TChannels Channels;
extern struct TCountries Countries[35];
extern struct TInterCast InterCast;
extern struct TLNB LNB;
extern struct TMixerAccess Mute;
extern struct TMixerAccess Volume;
extern struct TMixerLoad MixerLoad[64];
extern struct TPacket30 Packet30;
extern struct TTunerType Tuners[9];
extern struct TSoundSystem SoundSystem;
extern struct TVDat VDat;
extern struct TVT VTFrame[800];
extern struct TVTDialog VTDialog[MAXVTDIALOG];
extern struct TTVSetting TVSettings[10];
extern unsigned char *pBurstLine[5];
extern unsigned char InitialBrightness;
extern unsigned char InitialHue;
extern unsigned char InitialIFORM;
extern unsigned int ManuellAudio[8];
extern unsigned short cp_odd[256][285];
extern unsigned short UTCount;
extern unsigned short UTCount;
extern unsigned short UTPages[12];
extern unsigned short UTPages[12];
extern unsigned short VTColourTable[9];
extern PMemStruct Display_dma[5];
extern PMemStruct Burst_dma[5];
extern PMemStruct Risc_dma;
extern PMemStruct Vbi_dma[5];

extern int emsizex;
extern int emsizey;
extern int emstartx;
extern int emstarty;
extern BOOL bAlwaysOnTop;

extern int Res_X;
extern int Res_Y;

extern int pgsizex;
extern int pgsizey;
extern int pgstartx;
extern int pgstarty;

extern struct TBL ButtonList[15];

extern int PriorClassId;
extern int ThreadClassId;
extern BOOL bDisplayStatusBar;

extern BOOL USE_DX_LOCK;

extern BOOL Show_Menu;
extern BOOL Toggle_WithOut_Frame;
extern int AudioSource;

extern int CountryCode;

extern struct TProgramm Programm[MAXPROGS+1];
extern HFONT currFont;
extern HBITMAP BirneRot;
extern HBITMAP BirneGruen;

extern int VBI_lpf;

extern ePULLDOWNMODES gPulldownMode;

#endif


