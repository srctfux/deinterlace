/////////////////////////////////////////////////////////////////////////////
// DI_VideoWeave.c
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
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 30 Dec 2000   Mark Rejhon           Split into separate module
//
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "dTV_Deinterlace.h"

long TemporalTolerance = 300;
long SpatialTolerance = 600;
long SimilarityThreshold = 25;

#define IS_SSE 1
#include "DI_VideoWeave.asm"
#undef IS_SSE

#define IS_3DNOW 1
#include "DI_VideoWeave.asm"
#undef IS_3DNOW

#define IS_MMX 1
#include "DI_VideoWeave.asm"
#undef IS_MMX

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING DI_VideoWeaveSettings[DI_VIDEOWEAVE_SETTING_LASTONE] =
{
	{
		"Temporal Tolerance", SLIDER, 0, &TemporalTolerance,
		300, 0, 5000, 10, 1,
		NULL,
		"Deinterlace", "TemporalTolerance", NULL,
	},
	{
		"Spatial Tolerance", SLIDER, 0, &SpatialTolerance,
		600, 0, 5000, 10, 1,
		NULL,
		"Deinterlace", "SpatialTolerance", NULL,
	},
	{
		"Similarity Threshold", SLIDER, 0, &SimilarityThreshold,
		25, 0, 255, 1, 1,
		NULL,
		"Deinterlace", "SimilarityThreshold", NULL,
	},
};

DEINTERLACE_METHOD VideoWeaveMethod =
{
	"Video Deinterlace (Weave)", 
	"Weave", 
	FALSE, 
	FALSE, 
	DeinterlaceFieldWeave_MMX, 
	50, 
	60,
	DI_VIDEOWEAVE_SETTING_LASTONE,
	DI_VideoWeaveSettings,
	INDEX_VIDEO_WEAVE,
	NULL,
	NULL,
	NULL,
	2,
	0,
	0,
	WM_DI_VIDEOWEAVE_GETVALUE - WM_USER,
	NULL,
	0,
	FALSE,
	FALSE,
};


__declspec(dllexport) DEINTERLACE_METHOD* GetDeinterlacePluginInfo(long CpuFeatureFlags)
{
    if (CpuFeatureFlags & FEATURE_SSE)
    {
        VideoWeaveMethod.pfnAlgorithm = DeinterlaceFieldWeave_SSE;
    }
    else if (CpuFeatureFlags & FEATURE_3DNOW)
    {
        VideoWeaveMethod.pfnAlgorithm = DeinterlaceFieldWeave_3DNOW;
    }
    else
    {
        VideoWeaveMethod.pfnAlgorithm = DeinterlaceFieldWeave_MMX;
    }
	return &VideoWeaveMethod;
}

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}
