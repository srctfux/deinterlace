/////////////////////////////////////////////////////////////////////////////
// VBI_CCdecode.h
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1998 Timecop.  All rights reserved.
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
// 24 Jul 2000   John Adcock           Added Header file
//                                     removed xwindows calls
//                                     put definitions in header
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __CCDECODE_H___
#define __CCDECODE_H___

int CC_DecodeLine(BYTE* vbiline);
int CC_DecodeBit(BYTE* data, int threshold);
BOOL CC_IsParityOK(int n);
int CC_WrapAtWord(char *src, char *dest, char *rem);

#define WRAP_POINT 76

#endif
