/////////////////////////////////////////////////////////////////////////////
// FLT_TNoise.c
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 Steven Grimm.  All rights reserved.
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

#include "windows.h"
#include "dTV_Filter.h"

long TemporalLuminanceThreshold = 6;	// Pixel luminance differences below this are considered noise.
long TemporalChromaThreshold = 7;		// Pixel chroma differences below this are considered noise.


/////////////////////////////////////////////////////////////////////////////
// Temporal noise reduction filter.  This noise filter smooths out slight
// variations in pixels between adjacent frames.  If the difference between
// a pixel and its predecessor from two fields back is less than a threshold,
// replace the pixel on the current field with a weighted average of its
// current and previous values.
/////////////////////////////////////////////////////////////////////////////

BOOL FilterTemporalNoise(DEINTERLACE_INFO *info)
{
	short *NewPixel;
	short *OldPixel;
	int y;
	int Cycles;
	__int64 qwNoiseThreshold;

	// Need to have the current and next-to-previous fields to do the filtering.
	if ((info->IsOdd && (info->OddLines[0] == NULL || info->OddLines[1] == NULL)) ||
		(! info->IsOdd && (info->EvenLines[0] == NULL || info->EvenLines[1] == NULL)))
	{
		return FALSE;
	}

	qwNoiseThreshold = TemporalLuminanceThreshold | (TemporalChromaThreshold << 8);
	qwNoiseThreshold |= (qwNoiseThreshold << 48) | (qwNoiseThreshold << 32) | (qwNoiseThreshold << 16);
	Cycles = info->LineLength / 8;

	for (y = 0; y < info->FieldHeight; y++)
	{
		if (info->IsOdd)
		{
			NewPixel = info->OddLines[0][y];
			OldPixel = info->OddLines[1][y];
		}
		else
		{
			NewPixel = info->EvenLines[0][y];
			OldPixel = info->EvenLines[1][y];
		}

		if (info->CpuFeatureFlags & FEATURE_SSE) {
#ifdef USE_SSE
#define IS_SSE
#include "FLT_TNoise.asm"
#undef IS_SSE
#endif
		}
		else if (info->CpuFeatureFlags & FEATURE_3DNOW) {
#ifdef USE_3DNOW
#define IS_3DNOW
#include "FLT_TNoise.asm"
#undef IS_3DNOW
#endif
		}
		else {
#define IS_MMX
#include "FLT_TNoise.asm"
#undef IS_MMX
		}
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING FLT_TNoiseSettings[FLT_TNOISE_SETTING_LASTONE] =
{
	{
		"Temporal Luminance Threshold", SLIDER, 0, &TemporalLuminanceThreshold,
		6, 0, 255, 1, 1,
		NULL,
		"NoiseFilter", "TemporalLuminanceThreshold", NULL,
	},
	{
		"Temporal Chroma Threshold", SLIDER, 0, &TemporalChromaThreshold,
		7, 0, 255, 1, 1,
		NULL,
		"NoiseFilter", "TemporalChromaThreshold", NULL,
	},
};

FILTER_METHOD TemporalNoiseMethod =
{
	"Temporal Noise Filter",
	FALSE,
	TRUE,
	FilterTemporalNoise, 
	1,
	FALSE,
	NULL,
};


__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo(long CpuFeatureFlags)
{
	return &TemporalNoiseMethod;
}

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}

