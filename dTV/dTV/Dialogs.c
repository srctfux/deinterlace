/////////////////////////////////////////////////////////////////////////////
// dialogs.h
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
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 09 Jan 2001   Tom Barry             Added Chip type to hardware dialog
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "dialogs.h"
#include "bt848.h"
#include "dTV.h"
#include "OutThreads.h"
#include "VBI_VideoText.h"
#include "audio.h"
#include "tuner.h"
#include "vbi.h"
#include "cpu.h"

int UTLoop=0;
int UTPage=150;
HWND UTList;

char VTtoAscii[96] =
{
 " !##$%&´()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZÄÖÜß#-abcdefghijklmnopqrstuvwxyzäöüß#"
};

void SetSliderInt(HWND hDlgItem, int xPos, int Value, int nMin, int nMax)
{
	int y = (int)(235 - ((double) (Value - nMin) / (nMax - nMin) * 160));
	MoveWindow(hDlgItem, xPos, y - 4, 22, 8, TRUE);
}

int GetSliderInt(int MouseY, int nMin, int nMax)
{
	int i;
	i = nMin - (int) ((double) (MouseY - 235) / 160 * (nMax - nMin));
	if (i < nMin)
		i = nMin;
	else if (i > nMax)
		i = nMax;
	return i;
}


BOOL APIENTRY VideoSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	static char THue;
	static char TBrightness;
	static int TContrast;
	static int TSaturationU;
	static int TSaturationV;
	static int TOverscan;
	static int LastSaturation;

	int x, y, j;

	switch (message)
	{
	case WM_INITDIALOG:

		TBrightness = InitialBrightness;
		TContrast = InitialContrast;
		THue = InitialHue;
		TSaturationU = InitialSaturationU;
		TSaturationV = InitialSaturationV;
		TOverscan = InitialOverscan;
		SetDlgItemInt(hDlg, IDC_D1, TBrightness, TRUE);
		SetDlgItemInt(hDlg, IDC_D2, TContrast, FALSE);
		SetDlgItemInt(hDlg, IDC_D3, THue, TRUE);
		LastSaturation = (TSaturationU + TSaturationV) / 2;
		SetDlgItemInt(hDlg, IDC_D4, LastSaturation, FALSE);
		SetDlgItemInt(hDlg, IDC_D5, TSaturationU, FALSE);
		SetDlgItemInt(hDlg, IDC_D6, TSaturationV, FALSE);
		SetDlgItemInt(hDlg, IDC_D7, TOverscan, FALSE);

		SetSliderInt(GetDlgItem(hDlg, IDC_S1), 33, TBrightness,  -128, 127);
		SetSliderInt(GetDlgItem(hDlg, IDC_S2), 109, TContrast,  0, 255);
		SetSliderInt(GetDlgItem(hDlg, IDC_S3), 183, THue, -128, 127);
		SetSliderInt(GetDlgItem(hDlg, IDC_S4), 258, LastSaturation, 0, 255);
		SetSliderInt(GetDlgItem(hDlg, IDC_S5), 334, TSaturationU, 0, 255);
		SetSliderInt(GetDlgItem(hDlg, IDC_S6), 410, TSaturationV, 0, 255);
		SetSliderInt(GetDlgItem(hDlg, IDC_S7), 482, TOverscan, 0, 128);
		break;

	case WM_MOUSEMOVE:

		y = HIWORD(lParam);
		x = LOWORD(lParam);
		if (wParam == MK_LBUTTON)
		{
			if ((y >= 70) && (y <= 240))
			{
				if ((x >= 33) && (x <= 55))
				{
					MoveWindow(GetDlgItem(hDlg, IDC_S1), 33, y - 2, 22, 8, TRUE);
					TBrightness = GetSliderInt(y, -127, 127);
					SetDlgItemInt(hDlg, IDC_D1, TBrightness, TRUE);
					BT848_SetBrightness(TBrightness);
				}
				else if ((x >= 109) && (x <= 131))
				{
					MoveWindow(GetDlgItem(hDlg, IDC_S2), 109, y - 2, 22, 8, TRUE);
					TContrast = GetSliderInt(y, 0, 255);
					SetDlgItemInt(hDlg, IDC_D2, TContrast, FALSE);
					BT848_SetContrast(TContrast);
				}
				else if ((x >= 183) && (x <= 205))
				{
					MoveWindow(GetDlgItem(hDlg, IDC_S3), 183, y - 2, 22, 8, TRUE);
					THue = GetSliderInt(y, -127, 127);
					SetDlgItemInt(hDlg, IDC_D3, THue, TRUE);
					BT848_SetHue(THue);
				}
				else if ((x >= 258) && (x <= 280))
				{
					j = GetSliderInt(y, 0, 255) - LastSaturation;

					if ((j + TSaturationV <= 255) && (j + TSaturationU <= 255) && (j + TSaturationV >= 0) && (j + TSaturationU >= 0))
					{

						MoveWindow(GetDlgItem(hDlg, IDC_S4), 258, y - 2, 22, 8, TRUE);
						TSaturationU += j;
						SetSliderInt(GetDlgItem(hDlg, IDC_S5), 334, TSaturationU, 0, 255);
						TSaturationV += j;
						SetSliderInt(GetDlgItem(hDlg, IDC_S6), 410, TSaturationV, 0, 255);
						LastSaturation = (TSaturationU + TSaturationV) / 2;
						SetDlgItemInt(hDlg, IDC_D5, TSaturationU, FALSE);
						SetDlgItemInt(hDlg, IDC_D6, TSaturationV, FALSE);
						SetDlgItemInt(hDlg, IDC_D4, LastSaturation, FALSE);
						BT848_SetSaturationU(TSaturationU);
						BT848_SetSaturationV(TSaturationV);
					}
				}
				else if ((x >= 334) && (x <= 356))
				{
					MoveWindow(GetDlgItem(hDlg, IDC_S5), 334, y - 2, 22, 8, TRUE);
					TSaturationU = GetSliderInt(y, 0, 255);
					LastSaturation = (TSaturationU + TSaturationV) / 2;
					SetDlgItemInt(hDlg, IDC_D4, LastSaturation, FALSE);
					SetSliderInt(GetDlgItem(hDlg, IDC_S4), 258, LastSaturation, 0, 255);
					SetDlgItemInt(hDlg, IDC_D5, TSaturationU, FALSE);
					BT848_SetSaturationU(TSaturationU);
				}
				else if ((x >= 410) && (x <= 432))
				{
					MoveWindow(GetDlgItem(hDlg, IDC_S6), 410, y - 2, 22, 8, TRUE);
					TSaturationV = GetSliderInt(y, 0, 255);
					LastSaturation = (TSaturationU + TSaturationV) / 2;
					SetDlgItemInt(hDlg, IDC_D4, LastSaturation, FALSE);
					SetSliderInt(GetDlgItem(hDlg, IDC_S4), 258, LastSaturation, 0, 255);
					SetDlgItemInt(hDlg, IDC_D6, TSaturationV, FALSE);
					BT848_SetSaturationV(TSaturationV);
				}
				else if ((x >= 482) && (x <= 504))
				{
					MoveWindow(GetDlgItem(hDlg, IDC_S7), 482, y - 2, 22, 8, TRUE);
					TOverscan = GetSliderInt(y, 0, 128);
					SetDlgItemInt(hDlg, IDC_D7, TOverscan, TRUE);
				}
			}
		}
		return (FALSE);
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			InitialBrightness = TBrightness;
			InitialContrast = TContrast;
			InitialHue = THue;
			InitialSaturationU = TSaturationU;
			InitialSaturationV = TSaturationV;
			InitialOverscan = TOverscan;
			EndDialog(hDlg, TRUE);
		}

		if (LOWORD(wParam) == IDCANCEL)
		{
			BT848_SetBrightness(InitialBrightness);
			BT848_SetContrast(InitialContrast);
			BT848_SetHue(InitialHue);
			BT848_SetSaturationU(InitialSaturationU);
			BT848_SetSaturationV(InitialSaturationV);
			EndDialog(hDlg, TRUE);
		}

		if (LOWORD(wParam) == IDDEFAULT)
		{
			THue = 0x00;
			TBrightness = 0x00;
			TContrast = 0xd8;
			TSaturationU = 0xfe;
			TSaturationV = 0xb4;
			TOverscan = 4;
			BT848_SetBrightness(TBrightness);
			BT848_SetContrast(TContrast);
			BT848_SetHue(THue);
			BT848_SetSaturationU(TSaturationU);
			BT848_SetSaturationV(TSaturationV);
			SetDlgItemInt(hDlg, IDC_D1, TBrightness, TRUE);
			SetDlgItemInt(hDlg, IDC_D2, TContrast, FALSE);
			SetDlgItemInt(hDlg, IDC_D3, THue, TRUE);
			SetDlgItemInt(hDlg, IDC_D4, ((TSaturationU + TSaturationV) / 2), FALSE);
			SetDlgItemInt(hDlg, IDC_D5, TSaturationU, FALSE);
			SetDlgItemInt(hDlg, IDC_D6, TSaturationV, FALSE);
			SetDlgItemInt(hDlg, IDC_D7, TOverscan, FALSE);
			SetSliderInt(GetDlgItem(hDlg, IDC_S1), 33, TBrightness,  -128, 127);
			SetSliderInt(GetDlgItem(hDlg, IDC_S2), 109, TContrast,  0, 255);
			SetSliderInt(GetDlgItem(hDlg, IDC_S3), 183, THue, -128, 127);
			SetSliderInt(GetDlgItem(hDlg, IDC_S4), 258, LastSaturation, 0, 255);
			SetSliderInt(GetDlgItem(hDlg, IDC_S5), 334, TSaturationU, 0, 255);
			SetSliderInt(GetDlgItem(hDlg, IDC_S6), 410, TSaturationV, 0, 255);
			SetSliderInt(GetDlgItem(hDlg, IDC_S7), 482, TOverscan, 0, 128);
			break;
		}
		break;
	}
	return (FALSE);
}

BOOL APIENTRY AdvVideoSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	BYTE flag;
	
	switch (message)
	{
	case WM_INITDIALOG:

		// Luma AGC, 0 says AGC enabled
		CheckDlgButton(hDlg, IDC_AGC, 
			((BT848_ReadByte(BT848_ADC) & BT848_ADC_AGC_EN) == 0));

		// Adaptive AGC, 0 says Crush disabled
		CheckDlgButton(hDlg, IDC_CRUSH, 
			((BT848_ReadByte(BT848_ADC) & BT848_ADC_CRUSH) != 0));

		// Even CAGC, 0 says CAGC disable
		CheckDlgButton(hDlg, IDC_E_CAGC, 
			((BT848_ReadByte(BT848_E_SCLOOP) & BT848_SCLOOP_CAGC) != 0));

		// Odd CAGC
		CheckDlgButton(hDlg, IDC_O_CAGC, 
			((BT848_ReadByte(BT848_O_SCLOOP) & BT848_SCLOOP_CAGC) != 0));
		
		// Even Peak, 0 says normal, not Luma peak
		CheckDlgButton(hDlg, IDC_E_LUMA_PEAK, 
			((BT848_ReadByte(BT848_E_SCLOOP) & BT848_SCLOOP_LUMA_PEAK) != 0));
		
		// Odd Peak
		CheckDlgButton(hDlg, IDC_O_LUMA_PEAK,  
			((BT848_ReadByte(BT848_O_SCLOOP) & BT848_SCLOOP_LUMA_PEAK) != 0));

		// Luma Output Range, 0 says Luma Normal, 1=Full	
		CheckDlgButton(hDlg, IDC_LUMA_RANGE, 
			((BT848_ReadByte(BT848_OFORM) & BT848_OFORM_RANGE) != 0));
		
		// Even Luma decimation,  0 says disable
		CheckDlgButton(hDlg, IDC_E_LUMA_DEC, 
			((BT848_ReadByte(BT848_E_CONTROL) & BT848_CONTROL_LDEC) != 0));
		
		// Odd Luma decimation
		CheckDlgButton(hDlg, IDC_O_LUMA_DEC, 
			((BT848_ReadByte(BT848_O_CONTROL) & BT848_CONTROL_LDEC) != 0));

		// Even COMB, 0 = disable
		CheckDlgButton(hDlg, IDC_E_COMB, 
			((BT848_ReadByte(BT848_E_VSCALE_HI) & BT848_VSCALE_COMB) != 0));
		
		// Odd COMB
		CheckDlgButton(hDlg, IDC_O_COMB, 
			((BT848_ReadByte(BT848_O_VSCALE_HI) & BT848_VSCALE_COMB) != 0));
		
		// Color Bars, 0 = disable
		CheckDlgButton(hDlg, IDC_COLOR_BARS, 
			((BT848_ReadByte(BT848_COLOR_CTL) & BT848_COLOR_CTL_COLOR_BARS) != 0));
		
		// Gamma correction removal, 0=enabled
		CheckDlgButton(hDlg, IDC_GAMMA_CORR, 
			((BT848_ReadByte(BT848_COLOR_CTL) & BT848_COLOR_CTL_GAMMA) == 0));

		// More Vertical Filter, 0=no, 4=yes, other values no good at our res
		// (Z filter)	TRB 12/19/00
		CheckDlgButton(hDlg, IDC_VERT_FILTER, 
			((BtVertFilter & BT848_VTC_VFILT_2TAPZ) == BT848_VTC_VFILT_2TAPZ));
		
		// More Horizontal Filter, 0=no, else max full res filter TRB 12/19/00
		CheckDlgButton(hDlg, IDC_HOR_FILTER, 
			((BtHorFilter & BT848_SCLOOP_HFILT_FULL) == BT848_SCLOOP_HFILT_FULL));
		

		break;

	case WM_MOUSEMOVE:

		return (FALSE);

	case WM_COMMAND:

		switch LOWORD(wParam)
		{
		case IDOK:							// Is Done
			EndDialog(hDlg, TRUE);
			break;

		case IDC_AGC:						// Changed AGC
			flag = (BYTE)(BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_AGC)
				? 0 
				: BT848_ADC_AGC_EN);		// 0 says AGC enabled
			BT848_MaskDataByte(BT848_ADC, flag, BT848_ADC_AGC_EN);
			BtAgcDisable = flag;
			break;  

		case IDC_CRUSH:						// Changed Adaptive AGC
			flag = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CRUSH)
				? BT848_ADC_CRUSH
				:0);						// 0 says Crush disabled
			BT848_MaskDataByte(BT848_ADC, flag, BT848_ADC_CRUSH);
			BtCrush = flag;
			break;  

		case IDC_E_CAGC:					// Changed Even CAGC
			flag = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_E_CAGC)
				? BT848_SCLOOP_CAGC
				: 0);						// 0 says CAGC disable
			BT848_MaskDataByte(BT848_E_SCLOOP, flag, BT848_SCLOOP_CAGC);
			BtEvenChromaAGC = flag;
			break;

		case IDC_O_CAGC:					// Changed Odd CAGC
			flag = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_O_CAGC)
				? BT848_SCLOOP_CAGC
				: 0);						// 0 says CAGC disable
			BT848_MaskDataByte(BT848_O_SCLOOP, flag, BT848_SCLOOP_CAGC);
			BtOddChromaAGC = flag;
			break;

		case IDC_E_LUMA_PEAK:				// Changed Even Peak
			flag = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_E_LUMA_PEAK)
				? BT848_SCLOOP_LUMA_PEAK
				: 0);						// 0 says normal, not Luma peak
			BT848_MaskDataByte(BT848_E_SCLOOP, flag, BT848_SCLOOP_LUMA_PEAK);
			BtEvenLumaPeak = flag;
			break;

		case IDC_O_LUMA_PEAK:				// Changed Odd Peak
			flag = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_O_LUMA_PEAK)
				? BT848_SCLOOP_LUMA_PEAK
				: 0);						// 0 says normal, not Luma peak
			BT848_MaskDataByte(BT848_O_SCLOOP, flag, BT848_SCLOOP_LUMA_PEAK);
			BtOddLumaPeak = flag;
			break;

		case IDC_LUMA_RANGE:				// Luma Output Range
			flag = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_LUMA_RANGE)
				? BT848_OFORM_RANGE
				: 0);						// 0 says Luma Normal, 1=Full
			BT848_MaskDataByte(BT848_OFORM, flag, BT848_OFORM_RANGE);
			BtFullLumaRange = flag;
			break;

		case IDC_E_LUMA_DEC:				// Changed Even L.decimation
			flag = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_E_LUMA_DEC)
				? BT848_CONTROL_LDEC
				: 0);						// 0 says disable
			BT848_MaskDataByte(BT848_E_CONTROL, flag, BT848_CONTROL_LDEC);
			BtEvenLumaDec = flag;
			break;

		case IDC_O_LUMA_DEC:				// Changed Odd L.decimation
			flag = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_O_LUMA_DEC)
				? BT848_CONTROL_LDEC
				: 0);						// 0 says disable
			BT848_MaskDataByte(BT848_O_CONTROL, flag, BT848_CONTROL_LDEC);
			BtOddLumaDec = flag;
			break;

		case IDC_E_COMB:					// Changed Even COMB
			flag = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_E_COMB)
				? BT848_VSCALE_COMB
				: 0);						// 0 says disable
			BT848_MaskDataByte(BT848_E_VSCALE_HI, flag, BT848_VSCALE_COMB);
			BtEvenComb = flag;
			break;

		case IDC_O_COMB:					// Changed Odd COMB
			flag = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_O_COMB)
				? BT848_VSCALE_COMB
				: 0);						// 0 says disable
			BT848_MaskDataByte(BT848_O_VSCALE_HI, flag, BT848_VSCALE_COMB);
			BtOddComb = flag;
			break;

		case IDC_COLOR_BARS:				// Color Bars
			flag = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_COLOR_BARS)
				? BT848_COLOR_CTL_COLOR_BARS
				: 0);						// 0 says disable
			BT848_MaskDataByte(BT848_COLOR_CTL, flag, BT848_COLOR_CTL_COLOR_BARS);
			BtColorBars = flag;
			break;

		case IDC_GAMMA_CORR:				// Gamma correction removal
			flag = (BYTE)(BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_GAMMA_CORR)
				? 0
				: BT848_COLOR_CTL_GAMMA );	// 0 says enable removal
			BT848_MaskDataByte(BT848_COLOR_CTL, flag, BT848_COLOR_CTL_GAMMA);
			BtGammaCorrection = flag;
			break;

		case IDC_VERT_FILTER:				// Use vertical z-filter
			flag = (BYTE)(BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_VERT_FILTER)
				? BT848_VTC_VFILT_2TAPZ		// yes, use z-filter
				: 0);
			BT848_MaskDataByte(BT848_E_VTC, flag, BT848_VTC_VFILT_2TAPZ);
			BT848_MaskDataByte(BT848_O_VTC, flag, BT848_VTC_VFILT_2TAPZ);
			BtVertFilter = flag;
			break;

		case IDC_HOR_FILTER:				// Use Hor peaking filter
			flag = (BYTE)(BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_HOR_FILTER)
				? BT848_SCLOOP_HFILT_FULL	// yes, use max full res peak
				: 0);
			BT848_MaskDataByte(BT848_E_SCLOOP, flag, BT848_SCLOOP_HFILT_FULL);
			BT848_MaskDataByte(BT848_O_SCLOOP, flag, BT848_SCLOOP_HFILT_FULL);
			BtHorFilter = flag;
			break;

		}
		break;
	}
	return (FALSE);
}

void SetHorSliderInt(HWND hDlgItem, int yPos, int Value, int nMin, int nMax)
{
	int x = 5 + 160 *(Value - nMin) / (nMax - nMin);
	MoveWindow(hDlgItem, x, yPos +5, 10, 12, TRUE);
}

int GetHorSliderInt(int MouseX, int nMin, int nMax)
{
	int i;
	i = nMin + (MouseX - 5) * (nMax - nMin) / 160;
	if (i < nMin)
		i = nMin;
	else if (i > nMax)
		i = nMax;
	return i;
}

BOOL APIENTRY AudioSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	static BOOL TSuperBass;
	static int  TVolume;
	static char TBalance;
	static char TSpecial;
	static char TLoudness;
	static char TBass;
	static char TTreble;

	int x, y, i;

	switch (message)
	{
	case WM_INITDIALOG:
		if (Audio_MSP_IsPresent() == FALSE)
		{
			ErrorBox("No MSP Audio Device found");
			EndDialog(hDlg, 0);
		}

		TVolume = InitialVolume;
		TSpecial = InitialSpatial;
		TLoudness = InitialLoudness;
		TBass = InitialBass;
		TTreble = InitialTreble;
		TBalance = InitialBalance;
		TSuperBass = InitialSuperBass;

		SetDlgItemInt(hDlg, IDC_D1, TVolume, FALSE);
		SetDlgItemInt(hDlg, IDC_D2, TSpecial, TRUE);
		SetDlgItemInt(hDlg, IDC_D3, TLoudness, TRUE);
		SetDlgItemInt(hDlg, IDC_D4, TBass, TRUE);
		SetDlgItemInt(hDlg, IDC_D5, TTreble, TRUE);
		SetDlgItemInt(hDlg, IDC_D6, TBalance, TRUE);

		CheckDlgButton(hDlg, IDC_CHECK1, TSuperBass);

		y = (int) (235 - ((double) (TVolume + 1) / 1000 * 160));
		MoveWindow(GetDlgItem(hDlg, IDC_S1), 33, y - 4, 22, 8, FALSE);

		y = (int) (157 - ((double) (TSpecial + 1) / 256 * 160));
		MoveWindow(GetDlgItem(hDlg, IDC_S2), 109, y - 4, 22, 8, FALSE);

		y = (int) (235 - ((TLoudness + 1) / 68 * 160));
		MoveWindow(GetDlgItem(hDlg, IDC_S3), 183, y - 4, 22, 8, FALSE);

		y = (int) (157 - ((double) (TBass + 1) / 256 * 160));
		MoveWindow(GetDlgItem(hDlg, IDC_S4), 258, y - 4, 22, 8, FALSE);

		y = (int) (157 - ((double) (TTreble + 1) / 256 * 160));
		MoveWindow(GetDlgItem(hDlg, IDC_S5), 334, y - 4, 22, 8, FALSE);

		x = (int) (279 - ((double) (TBalance + 1) / 256 * 160));
		MoveWindow(GetDlgItem(hDlg, IDC_S6), x - 4, 271, 8, 22, FALSE);

		break;

	case WM_MOUSEMOVE:

		y = HIWORD(lParam);
		x = LOWORD(lParam);
		if (wParam == MK_LBUTTON)
		{
			if ((y >= 70) && (y <= 240))
			{
				if ((x >= 33) && (x <= 55))
				{
					i = (int) ((double) (y - 235) / 160 * 1000);
					i = -i;
					if (i < 0)
						i = 0;
					else if (i > 1000)
						i = 1000;
					MoveWindow(GetDlgItem(hDlg, IDC_S1), 33, y - 2, 22, 8, TRUE);
					InitialVolume = i;
					SetDlgItemInt(hDlg, IDC_D1, InitialVolume, FALSE);
					Audio_SetVolume(InitialVolume);
				}
				if ((x >= 109) && (x <= 131))
				{
					i = (int) ((double) (y - 155) / 160 * 255);
					if (i < -127)
						i = -127;
					else if (i > 128)
						i = 128;
					i = -i;
					MoveWindow(GetDlgItem(hDlg, IDC_S2), 109, y - 2, 22, 8, TRUE);
					InitialSpatial = i;
					SetDlgItemInt(hDlg, IDC_D2, InitialSpatial, TRUE);
					Audio_SetSpatial(InitialSpatial);
				}

				if ((x >= 183) && (x <= 205))
				{
					i = (int) ((double) (y - 235) / 160 * 68);
					i = -i;
					if (i < 0)
						i = 0;
					else if (i > 68)
						i = 68;
					MoveWindow(GetDlgItem(hDlg, IDC_S3), 183, y - 2, 22, 8, TRUE);
					InitialLoudness = i;
					SetDlgItemInt(hDlg, IDC_D3, InitialLoudness, FALSE);
					Audio_SetLoudness(InitialLoudness);
				}
				if ((x >= 258) && (x <= 280))
				{
					i = (int) ((double) (y - 155) / 160 * 255);
					i = -i;
					if (i < -96)
						i = -96;
					else if (i > 127)
						i = 127;
					MoveWindow(GetDlgItem(hDlg, IDC_S4), 258, y - 2, 22, 8, TRUE);
					InitialBass = i;
					SetDlgItemInt(hDlg, IDC_D4, InitialBass, TRUE);
					Audio_SetBass(InitialBass);
				}
				if ((x >= 334) && (x <= 356))
				{
					i = (int) ((double) (y - 155) / 160 * 255);
					i = -i;
					if (i < -96)
						i = -96;
					else if (i > 127)
						i = 127;
					MoveWindow(GetDlgItem(hDlg, IDC_S5), 334, y - 2, 22, 8, TRUE);
					InitialTreble = i;
					SetDlgItemInt(hDlg, IDC_D5, InitialTreble, TRUE);
					Audio_SetTreble(InitialTreble);
				}

			}
			if ((y >= 271) && (y <= 293) && (x >= 195) && (x <= 360))
			{

				i = (int) ((double) (x - 277) / 160 * 255);
				i = -i;
				if (i < -127)
					i = -127;
				else if (i > 127)
					i = 127;
				InitialBalance = i;
				SetDlgItemInt(hDlg, IDC_D6, InitialBalance, TRUE);
				x = (int) (279 - ((double) (InitialBalance + 1) / 256 * 160));
				MoveWindow(GetDlgItem(hDlg, IDC_S6), x - 4, 271, 8, 22, TRUE);
				Audio_SetBalance(InitialBalance);	// -127 - +128

			}

		}
		return (FALSE);
	case WM_COMMAND:
		if ((HWND) lParam == GetDlgItem(hDlg, IDC_CHECK1))
			Audio_SetSuperBass(IsDlgButtonChecked(hDlg, IDC_CHECK1));

		if (LOWORD(wParam) == IDOK)
		{

			Audio_SetSource(AUDIOMUX_MUTE);
			Audio_SetSuperBass(IsDlgButtonChecked(hDlg, IDC_CHECK1));
			Audio_SetVolume(InitialVolume);
			Audio_SetSpatial(InitialSpatial);
			Audio_SetLoudness(InitialLoudness);
			Audio_SetBass(InitialBass);
			Audio_SetTreble(InitialTreble);
			Audio_SetBalance(InitialBalance);

			Audio_SetSource(AudioSource);
			EndDialog(hDlg, TRUE);
		}

		if (LOWORD(wParam) == IDCANCEL)
		{
			Audio_SetSource(AUDIOMUX_MUTE);
			Audio_SetSuperBass(TSuperBass);
			Audio_SetVolume(TVolume);
			Audio_SetSpatial(TSpecial);
			Audio_SetLoudness(TLoudness);
			Audio_SetBass(TBass);
			Audio_SetTreble(TTreble);
			Audio_SetBalance(TBalance);
			Audio_SetSource(AudioSource);
			EndDialog(hDlg, TRUE);
		}

		if (LOWORD(wParam) == IDDEFAULT)
		{

			TVolume = 1000;
			TBalance = 0x00;
			TSpecial = 0x00;
			TLoudness = 0x00;
			TBass = 0x00;
			TTreble = 0x00;
			CheckDlgButton(hDlg, IDC_CHECK1, FALSE);
			Audio_SetSource(AUDIOMUX_MUTE);
			Audio_SetSuperBass(FALSE);
			Audio_SetVolume(TVolume);
			Audio_SetSpatial(TSpecial);
			Audio_SetLoudness(TLoudness);
			Audio_SetBass(TBass);
			Audio_SetTreble(TTreble);
			Audio_SetBalance(TBalance);

			SetDlgItemInt(hDlg, IDC_D1, TVolume, FALSE);
			SetDlgItemInt(hDlg, IDC_D2, TSpecial, TRUE);
			SetDlgItemInt(hDlg, IDC_D3, TLoudness, TRUE);
			SetDlgItemInt(hDlg, IDC_D4, TBass, TRUE);
			SetDlgItemInt(hDlg, IDC_D5, TTreble, TRUE);
			SetDlgItemInt(hDlg, IDC_D6, TBalance, TRUE);
			Audio_SetSource(AudioSource);

			y = (int) (235 - ((double) (TVolume + 1) / 1000 * 160));
			MoveWindow(GetDlgItem(hDlg, IDC_S1), 33, y - 4, 22, 8, TRUE);

			y = (int) (157 - ((double) (TSpecial + 1) / 256 * 160));
			MoveWindow(GetDlgItem(hDlg, IDC_S2), 109, y - 4, 22, 8, TRUE);

			y = (int) (235 - ((TLoudness + 1) / 68 * 160));
			MoveWindow(GetDlgItem(hDlg, IDC_S3), 183, y - 4, 22, 8, TRUE);

			y = (int) (157 - ((double) (TBass + 1) / 256 * 160));
			MoveWindow(GetDlgItem(hDlg, IDC_S4), 258, y - 4, 22, 8, TRUE);

			y = (int) (157 - ((double) (TTreble + 1) / 256 * 160));
			MoveWindow(GetDlgItem(hDlg, IDC_S5), 334, y - 4, 22, 8, TRUE);

			x = (int) (279 - ((double) (TBalance + 1) / 256 * 160));
			MoveWindow(GetDlgItem(hDlg, IDC_S6), x - 4, 271, 8, 22, TRUE);

			break;

		}

		break;
	}

	return (FALSE);
}

BOOL APIENTRY AudioSettingProc1(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	char TEqualizer[5];
	int x, y, i;

	switch (message)
	{
	case WM_INITDIALOG:
		if (Audio_MSP_IsPresent() == FALSE)
		{
			ErrorBox("No MSP Audio Device found");
			EndDialog(hDlg, 0);
		}

		for (i = 0; i < 5; i++)
		{
			TEqualizer[i] = InitialEqualizer[i];
			SetDlgItemInt(hDlg, IDC_D1 + i, TEqualizer[i], TRUE);
			y = (int) (157 - ((double) (TEqualizer[i]) / 192 * 210));
			MoveWindow(GetDlgItem(hDlg, IDC_S1 + i), 33, y - 4, 22, 8, FALSE);
		}
		break;

	case WM_MOUSEMOVE:

		y = HIWORD(lParam);
		x = LOWORD(lParam);
		if (wParam == MK_LBUTTON)
		{
			if ((y >= 70) && (y <= 240))
			{
				if ((x >= 33) && (x <= 55))
				{
					i = (int) ((double) (y - 155) / 210 * 192);
					if (i < -69)
						i = -69;
					else if (i > 69)
						i = 69;
					i = -i;
					MoveWindow(GetDlgItem(hDlg, IDC_S1), 33, y - 2, 22, 8, TRUE);
					InitialEqualizer[0] = i;
					SetDlgItemInt(hDlg, IDC_D1, InitialEqualizer[0], TRUE);
					Audio_SetEqualizer(0, InitialEqualizer[0]);
				}
				if ((x >= 109) && (x <= 131))
				{
					i = (int) ((double) (y - 155) / 210 * 192);
					if (i < -69)
						i = -69;
					else if (i > 69)
						i = 69;
					i = -i;
					MoveWindow(GetDlgItem(hDlg, IDC_S2), 109, y - 2, 22, 8, TRUE);
					InitialEqualizer[1] = i;
					SetDlgItemInt(hDlg, IDC_D2, InitialEqualizer[1], TRUE);
					Audio_SetEqualizer(1, InitialEqualizer[1]);
				}
				if ((x >= 183) && (x <= 205))
				{
					i = (int) ((double) (y - 155) / 210 * 192);
					if (i < -69)
						i = -69;
					else if (i > 69)
						i = 69;
					i = -i;
					MoveWindow(GetDlgItem(hDlg, IDC_S3), 183, y - 2, 22, 8, TRUE);
					InitialEqualizer[2] = i;
					SetDlgItemInt(hDlg, IDC_D3, InitialEqualizer[2], TRUE);
					Audio_SetEqualizer(2, InitialEqualizer[2]);
				}
				if ((x >= 258) && (x <= 280))
				{
					i = (int) ((double) (y - 155) / 210 * 192);
					if (i < -69)
						i = -69;
					else if (i > 69)
						i = 69;
					i = -i;
					MoveWindow(GetDlgItem(hDlg, IDC_S4), 258, y - 2, 22, 8, TRUE);
					InitialEqualizer[3] = i;
					SetDlgItemInt(hDlg, IDC_D4, InitialEqualizer[3], TRUE);
					Audio_SetEqualizer(3, InitialEqualizer[3]);
				}
				if ((x >= 334) && (x <= 356))
				{
					i = (int) ((double) (y - 155) / 210 * 192);
					if (i < -69)
						i = -69;
					else if (i > 69)
						i = 69;
					i = -i;
					MoveWindow(GetDlgItem(hDlg, IDC_S5), 334, y - 2, 22, 8, TRUE);
					InitialEqualizer[4] = i;
					SetDlgItemInt(hDlg, IDC_D5, InitialEqualizer[4], TRUE);
					Audio_SetEqualizer(4, InitialEqualizer[4]);
				}
			}

		}
		return (FALSE);
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			Audio_SetSource(AUDIOMUX_MUTE);
			for(i = 0; i < 5; i++)
			{
				Audio_SetEqualizer(i, InitialEqualizer[i]);
			}
			Audio_SetSource(AudioSource);
			EndDialog(hDlg, TRUE);
		}

		if (LOWORD(wParam) == IDCANCEL)
		{
			Audio_SetSource(AUDIOMUX_MUTE);
			for(i = 0; i < 5; i++)
			{
				Audio_SetEqualizer(i, TEqualizer[i]);
			}
			Audio_SetSource(AudioSource);
			EndDialog(hDlg, TRUE);
		}

		if (LOWORD(wParam) == IDDEFAULT)
		{
			Audio_SetSource(AUDIOMUX_MUTE);
			for(i = 0; i < 5; i++)
			{
				TEqualizer[i] = 0x00;
				Audio_SetEqualizer(i, 0);
				SetDlgItemInt(hDlg, IDC_D1 + i, TEqualizer[i], TRUE);
				y = (int) (157 - ((double) (TEqualizer[i]) / 192 * 210));
				MoveWindow(GetDlgItem(hDlg, IDC_S1 + i), 33, y - 4, 22, 8, TRUE);
			}
			Audio_SetSource(AudioSource);
			break;
		}

		break;
	}

	return (FALSE);
}

BOOL APIENTRY SplashProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{

	switch (message)
	{
	case WM_INITDIALOG:
		InvalidateRect(hDlg, NULL, TRUE);
		SetTimer(hDlg, 2, 5000, NULL);
		return (TRUE);

	case WM_TIMER:
		if (wParam == 2)
		{
			SplashWnd  = NULL;
			EndDialog(hDlg, 0);
		}

		return (FALSE);
	}
	return (FALSE);
	UNREFERENCED_PARAMETER(lParam);
}

BOOL APIENTRY AboutProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	DWORD   dwVerInfoSize;		// Size of version information block
	LPSTR   lpVersion;			// String pointer to 'version' text
	DWORD   dwVerHnd=0;			// An 'ignored' parameter, always '0'
	UINT    uVersionLen;		// Current length of full version string
	WORD    wRootLen;			// length of the 'root' portion of string
	char    szFullPath[MAX_PATH];	// full path of module
	char    szResult[256];		// Temporary result string
	char    szGetName[256];		// String to use for extracting version info

	switch (message)
	{
	case WM_INITDIALOG:
			// Now lets dive in and pull out the version information:
			GetModuleFileName (hInst, szFullPath, sizeof(szFullPath));
			dwVerInfoSize = GetFileVersionInfoSize(szFullPath, &dwVerHnd);
			if (dwVerInfoSize)
			{
				LPSTR   lpstrVffInfo;
				HANDLE  hMem;
				hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
				lpstrVffInfo  = GlobalLock(hMem);
				GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpstrVffInfo);
				// The below 'hex' value looks a little confusing, but
				// essentially what it is, is the hexidecimal representation
				// of a couple different values that represent the language
				// and character set that we are wanting string values for.
				// 040904E4 is a very common one, because it means:
				//   US English, Windows MultiLingual characterset
				// Or to pull it all apart:
				// 04------        = SUBLANG_ENGLISH_USA
				// --09----        = LANG_ENGLISH
				// ----04BO        = Codepage
				lstrcpy(szGetName, "\\StringFileInfo\\040904B0\\");	 
				wRootLen = lstrlen(szGetName); // Save this position
				
				// Set the title of the dialog:
				lstrcat (szGetName, "ProductName");
				if(VerQueryValue((LPVOID)lpstrVffInfo,
					(LPSTR)szGetName,
					(LPVOID)&lpVersion,
					(UINT *)&uVersionLen))
				{
					lstrcpy(szResult, "About ");
					lstrcat(szResult, lpVersion);
					SetWindowText (hDlg, szResult);

					lstrcpy(szResult, lpVersion);
					lstrcat(szResult, " Version ");

					szGetName[wRootLen] = (char)0;
					lstrcat (szGetName, "ProductVersion");

					if(VerQueryValue((LPVOID)lpstrVffInfo,
						(LPSTR)szGetName,
						(LPVOID)&lpVersion,
						(UINT *)&uVersionLen))
					{
						lstrcat(szResult, lpVersion);
						lstrcat(szResult, " Compiled ");
						lstrcat(szResult, __DATE__);
						lstrcat(szResult, " ");
						lstrcat(szResult, __TIME__);

						SetWindowText (GetDlgItem(hDlg, IDC_VERSION), szResult);
					}
				}
			} // if (dwVerInfoSize)

		break;
	case WM_COMMAND:
		if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
		{
			EndDialog(hDlg, TRUE);
		}
		break;
	}

	return (FALSE);
}

BOOL APIENTRY VTInfoProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{

	switch (message)
	{
	case WM_INITDIALOG:
		ShowVTInfo = hDlg;
		SetTimer(hDlg, 0, 2000, NULL);
	case WM_TIMER:
		if (UTPages[0] != 0)
			SetDlgItemInt(hDlg, TEXT8, UTPages[0], FALSE);
		if (UTPages[1] != 0)
			SetDlgItemInt(hDlg, TEXT9, UTPages[1], FALSE);
		if (UTPages[2] != 0)
			SetDlgItemInt(hDlg, TEXT10, UTPages[2], FALSE);
		if (UTPages[3] != 0)
			SetDlgItemInt(hDlg, TEXT11, UTPages[3], FALSE);
		if (UTPages[4] != 0)
			SetDlgItemInt(hDlg, TEXT12, UTPages[4], FALSE);
		if (UTPages[5] != 0)
			SetDlgItemInt(hDlg, TEXT13, UTPages[5], FALSE);
		if (UTPages[6] != 0)
			SetDlgItemInt(hDlg, TEXT14, UTPages[6], FALSE);
		if (UTPages[7] != 0)
			SetDlgItemInt(hDlg, TEXT15, UTPages[7], FALSE);
		if (UTPages[8] != 0)
			SetDlgItemInt(hDlg, TEXT16, UTPages[8], FALSE);
		if (UTPages[9] != 0)
			SetDlgItemInt(hDlg, TEXT17, UTPages[9], FALSE);
		if (UTPages[10] != 0)
			SetDlgItemInt(hDlg, TEXT18, UTPages[10], FALSE);
		if (UTPages[11] != 0)
			SetDlgItemInt(hDlg, TEXT19, UTPages[11], FALSE);
		SetDlgItemInt(hDlg, IDT_VBI_FPS, VBI_FPS, FALSE);

		break;

	case WM_COMMAND:
		if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
		{
			KillTimer(hDlg, 0);
			ShowVTInfo = NULL;
			EndDialog(hDlg, TRUE);
		}
		break;
	}

	return (FALSE);
}

BOOL APIENTRY VPSInfoProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{

	switch (message)
	{
	case WM_INITDIALOG:
		ShowVPSInfo = hDlg;
		SetTimer(hDlg, 100, 1000, NULL);
		break;
	case WM_TIMER:
		SetDlgItemInt(hDlg, IDT_VBI_FPS, VBI_FPS, FALSE);
		break;

	case WM_COMMAND:
		if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
		{
			ShowVPSInfo = NULL;
			KillTimer(hDlg, 100);
			EndDialog(hDlg, TRUE);
		}
		break;
	}

	return (FALSE);
}

BOOL APIENTRY ChipSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_TEXT1, BT848_ChipType());
		SetDlgItemText(hDlg, IDC_TEXT6, BT848_VendorID());
		SetDlgItemText(hDlg, IDC_TEXT7, BT848_DeviceID());
		SetDlgItemText(hDlg, IDC_TEXT13, Tuner_Status());
		SetDlgItemText(hDlg, IDC_TEXT14, Audio_MSP_Status());
		SetDlgItemText(hDlg, IDC_TEXT16, Audio_MSP_VersionString());

		SetDlgItemText(hDlg, IDC_TEXT18, "YUV2");
		
		// TB 20010109 added Chip type
		if (CpuFeatureFlags & FEATURE_SSE2)
		{
			SetDlgItemText(hDlg, IDC_CPU_TYPE, "SSE2");
		}
		else if (CpuFeatureFlags & FEATURE_SSE)
		{
			SetDlgItemText(hDlg, IDC_CPU_TYPE, "SSE");
		}
		else if (CpuFeatureFlags & FEATURE_MMXEXT)
		{
			SetDlgItemText(hDlg, IDC_CPU_TYPE, "MMXEXT");
		}
		else if (CpuFeatureFlags & FEATURE_3DNOWEXT)
		{
			SetDlgItemText(hDlg, IDC_CPU_TYPE, "3DNOWEXT");
		}
		else if (CpuFeatureFlags & FEATURE_3DNOW)
		{
			SetDlgItemText(hDlg, IDC_CPU_TYPE, "3DNOW");
		}
		else
		{
			SetDlgItemText(hDlg, IDC_CPU_TYPE, "MMX");
		}


		break;

	case WM_COMMAND:

		if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
		{
			EndDialog(hDlg, TRUE);
		}

		break;
	}

	return (FALSE);
}

BOOL APIENTRY SelectCardProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	int i;
	switch (message)
	{
	case WM_INITDIALOG:
		SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_RESETCONTENT, 0, 0);
		for(i = 0; i < TVCARD_LASTONE; i++)
		{
			SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_ADDSTRING, 0, (LONG)TVCards[i].szName);
		}
		SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_SETCURSEL, CardType, 0);

		SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_RESETCONTENT, 0, 0);
		for(i = 0; i < TUNER_LASTONE; i++)
		{
			SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_ADDSTRING, 0, (LONG)Tuners[i].szName);
		}
		SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_SETCURSEL, TunerType, 0);

		SetFocus(hDlg);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			TunerType = SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_GETCURSEL, 0, 0);
			CardType = SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_GETCURSEL, 0, 0);
			EndDialog(hDlg, TRUE);
			break;
		case IDCANCEL:
			EndDialog(hDlg, TRUE);
			break;
		case IDC_CARDSSELECT:
			i = SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_GETCURSEL, 0, 0);
			if(TVCards[i].TunerId == TUNER_USER_SETUP)
			{
				SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_SETCURSEL, TUNER_ABSENT, 0);
			}
			else if(TVCards[i].TunerId == TUNER_AUTODETECT)
			{
				SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_SETCURSEL, Card_AutoDetectTuner(i), 0);
			}
			else
			{
				SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_SETCURSEL, TVCards[i].TunerId, 0);
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return (FALSE);
}
