/////////////////////////////////////////////////////////////////////////////
// DI_TwoFrame.asm
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

// This is the implementation of the two-frame deinterlace algorithm described in
// DI_TwoFrame.c.  It's in a separate file so we can compile variants for different
// CPU types; most of the code is the same in the different variants.

#ifdef IS_SSE
#define MAINLOOP_LABEL DoNext8Bytes_SSE
#endif
#ifdef IS_3DNOW
#define MAINLOOP_LABEL DoNext8Bytes_3DNow
#endif
#ifdef IS_MMX
#define MAINLOOP_LABEL DoNext8Bytes_MMX
#endif

	_asm
	{
		// We'll be using a couple registers that have meaning in the C code, so
		// save them.
		mov OldSI, esi
		mov OldSP, esp

		// Figure out what to do with the scanline above the one we just copied.
		// See above for a description of the algorithm.

		mov ecx, LineLength
		mov eax, dword ptr [YVal0]		// eax = T1
		mov ebx, dword ptr [YVal1]		// ebx = M1
		mov esp, dword ptr [YVal2]		// esp = B1
		mov edx, dword ptr [OVal0]		// edx = T0
		mov esi, dword ptr [OVal1]		// esi = M0
		shr ecx, 3						// there are LineLength / 8 qwords

align 8
MAINLOOP_LABEL:

		mov edi, dword ptr [OVal2]		// edi = B0
		movq mm0, qword ptr[eax]		// mm0 = T1
		movq mm1, qword ptr[esp]		// mm1 = B1
		movq mm2, qword ptr[ebx]		// mm2 = M1

		// Average T1 and B1 so we can do interpolated bobbing if we bob onto T1.
		movq mm7, mm1					// mm7 = B1
#ifdef IS_SSE
		pavgb mm7, mm0
#endif
#ifdef IS_3DNOW
		pavgusb mm7, mm0
#endif
#ifdef IS_MMX
		movq mm5, mm0					// mm5 = T1
		psrlw mm7, 1					// mm7 = B1 / 2
		pand mm7, Mask					// mask off lower bits
		psrlw mm5, 1					// mm5 = T1 / 2
		pand mm5, Mask					// mask off lower bits
		paddw mm7, mm5					// mm7 = (T1 + B1) / 2
#endif
		movq qwBobbedPixels, mm7

		// Now that we've averaged them, we no longer care about the chroma
		// values of T1 and B1 (all our comparisons are luminance-only).
		pand mm0, YMask					// mm0 = luminance(T1)
		pand mm1, YMask					// mm1 = luminance(B1)

		// Find out whether M1 is new.  "New" means the square of the
		// luminance difference between M1 and M0 is less than the temporal
		// tolerance.
		//
		movq mm7, mm2					// mm7 = M1
		movq mm4, qword ptr[esi]		// mm4 = M0
		pand mm7, YMask					// mm7 = luminance(M1)
		movq mm6, mm7					// mm6 = luminance(M1)     used below
		pand mm4, YMask					// mm4 = luminance(M0)
		psubsw mm7, mm4					// mm7 = M1 - M0
		psraw mm7, 1					// mm7 = M1 - M0 (see SQUARING NOTE above)
		pmullw mm7, mm7					// mm7 = (M1 - M0) ^ 2
		pcmpgtw mm7, qwTemporalTolerance // mm7 = 0xffff where (M1 - M0) ^ 2 > threshold, 0x0000 otherwise

		// Find out how different T1 and M1 are.
		movq mm3, mm0					// mm3 = T1
		psubsw mm3, mm6					// mm3 = T1 - M1
		psraw mm3, 1					// mm3 = T1 - M1 (see SQUARING NOTE above)
		pmullw mm3, mm3					// mm3 = (T1 - M1) ^ 2
		pcmpgtw mm3, qwSpatialTolerance	// mm3 = 0xffff where (T1 - M1) ^ 2 > threshold, 0x0000 otherwise

		// Find out how different B1 and M1 are.
		movq mm4, mm1					// mm4 = B1
		psubsw mm4, mm6					// mm4 = B1 - M1
		psraw mm4, 1					// mm4 = B1 - M1 (see SQUARING NOTE above)
		pmullw mm4, mm4					// mm4 = (B1 - M1) ^ 2
		pcmpgtw mm4, qwSpatialTolerance	// mm4 = 0xffff where (B1 - M1) ^ 2 > threshold, 0x0000 otherwise

		// We care about cases where M1 is different from both T1 and B1.
		pand mm3, mm4					// mm3 = 0xffff where M1 is different from T1 and B1, 0x0000 otherwise

		// Find out whether T1 is new.
		movq mm4, mm0					// mm4 = T1
		movq mm5, qword ptr[edx]		// mm5 = T0
		pand mm5, YMask					// mm5 = luminance(T0)
		psubsw mm4, mm5					// mm4 = T1 - T0
		psraw mm4, 1					// mm4 = T1 - T0 (see SQUARING NOTE above)
		pmullw mm4, mm4					// mm4 = (T1 - T0) ^ 2 / 4
		pcmpgtw mm4, qwTemporalTolerance // mm4 = 0xffff where (T1 - T0) ^ 2 > threshold, 0x0000 otherwise

		// Find out whether B1 is new.
		movq mm5, mm1					// mm5 = B1
		movq mm6, qword ptr[edi]		// mm6 = B0
		pand mm6, YMask					// mm6 = luminance(B0)
		psubsw mm5, mm6					// mm5 = B1 - B0
		psraw mm5, 1					// mm5 = B1 - B0 (see SQUARING NOTE above)
		pmullw mm5, mm5					// mm5 = (B1 - B0) ^ 2
		pcmpgtw mm5, qwTemporalTolerance // mm5 = 0xffff where (B1 - B0) ^ 2 > threshold, 0x0000 otherwise

		// We care about cases where M1 is old and either T1 or B1 is old.
		por mm4, mm5					// mm4 = 0xffff where T1 or B1 is new
		por mm4, mm7					// mm4 = 0xffff where T1 or B1 or M1 is new
		movq mm6, qwAllOnes				// mm6 = 0xffffffffffffffff
		pxor mm4, mm6					// mm4 = 0xffff where T1 and B1 and M1 are old

		// Pick up the interpolated (T1+B1)/2 pixels.
		movq mm1, qwBobbedPixels		// mm1 = (T1 + B1) / 2

		// At this point:
		//  mm1 = (T1+B1)/2
		//  mm2 = M1
		//  mm3 = mask, 0xffff where M1 is different from both T1 and B1
		//  mm4 = mask, 0xffff where T1 and B1 and M1 are old
		//  mm6 = 0xffffffffffffffff
		//
		// Now figure out where we're going to weave and where we're going to bob.
		// We'll weave if all pixels are old or M1 isn't different from both its
		// neighbors.
		pxor mm3, mm6					// mm3 = 0xffff where M1 is the same as either T1 or B1
		por mm3, mm4					// mm3 = 0xffff where M1 and T1 and B1 are old or M1 = T1 or B1
		pand mm2, mm3					// mm2 = woven data where T1 or B1 isn't new or they're different
		pandn mm3, mm1					// mm3 = bobbed data where T1 or B1 is new and they're similar
		por mm3, mm2					// mm3 = finished pixels

		// Shuffle some registers around since there aren't enough of them
		// to hold all our pointers at once.
		add edi, 8
		mov dword ptr[OVal2], edi
		mov edi, dword ptr[Dest]

		// Put the pixels in place.
#ifdef IS_SSE
		movntq qword ptr[edi], mm3
#else
		movq qword ptr[edi], mm3
#endif

		// Advance to the next set of pixels.
		add eax, 8
		add ebx, 8
		add edx, 8
		add esi, 8
		add esp, 8
		add edi, 8
		mov dword ptr[Dest], edi
		dec ecx
		jne near MAINLOOP_LABEL

		emms

		mov esi, OldSI
		mov esp, OldSP
	}

#undef MAINLOOP_LABEL