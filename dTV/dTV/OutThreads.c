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
// 24 Jul 2000   John Adcock           Original Release
//                                     Translated most code from German
//                                     Combined Header files
//                                     Cut out all decoding
//                                     Cut out digital hardware stuff
// 09 Aug 2000   John Adcock           Fixed bug at end of GetCombFactor assember
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OutThreads.h"
#include "other.h"
#include "bt848.h"
#include "bTVPlugin.h"
#include "VBI_VideoText.h"
#include "vbi.h"
#include "deinterlace.h"
#include "AspectRatio.h"
#include "dtv.h"
#define DOLOGGING
#include "DebugLog.h"
#include "vbi.h"

short pPALplusCode[] = {  18,  27,  36,  45,  54,  63,  72,  81,  90, 100, 110, 120, 134, 149};
short pPALplusData[] = { 160, 178, 196, 214, 232, 250, 268, 286, 304, 322, 340, 358, 376, 394};
short nLevelLow      =  45;
short nLevelHigh     = 135;

// Thread related variables
BOOL                bStopThread = FALSE;
BOOL                bIsPaused = FALSE;
HANDLE              OutThread;

// Dynamically updated variables
ePULLDOWNMODES      gPulldownMode = VIDEO_MODE_BOB;
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
long				StaticImageFieldCount = 100;
long				VideoWeaveFieldCount = 4;
BOOL                bAutoDetectMode = TRUE;
BOOL                bFallbackToVideo = TRUE;

// TRB 10/28/00 changes, parms, and new fields for sync problem fixes
BYTE			    * lpCurOverlay;				// made static for Lock rtn, curr vid buff ptr
DDSURFACEDESC		ddsd;						// also add a surface descriptor for Lock			
BOOL				RunningLate = FALSE;        // Set when we are not keeping up
HRESULT             FlipResult = 0;             // Need to try again for flip?
BOOL                Wait_For_Flip = TRUE;       // User parm, default=TRUE
BOOL	            Hurry_When_Late = FALSE;    // " , default=FALSE, skip processing if behind
long				Sleep_Interval = 0;         // " , default=0, how long to wait for BT chip

///////////////////////////////////////////////////////////////////////////////
void Start_Thread()
{
	int i;
	DWORD LinkThreadID;

	CurrentFrame = 0;

	for (i = 0; i < 5; i++)
	{
		pDisplay[i] = Display_dma[i]->dwUser;
	}

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

	// reset intercast settings
	InterCast.esc = 0;
	InterCast.done = 0;
	InterCast.pnum = 0;
	InterCast.ok = 0;
	InterCast.datap = 0;
	InterCast.lastci = 0xff;

	BT848_SetRiscJumpsDecode(nFlags);
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
// The following function will continually check the BT chip's Interupt Status Flag
// (specifying even/odd field processing) until it is  is different from what we
// already have.  We get the buffer (or frame) number from the high 4 bits of that
// register which changes when the frame changes, in a pattern like 1,0,2,0...5,0,1,0..
// In other words, 0 means only an even/odd change (don't remember which).
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
	DWORD stat = BT848_ReadDword(BT848_INT_STAT);
	RunningLate = Hurry_When_Late;        // user specified bool parm
	while(LastField == ((stat & BT848_INT_FIELD) == BT848_INT_FIELD))
	{
		Sleep(Sleep_Interval);
		RunningLate = FALSE;			// if we waited then we are not late
		stat = BT848_ReadDword(BT848_INT_STAT);
	}

	bIsOddField = ((stat & BT848_INT_FIELD) == BT848_INT_FIELD);

	switch(stat >> 28)
	{
	case 1:  CurrentFrame = 0;  break;
	case 2:  CurrentFrame = 1;  break;
	case 3:  CurrentFrame = 2;  break;
	case 4:  CurrentFrame = 3;  break;
	case 5:  CurrentFrame = 4;  break;
	default: break;
	}

	BT848_WriteDword(BT848_INT_STAT, (DWORD) 0x0fffffff);

// taken out for testing may still have to use BT rest to recover from overruns
// Note: I don't think it is necessary for overruns.
// FIXME: We should blank out old video overlay buffers during overruns.
//
/*	if(stat & (BT848_INT_PPERR | BT848_INT_SCERR | BT848_INT_FBUS | BT848_INT_FTRGT | BT848_INT_RIPERR))
	{
		BT848_Restart_RISC_Code();
		CurrentFrame = 0;
		bIsOddField = TRUE;
		UpdatePALPulldownMode(-1, FALSE);
		UpdateNTSCPulldownMode(-1, FALSE);
	}
*/

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
	bAutoDetectMode = FALSE;
	gPulldownMode = mode;
	UpdatePulldownStatus();
	SetHalfHeight(IS_HALF_HEIGHT(mode));
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
		gPulldownMode = VIDEO_MODE_BOB;
		LastDiff = 0;
		UpdatePulldownStatus();
		dwLastFlipTicks = -1;
		return;
	}
	if(IS_VIDEO_MODE(gPulldownMode))
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
			gPulldownMode = VIDEO_MODE_WEAVE;
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
// The alogoritm and comments below are taken from Mark's post to the AVSCIENCE
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
		gPulldownMode = VIDEO_MODE_WEAVE;
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

		if (gPulldownMode == SIMPLE_WEAVE)
		{
			// We're in weave mode and the image is no longer static.  Go
			// back to video mode immediately.
			SwitchToVideo = TRUE;
		}

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

		if (FieldDiff >= ThresholdPulldownMismatch &&
			gPulldownMode == VIDEO_MODE_WEAVE)
		{
			// A big field difference; video-weave mode probably
			// isn't the best choice.
			SwitchToVideo = TRUE;
		}

		// If we're in a film mode and an incoming field would cause
		// weave artifacts, optionally switch to video mode but make
		// it very easy to get back into film mode in case this was
		// just a glitchy scene change.
		if (IS_PULLDOWN_MODE(gPulldownMode) &&
			bFallbackToVideo &&							// let the user turn this on and off.
			ThresholdPulldownMismatch > 0 &&		    // only do video-force check if there's a threshold.
			FieldDiff >= ThresholdPulldownMismatch &&	// only force video if this field is very different,
			DoWeWantToFlip(TRUE, OnOddField) &&			// and we would weave it with the previous field,
			(CombFactor = GetCombFactor(EvenField, OddField)) > ThresholdPulldownComb) // and it'd produce artifacts
		{
			SwitchToVideo = TRUE;
			NextPulldownRepeatCount = 1;
		}

		if (SwitchToVideo)
		{
			// If we're in weave mode, it might be okay to drop to video
			// weave mode.
			if (gPulldownMode == SIMPLE_WEAVE &&
				FieldDiff < ThresholdPulldownMismatch)
			{
				gPulldownMode = VIDEO_MODE_WEAVE;
			}
			else
			{
				gPulldownMode = VIDEO_MODE_BOB;
				MOVIE_VERIFY_CYCLE = 0;
				MOVIE_FIELD_CYCLE = 0;
			}
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
						gPulldownMode = VIDEO_MODE_BOB;
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
			// If we've seen matching fields for a while and we're in
			// video mode, switch to weave mode since we're probably
			// looking at a static image and video mode can introduce
			// flickering.  But don't switch out of film mode when we
			// hit a static image.
		}
		MISMATCH_COUNT = 0;

		if (MATCH_COUNT >= VideoWeaveFieldCount &&
			gPulldownMode == VIDEO_MODE_BOB)
		{
			gPulldownMode = VIDEO_MODE_WEAVE;
			LOG(" Match count %ld, switching to video-weave", MATCH_COUNT);
			UpdatePulldownStatus();
		}
		if (MATCH_COUNT >= StaticImageFieldCount &&
			gPulldownMode == VIDEO_MODE_WEAVE)
		{
			gPulldownMode = SIMPLE_WEAVE;
			LOG(" Match count %ld, switching to weave", MATCH_COUNT);
			UpdatePulldownStatus();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Returns true if the current field has given us a complete frame.
// If we're still waiting for another field, returns false.
///////////////////////////////////////////////////////////////////////////////
BOOL DoWeWantToFlip(BOOL bFlipNow, BOOL bIsOddField)
{
	BOOL RetVal;
	switch(gPulldownMode)
	{
	case VIDEO_MODE_WEAVE:       RetVal = TRUE;  break;
	case VIDEO_MODE_BOB:         RetVal = TRUE;  break;
	case SIMPLE_WEAVE:           RetVal = TRUE;  break;
	case INTERPOLATE_BOB:        RetVal = TRUE;  break;
	case BLENDED_CLIP:			 RetVal = BlcWantsToFlip;  break;
	case BTV_PLUGIN:             RetVal = bFlipNow;  break;
	case FILM_22_PULLDOWN_ODD:   RetVal = bIsOddField;  break;
	case FILM_22_PULLDOWN_EVEN:  RetVal = !bIsOddField;  break;
	case ODD_ONLY:				 RetVal = bIsOddField;  break;
	case EVEN_ONLY:				 RetVal = !bIsOddField;  break;
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
// Updates the pulldown mode status indicator in the window footer if the mode
// is different than the one currently listed there.
void UpdatePulldownStatus()
{
	static ePULLDOWNMODES lastPulldownMode = PULLDOWNMODES_LAST_ONE;

	if (gPulldownMode != lastPulldownMode)
	{
		switch(gPulldownMode)
		{
		case VIDEO_MODE_WEAVE:
			SetWindowText(hwndPalField, "Video Deinterlace (Weave)");
			break;
		case VIDEO_MODE_BOB:
			SetWindowText(hwndPalField, "Video Deinterlace (Bob)");
			break;
		case SIMPLE_WEAVE:
			SetWindowText(hwndPalField, "Simple Weave");
			break;
		case INTERPOLATE_BOB:
			SetWindowText(hwndPalField, "Interpolated BOB");
			break;
		case BLENDED_CLIP:
			SetWindowText(hwndPalField, "Blended Clip");
			break;
		case BTV_PLUGIN:
			SetWindowText(hwndPalField, "Using bTV Plugin");
			break;
		case FILM_22_PULLDOWN_ODD:
			SetWindowText(hwndPalField, "2:2 Pulldown Flip on Odd");
			break;
		case FILM_22_PULLDOWN_EVEN:
			SetWindowText(hwndPalField, "2:2 Pulldown Flip on Even");
			break;
		case FILM_32_PULLDOWN_0:
			SetWindowText(hwndPalField, "3:2 Pulldown Skip 1st Full Frame");
			break;
		case FILM_32_PULLDOWN_1:
			SetWindowText(hwndPalField, "3:2 Pulldown Skip 2nd Full Frame");
			break;
		case FILM_32_PULLDOWN_2:
			SetWindowText(hwndPalField, "3:2 Pulldown Skip 3rd Full Frame");
			break;
		case FILM_32_PULLDOWN_3:
			SetWindowText(hwndPalField, "3:2 Pulldown Skip 4th Full Frame");
			break;
		case FILM_32_PULLDOWN_4:
			SetWindowText(hwndPalField, "3:2 Pulldown Skip 5th Full Frame");
			break;
		case EVEN_ONLY:
			SetWindowText(hwndPalField, "Even Scanlines Only");
			break;
		case ODD_ONLY:
			SetWindowText(hwndPalField, "Odd Scanlines Only");
			break;
		default:
			SetWindowText(hwndPalField, "Unknown Pulldown Mode");
			break;
		}

		lastPulldownMode = gPulldownMode;
	}
}

///////////////////////////////////////////////////////////////////////////////
void Weave(short** pOddLines, short** pEvenLines, BYTE* lpOverlay)
{
	int i;

	for (i = 0; i < CurrentY / 2; i++)
	{
		memcpyMMX(lpOverlay, pEvenLines[i], CurrentX * 2);
		lpOverlay += OverlayPitch;

		memcpyMMX(lpOverlay, pOddLines[i], CurrentX * 2);
		lpOverlay += OverlayPitch;
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
	lpCurOverlay = ddsd.lpSurface;		// Set new address, also changes
	return ddsd.lpSurface;
}



DWORD WINAPI YUVOutThread(LPVOID lpThreadParameter)
{
	char Text[128];
	int i, j;
	int nLineTarget;
	int nFrame = 0;
	DWORD dwLastSecondTicks;
	long CompareResult;
	long CombFactor;
	long FlipTicks;
	short* ppEvenLines[5][CLEARLINES];
	short* ppOddLines[5][CLEARLINES];
	BYTE* pDest;
	int LastEvenFrame = 0;
	int LastOddFrame = 0;
	int CombNum = 0;
	BOOL bFlipNow = TRUE;
	BOOL bIsOddField = FALSE;
	HRESULT ddrval;

	BOOL bIsPAL = TVSettings[TVTYPE].Is25fps;
	lpCurOverlay = lpOverlayBack;		

	if (lpDDOverlay == NULL || lpDDOverlay == NULL || lpOverlayBack == NULL || lpOverlay == NULL)
	{
		ExitThread(-1);
	}

	// Sets processor Affinity and Thread priority according to menu selection
	SetupProcessorAndThread();

	// Set up 5 sets of pointers to the start of odd and even lines
	// we will always go up to the limit so that we can use bTV
	for (j = 0; j < 5; j++)
	{
		for (i = 0; i < BTV_VER1_HEIGHT; i += 2)
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

	// display the current pulldown mode
	UpdatePulldownStatus();
	
	// start the capture off
	BT848_Restart_RISC_Code();

	dwLastSecondTicks = GetTickCount();
	while(!bStopThread)
	{
		bIsOddField = WaitForNextField(bIsOddField);
		if(bIsPaused == FALSE)
		{
			if(bIsOddField)
			{
				if(bAutoDetectMode == TRUE)
				{
					if (bIsPAL)
					{
						CombFactor = GetCombFactor(ppEvenLines[CurrentFrame], ppOddLines[CurrentFrame]);
						UpdatePALPulldownMode(CombFactor, TRUE);
						LOG(" Frame %d O CF = %d", CurrentFrame, CombFactor);
					}
					else
					{
						RECT source;

						GetSourceRect(&source);
						CompareResult = CompareFields(ppOddLines[(CurrentFrame + 4) % 5], ppOddLines[CurrentFrame],
						                              &source);
						LOG(" Frame %d O CR = %d", CurrentFrame, CompareResult);
						UpdateNTSCPulldownMode(CompareResult, 
											   TRUE,
											   ppEvenLines[LastEvenFrame],
											   ppOddLines[CurrentFrame]);
					}
				}

				if (Capture_VBI == TRUE)
				{
					BYTE * pVBI = (LPBYTE) Vbi_dma[CurrentFrame]->dwUser;
					for (nLineTarget = VBI_lpf; nLineTarget < 2 * VBI_lpf ; nLineTarget++)
					{
						VBI_DecodeLine(pVBI + nLineTarget * 2048, nLineTarget - VBI_lpf);
					}
				}

				if (!RunningLate)
				{
					pDest=LockOverlay();	// Ready to access screen, Lock back buffer berfore accessing
											// can't do this until after Lock Call
				}

				if (RunningLate)
				{
					// do nothing
				}
				// if we have dropped a field then do BOB 
				else if(LastEvenFrame != CurrentFrame || gPulldownMode == INTERPOLATE_BOB)
				{
					for (nLineTarget = 0; nLineTarget < CurrentY / 2; nLineTarget++)
					{
						memcpyBOBMMX(pDest,
									ppOddLines[CurrentFrame][nLineTarget], 
									CurrentX * 2);
						pDest += 2 * OverlayPitch;
					}
					bFlipNow = TRUE;
					if(LastEvenFrame != CurrentFrame)
					{
						nFrame++;
					}
				}
				else if(gPulldownMode == VIDEO_MODE_WEAVE)
				{
					DeinterlaceFieldWeave(ppOddLines[CurrentFrame], ppEvenLines[LastEvenFrame],
										  ppOddLines[(CurrentFrame + 4) % 5], lpCurOverlay, TRUE);
				}
				else if (gPulldownMode == VIDEO_MODE_BOB)
				{
					DeinterlaceFieldBob(ppOddLines[CurrentFrame], ppEvenLines[LastEvenFrame],
										ppOddLines[(CurrentFrame + 4) % 5], lpCurOverlay, TRUE);
				}
				else if(gPulldownMode == SIMPLE_WEAVE)
				{
					Weave(ppOddLines[CurrentFrame], ppEvenLines[LastEvenFrame], lpCurOverlay);
				}
				else if(gPulldownMode == BLENDED_CLIP)
				{
					BlendedClipping(ppOddLines[CurrentFrame], ppEvenLines[LastEvenFrame], 
									ppOddLines[(CurrentFrame+4) % 5], lpCurOverlay,
									TRUE);
				}
				else if(gPulldownMode == BTV_PLUGIN)
				{
					BYTE* pDestEven[CLEARLINES];
					BYTE* pDestOdd[CLEARLINES];
					
					// set up desination pointers
					// may need to optimize this later
					for (i = 0; i < BTV_VER1_HEIGHT; i += 2)
					{
						pDestEven[i / 2] = pDest;
						pDest += OverlayPitch;
						pDestOdd[i / 2] = pDest;
						pDest += OverlayPitch;
					}
					
					BTVParams.IsOddField = 1;
					BTVParams.ppCurrentField = ppOddLines[CurrentFrame];
					BTVParams.ppLastField = ppEvenLines[LastEvenFrame];
					BTVParams.ppEvenDest = pDestEven;
					BTVParams.ppOddDest = pDestOdd;
					bFlipNow = BTVPluginDoField(&BTVParams);
				}
				else if (gPulldownMode == ODD_ONLY)
				{
					for (nLineTarget = 0; nLineTarget < CurrentY / 2; nLineTarget++)
					{
						// copy latest field's odd rows to overlay, resulting in a half-height image.
						memcpyBOBMMX(pDest,
									ppOddLines[CurrentFrame][nLineTarget],
									CurrentX * 2);
						pDest += OverlayPitch;
					}
					bFlipNow = TRUE;
				}
				else if (gPulldownMode == EVEN_ONLY)
				{
					// Do nothing.
				}
				else
				{
					// Pulldown mode.  If we have an entire new frame, display it.
					if (DoWeWantToFlip(TRUE, bIsOddField))
						Weave(ppOddLines[CurrentFrame], ppEvenLines[LastEvenFrame], pDest);
				}
				LastOddFrame = CurrentFrame;
			}
			else
			{
				if(bAutoDetectMode == TRUE)
				{
					if (bIsPAL)
					{
						// need to add one to the even lines
						// so that the top line is between the
						// top two even lines
						CombFactor = GetCombFactor(ppEvenLines[CurrentFrame], ppOddLines[(CurrentFrame + 4) % 5]);
						UpdatePALPulldownMode(CombFactor, FALSE);
						LOG(" Frame %d E CF = %d", CurrentFrame, CombFactor);
					}
					else
					{
						RECT source;

						GetSourceRect(&source);
						CompareResult = CompareFields(ppEvenLines[(CurrentFrame + 4) % 5], ppEvenLines[CurrentFrame],
														&source);
						LOG(" Frame %d E CR = %d", CurrentFrame, CompareResult);
						UpdateNTSCPulldownMode(CompareResult,
											   FALSE,
											   ppEvenLines[CurrentFrame],
											   ppOddLines[LastOddFrame]);
					}
				}

				if (Capture_VBI == TRUE)
				{
					BYTE * pVBI = (LPBYTE) Vbi_dma[CurrentFrame]->dwUser;
					for (nLineTarget = 0; nLineTarget < VBI_lpf ; nLineTarget++)
					{
						VBI_DecodeLine(pVBI + nLineTarget * 2048, nLineTarget);
					}
				}

				if (!RunningLate)
				{
					pDest = LockOverlay();	// Ready to access screen, Lock back buffer berfore accessing
											// can't do this until after Lock Call
				}
				
				if (RunningLate)
				{
					// do nothing
				}

				// if we have dropped a field then do BOB
				else if(LastOddFrame != ((CurrentFrame + 4) % 5) || gPulldownMode == INTERPOLATE_BOB)
				{
					for (nLineTarget = 0; nLineTarget < CurrentY / 2; nLineTarget++)
					{
						// copy latest field data to both odd and even rows
						memcpyBOBMMX(pDest, 
									ppEvenLines[CurrentFrame][nLineTarget],
									CurrentX * 2);
						pDest += 2 * OverlayPitch;
					}
					bFlipNow = TRUE;
					if(LastOddFrame != ((CurrentFrame + 4) % 5))
					{
						nFrame++;
					}
				}
				else if(gPulldownMode == VIDEO_MODE_WEAVE)
				{
					DeinterlaceFieldWeave(ppOddLines[LastOddFrame], ppEvenLines[CurrentFrame],
										  ppEvenLines[(CurrentFrame + 4) % 5], lpCurOverlay, FALSE);
				}
				else if(gPulldownMode == VIDEO_MODE_BOB)
				{
					DeinterlaceFieldBob(ppOddLines[LastOddFrame], ppEvenLines[CurrentFrame],
										ppEvenLines[(CurrentFrame + 4) % 5], lpCurOverlay, FALSE);
				}
				else if(gPulldownMode == SIMPLE_WEAVE)
				{
					Weave(ppOddLines[LastOddFrame], ppEvenLines[CurrentFrame], lpCurOverlay);
				}
				else if(gPulldownMode == BLENDED_CLIP)
				{
					BlendedClipping(ppOddLines[LastOddFrame], ppEvenLines[CurrentFrame],
									ppEvenLines[(CurrentFrame+4) % 5], lpCurOverlay,
									FALSE);
				}
				else if(gPulldownMode == BTV_PLUGIN)
				{
					BYTE* pDestEven[CLEARLINES];
					BYTE* pDestOdd[CLEARLINES];
					
					// set up desination pointers
					// may need to optimize this later
					for (i = 0; i < BTV_VER1_HEIGHT; i += 2)
					{
						pDestEven[i / 2] = pDest;
						pDest += OverlayPitch;
						pDestOdd[i / 2] = pDest;
						pDest += OverlayPitch;
					}
					
					BTVParams.IsOddField = 1;
					BTVParams.ppCurrentField = ppOddLines[CurrentFrame];
					BTVParams.ppLastField = ppEvenLines[LastEvenFrame];
					BTVParams.ppEvenDest = pDestEven;
					BTVParams.ppOddDest = pDestOdd;
					bFlipNow = BTVPluginDoField(&BTVParams);
				}
				else if (gPulldownMode == EVEN_ONLY)
				{
					for (nLineTarget = 0; nLineTarget < CurrentY / 2; nLineTarget++)
					{
						// copy latest field's even rows to overlay, resulting in a half-height image.
						memcpyBOBMMX(pDest,
									ppEvenLines[CurrentFrame][nLineTarget],
									CurrentX * 2);
						pDest += OverlayPitch;
					}
					bFlipNow = TRUE;
				}
				else if (gPulldownMode == ODD_ONLY)
				{
					// Do nothing.
				}
				else
				{
					// Pulldown mode.  If we have an entire new frame, display it.
					if (DoWeWantToFlip(TRUE, bIsOddField))
						Weave(ppOddLines[LastOddFrame], ppEvenLines[CurrentFrame], pDest);
				}
				LastEvenFrame = CurrentFrame;
			}

			AdjustAspectRatio(ppEvenLines[LastEvenFrame], ppOddLines[LastOddFrame]);

			// somewhere above we will have locked the buffer, unlock before flip
			if (!RunningLate)
			{
				ddrval = IDirectDrawSurface_Unlock(lpDDOverlayBack, lpCurOverlay);

				if(DoWeWantToFlip(bFlipNow, bIsOddField) )
				{
					if (Wait_For_Flip)			// user parm
					{
						FlipResult =
							IDirectDrawSurface_Flip(lpDDOverlay, NULL, DDFLIP_WAIT); 
					}
					else 
					{
						FlipResult =
							IDirectDrawSurface_Flip(lpDDOverlay, NULL, DDFLIP_DONOTWAIT);   
					}
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
				SetWindowText(hwndFPSField, Text);
				nFrame = 0;
				dwLastSecondTicks = GetTickCount();
			
			 }
		}
	}

	BT848_SetDMA(FALSE);
	ExitThread(0);
	return 0;
}
