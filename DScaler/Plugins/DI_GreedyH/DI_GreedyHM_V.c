/////////////////////////////////////////////////////////////////////////////
// $Id: DI_GreedyHM_V.c,v 1.2 2001-07-25 12:04:31 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Tom Barry.  All rights reserved.
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
// 01 Jul 2001   Tom Barry		       Break out Greedy (High Motion) Deinterlace, w/Vert Filter
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

// This version handles Greedy High Motion with Vertical Filtering

#include "windows.h"
#include "DS_Deinterlace.h"
#include "DI_GreedyHM.h"

// debugging options
#undef USE_JAGGIE_REDUCTION
#undef USE_GREEDY_CHOICE
#undef USE_CLIP
#undef USE_BOB_BLEND

#define USE_JAGGIE_REDUCTION
#define USE_GREEDY_CHOICE
#define USE_CLIP
#define USE_BOB_BLEND


#define FUNCT_NAME DI_GreedyHM_V

BOOL FUNCT_NAME()
{
#include "DI_GreedyHM2.h"
	int line;				// number of lines
	int	LoopCtr;				// number of qwords in line - 1
	int	LoopCtrW;				// number of qwords in line - 1

	int L1;						// offset to FieldStore elem holding top known pixels
	int L3;						// offset to FieldStore elem holding bottom known pxl
	int L2;						// offset to FieldStore elem holding newest weave pixels
	int L2P;					// offset to FieldStore elem holding prev weave pixels
	__int64* pFieldStore;		// ptr into FieldStore qwords
	__int64* pFieldStoreEnd;	// ptr to Last FieldStore qword
	__int64* pL2;				// ptr into FieldStore[L2] 
	BYTE* WeaveDest;					// dest for weave pixel
	BYTE* CopyDest;				// other dest, copy or vertical filter
	int CopySrc;

	int DestIncr = 2 * OverlayPitch;  // we go throug overlay buffer 2 lines per
	__int64 LastAvg=0;					//interp value from left qword



	// set up pointers, offsets
	SetFsPtrs(&L1, &L2, &L2P, &L3, &CopySrc, &CopyDest, &WeaveDest);
	L2 = __min(L2, L2P);				// Subscript to 1st of 2 possible weave pixels, our base addr
	L1 = (L1 - L2) * 8;					// now is signed offset from L2  
	L3 = (L3 - L2) * 8;					// now is signed offset from L2  
	pFieldStore = & FieldStore[0];		// starting ptr into FieldStore[L2]
	pFieldStoreEnd = & FieldStore[FieldHeight * FSCOLCT];		// ending ptr into FieldStore[L2]
	pL2 = & FieldStore[L2];				// starting ptr into FieldStore[L2]
	LoopCtrW = LineLength / 8 - 1;		// do 8 bytes at a time, adjusted

	for (line = 0; line < (FieldHeight); ++line)
	{
		LoopCtr = LoopCtrW;				// actually qword counter
		if (WeaveDest == lpCurOverlay)    // on first line may just copy first and last
		{
			FieldStoreCopy(lpCurOverlay, &FieldStore[CopySrc], LineLength);
			WeaveDest += DestIncr;		// bump for next, CopyDest already OK
			pL2 = & FieldStore[L2 + FSCOLCT];
		}
		else
		{			

		_asm		// should indent here but I can't read it
		{
		mov		edi, WeaveDest				// get ptr to line ptrs	
		mov		esi, dword ptr [pL2]		// addr of our 1st qword in FieldStore
		mov		eax, L1						// offset to top 	
		mov		ebx, L3						// offset to top comb 	
		mov		ecx, OverlayPitch			// overlay pitch
		mov		word ptr [LastAvg+6], 0     // init left avg lazy way
		
		lea     edx, [esi+eax]				// where L1 would point
		cmp		edx, pFieldStore			// before begin of fieldstore?
		jnb		L1OK						// n, ok
		mov		eax, ebx					// else use this for top pixel vals
L1OK:
		lea     edx, [esi+ebx]				// where L2 would point
		cmp		edx, pFieldStoreEnd			// after end of fieldstore?
		jb		L3OK						// n, ok
		mov		ebx, eax					// else use this bottom pixel vals

L3OK:		
		mov		edx, CopyDest

		align 8
QwordLoop:
		}
#define FSOFFS 0 * FSCOLSIZE				// following include needs an offset
#include "DI_GreedyDeLoop.asm"
		_asm
		{
		pavgb   mm1, mm4
		movntq	qword ptr[edx], mm1  // avg clipped best with above line
		pavgb   mm3, mm4
		movntq	qword ptr[edi], mm3	// avg clipped best with below line

		// bump ptrs and loop for next qword
		lea		edx,[edx+8]				// bump CopyDest
		lea		edi,[edi+8]				// bump WeaveDest
		lea		esi,[esi+FSCOLSIZE]			
		dec		LoopCtr
		jg		QwordLoop			

// Ok, done with one line
		mov		esi, pL2				// addr of our 1st qword in FieldStore
		lea     esi, [esi+FSROWSIZE]    // bump to next row
		mov		pL2, esi				// addr of our 1st qword in FieldStore for line
		mov     edi, WeaveDest			// ptr to curr overlay buff line start
		add     edi, DestIncr			// but we want to skip 1
		mov		WeaveDest, edi			// update for next loop
		mov     edx, CopyDest			// ptr to curr overlay buff line start
		add     edx, DestIncr			// but we want to skip 1
		mov		CopyDest, edx			// update for next loop
		sfence
		emms
		}
		}		// should undent here but I can't read it
	}

  return TRUE;
}	
		
