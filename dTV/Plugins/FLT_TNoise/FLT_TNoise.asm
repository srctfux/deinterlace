/////////////////////////////////////////////////////////////////////////////
// FLT_TNoise.asm
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 Steven Grimm.  All rights reserved.
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
// This is the implementation of the noise filter described in Noise.c.
// It is broken out into this separate file because most of the logic is
// the same for the different processor variants, and doing it this way
// means we only have to maintain one copy of the code.
//

#ifdef IS_SSE
#define MAINLOOP_LABEL NoiseLoop_SSE
#endif
#ifdef IS_3DNOW
#define MAINLOOP_LABEL NoiseLoop_3DNow
#endif
#ifdef IS_MMX
#define MAINLOOP_LABEL NoiseLoop_MMX
#endif

{
#ifdef IS_MMX
	const __int64 qwAvgMask = 0xFEFEFEFEFEFEFEFE;
#endif

	_asm {
		mov ecx, Cycles
		mov eax, dword ptr[NewPixel]
		mov ebx, dword ptr[OldPixel]
		movq mm5, qwNoiseThreshold		// mm5 = NoiseThreshold

MAINLOOP_LABEL:

		movq mm0, qword ptr[eax]		// mm0 = NewPixel
		movq mm1, qword ptr[ebx]		// mm1 = OldPixel
		movq mm2, mm0					// mm2 = NewPixel

		// Now determine the weighted averages of the old and new pixel values.
#ifdef IS_SSE
		pavgb mm2, mm1					// mm2 = avg(NewPixel, OldPixel)
		pavgb mm2, mm1					// mm2 = avg(NewPixel, OldPixel, OldPixel, OldPixel)
#endif
#ifdef IS_3DNOW
		pavgusb mm2, mm1				// mm2 = avg(NewPixel, OldPixel)
		pavgusb mm2, mm1				// mm2 = avg(NewPixel, OldPixel, OldPixel, OldPixel)
#endif
#ifdef IS_MMX
		movq mm3, mm1					// mm3 = OldPixel
		movq mm4, qwAvgMask				// mm4 = mask to remove lower bits of bytes
		pand mm3, mm4					// mm3 = OldPixel with LSBs removed
		pand mm2, mm4					// mm4 = OldPixel with LSBs removed
		psrlw mm3, 1					// mm3 = OldPixel / 2
		psrlw mm2, 1					// mm2 = NewPixel / 2
		paddusb mm2, mm3				// mm2 = avg(NewPixel, OldPixel)
		pand mm2, mm4					// mm2 = avg(NewPixel, OldPixel) with LSBs removed
		psrlw mm2, 1					// mm2 = avg(NewPixel, OldPixel) / 2
		paddusb mm2, mm3				// mm2 = avg(NewPixel, OldPixel, OldPixel, OldPixel)
#endif

		// Figure out which pixels are sufficiently different from their predecessors
		// to be considered new.  There is, unfortunately, no absolute-difference
		// MMX instruction, so we OR together two unsigned saturated differences
		// (one of which will always be zero).
		movq mm3, mm0					// mm3 = NewPixel
		psubusb mm3, mm1				// mm3 = max(NewPixel - OldPixel, 0)
		movq mm4, mm1					// mm4 = OldPixel
		psubusb mm4, mm0				// mm4 = max(OldPixel - NewPixel, 0)
		por mm3, mm4					// mm3 = abs(NewPixel - OldPixel)
		
		// Filter out pixels whose differences are less than the threshold.
		psubusb mm3, mm5				// mm3 = max(0, abs(NewPixel - OldPixel) - threshold)

		// Turn the filtered list into a mask.  While we've been operating on bytes
		// up to now since we want to treat luminance and chroma differences as
		// equal for purposes of comparing against the threshold, now we have to
		// treat pixels as a whole.
		pxor mm4, mm4					// mm4 = 0
		pcmpgtw mm3, mm4				// mm3 = 0xFFFF where abs(NewPixel - OldPixel) > threshold
		movq mm6, mm3					// mm6 = 0xFFFF where abs(NewPixel - OldPixel) > threshold
		pandn mm6, mm2					// mm6 = weighted avg. where abs(NewPixel - OldPixel) <= threshold
		pand mm3, mm0					// mm3 = NewPixel where abs(NewPixel - OldPixel) > threshold
		por mm3, mm6					// mm3 = result pixels

		movq qword ptr[eax], mm3

		add eax, 8
		add ebx, 8
		loop MAINLOOP_LABEL

		emms
	}
}

#undef MAINLOOP_LABEL