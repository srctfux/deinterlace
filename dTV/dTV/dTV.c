/////////////////////////////////////////////////////////////////////////////
// dTV.c
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
#include "other.h"
#include "bt848.h"
#include "mixerdev.h"
#include "VBI_VideoText.h"
#include "AspectRatio.h"
#include "dTV.h"
#include "settings.h"
#include "ProgramList.h"
#include "Dialogs.h"
#include "OutThreads.h"
#include "bTVPlugin.h"
#include "audio.h"
#include "tuner.h"
#include "status.h"
#include "vbi.h"

HWND hwndStatusBar;
HWND hwndTextField;
HWND hwndPalField;
HWND hwndKeyField;
HWND hwndFPSField;
HWND hwndAudioField;

BOOL bDoResize = FALSE;

struct TVTDialog VTDialog[MAXVTDIALOG];

BOOL VTLarge=TRUE;

int CurrentProgramm;

HWND VThWnd;

int PriorClassId;
int ThreadClassId = 1;

SYSTEM_INFO SysInfo;

unsigned long freq;
char Typ;
unsigned int srate;         

struct TBL ButtonList[15];

BYTE  *pDisplay[5] =  { NULL,NULL,NULL,NULL,NULL  };

int MoveXDist=-1;
int MoveYDist=-1;


BITMAPINFO          *VTCharSetLarge                    = NULL;
BITMAPINFO          *VTCharSetSmall                    = NULL;
HBITMAP             BirneRot;
HBITMAP             BirneGruen;
BITMAPINFO          *VTScreen[MAXVTDIALOG];

long WStyle;

BOOL    Show_Menu=TRUE;
HMENU   hMenu;
HANDLE  hAccel;

HWND SplashWnd;

struct TPacket30 Packet30;


char ChannelString[10];

char BTTyp[30];
char MSPStatus[30] = "";

int LastFrame;


int CurrentX,CurrentY;

unsigned char *pBurstLine[5];

int FORMAT_MASK = 0x0F;
int PalFormat = 0;

BOOL  BlackSet[288];

LOGFONT lf = {16,0,0,0,400,0,0,0,0,0,0,0,0,"MS Sans Serif"};   

int BeforeVD=0;

int WriteIndex=1;


BOOL USE_MIXER=FALSE;
int  MIXER_LINKER_KANAL=-1;
int  MIXER_RECHTER_KANAL=-1;
int MixerVolumeStep=-1;
int MixerVolumeMax=-1;

// Fantasio for Secam-Extensions
short NullTable[4] = {8192, 0, 0, 8192};

int InitialProg=-1;

int NumberOfProcessors=1;
int MainProcessor=0;
int DecodeProcessor=0;


BOOL bShowCursor = TRUE;


/****************************************************************************

    FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)

    PURPOSE: calls initialization function, processes message loop

****************************************************************************/

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASS wc;
	HINSTANCE statusbar;
	MSG msg;
	HGLOBAL hGlobal;
	HWND hPrevWindow;
	int i;

	hInst = hInstance;

	// if we are already runninmg then start up old version
	hPrevWindow = FindWindow((LPCTSTR) "dTV", (LPCTSTR) "dTV");
	if (hPrevWindow != NULL)
	{
		SetFocus(hPrevWindow);
		SetActiveWindow(hPrevWindow);
		SetForegroundWindow(hPrevWindow);
		return FALSE;
	}

	SplashWnd = CreateDialog(hInst, "SPLASHBOX", NULL, SplashProc);
#ifndef _DEBUG
	SetWindowPos(SplashWnd, HWND_TOPMOST, 10, 10, 20, 20, SWP_NOMOVE | SWP_NOCOPYBITS | SWP_NOSIZE);
#endif

	LoadSettingsFromIni();

	// try to load up bTV plugin
	bUseBTVPlugin = BTVPluginLoad(szBTVPluginName);

	if (strlen(IC_BASE_DIR) > 0)
	{
		i = (int) CreateDirectory(IC_BASE_DIR, NULL);
	}
	if (strlen(VD_DIR) > 0)
	{
		i = (int) CreateDirectory(VD_DIR, NULL);
	}
	if (strlen(VT_BASE_DIR) > 0)
	{
		i = (int) CreateDirectory(VT_BASE_DIR, NULL);
	}

	wc.style = 0;
	wc.lpfnWndProc = (WNDPROC) MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(LONG);
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, "DTVICON");
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = CreateSolidBrush(0);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "dTV";

	if (!RegisterClass(&wc))
	{
		return FALSE;
	}
	hMenu = LoadMenu(hInstance, "ANALOGMENU");
	hWnd = CreateWindow("dTV", "dTV", WS_OVERLAPPEDWINDOW, emstartx, emstarty, emsizex, emsizey, NULL, NULL, hInstance, NULL);

	if (!hWnd)
		return (FALSE);

	statusbar = hInst;
	if (!StatusBar_Init(statusbar))
	{
		return FALSE;
	}

	if (StatusBar_Create(hWnd, statusbar, ID_STATUSBAR))
	{
		hwndTextField = StatusBar_AddField(statusbar, ID_TEXTFIELD, 110, 0, FALSE);
		hwndAudioField = StatusBar_AddField(statusbar, ID_AUDIOFIELD, 110, 0, FALSE);
		hwndPalField = StatusBar_AddField(statusbar, ID_CODEFIELD, 110, 0, FALSE);
		hwndKeyField = StatusBar_AddField(statusbar, ID_KENNUNGFFIELD, 90, 50, FALSE);
		hwndFPSField = StatusBar_AddField(statusbar, ID_FPSFIELD, 45, 45, TRUE);
	}
	else
	{
		return FALSE;
	}

	StatusBar_Adjust(hWnd);

	if (bDisplayStatusBar == FALSE)
	{
		ShowWindow(hwndStatusBar, SW_HIDE);
	}

	hGlobal = LoadResource(hInst, FindResource(hInst, "VTCHARLARGE", RT_BITMAP));
	VTCharSetLarge = (BITMAPINFO *) LockResource(hGlobal);
	hGlobal = LoadResource(hInst, FindResource(hInst, "VTCHARSMALL", RT_BITMAP));
	VTCharSetSmall = (BITMAPINFO *) LockResource(hGlobal);

	BirneRot = LoadBitmap(hInst, "BROT");
	BirneGruen = LoadBitmap(hInst, "BGRUEN");
	currFont = CreateFontIndirect(&lf);

	ShowWindow(hWnd, nCmdShow);
	PostMessage(hWnd, WM_SIZE, SIZENORMAL, MAKELONG(emsizex, emsizey));
	if (!(hAccel = LoadAccelerators(hInstance, "ANALOGACCEL")))
	{
		ErrorBox("Accelerators not Loaded");
	}

	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(hWnd, hAccel, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	DeleteObject(currFont);
	DeleteObject(VTCharSetLarge);
	DeleteObject(VTCharSetSmall);
	DeleteObject(BirneRot);
	DeleteObject(BirneGruen);
	
	// unload any bTV plugin loaded
	BTVPluginUnload();

	ExitDD();
	// save settings
	WriteSettingsToIni();
	return msg.wParam;
}

/****************************************************************************

    FUNCTION: MainWndProc(HWND, unsigned, WORD, LONG)

    PURPOSE:  Processes messages

    MESSAGES:

        WM_COMMAND    - application menu (About dialog box)
        WM_CREATE     - create window and objects
        WM_PAINT      - update window, draw objects
        WM_DESTROY    - destroy window

    COMMENTS:

        Handles to the objects you will use are obtained when the WM_CREATE
        message is received, and deleted when the WM_DESTROY message is
        received.  The actual drawing is done whenever a WM_PAINT message is
        received.

****************************************************************************/
LONG APIENTRY MainWndProc(HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
	char Text[128];
	char Text1[128];
	int i, j, k;

	switch (message)
	{

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_SETUPCARD:
			Stop_Capture();
			DialogBox(hInst, "SELECTCARD", hWnd, (DLGPROC) SelectCardProc);
			Card_Init(CardType);
			Tuner_Init(TunerType);
			WorkoutOverlaySize();
			Start_Capture();
			break;

		case IDM_CLOSE_VT:
			for (i = 0; i < MAXVTDIALOG; i++)
			{
				if (VTDialog[i].Dialog != NULL)
					SendMessage(VTDialog[i].Dialog, WM_COMMAND, IDCANCEL, 0);
			}
			SetFocus(hWnd);
			break;

		case IDM_VT_PAGE_MINUS:
			for (i = 0; i < MAXVTDIALOG; i++)
			{
				if (VTDialog[i].Dialog != NULL)
				{
					j = Get_Dialog_Slot(VTDialog[i].Dialog);
					if (VTDialog[j].Page > 100)
					{
						SetDlgItemInt(VTDialog[j].Dialog, IDC_EDIT1, VTDialog[j].Page - 1, FALSE);
						VTDialog[j].PageChange = TRUE;
					}
				}
			}
			return (TRUE);

		case IDM_VT_PAGE_PLUS:
			for (i = 0; i < MAXVTDIALOG; i++)
			{
				if (VTDialog[i].Dialog != NULL)
				{
					j = Get_Dialog_Slot(VTDialog[i].Dialog);
					if (VTDialog[j].Page < 899)
					{
						SetDlgItemInt(VTDialog[j].Dialog, IDC_EDIT1, VTDialog[j].Page + 1, FALSE);
						VTDialog[j].PageChange = TRUE;
					}
				}
			}
			return (TRUE);

		case IDM_VT_PAGE_UP:
			for (i = 0; i < MAXVTDIALOG; i++)
			{
				if (VTDialog[i].Dialog != NULL)
				{
					j = Get_Dialog_Slot(VTDialog[i].Dialog);
					k = VTDialog[j].SubPage;
					if (VTFrame[VTDialog[j].FramePos].SubCount == 0)
						return (TRUE);
					k--;
					while ((k >= 0) && (VTFrame[VTDialog[j].FramePos].SubPage[k].Fill == FALSE))
						k--;
					if (k < 0)
						k = VTFrame[VTDialog[j].FramePos].SubCount - 1;	//DF:i
					while ((k >= 0) && (VTFrame[VTDialog[j].FramePos].SubPage[k].Fill == FALSE))
						k--;
					if (k < 0)
						return (TRUE);

					if ((k >= 0) && (VTFrame[VTDialog[j].FramePos].SubPage[k].Fill == TRUE))
						VTDialog[j].SubPage = k;	//DF: else...
					SetDlgItemInt(VTDialog[j].Dialog, IDC_EDIT1, VTDialog[j].Page, FALSE);
				}
			}
			return (TRUE);

		case IDM_VT_PAGE_DOWN:
			for (i = 0; i < MAXVTDIALOG; i++)
			{
				if (VTDialog[i].Dialog != NULL)
				{
					j = Get_Dialog_Slot(VTDialog[i].Dialog);
					k = VTDialog[j].SubPage;
					if (VTFrame[VTDialog[j].FramePos].SubCount == 0)
						return (TRUE);
					k++;
					while ((k <= VTFrame[VTDialog[j].FramePos].SubCount - 1) && (VTFrame[VTDialog[j].FramePos].SubPage[k].Fill == FALSE))
						k++;
					if (k >= VTFrame[VTDialog[j].FramePos].SubCount)
						k = 0;	//DF:i
					while ((k <= VTFrame[VTDialog[j].FramePos].SubCount - 1) && (VTFrame[VTDialog[j].FramePos].SubPage[k].Fill == FALSE))
						k++;
					if (k >= VTFrame[VTDialog[j].FramePos].SubCount)
						return (TRUE);

					//k++;
					//if ( k >= VTFrame[VTDialog[j].FramePos].SubCount ) k=0;
					//while (( k < VTFrame[VTDialog[j].FramePos].SubCount) && ( VTFrame[VTDialog[j].FramePos].SubPage[k].Fill == FALSE )) k++;
					if ((k < VTFrame[VTDialog[j].FramePos].SubCount) && (VTFrame[VTDialog[j].FramePos].SubPage[k].Fill == TRUE))
						VTDialog[j].SubPage = k;
					else
						return (TRUE);
					SetDlgItemInt(VTDialog[j].Dialog, IDC_EDIT1, VTDialog[j].Page, FALSE);
				}
			}
			return (TRUE);

		case IDM_CHANNELPLUS:
			ChangeChannel(CurrentProgramm + 1);
			if (bDisplayStatusBar == TRUE)
			{
				sprintf(Text, "Channel %d", CurrentProgramm);
				SetWindowText(hwndTextField, Text);
			}
			break;

		case IDM_CHANNELMINUS:
			ChangeChannel(CurrentProgramm - 1);
			if (bDisplayStatusBar == TRUE)
			{
				sprintf(Text, "Channel %d", CurrentProgramm);
				SetWindowText(hwndTextField, Text);
			}
			break;

		case IDM_RESET:
			Stop_Capture();
			Overlay_Clean();
			BT848_ResetHardware();
			BT848_SetGeoSize();
			WorkoutOverlaySize();
			Start_Capture();
			Sleep(100);
			Audio_SetSource(AudioSource);
			break;

		case IDM_TOGGLE_MENU:
			Show_Menu = !Show_Menu;
			WorkoutOverlaySize();
			break;

		case IDM_AUTODETECT:
			bAutoDetectMode = !bAutoDetectMode;
			if(bAutoDetectMode == TRUE)
			{
				gPulldownMode = VIDEO_MODE_BOB;
				UpdatePulldownStatus();
			}
			SetMenuAnalog();
			break;

		case IDM_FALLBACK:
			bFallbackToVideo = !bFallbackToVideo;
			SetMenuAnalog();
			break;

		case IDM_32PULL1:
		case IDM_32PULL2:
		case IDM_32PULL3:
		case IDM_32PULL4:
		case IDM_32PULL5:
		case IDM_22PULLODD:
		case IDM_22PULLEVEN:
		case IDM_WEAVE:
		case IDM_BOB:
		case IDM_VIDEO_WEAVE:
		case IDM_VIDEO_BOB:
			bAutoDetectMode = FALSE;
			gPulldownMode = LOWORD(wParam) - IDM_VIDEO_BOB;
			UpdatePulldownStatus();
			SetMenuAnalog();
			break;

		case IDM_ABOUT:
			DialogBox(hInst, "ABOUT", hWnd, AboutProc);
			break;

		case IDM_BRIGHTNESSPLUS:
			if (InitialBrightness < 127)
				InitialBrightness++;
			BT848_SetBrightness(InitialBrightness);
			if (bDisplayStatusBar == TRUE)
			{
				sprintf(Text, "Brightness %d", InitialBrightness);
				SetWindowText(hwndTextField, Text);
			}
			break;

		case IDM_BRIGHTNESSMINUS:
			if (InitialBrightness > -127)
				InitialBrightness--;
			BT848_SetBrightness(InitialBrightness);
			if (bDisplayStatusBar == TRUE)
			{
				sprintf(Text, "Brightness %d", InitialBrightness);
				SetWindowText(hwndTextField, Text);
			}
			break;

		case IDM_COLORPLUS:
			if ((InitialSaturationU < 255) && (InitialSaturationV < 255))
			{
				InitialSaturationU++;
				InitialSaturationV++;
				BT848_SetSaturationU(InitialSaturationU);
				BT848_SetSaturationV(InitialSaturationV);
				if (bDisplayStatusBar == TRUE)
				{
					sprintf(Text, "Colour U %d V %d", InitialSaturationU, InitialSaturationV);
					SetWindowText(hwndTextField, Text);
				}
			}
			break;

		case IDM_COLORMINUS:
			if ((InitialSaturationU > 0) && (InitialSaturationV > 0))
			{
				InitialSaturationU--;
				InitialSaturationV--;
				BT848_SetSaturationU(InitialSaturationU);
				BT848_SetSaturationV(InitialSaturationV);
				if (bDisplayStatusBar == TRUE)
				{
					sprintf(Text, "Colour U %d V %d", InitialSaturationU, InitialSaturationV);
					SetWindowText(hwndTextField, Text);
				}
			}
			break;

		case IDM_HUE_DOWN:
			if ((InitialHue > -127))
			{
				InitialHue--;
				BT848_SetHue(InitialHue);
				if (bDisplayStatusBar == TRUE)
				{
					sprintf(Text, "Hue %d", InitialHue);
					SetWindowText(hwndTextField, Text);
				}
			}
			break;

		case IDM_HUE_UP:
			if ((InitialHue < 127))
			{
				InitialHue++;
				if (bDisplayStatusBar == TRUE)
				{
					sprintf(Text, "Hue %d", InitialHue);
					SetWindowText(hwndTextField, Text);
				}
				BT848_SetHue(InitialHue);
			}
			break;

		case IDM_KONTRASTPLUS:
			if (InitialContrast < 256)
				InitialContrast++;
			BT848_SetContrast(InitialContrast);
			if (bDisplayStatusBar == TRUE)
			{
				sprintf(Text, "Contrast %d", InitialContrast);
				SetWindowText(hwndTextField, Text);
			}
			break;

		case IDM_KONTRASTMINUS:
			if (InitialContrast > 0)
				InitialContrast--;
			BT848_SetContrast(InitialContrast);
			if (bDisplayStatusBar == TRUE)
			{
				sprintf(Text, "Contrast %d", InitialContrast);
				SetWindowText(hwndTextField, Text);
			}
			break;

		case IDM_OVERSCAN_PLUS:
			if(InitialOverscan < 127)
			{
				InitialOverscan++;
				WorkoutOverlaySize();
				if (bDisplayStatusBar == TRUE)
				{
					sprintf(Text, "Overscan %d", InitialOverscan);
					SetWindowText(hwndTextField, Text);
				}
			}
			break;

		case IDM_OVERSCAN_MINUS:
			if(InitialOverscan > 0)
			{
				InitialOverscan--;
				WorkoutOverlaySize();
				if (bDisplayStatusBar == TRUE)
				{
					sprintf(Text, "Overscan %d", InitialOverscan);
					SetWindowText(hwndTextField, Text);
				}
			}
			break;

		case IDM_MUTE:
			if (System_In_Mute == FALSE)
			{
				System_In_Mute = TRUE;
				if (USE_MIXER == FALSE)
				{
					Audio_SetSource(AUDIOMUX_MUTE);
					sprintf(Text, "Mute BT");
				}
				if (USE_MIXER == TRUE)
				{
					Mixer_Mute();
					sprintf(Text, "Mute Mixer");
				}
			}
			else
			{
				System_In_Mute = FALSE;
				if (USE_MIXER == FALSE)
				{
					Audio_SetSource(AudioSource);
					sprintf(Text, "UnMute BT");
				}
				if (USE_MIXER == TRUE)
				{
					Mixer_UnMute();
					sprintf(Text, "UnMute Mixer");
				}

			}
			if (bDisplayStatusBar == TRUE)
				SetWindowText(hwndTextField, Text);
			break;

		case IDM_L_BALANCE:
			if (USE_MIXER == FALSE)
			{
				if (InitialBalance > -127)
					InitialBalance--;
				Audio_SetBalance(InitialBalance);	// -127 - +128
				sprintf(Text, "BT-Balance %d", InitialBalance);
			}
			if (USE_MIXER == TRUE)
			{
				if (MIXER_LINKER_KANAL + MixerVolumeStep <= MixerVolumeMax)
					MIXER_LINKER_KANAL += MixerVolumeStep;
				if (MIXER_RECHTER_KANAL - MixerVolumeStep >= 0)
					MIXER_RECHTER_KANAL -= MixerVolumeStep;
				Mixer_Set_Volume(MIXER_LINKER_KANAL, MIXER_RECHTER_KANAL);
				sprintf(Text, "Balance L %d R %d", MIXER_LINKER_KANAL, MIXER_RECHTER_KANAL);
			}

			if (bDisplayStatusBar == TRUE)
				SetWindowText(hwndTextField, Text);
			break;

		case IDM_R_BALANCE:
			if (USE_MIXER == FALSE)
			{
				if (InitialBalance < 128)
					InitialBalance++;
				Audio_SetBalance(InitialBalance);	// -127 - +128
				sprintf(Text, "BT-Balance %d", InitialBalance);
			}
			if (USE_MIXER == TRUE)
			{
				if (MIXER_LINKER_KANAL - MixerVolumeStep >= 0)
					MIXER_LINKER_KANAL -= MixerVolumeStep;
				if (MIXER_RECHTER_KANAL + MixerVolumeStep <= MixerVolumeMax)
					MIXER_RECHTER_KANAL += MixerVolumeStep;
				Mixer_Set_Volume(MIXER_LINKER_KANAL, MIXER_RECHTER_KANAL);
				sprintf(Text, "Balance L %d R %d", MIXER_LINKER_KANAL, MIXER_RECHTER_KANAL);
			}

			if (bDisplayStatusBar == TRUE)
				SetWindowText(hwndTextField, Text);
			break;

		case IDM_VOLUMEPLUS:
			if (USE_MIXER == FALSE)
			{
				if (InitialVolume < 980)
					InitialVolume += 20;
				Audio_SetVolume(InitialVolume);
				sprintf(Text, "BT-Volume %d", InitialVolume);
			}
			if (USE_MIXER == TRUE)
			{
				i = ((MixerVolumeMax / MixerVolumeStep) / 100) * MixerVolumeStep;
				if ((i + MIXER_LINKER_KANAL > MixerVolumeMax) || (i + MIXER_RECHTER_KANAL > MixerVolumeMax))
					i = MixerVolumeStep;
				if (MIXER_LINKER_KANAL + i <= MixerVolumeMax)
					MIXER_LINKER_KANAL += i;
				if (MIXER_RECHTER_KANAL + i <= MixerVolumeMax)
					MIXER_RECHTER_KANAL += i;
				Mixer_Set_Volume(MIXER_LINKER_KANAL, MIXER_RECHTER_KANAL);
				sprintf(Text, "Volume L %d R %d", MIXER_LINKER_KANAL, MIXER_RECHTER_KANAL);
			}

			if (bDisplayStatusBar == TRUE)
				SetWindowText(hwndTextField, Text);
			break;
		case IDM_VOLUMEMINUS:
			if (USE_MIXER == FALSE)
			{
				if (InitialVolume > 20)
					InitialVolume -= 20;
				Audio_SetVolume(InitialVolume);
				sprintf(Text, "BT-Volume %d", InitialVolume);
			}
			if (USE_MIXER == TRUE)
			{
				i = ((MixerVolumeMax / MixerVolumeStep) / 100) * MixerVolumeStep;
				if ((MIXER_LINKER_KANAL - i < 0) || (MIXER_RECHTER_KANAL - i < 0))
					i = MixerVolumeStep;
				if (MIXER_LINKER_KANAL - i >= 0)
					MIXER_LINKER_KANAL -= i;
				if (MIXER_RECHTER_KANAL - i >= 0)
					MIXER_RECHTER_KANAL -= i;
				Mixer_Set_Volume(MIXER_LINKER_KANAL, MIXER_RECHTER_KANAL);
				sprintf(Text, "Volume L %d R %d", MIXER_LINKER_KANAL, MIXER_RECHTER_KANAL);

			}

			if (bDisplayStatusBar == TRUE)
				SetWindowText(hwndTextField, Text);
			break;

		case IDM_TOGGLECURSOR:
			if(bIsFullScreen == FALSE)
			{
				bShowCursor = !bShowCursor;
				ShowCursor(bShowCursor);
			}
			break;

		case IDM_END:
			ShowWindow(hWnd, SW_HIDE);
			PostMessage(hWnd, WM_DESTROY, wParam, lParam);
			break;

		case IDM_MSPMODE_3:
		case IDM_MSPMODE_2:
		case IDM_MSPMODE_4:
		case IDM_MSPMODE_5:
		case IDM_MSPMODE_6:
			Audio_MSP_SetMode(LOWORD(wParam) - (IDM_MSPMODE_2 - 2));
			SetMenuAnalog();
			break;

		case IDM_MAJOR_CARRIER_0:
		case IDM_MAJOR_CARRIER_1:
		case IDM_MAJOR_CARRIER_2:
		case IDM_MAJOR_CARRIER_3:
			Audio_MSP_Set_MajorMinor_Mode(LOWORD(wParam) - IDM_MAJOR_CARRIER_0, MSPMinorMode);
			SetMenuAnalog();
			break;

		case IDM_MINOR_CARRIER_0:
		case IDM_MINOR_CARRIER_1:
		case IDM_MINOR_CARRIER_2:
		case IDM_MINOR_CARRIER_3:
		case IDM_MINOR_CARRIER_4:
		case IDM_MINOR_CARRIER_5:
		case IDM_MINOR_CARRIER_6:
		case IDM_MINOR_CARRIER_7:
			Audio_MSP_Set_MajorMinor_Mode(MSPMajorMode, LOWORD(wParam) - IDM_MINOR_CARRIER_0);
			SetMenuAnalog();
			break;

		case IDM_MSPSTEREO_1:
		case IDM_MSPSTEREO_2:
		case IDM_MSPSTEREO_3:
		case IDM_MSPSTEREO_4:
			Audio_MSP_SetStereo(MSPMajorMode, MSPMinorMode, LOWORD(wParam) - (IDM_MSPSTEREO_1 - 1));
			SetMenuAnalog();
			break;

		case IDM_AUTOSTEREO:
			AutoStereoSelect = !AutoStereoSelect;
			SetMenuAnalog();
			break;

		case IDM_AUDIO_0:
		case IDM_AUDIO_1:
		case IDM_AUDIO_2:
		case IDM_AUDIO_3:
		case IDM_AUDIO_4:
		case IDM_AUDIO_5:
			AudioSource = LOWORD(wParam) - IDM_AUDIO_0;
			Stop_Capture();
			Audio_SetSource(AudioSource);
			Start_Capture();
			SetMenuAnalog();
			break;

		case IDM_TUNER:
			VideoSource = 0;
			AudioSource = AUDIOMUX_TUNER;
			Stop_Capture();
			BT848_SetVideoSource(VideoSource);
			if (bDisplayStatusBar == TRUE)
			{
				sprintf(Text, "%04d. %s ", CurrentProgramm + 1, Programm[CurrentProgramm].Name);
				SetWindowText(hwndKeyField, Text);
			}
			if(!System_In_Mute)
			{
				Audio_SetSource(AudioSource);
			}
			Start_Capture();
			SetMenuAnalog();
			break;

		case IDM_EXTERN1:
		case IDM_EXTERN2:
		case IDM_EXTERN3:
		case IDM_EXTERN4:
		case IDM_EXTERN5:

			// 10/16/2000 by Mark Rejhon:
			// More guaranteed to work than mathematics with wParam
			VideoSource = 0;
			switch (LOWORD(wParam)) {
			case IDM_EXTERN1: VideoSource = 1; break;
			case IDM_EXTERN2: VideoSource = 2; break;
			case IDM_EXTERN3: VideoSource = 3; break;
			case IDM_EXTERN4: VideoSource = 4; break;
			case IDM_EXTERN5: VideoSource = 5; break;
			}

			AudioSource = AUDIOMUX_EXTERNAL;
			Stop_Capture();
			BT848_ResetHardware();
			BT848_SetGeoSize();
			WorkoutOverlaySize();
			BT848_SetVideoSource(VideoSource);
			if (bDisplayStatusBar == TRUE)
			{
				sprintf(Text, "Extern %d", VideoSource);
				SetWindowText(hwndKeyField, Text);
			}
			if(!System_In_Mute)
			{
				Audio_SetSource(AudioSource);
			}
			Start_Capture();
			SetMenuAnalog();
			break;

		case IDM_HWINFO:
			DialogBox(hInst, "HWINFO", hWnd, (DLGPROC) ChipSettingProc);
			break;

		case IDM_VBI_VT:
			if (VBI_Flags & VBI_VT)
			{
				VBI_Flags -= VBI_VT;
			}
			else
			{
				VBI_Flags += VBI_VT;
			}
			SetMenuAnalog();
			break;
	
		case IDM_CLOSEDCAPTION:
			if (VBI_Flags & VBI_CC)
			{
				VBI_Flags -= VBI_CC;
			}
			else
			{
				VBI_Flags += VBI_CC;
			}
			SetMenuAnalog();
			break;

		case IDM_VBI_VD:
			if (VBI_Flags & VBI_VD)
			{
				if (BeforeVD != 7)
				{
					VBI_Flags -= VBI_VD;
					VideoDat_Exit();
					VBI_lpf = 19;
					BT848_MakeVBITable(VBI_lpf);
					TVTYPE = BeforeVD;
					break;
				}
				VBI_Flags -= VBI_VD;
			}
			else
			{
				BeforeVD = TVTYPE;
				if (TVTYPE != 7)
				{
					VBI_Flags += VBI_VD;	// 
					VBI_lpf = 19;
					BT848_MakeVBITable(VBI_lpf);
					TVTYPE = 7;
					break;
				}
				VideoDat_Init();
				VBI_Flags += VBI_VD;
			}
			SetMenuAnalog();
			break;

		case IDM_VBI_VPS:
			if (VBI_Flags & VBI_VPS)
			{
				VBI_Flags -= VBI_VPS;
			}
			else
			{
				VBI_Flags += VBI_VPS;
			}
			SetMenuAnalog();
			break;

		case IDM_VBI_IC:
			if (VBI_Flags & VBI_IC)
			{
				VBI_Flags -= VBI_IC;
			}
			else
			{
				VBI_Flags += VBI_IC;
			}
			SetMenuAnalog();
			break;

		case IDM_UNTERTITEL:
			VThWnd = CreateDialog(hInst, "VIDEOTEXTUNTERTITEL", NULL, VideoTextUnterTitelProc);
			if (bAlwaysOnTop == TRUE)
				SetWindowPos(VThWnd, HWND_TOPMOST, 10, 10, 20, 20, SWP_NOMOVE | SWP_NOCOPYBITS | SWP_NOSIZE);
			break;

		case IDM_VIDEODAT_SETUP:
			DialogBox(hInst, "VDSETUP", hWnd, VDSettingProc);
			break;

		case IDM_VT_SETUP:
			DialogBox(hInst, "VTSETUP", hWnd, VTSettingProc);
			break;

		case IDM_INTERCAST_SETUP:
			DialogBox(hInst, "ICSETUP", hWnd, ICSettingProc);
			break;

		case IDM_CALL_VIDEOTEXTSMALL:
			VTLarge = FALSE;
			VThWnd = CreateDialog(hInst, "VIDEOTEXTSMALL", NULL, VideoTextProc);
			if (bAlwaysOnTop == TRUE)
				SetWindowPos(VThWnd, HWND_TOPMOST, 10, 10, 20, 20, SWP_NOMOVE | SWP_NOCOPYBITS | SWP_NOSIZE);
			break;

		case IDM_CALL_VIDEOTEXT:
			VTLarge = TRUE;
			VThWnd = CreateDialog(hInst, "VIDEOTEXT", NULL, VideoTextProc);
			if (bAlwaysOnTop == TRUE)
				SetWindowPos(VThWnd, HWND_TOPMOST, 10, 10, 20, 20, SWP_NOMOVE | SWP_NOCOPYBITS | SWP_NOSIZE);
			break;

		case IDM_VT_RESET:
			VT_ChannelChange();
			break;

		case IDM_AUDIOSETTINGS:
			DialogBox(hInst, "AUDIOEINSTELLUNGEN", hWnd, AudioSettingProc);
			break;

		case IDM_AUDIOSETTINGS1:
			DialogBox(hInst, "AUDIOEINSTELLUNGEN1", hWnd, AudioSettingProc1);
			break;

		case IDM_VIDEOSETTINGS:
			DialogBox(hInst, "VIDEOSETTINGS", hWnd, VideoSettingProc);
			WorkoutOverlaySize();
			break;
		
		case IDM_ADV_VIDEOSETTINGS:
			DialogBox(hInst, "ADV_VIDEOSETTINGS", hWnd, AdvVideoSettingProc);
			break;

		case IDM_VPS_OUT:
			DialogBox(hInst, "VPSSTATUS", hWnd, VPSInfoProc);
			break;

		case IDM_IC_OUT:
			DialogBox(hInst, "ICSTATUS", hWnd, ICInfoProc);
			break;

		case IDM_VD_OUT:
			if (VD_RAW == TRUE)
				DialogBox(hInst, "VDSTATUSRAW", hWnd, VDInfoProcRaw);
			else
				DialogBox(hInst, "VDSTATUS", hWnd, VDInfoProc);
			break;

		case IDM_VT_OUT:
			DialogBox(hInst, "VTSTATUS", hWnd, VTInfoProc);
			break;

		case IDM_VBI:
			Stop_Capture();
			Capture_VBI = !Capture_VBI;
			Start_Capture();
			SetMenuAnalog();
			break;

		case IDM_CAPTURE_PAUSE:
			bIsPaused = !bIsPaused;
			CheckMenuItem(GetMenu(hWnd), IDM_CAPTURE_PAUSE, bIsPaused?MF_CHECKED:MF_UNCHECKED);
			break;

		case IDM_AUDIO_MIXER:
			DialogBox(hInst, "MIXERSETUP", hWnd, MixerSetupProc);
			break;

		case IDM_STATUSBAR:
			bDisplayStatusBar = !bDisplayStatusBar;
			if(bDisplayStatusBar == TRUE)
			{
				SetTimer(hWnd, TIMER_STATUS, TIMER_STATUS_MS, NULL);
			}
			else
			{
				KillTimer(hWnd, TIMER_STATUS);
			}
			WorkoutOverlaySize();
			SetMenuAnalog();
			break;

		case IDM_ON_TOP:
			bAlwaysOnTop = !bAlwaysOnTop;
			WorkoutOverlaySize();
			SetMenuAnalog();
			break;

		case IDM_ANALOGSCAN:
			SendMessage(hWnd, WM_COMMAND, IDM_TUNER, 0);
			DialogBox(hInst, "ANALOGSCAN", hWnd, (DLGPROC) AnalogScanProc);
			break;

		case IDM_CHANNEL_LIST:
			DialogBox(hInst, "CHANNELLIST", hWnd, (DLGPROC) ProgramListProc);
			break;

		case IDM_TREADPRIOR_0:
		case IDM_TREADPRIOR_1:
		case IDM_TREADPRIOR_2:
		case IDM_TREADPRIOR_3:
		case IDM_TREADPRIOR_4:
			ThreadClassId = LOWORD(wParam) - IDM_TREADPRIOR_0;
			Stop_Capture();
			Start_Capture();
			SetMenuAnalog();
			break;

		case IDM_PRIORCLASS_0:
		case IDM_PRIORCLASS_1:
		case IDM_PRIORCLASS_2:
			PriorClassId = LOWORD(wParam) - IDM_PRIORCLASS_0;
			strcpy(Text, "Can't set Priority");
			if (PriorClassId == 0)
			{
				if (SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS) == TRUE)
					strcpy(Text, "Normal Priority");
			}
			else if (PriorClassId == 1)
			{
				if (SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS) == TRUE)
					strcpy(Text, "High Priority");
			}
			else
			{
				if (SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS) == TRUE)
					strcpy(Text, "Real-Time Priority");
			}
			SetWindowText(hwndTextField, Text);
			SetMenuAnalog();
			break;

		case IDM_TYPEFORMAT_0:
		case IDM_TYPEFORMAT_1:
		case IDM_TYPEFORMAT_2:
		case IDM_TYPEFORMAT_3:
		case IDM_TYPEFORMAT_4:
		case IDM_TYPEFORMAT_5:
		case IDM_TYPEFORMAT_6:
		case IDM_TYPEFORMAT_7:
		case IDM_TYPEFORMAT_8:
		case IDM_TYPEFORMAT_9:
			TVTYPE = LOWORD(wParam) - 1120;
			Stop_Capture();
			BT848_SetGeoSize();
			WorkoutOverlaySize();
			Start_Capture();
			SetMenuAnalog();
			break;

		case IDM_SPACEBAR:
			if(bAutoDetectMode == FALSE)
			{
				gPulldownMode++;
				// if we can't use a bTV plug-in then skip it
				if(gPulldownMode == BTV_PLUGIN && bUseBTVPlugin == FALSE)
				{
					gPulldownMode++;
				}
				else if(gPulldownMode == PULLDOWNMODES_LAST_ONE)
				{
					gPulldownMode = VIDEO_MODE_BOB;
				}
				UpdatePulldownStatus();
			}
			break;

		case IDM_SHIFT_SPACEBAR:
			if (bAutoDetectMode == FALSE)
			{
				if (gPulldownMode == VIDEO_MODE_BOB)
					gPulldownMode = PULLDOWNMODES_LAST_ONE;
				gPulldownMode--;
				if (gPulldownMode == BTV_PLUGIN && bUseBTVPlugin == FALSE)
					gPulldownMode--;
				UpdatePulldownStatus();
			}
			break;

		case IDM_FULL_SCREEN:
			bDoResize = FALSE;
			bIsFullScreen = !bIsFullScreen;
			if(bIsFullScreen == FALSE)
			{
				SetWindowPos(hWnd, 0, emstartx, emstarty, emsizex, emsizey, SWP_SHOWWINDOW);
				if(bShowCursor)
				{
					ShowCursor(TRUE);
				}
				if (bDisplayStatusBar == TRUE)
				{
					SetTimer(hWnd, TIMER_STATUS, TIMER_STATUS_MS, NULL);
				}
			}
			else
			{
				SaveWindowPos(hWnd);
				if(bShowCursor)
				{
					ShowCursor(FALSE);
				}
			}
			WorkoutOverlaySize();
			bDoResize = TRUE;
			break;

		case IDM_TAKESTILL:
			bIsPaused = TRUE;
			Sleep(100);
			SaveStill();
			bIsPaused = FALSE;
			Sleep(100);
			break;

		default:
			// Check whether menu ID is an aspect ratio related item
			ProcessAspectRatioSelection(hWnd, LOWORD(wParam));
			break;
		}
		
		break;

	case WM_CREATE:
		MainWndOnCreate(hWnd);
		break;

	case INIT_BT:
		MainWndOnInitBT(hWnd);
		break;

	case WM_LBUTTONUP:
		SendMessage(hWnd, WM_COMMAND, IDM_FULL_SCREEN, 0);
		break;

	case WM_RBUTTONUP:
		if(bIsFullScreen == FALSE)
		{
			bShowCursor = !bShowCursor;
			ShowCursor(bShowCursor);
		}
		break;

	case WM_TIMER:
		if (wParam == TIMER_MSP)
		{
			if (bDisplayStatusBar == TRUE)
				Audio_MSP_Print_Mode();
			if (AutoStereoSelect == TRUE)
				Audio_MSP_Watch_Mode();
		}
		else if (wParam == TIMER_STATUS)
		{
			if (!BT848_IsVideoPresent())
			{
				SetWindowText(hwndTextField, "No Video Signal Found");
			}
			else
			{
				Text[0] = 0x00;
				if (Packet30.Identifier[0] != 0x00)
				{
					sprintf(Text, "%s ", Packet30.Identifier);
					Packet30.Identifier[0] = 0x00;
				}
				else if (VPS_lastname[0] != 0x00)
				{
					sprintf(Text, "%s ", VPS_lastname);
					VPS_lastname[0] = 0x00;
				}

				strcpy(Text1, Text);

				if (System_In_Mute == TRUE)
					sprintf(Text1, "Volume Mute");
				SetWindowText(hwndTextField, Text1);
			}
		}
		else if (wParam == TIMER_KEYNUMBER)
		{
			KillTimer(hWnd, TIMER_KEYNUMBER);
			i = atoi(ChannelString);
			i = i - 1;
			ChangeChannel(i);
			ChannelString[0] = '\0';
		}
		break;

	case WM_SYSCOMMAND:
		switch (wParam & 0xFFF0)
		{
		case SC_SCREENSAVE:
		case SC_MONITORPOWER:
			return FALSE;
			break;
		}

	case WM_SIZE:
		StatusBar_Adjust(hWnd);
		if (bDoResize == TRUE)
		{
			switch(wParam)
			{
			case SIZE_MAXIMIZED:
				if(bIsFullScreen == FALSE)
				{
					bIsFullScreen = TRUE;
					ShowCursor(FALSE);
					WorkoutOverlaySize();
				}
				break;
			case SIZE_MINIMIZED:
				Overlay_Update(NULL, NULL, DDOVER_HIDE, FALSE);
				break;
			case SIZE_RESTORED:
				WorkoutOverlaySize();
				break;
			default:
				break;
			}
		}
		break;

	case WM_MOVE:
		StatusBar_Adjust(hWnd);
		if (bDoResize == TRUE && !IsIconic(hWnd) && !IsZoomed(hWnd))
		{
			WorkoutOverlaySize();
		}
		break;

	case WM_CHAR:
		if (((char) wParam >= '0') && ((char) wParam <= '9'))
		{
			sprintf(Text, "%c", LOWORD(wParam));
			strcat(ChannelString, Text);
			if (strlen(ChannelString) >= 3)
			{
				SetTimer(hWnd, TIMER_KEYNUMBER, 1, NULL);
			}
			else
			{
				SetTimer(hWnd, TIMER_KEYNUMBER, TIMER_KEYNUMBER_MS, NULL);
			}
		}
		break;
	
	case WM_PAINT:
		PaintOverlay(hWnd);
		break;

	case WM_QUERYENDSESSION:
	case WM_DESTROY:
		Stop_Thread();
		Audio_SetSource(AUDIOMUX_MUTE);
		Stop_Capture();

		CleanUpMemory();

		if(bIsFullScreen == FALSE)
		{
			SaveWindowPos(hWnd);
		}
		BT848_Close();
		PostQuitMessage(0);
		break;

	default:
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void SaveWindowPos(HWND hWnd)
{
	RECT rScreen;
	GetWindowRect(hWnd, &rScreen);
	emstarty = rScreen.top;
	emsizey = rScreen.bottom - rScreen.top; 
	emstartx = rScreen.left;
	emsizex = rScreen.right - rScreen.left;
}

void MainWndOnInitBT(HWND hWnd)
{
	char Text[128];
	int i;
	BOOL bInitOK = FALSE;
	MSPStatus[0] = 0x00;
	
	CurrentProgramm = InitialProg;

	if (BT848_FindTVCard(hWnd) == TRUE)
	{
		if(InitDD(hWnd) == TRUE)
		{
			if(Overlay_Create() == TRUE)
			{
				if (BT848_MemoryInit() == TRUE)
				{
					bInitOK = TRUE;
				}
			}
		}
	}
	else
	{
		SetDlgItemText(SplashWnd, IDC_TEXT1, "");
		SetDlgItemText(SplashWnd, IDC_TEXT2, "No");
		SetDlgItemText(SplashWnd, IDC_TEXT3, "Suitable");
		SetDlgItemText(SplashWnd, IDC_TEXT4, "Hardware");
		SetDlgItemText(SplashWnd, IDC_TEXT5, "");
	}

	if (bInitOK)
	{
		// if the user has not set up the card
		// ask them for the id
		if(CardType == TVCARD_UNKNOWN)
		{
			// try to detect the card
			CardType = Card_AutoDetect();
			Card_AutoDetectTuner(CardType);
			// if we cannot detect the card of the tuner
			// present them with the card setup dialog box
			if(CardType == TVCARD_UNKNOWN || TunerType == TUNER_ABSENT)
			{
				DialogBox(hInst, "SELECTCARD", hWnd, (DLGPROC) SelectCardProc);
			}
		}

		// default the TVTYPE dependant on the Tuner selected
		// should be OK most of the time
		if(TVTYPE == -1)
		{
			switch(Tuners[TunerType].Type)
			{
			case PAL:
			case PAL_I:
				TVTYPE = 0;
				break;
			case NTSC:
				TVTYPE = 1;
				break;
			case SECAM:
				TVTYPE = 2;
				break;
			default:
				TVTYPE = 0;
				break;
			}
		}
		
		BT848_MakeVBITable(VBI_lpf);
		
		WStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
		if (bAlwaysOnTop == FALSE)
		{
			WStyle = WStyle ^ 8;
			i = SetWindowLong(hWnd, GWL_EXSTYLE, WStyle);
			SetWindowPos(hWnd, HWND_NOTOPMOST, 10, 10, 20, 20, SWP_NOMOVE | SWP_NOCOPYBITS | SWP_NOSIZE | SWP_SHOWWINDOW);
		}
		else
		{
			WStyle = WStyle | 8;
			i = SetWindowLong(hWnd, GWL_EXSTYLE, WStyle);
			SetWindowPos(hWnd, HWND_TOPMOST, 10, 10, 20, 20, SWP_NOMOVE | SWP_NOCOPYBITS | SWP_NOSIZE | SWP_SHOWWINDOW);
		}

		if (Show_Menu == FALSE)
		{
			Show_Menu = TRUE;
			SendMessage(hWnd, WM_COMMAND, IDM_TOGGLE_MENU, 0);
		}

		sprintf(TunerStatus, "No Device on I2C-Bus");

		sprintf(Text, "No Tuner");
		if (Tuner_Init(TunerType) == TRUE)
		{
			sprintf(Text, "Tuner OK");
		}
		SetDlgItemText(SplashWnd, IDC_TEXT4, Text);

		if (MSPStatus[0] == 0x00)
		{
			sprintf(MSPStatus, "No Device on I2C-Bus");
			sprintf(Text, "No MSP-Device");
			if (Audio_Init(0x80, 0x81) == TRUE)
			{
				sprintf(Text, "MSP-Device OK");
				sprintf(MSPStatus, "MSP-Device I2C-Bus I/O 0x80/0x81");
			}
		}
		else
		{
			sprintf(Text, "MSP-Device OK");
		}
		SetDlgItemText(SplashWnd, IDC_TEXT5, Text);

		if (Has_MSP == TRUE)
		{
			SetTimer(hWnd, TIMER_MSP, TIMER_MSP_MS, NULL);
		}

		AudioSource = AUDIOMUX_EXTERNAL;
		switch (VideoSource) {
		case 1:	sprintf(Text, "Composite");             break;
		case 2: sprintf(Text, "S-Video");               break;
		case 3: sprintf(Text, "Other 1");               break;
		case 4: sprintf(Text, "Other 2");               break;
		case 5: sprintf(Text, "Composite via S-Video"); break;
		default:
			AudioSource = AUDIOMUX_TUNER;
			ChangeChannel(CurrentProgramm);
			break;
		}

		if (bDisplayStatusBar == TRUE)
		{
			SetWindowText(hwndKeyField, Text);
		}

		for (i = 0; i < 5; i++)
		{
			pDisplay[i] = Display_dma[i]->dwUser;
		}

		// OK we're ready to go
		BT848_ResetHardware();
		BT848_SetGeoSize();
		WorkoutOverlaySize();
		Start_Capture();
		Sleep(100);

		SetMenuAnalog();

		SetTimer(hWnd, 10, 5000, NULL);
		bDoResize = TRUE;
	}
	else
	{
		CleanUpMemory();
		BT848_Close();
		Sleep(2000);
		PostQuitMessage(0);
	}
}


void MainWndOnCreate(HWND hWnd)
{
	char Text[128];
	int i;
	int ProcessorMask;

	GetSystemInfo(&SysInfo);
	SetDlgItemText(SplashWnd, IDC_TEXT1, "Table Build");
	SetDlgItemText(SplashWnd, IDC_TEXT2, "");
	SetDlgItemText(SplashWnd, IDC_TEXT3, "");
	SetDlgItemText(SplashWnd, IDC_TEXT4, "");
	SetDlgItemText(SplashWnd, IDC_TEXT5, "");
	Sleep(100);
	
	SetDlgItemText(SplashWnd, IDC_TEXT2, "InterCast");
	for (i = 0; i < 12; i++)
	{
		UTPages[i] = 0;
	}
	InterCast_Init();

	Sleep(100);
	SetDlgItemText(SplashWnd, IDC_TEXT3, "VideoText");
	for (i = 0; i < 800; i++)
	{
		VTFrame[i].SubPage = NULL;
		VTFrame[i].SubCount = 0;
	}

	VT_ChannelChange();

	for (i = 0; i < MAXVTDIALOG; i++)
	{
		VTDialog[i].Dialog = NULL;
	}

	Load_Program_List();
	Load_Country_Settings();

	VTScreen[0] = NULL;

	// DIB-Bitmap VideoText erzeugen
	SetDlgItemText(SplashWnd, IDC_TEXT4, "HQ-Color");

	if (USE_MIXER == TRUE)
	{
		Sleep(100);
		SetDlgItemText(SplashWnd, IDC_TEXT1, "Sound-System");
		SetDlgItemText(SplashWnd, IDC_TEXT2, "");
		SetDlgItemText(SplashWnd, IDC_TEXT3, "");
		SetDlgItemText(SplashWnd, IDC_TEXT4, "");
		SetDlgItemText(SplashWnd, IDC_TEXT5, "");

		Enumerate_Sound_SubSystem();
		if (SoundSystem.DeviceCount == 0)
		{
			SetDlgItemText(SplashWnd, TEXT3, "No Soundsystem found");
		}
		else
		{
			if (SoundSystem.DeviceCount >= 1)
				SetDlgItemText(SplashWnd, TEXT3, SoundSystem.MixerDev[0].szPname);
			if (SoundSystem.DeviceCount >= 2)
				SetDlgItemText(SplashWnd, TEXT4, SoundSystem.MixerDev[1].szPname);
			if (SoundSystem.DeviceCount >= 3)
				SetDlgItemText(SplashWnd, TEXT5, SoundSystem.MixerDev[2].szPname);
			Sleep(100);
			SetDlgItemText(SplashWnd, IDC_TEXT3, "");
			SetDlgItemText(SplashWnd, IDC_TEXT4, "");
			SetDlgItemText(SplashWnd, IDC_TEXT5, "");

			if (Volume.SoundSystem >= 0)
				sprintf(Text, "%s", SoundSystem.MixerDev[Volume.SoundSystem].szPname);
			else if (Mute.SoundSystem >= 0)
				sprintf(Text, "%s", SoundSystem.MixerDev[Mute.SoundSystem].szPname);
			else
				sprintf(Text, "%s", SoundSystem.MixerDev[0].szPname);
			SetDlgItemText(SplashWnd, IDC_TEXT2, Text);

			if (Volume.SoundSystem >= 0)
				sprintf(Text, "Volume -> %s", SoundSystem.To_Lines[Volume.SoundSystem].To_Connection[Volume.Destination].MixerConnections[Volume.Connection].szName);
			else
				sprintf(Text, "Volume Not Set");
			SetDlgItemText(SplashWnd, IDC_TEXT3, Text);

			if (Mute.SoundSystem >= 0)
				sprintf(Text, "%s -> %s  %s", SoundSystem.To_Lines[Mute.SoundSystem].To_Connection[Mute.Destination].MixerConnections[Mute.Connection].szName,
						SoundSystem.To_Lines[Mute.SoundSystem].MixerLine[Mute.Destination].szName,
						SoundSystem.To_Lines[Mute.SoundSystem].To_Connection[Mute.Destination].To_Control[Mute.Connection].MixerControl[Mute.Control].szName);
			else
				sprintf(Text, "Mute Not Set");
			SetDlgItemText(SplashWnd, IDC_TEXT4, Text);

			if (MIXER_LINKER_KANAL == -1)
				Mixer_Get_Volume(&MIXER_LINKER_KANAL, &MIXER_RECHTER_KANAL);
			Mixer_Set_Defaults();
			Get_Volume_Param();
			Mixer_Set_Volume(MIXER_LINKER_KANAL, MIXER_RECHTER_KANAL);
		}
	}
	SetDlgItemText(SplashWnd, IDC_TEXT1, "System Analysis");
	SetDlgItemText(SplashWnd, IDC_TEXT2, "");
	SetDlgItemText(SplashWnd, IDC_TEXT3, "");
	SetDlgItemText(SplashWnd, IDC_TEXT4, "");
	SetDlgItemText(SplashWnd, IDC_TEXT5, "");

	Sleep(100);
	sprintf(Text, "Processor %d ", SysInfo.dwProcessorType);
	SetDlgItemText(SplashWnd, IDC_TEXT2, Text);
	sprintf(Text, "Number %d ", SysInfo.dwNumberOfProcessors);
	SetDlgItemText(SplashWnd, IDC_TEXT3, Text);

	if (SysInfo.dwNumberOfProcessors > 1)
	{
		if (DecodeProcessor == 0)
		{
			if (SysInfo.dwNumberOfProcessors == 2)
			{
				MainProcessor = 0;
				DecodeProcessor = 1;
			}
			if (SysInfo.dwNumberOfProcessors == 3)
			{
				MainProcessor = 0;
				DecodeProcessor = 2;
			}
			if (SysInfo.dwNumberOfProcessors > 3)
			{
				DecodeProcessor = 3;
			}

		}

		SetDlgItemText(SplashWnd, IDC_TEXT1, "Multi-Processor");
		sprintf(Text, "Main-CPU %d ", MainProcessor);
		SetDlgItemText(SplashWnd, IDC_TEXT2, Text);
		sprintf(Text, "DECODE-CPU %d ", DecodeProcessor);
		SetDlgItemText(SplashWnd, IDC_TEXT3, Text);
		Sleep(100);
	}

	ProcessorMask = 1 << (MainProcessor);
	i = SetThreadAffinityMask(GetCurrentThread(), ProcessorMask);

	SetDlgItemText(SplashWnd, IDC_TEXT1, "Hardware Analyse");
	SetDlgItemText(SplashWnd, IDC_TEXT2, "");
	SetDlgItemText(SplashWnd, IDC_TEXT3, "");
	SetDlgItemText(SplashWnd, IDC_TEXT4, "");
	SetDlgItemText(SplashWnd, IDC_TEXT5, "");
	Sleep(200);

	if(bIsFullScreen == TRUE)
	{
		SaveWindowPos(hWnd);
		ShowCursor(FALSE);
	}
	else
	{
		if (bDisplayStatusBar == TRUE)
		{
			SetTimer(hWnd, TIMER_STATUS, TIMER_STATUS_MS, NULL);
		}
	}

	PostMessage(hWnd, INIT_BT, 0, 0);
}

void SetMenuAnalog()
{
	CheckMenuItem(GetMenu(hWnd), ThreadClassId + 1150, MF_CHECKED);
	CheckMenuItem(GetMenu(hWnd), PriorClassId + 1160, MF_CHECKED);

	EnableMenuItem(GetMenu(hWnd), IDM_UNTERTITEL, MF_GRAYED);
	EnableMenuItem(GetMenu(hWnd), IDM_PDC_OUT, MF_GRAYED);
	EnableMenuItem(GetMenu(hWnd), IDM_VT_OUT, MF_GRAYED);
	EnableMenuItem(GetMenu(hWnd), IDM_IC_OUT, MF_GRAYED);
	EnableMenuItem(GetMenu(hWnd), IDM_VPS_OUT, MF_GRAYED);

	CheckMenuItem(GetMenu(hWnd), IDM_VBI, Capture_VBI?MF_CHECKED:MF_UNCHECKED);
	if (Capture_VBI == TRUE)
	{
		// set vt dialog menu items up
		EnableMenuItem(GetMenu(hWnd), IDM_CALL_VIDEOTEXTSMALL, (VBI_Flags & VBI_VT)?MF_ENABLED:MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), IDM_CALL_VIDEOTEXT, (VBI_Flags & VBI_VT)?MF_ENABLED:MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), IDM_VT_RESET, (VBI_Flags & VBI_VT)?MF_ENABLED:MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), IDM_UNTERTITEL, (VBI_Flags & VBI_VT)?MF_ENABLED:MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), IDM_PDC_OUT, (VBI_Flags & VBI_VT)?MF_ENABLED:MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), IDM_VT_OUT, (VBI_Flags & VBI_VT)?MF_ENABLED:MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), IDM_IC_OUT, (VBI_Flags & VBI_IC)?MF_ENABLED:MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), IDM_VPS_OUT, (VBI_Flags & VBI_VPS)?MF_ENABLED:MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), IDM_VBI_VT, MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), IDM_VBI_IC, MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), IDM_VBI_VPS, MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), IDM_VBI_VD, MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), IDM_CLOSEDCAPTION, MF_ENABLED);
		CheckMenuItem(GetMenu(hWnd), IDM_VBI_VT, (VBI_Flags & VBI_VT)?MF_CHECKED:MF_UNCHECKED);
		CheckMenuItem(GetMenu(hWnd), IDM_VBI_IC, (VBI_Flags & VBI_IC)?MF_CHECKED:MF_UNCHECKED);
		CheckMenuItem(GetMenu(hWnd), IDM_VBI_VPS, (VBI_Flags & VBI_VPS)?MF_CHECKED:MF_UNCHECKED);
		CheckMenuItem(GetMenu(hWnd), IDM_VBI_VD, (VBI_Flags & VBI_VD)?MF_CHECKED:MF_UNCHECKED);
		CheckMenuItem(GetMenu(hWnd), IDM_CLOSEDCAPTION, (VBI_Flags & VBI_CC)?MF_CHECKED:MF_UNCHECKED);
	}
	else
	{
		EnableMenuItem(GetMenu(hWnd), IDM_VBI_VT, MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), IDM_VBI_IC, MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), IDM_VBI_VPS, MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), IDM_VBI_VD, MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), IDM_CALL_VIDEOTEXTSMALL, MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), IDM_CALL_VIDEOTEXT, MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), IDM_VT_RESET, MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), IDM_UNTERTITEL, MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), IDM_PDC_OUT, MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), IDM_VT_OUT, MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), IDM_IC_OUT, MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), IDM_VPS_OUT, MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), IDM_CLOSEDCAPTION, MF_GRAYED);
	}


	EnableMenuItem(GetMenu(hWnd), IDM_CHANNELPLUS, (TunerType != TUNER_ABSENT)?MF_ENABLED:MF_GRAYED);
	EnableMenuItem(GetMenu(hWnd), IDM_CHANNELMINUS, (TunerType != TUNER_ABSENT)?MF_ENABLED:MF_GRAYED);
	EnableMenuItem(GetMenu(hWnd), IDM_ANALOGSCAN, (TunerType != TUNER_ABSENT)?MF_ENABLED:MF_GRAYED);

	CheckMenuItem(GetMenu(hWnd), IDM_TREADPRIOR_0, (ThreadClassId == 0)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_TREADPRIOR_1, (ThreadClassId == 1)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_TREADPRIOR_2, (ThreadClassId == 2)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_TREADPRIOR_3, (ThreadClassId == 3)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_TREADPRIOR_4, (ThreadClassId == 4)?MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(GetMenu(hWnd), IDM_PRIORCLASS_0, (PriorClassId == 0)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_PRIORCLASS_1, (PriorClassId == 1)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_PRIORCLASS_2, (PriorClassId == 2)?MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(GetMenu(hWnd), IDM_TYPEFORMAT_0, (TVTYPE == 0)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_TYPEFORMAT_1, (TVTYPE == 1)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_TYPEFORMAT_2, (TVTYPE == 2)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_TYPEFORMAT_3, (TVTYPE == 3)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_TYPEFORMAT_4, (TVTYPE == 4)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_TYPEFORMAT_5, (TVTYPE == 5)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_TYPEFORMAT_6, (TVTYPE == 6)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_TYPEFORMAT_7, (TVTYPE == 7)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_TYPEFORMAT_8, (TVTYPE == 8)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_TYPEFORMAT_9, (TVTYPE == 9)?MF_CHECKED:MF_UNCHECKED);

	EnableMenuItem(GetMenu(hWnd), IDM_TUNER, (TVCards[TVTYPE].TunerInput != -1)?MF_ENABLED:MF_GRAYED);
	if(TVCards[TVTYPE].SVideoInput == -1)
	{
		EnableMenuItem(GetMenu(hWnd), IDM_EXTERN2, MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), IDM_EXTERN3, (TVCards[TVTYPE].nVideoInputs > 2)?MF_ENABLED:MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), IDM_EXTERN4, (TVCards[TVTYPE].nVideoInputs > 3)?MF_ENABLED:MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), IDM_EXTERN5, MF_GRAYED);
	}
	else
	{
		EnableMenuItem(GetMenu(hWnd), IDM_EXTERN2, MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), IDM_EXTERN3, (TVCards[TVTYPE].nVideoInputs > 3)?MF_ENABLED:MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), IDM_EXTERN4, (TVCards[TVTYPE].nVideoInputs > 4)?MF_ENABLED:MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), IDM_EXTERN5, MF_ENABLED);
	}

	CheckMenuItem(GetMenu(hWnd), IDM_TUNER, (VideoSource == 0)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_EXTERN1, (VideoSource == 1)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_EXTERN2, (VideoSource == 2)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_EXTERN3, (VideoSource == 3)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_EXTERN4, (VideoSource == 4)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_EXTERN5, (VideoSource == 5)?MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(GetMenu(hWnd), IDM_AUDIO_0, (AudioSource == 0)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_AUDIO_1, (AudioSource == 1)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_AUDIO_2, (AudioSource == 2)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_AUDIO_3, (AudioSource == 3)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_AUDIO_4, (AudioSource == 4)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_AUDIO_5, (AudioSource == 5)?MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(GetMenu(hWnd), IDM_MSPMODE_2, (MSPMode == 2)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_MSPMODE_3, (MSPMode == 3)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_MSPMODE_4, (MSPMode == 4)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_MSPMODE_5, (MSPMode == 5)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_MSPMODE_6, (MSPMode == 6)?MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(GetMenu(hWnd), IDM_MSPSTEREO_1, (MSPStereo == 1)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_MSPSTEREO_2, (MSPStereo == 2)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_MSPSTEREO_3, (MSPStereo == 3)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_MSPSTEREO_4, (MSPStereo == 4)?MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(GetMenu(hWnd), IDM_MAJOR_CARRIER_0, (MSPMajorMode == 0)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_MAJOR_CARRIER_1, (MSPMajorMode == 1)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_MAJOR_CARRIER_2, (MSPMajorMode == 2)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_MAJOR_CARRIER_3, (MSPMajorMode == 3)?MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(GetMenu(hWnd), IDM_MINOR_CARRIER_0, (MSPMinorMode == 0)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_MINOR_CARRIER_1, (MSPMinorMode == 1)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_MINOR_CARRIER_2, (MSPMinorMode == 2)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_MINOR_CARRIER_3, (MSPMinorMode == 3)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_MINOR_CARRIER_4, (MSPMinorMode == 4)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_MINOR_CARRIER_5, (MSPMinorMode == 5)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_MINOR_CARRIER_6, (MSPMinorMode == 6)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_MINOR_CARRIER_7, (MSPMinorMode == 7)?MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(GetMenu(hWnd), IDM_STATUSBAR, bDisplayStatusBar?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_ON_TOP, bAlwaysOnTop?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_AUTOSTEREO, AutoStereoSelect?MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(GetMenu(hWnd), IDM_AUTODETECT, bAutoDetectMode?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_FALLBACK, bFallbackToVideo?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_VIDEO_BOB, (gPulldownMode == VIDEO_MODE_BOB && ! bAutoDetectMode) ?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_VIDEO_WEAVE, (gPulldownMode == VIDEO_MODE_WEAVE && ! bAutoDetectMode) ?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_WEAVE, (gPulldownMode == SIMPLE_WEAVE && ! bAutoDetectMode) ?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_BOB, (gPulldownMode == INTERPOLATE_BOB && ! bAutoDetectMode) ?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_BTV, (gPulldownMode == BTV_PLUGIN && ! bAutoDetectMode) ?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_22PULLODD, (gPulldownMode == FILM_22_PULLDOWN_ODD && ! bAutoDetectMode) ?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_22PULLEVEN, (gPulldownMode == FILM_22_PULLDOWN_EVEN && ! bAutoDetectMode) ?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_32PULL1, (gPulldownMode == FILM_32_PULLDOWN_0 && ! bAutoDetectMode) ?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_32PULL2, (gPulldownMode == FILM_32_PULLDOWN_1 && ! bAutoDetectMode) ?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_32PULL3, (gPulldownMode == FILM_32_PULLDOWN_2 && ! bAutoDetectMode) ?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_32PULL4, (gPulldownMode == FILM_32_PULLDOWN_3 && ! bAutoDetectMode) ?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_32PULL5, (gPulldownMode == FILM_32_PULLDOWN_4 && ! bAutoDetectMode) ?MF_CHECKED:MF_UNCHECKED);

	SetMenuAspectRatio(hWnd);
}

void CleanUpMemory()
{
	int i;

	Mixer_Exit();
	InterCast_Exit();
	VideoDat_Exit();
	for (i = 0; i < 800; i++)
	{
		if (VTFrame[i].SubPage != NULL)
		{
			free(VTFrame[i].SubPage);
		}
		VTFrame[i].SubPage = NULL;
		VTFrame[i].SubCount = 0;
	}

	Free_DMA(&Risc_dma);
	for(i = 0; i < 5; i++)
	{
		Free_DMA(&Vbi_dma[i]);
		Free_Display_DMA(i);
		Free_DMA(&Burst_dma[i]);
	}

	for (i = 0; i < MAXVTDIALOG; i++)
	{
		free(VTScreen[i]);
	}
}

void ChangeChannel(int NewChannel)
{
	char Text[128];

	if (TunerType != TUNER_ABSENT)
	{
		if(NewChannel >= 0 && NewChannel < MAXPROGS)
		{
			if (Programm[NewChannel].freq != 0)
			{
				Audio_SetSource(AUDIOMUX_MUTE);

				CurrentProgramm = NewChannel;

				Tuner_SetFrequency(TunerType, MulDiv(Programm[CurrentProgramm].freq * 1000, 16, 1000000));

				if (bDisplayStatusBar == TRUE)
				{
					sprintf(Text, "%04d. %s ", CurrentProgramm + 1, Programm[CurrentProgramm].Name);
					SetWindowText(hwndKeyField, Text);
				}
				VT_ChannelChange();
				Sleep(20);
				Audio_SetSource(AudioSource);
			}
		}
	}
}
