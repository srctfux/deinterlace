////////////////////////////////////////////////////////////////////////////
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
// 21 Dec 2000   John Adcock           Stopped Timer after ini write
//
// 26 Dec 2000   Eric Schmidt          Fixed remember-last-audio-input-at-start.
//
// 02 Jan 2001   John Adcock           Added pVBILines
//                                     Removed bTV plug-in
//                                     Added Scaled BOB method
//
// 03 Jan 2001   Michael Eskin         Added MSP muting
//
// 07 Jan 2001   John Adcock           Added Adaptive deinterlacing
//                                     Changed display and handling of
//                                     change deinterlacing method
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "other.h"
#include "bt848.h"
#include "CPU.h"
#include "mixerdev.h"
#include "VBI_VideoText.h"
#include "AspectRatio.h"
#include "dTV.h"
#include "settings.h"
#include "ProgramList.h"
#include "Dialogs.h"
#include "OutThreads.h"
#include "OSD.h"
#include "audio.h"
#include "tuner.h"
#include "status.h"
#include "vbi.h"
#include "DI_BlendedClip.H"
#include "FD_60Hz.H"
#include "FD_50Hz.H"
#include "FLT_TNoise.h"
#include "Splash.h"


HWND hWnd = NULL;
HANDLE hInst = NULL;

// Used to call MainWndOnInitBT
#define INIT_BT 1800


BOOL bDoResize = FALSE;

int CurrentProgramm = 0;

HWND VThWnd;

unsigned long freq;
char Typ;
unsigned int srate;

int MoveXDist=-1;
int MoveYDist=-1;

long WStyle;

BOOL    Show_Menu=TRUE;
HMENU   hMenu;
HANDLE  hAccel;

char ChannelString[10];

int LastFrame;

int FORMAT_MASK = 0x0F;
int PalFormat = 0;

BOOL  BlackSet[288];

int BeforeVD=0;

int WriteIndex=1;

int MainProcessor=0;
int DecodeProcessor=0;
int PriorClassId = 0;
int ThreadClassId = 1;

BOOL bShowCursor = TRUE;

long emsizex = 649;
long emsizey = 547;
long emstartx = 10;
long emstarty = 10;

int pgsizex = -1;
int pgsizey = -1;
int pgstartx = -1;
int pgstarty = -1;

BOOL bAlwaysOnTop = FALSE;
BOOL bDisplayStatusBar = TRUE;
BOOL bDisplaySplashScreen = TRUE;
BOOL bIsFullScreen = FALSE;
BOOL bForceFullScreen = FALSE;

int AudioSource = AUDIOMUX_MUTE;

HFONT currFont = NULL;

UINT CpuFeatureFlags;		// TRB 12/20/00 Processor capability flags


BOOL IsFullScreen_OnChange(long NewValue);
BOOL DisplayStatusBar_OnChange(long NewValue);
void SwitchToSource(int nInput);
void Cursor_SetVisibility(BOOL bVisible);
const char * GetSourceName(int nVideoSource);


/****************************************************************************

    FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)

    PURPOSE: calls initialization function, processes message loop

****************************************************************************/

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASS wc;
	MSG msg;
	HWND hPrevWindow;

	hInst = hInstance;
	CPU_SetupFeatureFlag();
	// if we are already runninmg then start up old version
	hPrevWindow = FindWindow((LPCTSTR) "dTV", (LPCTSTR) "dTV");
	if (hPrevWindow != NULL)
	{
		SetFocus(hPrevWindow);
		SetActiveWindow(hPrevWindow);
		SetForegroundWindow(hPrevWindow);
		return FALSE;
	}

	// JA 07/01/2001
	// Required to use slider control
	InitCommonControls();

	// JA 21/12/2000
	// Added single place to setup ini file name
	SetIniFileForSettings(lpCmdLine);
	LoadSettingsFromIni();

	if(bDisplaySplashScreen)
	{
		ShowSpashScreen();
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


	// 2000-10-31 Added by Mark: Changed to WS_POPUP for more cosmetic direct-to-full-screen startup,
	// let UpdateWindowState() handle initialization of windowed dTV instead.
	hWnd = CreateWindow("dTV", "dTV", WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL, NULL, hInstance, NULL);
	if (!hWnd) return FALSE;
	if (!bIsFullScreen) SetWindowPos(hWnd, 0, emstartx, emstarty, emsizex, emsizey, SWP_SHOWWINDOW);

	if (!StatusBar_Init()) return FALSE;

	if (bDisplayStatusBar == FALSE)
	{
		StatusBar_ShowWindow(FALSE);
	}

	// 2000-10-31 Added by Mark Rejhon
	// Now show the window, directly to maximized or windowed right away.
	// That way, if the end user has configured dTV to startup maximized,
	// it won't flash a window right before maximizing.
	UpdateWindowState();

	PostMessage(hWnd, WM_SIZE, SIZENORMAL, MAKELONG(emsizex, emsizey));
	if (!(hAccel = LoadAccelerators(hInstance, "ANALOGACCEL")))
	{
		ErrorBox("Accelerators not Loaded");
	}

	// catch any serious errors during message handling
	__try
	{
		while (GetMessage(&msg, NULL, 0, 0))
		{
			if (!TranslateAccelerator(hWnd, hAccel, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) 
    {
		ErrorBox("Crashed");
	}

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
            Reset_Capture();
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
			if (VideoSource == SOURCE_TUNER)
			{
				// MAE 8 Nov 2000 Added wrap around
				if (Programm[CurrentProgramm + 1].freq != 0)
					ChangeChannel(CurrentProgramm + 1);
				else
					ChangeChannel(0);

				StatusBar_ShowText(STATUS_KEY, Programm[CurrentProgramm].Name);
				OSD_ShowText(hWnd,Programm[CurrentProgramm].Name, 0);
			}

			break;

		case IDM_CHANNELMINUS:
			if (VideoSource == SOURCE_TUNER)
			{
				// MAE 8 Nov 2000 Added wrap around
				if (CurrentProgramm != 0)
				{
					ChangeChannel(CurrentProgramm - 1);
				}
				else
				{
					// Search for end of current program list
					for (i=0;i<MAXPROGS;++i)
					{
						if (Programm[i].freq == 0)
							break;
					}
					if (i != 0)
					{
						ChangeChannel(i - 1);
					}
					else
					{
						ChangeChannel(0);
					}
				}

				StatusBar_ShowText(STATUS_KEY, Programm[CurrentProgramm].Name);
				OSD_ShowText(hWnd,Programm[CurrentProgramm].Name, 0);
			}

			break;

		case IDM_RESET:
            Reset_Capture();
	        Sleep(100);
	        Audio_SetSource(AudioSource);
			break;

		case IDM_TOGGLE_MENU:
			Show_Menu = !Show_Menu;
			WorkoutOverlaySize();
			break;

		case IDM_AUTODETECT:
			if(Setting_GetValue(OutThreads_GetSetting(AUTODETECT)))
			{
				ShowText(hWnd, "Auto Pulldown Detect OFF");
				Setting_SetValue(OutThreads_GetSetting(AUTODETECT), FALSE);
			}
			else
			{
				ShowText(hWnd, "Auto Pulldown Detect ON");
				Setting_SetValue(OutThreads_GetSetting(AUTODETECT), TRUE);
			}
			// Set Deinterlace Mode to film fallback in
			// either case
			if(TVSettings[TVTYPE].Is25fps)
			{
				SetDeinterlaceMode(Setting_GetValue(FD50_GetSetting(PALFILMFALLBACKMODE)));
			}
			else
			{
				SetDeinterlaceMode(Setting_GetValue(FD60_GetSetting(NTSCFILMFALLBACKMODE)));
			}
			break;

		case IDM_FALLBACK:
			if(Setting_GetValue(FD60_GetSetting(FALLBACKTOVIDEO)))
			{
				ShowText(hWnd, "Fallback on Bad Pulldown OFF");
				Setting_SetValue(FD60_GetSetting(FALLBACKTOVIDEO), FALSE);
			}
			else
			{
				ShowText(hWnd, "Fallback on Bad Pulldown ON");
				Setting_SetValue(FD60_GetSetting(FALLBACKTOVIDEO), TRUE);
			}
			break;

		case IDM_32PULL1:
		case IDM_32PULL2:
		case IDM_32PULL3:
		case IDM_32PULL4:
		case IDM_32PULL5:
		case IDM_22PULLODD:
		case IDM_22PULLEVEN:
			Setting_SetValue(OutThreads_GetSetting(AUTODETECT), FALSE);
			SetDeinterlaceMode(LOWORD(wParam) - IDM_VIDEO_BOB);
			ShowText(hWnd, DeinterlaceModeName(-1));
			break;

		case IDM_WEAVE:
		case IDM_BOB:
		case IDM_VIDEO_WEAVE:
		case IDM_VIDEO_BOB:
		case IDM_EVEN_ONLY:
		case IDM_ODD_ONLY:
		case IDM_VIDEO_2FRAME:
		case IDM_SCALER_BOB:
		case IDM_BLENDED_CLIP:
		case IDM_ADAPTIVE:
		case IDM_VIDEO_GREEDY:
			if(Setting_GetValue(OutThreads_GetSetting(AUTODETECT)))
			{
				if(TVSettings[TVTYPE].Is25fps)
				{
					Setting_SetValue(FD50_GetSetting(PALFILMFALLBACKMODE), LOWORD(wParam) - IDM_VIDEO_BOB);
				}
				else
				{
					Setting_SetValue(FD60_GetSetting(NTSCFILMFALLBACKMODE), LOWORD(wParam) - IDM_VIDEO_BOB);
				}
				if(!DeintMethods[gPulldownMode].bIsFilmMode)
				{
					SetDeinterlaceMode(LOWORD(wParam) - IDM_VIDEO_BOB);
				}
			}
			else
			{
				
				SetDeinterlaceMode(LOWORD(wParam) - IDM_VIDEO_BOB);
			}
			ShowText(hWnd, DeinterlaceModeName(-1));
			if(LOWORD(wParam) == IDM_BLENDED_CLIP)
			{
				BlendedClip_ShowDlg(hInst, hWnd);
			}
			break;

		case IDM_NOISE_FILTER:
			if (Setting_GetValue(FLT_TNoise_GetSetting(USETEMPORALNOISEFILTER)))
			{
				ShowText(hWnd, "Noise Filter OFF");
				Setting_SetValue(FLT_TNoise_GetSetting(USETEMPORALNOISEFILTER), FALSE);
			}
			else
			{
				ShowText(hWnd, "Noise Filter ON");
				Setting_SetValue(FLT_TNoise_GetSetting(USETEMPORALNOISEFILTER), TRUE);
			}
			break;

		case IDM_ABOUT:
			DialogBox(hInst, "ABOUT", hWnd, AboutProc);
			break;

		case IDM_BRIGHTNESS_PLUS:
			Setting_Up(BT848_GetSetting(BRIGHTNESS));
            SendMessage(hWnd, WM_COMMAND, IDM_BRIGHTNESS_CURRENT, 0);
			break;

		case IDM_BRIGHTNESS_MINUS:
			Setting_Down(BT848_GetSetting(BRIGHTNESS));
			SendMessage(hWnd, WM_COMMAND, IDM_BRIGHTNESS_CURRENT, 0);
			break;

		case IDM_BRIGHTNESS_CURRENT:
			Setting_OSDShow(BT848_GetSetting(BRIGHTNESS), hWnd);
			break;

		case IDM_KONTRAST_PLUS:
			Setting_Up(BT848_GetSetting(CONTRAST));
            SendMessage(hWnd, WM_COMMAND, IDM_KONTRAST_CURRENT, 0);
			break;

		case IDM_KONTRAST_MINUS:
			Setting_Down(BT848_GetSetting(CONTRAST));
            SendMessage(hWnd, WM_COMMAND, IDM_KONTRAST_CURRENT, 0);
			break;

		case IDM_KONTRAST_CURRENT:
			Setting_OSDShow(BT848_GetSetting(CONTRAST), hWnd);
			break;

		case IDM_USATURATION_PLUS:
			Setting_Up(BT848_GetSetting(SATURATIONU));
            SendMessage(hWnd, WM_COMMAND, IDM_USATURATION_CURRENT, 0);
			break;
		
        case IDM_USATURATION_MINUS:
			Setting_Down(BT848_GetSetting(SATURATIONU));
            SendMessage(hWnd, WM_COMMAND, IDM_USATURATION_CURRENT, 0);
			break;

        case IDM_USATURATION_CURRENT:
			Setting_OSDShow(BT848_GetSetting(SATURATIONU), hWnd);
			break;
		
        case IDM_VSATURATION_PLUS:
			Setting_Up(BT848_GetSetting(SATURATIONV));
            SendMessage(hWnd, WM_COMMAND, IDM_VSATURATION_CURRENT, 0);
			break;

        case IDM_VSATURATION_MINUS:
			Setting_Down(BT848_GetSetting(SATURATIONU));
            SendMessage(hWnd, WM_COMMAND, IDM_VSATURATION_CURRENT, 0);
			break;

        case IDM_VSATURATION_CURRENT:
			Setting_OSDShow(BT848_GetSetting(SATURATIONU), hWnd);
			break;

		case IDM_COLOR_PLUS:
			Setting_Up(BT848_GetSetting(SATURATION));
            SendMessage(hWnd, WM_COMMAND, IDM_COLOR_CURRENT, 0);
			break;

		case IDM_COLOR_MINUS:
			Setting_Down(BT848_GetSetting(SATURATION));
            SendMessage(hWnd, WM_COMMAND, IDM_COLOR_CURRENT, 0);
			break;

		case IDM_COLOR_CURRENT:
			Setting_OSDShow(BT848_GetSetting(SATURATION), hWnd);
			break;

		case IDM_HUE_PLUS:
			Setting_Up(BT848_GetSetting(HUE));
            SendMessage(hWnd, WM_COMMAND, IDM_HUE_CURRENT, 0);
			break;

		case IDM_HUE_MINUS:
			Setting_Down(BT848_GetSetting(HUE));
            SendMessage(hWnd, WM_COMMAND, IDM_HUE_CURRENT, 0);
			break;

		case IDM_HUE_CURRENT:
			Setting_OSDShow(BT848_GetSetting(HUE), hWnd);
			break;

		case IDM_OVERSCAN_PLUS:
			Setting_Up(Aspect_GetSetting(OVERSCAN));
            SendMessage(hWnd, WM_COMMAND, IDM_OVERSCAN_CURRENT, 0);
			break;

		case IDM_OVERSCAN_MINUS:
			Setting_Down(Aspect_GetSetting(OVERSCAN));
            SendMessage(hWnd, WM_COMMAND, IDM_OVERSCAN_CURRENT, 0);
			break;

		case IDM_OVERSCAN_CURRENT:
			Setting_OSDShow(Aspect_GetSetting(OVERSCAN), hWnd);
			break;

		case IDM_BDELAY_PLUS:
			Setting_Up(BT848_GetSetting(BDELAY));
            SendMessage(hWnd, WM_COMMAND, IDM_BDELAY_CURRENT, 0);
			break;

		case IDM_BDELAY_MINUS:
			Setting_Down(BT848_GetSetting(BDELAY));
            SendMessage(hWnd, WM_COMMAND, IDM_BDELAY_CURRENT, 0);
			break;

		case IDM_BDELAY_CURRENT:
			Setting_OSDShow(BT848_GetSetting(BDELAY), hWnd);
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
				}
				
				// MAE 8 Dec 2000 Start of change
				// JA 8 Jan 20001 Changed to use function
				if (Audio_MSP_IsPresent())
				{
					// Mute the MSP decoder
					Audio_Mute();
				}
				// MAE 8 Dec 2000 End of change

				ShowText(hWnd,"MUTE");
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
				}
				// MAE 8 Dec 2000 Start of change
				// JA 8 Jan 20001 Changed to use function
				if (Audio_MSP_IsPresent())
				{
					Audio_SetVolume(InitialVolume);
				}
				// MAE 8 Dec 2000 End of change
				ShowText(hWnd,"UNMUTE");
			}
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
			ShowText(hWnd, Text);
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
			ShowText(hWnd, Text);
			break;

		case IDM_VOLUMEPLUS:
			if (USE_MIXER == FALSE)
			{
				if (InitialVolume < 1000)
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
			ShowText(hWnd, Text);
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
			ShowText(hWnd, Text);
			break;

		case IDM_TOGGLECURSOR:
			if(bIsFullScreen == FALSE)
			{
				bShowCursor = !bShowCursor;
				Cursor_SetVisibility(bShowCursor);
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
			break;

		case IDM_MAJOR_CARRIER_0:
		case IDM_MAJOR_CARRIER_1:
		case IDM_MAJOR_CARRIER_2:
		case IDM_MAJOR_CARRIER_3:
			Audio_MSP_Set_MajorMinor_Mode(LOWORD(wParam) - IDM_MAJOR_CARRIER_0, MSPMinorMode);
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
			break;

		case IDM_MSPSTEREO_1:
		case IDM_MSPSTEREO_2:
		case IDM_MSPSTEREO_3:
		case IDM_MSPSTEREO_4:
			Audio_MSP_SetStereo(MSPMajorMode, MSPMinorMode, LOWORD(wParam) - (IDM_MSPSTEREO_1 - 1));
			break;

		case IDM_AUTOSTEREO:
			AutoStereoSelect = !AutoStereoSelect;
			break;

		case IDM_AUDIO_0:
		case IDM_AUDIO_1:
		case IDM_AUDIO_2:
		case IDM_AUDIO_3:
		case IDM_AUDIO_4:
		case IDM_AUDIO_5:
			AudioSource = LOWORD(wParam) - IDM_AUDIO_0;
			switch (LOWORD(AudioSource)) {
			case 0: ShowText(hWnd, "Audio Input - Tuner");     break;
			case 1: ShowText(hWnd, "Audio Input - MSP/Radio"); break;
			case 2: ShowText(hWnd, "Audio Input - External");  break;
			case 3: ShowText(hWnd, "Audio Input - Internal");  break;
			case 4: ShowText(hWnd, "Audio Input - Disabled");  break;
			case 5: ShowText(hWnd, "Audio Input - Stereo");    break;
			}
			Stop_Capture();
			Audio_SetSource(AudioSource);
			Start_Capture();
			break;

		case IDM_SOURCE_TUNER:
			SwitchToSource(SOURCE_TUNER);
			break;

		case IDM_SOURCE_COMPOSITE:
			SwitchToSource(SOURCE_COMPOSITE);
			break;
		
		case IDM_SOURCE_SVIDEO:
			SwitchToSource(SOURCE_SVIDEO);
			break;

		case IDM_SOURCE_OTHER1:
			SwitchToSource(SOURCE_OTHER1);
			break;

		case IDM_SOURCE_OTHER2:
			SwitchToSource(SOURCE_OTHER2);
			break;

		case IDM_SOURCE_COMPVIASVIDEO:
			SwitchToSource(SOURCE_COMPVIASVIDEO);
			break;

		// MAE 13 Dec 2000 for CCIR656 Digital input
        case IDM_SOURCE_CCIR656:
			SwitchToSource(SOURCE_CCIR656);
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
			break;

		case IDM_ADV_VIDEOSETTINGS:
			DialogBox(hInst, "ADV_VIDEOSETTINGS", hWnd, AdvVideoSettingProc);
			break;

		case IDM_VPS_OUT:
			DialogBox(hInst, "VPSSTATUS", hWnd, VPSInfoProc);
			break;

		case IDM_VT_OUT:
			DialogBox(hInst, "VTSTATUS", hWnd, VTInfoProc);
			break;

		case IDM_VBI:
			Stop_Capture();
			Capture_VBI = !Capture_VBI;
			Start_Capture();
			break;

		case IDM_CAPTURE_PAUSE:
			Pause_Toggle_Capture();
			break;

		case IDM_AUDIO_MIXER:
			DialogBox(hInst, "MIXERSETUP", hWnd, MixerSetupProc);
			break;

		case IDM_STATUSBAR:
			DisplayStatusBar_OnChange(!bDisplayStatusBar);
			break;

		case IDM_ON_TOP:
			bAlwaysOnTop = !bAlwaysOnTop;
			WorkoutOverlaySize();
			break;

		case IDM_SPLASH_ON_STARTUP:
			bDisplaySplashScreen = !bDisplaySplashScreen;
			break;

		case IDM_ANALOGSCAN:
			SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_TUNER, 0);
			DialogBox(hInst, "ANALOGSCAN", hWnd, (DLGPROC) AnalogScanProc);
			break;

		case IDM_CHANNEL_LIST:
			if (VideoSource == SOURCE_TUNER)
			{
				DialogBox(hInst, "CHANNELLIST", hWnd, (DLGPROC) ProgramListProc);
				OSD_ShowText(hWnd,Programm[CurrentProgramm].Name, 0);
			}
			break;

		case IDM_TREADPRIOR_0:
		case IDM_TREADPRIOR_1:
		case IDM_TREADPRIOR_2:
		case IDM_TREADPRIOR_3:
		case IDM_TREADPRIOR_4:
			ThreadClassId = LOWORD(wParam) - IDM_TREADPRIOR_0;
			Stop_Capture();
			Start_Capture();
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
			ShowText(hWnd, Text);
			break;

		case IDM_TYPEFORMAT_0:
		case IDM_TYPEFORMAT_1:
		case IDM_TYPEFORMAT_2:
		case IDM_TYPEFORMAT_3:
		case IDM_TYPEFORMAT_4:
		case IDM_TYPEFORMAT_5:
		case IDM_TYPEFORMAT_6:
            // Video format (NTSC, PAL, etc)
			TVTYPE = LOWORD(wParam) - IDM_TYPEFORMAT_0;
			CurrentX = 720;
			ShowText(hWnd, TVSettings[TVTYPE].szDesc);
			Stop_Capture();
			BT848_SetGeoSize();
			WorkoutOverlaySize();
			Start_Capture();
			break;

		case ID_SETTINGS_PIXELWIDTH_768:
		case ID_SETTINGS_PIXELWIDTH_720:
		case ID_SETTINGS_PIXELWIDTH_640:
		case ID_SETTINGS_PIXELWIDTH_384:
		case ID_SETTINGS_PIXELWIDTH_320:
            switch(LOWORD(wParam))
            {
            case ID_SETTINGS_PIXELWIDTH_768:  CurrentX = 768; break;
		    case ID_SETTINGS_PIXELWIDTH_720:  CurrentX = 720; break;
		    case ID_SETTINGS_PIXELWIDTH_640:  CurrentX = 640; break;
		    case ID_SETTINGS_PIXELWIDTH_384:  CurrentX = 384; break; 
		    case ID_SETTINGS_PIXELWIDTH_320:  CurrentX = 320; break;
            default:                          CurrentX = 720; break;
            }
			sprintf(Text, "%d Pixel Sampling", CurrentX);
			Stop_Capture();
			BT848_SetGeoSize();
			WorkoutOverlaySize();
			Start_Capture();
			break;

		case IDM_SPACEBAR:
			if(!Setting_GetValue(OutThreads_GetSetting(AUTODETECT)))
			{
				int NewPulldownMode = gPulldownMode + 1;

				if(NewPulldownMode == PULLDOWNMODES_LAST_ONE)
				{
					NewPulldownMode = VIDEO_MODE_BOB;
				}
				SetDeinterlaceMode(NewPulldownMode);
				ShowText(hWnd, DeinterlaceModeName(-1));
			}
			break;

		case IDM_SHIFT_SPACEBAR:
			if (!Setting_GetValue(OutThreads_GetSetting(AUTODETECT)))
			{
				int NewPulldownMode = gPulldownMode;

				if (gPulldownMode == VIDEO_MODE_BOB)
					NewPulldownMode = PULLDOWNMODES_LAST_ONE;

				NewPulldownMode--;

				SetDeinterlaceMode(NewPulldownMode);
				ShowText(hWnd, DeinterlaceModeName(-1));
			}
			break;

		case IDM_FULL_SCREEN:
			IsFullScreen_OnChange(!bIsFullScreen);
			break;

		case IDM_TAKESTILL:
			Pause_Capture();
			Sleep(100);
			SaveStill();
			ShowText(hWnd, "Snapshot");
			UnPause_Capture();
			Sleep(100);
			break;

		case IDM_SHOW_OSD:
			OSD_ShowText(hWnd, GetSourceName(VideoSource), 0);
			break;

		case IDM_HIDE_OSD:
			OSD_Clear(hWnd);
			break;

		case IDM_SET_OSD_TEXT:
            // Useful for external programs for custom control of dTV's OSD display
            // Such as macros in software such as Girder, etc.
            if (lParam && GlobalGetAtomName((ATOM) lParam, Text, sizeof(Text)))
            {
                OSD_ShowTextOverride(hWnd, Text, 0);
                GlobalDeleteAtom((ATOM) lParam);
            }
			break;

        case IDM_OVERLAY_STOP:
		    Overlay_Stop(hWnd);
			break;

		case IDM_OVERLAY_START:
            Overlay_Start(hWnd);
			break;

        case IDM_FAST_REPAINT:
		    PaintColorkey(hWnd, TRUE);
		    OSD_Redraw(hWnd);
		    break;

		case IDM_HELP_HOMEPAGE:
			ShellExecute(hWnd, "open", "http://deinterlace.sourceforge.net/", NULL, NULL, SW_SHOWNORMAL);
			break;

		case IDM_HELP_FAQ:
			ShellExecute(hWnd, "open", ".\\Docs\\FAQ.htm", NULL, NULL, SW_SHOWNORMAL);
			break;

		case IDM_HELP_SUPPORT:
			ShellExecute(hWnd, "open", ".\\Docs\\user_support.htm", NULL, NULL, SW_SHOWNORMAL);
			break;

		case IDM_HELP_KEYBOARD:
			ShellExecute(hWnd, "open", ".\\Docs\\keyboard.htm", NULL, NULL, SW_SHOWNORMAL);
			break;

		case IDM_HELP_GPL:
			ShellExecute(hWnd, "open", ".\\Docs\\COPYING.html", NULL, NULL, SW_SHOWNORMAL);
			break;

		case IDM_HELP_README:
			ShellExecute(hWnd, "open", ".\\Docs\\dTV_Readme.htm", NULL, NULL, SW_SHOWNORMAL);
			break;
		default:
			// Check whether menu ID is an aspect ratio related item
			ProcessAspectRatioSelection(hWnd, LOWORD(wParam));
			break;
		}

        //-------------------------------------------------------
        // The following code executes on all WM_COMMAND calls

        // Updates the menu checkbox settings
		SetMenuAnalog();

        // Set the configuration file autosave timer.
        // We use an autosave timer so that when the user has finished
        // making adjustments and at least a small delay has occured,
        // that the DTV.INI file is properly up to date, even if 
        // the system crashes or system is turned off abruptly.
        KillTimer(hWnd, TIMER_AUTOSAVE);
        SetTimer(hWnd, TIMER_AUTOSAVE, TIMER_AUTOSAVE_MS, NULL);
        break;

	case WM_CREATE:
		MainWndOnCreate(hWnd);
		break;

	case INIT_BT:
		MainWndOnInitBT(hWnd);
		break;

// 2000-10-31 Added by Mark Rejhon
// This was an attempt to allow dTV to run properly through
// computer resolution changes.  Alas, dTV still crashes and burns
// in a fiery dive if you try to change resolution while dTV is
// running.  We need to somehow capture a message that comes right
// before a resolution change, so we can destroy the video overlay
// on time beforehand.   This message seems to happen right after
// a resolution change.
//
//	case WM_DISPLAYCHANGE:
//		// Windows resolution changed while software running
//		Stop_Capture();
//		Overlay_Destroy();
//		Sleep(100);
//		Overlay_Create();
//		Overlay_Clean();
//		BT848_ResetHardware();
//		BT848_SetGeoSize();
//		WorkoutOverlaySize();
//		Start_Capture();
//		Sleep(100);
//		Audio_SetSource(AudioSource);
//		break;

	case WM_POWERBROADCAST:
		// Handing to keep dTV running during computer suspend/resume
		switch ((DWORD) wParam)
		{
		case PBT_APMSUSPEND:
			// Stops video overlay upon suspend operation.
			Overlay_Stop(hWnd);
			break;
		case PBT_APMRESUMESUSPEND:
			// Restarts video overlay upon resume operation.
			// The following crashes unless we do either a HWND_BROADCAST
			// or a Sleep() operation.  To be on the safe side, I do both
			// here.  Perhaps the video overlay drivers needed to reinitialize.
			SendMessage(HWND_BROADCAST, WM_PAINT, 0, 0);
			Sleep(500);
			Overlay_Start(hWnd);
			break;
		}
		break;

	case WM_LBUTTONUP:
		SendMessage(hWnd, WM_COMMAND, IDM_FULL_SCREEN, 0);
		break;

	case WM_RBUTTONUP:
		if(bIsFullScreen == FALSE)
		{
			bShowCursor = !bShowCursor;
			Cursor_SetVisibility(bShowCursor);
		}
		break;

	case WM_TIMER:
        
        switch (LOWORD(wParam))
        {
        //-------------------------------
        case TIMER_MSP:
			if (bDisplayStatusBar == TRUE)
				Audio_MSP_Print_Mode();
			if (AutoStereoSelect == TRUE)
				Audio_MSP_Watch_Mode();
            break;
        //-------------------------------
        case TIMER_STATUS:
			if (!BT848_IsVideoPresent())
			{
				StatusBar_ShowText(STATUS_TEXT, "No Video Signal Found");
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
				StatusBar_ShowText(STATUS_TEXT, Text1);
			}
            break;
        //-------------------------------
        case TIMER_KEYNUMBER:
			KillTimer(hWnd, TIMER_KEYNUMBER);
			i = atoi(ChannelString);
			i = i - 1;
			ChangeChannel(i);
			if(CurrentProgramm == i)
			{
				StatusBar_ShowText(STATUS_KEY, Programm[CurrentProgramm].Name);
				OSD_ShowText(hWnd, Programm[CurrentProgramm].Name, 0);
			}
			else
			{
				StatusBar_ShowText(STATUS_KEY, "Not Found");
				OSD_ShowText(hWnd, "Not Found", 0);
			}
			ChannelString[0] = '\0';
            break;
        //-------------------------------
        case TIMER_AUTOSAVE:
			// JA 21/12/00 Added KillTimer so that settings are not
			// written repeatedly
	        KillTimer(hWnd, TIMER_AUTOSAVE);
			WriteSettingsToIni();
            break;
        //-------------------------------
        case OSD_TIMER_ID:
			OSD_Clear(hWnd);
            break;
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
					Cursor_SetVisibility(FALSE);
					WorkoutOverlaySize();
				}
				break;
			case SIZE_MINIMIZED:
				Overlay_Update(NULL, NULL, DDOVER_HIDE, FALSE);
				break;
			case SIZE_RESTORED:
				WorkoutOverlaySize();
                SetMenuAnalog();
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
		if (VideoSource == SOURCE_TUNER)
		{
			if (((char) wParam >= '0') && ((char) wParam <= '9'))
			{
				sprintf(Text, "%c", (char)wParam);
				strcat(ChannelString, Text);
				OSD_Clear(hWnd);
				OSD_ShowText(hWnd, ChannelString, 0);
				if (strlen(ChannelString) >= 3)
				{
					SetTimer(hWnd, TIMER_KEYNUMBER, 1, NULL);
				}
				else
				{
					SetTimer(hWnd, TIMER_KEYNUMBER, TIMER_KEYNUMBER_MS, NULL);
				}
			}
		}
		break;

	case WM_PAINT:
		PaintColorkey(hWnd, TRUE);
		OSD_Redraw(hWnd);
		break;

	case WM_QUERYENDSESSION:
	case WM_DESTROY:
		Stop_Capture();
		Audio_SetSource(AUDIOMUX_MUTE);
		
		// MAE 8 Dec 2000 Start of change
		// JA 8 Jan 2001 Changed to use function
		if (Audio_MSP_IsPresent())
		{
			// Mute the MSP decoder
			Audio_Mute();
		}
		// MAE 8 Dec 2000 End of change

		CleanUpMemory();

		if(bIsFullScreen == FALSE)
		{
			SaveWindowPos(hWnd);
		}
		BT848_Close();
		StatusBar_Destroy();
		PostQuitMessage(0);
		break;

	default:
		return Settings_HandleSettingMsgs(hWnd, message, wParam, lParam);
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

//---------------------------------------------------------------------------
void SaveWindowPos(HWND hWnd)
{
	RECT rScreen;
	if(hWnd != NULL)
	{
		GetWindowRect(hWnd, &rScreen);
		emstarty = rScreen.top;
		emsizey = rScreen.bottom - rScreen.top;
		emstartx = rScreen.left;
		emsizex = rScreen.right - rScreen.left;
	}
}

//---------------------------------------------------------------------------
void MainWndOnInitBT(HWND hWnd)
{
	int i;
	BOOL bInitOK = FALSE;

	AddSplashTextLine("Hardware Init");

	if (BT848_FindTVCard(hWnd) == TRUE)
	{
		AddSplashTextLine(BT848_ChipType());
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
		AddSplashTextLine("");
		AddSplashTextLine("No");
		AddSplashTextLine("Suitable");
		AddSplashTextLine("Hardware");
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

		if (Tuner_Init(TunerType) == TRUE)
		{
			AddSplashTextLine("Tuner OK");
		}
		else
		{
			AddSplashTextLine("No Tuner");
		}

		// MAE 8 Dec 2000 Start of change
		// JA 8 Jan 2001 Tidied up

		if (Audio_MSP_Init(0x80, 0x81) == TRUE)
		{
			AddSplashTextLine("MSP Device OK");
			Audio_SetVolume(InitialVolume);
		}
		else
		{
			AddSplashTextLine("No MSP Device");
		}

		// JA 8 Jan 2001 End of Tidy
		// MAE 8 Dec 2000 End of change
		if (Audio_MSP_IsPresent() == TRUE)
		{
			SetTimer(hWnd, TIMER_MSP, TIMER_MSP_MS, NULL);
		}

        // EAS20001226: Don't clobber what was read in from the ini file.
		//AudioSource = AUDIOMUX_EXTERNAL;

		if(VideoSource == SOURCE_TUNER)
		{
			ChangeChannel(CurrentProgramm);
		}

		StatusBar_ShowText(STATUS_KEY, GetSourceName(VideoSource));

        // OK we're ready to go
		BT848_ResetHardware();
        if (VideoSource == SOURCE_CCIR656)
        {
    		BT848_Enable656();
        }
        else
        {
    		BT848_SetGeoSize();
        }
		WorkoutOverlaySize();
		Start_Capture();

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

//---------------------------------------------------------------------------
void MainWndOnCreate(HWND hWnd)
{
	char Text[128];
	int i;
	int ProcessorMask;
	SYSTEM_INFO SysInfo;

	GetSystemInfo(&SysInfo);
	AddSplashTextLine("Table Build");
	AddSplashTextLine("VideoText");

	VBI_Init();	
	
	Load_Program_List_ASCII();
	Load_Country_Settings();

	if (USE_MIXER == TRUE)
	{
		AddSplashTextLine("Sound-System");

		Enumerate_Sound_SubSystem();
		if (SoundSystem.DeviceCount == 0)
		{
			AddSplashTextLine("No Soundsystem found");
		}
		else
		{
			if (SoundSystem.DeviceCount >= 1)
				AddSplashTextLine(SoundSystem.MixerDev[0].szPname);
			if (SoundSystem.DeviceCount >= 2)
				AddSplashTextLine(SoundSystem.MixerDev[1].szPname);
			if (SoundSystem.DeviceCount >= 3)
				AddSplashTextLine(SoundSystem.MixerDev[2].szPname);

			if (Volume.SoundSystem >= 0)
				AddSplashTextLine(SoundSystem.MixerDev[Volume.SoundSystem].szPname);
			else if (Mute.SoundSystem >= 0)
				AddSplashTextLine(SoundSystem.MixerDev[Mute.SoundSystem].szPname);
			else
				AddSplashTextLine(SoundSystem.MixerDev[0].szPname);

			if (Volume.SoundSystem >= 0)
				sprintf(Text, "Volume -> %s", SoundSystem.To_Lines[Volume.SoundSystem].To_Connection[Volume.Destination].MixerConnections[Volume.Connection].szName);
			else
				sprintf(Text, "Volume Not Set");
			AddSplashTextLine(Text);

			if (Mute.SoundSystem >= 0)
				sprintf(Text, "%s -> %s  %s", SoundSystem.To_Lines[Mute.SoundSystem].To_Connection[Mute.Destination].MixerConnections[Mute.Connection].szName,
						SoundSystem.To_Lines[Mute.SoundSystem].MixerLine[Mute.Destination].szName,
						SoundSystem.To_Lines[Mute.SoundSystem].To_Connection[Mute.Destination].To_Control[Mute.Connection].MixerControl[Mute.Control].szName);
			else
				sprintf(Text, "Mute Not Set");
			AddSplashTextLine(Text);

			if (MIXER_LINKER_KANAL == -1)
				Mixer_Get_Volume(&MIXER_LINKER_KANAL, &MIXER_RECHTER_KANAL);
			Mixer_Set_Defaults();
			Get_Volume_Param();
			Mixer_Set_Volume(MIXER_LINKER_KANAL, MIXER_RECHTER_KANAL);
		}
	}
	AddSplashTextLine("System Analysis");

	sprintf(Text, "Processor %d ", SysInfo.dwProcessorType);
	AddSplashTextLine(Text);
	sprintf(Text, "Number %d ", SysInfo.dwNumberOfProcessors);
	AddSplashTextLine(Text);

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

		AddSplashTextLine("Multi-Processor");
		sprintf(Text, "Main-CPU %d ", MainProcessor);
		AddSplashTextLine(Text);
		sprintf(Text, "DECODE-CPU %d ", DecodeProcessor);
		AddSplashTextLine(Text);
	}

	ProcessorMask = 1 << (MainProcessor);
	i = SetThreadAffinityMask(GetCurrentThread(), ProcessorMask);

	if(bIsFullScreen == TRUE)
	{
		Cursor_SetVisibility(FALSE);
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

//---------------------------------------------------------------------------
void SetMenuAnalog()
{
	HMENU hMenu;
	hMenu = GetMenu(hWnd);
	CheckMenuItem(hMenu, ThreadClassId + 1150, MF_CHECKED);
	CheckMenuItem(hMenu, PriorClassId + 1160, MF_CHECKED);

	EnableMenuItem(hMenu, IDM_PDC_OUT, MF_GRAYED);
	EnableMenuItem(hMenu, IDM_VT_OUT, MF_GRAYED);
	EnableMenuItem(hMenu, IDM_VPS_OUT, MF_GRAYED);

	CheckMenuItem(hMenu, IDM_VBI, Capture_VBI?MF_CHECKED:MF_UNCHECKED);
	if (Capture_VBI == TRUE)
	{
		// set vt dialog menu items up
		EnableMenuItem(hMenu, IDM_CALL_VIDEOTEXTSMALL, (VBI_Flags & VBI_VT)?MF_ENABLED:MF_GRAYED);
		EnableMenuItem(hMenu, IDM_CALL_VIDEOTEXT, (VBI_Flags & VBI_VT)?MF_ENABLED:MF_GRAYED);
		EnableMenuItem(hMenu, IDM_VT_RESET, (VBI_Flags & VBI_VT)?MF_ENABLED:MF_GRAYED);
		EnableMenuItem(hMenu, IDM_VT_OUT, (VBI_Flags & VBI_VT)?MF_ENABLED:MF_GRAYED);
		EnableMenuItem(hMenu, IDM_VPS_OUT, (VBI_Flags & VBI_VPS)?MF_ENABLED:MF_GRAYED);
		EnableMenuItem(hMenu, IDM_VBI_VT, MF_ENABLED);
		EnableMenuItem(hMenu, IDM_VBI_VPS, MF_ENABLED);
		EnableMenuItem(hMenu, IDM_CLOSEDCAPTION, MF_ENABLED);
		CheckMenuItem(hMenu, IDM_VBI_VT, (VBI_Flags & VBI_VT)?MF_CHECKED:MF_UNCHECKED);
		CheckMenuItem(hMenu, IDM_VBI_VPS, (VBI_Flags & VBI_VPS)?MF_CHECKED:MF_UNCHECKED);
		CheckMenuItem(hMenu, IDM_CLOSEDCAPTION, (VBI_Flags & VBI_CC)?MF_CHECKED:MF_UNCHECKED);
	}
	else
	{
		EnableMenuItem(hMenu, IDM_VBI_VT, MF_GRAYED);
		EnableMenuItem(hMenu, IDM_VBI_VPS, MF_GRAYED);
		EnableMenuItem(hMenu, IDM_CALL_VIDEOTEXTSMALL, MF_GRAYED);
		EnableMenuItem(hMenu, IDM_CALL_VIDEOTEXT, MF_GRAYED);
		EnableMenuItem(hMenu, IDM_VT_RESET, MF_GRAYED);
		EnableMenuItem(hMenu, IDM_VT_OUT, MF_GRAYED);
		EnableMenuItem(hMenu, IDM_VPS_OUT, MF_GRAYED);
		EnableMenuItem(hMenu, IDM_CLOSEDCAPTION, MF_GRAYED);
	}


	EnableMenuItem(hMenu, IDM_CHANNELPLUS, (TunerType != TUNER_ABSENT)?MF_ENABLED:MF_GRAYED);
	EnableMenuItem(hMenu, IDM_CHANNELMINUS, (TunerType != TUNER_ABSENT)?MF_ENABLED:MF_GRAYED);
	EnableMenuItem(hMenu, IDM_ANALOGSCAN, (TunerType != TUNER_ABSENT)?MF_ENABLED:MF_GRAYED);

	CheckMenuItem(hMenu, IDM_TREADPRIOR_0, (ThreadClassId == 0)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_TREADPRIOR_1, (ThreadClassId == 1)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_TREADPRIOR_2, (ThreadClassId == 2)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_TREADPRIOR_3, (ThreadClassId == 3)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_TREADPRIOR_4, (ThreadClassId == 4)?MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(hMenu, IDM_PRIORCLASS_0, (PriorClassId == 0)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_PRIORCLASS_1, (PriorClassId == 1)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_PRIORCLASS_2, (PriorClassId == 2)?MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(hMenu, IDM_TYPEFORMAT_0, (TVTYPE == 0)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_TYPEFORMAT_1, (TVTYPE == 1)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_TYPEFORMAT_2, (TVTYPE == 2)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_TYPEFORMAT_3, (TVTYPE == 3)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_TYPEFORMAT_4, (TVTYPE == 4)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_TYPEFORMAT_5, (TVTYPE == 5)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_TYPEFORMAT_6, (TVTYPE == 6)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_TYPEFORMAT_7, (TVTYPE == 7)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_TYPEFORMAT_8, (TVTYPE == 8)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_TYPEFORMAT_9, (TVTYPE == 9)?MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(hMenu, ID_SETTINGS_PIXELWIDTH_768, (CurrentX == 768)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_SETTINGS_PIXELWIDTH_720, (CurrentX == 720)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_SETTINGS_PIXELWIDTH_640, (CurrentX == 640)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_SETTINGS_PIXELWIDTH_384, (CurrentX == 384)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_SETTINGS_PIXELWIDTH_320, (CurrentX == 320)?MF_CHECKED:MF_UNCHECKED);

	EnableMenuItem(hMenu, IDM_SOURCE_TUNER, (TVCards[TVTYPE].TunerInput != -1)?MF_ENABLED:MF_GRAYED);
	if(TVCards[TVTYPE].SVideoInput == -1)
	{
		EnableMenuItem(hMenu, IDM_SOURCE_SVIDEO, MF_GRAYED);
		EnableMenuItem(hMenu, IDM_SOURCE_OTHER1, (TVCards[TVTYPE].nVideoInputs > 2)?MF_ENABLED:MF_GRAYED);
		EnableMenuItem(hMenu, IDM_SOURCE_OTHER2, (TVCards[TVTYPE].nVideoInputs > 3)?MF_ENABLED:MF_GRAYED);
		EnableMenuItem(hMenu, IDM_SOURCE_COMPVIASVIDEO, MF_GRAYED);
	}
	else
	{
		EnableMenuItem(hMenu, IDM_SOURCE_SVIDEO, MF_ENABLED);
		EnableMenuItem(hMenu, IDM_SOURCE_OTHER1, (TVCards[TVTYPE].nVideoInputs > 3)?MF_ENABLED:MF_GRAYED);
		EnableMenuItem(hMenu, IDM_SOURCE_OTHER2, (TVCards[TVTYPE].nVideoInputs > 4)?MF_ENABLED:MF_GRAYED);
		EnableMenuItem(hMenu, IDM_SOURCE_COMPVIASVIDEO, MF_ENABLED);
	}

	CheckMenuItem(hMenu, IDM_SOURCE_TUNER,         (VideoSource == 0)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_SOURCE_COMPOSITE,     (VideoSource == 1)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_SOURCE_SVIDEO,        (VideoSource == 2)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_SOURCE_OTHER1,        (VideoSource == 3)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_SOURCE_OTHER2,        (VideoSource == 4)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_SOURCE_COMPVIASVIDEO, (VideoSource == 5)?MF_CHECKED:MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_SOURCE_CCIR656,       (VideoSource == 6)?MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(hMenu, IDM_MUTE,    System_In_Mute?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_AUDIO_0, (AudioSource == 0)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_AUDIO_1, (AudioSource == 1)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_AUDIO_2, (AudioSource == 2)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_AUDIO_3, (AudioSource == 3)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_AUDIO_4, (AudioSource == 4)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_AUDIO_5, (AudioSource == 5)?MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(hMenu, IDM_MSPMODE_2, (MSPMode == 2)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_MSPMODE_3, (MSPMode == 3)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_MSPMODE_4, (MSPMode == 4)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_MSPMODE_5, (MSPMode == 5)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_MSPMODE_6, (MSPMode == 6)?MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(hMenu, IDM_MSPSTEREO_1, (MSPStereo == 1)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_MSPSTEREO_2, (MSPStereo == 2)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_MSPSTEREO_3, (MSPStereo == 3)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_MSPSTEREO_4, (MSPStereo == 4)?MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(hMenu, IDM_MAJOR_CARRIER_0, (MSPMajorMode == 0)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_MAJOR_CARRIER_1, (MSPMajorMode == 1)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_MAJOR_CARRIER_2, (MSPMajorMode == 2)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_MAJOR_CARRIER_3, (MSPMajorMode == 3)?MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(hMenu, IDM_MINOR_CARRIER_0, (MSPMinorMode == 0)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_MINOR_CARRIER_1, (MSPMinorMode == 1)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_MINOR_CARRIER_2, (MSPMinorMode == 2)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_MINOR_CARRIER_3, (MSPMinorMode == 3)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_MINOR_CARRIER_4, (MSPMinorMode == 4)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_MINOR_CARRIER_5, (MSPMinorMode == 5)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_MINOR_CARRIER_6, (MSPMinorMode == 6)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_MINOR_CARRIER_7, (MSPMinorMode == 7)?MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(hMenu, IDM_TOGGLECURSOR,      bShowCursor?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_STATUSBAR,         bDisplayStatusBar?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_ON_TOP,            bAlwaysOnTop?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_AUTOSTEREO,        AutoStereoSelect?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_SPLASH_ON_STARTUP, bDisplaySplashScreen?MF_CHECKED:MF_UNCHECKED);

	AspectRatio_SetMenu(hMenu);
	FD60_SetMenu(hMenu);
	OutThreads_SetMenu(hMenu);
	Deinterlace_SetMenu(hMenu);
	FLT_TNoise_SetMenu(hMenu);
}

//---------------------------------------------------------------------------
void CleanUpMemory()
{
	Mixer_Exit();
	VBI_Exit();
	BT848_MemoryFree();
}

//---------------------------------------------------------------------------
void ChangeChannel(int NewChannel)
{
	if (TunerType != TUNER_ABSENT)
	{
		if(NewChannel >= 0 && NewChannel < MAXPROGS)
		{
			if (Programm[NewChannel].freq != 0)
			{
				Audio_SetSource(AUDIOMUX_MUTE);
				CurrentProgramm = NewChannel;
				Tuner_SetFrequency(TunerType, MulDiv(Programm[CurrentProgramm].freq * 1000, 16, 1000000));
				Sleep(20);
				Audio_SetSource(AudioSource);

				VT_ChannelChange();
			}
		}
	}
}

//---------------------------------------------------------------------------
// Stops video overlay - 2000-10-31 Added by Mark Rejhon
// This ends the video overlay from operating, so that end users can
// write scripts that sends this special message to safely stop the video
// before switching computer resolutions or timings.
// This is also called during a Suspend operation
void Overlay_Stop(HWND hWnd)
{
    InvalidateRect(hWnd, NULL, FALSE);
	PaintColorkey(hWnd, FALSE);
	Stop_Capture();
	Overlay_Destroy();
}

//---------------------------------------------------------------------------
// Restarts video overlay - 2000-10-31 Added by Mark Rejhon
// This reinitializes the video overlay to continue operation,
// so that end users can write scripts that sends this special message
// to safely restart the video after a resolution or timings change.
// This is also called during a Resume operation
void Overlay_Start(HWND hWnd)
{
	Overlay_Create();
    Reset_Capture();
}

const char * GetSourceName(int nVideoSource)
{
	switch (nVideoSource)
	{
	case SOURCE_TUNER:         return Programm[CurrentProgramm].Name; break;
	case SOURCE_COMPOSITE:     return "Composite"; break;
	case SOURCE_SVIDEO:        return "S-Video"; break;
	case SOURCE_OTHER1:        return "Other 1"; break;
	case SOURCE_OTHER2:        return "Other 2"; break;
	case SOURCE_COMPVIASVIDEO: return "Composite via S-Video"; break;
	case SOURCE_CCIR656:       return "CCIR656"; break; // MAE 13 Dec 2000 For CCIR656 input
	}
	return "Unknown";
}

//---------------------------------------------------------------------------
// Show text on both OSD and statusbar
void ShowText(HWND hWnd, LPCTSTR szText)
{
	StatusBar_ShowText(STATUS_TEXT, szText);
	OSD_ShowText(hWnd, szText, 0);
}

//----------------------------------------------------------------------------
// Updates the window position/window state and enable/disable titlebar 
// as necessary.  This function should be globally used for everytime 
// you want to update the window everytime you have enabled/disabled the 
// statusbar, menus, full screen state, etc.
//
// This allows for more cosmetic handling - including the ability to 
// startup directly to maximized without any intermediate cosmetic
// glitches during startup.
//
void UpdateWindowState()
{
	if(bIsFullScreen == TRUE)
	{
		SetWindowLong(hWnd, GWL_STYLE, WS_VISIBLE);
		SetMenu(hWnd, NULL);
		StatusBar_ShowWindow(FALSE);
		SetWindowPos(hWnd,
					HWND_TOPMOST,
					0,
					0,
					GetSystemMetrics(SM_CXSCREEN),
					GetSystemMetrics(SM_CYSCREEN),
					SWP_SHOWWINDOW);
	}
	else
	{
		SetWindowLong(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
		SetMenu(hWnd, (Show_Menu == TRUE)?hMenu:NULL);
		StatusBar_ShowWindow(bDisplayStatusBar);
		SetWindowPos(hWnd,bAlwaysOnTop?HWND_TOPMOST:HWND_NOTOPMOST,
					0,0,0,0,
					SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
	}
}

BOOL IsStatusBarVisible()
{
	return (bDisplayStatusBar == TRUE && bIsFullScreen == FALSE);
}

///////////////////////////////////////////////////////////////////////////////
void SetThreadProcessorAndPriority()
{
	DWORD rc;
	int ProcessorMask;

	ProcessorMask = 1 << (DecodeProcessor);
	rc = SetThreadAffinityMask(GetCurrentThread(), ProcessorMask);
	
	if (ThreadClassId == 0)
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
	else if (ThreadClassId == 1)
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
	else if (ThreadClassId == 2)
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
	else if (ThreadClassId == 3)
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
	else if (ThreadClassId == 4)
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
}

void Cursor_SetVisibility(BOOL bVisible)
{
	static int nCursorIndex = 1;
	if(bVisible)
	{
		while(nCursorIndex < 1)
		{
			ShowCursor(TRUE);
			nCursorIndex++;
		}
	}
	else
	{
		while(nCursorIndex > 0)
		{
			ShowCursor(FALSE);
			nCursorIndex--;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
// On Change Functions for settings
/////////////////////////////////////////////////////////////////////////////

BOOL IsFullScreen_OnChange(long NewValue)
{
	bDoResize = FALSE;
	bIsFullScreen = (BOOL)NewValue;

	// make sure that the window has been created
	if(hWnd != NULL)
	{
		if(bIsFullScreen == FALSE)
		{
			SetWindowPos(hWnd, 0, emstartx, emstarty, emsizex, emsizey, SWP_SHOWWINDOW);
			Cursor_SetVisibility(bShowCursor);
			if (bDisplayStatusBar == TRUE)
			{
				SetTimer(hWnd, TIMER_STATUS, TIMER_STATUS_MS, NULL);
			}
		}
		else
		{
			SaveWindowPos(hWnd);
			Cursor_SetVisibility(FALSE);
		}
		WorkoutOverlaySize();
	}
	bDoResize = TRUE;
	return FALSE;
}

BOOL AlwaysOnTop_OnChange(long NewValue)
{
	bAlwaysOnTop = (BOOL)NewValue;
	WorkoutOverlaySize();
	return FALSE;
}

BOOL DisplayStatusBar_OnChange(long NewValue)
{
	bDisplayStatusBar = (BOOL)NewValue;
	if(bIsFullScreen == FALSE)
	{
		if(bDisplayStatusBar == TRUE)
		{
			SetTimer(hWnd, TIMER_STATUS, TIMER_STATUS_MS, NULL);
		}
		else
		{
			KillTimer(hWnd, TIMER_STATUS);
		}
		WorkoutOverlaySize();
	}
	return FALSE;
}

BOOL ShowMenu_OnChange(long NewValue)
{
	Show_Menu = (BOOL)NewValue;
	if(bIsFullScreen == FALSE)
	{
		WorkoutOverlaySize();
	}
	return FALSE;
}

void SwitchToSource(int nInput)
{
	Stop_Capture();
	VideoSource = nInput;
	OSD_ShowText(hWnd, GetSourceName(VideoSource), 0);
	StatusBar_ShowText(STATUS_KEY, GetSourceName(VideoSource));

	switch(nInput)
	{
	case SOURCE_TUNER:
		BT848_ResetHardware();
		BT848_SetGeoSize();
		WorkoutOverlaySize();
		if(Audio_MSP_IsPresent())
		{
			AudioSource = AUDIOMUX_MSP_RADIO;
		}
		else
		{
			AudioSource = AUDIOMUX_TUNER;
		}
		BT848_SetVideoSource(VideoSource);
		ChangeChannel(CurrentProgramm);
		break;

	// MAE 13 Dec 2000 for CCIR656 Digital input
    case SOURCE_CCIR656:
		BT848_ResetHardware();
		BT848_Enable656();
		WorkoutOverlaySize();
        AudioSource = AUDIOMUX_EXTERNAL;
		break;
/*
        // FIXME: We really need separate picture settings for each input.
		// Reset defaults for brightness, contrast, color U and V
		InitialBrightness = 0;
		InitialContrast = 0x80;
		InitialSaturationU = 0x80;
		InitialSaturationV = 0x80;
		BT848_SetBrightness(InitialBrightness);
		BT848_SetContrast(InitialContrast);
		BT848_SetSaturationU(InitialSaturationU);
		BT848_SetSaturationV(InitialSaturationV);
*/

	case SOURCE_COMPOSITE:
	case SOURCE_SVIDEO:
	case SOURCE_OTHER1:
	case SOURCE_OTHER2:
	case SOURCE_COMPVIASVIDEO:
		BT848_ResetHardware();
		BT848_SetGeoSize();
		WorkoutOverlaySize();
		BT848_SetVideoSource(VideoSource);
		AudioSource = AUDIOMUX_EXTERNAL;
		break;
	default:
		break;
	}

	if(!System_In_Mute)
	{
		Audio_SetSource(AudioSource);
	}
	Start_Capture();
}


////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING dTVSettings[DTV_SETTING_LASTONE] =
{
	{
		"Window Left", NUMBER, 0, &emstartx,
		10, 0, 2048, 0, NULL,
		"MainWindow", "StartLeft", NULL,
	},
	{
		"Window Top", NUMBER, 0, &emstarty,
		10, 0, 2048, 0, NULL,
		"MainWindow", "StartTop", NULL,
	},
	{
		"Window Width", NUMBER, 0, &emsizex,
		649, 0, 2048, 0, NULL,
		"MainWindow", "StartWidth", NULL,
	},
	{
		"Window Height", NUMBER, 0, &emsizey,
		547, 0, 2048, 0, NULL,
		"MainWindow", "StartHeight", NULL,
	},
	{
		"AlwaysOnTop", YESNO, 0, &bAlwaysOnTop,
		FALSE, 0, 1, 0, NULL,
		"MainWindow", "AlwaysOnTop", AlwaysOnTop_OnChange,
	},
	{
		"Display Splash Screen", YESNO, 0, &bDisplaySplashScreen,
		TRUE, 0, 1, 0, NULL,
		"MainWindow", "DisplaySplashScreen", NULL,
	},
	{
		"Full Screen", YESNO, 0, &bIsFullScreen,
		FALSE, 0, 1, 0, NULL,
		"MainWindow", "bIsFullScreen", IsFullScreen_OnChange,
	},
	{
		"Force Full Screen", YESNO, 0, &bForceFullScreen,
		FALSE, 0, 1, 0, NULL,
		"MainWindow", "AlwaysForceFullScreen", NULL,
	},
	{
		"Show Status Bar", YESNO, 0, &bDisplayStatusBar,
		TRUE, 0, 1, 0, NULL,
		"Show", "StatusBar", DisplayStatusBar_OnChange,
	},
	{
		"Show Menu", YESNO, 0, &Show_Menu,
		TRUE, 0, 1, 0, NULL,
		"Show", "Menu", ShowMenu_OnChange,
	},
	{
		"Window Processor", NUMBER, 0, &MainProcessor,
		0, 0, 3, 0, NULL,
		"Threads", "WindowProcessor", NULL,
	},
	{
		"Thread Processor", NUMBER, 0, &DecodeProcessor,
		0, 0, 3, 0, NULL,
		"Threads", "DecodeProcessor", NULL,
	},
	{
		"Window Priority", NUMBER, 0, &PriorClassId,
		0, 0, 2, 0, NULL,
		"Threads", "WindowPriority", NULL,
	},
	{
		"Thread Priority", NUMBER, 0, &ThreadClassId,
		1, 0, 4, 0, NULL,
		"Threads", "ThreadPriority", NULL,
	},
};

SETTING* dTV_GetSetting(DTV_SETTING Setting)
{
	if(Setting > -1 && Setting < DTV_SETTING_LASTONE)
	{
		return &(dTVSettings[Setting]);
	}
	else
	{
		return NULL;
	}
}

void dTV_ReadSettingsFromIni()
{
	int i;
	for(i = 0; i < DTV_SETTING_LASTONE; i++)
	{
		Setting_ReadFromIni(&(dTVSettings[i]));
	}
	if(bForceFullScreen)
	{
		bIsFullScreen = TRUE;
	}
}

void dTV_WriteSettingsToIni()
{
	int i;
	for(i = 0; i < DTV_SETTING_LASTONE; i++)
	{
		Setting_WriteToIni(&(dTVSettings[i]));
	}
}

