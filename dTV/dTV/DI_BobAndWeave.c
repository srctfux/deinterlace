/////////////////////////////////////////////////////////////////////////////
// DI_BobAndWeave.c
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
// Based on code from Virtual Dub Plug-in by Gunnar Thalin
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
// 30 Dec 2000   Mark Rejhon           Split into separate module
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "deinterlace.h"
#include "globals.h"
#include "cpu.h"

///////////////////////////////////////////////////////////////////////////////
// DeinterlaceFieldBob
//
// Deinterlaces a field with a tendency to bob rather than weave.  Best for
// high-motion scenes like sports.
//
// The algorithm for this was taken from the 
// Deinterlace - area based Vitual Dub Plug-in by
// Gunnar Thalin
///////////////////////////////////////////////////////////////////////////////
void DeinterlaceFieldBob(short** pOddLines, short** pEvenLines, short** pPrevLines, BYTE* lpCurOverlay, BOOL bIsOdd)
{
	int Line;
	short* YVal1;
	short* YVal2;
	short* YVal3;
	BYTE* Dest;
	
	const __int64 YMask    = 0x00ff00ff00ff00ff;
	const __int64 UVMask    = 0xff00ff00ff00ff00;

	__int64 qwEdgeDetect;
	__int64 qwThreshold;
	const __int64 Mask = 0x7f7f7f7f7f7f7f7f;

	pPrevLines = NULL;	// stop the compiler from whining
	qwEdgeDetect = EdgeDetect;
	qwEdgeDetect += (qwEdgeDetect << 48) + (qwEdgeDetect << 32) + (qwEdgeDetect << 16);
	qwThreshold = JaggieThreshold;
	qwThreshold += (qwThreshold << 48) + (qwThreshold << 32) + (qwThreshold << 16);


	// copy first even line no matter what, and the first odd line if we're
	// processing an odd field.
	memcpyMMX(lpCurOverlay, pEvenLines[0], CurrentX * 2);
	if (bIsOdd)
		memcpyMMX(lpCurOverlay + OverlayPitch, pOddLines[0], CurrentX * 2);

	for (Line = 0; Line < (CurrentY / 2 - 1); ++Line)
	{
		if (bIsOdd)
		{
			YVal1 = pOddLines[Line];
			YVal2 = pEvenLines[Line + 1];
			YVal3 = pOddLines[Line + 1];
			Dest = lpCurOverlay + (Line * 2 + 2) * OverlayPitch;
		}
		else
		{
			YVal1 = pEvenLines[Line];
			YVal2 = pOddLines[Line];
			YVal3 = pEvenLines[Line + 1];
			Dest = lpCurOverlay + (Line * 2 + 1) * OverlayPitch;
		}

		// For ease of reading, the comments below assume that we're operating on an odd
		// field (i.e., that bIsOdd is true).  The exact same processing is done when we
		// operate on an even field, but the roles of the odd and even fields are reversed.
		// It's just too cumbersome to explain the algorithm in terms of "the next odd
		// line if we're doing an odd field, or the next even line if we're doing an
		// even field" etc.  So wherever you see "odd" or "even" below, keep in mind that
		// half the time this function is called, those words' meanings will invert.

		// Copy the odd line to the overlay verbatim.
		memcpyMMX(Dest + OverlayPitch, YVal3, CurrentX * 2);

		_asm
		{
			mov ecx, CurrentX
			mov eax, dword ptr [YVal1]
			mov ebx, dword ptr [YVal2]
			mov edx, dword ptr [YVal3]
			mov edi, dword ptr [Dest]
			shr ecx, 2       // there are CurrentX * 2 / 8 qwords

align 8
DoNext8Bytes:			
			movq mm0, qword ptr[eax] 
			movq mm1, qword ptr[ebx] 
			movq mm2, qword ptr[edx]

			// get intensities in mm3 - 4
			movq mm3, mm0
			movq mm4, mm1
			movq mm5, mm2

			pand mm3, YMask
			pand mm4, YMask
			pand mm5, YMask

			// get average in mm0
			psrlw mm0, 01
			psrlw mm2, 01
			pand  mm0, Mask
			pand  mm2, Mask
			paddw mm0, mm2

			// work out (O1 - E) * (O2 - E) / 2 - EdgeDetect * (O1 - O2) ^ 2 >> 12
			// result will be in mm6

			psrlw mm3, 01
			psrlw mm4, 01
			psrlw mm5, 01

			movq mm6, mm3
			psubw mm6, mm4	//mm6 = O1 - E

			movq mm7, mm5
			psubw mm7, mm4	//mm7 = O2 - E

			pmullw mm6, mm7		// mm0 = (O1 - E) * (O2 - E)

			movq mm7, mm3
			psubw mm7, mm5		// mm7 = (O1 - O2)
			pmullw mm7, mm7		// mm7 = (O1 - O2) ^ 2
			psrlw mm7, 12		// mm7 = (O1 - O2) ^ 2 >> 12
			pmullw mm7, qwEdgeDetect		// mm7  = EdgeDetect * (O1 - O2) ^ 2 >> 12

			psubw mm6, mm7      // mm6 is what we want

			pcmpgtw mm6, qwThreshold

			movq mm7, mm6

			pand mm0, mm6

			pandn mm7, mm1

			por mm7, mm0

			movq qword ptr[edi], mm7

			add eax, 8
			add ebx, 8
			add edx, 8
			add edi, 8
			dec ecx
			jne near DoNext8Bytes
			emms
		}
	}

	// Copy last odd line if we're processing an even field.
	if (! bIsOdd)
		memcpyMMX(lpCurOverlay + (CurrentY - 1) * OverlayPitch, pOddLines[CurrentY / 2 - 1], CurrentX * 2);
}


///////////////////////////////////////////////////////////////////////////////
// Deinterlace the latest field, with a tendency to weave rather than bob.
// Good for high detail on low-movement scenes.
//
// The algorithm is described in comments below.
//
void DeinterlaceFieldWeave(short** pOddLines, short** pEvenLines, short** pPrevLines, BYTE* lpCurOverlay, BOOL bIsOdd)
{
	int Line;
	short* YVal1;
	short* YVal2;
	short* YVal3;
	short* YVal4;
	BYTE* OldStack;
	BYTE* Dest;

	const __int64 YMask    = 0x00ff00ff00ff00ff;
	const __int64 UVMask    = 0xff00ff00ff00ff00;

	__int64 qwSpatialTolerance;
	__int64 qwTemporalTolerance;
	__int64 qwThreshold;
	const __int64 Mask = 0xfefefefefefefefe;

	qwSpatialTolerance = SpatialTolerance;
	qwSpatialTolerance += (qwSpatialTolerance << 48) + (qwSpatialTolerance << 32) + (qwSpatialTolerance << 16);
	qwTemporalTolerance = TemporalTolerance;
	qwTemporalTolerance += (qwTemporalTolerance << 48) + (qwTemporalTolerance << 32) + (qwTemporalTolerance << 16);
	qwThreshold = SimilarityThreshold;
	qwThreshold += (qwThreshold << 48) + (qwThreshold << 32) + (qwThreshold << 16);

	// copy first even line no matter what, and the first odd line if we're
	// processing an even field.
	memcpyMMX(lpCurOverlay, pEvenLines[0], CurrentX * 2);
	if (! bIsOdd)
		memcpyMMX(lpCurOverlay + OverlayPitch, pOddLines[0], CurrentX * 2);

	for (Line = 0; Line < (CurrentY / 2 - 1); ++Line)
	{
		if (bIsOdd)
		{
			YVal1 = pEvenLines[Line];
			YVal2 = pOddLines[Line];
			YVal3 = pEvenLines[Line + 1];
			YVal4 = pPrevLines[Line];
			Dest = lpCurOverlay + (Line * 2 + 1) * OverlayPitch;
		}
		else
		{
			YVal1 = pOddLines[Line];
			YVal2 = pEvenLines[Line + 1];
			YVal3 = pOddLines[Line + 1];
			YVal4 = pPrevLines[Line + 1];
			Dest = lpCurOverlay + (Line * 2 + 2) * OverlayPitch;
		}

		// For ease of reading, the comments below assume that we're operating on an odd
		// field (i.e., that bIsOdd is true).  The exact same processing is done when we
		// operate on an even field, but the roles of the odd and even fields are reversed.
		// It's just too cumbersome to explain the algorithm in terms of "the next odd
		// line if we're doing an odd field, or the next even line if we're doing an
		// even field" etc.  So wherever you see "odd" or "even" below, keep in mind that
		// half the time this function is called, those words' meanings will invert.

		// Copy the even scanline below this one to the overlay buffer, since we'll be
		// adapting the current scanline to the even lines surrounding it.  The scanline
		// above has already been copied by the previous pass through the loop.
		memcpyMMX(Dest + OverlayPitch, YVal3, CurrentX * 2);

		_asm
		{
			mov dword ptr[OldStack], esi

			mov ecx, CurrentX
			mov eax, dword ptr [YVal1]
			mov ebx, dword ptr [YVal2]
			mov edx, dword ptr [YVal3]
			mov esi, dword ptr [YVal4]
			mov edi, dword ptr [Dest]
			shr ecx, 2       // there are ActiveX * 2 / 8 qwords

align 8
DoNext8Bytes:			
			movq mm0, qword ptr[eax]		// mm0 = E1
			movq mm1, qword ptr[ebx]		// mm1 = O
			movq mm2, qword ptr[edx]		// mm2 = E2

			movq mm3, mm0					// mm3 = intensity(E1)
			movq mm4, mm1					// mm4 = intensity(O)
			movq mm6, mm2					// mm6 = intensity(E2)
			pand mm3, YMask
			pand mm4, YMask
			pand mm6, YMask

			// Average E1 and E2 for interpolated bobbing.
#if 0
			// The following SSE instruction doesn't work on older CPUs, but is faster for
			// newer ones.  Comment it out until we have CPU type detection.
			// This is the hex encoding for "pavg mm0,mm2" which VC++'s assembler doesn't
			// seem to recognize.
			_emit 0x0F						// mm0 = avg(E1, E2)
			_emit 0xE0
			_emit 0xC2
#else
			pand mm0, Mask					// mm0 = E1 with lower chroma bit stripped off
			psrlw mm0, 1					// mm0 = E1 / 2
			pand mm2, Mask					// mm2 = E2 with lower chroma bit stripped off
			psrlw mm2, 1					// mm2 = E2 / 2
			paddb mm0, mm2					// mm2 = (E1 + E2) / 2
#endif

			// The meat of the work is done here.  We want to see whether this pixel is
			// close in luminosity to ANY of: its top neighbor, its bottom neighbor,
			// or its predecessor.  To do this without branching, we use MMX's
			// saturation feature, which gives us Z(x) = x if x>=0, or 0 if x<0.
			//
			// The formula we're computing here is
			//		Z(ST - (E1 - O) ^ 2) + Z(ST - (E2 - O) ^ 2) + Z(TT - (Oold - O) ^ 2)
			// where ST is spatial tolerance and TT is temporal tolerance.  The idea
			// is that if a pixel is similar to none of its neighbors, the resulting
			// value will be pretty low, probably zero.  A high value therefore indicates
			// that the pixel had a similar neighbor.  The pixel in the same position
			// in the field before last (Oold) is considered a neighbor since we want
			// to be able to display 1-pixel-high horizontal lines.

			movq mm7, qwSpatialTolerance
			movq mm5, mm3					// mm5 = E1
			psubsw mm5, mm4					// mm5 = E1 - O
			psraw mm5, 1
			pmullw mm5, mm5					// mm5 = (E1 - O) ^ 2
			psubusw mm7, mm5				// mm7 = ST - (E1 - O) ^ 2, or 0 if that's negative

			movq mm3, qwSpatialTolerance
			movq mm5, mm6					// mm5 = E2
			psubsw mm5, mm4					// mm5 = E2 - O
			psraw mm5, 1
			pmullw mm5, mm5					// mm5 = (E2 - O) ^ 2
			psubusw mm3, mm5				// mm0 = ST - (E2 - O) ^ 2, or 0 if that's negative
			paddusw mm7, mm3				// mm7 = (ST - (E1 - O) ^ 2) + (ST - (E2 - O) ^ 2)

			movq mm3, qwTemporalTolerance
			movq mm5, qword ptr[esi]		// mm5 = Oold
			pand mm5, YMask
			psubsw mm5, mm4					// mm5 = Oold - O
			psraw mm5, 1 // XXX
			pmullw mm5, mm5					// mm5 = (Oold - O) ^ 2
			psubusw mm3, mm5				// mm0 = TT - (Oold - O) ^ 2, or 0 if that's negative
			paddusw mm7, mm3				// mm7 = our magic number

			// Now compare the similarity totals against our threshold.  The pcmpgtw
			// instruction will populate the target register with a bunch of mask bits,
			// filling words where the comparison is true with 1s and ones where it's
			// false with 0s.  A few ANDs and NOTs and an OR later, we have bobbed
			// values for pixels under the similarity threshold and weaved ones for
			// pixels over the threshold.

			pcmpgtw mm7, qwThreshold		// mm7 = 0xffff where we're greater than the threshold, 0 elsewhere
			movq mm6, mm7					// mm6 = 0xffff where we're greater than the threshold, 0 elsewhere
			pand mm7, mm1					// mm7 = weaved data where we're greater than the threshold, 0 elsewhere
			pandn mm6, mm0					// mm6 = bobbed data where we're not greater than the threshold, 0 elsewhere
			por mm7, mm6					// mm7 = bobbed and weaved data

			movq qword ptr[edi], mm7
			add eax, 8
			add ebx, 8
			add edx, 8
			add edi, 8
			add esi, 8
			dec ecx
			jne near DoNext8Bytes
			emms

			mov esi, dword ptr[OldStack]
		}
	}

	// Copy last odd line if we're processing an odd field.
	if (bIsOdd)
		memcpyMMX(lpCurOverlay + (CurrentY - 1) * OverlayPitch, pOddLines[CurrentY / 2 - 1], CurrentX * 2);
}
