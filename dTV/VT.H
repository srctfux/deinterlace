/////////////////////////////////////////////////////////////////////////////
// vt.h
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

#ifndef __VT_H___
#define __VT_H___

#include "defines.h"
#include "structs.h"
#include "globals.h"

void VideoDat_Exit( void );
void VideoDat_Init( void );
BYTE DeHam(BYTE byte);
BYTE ReverseDeHam(BYTE b);
void StorePage(BYTE mag, BYTE page);
void StorePacket30();

int VBI_VDatScan(BYTE * VBI_Buffer, unsigned int step, int BytePos);
void Work_VideoDat(unsigned char *Buffer);
void VBI_VDat_Blockaustausch(void);
int VBI_VDAT_DecodeBlockz(void);
void VBI_VDat_Filename(struct TVDatBlockz Block, char *fNames);
void VBI_VDAT_SOTInfo(struct TVDatBlockz Block, struct SOTREC *Info);
BOOL VBI_VDat_WriteData(struct TVDatBlockz Block);
unsigned int VBI_VDat_FileSize(struct TVDatBlockz Block);
void VideoDat_Flush(void);

DWORD WINAPI DoVBI(LPVOID lpThreadParameter);

void VBI_decode_vps(unsigned char *data);
unsigned char unham(unsigned char *d);
unsigned char unham2(unsigned char *d);
unsigned char VBI_Scan(BYTE * Buffer, unsigned int step);

void VBI_AGC(BYTE * Buffer, int start, int stop, int step);
void VBI_decode_vt(unsigned char *dat);
void VBI_decode_line(unsigned char *d, int line);

void Start_VBI();
void Stop_VBI();

int New_Dialog_Slot(HWND hwnd);
int Get_Dialog_Slot(HWND hwnd);
int Del_Dialog_Slot(HWND hwnd);
void VT_ChannelChange();
void format_page(int Page, int SubPage, BOOL reveal, struct fmt_page *pg);
int html_output(HWND hwnd, char *name, int latin1, BOOL HtmlNewLine, BOOL HtmlHeaders, BOOL StrippedHtml, struct fmt_page *pg);
void Export_VT_Page(HWND hwnd, int Page, int SubPage);

#endif