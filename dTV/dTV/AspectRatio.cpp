/////////////////////////////////////////////////////////////////////////////
// AspectRatio.cpp
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
// Aspect ratio contrl was started by Michael Samblanet <mike@cardobe.com>
// Moved into separate module by Mark D Rejhon.  
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
// 09 Sep 2000   Michael Samblanet     Aspect ratio code contributed 
//                                     to dTV project
//
// 12 Sep 2000   Mark Rejhon           Centralized aspect ratio code
//                                     into separate module
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 11 Jan 2001   John Adcock           Added Code for window position
//                                     Changed SourceFrameAspect to use
//                                     CurrentX/Y rather than SourceRect
//                                     So that overscan does not effect Ratio
//                                     Fixed bug in FindBottomOfImage when
//                                     Overscan is 0
//
// 21 Feb 2001   Michael Samblanet     Added 1.44 and 1.55 ratio support
//                                     Added code to handle clipped menu item
//									   Added prototype image bouncing code
//									     (currently has issues with purple flashing)
//
// 22 Feb 2001   Michael Samblanet     Added defered setting of overlay region to
//                                     avoid purple flashing
//                                     Made bounce timer a ini setting and changed to 1sec default
//
// 23 Feb 2001   Michael Samblanet     Added experemental orbiting code
// 24 Feb 2001   Michael Samblanet     Minor bug fixes to invalidate code
// 10 Mar 2001   Michael Samblanet     Added first draft auto-resize window code
// 11 Mar 2001   Michael Samblanet     Converted to C++ file, initial pass at reworking the aspect calculation code
/////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
extern "C" {
	#include "other.h"
	#include "AspectRatio.h"
	#include "resource.h"
	#include "DebugLog.h"
	#include "Status.h"
	#include "bt848.h"
	#include "dTV.h"
	// From dtv.c .... We really need to reduce reliance on globals by going C++!
	// Perhaps in the meantime, it could be passed as a parameter to WorkoutOverlay()
	extern HMENU hMenu;
	extern BOOL  bIsFullScreen;
	extern void ShowText(HWND hWnd, LPCTSTR szText);
}
#include "AspectRect.hpp"

#define AR_STRETCH       0
#define AR_NONANAMORPHIC 1
#define AR_ANAMORPHIC    2


AspectSettingsStruct aspectSettings = {1333,0,1,0,0,30,0,FALSE,60,300,15,20,
									2000,VERT_POS_CENTRE,HORZ_POS_CENTRE,
									{0,0,0,0},{0,0,0,0},{0,0,0,0},FALSE,FALSE,4,TRUE,FALSE,
									0,60*30,1000,FALSE,8,60,60,1000,FALSE,FALSE};

BOOL Bounce_OnChange(long NewValue); // Forward declaration to reuse this code...
BOOL Orbit_OnChange(long NewValue); // Forward declaration to reuse this code...

//----------------------------------------------------------------------------
// Calculate the actual aspect ratio of the source frame, independent of
// grab or display resolutions.
double GetActualSourceFrameAspect()
{
	switch (aspectSettings.aspect_mode) {
	case 1:
		// Letterboxed or full-frame
		return 4.0/3.0;
	case 2:
		// Anamorphic
		return 16.0/9.0;
	default:
		// User-specified
		return aspectSettings.source_aspect/1000.0;
	}
}

//----------------------------------------------------------------------------
// Enter or leave half-height mode.
// True if we're in half-height mode (even or odd scanlines only).
static int HalfHeight;
void SetHalfHeight(int IsHalfHeight)
{
	if (IsHalfHeight != HalfHeight)
	{
		HalfHeight = IsHalfHeight;
		WorkoutOverlaySize();
	}
}


//----------------------------------------------------------------------------
// Calculate size and position coordinates for video overlay
// Takes into account of aspect ratio control.
void _WorkoutOverlaySize(BOOL allowResize)
{
	AspectRect rOverlayDest;
	AspectRect rOverlaySrc;
	RECT previousDest = aspectSettings.destinationRectangle; // MRS 2-22-01
	
	UpdateWindowState();

	// 1: Prepare source rectangle - adjust for overscan
		int overscan = aspectSettings.InitialOverscan;
		if (aspectSettings.orbitEnabled && overscan*2 < aspectSettings.orbitSize) overscan = (aspectSettings.orbitSize+1)/2;
		rOverlaySrc.left = overscan;
		rOverlaySrc.top  = overscan;
		rOverlaySrc.right = CurrentX - overscan;
		rOverlaySrc.bottom = CurrentY - overscan;
		// Apply orbit to source rectangle...
		// TODO: Eventually, the orbit objects should be persistent rather than re-built every time...
		if (aspectSettings.orbitEnabled) {
			if (aspectSettings.bounceStartTime == 0) time(&aspectSettings.bounceStartTime);
			PeriodBouncer xOrbit(aspectSettings.bounceStartTime,aspectSettings.orbitPeriodX,aspectSettings.orbitSize,-aspectSettings.orbitSize/2.0);
			PeriodBouncer yOrbit(aspectSettings.bounceStartTime,aspectSettings.orbitPeriodY,aspectSettings.orbitSize,-aspectSettings.orbitSize/2.0);

			rOverlaySrc.shift((int)xOrbit.position(),
							  (int)yOrbit.position());
		}
		// Set the aspect adjustment factor...
		rOverlaySrc.setAspectAdjust((double)CurrentX/(double)CurrentY,
			                         GetActualSourceFrameAspect());
		// If we're in half-height mode, squish the source rectangle accordingly.  This
		// allows the overlay hardware to do our bobbing for us.
		if (HalfHeight)	{ rOverlaySrc.top /= 2; rOverlaySrc.bottom /= 2; }

	// 2: Prepare target rectangle - start with client rectangle...
		rOverlayDest.setToClient(hWnd,TRUE);
		// Adjust for status bar...
		if (IsStatusBarVisible()) rOverlayDest.bottom -= StatusBar_Height();
		// Set the aspect adjustment factor if the screen aspect is specified...
		if (aspectSettings.target_aspect)
			rOverlayDest.setAspectAdjust((double) GetSystemMetrics(SM_CXSCREEN) / (double) GetSystemMetrics(SM_CYSCREEN),
										 aspectSettings.target_aspect/1000.0);


	// OK - start calculating....
	if (aspectSettings.aspect_mode) {
		double MaterialAspect = aspectSettings.source_aspect ? (aspectSettings.source_aspect/1000.0) : rOverlayDest.targetAspect();
	
		// Save source and dest going in - needed for un-cropping the window...
		AspectRect rOriginalDest(rOverlayDest);
		AspectRect rOriginalSrc(rOverlaySrc);

		// Crop the source rectangle down to the desired aspect...
		rOverlaySrc.adjustTargetAspectByShrink(MaterialAspect);

		// Crop the destination rectangle
		// Bouncers are used to position the target rectangle within the cropped region...
		// TODO: Ideally, the x and y bouncers should be saved rather than reconstructed
		// each time...
		Bouncer *xBouncer = NULL;
		Bouncer *yBouncer = NULL;
		if (aspectSettings.bounceEnabled) {
			if (aspectSettings.bounceEnabled && aspectSettings.bounceStartTime == 0) time(&aspectSettings.bounceStartTime);
			xBouncer = yBouncer = new PeriodBouncer(aspectSettings.bounceStartTime,2,-1);
		} else {
			switch (aspectSettings.HorizontalPos) {
				case HORZ_POS_LEFT:		xBouncer = new StaticBouncer(-1); break;
				case HORZ_POS_RIGHT:	xBouncer = new StaticBouncer(0); break;
				default:				xBouncer = new StaticBouncer(1); break;
			}
			switch (aspectSettings.VerticalPos) {
				case VERT_POS_TOP: 		yBouncer = new StaticBouncer(-1); break;
				case VERT_POS_BOTTOM:   yBouncer = new StaticBouncer(0); break;
				default:				yBouncer = new StaticBouncer(1); break;
			}
		}
		rOverlayDest.adjustTargetAspectByShrink(MaterialAspect,xBouncer,yBouncer);
		if (yBouncer != xBouncer) delete yBouncer;
		if (xBouncer) delete xBouncer;

		if (!aspectSettings.aspectImageClipped) {
			// The user requested we not clip the image
			// Figure out where we have space left and add it back in
			// (up to the amount of image we have)
			// Note: This could likely be done easier, but restructuring of all
			// the above code would be required.  Unless performance justified
			// I do not see this being worth the bug risk at this point
			// MRS 2-20-01
			double vScale = rOverlayDest.height() / rOverlaySrc.height();
			double hScale = rOverlayDest.width() / rOverlaySrc.width();

			// Scale the source image to use the entire image
			rOverlaySrc.left = rOverlaySrc.left - (int)floor((rOverlayDest.left - rOriginalDest.left)/hScale);
			rOverlaySrc.right = rOverlaySrc.right + (int)floor((rOriginalDest.right - rOverlayDest.right)/hScale);
			rOverlaySrc.top = rOverlaySrc.top - (int)floor((rOverlayDest.top - rOriginalDest.top)/vScale);
			rOverlaySrc.bottom = rOverlaySrc.bottom + (int)floor((rOriginalDest.bottom - rOverlayDest.bottom)/vScale);

			// Crop the source to the original source area and symetrically crop the destination...
			rOverlaySrc.crop(rOriginalSrc,&rOverlayDest);
		}
	}
	
	// crop the Destination rect so that the overlay destination region is 
	// always on the screen we will also update the source area to reflect this
	// so that we see the appropriate portion on the screen
	// (this should make us compatable with YXY)
	RECT screenRect = {0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN) };
	rOverlayDest.crop(screenRect,&rOverlaySrc);

	// make sure that any alignment restrictions are taken care of
	if (SrcSizeAlign > 1) rOverlaySrc.align(SrcSizeAlign);
	if (DestSizeAlign > 1) rOverlayDest.align(DestSizeAlign);

	// Ensure we do not shrink too small...avoids crashes when window gets too small
	rOverlayDest.enforceMinSize(1);

	if (aspectSettings.autoResizeWindow && allowResize && !bIsFullScreen) {
		// See if we need to resize the window
		AspectRect currentClientRect;
		AspectRect newRect = rOverlayDest;
		currentClientRect.setToClient(hWnd,TRUE);
		if (IsStatusBarVisible()) currentClientRect.bottom -= StatusBar_Height();
		
		if (currentClientRect.tolerantEquals(newRect,8)) { // Do we match????
			// Nope!  Scale the existing window using "smart" logic to grow or shrink the window as needed
			RECT sr = screenRect;
			if (IsStatusBarVisible()) sr.bottom -= StatusBar_Height();

			currentClientRect.adjustSourceAspectSmart(newRect.sourceAspect(),screenRect);
			// Add the status bar back in...
			if (IsStatusBarVisible()) currentClientRect.bottom += StatusBar_Height();		
			
			// Convert client rect to window rect...
			newRect.enforceMinSize(8);
			AdjustWindowRectEx(&newRect,GetWindowLong(hWnd,GWL_STYLE),TRUE,0);
			
			// Set the window...
			SetWindowPos(hWnd,NULL,newRect.left,newRect.top,newRect.width(),newRect.height(),
						 SWP_NOZORDER);

			// Recalculate the overlay, but do not get stuck in a loop...
			_WorkoutOverlaySize(FALSE);
			return;
		}
	} 

	// Save the settings....
	aspectSettings.destinationRectangle = rOverlayDest;
	aspectSettings.destinationRectangleWindow = rOverlayDest; 
	if (!aspectSettings.deferedSetOverlay) // MRS 2-22-01 - Defered overlay set
		Overlay_Update(&rOverlaySrc, &rOverlayDest, DDOVER_SHOW, TRUE);
	else aspectSettings.overlayNeedsSetting = TRUE;

	// Save the Overlay Destination and force a repaint 
	aspectSettings.sourceRectangle = rOverlaySrc;
	ScreenToClient(hWnd,((PPOINT)&aspectSettings.destinationRectangle));
	ScreenToClient(hWnd,((PPOINT)&aspectSettings.destinationRectangle)+1);

	// MRS 2-23-01 Only invalidate if we changed something
	if (memcmp(&previousDest,&aspectSettings.destinationRectangle,sizeof(previousDest))) { 
		// MRS 2-22-01 Invalidate just the union of the old region and the new region - no need to invalidate all of the window.
		RECT invalidate;
		UnionRect(&invalidate,&previousDest,&aspectSettings.destinationRectangle);
		InvalidateRect(hWnd,&invalidate,FALSE);
	} else if (aspectSettings.overlayNeedsSetting) {
		// If not invalidating, we need to update the overlay now...
		Overlay_Update(&rOverlaySrc, &rOverlayDest, DDOVER_SHOW, TRUE);
		aspectSettings.overlayNeedsSetting = FALSE;
	}

	return;
}
void WorkoutOverlaySize() {_WorkoutOverlaySize(TRUE);}

//----------------------------------------------------------------------------
// Returns the current source rectangle.
void GetSourceRect(RECT *rect)
{
	memcpy(rect, &aspectSettings.sourceRectangle, sizeof(RECT));
}

//----------------------------------------------------------------------------
// Returns the current source rectangle.
void GetDestRect(RECT *rect)
{
	memcpy(rect, &aspectSettings.destinationRectangle, sizeof(RECT));
}
