/////////////////////////////////////////////////////////////////////////////
// DI_BlendedClip.h
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 Tom Barry.  All rights reserved.
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
// 08 Jan 2001   John Adcock           Split into new file
//                                     as part of global Variable Tidy up
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __BLENDEDCLIP_H___
#define __BLENDEDCLIP_H___

BOOL APIENTRY BlendedClipProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

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

#endif
