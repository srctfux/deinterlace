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
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "deinterlace.h"
#include "globals.h"
#include "cpu.h"

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
	{"Video Deinterlace (Bob)", FALSE, FALSE, FALSE, FALSE, DeinterlaceFieldBob, 50, 60},
	// VIDEO_MODE_WEAVE
	{"Video Deinterlace (Weave)", FALSE, FALSE, FALSE, FALSE, DeinterlaceFieldWeave, 50, 60},
	// VIDEO_MODE_2FRAME
	{"Video Deinterlace (2-Frame)", FALSE, FALSE, FALSE, FALSE, DeinterlaceFieldTwoFrame, 50, 60},
	// SIMPLE_WEAVE
	{"Sipmle Weave", FALSE, FALSE, FALSE, FALSE, Weave, 50, 60},
	// SIMPLE_BOB
	{"Sipmle Bob", FALSE, FALSE, FALSE, FALSE, Bob, 50, 60},
	// SCALER_BOB
	{"Scaler Bob", FALSE, FALSE, TRUE, FALSE, HalfHeightBoth, 50, 60},
	// FILM_22_PULLDOWN_ODD
	{"2:2 Pulldown Flip on Odd", FALSE, FALSE, FALSE, TRUE, FilmMode, 25, 30},
	// FILM_22_PULLDOWN_EVEN
	{"2:2 Pulldown Flip on Even", FALSE, FALSE, FALSE, TRUE, FilmMode, 25, 30},
	// FILM_32_PULLDOWN_0
	{"3:2 Pulldown Skip 1st Full Frame", FALSE, FALSE, FALSE, TRUE, FilmMode, 0, 24},
	// FILM_32_PULLDOWN_1
	{"3:2 Pulldown Skip 2nd Full Frame", FALSE, FALSE, FALSE, TRUE, FilmMode, 0, 24},
	// FILM_32_PULLDOWN_2
	{"3:2 Pulldown Skip 3rd Full Frame", FALSE, FALSE, FALSE, TRUE, FilmMode, 0, 24},
	// FILM_32_PULLDOWN_3
	{"3:2 Pulldown Skip 4th Full Frame", FALSE, FALSE, FALSE, TRUE, FilmMode, 0, 24},
	// FILM_32_PULLDOWN_4
	{"3:2 Pulldown Skip 5th Full Frame", FALSE, FALSE, FALSE, TRUE, FilmMode, 0, 24},
	// EVEN_ONLY
	{"Even Scanlines Only", FALSE, FALSE, TRUE, FALSE, HalfHeightEvenOnly, 25, 30},
	// ODD_ONLY
	{"Odd Scanlines Only", FALSE, FALSE, TRUE, FALSE, HalfHeightOddOnly, 25, 30},
	// BLENDED_CLIP
	{"Blended Clip", FALSE, FALSE, FALSE, FALSE, BlendedClipping, 18, 15},
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
#ifdef USE_SSE
	// On SSE machines, we can use the 128-bit floating-point registers and
	// bypass write caching to copy a bit faster.  The destination has to be
	// 16-byte aligned.  
	if ((CpuFeatureFlags & FEATURE_SSE) && (((long) Dest) & 15) == 0)
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
	else
#endif /* USE_SSE */
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

///////////////////////////////////////////////////////////////////////////////
// GetCombFactor
//
// This routine basically calculates how close the pixels in pLines2
// are the interpelated pixels between pLines1
// this idea was taken from the VirtualDub CVideoTelecineRemover class
// at the moment it is the correct algoritm outlined in the comments
// not the one used in that program
// I only do this on the Y component as I assume that any noticable combing
// will be visible in the black and white image
// the relative sizes of the returns from this function will be used to 
// determine the best ordering of the fields
// This function only works on the area displayed so will perform better if any
// VBI lines are off screen
// the BitShift value is used to filter out noise and quantization error
///////////////////////////////////////////////////////////////////////////////
long GetCombFactor(short** pLines1, short** pLines2)
{
	int Line;
	long LineFactor;
	long CombFactor = 0;
	short* YVal1;
	short* YVal2;
	short* YVal3;
	long ActiveX = CurrentX - 2 * InitialOverscan;
	const __int64 YMask    = 0x00ff00ff00ff00ff;
	const __int64 qwOnes = 0x0001000100010001;
	__int64 wBitShift    = BitShift;

	__int64 qwEdgeDetect;
	__int64 qwThreshold;
	const __int64 Mask = 0x7f7f7f7f7f7f7f7f;

	// If one of the fields is missing, treat them as very different.
	if (pLines1 == NULL || pLines2 == NULL)
		return 0x7fffffff;

	qwEdgeDetect = EdgeDetect;
	qwEdgeDetect += (qwEdgeDetect << 48) + (qwEdgeDetect << 32) + (qwEdgeDetect << 16);
	qwThreshold = JaggieThreshold;
	qwThreshold += (qwThreshold << 48) + (qwThreshold << 32) + (qwThreshold << 16);

	for (Line = 100; Line < ((CurrentY - 100) / 2); ++Line)
	{
		YVal1 = pLines1[Line] + InitialOverscan;
		YVal2 = pLines2[Line] + InitialOverscan;
		YVal3 = pLines1[Line + 1] + InitialOverscan;
		_asm
		{
			mov ecx, ActiveX
			mov eax,dword ptr [YVal1]
			mov ebx,dword ptr [YVal2]
			mov edx,dword ptr [YVal3]
			shr ecx, 2       // there are ActiveX * 2 / 8 qwords
		    movq mm1, YMask
			pxor mm0, mm0    // mm0 = 0
align 8
Next8Bytes:
			movq mm3, qword ptr[eax] 
			movq mm4, qword ptr[ebx] 
			movq mm5, qword ptr[edx]

			pand mm3, YMask
			pand mm4, YMask
			pand mm5, YMask

			// work out (O1 - E) * (O2 - E) - EdgeDetect * (O1 - O2) ^ 2 >> 12
			// result will be in mm6

			psrlw mm3, 01
			psrlw mm4, 01
			psrlw mm5, 01

			movq mm6, mm3
			psubw mm6, mm4		//mm6 = O1 - E

			movq mm7, mm5
			psubw mm7, mm4		//mm7 = O2 - E

			pmullw mm6, mm7		// mm0 = (O1 - E) * (O2 - E)

			movq mm7, mm3
			psubw mm7, mm5		// mm7 = (O1 - O2)
			pmullw mm7, mm7		// mm7 = (O1 - O2) ^ 2
			psrlw mm7, 12		// mm7 = (O1 - O2) ^ 2 >> 12
			pmullw mm7, qwEdgeDetect		// mm1  = EdgeDetect * (O1 - O2) ^ 2 >> 12

			psubw mm6, mm7      // mm6 is what we want

			pcmpgtw mm6, qwThreshold

			pand mm6, qwOnes

			paddw mm0, mm6

			add eax, 8
			add ebx, 8
			add edx, 8

			dec ecx
			jne near Next8Bytes

			movd eax, mm0
			psrlq mm0,32
			movd ecx, mm0
			add ecx, eax
			mov dword ptr[LineFactor], ecx
			emms
		}
		CombFactor += (LineFactor & 0xFFFF);
		CombFactor += (LineFactor >> 16);
	}
	return CombFactor;
}

///////////////////////////////////////////////////////////////////////////////
// CompareFields
//
// This routine basically calculates how close the pixels in pLines2
// are to the pixels in pLines1
// this is my attempt to implement Mark Rejhon's 3:2 pulldown code
// we will use this to dect the times when we get three fields in a row from
// the same frame
// the result is the total average diffrence between the Y components of each pixel
// This function only works on the area displayed so will perform better if any
// VBI lines are off screen
// the BitShift value is used to filter out noise and quantization error
///////////////////////////////////////////////////////////////////////////////
long CompareFields(short** pLines1, short** pLines2, RECT *rect)
{
	int Line;
	long LineFactor;
	long DiffFactor = 0;
	short* YVal1;
	short* YVal2;
	long ActiveX = rect->right - rect->left;
	const __int64 YMask    = 0x00ff00ff00ff00ff;
	__int64 wBitShift    = BitShift;

	// If we skipped a field, treat the new one as maximally different.
	if (pLines1 == NULL || pLines2 == NULL)
		return 0x7fffffff;
	if (rect == NULL)
		return 0;

	for (Line = rect->top / 2; Line < rect->bottom / 2; ++Line)
	{
		YVal1 = pLines1[Line] + (rect->left & ~1);
		YVal2 = pLines2[Line] + (rect->left & ~1);
		_asm
		{
			mov ecx, ActiveX
			mov eax,dword ptr [YVal1]
			mov ebx,dword ptr [YVal2]
			shr ecx, 2		 // there are ActiveX * 2 / 8 qwords
		    movq mm1, YMask
			movq mm7, wBitShift
			pxor mm0, mm0    // mm0 = 0  this is running total
align 8
Next8Bytes:
			movq mm4, qword ptr[eax] 
			movq mm5, qword ptr[ebx] 
			pand mm5, mm1    // get only Y compoment
			pand mm4, mm1    // get only Y compoment

			psubw mm4, mm5   // mm4 = Y1 - Y2
			pmaddwd mm4, mm4 // mm4 = (Y1 - Y2) ^ 2
			psrld mm4, mm7   // divide mm4 by 2 ^ Bitshift
			paddd mm0, mm4   // keep total in mm0

			add eax, 8
			add ebx, 8
			
			dec ecx
			jne near Next8Bytes

			movd eax, mm0
			psrlq mm0,32
			movd ecx, mm0
			add ecx, eax
			mov dword ptr[LineFactor], ecx
			emms
		}
		DiffFactor += (long)sqrt(LineFactor);
	}
	return DiffFactor;
}

