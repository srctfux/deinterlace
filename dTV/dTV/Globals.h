/////////////////////////////////////////////////////////////////////////////
// globals.h
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
// ########### IMPORTANT ###########
//
// Globals are ugly and complicates keeping track of code.
// If you see variables that are used by only one module, please 
// delete it from here, and declare the variable at the top of the module 
// and then delete the 'extern' declaration.  Your Computer Science class
// should have taught you to avoid excessive global variables ;-)
// It will also make it easier to rewrite to C++ as well in the future.
//
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Original Release
//                                     Translated most code from German
//                                     Combined Header files
//                                     Cut out all decoding
//                                     Cut out digital hardware stuff
//
// 02 Jan 2001   John Adcock           Added CurrentVBILines and removed BurstDMA
//
// 05 Jan 2001   John Adcock           Added DoAccurateFlips
//
// 08 Jan 2001   John Adcock           Started Global Variable Tidy up
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __GLOBALS_H___
#define __GLOBALS_H___

#include "defines.h"
#include "structs.h"

//------------------------------------------------------------------------
// ######### IMPORTANT! READ ME FIRST! ##########
//
// Only put variables here IF it is necessary to share the same variable
// between multiple modules.  If it is not necessary, please instead
// put your variable declaration at the top of the module you are editing.
// This makes other programmers' lives easier keeping track of variables.
//------------------------------------------------------------------------

extern HFONT currFont;
extern HBITMAP RedBulb;
extern HBITMAP GreenBulb;

extern BOOL	Wait_For_Flip;          // User parm, default=TRUE
extern BOOL	DoAccurateFlips;        // User parm, default=TRUE
extern BOOL	Hurry_When_Late;        // " , default=FALSE, skip processing if behind
extern long	Sleep_Interval;         // " , default=0, how long to wait for BT chip
extern int	Back_Buffers;			// " , nuber of video back buffers
extern COLORREF OverlayColor;

// Add some global fields for Blended Clip Deinterlace - Tom Barry 11/05/00
// These should be treated as part of that, if it ever becomes a class.
extern	int		BlcMinimumClip;
extern	UINT	BlcPixelMotionSense;
extern	int		BlcRecentMotionSense;
extern	UINT	BlcMotionAvgPeriod;
extern	UINT	BlcPixelCombSense;
extern	UINT	BlcRecentCombSense;
extern	UINT	BlcCombAvgPeriod;
extern	UINT	BlcHighCombSkip;
extern	UINT	BlcLowMotionSkip;
extern  BOOL	BlcUseInterpBob;
extern  BOOL	BlcBlendChroma;
extern  BOOL	BlcWantsToFlip;
extern  UINT	BlcAverageMotions[5][2];
extern  UINT	BlcTotalAverageMotion;
extern  UINT	BlcAverageCombs[5][2];
extern  UINT	BlcTotalAverageComb;
extern  BOOL	BlcShowControls;
extern  UINT	BlcVerticalSmoothing;

// Add some global fields for Adv Video Flags - Tom Barry 12/15/00
extern  BYTE	BtAgcDisable;
extern  BYTE	BtCrush;
extern  BYTE	BtEvenChromaAGC;
extern  BYTE	BtOddChromaAGC;
extern  BYTE	BtEvenLumaPeak;
extern  BYTE	BtOddLumaPeak;
extern  BYTE	BtFullLumaRange;
extern  BYTE	BtEvenLumaDec;
extern  BYTE	BtOddLumaDec;
extern  BYTE	BtEvenComb;
extern  BYTE	BtOddComb;
extern  BYTE	BtColorBars;
extern  BYTE	BtGammaCorrection;
extern	BYTE    BtCoring;
extern  BYTE    BtHorFilter;
extern	BYTE    BtVertFilter;
extern	BYTE    BtColorKill;
extern	BYTE    BtWhiteCrushUp;
extern	BYTE    BtWhiteCrushDown;

extern  UINT    CpuFeatureFlags;		// TRB 12/20/00 Processor capability flags

//------------------------------------------------------------------------
// ######### IMPORTANT! READ ME FIRST! ##########
//
// Only put variables here IF it is necessary to share the same variable
// between multiple modules.  If it is not necessary, please instead
// put your variable declaration at the top of the module you are editing.
// This makes other programmers' lives easier keeping track of variables.
//------------------------------------------------------------------------

#endif
