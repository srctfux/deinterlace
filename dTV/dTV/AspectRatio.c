/////////////////////////////////////////////////////////////////////////////
// AspectRatio.c
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
/////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "other.h"
#include "AspectRatio.h"
#include "resource.h"
#include "DebugLog.h"
#include "Status.h"
#include "bt848.h"
#include "dTV.h"

#define AR_STRETCH       0
#define AR_NONANAMORPHIC 1
#define AR_ANAMORPHIC    2

// From dtv.c .... We really need to reduce reliance on globals by going C++!
// Perhaps in the meantime, it could be passed as a parameter to WorkoutOverlay()
extern HMENU hMenu;
extern BOOL  bIsFullScreen;
extern void ShowText(HWND hWnd, LPCTSTR szText);

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
	RECT rOverlayDest;
	RECT rOverlaySrc;
	RECT previousDest = aspectSettings.destinationRectangle; // MRS 2-22-01
	int overscan = aspectSettings.InitialOverscan;

	int DestWidth, DestHeight;

	UpdateWindowState();

	// Do overscan
	// Make sure the overscan is big enough for the orbit
	if (aspectSettings.orbitEnabled && overscan*2 < aspectSettings.orbitSize) overscan = (aspectSettings.orbitSize+1)/2;
	rOverlaySrc.left = overscan;
	rOverlaySrc.top  = overscan;
	rOverlaySrc.right = CurrentX - overscan;
	rOverlaySrc.bottom = CurrentY - overscan;

	if (aspectSettings.orbitEnabled) {
		int orbitX = 0, orbitY = 0;
		time_t t = time(NULL);
		
		if (aspectSettings.bounceStartTime == 0) time(&aspectSettings.bounceStartTime);
		
		// Figure out how far to move...
		orbitX = MulDiv((t-aspectSettings.bounceStartTime)%aspectSettings.orbitPeriodX,aspectSettings.orbitSize*2,aspectSettings.orbitPeriodX)+aspectSettings.orbitSize/2;
		orbitY = MulDiv((t-aspectSettings.bounceStartTime)%aspectSettings.orbitPeriodY,aspectSettings.orbitSize*2,aspectSettings.orbitPeriodY)+aspectSettings.orbitSize/2;
		if (orbitX > aspectSettings.orbitSize) orbitX = ABS(2*aspectSettings.orbitSize-orbitX);
		if (orbitY > aspectSettings.orbitSize) orbitY = ABS(2*aspectSettings.orbitSize-orbitY);

		// Shift image
		orbitX -= aspectSettings.orbitSize/2;
		orbitY -= aspectSettings.orbitSize/2;
		rOverlaySrc.left += orbitX;
		rOverlaySrc.right += orbitX;
		rOverlaySrc.top += orbitY;
		rOverlaySrc.bottom += orbitY;
	}


	// get main window client area
	// and convert to screen coordinates
	GetClientRect(hWnd, &rOverlayDest);
	ClientToScreen(hWnd, (POINT *) &(rOverlayDest.left));
	ClientToScreen(hWnd, (POINT *) &(rOverlayDest.right));
	if (IsStatusBarVisible())
	{
		rOverlayDest.bottom -= StatusBar_Height();
	}

	// MRS 9-2-00 - Adjust for aspect ratio preservation
	// Adjust source or destination rectangle to preserve aspect ratio (as requested)
	#define RWIDTH(aa) (aa.right-aa.left)
	#define RHEIGHT(aa) (aa.bottom-aa.top)
	#define ASPECT(aa) ((double)RWIDTH(aa)) / ((double)RHEIGHT(aa))


	if (aspectSettings.aspect_mode) {
		RECT rOriginalDest;
		RECT rOriginalSrc;	
		
		// OK - Lots of aspects to calculate
		// I am certain we could make this more efficient by caching values
		// and not calculating values needed - but this function is rarely called (window size, hardware setup, and aspect size changes)
		// and as such, I see no reason to go through the effort to modify it.
		
		// Aspect ratio of the Input - about 1.5 instead of 1.3 because sampling is at 720x480, not 640x480
		double SourceFrameAspect = (double)CurrentX/(double)CurrentY;
		// The source normally represents a 4:3 video frame (left as a variable for future processors to change)
		// BUT in anamorphic modes, it really represents a 16:9 video frame.
		double ActualSourceFrameAspect;
		// Aspect ratio of the computer screen		
		double ComputerAspect = (double) GetSystemMetrics(SM_CXSCREEN) / 
  						        (double) GetSystemMetrics(SM_CYSCREEN); 
		// Aspect ratio of the window we are displaying in
		double WindowAspect = ASPECT(rOverlayDest);
		// Aspect ratio of the projection screen
		double ScreenAspect = aspectSettings.target_aspect ? (aspectSettings.target_aspect/1000.0) : ComputerAspect;
		// Aspect Ratio of how the window appears on the projection screen
		double AdjustedWindowAspect = WindowAspect * ScreenAspect / ComputerAspect;
		// Aspect ratio of the material we are displaying
		double MaterialAspect = aspectSettings.source_aspect ? (aspectSettings.source_aspect/1000.0) : AdjustedWindowAspect;

		// To recap (for a 2.35 movie on a 16:9 anamorphic DVD being displayed on a 
		//           4:3 screen in a 100x100 window that is projected onto a 1.85 screen
		// All numbers are appromixate
		// MaterialAspect = 2.35 (movie aspect ratio)
		// SourceFrameAspect = 1.5 (720x480)
	    // AcutalSourceFrameAspect = 1.77 (16:9)
		// WindowAspect = 1.0 (100x100 window)
		// ComputerAspect = 1.33 (800x600, for example)
		// ScreenAspect = 1.85
		// AdustedWindowAspect = 1.4 (the 1.0 aspect window is stretched top 1.4 by the projector)

		// Now, we need to (based on mode) calculate what to crop the source to
		// and what to crop the target window to.
		// These need to be adjusted for differences between source frame and what the frame represnets
		// And for what the window is vs the projection screen
		double TargetSrcAspect; 
		double TargetDestAspect;

		// Save original source and dest rectangles
		memcpy(&rOriginalDest,&rOverlayDest,sizeof(rOverlayDest));
		memcpy(&rOriginalSrc,&rOverlaySrc,sizeof(rOverlaySrc));
		ActualSourceFrameAspect = GetActualSourceFrameAspect();

		TargetSrcAspect = MaterialAspect * SourceFrameAspect / ActualSourceFrameAspect;
		TargetDestAspect = MaterialAspect * ComputerAspect / ScreenAspect;	

		// Finally, perform the crops.
		// Crop the source rectangle - round to the nearest .5 pixels
		if (TargetSrcAspect > .1) {
			if (SourceFrameAspect > TargetSrcAspect) {
				// Source is wider - crop Left and Right
				int NewWidth = (int) floor((TargetSrcAspect*RHEIGHT(rOverlaySrc))+.5);
				rOverlaySrc.left += (RWIDTH(rOverlaySrc) - NewWidth)/2;
				rOverlaySrc.right = rOverlaySrc.left + NewWidth;
			} else {
				// Source is taller - crop top and bottom
				int NewHeight = (int) floor((RWIDTH(rOverlaySrc)/TargetSrcAspect)+.5);
				rOverlaySrc.top += (RHEIGHT(rOverlaySrc) - NewHeight)/2;
				rOverlaySrc.bottom = rOverlaySrc.top + NewHeight;
			}
		}
		// Crop the destination rectangle
		if (TargetDestAspect > .1)
		{
			#define BOUNCE_POS ((MulDiv((time(NULL)-aspectSettings.bounceStartTime)%aspectSettings.bouncePeriod,200,aspectSettings.bouncePeriod)+50)*10000l)
			if (aspectSettings.bounceEnabled && aspectSettings.bounceStartTime == 0) time(&aspectSettings.bounceStartTime);

			if (WindowAspect > TargetDestAspect)
			{
				// Source is wider - crop Left and Right
				long pos;
				int NewWidth;
				if (aspectSettings.bounceEnabled) pos = BOUNCE_POS;
				else switch (aspectSettings.HorizontalPos) {
					case HORZ_POS_LEFT: pos = 0; break;
					case HORZ_POS_RIGHT: pos = 1000000; break;
					default: pos = 500000; break;
				}
				if (pos > 1000000) pos = ABS(2000000 - pos);
				NewWidth = (int) floor((TargetDestAspect*RHEIGHT(rOverlayDest))+.5);
				rOverlayDest.left += MulDiv(RWIDTH(rOverlayDest)-NewWidth,pos,1000000l);
				rOverlayDest.right = rOverlayDest.left + NewWidth;
			}
			else
			{
				// Source is taller - crop top and bottom
				long pos;
				int NewWidth;
				if (aspectSettings.bounceEnabled) pos = BOUNCE_POS;
				else switch (aspectSettings.VerticalPos) {
					case VERT_POS_TOP: pos = 0; break;
					case VERT_POS_BOTTOM: pos = 1000000; break;
					default: pos = 500000; break;
				}
				if (pos > 1000000) pos = ABS(2000000 - pos);
				NewWidth = (int) floor((RWIDTH(rOverlayDest)/TargetDestAspect)+.5);
				rOverlayDest.top += MulDiv((RHEIGHT(rOverlayDest) - NewWidth),pos,1000000l);
				rOverlayDest.bottom = rOverlayDest.top + NewWidth;
			}
		}


		if (!aspectSettings.aspectImageClipped) {
			// The user requested we not clip the image
			// Figure out where we have space left and add it back in
			// (up to the amount of image we have)
			// Note: This could likely be done easier, but restructuring of all
			// the above code would be required.  Unless performance justified
			// I do not see this being worth the bug risk at this point
			// MRS 2-20-01
			RECT lastSrc;
			double vScale = (double)RHEIGHT(rOverlayDest) / (double)RHEIGHT(rOverlaySrc);
			double hScale = (double)RWIDTH(rOverlayDest) / (double)RWIDTH(rOverlaySrc);

			// Backup source so we can scale back
			memcpy(&lastSrc,&rOverlaySrc,sizeof(rOverlaySrc));

			// Scale the source image to use the entire image
			rOverlaySrc.left = rOverlaySrc.left - (int)floor((rOverlayDest.left - rOriginalDest.left)/hScale);
			rOverlaySrc.right = rOverlaySrc.right + (int)floor((rOriginalDest.right - rOverlayDest.right)/hScale);
			rOverlaySrc.top = rOverlaySrc.top - (int)floor((rOverlayDest.top - rOriginalDest.top)/vScale);
			rOverlaySrc.bottom = rOverlaySrc.bottom + (int)floor((rOriginalDest.bottom - rOverlayDest.bottom)/vScale);

			// Clip the source image to actually available image
			if (rOverlaySrc.left < rOriginalSrc.left) rOverlaySrc.left = rOriginalSrc.left;
			if (rOverlaySrc.right > rOriginalSrc.right) rOverlaySrc.right = rOriginalSrc.right;
			if (rOverlaySrc.top < rOriginalSrc.top) rOverlaySrc.top = rOriginalSrc.top;
			if (rOverlaySrc.bottom > rOriginalSrc.bottom) rOverlaySrc.bottom = rOriginalSrc.bottom;
			
			// Now scale the destination to the source remaining
			rOverlayDest.left += (int)floor((rOverlaySrc.left-lastSrc.left)*hScale);
			rOverlayDest.right -= (int)floor((lastSrc.right-rOverlaySrc.right)*hScale);
			rOverlayDest.top += (int)floor((rOverlaySrc.top-lastSrc.top)*vScale);
			rOverlayDest.bottom -= (int)floor((lastSrc.bottom-rOverlaySrc.bottom)*vScale);
		}

		//#define DEBUGASPECT
		#ifdef DEBUGASPECT
		#define TEST(aa) { char t[1024]; sprintf(t,"%i %i %i %i",aa.left,aa.right,aa.top,aa.bottom); MessageBox(NULL,t,#aa,MB_OK); }
		TEST(rOverlaySrc);
		TEST(rOverlayDest);
		{
			char t[1024];
			sprintf(t,"SF = %.4lf\nASF = %.4lf\nCA = %.4lf\nWA = %.4lf\nSA = %.4lf\nAWA=%.4lf\nMA=%.4lf\nTSA=%.4lf\nTDA=%.4lf",
				SourceFrameAspect,ActualSourceFrameAspect,ComputerAspect,WindowAspect,ScreenAspect,AdjustedWindowAspect,MaterialAspect,
				TargetSrcAspect,TargetDestAspect);
			MessageBox(NULL,t,"Ratios",MB_OK);
		}
		#endif
	}
	// END MRS 9-2-00

	// crop the Destination rect so that the
	// overlay destination region is 
	// always on the screen
	// we will also update the source area to reflect this
	// so that we see the appropriate portion
	// on the screen
	// (this should make us compatable with YXY)
	DestHeight = rOverlayDest.bottom - rOverlayDest.top;
	DestWidth = rOverlayDest.right - rOverlayDest.left;
	
	if (rOverlayDest.left < 0 && DestWidth > 0)
	{
		rOverlaySrc.left = (CurrentX * -rOverlayDest.left) / DestWidth;
		rOverlayDest.left = 0;
	}
	if (rOverlayDest.top < 0 && DestHeight > 0)
	{
		rOverlaySrc.top = (CurrentY * -rOverlayDest.top) / DestHeight;
		rOverlayDest.top = 0;
	}
	if (rOverlayDest.right >= GetSystemMetrics(SM_CXSCREEN) && DestWidth > 0)
	{
		rOverlaySrc.right -= (rOverlayDest.right - GetSystemMetrics(SM_CXSCREEN)) * 
							CurrentX / DestWidth;
		rOverlayDest.right = GetSystemMetrics(SM_CXSCREEN);
	}
	if (rOverlayDest.bottom >= GetSystemMetrics(SM_CYSCREEN) && DestHeight > 0)
	{
		rOverlaySrc.bottom -= (rOverlayDest.bottom - GetSystemMetrics(SM_CYSCREEN)) * 
							CurrentY / DestHeight;
		rOverlayDest.bottom = GetSystemMetrics(SM_CYSCREEN);
	}

	// If we're in half-height mode, squish the source rectangle accordingly.  This
	// allows the overlay hardware to do our bobbing for us.
	if (HalfHeight)
	{
		rOverlaySrc.top /= 2;
		rOverlaySrc.bottom /= 2;
	}

	// make sure that any alignment restrictions are taken care of
	if(SrcSizeAlign > 1)
	{
		rOverlaySrc.left += SrcSizeAlign - rOverlaySrc.left % SrcSizeAlign;
		rOverlaySrc.top += SrcSizeAlign - rOverlaySrc.top % SrcSizeAlign;
		rOverlaySrc.right -= rOverlaySrc.right % SrcSizeAlign;
		rOverlaySrc.bottom -= rOverlaySrc.bottom % SrcSizeAlign;
	}

	if(DestSizeAlign > 1)
	{
		rOverlayDest.left += DestSizeAlign - rOverlayDest.left % DestSizeAlign;
		rOverlayDest.top += DestSizeAlign - rOverlayDest.top % DestSizeAlign;
		rOverlayDest.right -= rOverlayDest.right % DestSizeAlign;
		rOverlayDest.bottom -= rOverlayDest.bottom % DestSizeAlign;
	}

	// 2000-09-14 Mark Rejhon
	// This was an attempt to eliminate error messages when window is resized to zero height
	if (rOverlayDest.left >= rOverlayDest.right)  rOverlayDest.right  = rOverlayDest.left + 1;
	if (rOverlayDest.top  >= rOverlayDest.bottom) rOverlayDest.bottom = rOverlayDest.top  + 1;
	if (rOverlaySrc.left  >= rOverlaySrc.right)   rOverlaySrc.right   = rOverlaySrc.left  + 1;
	if (rOverlaySrc.top   >= rOverlaySrc.bottom)  rOverlaySrc.bottom  = rOverlaySrc.top   + 1;

	if (aspectSettings.autoResizeWindow && allowResize && !bIsFullScreen) {
		// See if we need to resize the window
		RECT currentClientRect;
		RECT newRect = rOverlayDest;
		GetClientRect(hWnd, &rOverlayDest);
		ClientToScreen(hWnd, (POINT *) &(currentClientRect.left));
		ClientToScreen(hWnd, (POINT *) &(currentClientRect.right));
		if (IsStatusBarVisible()) newRect.bottom += StatusBar_Height();
		
		#define DELTA(aa) ABS(currentClientRect.aa-newRect.aa)
		if (DELTA(left) > 4 || DELTA(right) > 4 ||
			DELTA(top) > 4 || DELTA(bottom) > 4) {
		#undef DELTA
			// Rectangles do not match!  Resize window...

			// !!!!!!! Note - this will cause the window to shrink every
			// time a aspect change occurs.  Needs to be altered...
			// Please keep in mind that the new size needs to be adjusted for overlay alignments
			
			// Convert client rect to window rect...
			AdjustWindowRectEx(&newRect,GetWindowLong(hWnd,GWL_STYLE),TRUE,0);
			
			// Size the window
			if (RWIDTH(newRect) > 8 &&
				RHEIGHT(newRect) > 8) {
				// Sanity check - new window pos must be at least 8 pixels x 8 pixels
				SetWindowPos(hWnd,NULL,newRect.left,newRect.top,RWIDTH(newRect),RHEIGHT(newRect),
							 SWP_NOZORDER);

				// Resize message will force an aspect adjust, so no need to do it now...
				_WorkoutOverlaySize(FALSE);
				return;
			}
		}
	} 

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
