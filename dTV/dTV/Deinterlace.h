/////////////////////////////////////////////////////////////////////////////
// deinterlace.h
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
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Put all my deinterlacing code into this
//                                     file
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __DEINTERLACE_H___
#define __DEINTERLACE_H___

void memcpyMMX(void *Dest, void *Src, size_t nBytes);
void memcpyBOBMMX(void *Dest, void *Src, size_t nBytes);
void VideoDeinterlaceMMX(void *Dest, void *SrcUp, void *SrcSame, void *SrcDown, size_t nBytes);
void DeinterlaceField(short** pOddLines, short** pEvenLines, BYTE* lpCurOverlay, BOOL bIsOdd);
long GetCombFactor(short** pLines1, short** pLines2);
long CompareFields(short** pLines1, short** pLines2);
long CompareFields2(short** pLines1, short** pLines2);

extern long BitShift;
extern long EdgeDetect;
extern long JaggieThreshold;
extern long DiffThreshold;


#endif
