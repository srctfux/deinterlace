/////////////////////////////////////////////////////////////////////////////
// DI_Greedy2Frame.asm
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock, Tom Barry, Steve Grimm  All rights reserved.
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

// This is the implementation of the Greedy 2-frame deinterlace algorithm described in
// DI_Greedy2Frame.c.  It's in a separate file so we can compile variants for different
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
		mov eax, dword ptr [T1]		
		mov ebx, dword ptr [M1]		
		mov edx, dword ptr [B1]		
		mov esi, dword ptr [M0]		
		mov esp, dword ptr [T0]
		shr ecx, 3						// there are LineLength / 8 qwords
		movq    mm6, Mask

align 8
MAINLOOP_LABEL:

		mov edi, dword ptr [B0]
		movq	mm1, qword ptr[eax]		// T1
		movq	mm0, qword ptr[ebx]		// M1
		movq	mm3, qword ptr[edx]		// B1
		movq	mm2, qword ptr[esi]     // M0

		// Average T1 and B1 so we can do interpolated bobbing if we bob onto T1.
		movq mm7, mm3					// mm7 = B1
#ifdef IS_SSE
		pavgb mm7, mm1
#endif
#ifdef IS_3DNOW
		pavgusb mm7, mm1
#endif
#ifdef IS_MMX
		movq mm5, mm1					// mm5 = T1
		psrlw mm7, 1					// mm7 = B1 / 2
		pand mm7, mm6					// mask off lower bits
		psrlw mm5, 1					// mm5 = T1 / 2
		pand mm5, mm6					// mask off lower bits
		paddw mm7, mm5					// mm7 = (T1 + B1) / 2
#endif

// calculate |M1-M0| put result in mm4 nned to keep mm0 intact
		movq	mm4, mm0
		psubusb mm4, mm2
		psubusb mm2, mm0
		por		mm4, mm2
		psrlw	mm4, 1
		pand	mm4, mm6

// if |M1-M0| > Threshold we want dword worth of twos
		pcmpgtb mm4, qwGreedyTwoFrameThreshold
		pand	mm4, Mask				// get rid of any sign bit
		pcmpgtd mm4, DwordOne			// do we want to bob
		pandn   mm4, DwordTwo

		movq	mm2, qword ptr[esp]		// mm2 = T0

// calculate |T1-T0| put result in mm5
		movq	mm5, mm2
		psubusb mm5, mm1
		psubusb mm1, mm2
		por		mm5, mm1
		psrlw	mm5, 1
		pand	mm5, mm6

// if |T1-T0| > Threshold we want dword worth of ones
		pcmpgtb mm5, qwGreedyTwoFrameThreshold
		pand	mm5, mm6				// get rid of any sign bit
		pcmpgtd mm5, DwordOne			
		pandn   mm5, DwordOne
		paddd mm4, mm5

		movq	mm2, qword ptr[edi]     // B0

// calculate |B1-B0| put result in mm5
		movq	mm5, mm2
		psubusb mm5, mm3
		psubusb mm3, mm2
		por		mm5, mm3
		psrlw	mm5, 1
		pand	mm5, mm6

// if |B1-B0| > Threshold we want dword worth of ones
		pcmpgtb mm5, qwGreedyTwoFrameThreshold
		pand	mm5, mm6				// get rid of any sign bit
		pcmpgtd mm5, DwordOne
		pandn   mm5, DwordOne
		paddd mm4, mm5

		// Get the dest pointer.
		add edi, 8
		mov dword ptr[B0], edi
		mov edi, dword ptr[Dest]

		pcmpgtd mm4, DwordTwo

// debugging feature
// output the value of mm4 at this point which is pink where we will weave
// and green were we are going to bob
#ifdef CHECK_BOBWEAVE
#ifdef IS_SSE
		movntq qword ptr[edi], mm4
#else
		movq qword ptr[edi], mm4
#endif
#else

		movq mm5, mm4
// mm4 now is 1 where we want to weave and 0 where we want to bob
		pand	mm4, mm0				
		pandn	mm5, mm7				
		por		mm4, mm5				
#ifdef IS_SSE
		movntq qword ptr[edi], mm4
#else
		movq qword ptr[edi], mm4
#endif
#endif
		// Advance to the next set of pixels.
		add edi, 8
		add eax, 8
		add ebx, 8
		add edx, 8
		mov dword ptr[Dest], edi
		add esi, 8
		dec ecx
		jne near MAINLOOP_LABEL

		emms

		mov esi, OldSI
		mov esp, OldSP
	}

#undef MAINLOOP_LABEL