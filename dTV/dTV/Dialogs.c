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

struct TUT UT[128];
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
			break;  

		case IDC_CRUSH:						// Changed Adaptive AGC
			flag = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CRUSH)
				? BT848_ADC_CRUSH
				:0);						// 0 says Crush disabled
			BT848_MaskDataByte(BT848_ADC, flag, BT848_ADC_CRUSH);
			break;  

		case IDC_E_CAGC:					// Changed Even CAGC
			flag = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_E_CAGC)
				? BT848_SCLOOP_CAGC
				: 0);						// 0 says CAGC disable
			BT848_MaskDataByte(BT848_E_SCLOOP, flag, BT848_SCLOOP_CAGC);
			break;

		case IDC_O_CAGC:					// Changed Odd CAGC
			flag = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_O_CAGC)
				? BT848_SCLOOP_CAGC
				: 0);						// 0 says CAGC disable
			BT848_MaskDataByte(BT848_O_SCLOOP, flag, BT848_SCLOOP_CAGC);
			break;

		case IDC_E_LUMA_PEAK:				// Changed Even Peak
			flag = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_E_LUMA_PEAK)
				? BT848_SCLOOP_LUMA_PEAK
				: 0);						// 0 says normal, not Luma peak
			BT848_MaskDataByte(BT848_E_SCLOOP, flag, BT848_SCLOOP_LUMA_PEAK);
			break;

		case IDC_O_LUMA_PEAK:				// Changed Odd Peak
			flag = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_O_LUMA_PEAK)
				? BT848_SCLOOP_LUMA_PEAK
				: 0);						// 0 says normal, not Luma peak
			BT848_MaskDataByte(BT848_O_SCLOOP, flag, BT848_SCLOOP_LUMA_PEAK);
			break;

		case IDC_LUMA_RANGE:				// Luma Output Range
			flag = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_LUMA_RANGE)
				? BT848_OFORM_RANGE
				: 0);						// 0 says Luma Normal, 1=Full
			BT848_MaskDataByte(BT848_OFORM, flag, BT848_OFORM_RANGE);
			break;

		case IDC_E_LUMA_DEC:				// Changed Even L.decimation
			flag = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_E_LUMA_DEC)
				? BT848_CONTROL_LDEC
				: 0);						// 0 says disable
			BT848_MaskDataByte(BT848_E_CONTROL, flag, BT848_CONTROL_LDEC);
			break;

		case IDC_O_LUMA_DEC:				// Changed Odd L.decimation
			flag = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_O_LUMA_DEC)
				? BT848_CONTROL_LDEC
				: 0);						// 0 says disable
			BT848_MaskDataByte(BT848_O_CONTROL, flag, BT848_CONTROL_LDEC);
			break;

		case IDC_E_COMB:					// Changed Even COMB
			flag = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_E_COMB)
				? BT848_VSCALE_COMB
				: 0);						// 0 says disable
			BT848_MaskDataByte(BT848_E_VSCALE_HI, flag, BT848_VSCALE_COMB);
			break;

		case IDC_O_COMB:					// Changed Odd COMB
			flag = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_O_COMB)
				? BT848_VSCALE_COMB
				: 0);						// 0 says disable
			BT848_MaskDataByte(BT848_O_VSCALE_HI, flag, BT848_VSCALE_COMB);
			break;

		case IDC_COLOR_BARS:				// Color Bars
			flag = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_COLOR_BARS)
				? BT848_COLOR_CTL_COLOR_BARS
				: 0);						// 0 says disable
			BT848_MaskDataByte(BT848_COLOR_CTL, flag, BT848_COLOR_CTL_COLOR_BARS);
			break;

		case IDC_GAMMA_CORR:				// Gamma correction removal
			flag = (BYTE)(BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_GAMMA_CORR)
				? 0
				: BT848_COLOR_CTL_GAMMA );	// 0 says enabled
			BT848_MaskDataByte(BT848_COLOR_CTL, flag, BT848_COLOR_CTL_GAMMA);
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

void SetBlcDisplayControls(HWND hDlg)
{
	// Set the sliders and visible numbers correctly
	SetDlgItemInt(hDlg, IDC_MIN_CLIP_V, BlcMinimumClip, FALSE);
	SetHorSliderInt(GetDlgItem(hDlg, IDC_MIN_CLIP),  35, BlcMinimumClip,  0, 100);

	SetDlgItemInt(hDlg, IDC_PIXEL_MOV_V, BlcPixelMotionSense, FALSE);
	SetHorSliderInt(GetDlgItem(hDlg, IDC_PIXEL_MOV),  85, BlcPixelMotionSense,  0, 100);
	
	SetDlgItemInt(hDlg, IDC_AVG_MOV_V, BlcRecentMotionSense, FALSE);
	SetHorSliderInt(GetDlgItem(hDlg, IDC_AVG_MOV),  135, BlcRecentMotionSense,  0, 100);
	
	SetDlgItemInt(hDlg, IDC_MOV_PERIOD_V, BlcMotionAvgPeriod, FALSE);
	SetHorSliderInt(GetDlgItem(hDlg, IDC_MOV_PERIOD),  185,  BlcMotionAvgPeriod,  1, 200);
	
	SetDlgItemInt(hDlg, IDC_PIXEL_COMB_V, BlcPixelCombSense, FALSE);
	SetHorSliderInt(GetDlgItem(hDlg, IDC_PIXEL_COMB),  235, BlcPixelCombSense,  0, 100);
	
	SetDlgItemInt(hDlg, IDC_AVG_COMB_V, BlcRecentCombSense, FALSE);
	SetHorSliderInt(GetDlgItem(hDlg, IDC_AVG_COMB),  285, BlcRecentCombSense,  0, 100);
	
	SetDlgItemInt(hDlg, IDC_COMB_PERIOD_V, BlcCombAvgPeriod, FALSE);
	SetHorSliderInt(GetDlgItem(hDlg, IDC_COMB_PERIOD),  335, BlcCombAvgPeriod,  1, 200);
	
	SetDlgItemInt(hDlg, IDC_COMB_SKIP_V, BlcHighCombSkip, FALSE);
	SetHorSliderInt(GetDlgItem(hDlg, IDC_COMB_SKIP),  385, BlcHighCombSkip,  0, 100);
	
	SetDlgItemInt(hDlg, IDC_MOTION_SKIP_V, BlcLowMotionSkip, FALSE);
	SetHorSliderInt(GetDlgItem(hDlg, IDC_MOTION_SKIP),  435, BlcLowMotionSkip,  0, 100);
	
	CheckDlgButton(hDlg, IDC_BLEND_CHROMA, BlcBlendChroma);
	CheckDlgButton(hDlg, IDC_USE_INTERP_BOB, BlcUseInterpBob);
	

}
// dialog code for new Blended Clipping Deinterlace control panel - TRB 11/00
BOOL APIENTRY BlendedClipProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	int x, y;
	static UINT TMinimumClip;
	static UINT TPixelMotionSense;
	static UINT TRecentMotionSense;
	static UINT TMotionAvgPeriod;
	static UINT TPixelCombSense;
	static UINT TRecentCombSense;
	static UINT TCombAvgPeriod;
	static UINT THighCombSkip;
	static UINT TLowMotionSkip;
	static BOOL TUseInterpBob;
	static BOOL TBlendChroma;
	switch (message)
	{
	case WM_INITDIALOG:

		// Capture the current global values
		TMinimumClip = BlcMinimumClip;
		TPixelMotionSense = BlcPixelMotionSense;
		TRecentMotionSense = BlcRecentMotionSense;
		TMotionAvgPeriod = BlcMotionAvgPeriod;
		TPixelCombSense = BlcPixelCombSense;
		TRecentCombSense = BlcRecentCombSense;
		TCombAvgPeriod = BlcCombAvgPeriod;
		THighCombSkip = BlcHighCombSkip;
		TLowMotionSkip = BlcLowMotionSkip;
		TUseInterpBob = BlcUseInterpBob;
		TBlendChroma = BlcBlendChroma;
		SetBlcDisplayControls(hDlg);


		break;

	case WM_MOUSEMOVE:

		if (wParam == MK_LBUTTON)
			{
				y = HIWORD(lParam);
				x = LOWORD(lParam);
				
				if ((x >= 5) && (x <= 166))
				{
					if ((y >= 35) && (y <= 55))		// Is Minimum clip slider?
					{
						MoveWindow(GetDlgItem(hDlg, IDC_MIN_CLIP), x, 35+5, 10, 12, TRUE);
						BlcMinimumClip = GetHorSliderInt(x, 0, 100);
						SetDlgItemInt(hDlg, IDC_MIN_CLIP_V, BlcMinimumClip, TRUE);
					}
					if ((y >= 85) && (y <= 105))		// Is Pixel Motion slider?
					{
						MoveWindow(GetDlgItem(hDlg, IDC_PIXEL_MOV), x, 85+5, 10, 12, TRUE);
						BlcPixelMotionSense = GetHorSliderInt(x, 0, 100);
						SetDlgItemInt(hDlg, IDC_PIXEL_MOV_V, BlcPixelMotionSense, TRUE);
					}
					if ((y >= 135) && (y <= 155))		// Is Avg Hist slider?
					{
						MoveWindow(GetDlgItem(hDlg, IDC_AVG_MOV), x, 135+5, 10, 12, TRUE);
						BlcRecentMotionSense = GetHorSliderInt(x, 0, 100);
						SetDlgItemInt(hDlg, IDC_AVG_MOV_V, BlcRecentMotionSense, TRUE);
					}
					if ((y >= 185) && (y <= 205))		// Is Hist Mov Avg Peroid clip slider?
					{
						MoveWindow(GetDlgItem(hDlg, IDC_MOV_PERIOD), x, 185+5, 10, 12, TRUE);
						BlcMotionAvgPeriod = GetHorSliderInt(x, 1, 200);
						SetDlgItemInt(hDlg, IDC_MOV_PERIOD_V, BlcMotionAvgPeriod, TRUE);
					}
					if ((y >= 235) && (y <= 255))		// Is Pixel Comb clip slider?
					{
						MoveWindow(GetDlgItem(hDlg, IDC_PIXEL_COMB), x, 235+5, 10, 12, TRUE);
						BlcPixelCombSense = GetHorSliderInt(x, 0, 100);
						SetDlgItemInt(hDlg, IDC_PIXEL_COMB_V, BlcPixelCombSense, TRUE);
					}
					if ((y >= 285) && (y <= 305))		// Is Hist Comb slider?
					{
						MoveWindow(GetDlgItem(hDlg, IDC_AVG_COMB), x, 285+5, 10, 12, TRUE);
						BlcRecentCombSense = GetHorSliderInt(x, 0, 100);
						SetDlgItemInt(hDlg, IDC_AVG_COMB_V, BlcRecentCombSense, TRUE);
					}
					if ((y >= 335) && (y <= 355))		// Is Hist Comb Period slider?
					{
						MoveWindow(GetDlgItem(hDlg, IDC_COMB_PERIOD), x, 335+5, 10, 12, TRUE);
						BlcCombAvgPeriod = GetHorSliderInt(x, 1, 200);
						SetDlgItemInt(hDlg, IDC_COMB_PERIOD_V, BlcCombAvgPeriod, TRUE);
					}
					if ((y >= 385) && (y <= 405))		// Is Suppress High Comb slider?
					{
						MoveWindow(GetDlgItem(hDlg, IDC_COMB_SKIP), x, 385+5, 10, 12, TRUE);
						BlcHighCombSkip = GetHorSliderInt(x, 0, 100);
						SetDlgItemInt(hDlg, IDC_COMB_SKIP_V, BlcHighCombSkip, TRUE);
					}
					if ((y >= 435) && (y <= 455))		// Is Suppress Low motion slider?
					{
						MoveWindow(GetDlgItem(hDlg, IDC_MOTION_SKIP), x, 435+5, 10, 12, TRUE);
						BlcLowMotionSkip = GetHorSliderInt(x, 0, 100);
						SetDlgItemInt(hDlg, IDC_MOTION_SKIP_V, BlcLowMotionSkip, TRUE);
					}
				}
			}
		return (FALSE);
/*	
	case WM_GETTEXT:	
	case WM_PAINT:
		return FALSE;						// say we did not process this

	case WM_NCPAINT:
		return FALSE;						// say we did not process this
*/
	case WM_COMMAND:

		switch LOWORD(wParam)
		{
		case IDOK:							// Is Done, use our new parms
			EndDialog(hDlg, TRUE);
			break;

		case IDCANCEL:						
			BlcMinimumClip = TMinimumClip;
			BlcPixelMotionSense = TPixelMotionSense;
			BlcRecentMotionSense = TRecentMotionSense;
			BlcMotionAvgPeriod = TMotionAvgPeriod;
			BlcPixelCombSense = TPixelCombSense;
			BlcRecentCombSense = TRecentCombSense;
			BlcCombAvgPeriod = TCombAvgPeriod;
			BlcHighCombSkip = THighCombSkip;
			BlcLowMotionSkip = TLowMotionSkip;
			BlcUseInterpBob = TUseInterpBob;
			BlcBlendChroma = TBlendChroma;
			EndDialog(hDlg, TRUE);
			break;

		case IDC_BLEND_CHROMA:				// Blend chroma from mult pixels
			BlcBlendChroma = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_BLEND_CHROMA));
			break;  

		case IDC_USE_INTERP_BOB:				// Blend chroma from mult pixels
			BlcUseInterpBob = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_USE_INTERP_BOB));
			break;  


		case IDC_DEFAULT:
			BlcMinimumClip= 10;
			BlcPixelMotionSense = 55;
			BlcRecentMotionSense = 0;
			BlcMotionAvgPeriod = 40;		// currently 1..200
			BlcPixelCombSense = 50;
			BlcRecentCombSense = 0;
			BlcCombAvgPeriod = 40;			// currently 1.200
			BlcHighCombSkip = 0;			// larger values skip more
			BlcLowMotionSkip = 0;			// larger values skip more
			BlcUseInterpBob = FALSE;
			BlcBlendChroma = FALSE;			// default should maybe be TRUE?
			SetBlcDisplayControls(hDlg);
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
		if (Has_MSP == FALSE)
		{
			ErrorBox("No MSP-Audio-Device found");
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
		if (Has_MSP == FALSE)
		{
			ErrorBox("No MSP-Audio-Device found");
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

	switch (message)
	{
	case WM_INITDIALOG:
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

BOOL APIENTRY ICSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	int i;

	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_EDIT1, IC_BASE_DIR);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			GetDlgItemText(hDlg, IDC_EDIT1, IC_BASE_DIR, sizeof(IC_BASE_DIR));
			i = (int) CreateDirectory(IC_BASE_DIR, NULL);
			EndDialog(hDlg, TRUE);
		}

		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, TRUE);
		}
		break;
	}

	return (FALSE);
}


BOOL APIENTRY VTSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	int i;
	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_EDIT1, VT_BASE_DIR);
		CheckDlgButton(hDlg, IDC_CHECK1, VT_HEADERS);
		CheckDlgButton(hDlg, IDC_CHECK2, VT_STRIPPED);
		CheckDlgButton(hDlg, IDC_CHECK3, VT_NL);
		CheckDlgButton(hDlg, IDC_CHECK4, VT_REVEAL);
		CheckDlgButton(hDlg, IDC_CHECK5, VT_ALWAYS_EXPORT);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			GetDlgItemText(hDlg, IDC_EDIT1, VT_BASE_DIR, sizeof(VT_BASE_DIR));
			i = (int) CreateDirectory(VT_BASE_DIR, NULL);
			VT_HEADERS = IsDlgButtonChecked(hDlg, IDC_CHECK1);
			VT_STRIPPED = IsDlgButtonChecked(hDlg, IDC_CHECK2);
			VT_NL = IsDlgButtonChecked(hDlg, IDC_CHECK3);
			VT_REVEAL = IsDlgButtonChecked(hDlg, IDC_CHECK4);
			VT_ALWAYS_EXPORT = IsDlgButtonChecked(hDlg, IDC_CHECK5);
			EndDialog(hDlg, TRUE);
		}

		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, TRUE);
		}
		break;
	}

	return (FALSE);
}

BOOL APIENTRY VDSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	char FileName[255];
	unsigned char Buffer[10];
	FILE *sFile;
	int rc;

	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_EDIT1, VD_DIR);
		SetDlgItemText(hDlg, IDC_EDIT2, VDat.RawName);

		CheckDlgButton(hDlg, IDC_CHECK1, VD_RAW);

		if (VBI_Flags & VBI_VD)
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), FALSE);	// Videodat läuft also umwandeln nicht möglich !!
		sprintf(FileName, "%s//%s", VD_DIR, VDat.RawName);
		sFile = fopen(FileName, "rb");
		if (sFile == NULL)
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), FALSE);	// Keine RAW-Datei da !!
		else
			fclose(sFile);
		if (VD_RAW == FALSE)
		{
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT2), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), FALSE);
		}
		break;

	case WM_COMMAND:

		if ((HWND) lParam == GetDlgItem(hDlg, IDC_CHECK1))
		{
			VD_RAW = IsDlgButtonChecked(hDlg, IDC_CHECK1);
			if (VD_RAW == FALSE)
			{
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT2), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), FALSE);
			}
			else if (VD_RAW == TRUE)
			{
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT2), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), TRUE);
			}
		}

		if (LOWORD(wParam) == IDC_BUTTON1)
		{
			sprintf(FileName, "%s//%s", VD_DIR, VDat.RawName);
			sFile = fopen(FileName, "rb");
			if (sFile == NULL)
			{
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), FALSE);	// Keine RAW-Datei da !!
				return (TRUE);
			}
			VideoDat_Init();
			while (fread(Buffer, sizeof(unsigned char), 10, sFile) == (size_t) 10)
			{
				Work_VideoDat(Buffer);
			}
			fclose(sFile);
			return (TRUE);
		}

		if (LOWORD(wParam) == IDOK)
		{
			VD_RAW = IsDlgButtonChecked(hDlg, IDC_CHECK1);
			GetDlgItemText(hDlg, IDC_EDIT1, VD_DIR, sizeof(VD_DIR));
			rc = (int) CreateDirectory(VD_DIR, NULL);
			GetDlgItemText(hDlg, IDC_EDIT2, VDat.RawName, sizeof(VDat.RawName));
			EndDialog(hDlg, TRUE);
		}

		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, TRUE);
		}
		break;
	}

	return (FALSE);
}

BOOL APIENTRY ICInfoProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{

	switch (message)
	{
	case WM_INITDIALOG:
		SetTimer(hDlg, 0, 500, NULL);
	case WM_TIMER:
		SetDlgItemText(hDlg, TEXT1, InterCast.fname);
		SetDlgItemInt(hDlg, TEXT2, InterCast.total, FALSE);
		SetDlgItemInt(hDlg, TEXT3, InterCast.packnum, FALSE);
		SetDlgItemInt(hDlg, TEXT4, InterCast.packtot, FALSE);
		SetDlgItemInt(hDlg, TEXT5, InterCast.packlen, FALSE);
		SetDlgItemInt(hDlg, TEXT9, InterCast.BytesSoFar, FALSE);
		SetDlgItemInt(hDlg, TEXT6, InterCast.total, FALSE);
		SetDlgItemText(hDlg, TEXT10, InterCast.Error);
		SetDlgItemInt(hDlg, IDT_VBI_FPS, VBI_FPS, FALSE);

		break;

	case WM_COMMAND:
		if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
		{
			KillTimer(hDlg, 0);
			EndDialog(hDlg, TRUE);
		}
		break;
	}

	return (FALSE);
}


BOOL APIENTRY VDInfoProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{

	switch (message)
	{
	case WM_INITDIALOG:
		SetTimer(hDlg, 0, 500, NULL);
	case WM_TIMER:
		SetDlgItemText(hDlg, TEXT3, SOTInfoRec.GeneralName);
		SetDlgItemText(hDlg, TEXT11, SOTInfoRec.Kommentar);
		SetDlgItemInt(hDlg, TEXT13, SOTInfoRec.date, FALSE);
		SetDlgItemInt(hDlg, TEXT9, SOTInfoRec.fanz, FALSE);
		SetDlgItemText(hDlg, TEXT1, VDat.FileName);
		SetDlgItemInt(hDlg, TEXT2, VDat.FileSize, FALSE);
		SetDlgItemInt(hDlg, TEXT8, VDat.BlockSoFar, FALSE);
		SetDlgItemInt(hDlg, TEXT6, VDat.CRCError, FALSE);
		SetDlgItemInt(hDlg, TEXT4, VDat.LBN, FALSE);
		SetDlgItemInt(hDlg, TEXT5, VDat.Lenght, FALSE);
		SetDlgItemInt(hDlg, TEXT7, VDat.KEY, FALSE);
		SetDlgItemText(hDlg, TEXT10, VDat.Error);
		SetDlgItemInt(hDlg, IDT_VBI_FPS, VBI_FPS, FALSE);

		break;

	case WM_COMMAND:
		if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
		{
			KillTimer(hDlg, 0);
			EndDialog(hDlg, TRUE);
		}
		break;
	}

	return (FALSE);
}

BOOL APIENTRY VDInfoProcRaw(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{

	switch (message)
	{
	case WM_INITDIALOG:
		SetTimer(hDlg, 0, 500, NULL);
	case WM_TIMER:
		SetDlgItemText(hDlg, TEXT1, VD_DIR);
		SetDlgItemText(hDlg, TEXT2, VDat.RawName);
		SetDlgItemInt(hDlg, TEXT3, VDat.BlocksOK, FALSE);
		SetDlgItemInt(hDlg, TEXT5, VDat.BlocksError, FALSE);
		SetDlgItemText(hDlg, TEXT10, VDat.Error);
		SetDlgItemInt(hDlg, IDT_VBI_FPS, VBI_FPS, FALSE);
		break;

	case WM_COMMAND:
		if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
		{
			KillTimer(hDlg, 0);
			EndDialog(hDlg, TRUE);
		}
		break;
	}

	return (FALSE);
}

BOOL APIENTRY VideoTextUnterTitelProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	LPDRAWITEMSTRUCT lpdis;
	LPMEASUREITEMSTRUCT lpmis;
	RECT ptrtoWndPos;
	int currX, currY;
	int row, n;
	int i;
	unsigned short CurrentFg, CurrentBg, RealBkg;
	BOOL bGraph, bHoldGraph, bSepGraph, bBox, bFlash, bDouble, bConceal, bHasDouble;
	BYTE nLastGraph;
	BYTE c, ch;
	unsigned short Black, ForceBlack, ForceTransparent;
	unsigned char buffer[40];

	char VTUBuffer[400];
	unsigned short VTUBufferFg[400];
	char TargetChar;
	int BufferFill;
	int InsertPos;

	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemInt(hDlg, IDC_EDIT1, UTPage, TRUE);
		UTList = CreateWindow("LISTBOX", NULL, WS_BORDER | WS_CHILD | WS_VISIBLE |
							  // LBS_NOINTEGRALHEIGHT |
							  LBS_NOTIFY |
							  // LBS_HASSTRINGS |
							  // LBS_WANTKEYBOARDINPUT |
							  LBS_OWNERDRAWVARIABLE | WS_VSCROLL,
							  //    WS_HSCROLL |
							  //    LBS_SORT ,
							  //       LBS_DISABLENOSCROLL ,
							  0, 0, 0, 0, hDlg, NULL, hInst, NULL);

		SendMessage(UTList, WM_SETFONT, (WPARAM) currFont, TRUE);
		SetTimer(hDlg, 0, 500, NULL);

	case WM_SIZE:
		GetWindowRect(hDlg, &ptrtoWndPos);
		currY = ptrtoWndPos.bottom - ptrtoWndPos.top;
		currX = ptrtoWndPos.right - ptrtoWndPos.left;
		currY = currY - 90;
		currX = currX - 20;

		MoveWindow(UTList, 6, 35, currX, currY, TRUE);

		MoveWindow(GetDlgItem(hDlg, IDOK), currX / 2 - 10, 40 + currY, 40, 20, TRUE);

		break;

	case WM_DRAWITEM:
		/* Get pointer to the DRAWITEMSTRUCT */
		lpdis = (LPDRAWITEMSTRUCT) lParam;

		if (lpdis->itemID == -1)
		{
			/* We have a request to draw an item in the list box, yet there
			 * are no list box items. This is sent when the user TABS into
			 * an empty list box or an empty list box gets the focus. We
			 * have to indicate (somehow) that this owner-draw list box has
			 * the focus. We do it in response to this message. Note that
			 * lpdis->itemData field would be invalid in this instance so
			 * we can't allow it to fall into our standard routines.
			 */
			HandleFocusState(UTList, lpdis);
		}
		else
		{
			switch (lpdis->itemAction)
			{
			case ODA_DRAWENTIRE:
				DrawEntireItem(UTList, lpdis, 0);
				break;

			case ODA_SELECT:
				HandleFocusState(UTList, lpdis);
				break;

			case ODA_FOCUS:
				HandleFocusState(UTList, lpdis);
				break;
			}
		}

		/* Return TRUE meaning that we processed this message. */
		return (TRUE);
		break;

	case WM_MEASUREITEM:
		lpmis = (LPMEASUREITEMSTRUCT) lParam;

		/* All the items are the same height since the list box style is
		 * LBS_OWNERDRAWFIXED
		 */
		lpmis->itemHeight = 14;
		return (TRUE);

	case WM_TIMER:

		if (VTFrame[UTPage].SubCount == 0)
			return (TRUE);
		BufferFill = 0;
		InsertPos = 0;
		for (row = 20; row < 25; row++)
		{
			if (VTFrame[UTPage].SubPage[0].LineUpdate[row] == 1)
			{

				VTFrame[UTPage].SubPage[0].LineUpdate[row] = 0;
				bGraph = bHoldGraph = bSepGraph = bBox = bFlash = bDouble = bConceal = bHasDouble = FALSE;
				nLastGraph = 32;
				Black = VTColourTable[0];	//
				ForceTransparent = VTColourTable[8];
				ForceBlack = VTColourTable[0];

				for (n = 0; n < 40; n++)
				{
					buffer[n] = VTFrame[UTPage].SubPage[0].Frame[row][n] & 0x7f;
				}

				CurrentFg = VTColourTable[7];
				for (n = 0; n < 40; n++)
				{
					c = buffer[n];
					ch = c;
					if (c < 32)
					{
						if (c < 8)
						{
							CurrentFg = VTColourTable[c];
							bGraph = FALSE;
						}
						if (c >= 0x10 && c <= 0x17)
						{
							bGraph = TRUE;
							CurrentFg = VTColourTable[c - 0x10];
						}
						if (c == 0x1d)
						{
							CurrentBg = (bBox || Black != ForceTransparent) ? CurrentFg : ForceTransparent;
							RealBkg = CurrentFg;
						}
						if (c == 0x1c)
						{
							CurrentBg = bBox ? ForceBlack : Black;
							RealBkg = ForceBlack;
						}
						if (c == 0x08)
							bFlash = TRUE;
						if (c == 0x09)
							bFlash = FALSE;
						if (c == 0x0c)
							bDouble = FALSE;
						if (c == 0x0d)
							bDouble = TRUE;
						if (c == 0x18)
							bConceal = TRUE;
						if (c == 0x19)
							bSepGraph = FALSE;
						if (c == 0x1a)
							bSepGraph = TRUE;
						if (c == 0x1e)
							bHoldGraph = TRUE;
						if (c == 0x1f)
							bHoldGraph = FALSE;
						ch = bHoldGraph ? nLastGraph : 32;
					}
					nLastGraph = 32;
					if (bGraph && (ch & 0x20))
					{
						nLastGraph = ch;
						ch = (ch & 0x1f) | ((ch & 0x40) >> 1);
						ch += 96;
						if (bSepGraph)
							ch += 64;
					}
					else
						ch -= 32;

					TargetChar = ' ';
					if (ch < 96)
					{
						TargetChar = VTtoAscii[ch];
						if ((BufferFill == 0) && (TargetChar != ' '))
						{
							VTUBuffer[BufferFill] = TargetChar;
							VTUBufferFg[BufferFill] = CurrentFg;
							BufferFill++;
						}
						else if (BufferFill > 0)
						{
							if ((TargetChar == ' ') && (VTUBuffer[BufferFill - 1] != ' '))
							{
								VTUBuffer[BufferFill] = TargetChar;
								VTUBufferFg[BufferFill] = CurrentFg;
								BufferFill++;
							}
							else if (TargetChar != ' ')
							{
								VTUBuffer[BufferFill] = TargetChar;
								VTUBufferFg[BufferFill] = CurrentFg;
								BufferFill++;
							}
						}
					}
				}

				if (BufferFill > 0)
				{
					if (VTUBuffer[BufferFill - 1] != ' ')
					{
						VTUBuffer[BufferFill] = 0x20;
						VTUBufferFg[BufferFill] = CurrentFg;
						BufferFill++;
					}
				}
			}
		}

		i = 0;
		n = 0;

		while (n < BufferFill)
		{

			if (i == 0)
				UT[UTLoop].Fg = VTUBufferFg[n];

			UT[UTLoop].Zeile[i] = VTUBuffer[n];
			i++;
			n++;
			UT[UTLoop].Zeile[i] = 0x00;

			if (((n == BufferFill) || (VTUBufferFg[n] != UT[UTLoop].Fg)) && (strlen(UT[UTLoop].Zeile) > 0))
			{
				SendMessage(UTList, LB_INSERTSTRING, InsertPos++, (LPARAM) UTLoop);
				SendMessage(UTList, LB_DELETESTRING, 128, (LPARAM) 0);
				UTLoop++;
				i = 0;
				if (UTLoop > 127)
					UTLoop = 0;
			}
		}

		// Zeile Voll //
		break;

	case WM_COMMAND:

		if ((HWND) lParam == GetDlgItem(hDlg, IDC_EDIT1))
		{
			i = GetDlgItemInt(hDlg, IDC_EDIT1, NULL, FALSE);
			if ((i >= 100) && (i <= 899))
				UTPage = i - 100;
			break;
		}

		if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
		{
			KillTimer(hDlg, 0);
			EndDialog(hDlg, TRUE);
		}
		break;
	}

	return (FALSE);
}

VOID APIENTRY HandleFocusState(HWND hDlg, LPDRAWITEMSTRUCT lpdis)
{
	int Typ;

	if ((lpdis->itemState & ODS_FOCUS) || (lpdis->itemState & ODS_SELECTED))
	{
		Typ = 1;
	}
	else
	{
		Typ = 0;
	}

	DrawEntireItem(hDlg, lpdis, Typ);
}

VOID APIENTRY DrawEntireItem(HWND hDlg, LPDRAWITEMSTRUCT lpdis, INT Typ)
{
	HFONT OldFont;
	BYTE R, G, B;

	OldFont = SelectObject(lpdis->hDC, currFont);
	R = (BYTE) ((UT[lpdis->itemData].Fg & 0x7C00) >> 10) << 3;
	G = (BYTE) ((UT[lpdis->itemData].Fg & 0x3E0) >> 5) << 3;
	B = (BYTE) ((UT[lpdis->itemData].Fg & 0x1F)) << 3;
	if (32767 == UT[lpdis->itemData].Fg)
	{
		R = 0;
		G = 0;
		B = 0;
	}

	SetTextColor(lpdis->hDC, RGB(R, G, B));
	SetBkColor(lpdis->hDC, RGB(255, 255, 255));
	TextOut(lpdis->hDC, 10, lpdis->rcItem.top + 1, UT[lpdis->itemData].Zeile, strlen(UT[lpdis->itemData].Zeile));
	SelectObject(lpdis->hDC, OldFont);
	return;
}

BOOL APIENTRY CardSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	static int SaveTuner;
	static int SaveVideoSource;

	switch (message)
	{
	case WM_INITDIALOG:
		SaveTuner = CardType;
		SaveVideoSource = AudioSource;
		PostMessage(hDlg, WM_USER, 1, 0);
		break;

	case WM_USER:

		if (wParam == 1)
		{
			SetDlgItemInt(hDlg, IDC_EDIT1, ManuellAudio[0], FALSE);
			SetDlgItemInt(hDlg, IDC_EDIT2, ManuellAudio[1], FALSE);
			SetDlgItemInt(hDlg, IDC_EDIT3, ManuellAudio[2], FALSE);
			SetDlgItemInt(hDlg, IDC_EDIT4, ManuellAudio[3], FALSE);
			SetDlgItemInt(hDlg, IDC_EDIT5, ManuellAudio[4], FALSE);
			SetDlgItemInt(hDlg, IDC_EDIT6, ManuellAudio[5], FALSE);
			SetDlgItemInt(hDlg, IDC_EDIT7, ManuellAudio[6], FALSE);
			SetDlgItemInt(hDlg, IDC_EDIT8, ManuellAudio[7], FALSE);

		}

		break;
	case WM_COMMAND:

		if (LOWORD(wParam) == IDSET)
		{
			CardType = 6;
			AudioSource = 0;
			if (IsDlgButtonChecked(hDlg, IDC_RADIO1))
				AudioSource = 0;
			if (IsDlgButtonChecked(hDlg, IDC_RADIO2))
				AudioSource = 1;
			if (IsDlgButtonChecked(hDlg, IDC_RADIO3))
				AudioSource = 2;
			if (IsDlgButtonChecked(hDlg, IDC_RADIO4))
				AudioSource = 3;
			if (IsDlgButtonChecked(hDlg, IDC_RADIO5))
				AudioSource = 4;
			if (IsDlgButtonChecked(hDlg, IDC_RADIO6))
				AudioSource = 5;
			ManuellAudio[0] = GetDlgItemInt(hDlg, IDC_EDIT1, NULL, FALSE);
			ManuellAudio[1] = GetDlgItemInt(hDlg, IDC_EDIT2, NULL, FALSE);
			ManuellAudio[2] = GetDlgItemInt(hDlg, IDC_EDIT3, NULL, FALSE);
			ManuellAudio[3] = GetDlgItemInt(hDlg, IDC_EDIT4, NULL, FALSE);
			ManuellAudio[4] = GetDlgItemInt(hDlg, IDC_EDIT5, NULL, FALSE);
			ManuellAudio[5] = GetDlgItemInt(hDlg, IDC_EDIT6, NULL, FALSE);
			ManuellAudio[6] = GetDlgItemInt(hDlg, IDC_EDIT7, NULL, FALSE);
			ManuellAudio[7] = GetDlgItemInt(hDlg, IDC_EDIT8, NULL, FALSE);
			Audio_SetSource(AudioSource);
		}

		if (LOWORD(wParam) == IDOK)
		{
			ManuellAudio[0] = GetDlgItemInt(hDlg, IDC_EDIT1, NULL, FALSE);
			ManuellAudio[1] = GetDlgItemInt(hDlg, IDC_EDIT2, NULL, FALSE);
			ManuellAudio[2] = GetDlgItemInt(hDlg, IDC_EDIT3, NULL, FALSE);
			ManuellAudio[3] = GetDlgItemInt(hDlg, IDC_EDIT4, NULL, FALSE);
			ManuellAudio[4] = GetDlgItemInt(hDlg, IDC_EDIT5, NULL, FALSE);
			ManuellAudio[5] = GetDlgItemInt(hDlg, IDC_EDIT6, NULL, FALSE);
			ManuellAudio[6] = GetDlgItemInt(hDlg, IDC_EDIT7, NULL, FALSE);
			ManuellAudio[7] = GetDlgItemInt(hDlg, IDC_EDIT8, NULL, FALSE);

			SetMenuAnalog();
			EndDialog(hDlg, TRUE);
		}

		if (LOWORD(wParam) == IDCANCEL)
		{
			CardType = SaveTuner;
			AudioSource = SaveVideoSource;
			Audio_SetSource(AudioSource);
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
		SetDlgItemText(hDlg, IDC_TEXT6, BTVendorID);
		SetDlgItemText(hDlg, IDC_TEXT7, BTDeviceID);

		SetDlgItemText(hDlg, IDC_TEXT1, BTTyp);
		SetDlgItemText(hDlg, IDC_TEXT13, TunerStatus);
		SetDlgItemText(hDlg, IDC_TEXT14, MSPStatus);
		SetDlgItemText(hDlg, IDC_TEXT16, MSPVersion);

		SetDlgItemText(hDlg, IDC_TEXT18, "YUV2");

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
