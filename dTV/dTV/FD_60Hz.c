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
#define DOLOGGING
#include "DebugLog.h"

// Settings
// Default values which can be overwritten by the INI file
ePULLDOWNMODES      gNTSCFilmFallbackMode = ADAPTIVE;
long                Threshold32Pulldown = 15;
long                ThresholdPulldownMismatch = 900;
long                ThresholdPulldownComb = 150;
BOOL                bAutoDetectMode = TRUE;
BOOL                bFallbackToVideo = TRUE;

// Module wide declarations
long				NextPulldownRepeatCount = 0;    // for temporary increases of PullDownRepeatCount


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
	static ePULLDOWNMODES OldPulldownMode = VIDEO_MODE_BOB;

	// Call with pInfo == NULL is an initialization call.
	// This resets static variables when we start the thread each time.
	if(pInfo == NULL)
	{
        MOVIE_VERIFY_CYCLE = 0;
        MOVIE_FIELD_CYCLE = 0;
		MISMATCH_COUNT = 0;
		MATCH_COUNT = 0;
		UpdatePulldownStatus();
		dwLastFlipTicks = -1;
		ResetModeSwitches();
		return;
	}


	// If the field difference is bigger than the threshold, then
	// the current field is very different from the field two fields ago.
	// Threshold32Pulldown probably should be changed to be automatically
	// compensating depending on the material.
	
	CompareFields(pInfo);
    if(pInfo->FieldDiff > Threshold32Pulldown)
	{
		MATCH_COUNT = 0;

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
		if (DeintMethods[gPulldownMode].bIsFilmMode &&
			bFallbackToVideo &&							// let the user turn this on and off.
			ThresholdPulldownMismatch > 0 &&		    // only do video-force check if there's a threshold.
			pInfo->FieldDiff >= ThresholdPulldownMismatch &&	// only force video if this field is very different,
			DoWeWantToFlipNTSC(pInfo) &&				// and we would weave it with the previous field,
			GetCombFactor(pInfo) > ThresholdPulldownComb) // and it'd produce artifacts
		{
			SwitchToVideo = TRUE;
			NextPulldownRepeatCount = 1;
		}

		if (SwitchToVideo)
		{
			gPulldownMode = gNTSCFilmFallbackMode;
			MOVIE_VERIFY_CYCLE = 0;
			MOVIE_FIELD_CYCLE = 0;
			UpdatePulldownStatus();
			LOG(" Back to Video, comb factor %d", pInfo->CombFactor);
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
					case 0:  gPulldownMode = FILM_32_PULLDOWN_2;  break;
					case 1:  gPulldownMode = FILM_32_PULLDOWN_3;  break;
					case 2:  gPulldownMode = FILM_32_PULLDOWN_4;  break;
					case 3:  gPulldownMode = FILM_32_PULLDOWN_0;  break;
					case 4:  gPulldownMode = FILM_32_PULLDOWN_1;  break;
					}
					UpdatePulldownStatus();
				}
				else
				{
					switch(pInfo->CurrentFrame)
					{
					case 0:  gPulldownMode = FILM_32_PULLDOWN_4;  break;
					case 1:  gPulldownMode = FILM_32_PULLDOWN_0;  break;
					case 2:  gPulldownMode = FILM_32_PULLDOWN_1;  break;
					case 3:  gPulldownMode = FILM_32_PULLDOWN_2;  break;
					case 4:  gPulldownMode = FILM_32_PULLDOWN_3;  break;
					}
					UpdatePulldownStatus();
				}

				if (OldPulldownMode != gPulldownMode)
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
					if (bFallbackToVideo && TrackModeSwitches(gPulldownMode))
					{
						gPulldownMode = gNTSCFilmFallbackMode;
						MOVIE_VERIFY_CYCLE = 0;
						MOVIE_FIELD_CYCLE = 0;
						UpdatePulldownStatus();
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
						NextPulldownRepeatCount = PulldownSwitchInterval / 83;
					}
				}

				OldPulldownMode = gPulldownMode;
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
}


BOOL FilmModeNTSC(DEINTERLACE_INFO *pInfo)
{
	BOOL bFlipNow;
	// Film mode.  If we have an entire new frame, display it.
	bFlipNow = DoWeWantToFlipNTSC(pInfo);
	if (bFlipNow)
		Weave(pInfo);
	return bFlipNow;
}

///////////////////////////////////////////////////////////////////////////////
// Flip decisioning for film modes.
// Returns true if the current field has given us a complete frame.
// If we're still waiting for another field, returns false.
///////////////////////////////////////////////////////////////////////////////
BOOL DoWeWantToFlipNTSC(DEINTERLACE_INFO *pInfo)
{
	BOOL RetVal;
	switch(gPulldownMode)
	{
	case FILM_22_PULLDOWN_ODD:   RetVal = pInfo->IsOdd;  break;
	case FILM_22_PULLDOWN_EVEN:  RetVal = !pInfo->IsOdd;  break;
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
