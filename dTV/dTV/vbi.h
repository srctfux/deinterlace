/////////////////////////////////////////////////////////////////////////////
// vbi.h
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
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __VBI_H___
#define __VBI_H___

#define MAXVTDIALOG 8

#define VBI_VT  1
#define VBI_VPS 2
#define VBI_CC  32

#define FPSHIFT 16
#define FPFAC (1<<FPSHIFT)


typedef struct TVTDialog
{
	HWND Dialog;
	int  Page;
	int  SubPage;
	int  FramePos;
	BOOL Large;
	BOOL PageChange;
	unsigned char AsciiBuffer[26][40];
};

struct TPacket30
{
	// Type 0 data

	struct
	{
		unsigned nMag:3;
		unsigned nPage:8;
		unsigned:5; // Unused
		WORD nSubcode;
	} HomePage;
	WORD NetId;
	struct
	{
		char Offset;
		DWORD JulianDay;
		BYTE Hour;
		BYTE Min;
		BYTE Sec;
	} UTC;
	char Unknown[5];
	char Identifier[21];

	// Type 2 data

	struct
	{
		unsigned LCI:2;
		unsigned LUF:1;
		unsigned PRF:1;
		unsigned PCS:2;
		unsigned MI:1;
		unsigned day:5;
		unsigned month:4;
		unsigned hour:5;
		unsigned minute:6;
		unsigned:5;
		WORD CNI;
		WORD PTY;
	} PDC;
};

typedef struct TVTPage
{
	WORD wCtrl;
	BOOL Fill;
	BYTE Frame[25][40];
    BYTE LineUpdate[25];
	BYTE bUpdated;
};

typedef struct TVT
{
    unsigned short SubCount;
	struct TVTPage *SubPage;
};

void VBI_Init();
void VBI_Exit();
void VBI_DecodeLine(unsigned char *VBI_Buffer, int line);
void VBI_AGC(BYTE * Buffer, int start, int stop, int step);

extern int VBI_Flags;
extern int VBI_FPS;
extern BYTE VBI_thresh;
extern BYTE VBI_off;
extern BOOL Capture_VBI;
extern BOOL VTLarge;
extern char VPS_lastname[9];
extern HWND ShowVPSInfo;
extern HWND ShowVTInfo;
extern struct TPacket30 Packet30;

extern struct TVT VTFrame[800];
extern struct TVTDialog VTDialog[MAXVTDIALOG];
extern unsigned short VTColourTable[9];

#endif
