/////////////////////////////////////////////////////////////////////////////
// $Id: AspectDetect.cpp,v 1.14 2001-08-05 17:14:26 laurentg Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 Michael Samblanet.  All rights reserved.
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
//
// Aspect ratio control was started by Michael Samblanet <mike@cardobe.com>
// Moved into separate module by Mark D Rejhon.  
// AspectRatio.c was separated into 3 files by Michael Samblanet 13 March 2001
//
// The purpose of this module is all the calculations and handling necessary
// to map the source image onto the destination display, even if they are
// different aspect ratios.
//
// Portions copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
// Portions copyright (C) 2000 John Adcock
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 13 Mar 2001   Michael Samblanet     File created from code in AspectRatio.c
//                                     Split out to improve redability
//
//////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.13  2001/08/05 09:54:54  laurentg
// Bug fixed concerning WSS AR data in AR autodetection mode
//
// Revision 1.12  2001/08/03 09:44:06  koreth
// Remove an SSE dependency in black bar detection.
//
// Revision 1.11  2001/08/02 17:43:19  koreth
// Maybe fix WSS handling; the code was using an integer expression expecting it to behave like a floating-point one.
//
// Revision 1.10  2001/08/02 16:43:05  adcockj
// Added Debug level to LOG function
//
// Revision 1.9  2001/07/13 16:14:55  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.8  2001/07/12 16:16:39  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Other.h"
#include "AspectRatio.h"
#include "Resource.h"
#include "DebugLog.h"
#include "Status.h"
#include "BT848.h"
#include "DScaler.h"
#include "VBI_WSSdecode.h"

// From dtv.c .... We really need to reduce reliance on globals by going C++!
// Perhaps in the meantime, it could be passed as a parameter to WorkoutOverlay()
extern HMENU hMenu;
extern BOOL  bIsFullScreen;
extern void ShowText(HWND hWnd, LPCTSTR szText);

//---------
// Internal variables used by auto aspect ratio detect code.

// Number of seconds of aspect ratio history to keep.
#define RATIO_HISTORY_SECONDS   600

// Number of aspect ratio changes to remember.
#define RATIO_HISTORY_CHANGES   50

// Minimum aspect ratio encountered each second for the last several minutes.
// The 0th element of this is the current second.
static int min_ratio_found[RATIO_HISTORY_SECONDS];

// Timestamp of the most recent computation of min_ratio_found.
static int min_ratio_tick_count = 0;

// Aspect ratios we've used recently.
static int ratio_used[RATIO_HISTORY_CHANGES];

// When we switched to each of the ratios in ratio_used[].
static int ratio_time[RATIO_HISTORY_CHANGES];



//----------------------------------------------------------------------------
// Switch to a new aspect ratio and record it in the ratio history list.
// Use nRatio = -1 to only switch between anamorphic/nonanamorphic
void SwitchToRatio(int nMode, int nRatio)
{
    int now = GetTickCount();

    // Update anamorphic/nonanamorphic status
    AspectSettings.AspectMode = nMode;

    // Update aspect ratio only if a positive one is specified
    if (nRatio > 0)
    {
        LOG(1, " Switching to ratio %d", nRatio);

        // If the most recent ratio switch just happened, don't remember it since it
        // was probably a transient ratio due to improperly locking onto a dark scene.
        if (now - ratio_time[0] > AspectSettings.ShortRatioIgnoreMs)
        {
            memmove(&ratio_used[1], &ratio_used[0], sizeof(ratio_used[0]) * (RATIO_HISTORY_CHANGES - 1));
            memmove(&ratio_time[1], &ratio_time[0], sizeof(ratio_time[0]) * (RATIO_HISTORY_CHANGES - 1));
        }

        ratio_used[0] = nRatio;
        ratio_time[0] = GetTickCount();
        AspectSettings.SourceAspect = nRatio;
    }
    WorkoutOverlaySize();
}

//----------------------------------------------------------------------------
// Scan the top or bottom of a letterboxed image to find out where the picture
// starts.
//
// The basic idea is that we find a bounding rectangle for the image (which
// is assumed to be centered in the overlay buffer, an assumption the aspect
// ratio code makes in general) by searching from the top down to find the first
// scanline that isn't all black.  "All black" means that there aren't many
// pixels with luminance above a certain threshold.
//
// To support letterboxed movies shown on TV channels that put little channel
// logos in the corner, we allow the user to configure a percentage of the
// image that will be ignored on either side.  We only look at pixels in the
// middle of the image, so channel logos will be ignored.  The default is 15%.
// This is in addition to any pixels we ignore due to the m_Overscan setting.
//
// For speed's sake, the code is a little sloppy at the moment; it always
// looks at a multiple of 4 pixels.  It never looks at *more* pixels than are
// requested, though.
static inline int GetNonBlackCount(short* Line, int StartX, int EndX)
{
    int qwordCount = (EndX - StartX) / 4;
    int threshold;
    int counts;
    __int64 luminances;
    __int64 chromaMins;
    __int64 chromaMaxes;
	__int64 totals;
    const __int64 YMask = 0x00FF00FF00FF00FF;
    const __int64 OneMask = 0x0001000100010001;
    int chromaMin, chromaMax;

    if (qwordCount <= 0)
        return 0;

    // Black pixels are U=128 and V=128.  If U or V is significantly
    // different from 128, it's a different color, e.g. green if
    // they're both 0.  We compare each U and V Value to the chroma
    // min and max computed here: if ((chroma - chromaMin) > chromaMax)...
    chromaMax = AspectSettings.ChromaRange;
    if (chromaMax > 255)
        chromaMax = 255;
    chromaMin = 128 - (chromaMax / 2);

    threshold = AspectSettings.LuminanceThreshold;
    if (threshold > 255)
        threshold = 255;

    luminances = threshold;
    luminances |= (luminances << 48) | (luminances << 32) | (luminances << 16);
    chromaMins = chromaMin;
    chromaMins |= (chromaMins << 48) | (chromaMins << 32) | (chromaMins << 16);
    chromaMaxes = chromaMax;
    chromaMaxes |= (chromaMaxes << 48) | (chromaMaxes << 32) | (chromaMaxes << 16);

    // Start on a 16-byte boundary even if it means ignoring some extra
    // pixels?  Try not doing it for now, but it could make memory access
    // more efficient.

    Line += StartX;

    _asm {
        mov     ecx, qwordCount
        mov     eax, dword ptr[Line]
        movq    mm1, YMask
        movq    mm2, luminances
        movq    mm3, OneMask
        pxor    mm4, mm4            // mm4 = pixel counts, one Count in each word
        movq    mm6, chromaMins
        movq    mm7, chromaMaxes

BlackLoop:
        movq    mm0, qword ptr[eax] // mm0 = next 4 pixels
        movq    mm5, mm0
        psrlw   mm5, 8              // mm5 = chroma values in lower 8 bits of each word
        pand    mm0, mm1            // mm0 = luminance values in lower 8 bits of each word
        pcmpgtw mm0, mm2            // mm0 = 0xFFFF where luminance > threshold
        psubw   mm5, mm6            // mm5 = chroma - chromaMin
        pand    mm5, mm1            // mm5 = lower 8 bits of (chroma - chromaMin); <0 becomes positive
        pcmpgtw mm5, mm7            // mm5 = 0xFFFF where chroma < chromaMin or chroma > chromaMax
        por     mm0, mm5            // mm0 = 0xFFFF where U, V, or Y is past threshold
        pand    mm0, mm3            // Translate that into 1 for exceed, 0 for not, in each word
        paddw   mm4, mm0            // And add up the ones, maintaining four separate counts
        add     eax, 8              // Next qword
        loop    BlackLoop

		movq	totals, mm4			// Save totals so C code can deal with them
        emms
    }

	// Add up the four totals (each in one word of the "totals" qword) to
	// get the total overall count.
	counts = (int)(totals & 0xffff) +
			 (int)((totals >> 16) & 0xffff) +
			 (int)((totals >> 32) & 0xffff) +
			 (int)((totals >> 48) & 0xffff);

    //
    // Log the offending pixels
    if (counts > 0) {
        int x;
        LOG(3, "Count %d min %d max %d lumthresh %d", counts, chromaMin, chromaMax, threshold);
        for (x = 0; x < qwordCount * 4; x++) {
            if ((Line[x] & 0xff) > threshold || ((((Line[x] & 0xff00) >> 8) - chromaMin) & 0xff) > chromaMax)
            {
                LOG(3, "pixel %d lum %d chrom %d", x, Line[x] & 0xff, (Line[x] & 0xff00) >> 8);
            }
        }
    }
    

    return counts;
}

// direction is -1 to scan up from bottom of image, 1 to scan down from top.
int FindEdgeOfImage(short** EvenField, short** OddField, int direction)
{
    int y;
    short* line;
    int skipCount = 0;
    int skipCountPercent = AspectSettings.SkipPercent;
    int pixelCount;

    skipCount = (CurrentX * AspectSettings.SkipPercent / 100) +
                AspectSettings.InitialOverscan;

    // Decide whether we're scanning from the top or bottom
    if (direction < 0)
    {
        y = CurrentY - 1 - AspectSettings.InitialOverscan;  // from bottom
    }
    else
    {
        y = AspectSettings.InitialOverscan; // from top
    }

    for (; y != CurrentY / 2; y += direction)
    {
        if (y & 1)
            line = OddField[y / 2];
        else
            line = EvenField[y / 2];

        pixelCount = GetNonBlackCount(line, skipCount, CurrentX - skipCount * 2);
        if (pixelCount > 0)
        {
            LOG(3, "FindEdgeOfImage line %d Count %d", y, pixelCount);
        }

        if (pixelCount > AspectSettings.IgnoreNonBlackPixels)
            break;

// LG        if (y < 0 || y > CurrentY)
        if (y < 0 || y >= CurrentY)
        {
            LOG(2, "Sanity check failed; scanned past edge of screen");
// LG            y = (direction > 0) ? AspectSettings.InitialOverscan : CurrentY - AspectSettings.InitialOverscan;
            y = (direction > 0) ? AspectSettings.InitialOverscan : CurrentY - 1 - AspectSettings.InitialOverscan;
            break;
        }
    }

    return y;
}


//----------------------------------------------------------------------------
// Adjust the source aspect ratio to fit whatever is currently onscreen.
int FindAspectRatio(short** EvenField, short** OddField)
{
    int ratio;
    int topBorder, bottomBorder, border;
    int imageHeight = CurrentY - AspectSettings.InitialOverscan * 2;

    // If the aspect Mode is set to "use source", revert to assuming that the
    // source frame is 4:3.  We have to assume *some* source-frame aspect ratio
    // here or there aren't enough inputs to derive the material aspect ratio.
    if (AspectSettings.AspectMode == 0)
        AspectSettings.AspectMode = 1;

    // Find the top of the image relative to the m_Overscan area.  Overscan has to
    // be discarded from the computations since it can't really be regarded as
    // part of the picture.
    topBorder = FindEdgeOfImage(EvenField, OddField, 1) - AspectSettings.InitialOverscan;

    // Now find the size of the border at the bottom of the image.
// LG    bottomBorder = CurrentY - FindEdgeOfImage(EvenField, OddField, -1) - AspectSettings.InitialOverscan;
    bottomBorder = CurrentY - 1 - FindEdgeOfImage(EvenField, OddField, -1) - AspectSettings.InitialOverscan;

    // The border size is the smaller of the two.
    border = (topBorder < bottomBorder) ? topBorder : bottomBorder;

    // Now the material aspect ratio is simply
    //
    //  effective width / (total image height - number of black lines at top and bottom)
    //
    // We compute effective width from height using the source-frame aspect ratio, since
    // this will change depending on whether or not the image is anamorphic.
// LG    ratio = (int)((imageHeight * 1000) * GetActualSourceFrameAspect() / (imageHeight - border * 2));
    if ((imageHeight - border * 2) > 0)
	{
        ratio = (int)((imageHeight * 1000) * GetActualSourceFrameAspect() / (imageHeight - border * 2));
	}
	else
	{
        // Screen is all black => don't change ratio
        ratio = AspectSettings.SourceAspect;
	}
    LOG(2, " top %d bot %d bord %d rat %d", topBorder, bottomBorder, border, ratio);

    return ratio;
}

//----------------------------------------------------------------------------
// Automatic Aspect Ratio Detection
// Continuously adjust the source aspect ratio.  This is called once per frame.
void AdjustAspectRatio(long SourceAspectAdjust, short** EvenField, short** OddField)
{
    static int lastNewRatio = 0;
    static int newRatioFrameCount = 0;
    static long LastSourceAspectAdjust = -1;
    int newRatio, newMode;
    int tick_count = GetTickCount();
    int tickCutoff = tick_count - (AspectSettings.AspectHistoryTime * 1000);
    int i;
    int haveSeenThisRatio, haveSeenSmallerRatio;
    int WssSourceRatio;
    int maxRatio;


    if(EvenField == NULL || OddField == NULL)
    {
        return;
    }

    // first see if we have go any filters changing the aspect ratio
    if(LastSourceAspectAdjust != SourceAspectAdjust)
    {
        AspectSettings.SourceAspectAdjust = SourceAspectAdjust;
        WorkoutOverlaySize(TRUE);
        LastSourceAspectAdjust = SourceAspectAdjust;
        return;
    }

    // ADDED by Mark Rejhon: Eliminates the "tiny slit" problem in starry 
    // scenes such as those in Star Wars or start of Toy Story 2,
    // at least during full screen Mode.
    // FIXME: Would be nice to access 'AdjustedWindowAspect' in WorkoutOverlaySize()
    // so that I can also do this for windowed Mode too.
    if (AspectSettings.DetectAspectNow || AspectSettings.AutoDetectAspect)
    {
        newRatio = FindAspectRatio(EvenField, OddField);

        // Get aspect ratio from WSS data
        // (WssSourceRatio = -1 if ratio is not defined in WSS data)
        if (! WSS_GetRecommendedAR(&newMode, &WssSourceRatio))
        {
            newMode = AspectSettings.AspectMode;
            WssSourceRatio = -1;
        }

        // The ratio must be at least the ratio defined in WSS data
        if (newRatio < WssSourceRatio)
        {
            newRatio = WssSourceRatio;
        }

        if (bIsFullScreen && AspectSettings.TargetAspect)
        {
            if (AspectSettings.TargetAspect > WssSourceRatio)
                maxRatio = AspectSettings.TargetAspect;
            else
                maxRatio = WssSourceRatio;
            if (newRatio > maxRatio)
            {
                newRatio = maxRatio;
            }
        }
    }

    // If the user told us to detect the current ratio, do it.
    if (AspectSettings.DetectAspectNow)
    {

        SwitchToRatio(newMode, newRatio);
        newRatioFrameCount = 0;
        AspectSettings.DetectAspectNow = FALSE;
        return;
    }
    else if (AspectSettings.AutoDetectAspect)
    {
        // If we've just crossed a 1-second boundary, scroll the aspect ratio
        // histories.  If not, update the max ratio found in the current second.
        if (tick_count / 1000 != min_ratio_tick_count / 1000)
        {
            min_ratio_tick_count = tick_count;
            memmove(&min_ratio_found[1], &min_ratio_found[0], sizeof(min_ratio_found[0]) * (RATIO_HISTORY_SECONDS - 1));
            min_ratio_found[0] = newRatio;
        }
        else if (newRatio < min_ratio_found[0])
        {
            min_ratio_found[0] = newRatio;
        }

        // If the Mode (anamorphic/non anamorphic) changes, switch to the new
        // Mode and ratio immediately
        if (newMode != AspectSettings.AspectMode)
        {
            SwitchToRatio(newMode, newRatio);
            newRatioFrameCount = 0;
        }
        // If the new ratio is less than the old one -- that is, if we've just
        // become less letterboxed -- switch to the new ratio immediately to
        // avoid cutting the image off.
        else if (newRatio < AspectSettings.SourceAspect)
        {
            SwitchToRatio(newMode, newRatio);
            newRatioFrameCount = 0;
        }
        else if (ABS(newRatio - AspectSettings.SourceAspect) > AspectSettings.AspectEqualFudgeFactor && newRatio == lastNewRatio)
        {
            // Require the same aspect ratio for some number of frames, or no
            // narrower aspect ratio found for the last AspectConsistencyTime seconds,
            // before zooming in.
            haveSeenSmallerRatio = 0;
            for (i = 1; i < AspectSettings.AspectConsistencyTime; i++)
                if (newRatio > min_ratio_found[i])
                {
                    haveSeenSmallerRatio = 1;
                    break;
                }

            if (! haveSeenSmallerRatio || ++newRatioFrameCount >= AspectSettings.ZoomInFrameCount)
            {
                // If we're looking at aspect ratio histories, the new ratio must be
                // equal (or very close) to one we've _used_ in the recent past, or
                // there must have been no smaller ratio _found_ in the recent past.
                // That is, don't zoom in more than we've zoomed in recently unless
                // we'd previously zoomed to the same ratio.  This helps prevent
                // temporary zooms into letterboxed material during dark scenes, while
                // allowing the code to quickly switch in and out of zoomed Mode when
                // full-frame commercials come on.
                haveSeenThisRatio = 0;
                if (AspectSettings.AspectHistoryTime > 0)
                {
                    for (i = 1; i < RATIO_HISTORY_CHANGES && ratio_time[i] > tickCutoff; i++)
                        if (ABS(newRatio - ratio_used[i]) <= AspectSettings.AspectEqualFudgeFactor)
                        {
                            haveSeenThisRatio = 1;
                            break;
                        }
                }

                if (AspectSettings.AspectConsistencyTime <= 0 ||
                    (haveSeenThisRatio && newRatioFrameCount >= AspectSettings.ZoomInFrameCount) ||
                    ! haveSeenSmallerRatio)
                {
                    SwitchToRatio(newMode, newRatio);
                }
            }
        }
        else
        {
            // If this is a wider ratio than the previous one, require it to stick
            // around for the full frame Count.
            if (lastNewRatio < newRatio)
                newRatioFrameCount = 0;

            lastNewRatio = newRatio;
        }
    }
}

