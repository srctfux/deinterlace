/////////////////////////////////////////////////////////////////////////////
// $Id: FLT_TNoise.c,v 1.10 2002-08-07 00:44:12 lindsey Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 Steven Grimm.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//  This file is subject to the terms of the GNU General Public License as
//  published by the Free Software Foundation.  A copy of this license is
//  included with this software distribution in the file COPYING.  If you
//  do not have a copy, you may obtain a copy by writing to the Free
//  Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.9  2002/06/18 19:46:10  adcockj
// Changed appliaction Messages to use WM_APP instead of WM_USER
//
// Revision 1.8  2002/06/13 12:10:26  adcockj
// Move to new Setings dialog for filers, video deint and advanced settings
//
// Revision 1.7  2002/03/08 04:16:01  lindsey
// Changed MMXEXT (Athlon) processors to use the SSE version
//
// Revision 1.6  2002/02/15 15:27:48  robmuller
// pcmpgtw -> pcmpgtb. Solved signed compare problem with pcmpgt..
// Replaced averaging code with the AVERAGE macro. Added prefetching.
//
// Revision 1.5  2002/02/01 19:51:30  robmuller
// Changed the replacement value. The new value favors the new pixel value.
// This reduces the speckles, posterization and noise reduction.
// Changed the defaults of the thresholds to reflect the new situation.
// Added Lock Thresholds option.
//
// Revision 1.4  2001/11/26 15:27:19  adcockj
// Changed filter structure
//
// Revision 1.3  2001/07/13 16:13:33  adcockj
// Added CVS tags and removed tabs
//
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DS_Filter.h"
#include "..\help\helpids.h"

long        DispatchTemporalNoise( TDeinterlaceInfo *pInfo );

long TemporalLuminanceThreshold = 6;    // Pixel luminance differences below this are considered noise.
long TemporalChromaThreshold = 7;       // Pixel chroma differences below this are considered noise.
BOOL LockThresholdsTogether = FALSE;
FILTER_METHOD TemporalNoiseMethod;


/////////////////////////////////////////////////////////////////////////////
// Temporal noise reduction filter.  This noise filter smooths out slight
// variations in pixels between adjacent frames.  If the difference between
// a pixel and its predecessor from two fields back is less than a threshold,
// replace the pixel on the current field with a weighted average of its
// current and previous values.
/////////////////////////////////////////////////////////////////////////////

#define USE_PREFETCH

#define IS_SSE 1
#include "FLT_TNoise.asm"
#undef IS_SSE

#define IS_3DNOW 1
#include "FLT_TNoise.asm"
#undef IS_3DNOW

#undef USE_PREFETCH

#define IS_SSE 1
#include "FLT_TNoise.asm"
#undef IS_SSE

#define IS_3DNOW 1
#include "FLT_TNoise.asm"
#undef IS_3DNOW

#define IS_MMX 1
#include "FLT_TNoise.asm"
#undef IS_MMX


/////////////////////////////////////////////////////////////////////////////
// Begin plugin globals
/////////////////////////////////////////////////////////////////////////////

long        gUsePrefetching = TRUE;

// Stored information about the machine, used when choosing which code version to run

static long gCpuFeatureFlags = 0;


////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////

BOOL LuminanceThreshold_OnChange(long NewValue)
{
    TemporalLuminanceThreshold = NewValue;
    if(LockThresholdsTogether)
    {
        TemporalChromaThreshold = NewValue;
    }
    return TRUE;   
}

BOOL ChromaThreshold_OnChange(long NewValue)
{
    TemporalChromaThreshold = NewValue;
    if(LockThresholdsTogether)
    {
        TemporalLuminanceThreshold = NewValue;
    }
    return TRUE;   
}

SETTING FLT_TNoiseSettings[FLT_TNOISE_SETTING_LASTONE] =
{
    {
        "Temporal Luminance Threshold", SLIDER, 0, &TemporalLuminanceThreshold,
        6, 0, 255, 1, 1,
        NULL,
        "NoiseFilter", "TemporalLuminanceThreshold", LuminanceThreshold_OnChange,
    },
    {
        "Fast Memory Access", ONOFF, 0, &gUsePrefetching,
        TRUE, 0, 1, 1, 1,
        NULL,
        "NoiseFilter", "UsePrefetching", NULL,
    },
    {
        "Temporal Chroma Threshold", SLIDER, 0, &TemporalChromaThreshold,
        7, 0, 255, 1, 1,
        NULL,
        "NoiseFilter", "TemporalChromaThreshold", ChromaThreshold_OnChange,
    },
    {
        "Lock Thresholds", ONOFF, 0, &LockThresholdsTogether,
        FALSE, 0, 1, 1, 1,
        NULL,
        "NoiseFilter", "LockThresholdsTogether", NULL,
    },
    {
        "Noise Filter", ONOFF, 0, &TemporalNoiseMethod.bActive,
        FALSE, 0, 1, 1, 1,
        NULL,
        "NoiseFilter", "UseTemporalNoiseFilter", NULL,
    },
};

FILTER_METHOD TemporalNoiseMethod =
{
    sizeof(FILTER_METHOD),
    FILTER_CURRENT_VERSION,
    DEINTERLACE_INFO_CURRENT_VERSION,
    "Temporal Noise Filter",
    "Noise Reduction (Temporal)\tN",
    FALSE,
    TRUE,
    DispatchTemporalNoise, 
    // IDM_NOISE_FILTER so that accelerator works
    768,
    FALSE,
    NULL,
    NULL,
    NULL,
    FLT_TNOISE_SETTING_LASTONE,
    FLT_TNoiseSettings,
    WM_FLT_TNOISE_GETVALUE - WM_APP,
    TRUE,
    3,
    IDH_TEMPORAL_NOISE,
};

long DispatchTemporalNoise( TDeinterlaceInfo *pInfo )
{

    if( gUsePrefetching == TRUE )
    {
        if( (gCpuFeatureFlags & FEATURE_SSE) || (gCpuFeatureFlags & FEATURE_MMXEXT) )
        {
            FilterTemporalNoise_SSE_PREFETCH( pInfo );
        }
        else if( gCpuFeatureFlags & FEATURE_3DNOW )
        {
            FilterTemporalNoise_3DNOW_PREFETCH( pInfo );
        }
        else
        {
            FilterTemporalNoise_MMX( pInfo );
        }
    }
    else
    {
        if( (gCpuFeatureFlags & FEATURE_SSE) || (gCpuFeatureFlags & FEATURE_MMXEXT) )
        {
            FilterTemporalNoise_SSE( pInfo );
        }
        else if( gCpuFeatureFlags & FEATURE_3DNOW )
        {
            FilterTemporalNoise_3DNOW( pInfo );
        }
        else
        {
            FilterTemporalNoise_MMX( pInfo );
        }
    }
    return 1000;
}

__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo(long CpuFeatureFlags)
{
    gCpuFeatureFlags = CpuFeatureFlags;
    return &TemporalNoiseMethod;
}

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}

