///////////////////////////////////////////////////////////////////////////////
// DI_TwoFrame.c
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
///////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 30 Dec 2000   Mark Rejhon           Split into separate module
//
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "dTV_Deinterlace.h"

long TwoFrameTemporalTolerance = 300;
long TwoFrameSpatialTolerance = 600;

static BOOL TwoFrameSSE(DEINTERLACE_INFO *info);

///////////////////////////////////////////////////////////////////////////////
// Deinterlace the latest field, attempting to weave wherever it won't cause
// visible artifacts.
//
// The data from the most recently captured field is always copied to the overlay
// verbatim.  For the data from the previous field, the following algorithm is
// applied to each pixel.
//
// We use the following notation for the top, middle, and bottom pixels
// of concern:
//
// Field 1 | Field 2 | Field 3 | Field 4 |
//         |   T0    |         |   T1    | scanline we copied in last iteration
//   M0    |         |    M1   |         | intermediate scanline from alternate field
//         |   B0    |         |   B1    | scanline we just copied
//
// We will weave M1 into the image if any of the following is true:
//   - M1 is similar to either B1 or T1.  This indicates that no weave
//     artifacts would be visible.  The SpatialTolerance setting controls
//     how far apart the luminances can be before pixels are considered
//     non-similar.
//   - T1 and B1 and M1 are old.  In that case any weave artifact that
//     appears isn't due to fast motion, since it was there in the previous
//     frame too.  By "old" I mean similar to their counterparts in the
//     previous frame; TemporalTolerance controls the maximum squared
//     luminance difference above which a pixel is considered "new".
//
// Pixels are processed 4 at a time using MMX instructions.
//
// SQUARING NOTE:
// We square luminance differences to amplify the effects of large
// differences and to avoid dealing with negative differences.  Unfortunately,
// we can't compare the square of difference directly against a threshold,
// thanks to the lack of an MMX unsigned compare instruction.  The
// problem is that if we had two pixels with luminance 0 and 255,
// the difference squared would be 65025, which is a negative
// 16-bit signed value and would thus compare less than a threshold.
// We get around this by dividing all the luminance values by two before
// squaring them; this results in an effective maximum luminance
// difference of 127, whose square (16129) is safely comparable.


BOOL DeinterlaceFieldTwoFrame(DEINTERLACE_INFO *info)
{
	int Line;
	short* YVal0;
	short* YVal1;
	short* YVal2;
	short* OVal0;
	short* OVal1;
	short* OVal2;
	DWORD OldSI;
	DWORD OldSP;
	BYTE* Dest;
	DWORD LineLength = info->LineLength;

	const __int64 YMask    = 0x00ff00ff00ff00ff;

	__int64 qwSpatialTolerance;
	__int64 qwTemporalTolerance;
	__int64 qwAllOnes = 0xffffffffffffffff;
	__int64 qwBobbedPixels;
	const __int64 Mask = 0x7f7f7f7f7f7f7f7f;

	if (info->OddLines[0] == NULL || info->OddLines[1] == NULL ||
		info->EvenLines[0] == NULL || info->EvenLines[1] == NULL)
	{
		return FALSE;
	}

	qwSpatialTolerance = TwoFrameSpatialTolerance / 4;		// divide by 4 because of squaring behavior, see below
	qwSpatialTolerance += (qwSpatialTolerance << 48) + (qwSpatialTolerance << 32) + (qwSpatialTolerance << 16);
	qwTemporalTolerance = TwoFrameTemporalTolerance / 4;
	qwTemporalTolerance += (qwTemporalTolerance << 48) + (qwTemporalTolerance << 32) + (qwTemporalTolerance << 16);

	// copy first even line no matter what, and the first odd line if we're
	// processing an odd field.
	info->pMemcpy(info->Overlay, info->EvenLines[0][0], info->LineLength);
	if (info->IsOdd)
	{
		info->pMemcpy(info->Overlay + info->OverlayPitch, info->OddLines[0][0], info->LineLength);
	}

	for (Line = 0; Line < info->FieldHeight - 1; ++Line)
	{
		if (info->IsOdd)
		{
			YVal0 = info->OddLines[0][Line];
			YVal1 = info->EvenLines[0][Line + 1];
			YVal2 = info->OddLines[0][Line + 1];
			OVal0 = info->OddLines[1][Line];
			OVal1 = info->EvenLines[1][Line + 1];
			OVal2 = info->OddLines[1][Line + 1];
			Dest = info->Overlay + (Line * 2 + 3) * info->OverlayPitch;
		}
		else
		{
			YVal0 = info->EvenLines[0][Line];
			YVal1 = info->OddLines[0][Line];
			YVal2 = info->EvenLines[0][Line + 1];
			OVal0 = info->EvenLines[1][Line];
			OVal1 = info->OddLines[1][Line];
			OVal2 = info->EvenLines[1][Line + 1];
			Dest = info->Overlay + (Line * 2 + 2) * info->OverlayPitch;
		}

		// Always use the most recent data verbatim.  By definition it's correct (it'd
		// be shown on an interlaced display) and our job is to fill in the spaces
		// between the new lines.
		info->pMemcpy(Dest, YVal2, info->LineLength);
		Dest -= info->OverlayPitch;

		if (info->CpuFeatureFlags & FEATURE_SSE) {
#ifdef USE_SSE
#define IS_SSE 1
#include "DI_TwoFrame.asm"
#undef IS_SSE
#endif
		}
		else if (info->CpuFeatureFlags & FEATURE_3DNOW) {
#ifdef USE_3DNOW
#define IS_3DNOW 1
#include "DI_TwoFrame.asm"
#undef IS_3DNOW
#endif
		}
		else {
#define IS_MMX 1
#include "DI_TwoFrame.asm"
#undef IS_MMX
		}
	}

	// Copy last odd line if we're processing an even field.
	if (!info->IsOdd)
	{
		info->pMemcpy(info->Overlay + (info->FrameHeight - 1) * info->OverlayPitch,
				  info->OddLines[info->FieldHeight - 1],
				  info->LineLength);
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING DI_TwoFrameSettings[DI_TWOFRAME_SETTING_LASTONE] =
{
	{
		"2 Frame Spatial Tolerance", SLIDER, 0, &TwoFrameSpatialTolerance,
		600, 0, 5000, 10, 1,
		NULL,
		"Deinterlace", "TwoFrameSpatialTolerance", NULL,
	},
	{
		"2 Frame Temporal Tolerance", SLIDER, 0, &TwoFrameTemporalTolerance,
		300, 0, 5000, 10, 1,
		NULL,
		"Deinterlace", "TwoFrameTemporalTolerance", NULL,
	},
};

DEINTERLACE_METHOD TwoFrameMethod =
{
	"Video Deinterlace (2-Frame)", 
	"2-Frame", 
	FALSE, 
	FALSE, 
	DeinterlaceFieldTwoFrame, 
	50, 
	60,
	DI_TWOFRAME_SETTING_LASTONE,
	DI_TwoFrameSettings,
	INDEX_VIDEO_2FRAME,
	NULL,
	NULL,
	NULL,
	4,
	0,
	0,
	WM_DI_TWOFRAME_GETVALUE - WM_USER,
	NULL,
	0,
	FALSE,
	FALSE,
};


__declspec(dllexport) DEINTERLACE_METHOD* GetDeinterlacePluginInfo(long CpuFeatureFlags)
{
	return &TwoFrameMethod;
}

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}
