///////////////////////////////////////////////////////////////////////////////
// DI_Greedy2Frame.c
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock, Tom Barry, Steve Grimm  All rights reserved.
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
// 08 Feb 2000   John Adcock           New Method
//
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "dTV_Deinterlace.h"

long GreedyTwoFrameThreshold = 8;

///////////////////////////////////////////////////////////////////////////////
// Field 1 | Field 2 | Field 3 | Field 4 |
//   T0    |         |    T1   |         | 
//         |   M0    |         |    M1   | 
//   B0    |         |    B1   |         | 
//


// debugging feature
// output the value of mm4 at this point which is pink where we will weave
// and green were we are going to bob
// uncomment next line to see this
//#define CHECK_BOBWEAVE

BOOL DeinterlaceGreedy2Frame(DEINTERLACE_INFO *info)
{
	int Line;
	short* M1;
	short* M0;
	short* T0;
	short* T1;
	short* B1;
	short* B0;
	DWORD OldSI;
	DWORD OldSP;
	BYTE* Dest;
	DWORD LineLength = info->LineLength;

	const __int64 YMask    = 0x00ff00ff00ff00ff;

	__int64 qwGreedyTwoFrameThreshold;
	const __int64 Mask = 0x7f7f7f7f7f7f7f7f;
	const __int64 DwordOne = 0x0000000100000001;	
	const __int64 DwordTwo = 0x0000000200000002;	

	if (info->OddLines[0] == NULL || info->OddLines[1] == NULL ||
		info->EvenLines[0] == NULL || info->EvenLines[1] == NULL)
	{
		return FALSE;
	}

	qwGreedyTwoFrameThreshold = GreedyTwoFrameThreshold;
	qwGreedyTwoFrameThreshold += (qwGreedyTwoFrameThreshold << 56) +
								(qwGreedyTwoFrameThreshold << 48) +
								(qwGreedyTwoFrameThreshold << 40) + 
								(qwGreedyTwoFrameThreshold << 32) + 
								(qwGreedyTwoFrameThreshold << 24) + 
								(qwGreedyTwoFrameThreshold << 16) + 
								(qwGreedyTwoFrameThreshold << 8);

	// copy first even line no matter what, and the first odd line if we're
	// processing an odd field.
	info->pMemcpy(info->Overlay, info->EvenLines[0][0], info->LineLength);
	if (!info->IsOdd)
	{
		info->pMemcpy(info->Overlay + info->OverlayPitch, info->OddLines[0][0], info->LineLength);
	}

	for (Line = 0; Line < info->FieldHeight - 1; ++Line)
	{
		if (info->IsOdd)
		{
			M1 = info->OddLines[0][Line];
			T1 = info->EvenLines[0][Line];
			B1 = info->EvenLines[0][Line + 1];
			M0 = info->OddLines[1][Line];
			T0 = info->EvenLines[1][Line];
			B0 = info->EvenLines[1][Line + 1];
			Dest = info->Overlay + (Line * 2 + 2) * info->OverlayPitch;
		}
		else
		{
			M1 = info->EvenLines[0][Line + 1];
			T1 = info->OddLines[0][Line];
			B1 = info->OddLines[0][Line + 1];
			M0 = info->EvenLines[1][Line + 1];
			T0 = info->OddLines[1][Line];
			B0 = info->OddLines[1][Line + 1];
			Dest = info->Overlay + (Line * 2 + 3) * info->OverlayPitch;
		}

		// Always use the most recent data verbatim.  By definition it's correct (it'd
		// be shown on an interlaced display) and our job is to fill in the spaces
		// between the new lines.
		info->pMemcpy(Dest, B1, info->LineLength);
		Dest -= info->OverlayPitch;

		if (info->CpuFeatureFlags & FEATURE_SSE) {
#ifdef USE_SSE
#define IS_SSE 1
#include "DI_Greedy2Frame.asm"
#undef IS_SSE
#endif
		}
		else if (info->CpuFeatureFlags & FEATURE_3DNOW) {
#ifdef USE_3DNOW
#define IS_3DNOW 1
#include "DI_Greedy2Frame.asm"
#undef IS_3DNOW
#endif
		}
		else {
#define IS_MMX 1
#include "DI_Greedy2Frame.asm"
#undef IS_MMX
		}
	}

	// Copy last odd line if we're processing an even field.
	if (info->IsOdd)
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
SETTING DI_Greedy2FrameSettings[DI_GREEDY2FRAME_SETTING_LASTONE] =
{
	{
		"Greedy 2 Frame Threshold", SLIDER, 0, &GreedyTwoFrameThreshold,
		4, 0, 128, 1, 1,
		NULL,
		"Deinterlace", "GreedyTwoFrameThreshold", NULL,
	},
};


DEINTERLACE_METHOD Greedy2FrameMethod =
{
	"Greedy 2 Frame", 
	NULL, 
	FALSE, 
	FALSE, 
	DeinterlaceGreedy2Frame, 
	50, 
	60,
	DI_GREEDY2FRAME_SETTING_LASTONE,
	DI_Greedy2FrameSettings,
	18,
	NULL,
	NULL,
	INDEX_VIDEO_GREEDY2FRAME,
	0,
	0,
	WM_DI_GREEDY2FRAME_GETVALUE - WM_USER,
};


__declspec(dllexport) DEINTERLACE_METHOD* GetDeinterlacePluginInfo(long CpuFeatureFlags)
{
	return &Greedy2FrameMethod;
}

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}
