/////////////////////////////////////////////////////////////////////////////
// OSD.h
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
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 8 Nov 2000    Michael Eskin         Initial onscreen display
//
// 28 Nov 2000   Mark Rejhon           Reorganization and visual improvements
//
/////////////////////////////////////////////////////////////////////////////

// MAE 8 Nov 2000
// Added defines for on-screen display timer
#define OSD_FONT				"Arial"
#define	OSD_COLOR_OUTLINE		RGB(0,0,0)
#define OSD_COLOR_FILL			RGB(0,255,0)
#define OSD_TIMER_ID			42
#define OSD_TIMER_DELAY			4000

void OSD_ShowText(HWND hWnd,char *szText);
void OSD_Redraw(HWND hWnd);
void OSD_Clear(HWND hWnd);
void OSD_ShowVideoSource(HWND hWnd, int nVideoSource);
