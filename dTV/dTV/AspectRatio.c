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
#include "DebugLog.h"

#define AR_STRETCH    0
#define AR_LETTERBOX  1
#define AR_ANAMORPHIC 2

// From dtv.c .... We really need to reduce reliance on globals by going C++!
// Perhaps in the meantime, it could be passed as a parameter to WorkoutOverlay()
extern HMENU hMenu;
extern void ShowText(HWND hWnd, LPCTSTR szText);

// Added variable in dTV.c to track which aspect mode we are currently in
// Use aspect * 1000 (1.66 = 1660, 2.35 = 2350, etc)
// Use negative values for source_aspect to imply anamorphic sources
// Note: target_aspect is the aspect ratio of the screen.
int source_aspect = 0;
int target_aspect = 0;

// Mode 0 = do nothing, 1 = Letterboxed, 2 = 16:9 anamorphic
int aspect_mode = 0;

int custom_source_aspect = 0;
int custom_target_aspect = 0;

// True if we're in half-height mode (even or odd scanlines only).
static int HalfHeight = FALSE;

// Luminance cutoff for a black pixel for letterbox detection.  0-127.
long LuminanceThreshold = 30;

// Ignore this many non-black pixels when detecting letterbox.  0 means
// use a reasonable default.
long IgnoreNonBlackPixels = 0;

// Nonzero to continuously scan for aspect ratio changes.
long AutoDetectAspect = 0;

// For aspect autodetect, require the same aspect ratio for this number of
// frames before zooming in.
long ZoomInFrameCount = 60;

// For aspect autodetect, zoom in quickly if we've used this ratio in the
// last N seconds.
long AspectHistoryTime = 300;

// For aspect autodetect, only zoom in if we haven't detected a smaller
// ratio in some amount of time.
long AspectConsistencyTime = 15;

// For aspect autodetect, consider two ratios to be equal if they're within
// this amount of each other.  This is not in pixels, but in aspect*1000
// units.
long AspectEqualFudgeFactor = 10;

// Don't remember aspect ratios that lasted less than this many milliseconds.
long ShortRatioIgnoreMs = 1000;

//---------
// Internal variables used by auto aspect ratio detect code.

// Number of seconds of aspect ratio history to keep.
#define RATIO_HISTORY_SECONDS	600

// Number of aspect ratio changes to remember.
#define RATIO_HISTORY_CHANGES	50

// Minimum aspect ratio encountered each second for the last several minutes.
// The 0th element of this is the current second.
static int min_ratio_found[RATIO_HISTORY_SECONDS];

// Timestamp of the most recent computation of min_ratio_found.
static int min_ratio_tick_count = 0;

// Aspect ratios we've used recently.
static int ratio_used[RATIO_HISTORY_CHANGES];

// When we switched to each of the ratios in ratio_used[].
static int ratio_time[RATIO_HISTORY_CHANGES];

// True if we want to use whatever ratio is present on the next frame.
static int DetectAspectNow = FALSE;

RECT destinationRectangle = {0,0,0,0};
RECT sourceRectangle = {0,0,0,0};


//----------------------------------------------------------------------------
// Switch to a new aspect ratio and record it in the ratio history list.
void SwitchToRatio(int nMode, int nRatio)
{
	int now = GetTickCount();

	LOG("Switching to ratio %d", nRatio);

	aspect_mode = nMode;

	// If the most recent ratio switch just happened, don't remember it since it
	// was probably a transient ratio due to improperly locking onto a dark scene.
	if (now - ratio_time[0] > ShortRatioIgnoreMs)
	{
		memmove(&ratio_used[1], &ratio_used[0], sizeof(ratio_used[0]) * (RATIO_HISTORY_CHANGES - 1));
		memmove(&ratio_time[1], &ratio_time[0], sizeof(ratio_time[0]) * (RATIO_HISTORY_CHANGES - 1));
	}

	ratio_used[0] = nRatio;
	ratio_time[0] = GetTickCount();
	source_aspect = nRatio;
	WorkoutOverlaySize();
}

//----------------------------------------------------------------------------
// Enter or leave half-height mode.
void SetHalfHeight(int IsHalfHeight)
{
	if (IsHalfHeight != HalfHeight)
	{
		HalfHeight = IsHalfHeight;
		WorkoutOverlaySize();
	}
}


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
	CheckMenuItem(GetMenu(hWnd), IDM_SASPECT_AUTO, MF_UNCHECKED);

	if (AutoDetectAspect)
	{
		CheckMenuItem(GetMenu(hWnd), IDM_SASPECT_AUTO, MF_CHECKED);
	}
	else if (aspect_mode == 1)
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
	case IDM_SASPECT_COMPUTE:
	case IDM_SASPECT_CUSTOM:
	case IDM_SASPECT_AUTO:
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
			case IDM_ASPECT_FULLSCREEN:
				SwitchToRatio(AR_LETTERBOX, 1333);
				ShowText(hWnd, "4:3 Fullscreen Signal");
				break;
			case IDM_ASPECT_LETTERBOX:
				SwitchToRatio(AR_LETTERBOX, 1778);
				ShowText(hWnd, "1.78:1 Letterbox Signal");
				break;
			case IDM_ASPECT_ANAMORPHIC:
				SwitchToRatio(AR_ANAMORPHIC, 1778);
				ShowText(hWnd, "1.78:1 Anamorphic Signal");
				break;

			// Advanced Aspect Ratios
			case IDM_SASPECT_0:
				SwitchToRatio(AR_STRETCH, 0);
				ShowText(hWnd, "Stretch Video");
				break;
			case IDM_SASPECT_133:
				SwitchToRatio(AR_LETTERBOX, 1333);
				ShowText(hWnd, "4:3 Fullscreen Signal");
				break;
			case IDM_SASPECT_166:
				SwitchToRatio(AR_LETTERBOX, 1667);
				ShowText(hWnd, "1.66:1 Letterbox Signal");
				break;
			case IDM_SASPECT_178:
				SwitchToRatio(AR_LETTERBOX, 1778);
				ShowText(hWnd, "1.78:1 Letterbox Signal");
				break;
			case IDM_SASPECT_185:
				SwitchToRatio(AR_LETTERBOX, 1850);
				ShowText(hWnd, "1.85:1 Letterbox Signal");
				break;
			case IDM_SASPECT_200:
				SwitchToRatio(AR_LETTERBOX, 2000);
				ShowText(hWnd, "2.00:1 Letterbox Signal");
				break;
			case IDM_SASPECT_235:
				SwitchToRatio(AR_LETTERBOX, 2350);
				ShowText(hWnd, "2.35:1 Letterbox Signal");
				break;
			case IDM_SASPECT_166A:
				SwitchToRatio(AR_ANAMORPHIC, 1667);
				ShowText(hWnd, "1.66:1 Anamorphic Signal");
				break;
			case IDM_SASPECT_178A:
				SwitchToRatio(AR_ANAMORPHIC, 1778);
				ShowText(hWnd, "1.78:1 Anamorphic Signal");
				break;
			case IDM_SASPECT_185A:
				SwitchToRatio(AR_ANAMORPHIC, 1850);
				ShowText(hWnd, "1.85:1 Anamorphic Signal");
				break;
			case IDM_SASPECT_200A:
				SwitchToRatio(AR_ANAMORPHIC, 2000);
				ShowText(hWnd, "2.00:1 Anamorphic Signal");
				break;
			case IDM_SASPECT_235A:
				SwitchToRatio(AR_ANAMORPHIC, 2350);
				ShowText(hWnd, "2.35:1 Anamorphic Signal");
				break;
			case IDM_SASPECT_CUSTOM:
				SwitchToRatio(AR_ANAMORPHIC, custom_source_aspect);
				ShowText(hWnd, "Custom Aspect Ratio Signal");
				break;
			case IDM_SASPECT_COMPUTE:
				DetectAspectNow = TRUE;
				break;

			case IDM_SASPECT_AUTO:
				AutoDetectAspect = ! AutoDetectAspect;
				if (AutoDetectAspect)
				{
					ShowText(hWnd, "Auto Aspect Detect ON");
				}
				else
				{
					ShowText(hWnd, "Auto Aspect Detect OFF");
				}
				break;

			// Output Display Aspect Ratios
			case IDM_TASPECT_0:
				target_aspect = 0;
				ShowText(hWnd, "Aspect Ratio From Current Resolution");
				break;
			case IDM_TASPECT_133:
				target_aspect = 1330;
				ShowText(hWnd, "1.33:1 Screen");
				break;
			case IDM_TASPECT_166:
				target_aspect = 1667;
				ShowText(hWnd, "1.66:1 Screen");
				break;
			case IDM_TASPECT_178:
				target_aspect = 1778;
				ShowText(hWnd, "1.78:1 Screen");
				break;
			case IDM_TASPECT_185:
				target_aspect = 1850;
				ShowText(hWnd, "1.85:1 Screen");
				break;
			case IDM_TASPECT_200:
				target_aspect = 2000;
				ShowText(hWnd, "2.00:1 Screen");
				break;
			case IDM_TASPECT_235:
				target_aspect = 2350;
				ShowText(hWnd, "2.35:1 Screen");
				break;
			case IDM_TASPECT_CUSTOM:
				target_aspect = custom_target_aspect;
				ShowText(hWnd, "Custom Aspect Ratio Screen");
				break;
			default:
				break;
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
void PaintColorkey(HWND hWnd, BOOL bEnable)
{
	PAINTSTRUCT sPaint;
	// MRS 9-9-00
	HBRUSH black = CreateSolidBrush(RGB(0,0,0));
	HBRUSH overlay;
	BeginPaint(hWnd, &sPaint);

	if (bEnable && OverlayActive())
	{
		overlay = CreateSolidBrush(GetNearestColor(sPaint.hdc, OverlayColor));
	}
	else
	{
		overlay = CreateSolidBrush(RGB(0,0,0));
	}

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

		// Draw overlay color in the middle.
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
// Calculate the actual aspect ratio of the source frame, independent of
// grab or display resolutions.
double GetActualSourceFrameAspect()
{
	switch (aspect_mode) {
	case 1:
		// Letterboxed or full-frame
		return 4.0/3.0;
	case 2:
		// Anamorphic
		return 16.0/9.0;
	default:
		// User-specified
		return source_aspect/1000.0;
	}
}

//----------------------------------------------------------------------------
// Updates the window position/window state and enable/disable titlebar 
// as necessary.  This function should be globally used for everytime 
// you want to update the window everytime you have enabled/disabled the 
// statusbar, menus, full screen state, etc.
//
// This allows for more cosmetic handling - including the ability to 
// startup directly to maximized without any intermediate cosmetic
// glitches during startup.
//
void UpdateWindowState()
{
	if(bIsFullScreen == TRUE)
	{
		SetWindowLong(hWnd, GWL_STYLE, WS_VISIBLE);
		SetMenu(hWnd, NULL);
		ShowWindow(hwndStatusBar, SW_HIDE);
		SetWindowPos(hWnd,
					HWND_TOPMOST,
					0,
					0,
					GetSystemMetrics(SM_CXSCREEN),
					GetSystemMetrics(SM_CYSCREEN),
					SWP_SHOWWINDOW);
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
}


//----------------------------------------------------------------------------
// Calculate size and position coordinates for video overlay
// Takes into account of aspect ratio control.
void WorkoutOverlaySize()
{
	RECT rOverlayDest;
	RECT rOverlaySrc;

	int DestWidth, DestHeight;

	UpdateWindowState();

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

	Overlay_Update(&rOverlaySrc, &rOverlayDest, DDOVER_SHOW, TRUE);

	// MRS 9-9-00
	// Save the Overlay Destination and force a repaint 
	// Moved to after Overlay_Update in hopes of removing purple flashing.
	destinationRectangle = rOverlayDest;
	sourceRectangle = rOverlaySrc;
	ScreenToClient(hWnd,((PPOINT)&destinationRectangle));
	ScreenToClient(hWnd,((PPOINT)&destinationRectangle)+1);
	InvalidateRect(hWnd,NULL,FALSE);

	return;
}


//----------------------------------------------------------------------------
// Scan the top of a letterboxed image to find out where the picture starts.
//
// The basic idea is that we find a bounding rectangle for the image (which
// is assumed to be centered in the overlay buffer, an assumption the aspect
// ratio code makes in general) by searching from the top down to find the first
// scanline that isn't all black.  "All black" means that there aren't many
// pixels with luminance above a certain threshold.
//
// To support letterboxed movies shown on TV channels that put little channel
// logos in the corner, we allow the user to configure a maximum number of
// non-black pixels which will be ignored for purposes of the computation.
// By default this is 15% of the horizontal capture resolution, which will
// produce good results if the aspect ratio analysis is done on a bright scene.
//
// This function can almost certainly be made more efficient using MMX
// instructions.
int FindTopOfImage(short** EvenField, short **OddField)
{
	int y, x;
	int maxX = CurrentX - InitialOverscan * 2;
	int maxY = CurrentY / 2 - InitialOverscan;
	BYTE *pixel;
	int ignoreCount = IgnoreNonBlackPixels;
	int pixelCount;
	const int BytesBetweenLuminanceValues = 2;	// just for clarity's sake
	const int SkipPixels = 8;			// check fewer pixels to reduce CPU hit

	if (ignoreCount == 0)
	{
		// The user didn't specify an ignore count.  Default to ~15%
		// of the horizontal size of the image.
		ignoreCount = (CurrentX / 7) / SkipPixels;
	}
	else
	{
		ignoreCount /= SkipPixels;
		if (ignoreCount == 0)
			ignoreCount = 1;
	}

	for (y = InitialOverscan; y < maxY; y++)
	{
		if (y & 1)
			pixel = (BYTE *)OddField[y / 2];
		else
			pixel = (BYTE *)EvenField[y / 2];

		pixel += InitialOverscan * BytesBetweenLuminanceValues;

		pixelCount = -ignoreCount;
		for (x = InitialOverscan; x < maxX; x += SkipPixels)
		{
			if (((*pixel) & 0x7f) > LuminanceThreshold)
				pixelCount++;
			pixel += BytesBetweenLuminanceValues * SkipPixels;
		}

		if (pixelCount > 0)
			break;
	}

	return y;
}

int FindBottomOfImage(short** EvenField, short** OddField)
{
	int y, x;
	int maxX = CurrentX - InitialOverscan * 2;
	int maxY = CurrentY - InitialOverscan * 2;
	BYTE *pixel;
	int ignoreCount = IgnoreNonBlackPixels;
	int pixelCount;
	const int BytesBetweenLuminanceValues = 2;	// just for clarity's sake
	const int SkipPixels = 8;			// check fewer pixels to reduce CPU hit

	if (ignoreCount == 0)
	{
		// The user didn't specify an ignore count.  Default to ~15%
		// of the horizontal size of the image.
		ignoreCount = (CurrentX / 7) / SkipPixels;
	}
	else
	{
		ignoreCount /= SkipPixels;
		if (ignoreCount == 0)
			ignoreCount = 1;
	}

	for (y = maxY; y >= maxY / 2; y--)
	{
		if (y & 1)
			pixel = (BYTE *)OddField[y / 2];
		else
			pixel = (BYTE *)EvenField[y / 2];

		pixel += InitialOverscan * BytesBetweenLuminanceValues;

		pixelCount = -ignoreCount;
		for (x = InitialOverscan; x < maxX; x += SkipPixels)
		{
			if (((*pixel) & 0x7f) > LuminanceThreshold)
				pixelCount++;
			pixel += BytesBetweenLuminanceValues * SkipPixels;
		}

		if (pixelCount > 0)
			break;
	}

	return y;
}


//----------------------------------------------------------------------------
// Adjust the source aspect ratio to fit whatever is currently onscreen.
int FindAspectRatio(short** EvenField, short** OddField)
{
	int ratio;
	int topBorder, bottomBorder, border;
	int imageHeight = CurrentY - InitialOverscan * 2;

	// If the aspect mode is set to "use source", revert to assuming that the
	// source frame is 4:3.  We have to assume *some* source-frame aspect ratio
	// here or there aren't enough inputs to derive the material aspect ratio.
	if (aspect_mode == 0)
		aspect_mode = 1;

	// Find the top of the image relative to the overscan area.  Overscan has to
	// be discarded from the computations since it can't really be regarded as
	// part of the picture.
	topBorder = FindTopOfImage(EvenField, OddField) - InitialOverscan;

	// Now find the size of the border at the bottom of the image.
	bottomBorder = CurrentY - FindBottomOfImage(EvenField, OddField) - InitialOverscan * 2;

	// The border size is the smaller of the two.
	border = (topBorder < bottomBorder) ? topBorder : bottomBorder;

	// Now the material aspect ratio is simply
	//
	//	effective width / (total image height - number of black lines at top and bottom)
	//
	// We compute effective width from height using the source-frame aspect ratio, since
	// this will change depending on whether or not the image is anamorphic.
	ratio = (int)((imageHeight * 1000) * GetActualSourceFrameAspect() / (imageHeight - border * 2));
	//LOG("top %d bot %d bord %d rat %d", topBorder, bottomBorder, border, ratio);

	return ratio;
}

//----------------------------------------------------------------------------
// Continuously adjust the source aspect ratio.  This is called once per frame.
void AdjustAspectRatio(short** EvenField, short** OddField)
{
	static int lastNewRatio = 0;
	static int newRatioFrameCount = 0;
	int newRatio;
	int tick_count = GetTickCount();
	int tickCutoff = tick_count - (AspectHistoryTime * 1000);
	int i;
	int haveSeenThisRatio, haveSeenSmallerRatio;

	// If the user told us to detect the current ratio, do it.
	if (DetectAspectNow)
	{
		newRatio = FindAspectRatio(EvenField, OddField);
		SwitchToRatio(aspect_mode, newRatio);
		newRatioFrameCount = 0;
		DetectAspectNow = FALSE;
		return;
	}

	if (AutoDetectAspect)
	{
		newRatio = FindAspectRatio(EvenField, OddField);

		// If we've just crossed a 1-second boundary, scroll the aspect ratio
		// histories.  If not, update the max ratio found in the current second.
		if (tick_count / 1000 != min_ratio_tick_count / 1000)
		{
			min_ratio_tick_count = tick_count;
			memmove(&min_ratio_found[1], &min_ratio_found[0], sizeof(min_ratio_found[0]) * (RATIO_HISTORY_SECONDS - 1));
			min_ratio_found[0] = newRatio;
		}
		else if (newRatio < min_ratio_found[0]) {
			min_ratio_found[0] = newRatio;
		}

		// If the new ratio is less than the old one -- that is, if we've just
		// become less letterboxed -- switch to the new ratio immediately to
		// avoid cutting the image off.
		if (newRatio < source_aspect)
		{
			SwitchToRatio(aspect_mode, newRatio);
			newRatioFrameCount = 0;
		}
		else if (ABS(newRatio - source_aspect) > AspectEqualFudgeFactor && newRatio == lastNewRatio)
		{
			// Require the same aspect ratio for some number of frames, or no
			// narrower aspect ratio found for the last AspectConsistencyTime seconds,
			// before zooming in.
			haveSeenSmallerRatio = 0;
			for (i = 1; i < AspectConsistencyTime; i++)
				if (newRatio > min_ratio_found[i])
				{
					haveSeenSmallerRatio = 1;
					break;
				}

			if (! haveSeenSmallerRatio || ++newRatioFrameCount >= ZoomInFrameCount)
			{
				// If we're looking at aspect ratio histories, the new ratio must be
				// equal (or very close) to one we've _used_ in the recent past, or
				// there must have been no smaller ratio _found_ in the recent past.
				// That is, don't zoom in more than we've zoomed in recently unless
				// we'd previously zoomed to the same ratio.  This helps prevent
				// temporary zooms into letterboxed material during dark scenes, while
				// allowing the code to quickly switch in and out of zoomed mode when
				// full-frame commercials come on.
				haveSeenThisRatio = 0;
				if (AspectHistoryTime > 0)
				{
					for (i = 1; i < RATIO_HISTORY_CHANGES && ratio_time[i] > tickCutoff; i++)
						if (ABS(newRatio - ratio_used[i]) <= AspectEqualFudgeFactor)
						{
							haveSeenThisRatio = 1;
							break;
						}
				}

				if (AspectConsistencyTime <= 0 ||
					(haveSeenThisRatio && newRatioFrameCount >= ZoomInFrameCount) ||
					! haveSeenSmallerRatio)
				{
					SwitchToRatio(aspect_mode, newRatio);
				}
			}
		}
		else
		{
			// If this is a wider ratio than the previous one, require it to stick
			// around for the full frame count.
			if (lastNewRatio < newRatio)
				newRatioFrameCount = 0;

			lastNewRatio = newRatio;
		}
	}
}


//----------------------------------------------------------------------------
// Returns the current source rectangle.
void GetSourceRect(RECT *rect)
{
	memcpy(rect, &sourceRectangle, sizeof(RECT));
}
