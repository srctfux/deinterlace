/////////////////////////////////////////////////////////////////////////////
// DI_Adaptive.c
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 Mark Rejhon and Steve Grimm.  All rights reserved.
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
// 07 Jan 2001   John Adcock           Split into separate module
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "deinterlace.h"
#include "cpu.h"
#define DOLOGGING
#include "DebugLog.h"
#include "OutThreads.h"
#include "Status.h"

long				StaticImageFieldCount = 100;
long				LowMotionFieldCount = 4;
ePULLDOWNMODES		StaticImageMode = SIMPLE_WEAVE;
ePULLDOWNMODES		LowMotionMode = VIDEO_MODE_2FRAME;
ePULLDOWNMODES		HighMotionMode = VIDEO_MODE_2FRAME;

///////////////////////////////////////////////////////////////////////////////
// AdaptiveDeinterlace
//
// This mode supports three styles of deinterlacing and switch among
// them depending on the amount of motion in the scene.  If there's a lot of
// motion, we use HighMotionMode.  If there's little or no motion for at least
// LowMotionFieldCount fields, we switch to LowMotionMode.  And if, after that,
// there's no motion for StaticImageFieldCount fields, we switch to
// StaticImageMode.
//
// Exactly which modes are used in these three cases is configurable in the INI
// file. It is entirely legal for some or all of the three video modes to be the
// same; by default, VIDEO_MODE_2FRAME is used for both the high and low motion
// modes.  On slower machines VIDEO_MODE_BOB (high) and VIDEO_MODE_WEAVE (low)
// can be used instead since they're less CPU-intensive.
///////////////////////////////////////////////////////////////////////////////
BOOL AdaptiveDeinterlace(DEINTERLACE_INFO *info)
{
	int CombFactor = 0;
	static long MATCH_COUNT = 0;
	static ePULLDOWNMODES CurrentMode = VIDEO_MODE_2FRAME;


	// reset MATCH_COUNT when we are called and the info
	// struct doesn't contain at least an odd and an even frame
	if(info->EvenLines[0] == NULL || info->OddLines[0] == NULL)
	{
		MATCH_COUNT = 0;
		CurrentMode = HighMotionMode;
		StatusBar_ShowText(STATUS_PAL, "Adaptive - High Motion");
		return Bob(info);
	}

	// If the field difference is bigger than the threshold, then
	// the current field is very different from the field two fields ago.
	// so reset the match count and switch back
	// to either low or high motion
    if(info->FieldDiff > Threshold32Pulldown)
	{
		MATCH_COUNT = 0;

		// If we're in still mode, it might be okay to drop to
		// low-motion mode.
		if (CurrentMode == StaticImageMode &&
			info->FieldDiff < ThresholdPulldownMismatch)
		{
			CurrentMode = LowMotionMode;
			LOG(" Match count 0, switching to low-motion");
			StatusBar_ShowText(STATUS_PAL, "Adaptive - Low Motion");
		}
		else if(CurrentMode != HighMotionMode)
		{
			CurrentMode = HighMotionMode;
			LOG(" Match count 0, switching to high-motion");
			StatusBar_ShowText(STATUS_PAL, "Adaptive - High Motion");
		}
	}
    else
	{
		MATCH_COUNT++;

		if (MATCH_COUNT >= LowMotionFieldCount &&
			CurrentMode == HighMotionMode)
		{
			CurrentMode = LowMotionMode;
			LOG(" Match count %ld, switching to low-motion", MATCH_COUNT);
			StatusBar_ShowText(STATUS_PAL, "Adaptive - Low Motion");
		}
		if (MATCH_COUNT >= StaticImageFieldCount &&
			CurrentMode == LowMotionMode)
		{
			CurrentMode = StaticImageMode;
			LOG(" Match count %ld, switching to static-image", MATCH_COUNT);
			StatusBar_ShowText(STATUS_PAL, "Adaptive - Static");
		}
	}
	return DeintMethods[CurrentMode].pfnAlgorithm(info);
}

