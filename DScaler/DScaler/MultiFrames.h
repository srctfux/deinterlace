/////////////////////////////////////////////////////////////////////////////
// $Id: MultiFrames.h,v 1.7 2003-03-23 09:24:27 laurentg Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 Laurent Garnier.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//  This file is subject to the terms of the GNU General Public License as
//  published by the Free Software Foundation.  A copy of this license is
//  included with this software distribution in the file COPYING.  If you
//  do not have a copy, you may obtain a copy by writing to the Free
//  Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// $Log: not supported by cvs2svn $
// Revision 1.6  2003/03/22 18:58:40  laurentg
// New key to switch to or from preview mode
// Spped up initial display of channels in preview mode
//
// Revision 1.5  2003/03/22 15:41:58  laurentg
// Half height deinterlace modes correctly handled in previow mode
// Center of the image in its frame with black borders
//
// Revision 1.4  2003/03/21 22:48:07  laurentg
// Preview mode (multiple frames) improved
//
// Revision 1.3  2003/03/20 23:27:28  laurentg
// Navigation through frames updated - bugs fixed - few comments added
//
// Revision 1.2  2003/03/17 22:34:29  laurentg
// First step for the navigation through channels in preview mode
//
// Revision 1.1  2003/03/16 18:27:46  laurentg
// New multiple frames feature
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __MULTIFRAMES_H___
#define __MULTIFRAMES_H___


#include "Source.h"


// Different possible modes
enum eMultiFramesMode {
    PREVIEW_CHANNELS = 0,
    PREVIEW_STILLS,
};


#include "DS_ApiCommon.h"


class CMultiFrames
{
public:
    CMultiFrames(eMultiFramesMode eMode, int iNbCols, int iNbRows, CSource* pSource);
    ~CMultiFrames();

	eMultiFramesMode GetMode();
	CSource* GetSource();
	int GetWidth();
	int GetHeight();
	BOOL IsActive();
	void Enable();
	void Disable();
	BOOL IsSwitchRequested();
	void RequestSwitch();
	void DoSwitch();
	void Reset();
	void SelectFrame();
	void UpdateFrame(TDeinterlaceInfo* pInfo, BOOL* bUseExtraBuffer, BYTE** lpBuffer, int* Pitch);
	BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);

protected:

private:
	// Allocate memory buffer to store the picture containing all the frames
	void AllocateMemoryBuffer();

	// Free previously allocated memory buffer to store the picture containing all the frames
	void FreeMemoryBuffer();

	void SelectFrameBuffer(int iFrame, BOOL bIncludingBorder, BYTE** lpFrameBuffer, int *iFramePitch, int *iFrameWidth, int *iFrameHeight);

	void DrawBorder(int iFrame, BOOL bIncludingExternalBorder, int iLuminLevel, unsigned int iLeftThick, unsigned int iRightThick, unsigned int iTopThick, unsigned int iBottomThick);

	// Add a border around each frame - boder size is 2 pixels
	// Color is different for the current active frame and other frames
	void DrawBorders();

	// Shift all frames in the grid
	//
	// A positive value for iDeltaFrames means that some frames will
	// disappear at top left and new ones will appeared at bottom right
	// A negative value for iDeltaFrames means that some frames will
	// disappear at bottom right and new ones will appeared at top left
	void ShiftFrames(int iDeltaFrames);

	// Move the content of one frame from one position to another one
	void MoveFrame(int iFrameSrc, int iFrameDest);

	// Paint in black the content of a frame
	void ResetFrameToBlack(int iFrame);

	// Kind of mode : channels or stills
	eMultiFramesMode m_Mode;

	// The screen is cut in m_NbCols columns and m_NbRows rows,
	// that is a total of m_NbFrames frames
	int m_NbFrames;
	int m_NbCols;
	int m_NbRows;

	// Width and height in pixels of the picture containing all the frames
	int m_Width;
	int m_Height;

	// Memory buffer containing all the frames
	BYTE* m_MemoryBuffer;

	// The active frame in the grid
	int m_CurrentFrame;

	// Indicate if multiple frames mode is ON or OFF
	BOOL m_Active;

	BOOL bSwitchRequested;
	int* bFrameFilled;
	BOOL bNavigAllowed;
	int iDeltaNewFrame;
	CSource* m_Source;
};


/// Global instance of the MultiFrames object
extern CMultiFrames* pMultiFrames;


#endif
