/////////////////////////////////////////////////////////////////////////////
// deinterlace.c
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
// 09 Nov 2000   Tom Barry		       Added Blended Clipping Deinterlace method
//
// 30 Dec 2000   Mark Rejhon           Split out deinterlace routines
//                                     into separate modules
//
// 05 Jan 2001   John Adcock           Added flip frequencies to DeintMethods
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 09 Jan 2001   John Adcock           Split out memcpySSE as separate function
//                                     Changed DeintMethods to reflect the two
//                                     film mode functions replacing the one before
//                                     Moved CombFactor and CompareFields to new file
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "deinterlace.h"
#include "cpu.h"
#include "bt848.h"
#include "dTV.h"

long BitShift = 13;
long EdgeDetect = 625;
long JaggieThreshold = 73;
long DiffThreshold = 224;
long TemporalTolerance = 300;
long SpatialTolerance = 600;
long SimilarityThreshold = 25;

DEINTERLACE_METHOD DeintMethods[PULLDOWNMODES_LAST_ONE] =
{
	// VIDEO_MODE_BOB
	{"Video Deinterlace (Bob)", "Bob", FALSE, FALSE, FALSE, FALSE, DeinterlaceFieldBob, 50, 60},
	// VIDEO_MODE_WEAVE
	{"Video Deinterlace (Weave)", "Weave", FALSE, FALSE, FALSE, FALSE, DeinterlaceFieldWeave, 50, 60},
	// VIDEO_MODE_2FRAME
	{"Video Deinterlace (2-Frame)", "2-Frame", FALSE, FALSE, FALSE, FALSE, DeinterlaceFieldTwoFrame, 50, 60},
	// SIMPLE_WEAVE
	{"Simple Weave", NULL, FALSE, FALSE, FALSE, FALSE, Weave, 50, 60},
	// SIMPLE_BOB
	{"Simple Bob", NULL, FALSE, FALSE, FALSE, FALSE, Bob, 50, 60},
	// SCALER_BOB
	{"Scaler Bob", NULL, FALSE, FALSE, TRUE, FALSE, HalfHeightBoth, 50, 60},
	// FILM_22_PULLDOWN_ODD
	{"2:2 Pulldown Flip on Odd", "2:2 Odd", FALSE, FALSE, FALSE, TRUE, FilmModePAL, 25, 30},
	// FILM_22_PULLDOWN_EVEN
	{"2:2 Pulldown Flip on Even", "2:2 Even", FALSE, FALSE, FALSE, TRUE, FilmModePAL, 25, 30},
	// FILM_32_PULLDOWN_0
	{"3:2 Pulldown Skip 1st Full Frame", "3:2 1st", FALSE, FALSE, FALSE, TRUE, FilmModeNTSC, 1000, 24},
	// FILM_32_PULLDOWN_1
	{"3:2 Pulldown Skip 2nd Full Frame", "3:2 2nd", FALSE, FALSE, FALSE, TRUE, FilmModeNTSC, 1000, 24},
	// FILM_32_PULLDOWN_2
	{"3:2 Pulldown Skip 3rd Full Frame", "3:2 3rd", FALSE, FALSE, FALSE, TRUE, FilmModeNTSC, 1000, 24},
	// FILM_32_PULLDOWN_3
	{"3:2 Pulldown Skip 4th Full Frame", "3:2 4th", FALSE, FALSE, FALSE, TRUE, FilmModeNTSC, 1000, 24},
	// FILM_32_PULLDOWN_4
	{"3:2 Pulldown Skip 5th Full Frame", "3:2 5th", FALSE, FALSE, FALSE, TRUE, FilmModeNTSC, 1000, 24},
	// EVEN_ONLY
	{"Even Scanlines Only", "Even", FALSE, FALSE, TRUE, FALSE, HalfHeightEvenOnly, 25, 30},
	// ODD_ONLY
	{"Odd Scanlines Only", "Odd", FALSE, FALSE, TRUE, FALSE, HalfHeightOddOnly, 25, 30},
	// BLENDED_CLIP
	{"Blended Clip", NULL, FALSE, FALSE, FALSE, FALSE, BlendedClipping, 50, 60},
	// ADAPTIVE
	{"Adaptive", NULL, TRUE, FALSE, FALSE, FALSE, AdaptiveDeinterlace, 50, 60},
};

/////////////////////////////////////////////////////////////////////////////
// memcpyMMX
// Uses MMX instructions to move memory around
// does as much as we can in 64 byte chunks (128-byte on SSE machines)
// using MMX instructions
// then copies any extra bytes
// assumes there will be at least 64 bytes to copy
// This code was originally from Borg's bTV plugin SDK 
/////////////////////////////////////////////////////////////////////////////
void memcpyMMX(void *Dest, void *Src, size_t nBytes)
{
	__asm {
		mov		esi, dword ptr[Src]
		mov		edi, dword ptr[Dest]
		mov		ecx, nBytes
		shr     ecx, 6                      // nBytes / 64
align 8
CopyLoop:
		movq	mm0, qword ptr[esi]
		movq	mm1, qword ptr[esi+8*1]
		movq	mm2, qword ptr[esi+8*2]
		movq	mm3, qword ptr[esi+8*3]
		movq	mm4, qword ptr[esi+8*4]
		movq	mm5, qword ptr[esi+8*5]
		movq	mm6, qword ptr[esi+8*6]
		movq	mm7, qword ptr[esi+8*7]
		movq	qword ptr[edi], mm0
		movq	qword ptr[edi+8*1], mm1
		movq	qword ptr[edi+8*2], mm2
		movq	qword ptr[edi+8*3], mm3
		movq	qword ptr[edi+8*4], mm4
		movq	qword ptr[edi+8*5], mm5
		movq	qword ptr[edi+8*6], mm6
		movq	qword ptr[edi+8*7], mm7
		add		esi, 64
		add		edi, 64
		loop CopyLoop
		mov		ecx, nBytes
		and     ecx, 63
		cmp     ecx, 0
		je EndCopyLoop
align 8
CopyLoop2:
		mov dl, byte ptr[esi] 
		mov byte ptr[edi], dl
		inc esi
		inc edi
		dec ecx
		jne near CopyLoop2
EndCopyLoop:
		emms
	}
}

#ifdef USE_SSE
/////////////////////////////////////////////////////////////////////////////
// memcpySSE
// On SSE machines, we can use the 128-bit floating-point registers and
// bypass write caching to copy a bit faster.  The destination has to be
// 16-byte aligned.  
/////////////////////////////////////////////////////////////////////////////
void memcpySSE(void *Dest, void *Src, size_t nBytes)
{
	__asm {
		mov		esi, dword ptr[Src]
		mov		edi, dword ptr[Dest]
		mov		ecx, nBytes
		shr     ecx, 7                      // nBytes / 128
align 8
CopyLoopSSE:
		// movaps would be slightly more efficient but the capture data
		// isn't reliably 16-byte aligned.
		movups	xmm0, xmmword ptr[esi]
		movups	xmm1, xmmword ptr[esi+16*1]
		movups	xmm2, xmmword ptr[esi+16*2]
		movups	xmm3, xmmword ptr[esi+16*3]
		movups	xmm4, xmmword ptr[esi+16*4]
		movups	xmm5, xmmword ptr[esi+16*5]
		movups	xmm6, xmmword ptr[esi+16*6]
		movups	xmm7, xmmword ptr[esi+16*7]
		movntps	xmmword ptr[edi], xmm0
		movntps	xmmword ptr[edi+16*1], xmm1
		movntps	xmmword ptr[edi+16*2], xmm2
		movntps	xmmword ptr[edi+16*3], xmm3
		movntps	xmmword ptr[edi+16*4], xmm4
		movntps	xmmword ptr[edi+16*5], xmm5
		movntps	xmmword ptr[edi+16*6], xmm6
		movntps	xmmword ptr[edi+16*7], xmm7
		add		esi, 128
		add		edi, 128
		loop CopyLoopSSE
		mov		ecx, nBytes
		and     ecx, 127
		cmp     ecx, 0
		je EndCopyLoopSSE
align 8
CopyLoop2SSE:
		mov dl, byte ptr[esi] 
		mov byte ptr[edi], dl
		inc esi
		inc edi
		dec ecx
		jne near CopyLoop2SSE
EndCopyLoopSSE:
		emms
	}
}

#endif