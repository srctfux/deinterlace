/////////////////////////////////////////////////////////////////////////////
// VBI_VideoText.h
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

#ifndef __VT_H___
#define __VT_H___

#define LARGE_WIDTH 15
#define LARGE_HEIGHT 18

#define SMALL_WIDTH 10
#define SMALL_HEIGHT 12

#define VTTimeout 60000

#define VT_LARGE_BITMAP_HEIGHT (LARGE_HEIGHT*25)
#define VT_LARGE_BITMAP_WIDTH (LARGE_WIDTH*40)

#define VT_SMALL_BITMAP_HEIGHT (SMALL_HEIGHT*25)
#define VT_SMALL_BITMAP_WIDTH (SMALL_WIDTH*40)


#define ROUNDUP(w) (((w)+3)&~3)
#define _BitmapDataP(bmp)	(bmp->bmiHeader.biBitCount==8?(((BYTE*)(bmp))+sizeof(BITMAPINFOHEADER)+sizeof(WORD)*256):(((BYTE*)(bmp))+sizeof(BITMAPINFOHEADER)))
#define _BitmapDataR(bmp)	(((BYTE*)(bmp))+sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)*256)
#define _BitmapPalR(bmp)	((RGBQUAD*)(((BYTE*)(bmp))+sizeof(BITMAPINFOHEADER)))

#define _BitmapLargeRowR(bmp,row) (_BitmapDataR(bmp)+ROUNDUP(bmp->bmiHeader.biWidth)*(bmp->bmiHeader.biHeight-(row)*LARGE_HEIGHT))
#define _BitmapLargeChar(bmp,c)	(_BitmapLargeRowR(bmp,(c)/32)+((c)&0x1f)*LARGE_WIDTH)

#define _BitmapLargeRowP(bmp,row) ((_BitmapDataP(bmp))+VT_LARGE_BITMAP_WIDTH*((row)*LARGE_HEIGHT))
#define _BitmapLargeText(bmp,row,c) (_BitmapLargeRowP(bmp,row)+((c)*LARGE_WIDTH*2))


#define _BitmapSmallRowR(bmp,row) (_BitmapDataR(bmp)+ROUNDUP(bmp->bmiHeader.biWidth)*(bmp->bmiHeader.biHeight-(row)*SMALL_HEIGHT))
#define _BitmapSmallChar(bmp,c)	(_BitmapSmallRowR(bmp,(c)/32)+((c)&0x1f)*SMALL_WIDTH)

#define _BitmapSmallRowP(bmp,row) ((_BitmapDataP(bmp))+VT_SMALL_BITMAP_WIDTH*((row)*SMALL_HEIGHT))
#define _BitmapSmallText(bmp,row,c) (_BitmapSmallRowP(bmp,row)+((c)*SMALL_WIDTH*2))


#define _MakePage(mag,page) (((mag)?(mag):8)*100+((page)>>4)*10+((page)&0xf))


void VBI_VT_Init();
void VBI_VT_Exit();

BYTE DeHam(BYTE byte);
BYTE ReverseDeHam(BYTE b);
void StorePage(BYTE mag, BYTE page);
void StorePacket30();

unsigned char unham(unsigned char *d);
unsigned char unham2(unsigned char *d);
unsigned char VBI_Scan(BYTE * Buffer, unsigned int step);

void VBI_decode_vt(unsigned char *dat);
void VBI_decode_line(unsigned char *d, int line);

int New_Dialog_Slot(HWND hwnd);
int Get_Dialog_Slot(HWND hwnd);
int Del_Dialog_Slot(HWND hwnd);
void VT_ChannelChange();

void VT_DecodeLine(BYTE* VBI_Buffer);
void VTS_DecodeLine(BYTE* VBI_Buffer);

extern unsigned short UTPages[12];


#endif
