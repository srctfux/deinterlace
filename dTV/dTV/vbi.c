/////////////////////////////////////////////////////////////////////////////
// VBI.c
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
// 02 Jan 2001   John Adcock           Started VBI Clean Up
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VBI.h"
#include "bt848.h"
#include "VBI_VideoText.h"
#include "VBI_CCdecode.h"

int VBI_Flags = VBI_VT | VBI_VPS | VBI_CC;

BOOL bStopVBI;
HANDLE VBIThread;

BYTE VBI_thresh;
BYTE VBI_off;
int vtstep;
BOOL VTLarge=TRUE;

BOOL Capture_VBI = FALSE;

HWND ShowVTInfo=NULL;
HWND ShowVPSInfo=NULL;

struct TPacket30 Packet30;

void VBI_Init()
{
	VBI_VT_Init();
}

void VBI_Exit()
{
	VBI_VT_Exit();
}

void VBI_DecodeLine(unsigned char *VBI_Buffer, int line, BOOL IsOdd)
{
	vtstep = (int) ((28.636363 / 5.72725) * FPFAC + 0.5);

	// set up threshold and offset data
	VBI_AGC(VBI_Buffer, 120, 450, 1);

	/* all kinds of data with videotext data format: videotext, intercast, ... */
	if (VBI_Flags & VBI_VT)
	{
		VT_DecodeLine(VBI_Buffer);
	}

	// Closed caption information appears on line 21 (line == 11) for NTSC
	// it also appears on PAL videos at line 22
	// see http://www.wgbh.org/wgbh/pages/captioncenter/cctechfacts4.html
	// for more infomation
	if ((VBI_Flags & VBI_CC) && (line == 11/* || line == 10 || line == 12*/)) 
	{
		CC_DecodeLine(VBI_Buffer);
	}

	/* VPS information with channel name, time, VCR programming info, etc. */
	if ((VBI_Flags & VBI_VPS) && (line == 9))
	{
		VTS_DecodeLine(VBI_Buffer);
	}
}

void VBI_AGC(BYTE * Buffer, int start, int stop, int step)
{
	int i, min = 255, max = 0;

	for (i = start; i < stop; i += step)
	{
		if (Buffer[i] < min)
		{
			min = Buffer[i];
		}
		else if (Buffer[i] > max)
		{
			max = Buffer[i];
		}
	}
	VBI_thresh = (max + min) / 2;
	VBI_off = 128 - VBI_thresh;
}
