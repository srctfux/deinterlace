/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001, 2002 Lindsey Dubb.  All rights reserved.
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
// Revision 1.8  2002/02/01 23:50:39  lindsey
// Disbled the debug flag again
//
// Revision 1.7  2002/02/01 23:49:28  lindsey
// Corrected filter disabling on MMX computers
// Slightly changed starting values
//
// Revision 1.6  2002/01/26 01:03:11  lindsey
// Fixed some comments
// Effect of coefficient of varience on reliability scaled to the sample size
//
// Revision 1.5  2002/01/26 00:47:00  lindsey
// Fixed big bug in noise threshold / motion correlation interaction
// Fixed bug in parameterization
// Changed to an additive filter for motion correlation
// Changed parameterization
// Changed use of coefficient of variation in reliability estimate
// Reduced size of histogram
// Optimized a little
// Moved algorithm description to a separate file
//
// Revision 1.4  2002/01/17 07:55:12  lindsey
// Turned off the debug flag
//
// Revision 1.3  2002/01/17 07:50:32  lindsey
// Increased effect of nearby motion on averaging
// Moved some magic numbers into #defines
// Reduced artifacts with the "lock dot"
// Slightly changed parameterization
// Improved documentation and formatting
//
// Revision 1.2  2002/01/04 01:29:54  lindsey
// Changed parameterization
//
// Revision 1.1.1.1  2001/12/31 01:25:19  lindsey
// Added FLT_AdaptiveNoise
//
//
/////////////////////////////////////////////////////////////////////////////

// #define ADAPTIVE_NOISE_DEBUG

#include <limits.h>
#include <math.h>

#include "windows.h"
#include "DS_Filter.h"


// See FLT_AdaptiveNoise.txt for an explanation of this filter.


/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////

// This might need to be tuned for other processors.
// Then again, it doesn't seem to matter much what I set it to on my Athlon
// so long as it's between 64 and about 300

#define PREFETCH_STRIDE                         128

// These constants help place the noise maximum, above which further indication
// of motion is ignored.  This maximum results in a false peak in the histogram
// which is due to motion map values above the noise peak, times one noise decay.

// NOISE_MAX_CURVE_WIDTHS is the number of "curve widths" (distance from the ~1/16
// quantile and the peak) away from the histogram peak at which to allow the
// artifactual peak. NOISE_MAX_EXTRA is added directly to the noise threshold.

#define NOISE_MAX_CURVE_WIDTHS                  8
#define NOISE_MAX_EXTRA                         25

// Minimum acceptable height at the low quantile

#define MIN_QUANTILE_HEIGHT                     6

// Intervals at which to make comparisons when finding peaks in the histogram

#define PEAK_NEAR_COMPARISON                    1
#define PEAK_MIDDLE_COMPARISON                  4
#define PEAK_FAR_COMPARISON                     16

// Location (horizontal and vertical distance from the top left) of the "lock dot"
// V is defined as lines within the field

#define LOCK_DOT_H                              32
#define LOCK_DOT_V                              16

// Initial values of the reliability, baseline, and first and second moving moments of the
// peak estimate.  The initial reliability should be low to reflect a lack of knowledge when
// the filter is initialized.

#define   INITIAL_RELIABILITY                   0.001
#define   INITIAL_BASELINE                      75.0 
#define   INITIAL_PEAK_MEAN                     100.0
#define   INITIAL_PEAK_SQUARED_MEAN             10000.0

// Used to scale up the estimate of reliability of the signal

#define SIGNAL_STRENGTH_MULTIPLIER              50

// Multiplier applied to reliability of peak estimate when it is potentially a "false peak"
// (due to decay from the noise maximum)

#define PAST_PEAK_REDUCTION                     0.2

// Base rate at which the estimate of reliability is moved toward the new estimate

#define RELIABILITY_DECAY                       0.008

// Vertical scale of the histogram display: Each hash mark represents 10*HISTOGRAM_SCALE points of
// the cumulative color difference.

#define HISTOGRAM_SCALE                         4

// Total length of the histogram
// If gDecayCoefficient is increased above ~95%, this should be increased, too
// This must be evenly divisible by sizeof(DOUBLE) because of the way it is cleared

#define HISTOGRAM_LENGTH                        1024

// Rate of decay of the mean and mean^2 averaging for determining peak variance

#define PEAK_VARIANCE_DECAY                     0.80

// Multiplier to the coefficient of variance

#define COEFF_VAR_SCALING                       8

/////////////////////////////////////////////////////////////////////////////
// Function prototypes
/////////////////////////////////////////////////////////////////////////////

long            FilterAdaptiveNoise( TDeinterlaceInfo* pInfo );
long            FilterAdaptiveNoise_DEBUG( TDeinterlaceInfo* pInfo );

void            AnalyzeHistogram( TDeinterlaceInfo* pInfo, DWORD MaxNoise, DOUBLE* pCumBaseline,
                    DOUBLE* pCumDifferencePeak, BOOLEAN* pDoShowDot );

void __cdecl    ExitAdaptiveNoise( void );
void            CleanupAdaptiveNoise( void );


/////////////////////////////////////////////////////////////////////////////
// Begin plugin globals
/////////////////////////////////////////////////////////////////////////////

// Map of the N statistic (see the explanation of the algorithm, above)

static BYTE*    gpChangeMap = NULL;

// Histogram of the number of pixels with each value of the N statistic

static BYTE*    gpHistogram = NULL;

// Percent (well, sort of a percent) of spatially and temporally shared motion
// evidence

// This setting is currently hidden, since it doesn't have much useful play.
// 88 looks like the best compromise between keeping old information (which improves
// detection of motion) and stability of the motion estimate. (If the decay is any
// higher, the peak becomes bimodal as ~half of it is displaced downward.)

long            gDecayCoefficient = 83;

// Determines the placement toward the "start" of the N histogram at which we completely
// compensate for noise

long            gStability = 25;

// Determines the placement after the peak of the N histogram where we decide that there
// must be motion in a block

long            gNoiseReduction = 55;

// Turn on a histogram and statistics readout

long            gIndicator = FALSE;

// Turn on a dot when the running estimate of noise is close to the estimate from the
// current picture

long            gShowLockDot = FALSE;


#ifdef ADAPTIVE_NOISE_DEBUG
// Toggle pink pixel flashing

long            gTestingFlag = TRUE;

// Value of N above which the block will periodically be flashed pink

long            gTestingThreshold = 50;

// Value of N below which the block will periodically be flashed pink

long            gSecondTestingThreshold = 50;
#endif  // Debug globals


/////////////////////////////////////////////////////////////////////////////
// Control settings
/////////////////////////////////////////////////////////////////////////////


// The debug version has a few more switches

#ifdef ADAPTIVE_NOISE_DEBUG
typedef enum
{
    ANOISETESTOPTION = FLT_ANOISE_SETTING_LASTONE,
    ANOISETESTTHRESHOLD,
    ANOISESECONDTESTTHRESHOLD,
    FLT_ANOISE_SETTING_DEBUG_LASTONE,
} FLT_ANOISE_DEBUG_SETTING;
#endif


FILTER_METHOD AdaptiveNoiseMethod;


#ifdef ADAPTIVE_NOISE_DEBUG
SETTING FLT_AdaptiveNoiseSettings[FLT_ANOISE_SETTING_DEBUG_LASTONE] =
#else
SETTING FLT_AdaptiveNoiseSettings[FLT_ANOISE_SETTING_LASTONE] =
#endif
{
    {
        "Stability", SLIDER, 0, &gStability,
        25, 0, 100, 1, 1,
        NULL,
        "AdaptiveNoiseFilter", "AStability", NULL,
    },
    {
        "Noise Reduction", SLIDER, 0, &gNoiseReduction,
        55, 0, 200, 1, 1,
        NULL,
        "AdaptiveNoiseFilter", "AdaptiveNoiseReduction", NULL,
    },
    {
        "Lock Dot", ONOFF, 0, &gShowLockDot,
        FALSE, 0, 1, 1, 1,
        NULL,
        "AdaptiveNoiseFilter", "LockDot", NULL,
    },
    {
#ifdef ADAPTIVE_NOISE_DEBUG
        "Use Adaptive Noise Filter", ONOFF, 0, &AdaptiveNoiseMethod.bActive,
#else
        "Use Adaptive Noise Filter", ONOFF, 0, &AdaptiveNoiseMethod.bActive,
#endif // Show "activate" flag when testing
        FALSE, 0, 1, 1, 1,
        NULL,
        "AdaptiveNoiseFilter", "ActivateANoiseFilter", NULL,
    },
    {
#ifdef ADAPTIVE_NOISE_DEBUG
        // Okay, so they're not pink
        "Pink dots", ONOFF, 0, &gIndicator,
#else
        "Pink dots", NOT_PRESENT, 0, &gIndicator,
#endif // Hide histogram option (but keep it in the .ini file) for users
        FALSE, 0, 1, 1, 1,
        NULL,
        "AdaptiveNoiseFilter", "APinkIndicator", NULL,
    },
        {
#ifdef ADAPTIVE_NOISE_DEBUG
        "Motion memory (percent)", SLIDER, 0, &gDecayCoefficient,
#else
        "Motion memory (percent)", NOT_PRESENT, 0, &gDecayCoefficient,
#endif // Show motion memory slider when testing
        83, 0, 99, 1, 1,
        NULL,
        "AdaptiveNoiseFilter", "MotionSharing", NULL,
    },
#ifdef ADAPTIVE_NOISE_DEBUG
    {
        "Use Testing Switch", ONOFF, 0, &gTestingFlag,
        TRUE, 0, 1, 1, 1,
        NULL,
        "AdaptiveNoiseFilter", "ATestingIndicator", NULL,
    },
    {
        "Testing Threshold", SLIDER, 0, &gTestingThreshold,
        50, 0, 2000, 1, 1,
        NULL,
        "AdaptiveNoiseFilter", "ATestingThreshold", NULL,
    },
    {
        "Second Testing Threshold", SLIDER, 0, &gSecondTestingThreshold,
        50, 0, 2000, 1, 1,
        NULL,
        "AdaptiveNoiseFilter", "AsecondTestingThreshold", NULL,
    },
#endif  // Add a debug switch and two sliders for testing purposes
};


FILTER_METHOD AdaptiveNoiseMethod =
{
    sizeof(FILTER_METHOD),
    FILTER_CURRENT_VERSION,
    DEINTERLACE_INFO_CURRENT_VERSION,
    "Adaptive Noise Filter",                // Displayed name
    "Noise Reduction (Adaptive)",           // For consistency with the Temporal Noise filter
    FALSE,                                  // Not initially active
    TRUE,                                   // Call on input so pulldown/deinterlacing can benefit
    FilterAdaptiveNoise,                    // Algorithm to use (really decided by GetFilterPluginInfo)
    0,                                      // Menu: assign automatically
    FALSE,                                  // Does not run if we've run out of time
    NULL,                                   // No initialization procedure
    ExitAdaptiveNoise,                      // Deallocation routine
    NULL,                                   // Menu handle (for DScaler)
#ifdef ADAPTIVE_NOISE_DEBUG
    FLT_ANOISE_SETTING_DEBUG_LASTONE,
#else
    FLT_ANOISE_SETTING_LASTONE,             // Number of settings
#endif  // More settings when in debug mode
    FLT_AdaptiveNoiseSettings,
    WM_FLT_ANOISE_GETVALUE - WM_USER,       // Settings offset
    TRUE,                                   // Can handle interlaced material
    2,                                      // Requires field before last
};


/////////////////////////////////////////////////////////////////////////////
// Main code (included from FLT_AdaptiveNoise.asm)
/////////////////////////////////////////////////////////////////////////////

#ifdef ADAPTIVE_NOISE_DEBUG
long DispatchAdaptiveNoise( TDeinterlaceInfo *pInfo )
{
    long    FilterResult = 1000;

    if( gTestingFlag == TRUE )
    {
        FilterResult = FilterAdaptiveNoise_DEBUG( pInfo );
    }
    else
    {
        FilterResult = FilterAdaptiveNoise( pInfo );
    }
    return FilterResult;
}
#endif // Function dispatcher for the debug version


// Include the main code from a different file to allow for easy comparisons of
// different algorithms -- or different versions for different processors, if that
// becomes a good idea.

#include "FLT_AdaptiveNoise.asm"

#ifdef ADAPTIVE_NOISE_DEBUG
#define IS_DEBUG_FLAG
#include "FLT_AdaptiveNoise.asm"
#undef IS_DEBUG_FLAG
#endif  // Debug switch version of code


////////////////////////////////////////////////////////////////////////////
// Start of utility code
/////////////////////////////////////////////////////////////////////////////


// Memory cleanup routines

void __cdecl ExitAdaptiveNoise( void )
{
    CleanupAdaptiveNoise();
    return;
}


void CleanupAdaptiveNoise( void )
{
    if( gpChangeMap != NULL )
    {
        free( gpChangeMap );
        gpChangeMap = NULL;
    }
    if( gpHistogram != NULL )
    {
        free( gpHistogram );
        gpHistogram = NULL;
    }
    return;
}


// DScaler interaction routines

__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo( long CpuFeatureFlags )
{
    // We need psadbw and make use of the max/min instructions, so at least
    // MMXEXT is required.
    if( CpuFeatureFlags & (FEATURE_SSE | FEATURE_MMXEXT) )
    {
#ifdef ADAPTIVE_NOISE_DEBUG
        AdaptiveNoiseMethod.pfnAlgorithm = DispatchAdaptiveNoise;
#else
        AdaptiveNoiseMethod.pfnAlgorithm = FilterAdaptiveNoise;
#endif // Debug switch version of code
    }
    else
    {
        AdaptiveNoiseMethod.pfnAlgorithm = NULL;
    }

    if (AdaptiveNoiseMethod.pfnAlgorithm == NULL)
    {
        return NULL;
    }
    else
    {
        return &AdaptiveNoiseMethod;
    }
}


// Need malloc() and free(), so this is commented out:
/*
BOOL WINAPI _DllMainCRTStartup( HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved )
{
    return TRUE;
}
*/


/*

Automated curve psychoanalysis:

This is currently poorly documented.  Yes, I hope to fix it eventually.

Often, material (especially sports) will have a few brief parts with good information about the noise, but
otherwise have too much motion to tell much.  This routine is designed to lock onto the good noise estimate,
but to eventually give up and go with a poor estimate if that's all which is available.  The use of poor
estimates is necessary when viewing material with heavy interference.

The baseline of the noise is easy to determine -- It's just defined as a low quantile of the histogram.
More exactly, it's defined as the value at which a fixed number of pixels (proportional to the pixel
width) are at a lower value in the histogram.  That's not a real quantile, but it's more robust to bad
peak estimates than a real quantile would be.

The peak is a little tougher -- I use a median smoother followed by a simple hill climber.  The peak
finder is meant to find the first moderately large peak.  This purposefully avoids higher valued peaks
in favor of the one with the lowest color difference, since the lowest peak is most likely to be
the one which represents noise.  There are two drawbacks to this:
- If the screen is showing material with two different noise levels, only the lower will be detected
- If interference is affecting only part of the screen, only the cleanest part will be detected

This peak finder works poorly when the histogram is bumpy.  This is mostly beneficial -- when the
histogram is very bumpy, there's usually a lot of motion in the picture, so missing the strongest
peak is a good idea.

The reliability measure of an estimate depends on four factors:
- The height of the histogram's peak
- The (time weighted) coefficient of variation of the peak estimate
- The difference between the new peak estimate and the old estimate
- The reliability of the previous estimate

This is handled in a fairly haphazard way, with some arbitrary powers and constants  -- I'll just refer you
to the code, since it's likely to change before I next update this comment.  Good luck, since the commenting in
the code is poor.

This could be made much less arbitrary (and be significantly more accurate) by keeping track of the likelihood
of the data given each proposed value of the peak.

There is also some code in here to show a bunch of data about the histogram.  This can be a very useful
tool for people working on motion detection.  The histogram shows:

- The histogram is displayed on the left of the screen.  Each hash mark represents 40 points of the
  motion statistic (decayed sum of absolute color differences).
- There are five dots which appear within the histogram display.  From left to right:
    - The current peak estimate
    - The current baseline estimate
    - The cumulative baseline estimate
    - The cumulative peak estimate
    - The quantity (maximum allowed noise value * decay). Expect a peak just above this if there is lots of motion.
      This dot often drops below the bottom of the screen.
- There are two dots which appear above the histogram.  From top to bottom:
    - The reliability
    - (1.0 - Coefficient of variation of the peak estimate)^16  (Yes, really!)

*/

void AnalyzeHistogram( TDeinterlaceInfo* pInfo, DWORD MaxNoise, DOUBLE* pCumBaseline,
                            DOUBLE* pCumDifferencePeak, BOOLEAN* pDoShowDot )
{
    LONG            Index = 0;

    static DOUBLE   sReliability = INITIAL_RELIABILITY;             // Reliability of current estimates
    static DOUBLE   sPeakMean = INITIAL_PEAK_MEAN;                  // Accumulated average of noise value peaks (for variance calculation)
    static DOUBLE   sPeakSquaredMean = INITIAL_PEAK_SQUARED_MEAN;   // Accumulated average of squared noise value peaks (for variance)

    // Reset the estimates if we've switched the picture source

    if( pInfo->PictureHistory[0]->IsFirstInSeries )
    {
        sReliability = INITIAL_RELIABILITY;
        sPeakMean = INITIAL_PEAK_MEAN;
        sPeakSquaredMean = INITIAL_PEAK_SQUARED_MEAN;
        *pCumBaseline = INITIAL_BASELINE;
        *pCumDifferencePeak = INITIAL_PEAK_MEAN;
    }

    // Medianish smoothing of the histogram (doesn't really need to be done for the whole histogram)

    for( Index = 1; Index < HISTOGRAM_LENGTH - 1; ++Index )
    {
        DWORD           Highest = 0;
        DWORD           Lowest = 0;
        DWORD           ThisTry = 0;
        DWORD*          pDWordHistogram = (DWORD*)gpHistogram;

        Highest = pDWordHistogram[Index - 1];
        Lowest = Highest;
        ThisTry = pDWordHistogram[Index];
        if( ThisTry > Highest )
        {
            Highest = ThisTry;
        }
        else 
        {
            Lowest = ThisTry;
        }
        ThisTry = pDWordHistogram[Index+1];
        if( ThisTry > Highest )
        {
            pDWordHistogram[Index] = Highest;
        }
        else if( ThisTry < Lowest )
        {
            pDWordHistogram[Index] = Lowest;
        }
        else
        {
            pDWordHistogram[Index] = ThisTry;
        }
    }


    // Draw histogram

    if( gIndicator == TRUE )
    {
        for( Index = 0; Index < pInfo->FieldHeight - 20; ++Index )
        {
            if ( ((DWORD*)gpHistogram)[Index*HISTOGRAM_SCALE] < (DWORD) (pInfo->FrameWidth*2 - 20) )
            {
                *(pInfo->PictureHistory[0]->pData + ((Index+20) * pInfo->InputPitch) + (((DWORD*)gpHistogram)[Index*HISTOGRAM_SCALE]/2)*2) = 0xFF;
            }
            else
            {
                *(pInfo->PictureHistory[0]->pData + ((Index+20) * pInfo->InputPitch) + pInfo->FrameWidth*2 - 20) = 0xFF;
            }
            if (Index % 10 == 0)
            {
                *(pInfo->PictureHistory[0]->pData + ((Index+20) * pInfo->InputPitch) + 100) = 0xFF;
            }
        }
    }

    // Find baseline and first local maximum in histogram, as well as their reliability

    {
        DWORD*      pDWordHistogram = (DWORD*)gpHistogram;
        DOUBLE      SignalStrength = 0.0;
        DOUBLE      CoeffVar = 0.01;
        DOUBLE      ProbSameSignal = 0.0;   // Not really a probability
        DWORD       Accumulator = 0;
        DWORD       Baseline = 0;
        DWORD       DeltaPeak = 0;
        LONG        FalsePeak = 0;
        
        // Find the part of the histogram after which the false peak (due to decay from the MaxNoise threshold)
        // will be found.

        FalsePeak = (LONG)(MaxNoise*(gDecayCoefficient+1))/100;
        if( FalsePeak > HISTOGRAM_LENGTH - PEAK_FAR_COMPARISON - 1 )
        {
            FalsePeak = HISTOGRAM_LENGTH - PEAK_FAR_COMPARISON - 1;
        }

        // Find baseline (  = LineLength / 16 accumulated value )
        // Note that this isn't a real quantile.  But it is robust to poor peak estimates.
        
        for( Index = 0; Index <= FalsePeak; ++Index )
        {

            Accumulator += pDWordHistogram[Index];
            if( (pDWordHistogram[Index] >= MIN_QUANTILE_HEIGHT) && (Accumulator >  pInfo->LineLength) )
            {
                if( (gIndicator == TRUE) && (Index/HISTOGRAM_SCALE < pInfo->FieldHeight - 20) )
                {   // Show this field's baseline
                    *(pInfo->PictureHistory[0]->pData + ((Index/HISTOGRAM_SCALE+20) * pInfo->InputPitch) + 300) = 0xFF;
                }
                Baseline = Index;
                break;
            }
        }

        // Find lowest peak maximum:

        for( ; Index < HISTOGRAM_LENGTH; ++Index )
        { // Simple peak finder -- It's designed to find the first moderately big peak
            if( (pDWordHistogram[Index] >= 5) &&
                (pDWordHistogram[Index+PEAK_NEAR_COMPARISON] < pDWordHistogram[Index]) &&
                (pDWordHistogram[Index+PEAK_MIDDLE_COMPARISON] <= pDWordHistogram[Index+PEAK_NEAR_COMPARISON]) &&
                (pDWordHistogram[Index+PEAK_FAR_COMPARISON] <= pDWordHistogram[Index+PEAK_MIDDLE_COMPARISON]) )
            {
                if( (gIndicator == TRUE) && (Index/HISTOGRAM_SCALE < pInfo->FieldHeight - 20) )
                {   // Show this field's peak
                    *(pInfo->PictureHistory[0]->pData + ((Index/HISTOGRAM_SCALE+20) * pInfo->InputPitch) + 200) = 0xFF;
                }

                DeltaPeak = Index;

                sPeakMean = sPeakMean * PEAK_VARIANCE_DECAY +
                    DeltaPeak * (1.0 - PEAK_VARIANCE_DECAY);
                sPeakSquaredMean = sPeakSquaredMean * PEAK_VARIANCE_DECAY +
                    (DeltaPeak * DeltaPeak) * (1.0 - PEAK_VARIANCE_DECAY);
                CoeffVar = sPeakSquaredMean - sPeakMean*sPeakMean;  // the variance
                if( sPeakSquaredMean < .999 )
                {   // Avoid division by 0
                    sPeakSquaredMean = .999;
                }
                CoeffVar /= sPeakSquaredMean;                       // really CV^2
                if( CoeffVar < 0.000001 )
                {   // Avoid division by 0
                    CoeffVar = 0.000001;
                }
                CoeffVar = 1.0/sqrt(CoeffVar);
                CoeffVar /= sqrt(pInfo->FrameWidth*(2 + (pInfo->DestRect.bottom - pInfo->DestRect.top)/2));
                CoeffVar *= COEFF_VAR_SCALING;
                if (CoeffVar > 0.999999)
                {
                    CoeffVar = 0.999999;
                }
                // Show Coefficient of variation statistic
                if( gIndicator == TRUE )
                {
                    DWORD  Jindex = (DWORD)(CoeffVar*pInfo->FrameWidth);
                    Jindex *= 2;
                    *(pInfo->PictureHistory[0]->pData + (15 * pInfo->InputPitch) + Jindex) = 0xFF;
                }

                SignalStrength = pDWordHistogram[DeltaPeak]*sqrt((DOUBLE)DeltaPeak);
                // Normalize the maximum strength of signal to the amount of data collected
                SignalStrength /= pInfo->FrameWidth*(2 + (pInfo->DestRect.bottom - pInfo->DestRect.top)/2);
                SignalStrength *= SIGNAL_STRENGTH_MULTIPLIER;
                SignalStrength *= CoeffVar;
                if (SignalStrength > 1.0)
                {
                    SignalStrength = 1.0;
                }
                SignalStrength *= SignalStrength;
                SignalStrength *= SignalStrength;
                SignalStrength *= RELIABILITY_DECAY;

                // Jump out since we've found the peak
                break;
            }
        }

        // If the peak is after the false peak threshold, downweight it.
        if( Index >= (LONG)(MaxNoise*(gDecayCoefficient+1))/100 )
        {
            SignalStrength *= PAST_PEAK_REDUCTION;
        }

        // Update the baseline and peak estimates, weighting with new estimate reliability relative to old reliability

        *pCumBaseline = (*pCumBaseline)*(sReliability/(SignalStrength+sReliability)) +
            Baseline*SignalStrength/(sReliability+SignalStrength);
        *pCumDifferencePeak = (*pCumDifferencePeak)*(sReliability/(SignalStrength+sReliability)) +
            DeltaPeak*SignalStrength/(sReliability+SignalStrength);

        if( (gIndicator == TRUE) && ((*pCumBaseline)/HISTOGRAM_SCALE < pInfo->FieldHeight - 20) )
        {   // Show baseline estimate
            *(pInfo->PictureHistory[0]->pData + (((LONG)((*pCumBaseline)/HISTOGRAM_SCALE) + 20) * pInfo->InputPitch) + 400) = 0xFF;
        }
        if( (gIndicator == TRUE) && ((*pCumDifferencePeak)/HISTOGRAM_SCALE < pInfo->FieldHeight - 20) )
        {   // Show peak estimate
            *(pInfo->PictureHistory[0]->pData + (((LONG)((*pCumDifferencePeak)/HISTOGRAM_SCALE) + 20) * pInfo->InputPitch) + 500) = 0xFF;
        }

        // If we get a weak signal at the location of our current estimate, don't drop the reliability

        ProbSameSignal = (*pCumDifferencePeak) - DeltaPeak;
        ProbSameSignal = ProbSameSignal*ProbSameSignal/(*pCumDifferencePeak);
        ProbSameSignal = 1.0 - ProbSameSignal;
        if( ProbSameSignal < 0.0 )
        {
            ProbSameSignal = 0.0;
        }

        // Detect whether the lock dot should be shown

        if( ProbSameSignal > 0.001 )
        {   
            *pDoShowDot = TRUE;
        }
        else
        {
            *pDoShowDot = FALSE;
        }

        // Bias toward the more reliable signals, since we can just ignore the lousy ones.
        // Paradoxically, this means poor signals reduce the reliability less than mediocre signals do.  This is
        // desirable behavior because the worse the signal, the more we want to hold onto the older estimate.

        if( sReliability > SignalStrength/RELIABILITY_DECAY )
        {
            sReliability = sReliability*ProbSameSignal +
                (1.0 - ProbSameSignal)*sReliability*((sReliability)/(SignalStrength+sReliability)) +
                (1.0 - ProbSameSignal)*(SignalStrength/RELIABILITY_DECAY)*(SignalStrength)/(sReliability+SignalStrength);
        }
        else
        {
            sReliability = sReliability*((sReliability)/(SignalStrength+sReliability)) +
                (SignalStrength/RELIABILITY_DECAY)*(SignalStrength)/(sReliability+SignalStrength);
        }
        // Show reliability estimate
        if( gIndicator == TRUE )
        {
            DWORD  Jindex = (DWORD) (sReliability * pInfo->FrameWidth);
            *(pInfo->PictureHistory[0]->pData + (10 * pInfo->InputPitch) + Jindex*sizeof(WORD)) = 0xFF;
        }
    }

    // Show MaxNoise point -- This is often off the bottom of the screen
    if( ((LONG)(MaxNoise*(gDecayCoefficient+1))/(100*HISTOGRAM_SCALE) < pInfo->FieldHeight - 20) && (gIndicator == TRUE) )
    {
        *(pInfo->PictureHistory[0]->pData + ((((MaxNoise*(gDecayCoefficient+1))/(100*HISTOGRAM_SCALE)) + 20) * pInfo->InputPitch) + 600) = 0xFF;
    }
}