/////////////////////////////////////////////////////////////////////////////
// vt.c
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
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "vbi.h"
#include "bt848.h"
#include "vt.h"
#include "ccdecode.h"

HANDLE VBI_Event=NULL;
BOOL bStopVBI;
HANDLE VBIThread;

int VBI_lpf = 19;   // lines per field
BYTE VBI_thresh;
BYTE VBI_off;
int vtstep;

int VBIProcessor = 0;

void VBI_Start()
{
	DWORD LinkThreadID;
	
	bStopVBI = FALSE;
	VBI_Event = CreateEvent(NULL, FALSE, FALSE, NULL);
	ResetEvent(VBI_Event);

	VBIThread = CreateThread(NULL, 0, VBI_DecodeThread, NULL, (DWORD) 0, &LinkThreadID);
}

void VBI_Stop()
{
	DWORD ExitCode;
	int i;
	BOOL Thread_Stopped = FALSE;

	if (VBIThread != NULL)
	{
		i = 5;
		SetThreadPriority(VBIThread, THREAD_PRIORITY_NORMAL);
		bStopVBI = TRUE;
		SetEvent(VBI_Event);
		while(i > 0 && !Thread_Stopped)
		{
			if (GetExitCodeThread(VBIThread, &ExitCode) == TRUE)
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
			TerminateThread(VBIThread, 0);
		}
		Sleep(50);
		CloseHandle(VBIThread);
		VBIThread = NULL;
	}
	CloseHandle(VBI_Event);
}

DWORD WINAPI VBI_DecodeThread(LPVOID lpThreadParameter)
{
	BYTE *pVBI;
	DWORD VBI_Tic_Count = 0;
	int vbi_frames;
	int line;
	int ProcessorMask;
	
	ProcessorMask = 1 << (VBIProcessor);
	SetThreadAffinityMask(GetCurrentThread(), ProcessorMask);

	vbi_frames = 0;
	VBI_Tic_Count = GetTickCount();


	vtstep = (int) ((28.636363 / 5.72725) * FPFAC + 0.5);

	while(!bStopVBI)
	{
		if (WaitForSingleObject(VBI_Event, INFINITE) == WAIT_OBJECT_0)
		{
			ResetEvent(VBI_Event);
			if (BT848_IsVideoPresent())
			{
				vbi_frames++;

				// we get here because we have just got an odd field.
				// that should mean that all the previous VBI data
				// is availiable
				pVBI = (LPBYTE) Vbi_dma[((CurrentFrame + 4) % 5)]->dwUser;

				for (line = 0; line < VBI_lpf * 2; line++)
				{
					VBI_DecodeLine(pVBI + line * 2048, line);
				}

				if (VBI_Tic_Count + 960 <= GetTickCount())
				{
					VBI_FPS = vbi_frames + 1;
					if (VBI_FPS > 25)
						VBI_FPS = 25;
					vbi_frames = 0;
					VBI_Tic_Count = GetTickCount();
				}
			}
		}
	}
	ExitThread(0);
	return 0;
}

void VBI_DecodeLine(unsigned char *VBI_Buffer, int line)
{
	if (line >= VBI_lpf)
	{
		line -= VBI_lpf;
	}

	// set up threshold and offset data
	VBI_AGC(VBI_Buffer, 120, 450, 1);

	/* all kinds of data with videotext data format: videotext, intercast, ... */
	if (((VBI_Flags & VBI_VT) || (VBI_Flags & VBI_IC)))
	{
		VT_DecodeLine(VBI_Buffer);
	}

	// Closed caption information appears on line 21 (line == 11) for NTSC
	// it also appears on PAL videos at line 22
	// see http://www.wgbh.org/wgbh/pages/captioncenter/cctechfacts4.html
	// for more infomation
	if ((VBI_Flags & VBI_CC) && (line == 11 || line == 12))
	{
		CC_DecodeLine(VBI_Buffer);
	}

	/* VPS information with channel name, time, VCR programming info, etc. */
	if ((VBI_Flags & VBI_VPS) && (line == 9))
	{
		VTS_DecodeLine(VBI_Buffer);
	}

	/* Video_Dat_Stuff  */
	if ((VBI_Flags & VBI_VD) && ((line == 17) || (line == 18)))
	{
		VDAT_DecodeLine(VBI_Buffer);
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
