/////////////////////////////////////////////////////////////////////////////
// OutThread.c
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
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
// Refminements made by Mark Rejhon and Steve Grimm
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 17 Sep 2000   Mark Rejhon           Implemented Steve Grimm's changes
//                                     Some cleanup done.
//                                     Made refinements to Steve Grimm's changes
//
// 24 Jul 2000   John Adcock           Original Release
//                                     Translated most code from German
//                                     Combined Header files
//                                     Cut out all decoding
//                                     Cut out digital hardware stuff
//
// 09 Aug 2000   John Adcock           Changed WaitForNextFrame to use current RISC
//                                     pointer rather than the status flag
//                                     Also changed VBI processing 
//
// 02 Jan 2001   John Adcock           Fixed bug at end of GetCombFactor assember
//                                     Made PAL pulldown detect remember last video
//                                     mode
//                                     Removed bTV plug-in
//                                     Added Scaled BOB method
//
// 05 Jan 2001   John Adcock           First attempt at judder fix
//                                     Added loop to make sure that we are never
//                                     too early for a flip
//                                     Changed default for gPulldownMode to 2 frame
//
// 07 Jan 2001   John Adcock           Added Adaptive deinterlacing method
//                                     Split code that did adaptive method
//                                     out of UpdateNTSCPulldownMode
//                                     Added gNTSCFilmFallbackMode setting
//                                     Fixed PAL detection bug
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OutThreads.h"
#include "other.h"
#include "bt848.h"
#include "VBI_VideoText.h"
#include "vbi.h"
#include "deinterlace.h"
#include "AspectRatio.h"
#include "dtv.h"
#define DOLOGGING
#include "DebugLog.h"
#include "vbi.h"
#include "Settings.h"
#include "Status.h"

// Thread related variables
BOOL                bStopThread = FALSE;
BOOL                bIsPaused = FALSE;
HANDLE              OutThread;

// Dynamically updated variables
ePULLDOWNMODES      gPulldownMode = VIDEO_MODE_2FRAME;
ePULLDOWNMODES      gPALFilmFallbackMode = VIDEO_MODE_2FRAME;
ePULLDOWNMODES      gNTSCFilmFallbackMode = ADAPTIVE;
int                 CurrentFrame=0;
DWORD               dwLastFlipTicks = -1;
DWORD				ModeSwitchTimestamps[MAXMODESWITCHES];
long				NextPulldownRepeatCount = 0;    // for temporary increases of PullDownRepeatCount

// Default values which can be overwritten by the INI file
long                PulldownThresholdLow = -2000;
long                PulldownThresholdHigh = 2000;
long                PulldownRepeatCount = 4;
long                PulldownRepeatCount2 = 2;
long                Threshold32Pulldown = 15;
long                ThresholdPulldownMismatch = 900;
long                ThresholdPulldownComb = 150;
long                PulldownSwitchInterval = 3000;
long                PulldownSwitchMax = 4;
BOOL                bAutoDetectMode = TRUE;
BOOL                bFallbackToVideo = TRUE;

// TRB 10/28/00 changes, parms, and new fields for sync problem fixes
DDSURFACEDESC		ddsd;						// also add a surface descriptor for Lock			
BOOL				RunningLate = FALSE;        // Set when we are not keeping up
HRESULT             FlipResult = 0;             // Need to try again for flip?
BOOL                Wait_For_Flip = TRUE;       // User parm, default=TRUE
BOOL	            DoAccurateFlips = TRUE;     // User parm, default=TRUE
BOOL	            Hurry_When_Late = FALSE;    // " , default=FALSE, skip processing if behind
long				Sleep_Interval = 0;         // " , default=0, how long to wait for BT chip
BOOL bIsOddField = FALSE;

// FIXME: should be able to get of this variable
long OverlayPitch = 0;


///////////////////////////////////////////////////////////////////////////////
void Start_Thread()
{
	DWORD LinkThreadID;

	CurrentFrame = 0;

	// make sure we start with a clean sheet of paper
	Overlay_Clean();

	bStopThread = FALSE;

	OutThread = CreateThread((LPSECURITY_ATTRIBUTES) NULL,	// No security.
							 (DWORD) 0,	                    // Same stack size.
							 YUVOutThread,		            // Thread procedure.
							 NULL,	                        // Parameter.
							 (DWORD) 0,	                    // Start immediatly.
							 (LPDWORD) & LinkThreadID);	    // Thread ID.
}

///////////////////////////////////////////////////////////////////////////////
void Stop_Thread()
{
	DWORD ExitCode;
	int i;
	BOOL Thread_Stopped = FALSE;

	if (OutThread != NULL)
	{
		i = 5;
		SetThreadPriority(OutThread, THREAD_PRIORITY_NORMAL);
		bStopThread = TRUE;
		while(i > 0 && !Thread_Stopped)
		{
			if (GetExitCodeThread(OutThread, &ExitCode) == TRUE)
			{
				if (ExitCode != STILL_ACTIVE)
				{
					Thread_Stopped = TRUE;
				}
			}
			else
			{
				Thread_Stopped = TRUE;
			}
			Sleep(100);
			i--;
		}

		if (Thread_Stopped == FALSE)
		{
			TerminateThread(OutThread, 0);
		}
		Sleep(50);
		CloseHandle(OutThread);
		OutThread = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////
void Start_Capture()
{
	int nFlags = BT848_CAP_CTL_CAPTURE_EVEN | BT848_CAP_CTL_CAPTURE_ODD;
	if (Capture_VBI == TRUE)
	{
		nFlags |= BT848_CAP_CTL_CAPTURE_VBI_EVEN | BT848_CAP_CTL_CAPTURE_VBI_ODD;
	}

	BT848_MaskDataByte(BT848_CAP_CTL, 0, 0x0f);

	BT848_CreateRiscCode(nFlags);
	BT848_MaskDataByte(BT848_CAP_CTL, (BYTE) nFlags, (BYTE) 0x0f);
	BT848_SetDMA(TRUE);

	Start_Thread();
}

///////////////////////////////////////////////////////////////////////////////
void Stop_Capture()
{
	//  Stop The Output Thread
	Stop_Thread();

	// stop capture
	BT848_MaskDataByte(BT848_CAP_CTL, 0, 0x0f);
}

///////////////////////////////////////////////////////////////////////////////
void Reset_Capture()
{
	Stop_Capture();
	Overlay_Clean();
	BT848_ResetHardware();
	BT848_SetGeoSize();
	WorkoutOverlaySize();
	Start_Capture();
}

////////////////////////////////////////////////////////////////////////////////////
// The following function will continually check the position in the RISC code
// until it is  is different from what we already have.
// We know were we are so we set the current field to be the last one
// that has definitely finished.
//
// Added code here to use a user specified parameter for how long to sleep.  Note that
// windows timer tick resolution is really MUCH worse than 1 millesecond.  Who knows 
// what logic W98 really uses?
//
// Note also that sleep(0) just tells the operating system to dispatch some other
// task now if one is ready, not to sleep for zero seconds.  Since I've taken most
// of the unneeded waits out of other processing here Windows will eventually take 
// control away from us anyway, We might as well choose the best time to do it, without
// waiting more than needed. 
//
// Also added code to HurryWhenLate.  This checks if the new field is already here by
// the time we arrive.  If so, assume we are not keeping up with the BT chip and skip
// some later processing.  Skip displaying this field and use the CPU time gained to 
// get back here faster for the next one.  This should help us degrade gracefully on
// slower or heavily loaded systems but use all available time for processing a good
// picture when nothing else is running.  TRB 10/28/00
//
BOOL WaitForNextField(BOOL LastField)
{
	BOOL bIsOddField;
	int OldPos = (CurrentFrame * 2 + LastField + 1) % 10;
	RunningLate = Hurry_When_Late;        // user specified bool parm
	while(OldPos == BT848_GetRISCPosAsInt())
	{
		Sleep(Sleep_Interval);
		RunningLate = FALSE;			// if we waited then we are not late
	}

	switch(BT848_GetRISCPosAsInt())
	{
	case 0: bIsOddField = TRUE;  CurrentFrame = 4; break;
	case 1: bIsOddField = FALSE; CurrentFrame = 0; break;
	case 2: bIsOddField = TRUE;  CurrentFrame = 0; break;
	case 3: bIsOddField = FALSE; CurrentFrame = 1; break;
	case 4: bIsOddField = TRUE;  CurrentFrame = 1; break;
	case 5: bIsOddField = FALSE; CurrentFrame = 2; break;
	case 6: bIsOddField = TRUE;  CurrentFrame = 2; break;
	case 7: bIsOddField = FALSE; CurrentFrame = 3; break;
	case 8: bIsOddField = TRUE;  CurrentFrame = 3; break;
	case 9: bIsOddField = FALSE; CurrentFrame = 4; break;
	}

	return bIsOddField;
}

///////////////////////////////////////////////////////////////////////////////
void SetupProcessorAndThread()
{
	DWORD rc;
	int ProcessorMask;

	ProcessorMask = 1 << (DecodeProcessor);
	rc = SetThreadAffinityMask(GetCurrentThread(), ProcessorMask);
	
	if (ThreadClassId == 0)
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
	else if (ThreadClassId == 1)
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
	else if (ThreadClassId == 2)
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
	else if (ThreadClassId == 3)
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
	else if (ThreadClassId == 4)
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
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
static int TrackModeSwitches()
{
	// Scroll the list of timestamps.  Most recent is first in the list.
	memmove(&ModeSwitchTimestamps[1], &ModeSwitchTimestamps[0], sizeof(ModeSwitchTimestamps) - sizeof(DWORD));
	ModeSwitchTimestamps[0] = GetTickCount();
	
	if (PulldownSwitchMax > 1 && PulldownSwitchInterval > 0 &&	// if the user wants to track switches
		ModeSwitchTimestamps[PulldownSwitchMax - 1] > 0)		// and there have been enough of them
	{
		int ticks = ModeSwitchTimestamps[0] - ModeSwitchTimestamps[PulldownSwitchMax - 1];
		if (ticks <= PulldownSwitchInterval)
			return TRUE;
	}

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// SetDeinterlaceMode
//
// Sets the deinterlace mode as a result of a menu selection.  This turns off
// autodetection, updates the mode indicator, etc.
///////////////////////////////////////////////////////////////////////////////
void SetDeinterlaceMode(int mode)
{
	gPulldownMode = mode;
	UpdatePulldownStatus();
	SetHalfHeight(DeintMethods[mode].bIsHalfHeight);
}

///////////////////////////////////////////////////////////////////////////////
// UpdatePALPulldownMode
//
// This is the 2:2 pulldown detection for PAL.
///////////////////////////////////////////////////////////////////////////////
void UpdatePALPulldownMode(long CombFactor, BOOL IsOddField)
{
	static long LastCombFactor;
	static long RepeatCount;
	static long LastPolarity;
	static long LastDiff;

	// call with CombFactors -1 to reset static variables when we start the thread
	// each time
	if(CombFactor == -1)
	{
		LastCombFactor = 0;
		RepeatCount = 0;
		LastPolarity = -1;
		LastDiff = 0;
		UpdatePulldownStatus();
		dwLastFlipTicks = -1;
		return;
	}
	if(!DeintMethods[gPulldownMode].bIsFilmMode)
	{
		if((CombFactor - LastCombFactor) < PulldownThresholdLow && LastDiff > PulldownThresholdLow)
		{
			if(LastPolarity == IsOddField)
			{
				if(RepeatCount < PulldownRepeatCount)
				{
					RepeatCount++;
					LOG("Upped RepeatCount %d", RepeatCount);
				}
				else
				{
					if(IsOddField == TRUE)
					{
						gPulldownMode = FILM_22_PULLDOWN_ODD;
						UpdatePulldownStatus();
						LOG("Gone to Odd");
					}
					if(IsOddField == FALSE)
					{
						gPulldownMode = FILM_22_PULLDOWN_EVEN;
						UpdatePulldownStatus();
						LOG("Gone to Even");
					}
				}
			}
			else
			{
				LastPolarity = IsOddField;
				RepeatCount = 1;
				LOG("Reset RepeatCount %d", RepeatCount);
			}
		}
	}
	else
	{
		if((CombFactor - LastCombFactor) < PulldownThresholdLow)
		{
			if(LastPolarity != IsOddField)
			{
				if(LastDiff > PulldownThresholdLow)
				{
					RepeatCount--;
					LOG("Downed RepeatCount 1 %d", RepeatCount);
				}
			}
			else
			{
				if(RepeatCount < PulldownRepeatCount)
				{
					RepeatCount++;
					LOG("Upped RepeatCount 1 %d", RepeatCount);
				}
			}
		}
		if((CombFactor - LastCombFactor) > PulldownThresholdHigh && LastDiff > PulldownThresholdLow)
		{
			if(gPulldownMode == FILM_22_PULLDOWN_ODD && IsOddField == TRUE)
			{
				RepeatCount--;
				LOG("Downed RepeatCount 2 %d", RepeatCount);
			}
			if(gPulldownMode == FILM_22_PULLDOWN_EVEN && IsOddField == FALSE)
			{
				RepeatCount--;
				LOG("Downed RepeatCount 2 %d", RepeatCount);
			}
		}
		if(RepeatCount == PulldownRepeatCount - PulldownRepeatCount2)
		{
			gPulldownMode = gPALFilmFallbackMode;
			RepeatCount = 0;
			UpdatePulldownStatus();
			LOG("Back To Video Mode");
			LastPolarity = -1;
		}
	}

	LastDiff = (CombFactor - LastCombFactor);
	LastCombFactor = CombFactor;
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
void UpdateNTSCPulldownMode(long FieldDiff,
							BOOL OnOddField,
							short **EvenField,
							short **OddField)
{
	int CombFactor = 0;
	boolean SwitchToVideo = FALSE;
	static long MISMATCH_COUNT = 0;
	static long MOVIE_FIELD_CYCLE = 0;
	static long MOVIE_VERIFY_CYCLE = 0;
	static long MATCH_COUNT = 0;
	static ePULLDOWNMODES OldPulldownMode = VIDEO_MODE_BOB;

	// Call with FieldDiff -1 is an initialization call.
	// This resets static variables when we start the thread each time.
	// This probably should be done in an .Initialize() member function
	// when we port to C++
	if(FieldDiff == -1)
	{
        MOVIE_VERIFY_CYCLE = 0;
        MOVIE_FIELD_CYCLE = 0;
		MISMATCH_COUNT = 0;
		MATCH_COUNT = 0;
		UpdatePulldownStatus();
		dwLastFlipTicks = -1;
		memset(&ModeSwitchTimestamps[0], 0, sizeof(ModeSwitchTimestamps));
		return;
	}


	// If the field difference is bigger than the threshold, then
	// the current field is very different from the field two fields ago.
	// Threshold32Pulldown probably should be changed to be automatically
	// compensating depending on the material.
	//
    if(FieldDiff > Threshold32Pulldown)
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
			FieldDiff >= ThresholdPulldownMismatch &&	// only force video if this field is very different,
			DoWeWantToFlip(OnOddField) &&				// and we would weave it with the previous field,
			(CombFactor = GetCombFactor(EvenField, OddField, FALSE)) > ThresholdPulldownComb) // and it'd produce artifacts
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
			LOG(" Back to Video, comb factor %d", CombFactor);
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
				if(OnOddField == TRUE)
				{
					switch(CurrentFrame)
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
					switch(CurrentFrame)
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

///////////////////////////////////////////////////////////////////////////////
// Flip decisioning for film modes.
// Returns true if the current field has given us a complete frame.
// If we're still waiting for another field, returns false.
///////////////////////////////////////////////////////////////////////////////
BOOL DoWeWantToFlip(BOOL bIsOddField)
{
	BOOL RetVal;
	switch(gPulldownMode)
	{
	case FILM_22_PULLDOWN_ODD:   RetVal = bIsOddField;  break;
	case FILM_22_PULLDOWN_EVEN:  RetVal = !bIsOddField;  break;
	case FILM_32_PULLDOWN_0:
		switch(CurrentFrame)
		{
		case 0:  RetVal = FALSE;         break;
		case 1:  RetVal = !bIsOddField;  break;
		case 2:  RetVal = !bIsOddField;  break;
		case 3:  RetVal = bIsOddField;   break;
		case 4:  RetVal = bIsOddField;   break;
		}
		break;
	case FILM_32_PULLDOWN_1:
		switch(CurrentFrame)
		{
		case 0:  RetVal = bIsOddField;   break;
		case 1:  RetVal = FALSE;         break;
		case 2:  RetVal = !bIsOddField;  break;
		case 3:  RetVal = !bIsOddField;  break;
		case 4:  RetVal = bIsOddField;   break;
		}
		break;
	case FILM_32_PULLDOWN_2:
		switch(CurrentFrame)
		{
		case 0:  RetVal = bIsOddField;   break;
		case 1:  RetVal = bIsOddField;   break;
		case 2:  RetVal = FALSE;         break;
		case 3:  RetVal = !bIsOddField;  break;
		case 4:  RetVal = !bIsOddField;  break;
		}
		break;
	case FILM_32_PULLDOWN_3:
		switch(CurrentFrame)
		{
		case 0:  RetVal = !bIsOddField;  break;
		case 1:  RetVal = bIsOddField;   break;
		case 2:  RetVal = bIsOddField;   break;
		case 3:  RetVal = FALSE;         break;
		case 4:  RetVal = !bIsOddField;  break;
		}
		break;
	case FILM_32_PULLDOWN_4:
		switch(CurrentFrame)
		{
		case 0:  RetVal = !bIsOddField;  break;
		case 1:  RetVal = !bIsOddField;  break;
		case 2:  RetVal = bIsOddField;   break;
		case 3:  RetVal = bIsOddField;   break;
		case 4:  RetVal = FALSE;         break;
		}
		break;
	default:
		RetVal = FALSE;
		break;
	}
	return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
// Translates a deinterlace mode name to human-readable form.
// Parameter: mode to translate, or -1 to return name of current mode.
char *DeinterlaceModeName(int mode)
{
	if (mode < 0)
		mode = gPulldownMode;

	if(mode < PULLDOWNMODES_LAST_ONE)
		return DeintMethods[mode].szName;

	return "Unknown Pulldown Mode";
}

///////////////////////////////////////////////////////////////////////////////
// Updates the pulldown mode status indicator in the window footer if the mode
// is different than the one currently listed there.
void UpdatePulldownStatus()
{
	static ePULLDOWNMODES lastPulldownMode = PULLDOWNMODES_LAST_ONE;

	if (gPulldownMode != lastPulldownMode)
	{
		StatusBar_ShowText(STATUS_PAL, DeinterlaceModeName(gPulldownMode));
		lastPulldownMode = gPulldownMode;
	}
}

//
// Add a function to Lock the overlay surface and update some info from it.
// We always lock and write to the back buffer.
// Flipping takes care of the proper buffer addresses.
// Some of this info can change each time.  
// We also check to see if we still need to Flip because the
// non-waiting last flip failed.  If so, try it one more time,
// then give up.  Tom Barry 10/26/00
//
BYTE* LockOverlay()
{
	HRESULT ddrval;

	if (FAILED(FlipResult))				// prev flip was busy?
	{
		IDirectDrawSurface_Flip(lpDDOverlay, NULL, DDFLIP_DONOTWAIT);  
		FlipResult = 0;					// but no time to try any more
	}

	memset(&ddsd, 0x00, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddrval = IDirectDrawSurface_Lock(lpDDOverlayBack, NULL, &ddsd, 
		DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL);
	OverlayPitch = ddsd.lPitch;			// Set new pitch, may change
	return ddsd.lpSurface;
}

DWORD WINAPI YUVOutThread(LPVOID lpThreadParameter)
{
	char Text[128];
	int i, j;
	int nLineTarget;
	int nFrame = 0;
	DWORD dwLastSecondTicks;
	long FlipTicks;
	short* ppEvenLines[5][CLEARLINES];
	short* ppOddLines[5][CLEARLINES];
	BYTE* pDest;
	int LastEvenFrame = 0;
	int LastOddFrame = 0;
	int CombNum = 0;
	BOOL bFlipNow = TRUE;
	BOOL bMissedFrame;
	HRESULT ddrval;
	DEINTERLACE_INFO info;
	DWORD FlipFlag;
	ePULLDOWNMODES PrevPulldownMode;
	DWORD RefreshRate;

	BOOL bIsPAL = TVSettings[TVTYPE].Is25fps;

	RefreshRate = GetRefreshRate();

	if (lpDDOverlay == NULL || lpDDOverlayBack == NULL)
	{
		ExitThread(-1);
	}

	// Sets processor Affinity and Thread priority according to menu selection
	SetupProcessorAndThread();

	// Set up 5 sets of pointers to the start of odd and even lines
	for (j = 0; j < 5; j++)
	{
		for (i = 0; i < CurrentY; i += 2)
		{
			ppOddLines[j][i / 2] = (short *) pDisplay[j] + (i + 1) * 1024;
			ppEvenLines[j][i / 2] = (short *) pDisplay[j] + i * 1024;
		}
	}

	// reset the static variables in the detection code
	if (bIsPAL)
		UpdatePALPulldownMode(-1, FALSE);
	else
		UpdateNTSCPulldownMode(-1, FALSE, NULL, NULL);

	memset(&info, 0, sizeof(info));

	// display the current pulldown mode
	UpdatePulldownStatus();
	
	// start the capture off
	BT848_Restart_RISC_Code();

	dwLastSecondTicks = GetTickCount();
	while(!bStopThread)
	{
		info.IsOdd = WaitForNextField(info.IsOdd);
		if(bIsPaused == FALSE)
		{
			info.OverlayPitch = OverlayPitch;
			info.LineLength = CurrentX * 2;
			info.FrameWidth = CurrentX;
			info.FrameHeight = CurrentY;
			info.FieldHeight = CurrentY / 2;

			bMissedFrame = FALSE;
			bFlipNow = FALSE;

			if(info.IsOdd)
			{
				memmove(&info.OddLines[1], &info.OddLines[0], sizeof(info.OddLines) - sizeof(info.OddLines[0]));
				info.OddLines[0] = ppOddLines[CurrentFrame];
				LastOddFrame = CurrentFrame;

				// If we skipped the previous field, note the missing field in the deinterlace
				// info structure and force this field to be bobbed.
				if (LastEvenFrame != CurrentFrame)
				{
					// in film mode in the 60Hz mode
					// we might wait quite a long time after doing a flip
					// on the 2 field part of the 3:2 pulldown
					// we might then get a single frame behind
					// we need to cope with this so we fill the info struct properly
					// rather than dropping a frame
					if(DoAccurateFlips && DeintMethods[gPulldownMode].bIsFilmMode && !bIsPAL &&
						(LastEvenFrame + 1) % 5 == CurrentFrame)
					{
						memmove(&info.EvenLines[1], &info.EvenLines[0], sizeof(info.EvenLines) - sizeof(info.EvenLines[0]));
						info.EvenLines[0] = ppEvenLines[CurrentFrame];
						LastEvenFrame = CurrentFrame;
					}
					else
					{
						memmove(&info.EvenLines[1], &info.EvenLines[0], sizeof(info.EvenLines) - sizeof(info.EvenLines[0]));
						info.EvenLines[0] = NULL;
						bMissedFrame = TRUE;
						nFrame++;
					}
				}
			}
			else
			{
				memmove(&info.EvenLines[1], &info.EvenLines[0], sizeof(info.EvenLines) - sizeof(info.EvenLines[0]));
				info.EvenLines[0] = ppEvenLines[CurrentFrame];
				LastEvenFrame = CurrentFrame;

				// If we skipped the previous field, note the missing field in the deinterlace
				// info structure and force this field to be bobbed.
				if(LastOddFrame != ((CurrentFrame + 4) % 5))
				{
					// in film mode in the 60Hz mode
					// we might wait quite a long time after doing a flip
					// on the 2 field part of the 3:2 pulldown
					// we might then get a single frame behind
					// we need to cope with this so we fill the info struct properly
					// rather than dropping a frame
					if(DoAccurateFlips && DeintMethods[gPulldownMode].bIsFilmMode && !bIsPAL &&
						(LastOddFrame + 2) % 5 == CurrentFrame)
					{
						memmove(&info.OddLines[1], &info.OddLines[0], sizeof(info.OddLines) - sizeof(info.OddLines[0]));
						info.OddLines[0] = ppOddLines[((CurrentFrame + 4) % 5)];
						LastOddFrame = CurrentFrame;
					}
					else
					{
						memmove(&info.OddLines[1], &info.OddLines[0], sizeof(info.OddLines) - sizeof(info.OddLines[0]));
						info.OddLines[0] = NULL;
						bMissedFrame = TRUE;
						nFrame++;
					}
				}
			}

			if(!bMissedFrame)
			{
				if((bAutoDetectMode == TRUE && bIsPAL) || DeintMethods[gPulldownMode].bRequiresCombFactor)
				{
					if(info.IsOdd)
					{
						info.CombFactor = GetCombFactor(info.OddLines[0], info.EvenLines[0], TRUE);
					}
					else
					{
						info.CombFactor = GetCombFactor(info.EvenLines[0], info.OddLines[0], FALSE);
					}
					LOG(" Frame %d %c CF = %d", CurrentFrame, info.IsOdd ? 'O' : 'E', info.CombFactor);
				}

				if((bAutoDetectMode == TRUE && !bIsPAL) || DeintMethods[gPulldownMode].bRequiresFieldDiff)
				{
					RECT source;

					GetSourceRect(&source);

					if (info.IsOdd)
						info.FieldDiff = CompareFields(info.OddLines[1], info.OddLines[0], &source);
					else
						info.FieldDiff = CompareFields(info.EvenLines[1], info.EvenLines[0], &source);
					
					LOG(" Frame %d %c CR = %d", CurrentFrame, info.IsOdd ? 'O' : 'E', info.FieldDiff);
				}
				PrevPulldownMode = gPulldownMode;				
				if(bAutoDetectMode == TRUE && bIsPAL)
				{
					UpdatePALPulldownMode(info.CombFactor, info.IsOdd);
				}

				if(bAutoDetectMode == TRUE && !bIsPAL)
				{
					UpdateNTSCPulldownMode(info.FieldDiff, 
										   info.IsOdd,
										   info.EvenLines[0],
										   info.OddLines[0]);
				}
			}

			if (!RunningLate && Capture_VBI == TRUE)
			{
				BYTE * pVBI = (LPBYTE) pVBILines[CurrentFrame];
				if (info.IsOdd)
				{
					pVBI += CurrentVBILines * 2048;
				}
				for (nLineTarget = 0; nLineTarget < CurrentVBILines ; nLineTarget++)
				{
					VBI_DecodeLine(pVBI + nLineTarget * 2048, nLineTarget);
				}
			}

			if (!RunningLate)
			{
				pDest = LockOverlay();	// Ready to access screen, Lock back buffer berfore accessing
										// can't do this until after Lock Call
				info.Overlay = pDest;
			}

			if (RunningLate)
			{
				RunningLate = RunningLate;
				// do nothing
			}
			// if we have dropped a field then do BOB 
			// if we are doing a half height mode then just do that
			// anyway as it will be just as fast
			else if(bMissedFrame && !DeintMethods[gPulldownMode].bIsHalfHeight)
			{
				bFlipNow = Bob(&info);
			}
			// When we first detect film mode we will be on the right flip mode in PAL
			// and at the end of a three series in NTSC this will be the starting point for
			// our 2.5 field timings
			else if(PrevPulldownMode != gPulldownMode && DeintMethods[gPulldownMode].bIsFilmMode)
			{
				bFlipNow = Weave(&info);
			}
			else
			{
				bFlipNow = DeintMethods[gPulldownMode].pfnAlgorithm(&info);
			}
			
			AdjustAspectRatio(ppEvenLines[LastEvenFrame], ppOddLines[LastOddFrame]);

			// somewhere above we will have locked the buffer, unlock before flip
			if (!RunningLate)
			{
				ddrval = IDirectDrawSurface_Unlock(lpDDOverlayBack, NULL);

				if (bFlipNow)
				{
					// Need to wait for a good time to flip
					// only if we have been in the same mode for at least one flip
					if(DoAccurateFlips && PrevPulldownMode == gPulldownMode && RefreshRate > 0)
					{
						if(bIsPAL)
						{
							if(RefreshRate % DeintMethods[gPulldownMode].FrameRate50Hz == 0)
							{
								while((GetTickCount() - FlipTicks) < (1000 / DeintMethods[gPulldownMode].FrameRate50Hz - 3));
							}
						}
						else
						{
							if(RefreshRate % DeintMethods[gPulldownMode].FrameRate60Hz == 0)
							{
								while((GetTickCount() - FlipTicks) < (1000/DeintMethods[gPulldownMode].FrameRate60Hz - 3));
							}
						}
					}

					// setup flip flag
					// the odd and even flags may help the scaled bob
					// on some cards
					FlipFlag = (Wait_For_Flip)?DDFLIP_WAIT:DDFLIP_DONOTWAIT;
					if(gPulldownMode == SCALER_BOB)
					{
						FlipFlag |= (info.IsOdd)?DDFLIP_ODD:DDFLIP_EVEN;
					}
					FlipResult = IDirectDrawSurface_Flip(lpDDOverlay, NULL, FlipFlag); 
					LOG(" Flip ms (%dms)", timeGetTime() % 1000);
					FlipTicks = GetTickCount();
					if (dwLastFlipTicks > -1 && FlipTicks - dwLastFlipTicks > (1000 / 23))
					{
						// We should always be running at 24fps or greater.  Check for
						// 23fps instead to allow some wiggle room, but if more than
						// 1/23 of a second has passed since the last flip, something's
						// screwy.
						LOG(" Long time since last flip (%dms)", dwLastFlipTicks - FlipTicks);
					}
					dwLastFlipTicks = FlipTicks;
				}
			}
		}

		if (bDisplayStatusBar == TRUE)
		{
			if (dwLastSecondTicks + 1000 < GetTickCount())
			{
				sprintf(Text, "%d DF/S", nFrame);
				StatusBar_ShowText(STATUS_FPS, Text);
				nFrame = 0;
				dwLastSecondTicks = GetTickCount();
			 }
		}
	}

	BT848_SetDMA(FALSE);
	ExitThread(0);
	return 0;
}

BOOL FilmMode(DEINTERLACE_INFO *info)
{
	BOOL bFlipNow;
	// Film mode.  If we have an entire new frame, display it.
	bFlipNow = DoWeWantToFlip(info->IsOdd);
	if (bFlipNow)
		Weave(info);
	return bFlipNow;
}