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
// 12 Sep 2000   Mark Rejhon           Centralized aspect ratio code
//                                     into separate module
//
// 09 Sep 2000   Michael Samblanet     Aspect ratio code contributed 
//                                     to dTV project
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "other.h"
#include "AspectRatio.h"
#include "resource.h"

// From dtv.c .... We really need to reduce reliance on globals by going C++!
// Perhaps in the meantime, it could be passed as a parameter to WorkoutOverlay()
extern HMENU hMenu;

// Added variable in dTV.c to track which aspect mode we are currently in
// Use aspect * 1000 (1.66 = 1660, 2.35 = 2350, etc)
// Use negative values for source_aspect to imply anamorphic sources
// Note: target_aspect is the aspect ratio of the screen.
int source_aspect = 0;
int target_aspect = 0;
int aspect_mode = 0;

// Mode 0 = do nothing, 1 = Letterboxed, 2 = 16:9 anamorphic
int custom_source_aspect = 0;
int custom_target_aspect = 0;

RECT destinationRectangle = {0,0,0,0};



//----------------------------------------------------------------------------
// MENU INITIALIZATION:
// Initializing aspect ratio control related checkboxes in menu
void SetMenuAspectRatio(HWND hWnd)
{
	CheckMenuItem(GetMenu(hWnd), IDM_ASPECT_FULLSCREEN, MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_ASPECT_LETTERBOX,  MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_ASPECT_ANAMORPHIC, MF_UNCHECKED);

	CheckMenuItem(GetMenu(hWnd), IDM_SASPECT_133, MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_SASPECT_166, MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_SASPECT_178, MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_SASPECT_185, MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_SASPECT_200, MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_SASPECT_235, MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_SASPECT_166A, MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_SASPECT_178A, MF_UNCHECKED); 
	CheckMenuItem(GetMenu(hWnd), IDM_SASPECT_185A, MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_SASPECT_200A, MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_SASPECT_235A, MF_UNCHECKED);

	if (aspect_mode == 1)
	{
		switch (source_aspect)
		{
		case 1333:
			CheckMenuItem(GetMenu(hWnd), IDM_SASPECT_133, MF_CHECKED);
			CheckMenuItem(GetMenu(hWnd), IDM_ASPECT_FULLSCREEN, MF_CHECKED);
			break;
		case 1667:
			CheckMenuItem(GetMenu(hWnd), IDM_SASPECT_166, MF_CHECKED);
			break;
		case 1778:
			CheckMenuItem(GetMenu(hWnd), IDM_SASPECT_178, MF_CHECKED);
			CheckMenuItem(GetMenu(hWnd), IDM_ASPECT_LETTERBOX, MF_CHECKED);
			break;
		case 1850:
			CheckMenuItem(GetMenu(hWnd), IDM_SASPECT_185, MF_CHECKED);
			break;
		case 2000:
			CheckMenuItem(GetMenu(hWnd), IDM_SASPECT_200, MF_CHECKED);
			break;
		case 2350:
			CheckMenuItem(GetMenu(hWnd), IDM_SASPECT_235, MF_CHECKED);
			break;
		}
	}
	else if (aspect_mode == 2)
	{
		switch (source_aspect)
		{
		case 1667:
			CheckMenuItem(GetMenu(hWnd), IDM_SASPECT_166A, MF_CHECKED);
			break;
		case 1778:
			CheckMenuItem(GetMenu(hWnd), IDM_SASPECT_178A, MF_CHECKED); 
			CheckMenuItem(GetMenu(hWnd), IDM_ASPECT_ANAMORPHIC, MF_CHECKED); 
            break;
		case 1850:
			CheckMenuItem(GetMenu(hWnd), IDM_SASPECT_185A, MF_CHECKED);
			break;
		case 2000:
			CheckMenuItem(GetMenu(hWnd), IDM_SASPECT_200A, MF_CHECKED);
			break;
		case 2350:
			CheckMenuItem(GetMenu(hWnd), IDM_SASPECT_235A, MF_CHECKED);
			break;
		}
	}
	CheckMenuItem(GetMenu(hWnd), IDM_SASPECT_0,   (source_aspect == 0)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_SASPECT_CUSTOM, (source_aspect && source_aspect == custom_source_aspect)?MF_CHECKED:MF_UNCHECKED);

	// Advanced Aspect Ratio -> Display Aspect Ratio
	CheckMenuItem(GetMenu(hWnd), IDM_TASPECT_0,   (target_aspect == 0)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_TASPECT_133, (target_aspect == 1333)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_TASPECT_166, (target_aspect == 1667)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_TASPECT_178, (target_aspect == 1778)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_TASPECT_185, (target_aspect == 1850)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_TASPECT_200, (target_aspect == 2000)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_TASPECT_235, (target_aspect == 2350)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), IDM_TASPECT_CUSTOM, (target_aspect && target_aspect == custom_target_aspect)?MF_CHECKED:MF_UNCHECKED);
}

//----------------------------------------------------------------------------
// MENU PROCESSING:
// Processing aspect ratio control related menu selection
// during aspect ratio control
int ProcessAspectRatioSelection(HWND hWnd, WORD wMenuID)
{
	switch (wMenuID) {
	case IDM_ASPECT_FULLSCREEN:
	case IDM_ASPECT_LETTERBOX:
	case IDM_ASPECT_ANAMORPHIC:
	case IDM_SASPECT_0:
	case IDM_SASPECT_133:
	case IDM_SASPECT_166:
	case IDM_SASPECT_178:
	case IDM_SASPECT_185:
	case IDM_SASPECT_200:
	case IDM_SASPECT_235:
	case IDM_SASPECT_166A:
	case IDM_SASPECT_178A:
	case IDM_SASPECT_185A:
	case IDM_SASPECT_200A:
	case IDM_SASPECT_235A:
	case IDM_SASPECT_CUSTOM:
	case IDM_TASPECT_0:
	case IDM_TASPECT_133:
	case IDM_TASPECT_166:
	case IDM_TASPECT_178:
	case IDM_TASPECT_185:
	case IDM_TASPECT_200:
	case IDM_TASPECT_235:
	case IDM_TASPECT_CUSTOM:
		
		switch (wMenuID) {
			// Easily Accessible Aspect Ratios
			case IDM_ASPECT_FULLSCREEN:  aspect_mode = 1;  source_aspect = 1333;  break;
			case IDM_ASPECT_LETTERBOX:   aspect_mode = 1;  source_aspect = 1778;  break;
			case IDM_ASPECT_ANAMORPHIC:  aspect_mode = 2;  source_aspect = 1778;  break;

			// Advanced Aspect Ratios
			case IDM_SASPECT_0:       aspect_mode = 0;  source_aspect = 0;     break;
			case IDM_SASPECT_133:     aspect_mode = 1;  source_aspect = 1333;  break;
			case IDM_SASPECT_166:     aspect_mode = 1;  source_aspect = 1667;  break;
			case IDM_SASPECT_178:     aspect_mode = 1;  source_aspect = 1778;  break;
			case IDM_SASPECT_185:     aspect_mode = 1;  source_aspect = 1850;  break;
			case IDM_SASPECT_200:     aspect_mode = 1;  source_aspect = 2000;  break;
			case IDM_SASPECT_235:     aspect_mode = 1;  source_aspect = 2350;  break;
			case IDM_SASPECT_166A:    aspect_mode = 2;  source_aspect = 1667;  break;
			case IDM_SASPECT_178A:    aspect_mode = 2;  source_aspect = 1778;  break;
			case IDM_SASPECT_185A:    aspect_mode = 2;  source_aspect = 1850;  break;
			case IDM_SASPECT_200A:    aspect_mode = 2;  source_aspect = 2000;  break;
			case IDM_SASPECT_235A:    aspect_mode = 2;  source_aspect = 2350;  break;
			case IDM_SASPECT_CUSTOM:  aspect_mode = 2;  source_aspect = custom_source_aspect;  break;

			// Output Display Aspect Ratios
			case IDM_TASPECT_0:       target_aspect = 0;     break;
			case IDM_TASPECT_133:     target_aspect = 1330;  break;
			case IDM_TASPECT_166:     target_aspect = 1667;  break;
			case IDM_TASPECT_178:     target_aspect = 1778;  break;
			case IDM_TASPECT_185:     target_aspect = 1850;  break;
			case IDM_TASPECT_200:     target_aspect = 2000;  break;
			case IDM_TASPECT_235:     target_aspect = 2350;  break;
			case IDM_TASPECT_CUSTOM:  target_aspect = custom_target_aspect;  break;

			default:  break;
		}
		WorkoutOverlaySize();
		SetMenuAspectRatio(hWnd);
		break;

	default:
		// It's not an aspect ratio related menu selection
		return 0;
	}
	// Yes, we processed the menu selection.
	return 1;
}

//----------------------------------------------------------------------------
// Repaints the overlay colorkey, optionally with black borders around it
// during aspect ratio control
void PaintOverlay(HWND hWnd)
{
	PAINTSTRUCT sPaint;
	// MRS 9-9-00
	HBRUSH black = CreateSolidBrush(RGB(0,0,0));
	HBRUSH overlay;
	BeginPaint(hWnd, &sPaint);
	overlay = CreateSolidBrush(GetNearestColor(sPaint.hdc, OverlayColor));
	//FillRect(sPaint.hdc, &sPaint.rcPaint, CreateSolidBrush(OverlayColor));

	if (destinationRectangle.right > destinationRectangle.left) {
		RECT r;
		// Draw black in the 4 borders
		RECT r2, winRect;
		GetWindowRect(hWnd,&winRect);

		// Top
		r2.left = 0;
		r2.top = 0;
		r2.right = winRect.right;
		r2.bottom = destinationRectangle.top;
		IntersectRect(&r,&r2,&sPaint.rcPaint);
		FillRect(sPaint.hdc, &r, black);

		// Bottom
		r2.left = 0;
		r2.top = destinationRectangle.bottom;
		r2.right = winRect.right;
		r2.bottom = winRect.bottom;
		IntersectRect(&r,&r2,&sPaint.rcPaint);
		FillRect(sPaint.hdc, &r, black);

		// Left
		r2.left = 0;
		r2.top = 0;
		r2.right = destinationRectangle.left;
		r2.bottom = winRect.bottom;
		IntersectRect(&r,&r2,&sPaint.rcPaint);
		FillRect(sPaint.hdc, &r, black);

		// Right
		r2.left = destinationRectangle.right;
		r2.top = 0;
		r2.right = winRect.right;
		r2.bottom = winRect.bottom;
		IntersectRect(&r,&r2,&sPaint.rcPaint);
		FillRect(sPaint.hdc, &r, black);

		// Draw overlay color in middle
		IntersectRect(&r,&destinationRectangle,&sPaint.rcPaint);
		FillRect(sPaint.hdc, &r, overlay);

	} else {
		FillRect(sPaint.hdc, &sPaint.rcPaint, overlay);
	}
	DeleteObject(black);
	DeleteObject(overlay);
	// END MRS
	EndPaint(hWnd, &sPaint);
}

//----------------------------------------------------------------------------
// Calculate size and position coordinates for video overlay
// Takes into account of aspect ratio control.
void WorkoutOverlaySize()
{
	RECT rOverlayDest;
	RECT rOverlaySrc;

	int DestWidth, DestHeight;

	if(bIsFullScreen == TRUE)
	{
		SetWindowLong(hWnd, GWL_STYLE, WS_VISIBLE);

		SetWindowPos(hWnd,
					HWND_TOPMOST,
					0,
					0,
					GetSystemMetrics(SM_CXSCREEN),
					GetSystemMetrics(SM_CYSCREEN),
					SWP_SHOWWINDOW);
		ShowWindow(hwndStatusBar, SW_HIDE);
		SetMenu(hWnd, NULL);
	}
	else
	{
		SetWindowLong(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);

		SetMenu(hWnd, (Show_Menu == TRUE)?hMenu:NULL);

		ShowWindow(hwndStatusBar, bDisplayStatusBar?SW_SHOW:SW_HIDE);

		SetWindowPos(hWnd,bAlwaysOnTop?HWND_TOPMOST:HWND_NOTOPMOST,
					0,0,0,0,
					SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
	}

	// Do overscan
	rOverlaySrc.left = InitialOverscan;
	rOverlaySrc.top  = InitialOverscan;
	rOverlaySrc.right = CurrentX - InitialOverscan;
	rOverlaySrc.bottom = CurrentY - InitialOverscan;

	// get main window client area
	// and convert to screen coordinates
	GetClientRect(hWnd, &rOverlayDest);
	ClientToScreen(hWnd, (POINT *) &(rOverlayDest.left));
	ClientToScreen(hWnd, (POINT *) &(rOverlayDest.right));
	if (bDisplayStatusBar == TRUE && bIsFullScreen == FALSE)
	{
		rOverlayDest.bottom -= 21;
	}

	// MRS 9-2-00 - Adjust for aspect ratio preservation
	// Adjust source or destination rectangle to preserve aspect ratio (as requested)
	#define RWIDTH(aa) (aa.right-aa.left)
	#define RHEIGHT(aa) (aa.bottom-aa.top)
	#define ASPECT(aa) ((double)RWIDTH(aa)) / ((double)RHEIGHT(aa))

	if (aspect_mode) {
		// OK - Lots of aspects to calculate
		// I am certain we could make this more efficient by caching values
		// and not calculating values needed - but this function is rarely called (window size, hardware setup, and aspect size changes)
		// and as such, I see no reason to go through the effort to modify it.
		
		// Aspect ratio of the source rectangle - about 1.5 instead of 1.3 because sampling is at 720x480, not 640x480
		double SourceFrameAspect = ASPECT(rOverlaySrc);
		// The source normally represents a 4:3 video frame (left as a variable for future processors to change)
		// BUT in anamorphic modes, it really represents a 16:9 video frame.
		double ActualSourceFrameAspect;
		// Aspect ratio of the computer screen		
		double ComputerAspect = (double) GetSystemMetrics(SM_CXSCREEN) / 
  						        (double) GetSystemMetrics(SM_CYSCREEN); 
		// Aspect ratio of the window we are displaying in
		double WindowAspect = ASPECT(rOverlayDest);
		// Aspect ratio of the projection screen
		double ScreenAspect = target_aspect ? (target_aspect/1000.0) : ComputerAspect;
		// Aspect Ratio of how the window appears on the projection screen
		double AdjustedWindowAspect = WindowAspect * ScreenAspect / ComputerAspect;
		// Aspect ratio of the material we are displaying
		double MaterialAspect = source_aspect ? (source_aspect/1000.0) : AdjustedWindowAspect;

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

				// --- From Mark Rejhon, Sept 11, 2000
		// Rewritten one line of code into easier-to-read switch statement
		// Michael, never use nested x?y:z syntax in one line, use switch instead :)
		// It's hard to read.  We want to keep code readable to harried coders and
		// new coders alike.  Delete this comment block after reading this comment.
		// --- End Mark Rejhon comment
		switch (aspect_mode) {
		case 1:
			ActualSourceFrameAspect = (4.0/3.0);
			break;
		case 2:
			ActualSourceFrameAspect = (16.0/9.0);
			break;
		default:
			ActualSourceFrameAspect = source_aspect/1000.0;
			break;
		}
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
		if (TargetDestAspect > .1) {
			if (WindowAspect > TargetDestAspect) {
				// Source is wider - crop Left and Right
				int NewWidth = (int) floor((TargetDestAspect*RHEIGHT(rOverlayDest))+.5);
				rOverlayDest.left += (RWIDTH(rOverlayDest) - NewWidth)/2;
				rOverlayDest.right = rOverlayDest.left + NewWidth;
			} else {
				// Source is taller - crop top and bottom
				int NewHeight = (int) floor((RWIDTH(rOverlayDest)/TargetDestAspect)+.5);
				rOverlayDest.top += (RHEIGHT(rOverlayDest) - NewHeight)/2;
				rOverlayDest.bottom = rOverlayDest.top + NewHeight;
			}
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
	// Apparently, this attempt doesn't work.
	if (rOverlayDest.left >= rOverlayDest.right)  rOverlayDest.right = rOverlayDest.left   + 1;
	if (rOverlayDest.top  >= rOverlayDest.bottom) rOverlayDest.top   = rOverlayDest.bottom + 1;
	if (rOverlaySrc.left  >= rOverlaySrc.right)   rOverlaySrc.right  = rOverlaySrc.left    + 1;
	if (rOverlaySrc.top   >= rOverlaySrc.bottom)  rOverlaySrc.top    = rOverlaySrc.bottom  + 1;

	Overlay_Update(&rOverlaySrc, &rOverlayDest, DDOVER_SHOW, TRUE);

	// MRS 9-9-00
	// Save the Overlay Destination and force a repaint 
	// Moved to after Overlay_Update in hopes of removing purple flashing.
	destinationRectangle = rOverlayDest;
	ScreenToClient(hWnd,((PPOINT)&destinationRectangle));
	ScreenToClient(hWnd,((PPOINT)&destinationRectangle)+1);
	InvalidateRect(hWnd,NULL,FALSE);

	return;
}
