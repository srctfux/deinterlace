/////////////////////////////////////////////////////////////////////////////
// FD_60Hz.c
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock. All rights reserved.
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
// Refinements made by Mark Rejhon and Steve Grimm
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Original Release
//
// 17 Sep 2000   Mark Rejhon           Implemented Steve Grimm's changes
//                                     Some cleanup done.
//                                     Made refinements to Steve Grimm's changes
//
// 07 Jan 2001   John Adcock           Split code that did adaptive method
//                                     out of UpdateNTSCPulldownMode
//                                     Added gNTSCFilmFallbackMode setting
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 09 Jan 2001   John Adcock           Split out into new file
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OutThreads.h"
#include "FD_60Hz.h"
#include "FD_Common.h"
#include "DebugLog.h"

// Settings
// Default values which can be overwritten by the INI file
long gNTSCFilmFallbackIndex = INDEX_ADAPTIVE;
long Threshold32Pulldown = 15;
long ThresholdPulldownMismatch = 100;
long ThresholdPulldownComb = 150;
BOOL bFallbackToVideo = TRUE;
long PulldownRepeatCount = 4;
long PulldownRepeatCount2 = 2;
long PulldownSwitchMax = 4;
long PulldownSwitchInterval = 3000;

// Module wide declarations
long NextPulldownRepeatCount = 0;    // for temporary increases of PullDownRepeatCount
DWORD ModeSwitchTimestamps[MAXMODESWITCHES];
DEINTERLACE_METHOD* ModeSwitchMethods[MAXMODESWITCHES];
int NumSwitches;

BOOL DidWeExpectFieldMatch(DEINTERLACE_INFO *pInfo);

// new stuff
long NoiseThreshold = 150;


///////////////////////////////////////////////////////////////////////////////
// ResetModeSwitches
//
// Resets the memory used by TrackModeSwitches
///////////////////////////////////////////////////////////////////////////////
void ResetModeSwitches()
{
	memset(&ModeSwitchTimestamps[0], 0, sizeof(ModeSwitchTimestamps));
	memset(&ModeSwitchMethods[0], 0, sizeof(ModeSwitchMethods));
	NumSwitches = 0;
}

///////////////////////////////////////////////////////////////////////////////
// TrackModeSwitches
//
// Called whenever we switch to a new film mode.  Keeps track of the frequency
// of mode switches; if we switch too often, we want to drop down to video
// mode since it means we're having trouble locking onto a particular film
// mode.
//
// The settings PulldownSwitchInterval and PulldownSwitchMax control the
// sensitivity of this algorithm.  To trigger video mode there need to be
// PulldownSwitchMax mode switches in PulldownSwitchInterval milliseconds.
///////////////////////////////////////////////////////////////////////////////
BOOL TrackModeSwitches()
{
	if(IsFilmMode() &&
		GetCurrentDeintMethod() != ModeSwitchMethods[0])
	{
		// Scroll the list of timestamps.  Most recent is first in the list.
		memmove(&ModeSwitchTimestamps[1], &ModeSwitchTimestamps[0], sizeof(ModeSwitchTimestamps) - sizeof(DWORD));
		memmove(&ModeSwitchMethods[1], &ModeSwitchMethods[0], sizeof(ModeSwitchMethods) - sizeof(DEINTERLACE_METHOD*));
		ModeSwitchTimestamps[0] = GetTickCount();
		ModeSwitchMethods[0] = GetCurrentDeintMethod();
		if(NumSwitches < MAXMODESWITCHES)
		{
			NumSwitches++;
		}
	
		if (PulldownSwitchMax > 1 && PulldownSwitchInterval > 0 &&	// if the user wants to track switches
			PulldownSwitchMax <= NumSwitches &&
			ModeSwitchTimestamps[PulldownSwitchMax - 1] > 0)		// and there have been enough of them
		{
			int ticks = ModeSwitchTimestamps[0] - ModeSwitchTimestamps[PulldownSwitchMax - 1];
			if (ticks <= PulldownSwitchInterval)
				return TRUE;
		}
	}

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// UpdateNTSCPulldownMode
//
// This gem is where our software competes with the big boys - you know
// the expensive settop scaler boxes that detect 3:2 pulldown mode!
//
// Original Programmer: JohnAd 
// Other maintainers: Mark Rejhon and Steve Grimm
//
// His attempt to implement Mark Rejhon's 3:2 pulldown code.
// This is an advanced descendant of Mark Rejhon's 3:2 algorithm designed 
// completely from scratch on paper in May 1999 at the following URL:
// http://www.avsforum.com/ubb/Forum12/HTML/000071.html
//
// There are numerous refinements contributed by Mark Rejhon and Steve Grimm.
// Mark Rejhon can be reached at dtv@marky.com
// Discussion forum is http://www.avsforum.com - AVSCIENCE HTPC Forum
//
// The algorithm and comments below are taken from Mark's post to the AVSCIENCE
// Home Theater Computer Forum reproduced in the file 32Spec.htm that should be
// with this source.  The key to getting this to work will be choosing the right 
// value for the Threshold32Pulldown variable and others.  This should probably 
// vary depending on the input source, higher for video lower for TV and cable 
// and very low for laserdisk or DVD.
//
// In addition, we have a sanity check for poorly-transfered material.  If a
// field that will be woven with the previous field is dramatically different
// than the field before the previous one, we check the comb factor of the
// woven-together frame.  If it's too high, we immediately switch to video
// deinterlace mode to prevent weave artifacts from appearing.
//
// This function normally gets called 60 times per second.
///////////////////////////////////////////////////////////////////////////////
void UpdateNTSCPulldownMode(DEINTERLACE_INFO *pInfo)
{
	boolean SwitchToVideo = FALSE;
	static long MISMATCH_COUNT = 0;
	static long MOVIE_FIELD_CYCLE = 0;
	static long MOVIE_VERIFY_CYCLE = 0;
	static long MATCH_COUNT = 0;
	static DEINTERLACE_METHOD* OldPulldownMethod = NULL;

    // temporary additions to try
    // and see if we can do a better job with
    // bad pulldown material
    static long MovingAverage = 1;
    static long LastCombFactor = 1;

	// Call with pInfo == NULL is an initialization call.
	// This resets static variables when we start the thread each time.
	if(pInfo == NULL)
	{
        MOVIE_VERIFY_CYCLE = 0;
        MOVIE_FIELD_CYCLE = 0;
		MISMATCH_COUNT = 0;
		MATCH_COUNT = 0;
		ResetModeSwitches();
        MovingAverage = 1;
		return;
	}

	// If the field difference is bigger than the threshold, then
	// the current field is very different from the field two fields ago.
	// Threshold32Pulldown probably should be changed to be automatically
	// compensating depending on the material.
	
	CompareFields(pInfo);

    // if we are not very small compared to the moving average and
    // we are greater than a fixed limit
    // then we have not found a pulldown match
    if(pInfo->FieldDiff * 100 / MovingAverage > Threshold32Pulldown ||
        pInfo->FieldDiff > NoiseThreshold)
	{
		MATCH_COUNT = 0;
        if(IsFilmMode())
        {
            // if we are in film mode then we need to decide if we should stay in film
            // or switch back to video or to another mode
            if (MISMATCH_COUNT > PulldownRepeatCount2 * 5)
		    {
			    // There have been no duplicate fields lately.
			    // It's probably video source.
			    //
			    // MAX_MISMATCH should be a reasonably high value so
			    // that we do not have an oversensitive hair-trigger
			    // in switching to video source everytime there is
			    // video noise or a single spurious field added/dropped
			    // during a movie causing mis-synchronization problems. 
			    SwitchToVideo = TRUE;
		    }

		    // If we're in a film mode and an incoming field would cause
		    // weave artifacts, optionally switch to video mode but make
		    // it very easy to get back into film mode in case this was
		    // just a glitchy scene change.
		    if (bFallbackToVideo)
            {
                // only do video-force check if there's a threshold.
                // only force video if this field is very different
                // and we would weave it with the previous field
                // and it'd produce artifacts
                if(ThresholdPulldownMismatch > 0 &&
			        pInfo->FieldDiff >= ThresholdPulldownMismatch &&
			        DoWeWantToFlip(pInfo) &&
			        GetCombFactor(pInfo) > ThresholdPulldownComb) 
		        {
			        NextPulldownRepeatCount = 1;
			        SetVideoDeinterlaceIndex(gNTSCFilmFallbackIndex);
			        MOVIE_VERIFY_CYCLE = 0;
			        MOVIE_FIELD_CYCLE = 0;
			        LOG(" Back to Video, comb factor %d", pInfo->CombFactor);
		        }
                else
                {
    			    MISMATCH_COUNT++;
                }
            }
            else
            {
                // So the user has requested that we stay in film mode
                // where possible as so we need to check that the current
                // film mode is still sensible
                // we do this by confiming the the combing isn't going to go mad
                // if we weave according to the current mode
                // we first check for movement 
                if(ThresholdPulldownMismatch > 0 &&
			        pInfo->FieldDiff >= ThresholdPulldownMismatch &&
			        DoWeWantToFlip(pInfo) &&
			        GetCombFactor(pInfo) > ThresholdPulldownComb) 
		        {
			        NextPulldownRepeatCount = 1;
                    // Reset the paramters of the Comb method
			        FilmModeNTSCComb(NULL);
                    SetFilmDeinterlaceMode(FILM_32_PULLDOWN_COMB);
			        MOVIE_VERIFY_CYCLE = 0;
			        MOVIE_FIELD_CYCLE = 0;
			        LOG(" Gone to Comb Method, comb factor %d", pInfo->CombFactor);
                }
                else
                {
        			MISMATCH_COUNT++;
                }
            }
        }
        else
        {
			MISMATCH_COUNT++;
        }
	}
    else
	{
		MATCH_COUNT++;

		// It's either a stationary image OR a duplicate field in a movie
		if(MISMATCH_COUNT == 4)
		{
			// 3:2 pulldown is a cycle of 5 fields where there is only
			// one duplicate field pair, and 4 mismatching pairs.
			// We need to continue detection for at least PulldownRepeatCount
			// cycles to be very certain that it is actually 3:2 pulldown.
			// For a repeat count of 2, this would mean a latency of 10
			// fields.
			//
			// If NextPulldownRepeatCount is nonzero, it's a temporary
			// repeat count setting attempting to compensate for some kind
			// of anomaly in the sequence of fields, so use it instead.
			if(NextPulldownRepeatCount > 0 && MOVIE_VERIFY_CYCLE >= NextPulldownRepeatCount ||
			   NextPulldownRepeatCount == 0 && MOVIE_VERIFY_CYCLE >= PulldownRepeatCount)
			{
				// If the pulldown repeat count was temporarily changed, get
				// rid of the temporary setting.
				NextPulldownRepeatCount = 0;

				// This executes regardless whether we've just entered or
				// if we're *already* in 3:2 pulldown. Either way, we are
				// currently now (re)synchronized to 3:2 pulldown and that
				// we've now detected the duplicate field.
				//
				if(pInfo->IsOdd == TRUE)
				{
					switch(pInfo->CurrentFrame)
					{
					case 0:  SetFilmDeinterlaceMode(FILM_32_PULLDOWN_2);  break;
					case 1:  SetFilmDeinterlaceMode(FILM_32_PULLDOWN_3);  break;
					case 2:  SetFilmDeinterlaceMode(FILM_32_PULLDOWN_4);  break;
					case 3:  SetFilmDeinterlaceMode(FILM_32_PULLDOWN_0);  break;
					case 4:  SetFilmDeinterlaceMode(FILM_32_PULLDOWN_1);  break;
					}
					LOG("Gone to film mode %d", (pInfo->CurrentFrame + 2) % 5); 
				}
				else
				{
					switch(pInfo->CurrentFrame)
					{
					case 0:  SetFilmDeinterlaceMode(FILM_32_PULLDOWN_4);  break;
					case 1:  SetFilmDeinterlaceMode(FILM_32_PULLDOWN_0);  break;
					case 2:  SetFilmDeinterlaceMode(FILM_32_PULLDOWN_1);  break;
					case 3:  SetFilmDeinterlaceMode(FILM_32_PULLDOWN_2);  break;
					case 4:  SetFilmDeinterlaceMode(FILM_32_PULLDOWN_3);  break;
					}
					LOG("Gone to film mode %d", (pInfo->CurrentFrame + 4) % 5); 
				}

				if (OldPulldownMethod != GetCurrentDeintMethod())
				{
					// A mode switch.  If we've done a lot of them recently,
					// force video mode since it means we're having trouble
					// locking onto a reliable film mode.   
					// 
					// This stuff happens during these situations
					// - Time compressed movies, which ruins 3:2 sequence 
					// - Poorly telecined movies
					// - Erratic framerates during transfers of old movies
					// - TV commercials that vary framerates
					// - Cartoons using nonstandard pulldown
					// - Super-noisy video
					//
					// Therefore, we should switch to video mode and ignore
					// switching back to movie mode for at least a certain
					// amount of time.
					//
					// This is only triggered on switches between different
					// film modes, not switches between video and film mode.
					// Since we can drop down to video if we're not sure
					// we should stay in pulldown mode, we don't want to
					// bail out of film mode if we subsequently decide that
					// the film mode we just dropped out of was correct.
					if (bFallbackToVideo &&
						TrackModeSwitches())
					{
						SetVideoDeinterlaceIndex(gNTSCFilmFallbackIndex);
						MOVIE_VERIFY_CYCLE = 0;
						MOVIE_FIELD_CYCLE = 0;
						LOG(" Too much film mode cycling, switching to video");
						
						// Require pulldown mode to be consistent for the
						// rapid-mode-switch interval before we'll lock onto
						// film mode again.  This is probably too long in most
						// cases, but making it shorter than the interval would
						// run the risk of switching back to film mode just in
						// time to hit a rapid sequence of mode changes.
						//
						// 83 is (5 fields/cycle) / (60 fields/sec) * (1000 ms/sec).
						//
						// FIXME: Eliminate hardcoded values
						//
						//NextPulldownRepeatCount = PulldownSwitchInterval / 83;
						NextPulldownRepeatCount = PulldownRepeatCount * 2;
					}
				}

				OldPulldownMethod = GetCurrentDeintMethod();
			}
			else
			{
				// We've detected possible 3:2 pulldown.  However, we need
				// to keep watching the 3:2 pulldown for at least a few 5-field
				// cycles before jumping to conclusion that it's really 3:2
				// pulldown and not a false alarm
				//
				MOVIE_VERIFY_CYCLE++;
				LOG(" Found Pulldown Match");
			}
		}
		else
		{
			// If we've just seen rapid-fire pulldown mode switches,
			// require all the duplicate fields to be at the same point
			// in the pulldown sequence.  Getting here means a duplicate
			// field happened in a different place than in the previous
			// cycle, so reset the count of pulldown matches.
			//
			// As noted below, this will mean that it takes a long time to
			// switch back to film mode if we hit a bunch of still frames
			// after a rapid-fire mode switch sequence.
			if (NextPulldownRepeatCount > 0) MOVIE_VERIFY_CYCLE = 0;

			// Normally,
			// If execution point hits here, it is probably just
			// stationary video. That would produce lots of duplicate
			// field pairs. We can't determine whether it's video or
			// movie source. Therefore, we want to keep doing the
			// current algorithm from a prior detection. Therefore,
			// don't modify any variables EXCEPT the MISMATCH_COUNT
			// variable which will be reset to 0 below.
			// Also, occasionally we'll hit here during synchronization
			// problems during a movie, such as a spurious field added
			// or dropped. Reset MISMATCH_COUNT to 0 below and let
			// detection cycle happen again in order to re-synchronize.
			//
		}
		MISMATCH_COUNT = 0;
	}

    // update moving average
    if(MovingAverage > 1)
    {
        MovingAverage = MovingAverage / 2 + pInfo->FieldDiff / 2;
    }
    else
    {
        // make sure we don't ever get divide by zero errors
        MovingAverage = 1;
    }
    LastCombFactor = pInfo->CombFactor;
}


BOOL FilmModeNTSC1st(DEINTERLACE_INFO *pInfo)
{
	BOOL bFlipNow;
	// Film mode.  If we have an entire new frame, display it.
	switch(pInfo->CurrentFrame)
	{
	case 0:  bFlipNow = FALSE;         break;
	case 1:  bFlipNow = !pInfo->IsOdd;  break;
	case 2:  bFlipNow = !pInfo->IsOdd;  break;
	case 3:  bFlipNow = pInfo->IsOdd;   break;
	case 4:  bFlipNow = pInfo->IsOdd;   break;
	}
	if (bFlipNow)
		Weave(pInfo);
	return bFlipNow;
}

BOOL FilmModeNTSC2nd(DEINTERLACE_INFO *pInfo)
{
	BOOL bFlipNow;
	// Film mode.  If we have an entire new frame, display it.
	switch(pInfo->CurrentFrame)
	{
	case 0:  bFlipNow = pInfo->IsOdd;   break;
	case 1:  bFlipNow = FALSE;         break;
	case 2:  bFlipNow = !pInfo->IsOdd;  break;
	case 3:  bFlipNow = !pInfo->IsOdd;  break;
	case 4:  bFlipNow = pInfo->IsOdd;   break;
	}
	if (bFlipNow)
		Weave(pInfo);
	return bFlipNow;
}

BOOL FilmModeNTSC3rd(DEINTERLACE_INFO *pInfo)
{
	BOOL bFlipNow;
	// Film mode.  If we have an entire new frame, display it.
	switch(pInfo->CurrentFrame)
	{
	case 0:  bFlipNow = pInfo->IsOdd;   break;
	case 1:  bFlipNow = pInfo->IsOdd;   break;
	case 2:  bFlipNow = FALSE;         break;
	case 3:  bFlipNow = !pInfo->IsOdd;  break;
	case 4:  bFlipNow = !pInfo->IsOdd;  break;
	}
	if (bFlipNow)
		Weave(pInfo);
	return bFlipNow;
}

BOOL FilmModeNTSC4th(DEINTERLACE_INFO *pInfo)
{
	BOOL bFlipNow;
	// Film mode.  If we have an entire new frame, display it.
	switch(pInfo->CurrentFrame)
	{
	case 0:  bFlipNow = !pInfo->IsOdd;  break;
	case 1:  bFlipNow = pInfo->IsOdd;   break;
	case 2:  bFlipNow = pInfo->IsOdd;   break;
	case 3:  bFlipNow = FALSE;         break;
	case 4:  bFlipNow = !pInfo->IsOdd;  break;
	}
	if (bFlipNow)
		Weave(pInfo);
	return bFlipNow;
}

BOOL FilmModeNTSC5th(DEINTERLACE_INFO *pInfo)
{
	BOOL bFlipNow;
	// Film mode.  If we have an entire new frame, display it.
	switch(pInfo->CurrentFrame)
	{
	case 0:  bFlipNow = !pInfo->IsOdd;  break;
	case 1:  bFlipNow = !pInfo->IsOdd;  break;
	case 2:  bFlipNow = pInfo->IsOdd;   break;
	case 3:  bFlipNow = pInfo->IsOdd;   break;
	case 4:  bFlipNow = FALSE;         break;
	}
	if (bFlipNow)
		Weave(pInfo);
	return bFlipNow;
}

BOOL FilmModeNTSCComb(DEINTERLACE_INFO *pInfo)
{
    static long LastComb = 0;
    static long NumSkipped = 0;
    static long NumVideo = 0;
    if(pInfo == NULL)
    {
        LastComb = 0;
        NumSkipped = 0;
    }

    // if we can weave these frames together without too
    // much weaving then go ahead
    if(pInfo->CombFactor < LastComb &&
        pInfo->CombFactor < ThresholdPulldownComb)
    {
        NumSkipped = 0;
        LOG(" Weaved in Comb mode");
		return Weave(pInfo);
    }
    else
    {
        // otherwise we must keep track of how long it's been
        // since we flipped and every so often throw in a video
        // deinterlaced frame so that we don't freeze up
        ++NumSkipped;
        if(NumSkipped > 2)
        {
            LOG(" Had too many skipped frames, had to use video method");
            ++NumVideo;
            if(NumVideo > 2)
            {
                SetVideoDeinterlaceIndex(gNTSCFilmFallbackIndex);
                LOG(" Gone back to video from Comb");
            }
			return GetVideoDeintMethod(gNTSCFilmFallbackIndex)->pfnAlgorithm(pInfo);
        }
        else
        {
            return FALSE;
        }
    }
}

BOOL DidWeExpectFieldMatch(DEINTERLACE_INFO *pInfo)
{
	BOOL RetVal;
	switch(GetFilmMode())
	{
	case FILM_32_PULLDOWN_0:
		switch(pInfo->CurrentFrame)
		{
		case 0:  RetVal = FALSE;          break;
		case 1:  RetVal = !pInfo->IsOdd;  break;
		case 2:  RetVal = FALSE;          break;
		case 3:  RetVal = pInfo->IsOdd;   break;
		case 4:  RetVal = FALSE;          break;
		}
		break;
	case FILM_32_PULLDOWN_1:
		switch(pInfo->CurrentFrame)
		{
		case 0:  RetVal = pInfo->IsOdd;   break;
		case 1:  RetVal = FALSE;         break;
		case 2:  RetVal = !pInfo->IsOdd;  break;
		case 3:  RetVal = !pInfo->IsOdd;  break;
		case 4:  RetVal = pInfo->IsOdd;   break;
		}
		break;
	case FILM_32_PULLDOWN_2:
		switch(pInfo->CurrentFrame)
		{
		case 0:  RetVal = pInfo->IsOdd;   break;
		case 1:  RetVal = FALSE;          break;
		case 2:  RetVal = FALSE;          break;
		case 3:  RetVal = !pInfo->IsOdd;  break;
		case 4:  RetVal = FALSE;          break;
		}
		break;
	case FILM_32_PULLDOWN_3:
		switch(pInfo->CurrentFrame)
		{
		case 0:  RetVal = FALSE;          break;
		case 1:  RetVal = pInfo->IsOdd;   break;
		case 2:  RetVal = FALSE;          break;
		case 3:  RetVal = FALSE;          break;
		case 4:  RetVal = !pInfo->IsOdd;  break;
		}
		break;
	case FILM_32_PULLDOWN_4:
		switch(pInfo->CurrentFrame)
		{
		case 0:  RetVal = !pInfo->IsOdd;  break;
		case 1:  RetVal = FALSE;          break;
		case 2:  RetVal = pInfo->IsOdd;   break;
		case 3:  RetVal = FALSE;          break;
		case 4:  RetVal = FALSE;          break;
		}
		break;
	default:
		RetVal = FALSE;
		break;
	}
	return RetVal;
}

BOOL DoWeWantToFlip(DEINTERLACE_INFO *pInfo)
{
	BOOL RetVal;
	switch(GetFilmMode())
	{
	case FILM_32_PULLDOWN_0:
		switch(pInfo->CurrentFrame)
		{
	    case 0:  RetVal = FALSE;         break;
	    case 1:  RetVal = !pInfo->IsOdd;  break;
	    case 2:  RetVal = !pInfo->IsOdd;  break;
	    case 3:  RetVal = pInfo->IsOdd;   break;
	    case 4:  RetVal = pInfo->IsOdd;   break;
		}
		break;
	case FILM_32_PULLDOWN_1:
		switch(pInfo->CurrentFrame)
		{
	    case 0:  RetVal = pInfo->IsOdd;   break;
	    case 1:  RetVal = FALSE;         break;
	    case 2:  RetVal = !pInfo->IsOdd;  break;
	    case 3:  RetVal = !pInfo->IsOdd;  break;
	    case 4:  RetVal = pInfo->IsOdd;   break;
		}
		break;
	case FILM_32_PULLDOWN_2:
		switch(pInfo->CurrentFrame)
		{
	    case 0:  RetVal = pInfo->IsOdd;   break;
	    case 1:  RetVal = pInfo->IsOdd;   break;
	    case 2:  RetVal = FALSE;         break;
	    case 3:  RetVal = !pInfo->IsOdd;  break;
	    case 4:  RetVal = !pInfo->IsOdd;  break;
		}
		break;
	case FILM_32_PULLDOWN_3:
		switch(pInfo->CurrentFrame)
		{
	    case 0:  RetVal = !pInfo->IsOdd;  break;
	    case 1:  RetVal = pInfo->IsOdd;   break;
	    case 2:  RetVal = pInfo->IsOdd;   break;
	    case 3:  RetVal = FALSE;         break;
        case 4:  RetVal = !pInfo->IsOdd;  break;
		}
		break;
	case FILM_32_PULLDOWN_4:
		switch(pInfo->CurrentFrame)
		{
        case 0:  RetVal = !pInfo->IsOdd;  break;
	    case 1:  RetVal = !pInfo->IsOdd;  break;
	    case 2:  RetVal = pInfo->IsOdd;   break;
	    case 3:  RetVal = pInfo->IsOdd;   break;
	    case 4:  RetVal = FALSE;         break;
		}
		break;
	default:
		RetVal = FALSE;
		break;
	}
	return RetVal;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING FD60Settings[FD60_SETTING_LASTONE] =
{
	{
		"NTSC Film Fallback Mode", ITEMFROMLIST, 0, (long*)&gNTSCFilmFallbackIndex,
		INDEX_ADAPTIVE, 0, 99, 1, 1,
		NULL,
		"Pulldown", "NTSCFilmFallbackMode", NULL,
	},
	{
		"NTSC Pulldown Repeat Count In", SLIDER, 0, (long*)&PulldownRepeatCount,
		4, 1, 10, 1, 1,
		NULL,
		"Pulldown", "PulldownRepeatCount", NULL,
	},
	{
		"NTSC Pulldown Repeat Count Out", SLIDER, 0, (long*)&PulldownRepeatCount2,
		2, 1, 10, 1, 1,
		NULL,
		"Pulldown", "PulldownRepeatCount2", NULL,
	},
	{
		"Threshold 3:2 Pulldown", SLIDER, 0, (long*)&Threshold32Pulldown,
		15, 1, 5000, 5, 1,
		NULL,
		"Pulldown", "Threshold32Pulldown", NULL,
	},
	{
		"Threshold 3:2 Pulldown Mismatch", SLIDER, 0, (long*)&ThresholdPulldownMismatch,
		100, 1, 10000, 10, 1,
		NULL,
		"Pulldown", "ThresholdPulldownMismatch", NULL,
	},
	{
		"Threshold 3:2 Pulldown Comb", SLIDER, 0, (long*)&ThresholdPulldownComb,
		150, 1, 5000, 10, 1,
		NULL,
		"Pulldown", "ThresholdPulldownComb", NULL,
	},
	{
		"Bad Pulldown Filter", ONOFF, 0, (long*)&bFallbackToVideo,
		TRUE, 0, 1, 1, 1,
		NULL,
		"Pulldown", "bFallbackToVideo", NULL,
	},
	{
		"Pulldown Switch Interval", SLIDER, 0, (long*)&PulldownSwitchInterval,
		3000, 0, 10000, 10, 1,
		NULL,
		"Pulldown", "PulldownSwitchInterval", NULL,

	},
	{
		"Pulldown Switch Max", SLIDER, 0, (long*)&PulldownSwitchMax,
		4, 0, 100, 10, 1,
		NULL,
		"Pulldown", "PulldownSwitchMax", NULL,

	},
};

SETTING* FD60_GetSetting(FD60_SETTING Setting)
{
	if(Setting > -1 && Setting < FD60_SETTING_LASTONE)
	{
		return &(FD60Settings[Setting]);
	}
	else
	{
		return NULL;
	}
}

void FD60_ReadSettingsFromIni()
{
	int i;
	for(i = 0; i < FD60_SETTING_LASTONE; i++)
	{
		Setting_ReadFromIni(&(FD60Settings[i]));
	}
}

void FD60_WriteSettingsToIni()
{
	int i;
	for(i = 0; i < FD60_SETTING_LASTONE; i++)
	{
		Setting_WriteToIni(&(FD60Settings[i]));
	}
}

void FD60_SetMenu(HMENU hMenu)
{
	CheckMenuItem(hMenu, IDM_FALLBACK, bFallbackToVideo?MF_CHECKED:MF_UNCHECKED);
}
