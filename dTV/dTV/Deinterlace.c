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
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "deinterlace.h"
#include "globals.h"

long BitShift = 13;
long EdgeDetect = 625;
long JaggieThreshold = 73;
long DiffThreshold = 224;

/////////////////////////////////////////////////////////////////////////////
// memcpyMMX
// Uses MMX instructions to move memory around
// does as much as we can in 64 byte chunks
// using MMX instructions
// then copies any extra bytes
// assumes there will be at least 64 bytes to copy
// This code was originally from Borg's bTV plugin SDK 
/////////////////////////////////////////////////////////////////////////////
void memcpyMMX(void *Dest, void *Src, size_t nBytes)
{
	size_t nCharsLeft = nBytes & 0x3F;
	__asm
	{
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

/////////////////////////////////////////////////////////////////////////////
// memcpyBOBMMX
// Uses MMX instructions to move memory around to two places
// does as much as we can in 64 byte chunks
// using MMX instructions
// then copies any extra bytes
// Dest must be the upper of the two lines
// assumes there will be at least 64 bytes to copy
/////////////////////////////////////////////////////////////////////////////
void memcpyBOBMMX(void *Dest, void *Src, size_t nBytes)
{
	__asm
	{
		mov		esi, dword ptr[Src]
		mov		edi, dword ptr[Dest]
		mov     ebx, edi
		add     ebx, OverlayPitch
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
		movq	qword ptr[ebx], mm0
		movq	qword ptr[ebx+8*1], mm1
		movq	qword ptr[ebx+8*2], mm2
		movq	qword ptr[ebx+8*3], mm3
		movq	qword ptr[ebx+8*4], mm4
		movq	qword ptr[ebx+8*5], mm5
		movq	qword ptr[ebx+8*6], mm6
		movq	qword ptr[ebx+8*7], mm7
		add		esi, 64
		add		edi, 64
		add		ebx, 64
		dec ecx
		jne near CopyLoop

		mov		ecx, nBytes
		and     ecx, 63
		cmp     ecx, 0
		je EndCopyLoop
align 8
CopyLoop2:
		mov dl, byte ptr[esi] 
		mov byte ptr[edi], dl
		mov byte ptr[ebx], dl
		inc esi
		inc edi
		inc ebx
		dec ecx
		jne near CopyLoop2
EndCopyLoop:
		emms
	}
}

/////////////////////////////////////////////////////////////////////////////
// VideoDeinterlaceMMX
// First go at this
// this one just averages between the incoming line and the
// average between the two surrounding lines
// this should reduce weave aftifacts while allowing
// some detail through
// this function copies two lines to the destination
// uses MMX instructions
// assumes there will be at least 32 bytes to copy
// Dest: This is the pointer to the middle line
// The algoritm comes from Borg's bTV deinterlace plug-in
// with key1 enabled
/////////////////////////////////////////////////////////////////////////////
void VideoDeinterlaceMMX(void *Dest, void *SrcUp, void *SrcSame, void *SrcDown, size_t nBytes)
{
	const __int64 Mask = 0x7f7f7f7f7f7f7f7f;
	__asm
	{
		mov		esi, dword ptr[SrcUp]
		mov		edx, dword ptr[SrcDown]
		mov		ebx, dword ptr[SrcSame]
		mov		edi, dword ptr[Dest]
		mov		ecx, nBytes
		shr     ecx, 5                      // nBytes / 32
align 8
CopyLoop:
		// read in the upper source line data
		movq  mm0, qword ptr [esi]
		movq  mm1, qword ptr [esi + 8]
		movq  mm2, qword ptr [esi + 16]
		movq  mm3, qword ptr [esi + 24]
		// read in the lower source line data
		movq  mm4, qword ptr [edx]
		movq  mm5, qword ptr [edx + 8]
		movq  mm6, qword ptr [edx + 16]
		movq  mm7, qword ptr [edx + 24]
		// move destination for the source line and
		// just put that straight in
		add edi, OverlayPitch
		movq  qword ptr [edi], mm4
		movq  qword ptr [edi + 8], mm5
		movq  qword ptr [edi + 16], mm6
		movq  qword ptr [edi + 24], mm7
		sub edi, OverlayPitch
		// divide all data bytes by 2
		// we do this by shifting all bytes
		// left one and anding to 127
		psrlw mm0, 01
		psrlw mm1, 01
		psrlw mm2, 01
		psrlw mm3, 01
		psrlw mm4, 01
		psrlw mm5, 01
		psrlw mm6, 01
		psrlw mm7, 01
		pand  mm0, Mask
		pand  mm1, Mask
		pand  mm2, Mask
		pand  mm3, Mask
		pand  mm4, Mask
		pand  mm5, Mask
		pand  mm6, Mask
		pand  mm7, Mask
		// work out average
		// of upper and lower
		paddw  mm0, mm4
		paddw  mm1, mm5
		paddw  mm2, mm6
		paddw  mm3, mm7
		// get middle line
		movq  mm4, qword ptr [ebx]
		movq  mm5, qword ptr [ebx + 8]
		movq  mm6, qword ptr [ebx + 16]
		movq  mm7, qword ptr [ebx + 24]
		// divide average and middle line by 2
		psrlw mm0, 01
		psrlw mm1, 01
		psrlw mm2, 01
		psrlw mm3, 01
		psrlw mm4, 01
		psrlw mm5, 01
		psrlw mm6, 01
		psrlw mm7, 01
		pand  mm0, Mask
		pand  mm1, Mask
		pand  mm2, Mask
		pand  mm3, Mask
		pand  mm4, Mask
		pand  mm5, Mask
		pand  mm6, Mask
		pand  mm7, Mask
		// add to give average of the middle line 
		// and the average of upper and lower lines
		paddw  mm0, mm4
		paddw  mm1, mm5
		paddw  mm2, mm6
		paddw  mm3, mm7
		// copy the result to the output buffer
		movq  qword ptr [edi], mm0
		movq  qword ptr [edi + 8], mm1
		movq  qword ptr [edi + 16], mm2
		movq  qword ptr [edi + 24], mm3
		// move to next set of 4 qwords
		add esi, 32
		add edi, 32
		add edx, 32
		add ebx, 32
		dec ecx
		jne near CopyLoop

		// see if we have any extra bytes to do
		mov		ecx, nBytes
		and     ecx, 31
		shr     ecx, 2                      // nBytes / 4
		cmp     ecx, 0
		je EndCopyLoop
align 8
CopyLoop2:
		// if we do have some left over then
		// do the same as before but only 4 bytes at a time
		movq  mm0, qword ptr [esi]
		movq  mm4, qword ptr [edx]
		psrlw mm0, 01
		psrlw mm4, 01
		pand  mm0, Mask
		pand  mm4, Mask
		paddw  mm0, mm4
		movq  mm4, qword ptr [ebx]
		psrlw mm0, 01
		psrlw mm4, 01
		pand  mm0, Mask
		pand  mm4, Mask
		paddw  mm0, mm4
		movq  dword ptr [edi], mm0
		add esi, 4
		add edi, 4
		add edx, 4
		add ebx, 4
		dec ecx
		jne near CopyLoop2
EndCopyLoop:
		emms
	}
}

///////////////////////////////////////////////////////////////////////////////
// DeinterlaceOdd
//
// The algorithm for this was taken from the 
// Deinterlace - area based Vitual Dub Plug-in by
// Gunnar Thalin
//
// 
// The algorithm for Deinterlace in VirtualDub was probably not intended for
// writing directly to video memory.  Most video memory these days supports
// Write Combining which goes much faster in chunks of at least 32 bytes.
// To do this here I unrolled the loop below and in function DeinterlaceEven 4 times
// to write 32 byte sequential chunks.  This function now assumes the lines we write
// are a multiple of 32 bytes.  It used to assume only 8.  -  Tom Barry 10/11/00
///////////////////////////////////////////////////////////////////////////////
void DeinterlaceOdd(short** pOddLines, short** pEvenLines, BYTE* lpCurOverlay)
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

	qwEdgeDetect = EdgeDetect;
	qwEdgeDetect += (qwEdgeDetect << 48) + (qwEdgeDetect << 32) + (qwEdgeDetect << 16);
	qwThreshold = JaggieThreshold;
	qwThreshold += (qwThreshold << 48) + (qwThreshold << 32) + (qwThreshold << 16);


	// copy first even and odd line anyway
	memcpyMMX(lpCurOverlay, pEvenLines[0], CurrentX * 2);
	memcpyMMX(lpCurOverlay + OverlayPitch, pOddLines[0], CurrentX * 2);
	for (Line = 0; Line < (CurrentY / 2 - 1); ++Line)
	{
		YVal1 = pOddLines[Line];
		YVal2 = pEvenLines[Line + 1];
		YVal3 = pOddLines[Line + 1];
		Dest = lpCurOverlay + (Line * 2 + 2) * OverlayPitch;
		_asm
		{
			mov ecx, CurrentX
			mov eax, dword ptr [YVal1]
			mov ebx, dword ptr [YVal2]
			mov edx, dword ptr [YVal3]
			mov edi, dword ptr [Dest]
			shr ecx, 4       // there are ActiveX * 2 / 8 qwords, now done 4 at a time

align 8
DoNext32Bytes:			

// Loop pass 1, unrolled
			add edi, OverlayPitch
			
			movq mm0, qword ptr[eax] 
			movq mm1, qword ptr[ebx] 

			// move all data movement to odd line code here to front of unrolled loop
			movq mm2, qword ptr[edx] 
			movq mm3, qword ptr[edx+8] 
			movq mm4, qword ptr[edx+16] 
			movq mm5, qword ptr[edx+24] 

			// copy the odd line to destination, 32 bytes at a time
			movq qword ptr[edi], mm2
			movq qword ptr[edi+8], mm3
			movq qword ptr[edi+16], mm4
			movq qword ptr[edi+24], mm5

			// get intensities in mm3 - 4
			movq mm3, mm1
			movq mm4, mm2
			movq mm5, mm3

			sub edi, OverlayPitch

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

			movq mm7, mm6

			pand mm0, mm6

			pandn mm7, mm1

			por mm7, mm0

			movq qword ptr[edi], mm7

// Loop pass 2, unrolled
			movq mm0, qword ptr[eax+8] 
			movq mm1, qword ptr[ebx+8] 
			movq mm2, qword ptr[edx+8]

			// get intensities in mm3 - 4
			movq mm3, mm1
			movq mm4, mm2
			movq mm5, mm3

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

			movq mm7, mm6

			pand mm0, mm6

			pandn mm7, mm1

			por mm7, mm0

			movq qword ptr[edi+8], mm7
	
// Loop pass 2, unrolled
			movq mm0, qword ptr[eax+16] 
			movq mm1, qword ptr[ebx+16] 
			movq mm2, qword ptr[edx+16]

			// get intensities in mm3 - 4
			movq mm3, mm1
			movq mm4, mm2
			movq mm5, mm3

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

			movq mm7, mm6

			pand mm0, mm6

			pandn mm7, mm1

			por mm7, mm0

			movq qword ptr[edi+16], mm7
	
// Loop pass 4, unrolled
			movq mm0, qword ptr[eax+24] 
			movq mm1, qword ptr[ebx+24] 
			movq mm2, qword ptr[edx+24]

			// get intensities in mm3 - 4
			movq mm3, mm1
			movq mm4, mm2
			movq mm5, mm3

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

			movq mm7, mm6

			pand mm0, mm6

			pandn mm7, mm1

			por mm7, mm0

			movq qword ptr[edi+24], mm7
	

			add eax, 32
			add ebx, 32
			add edx, 32
			add edi, 32
			dec ecx
			jne near DoNext32Bytes
			emms
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// DeinterlaceEven
//
// The algorithm for this was taken from the VirtualDub Deinterlace class
//
// Unrolled loop to write 32 bytes at a time, see above - Tom Barry 10/11/00
///////////////////////////////////////////////////////////////////////////////
void DeinterlaceEven(short** pOddLines, short** pEvenLines, BYTE* lpCurOverlay)
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

	qwEdgeDetect = EdgeDetect;
	qwEdgeDetect += (qwEdgeDetect << 48) + (qwEdgeDetect << 32) + (qwEdgeDetect << 16);
	qwThreshold = JaggieThreshold;
	qwThreshold += (qwThreshold << 48) + (qwThreshold << 32) + (qwThreshold << 16);


	// copy first even line
	memcpyMMX(lpCurOverlay, pEvenLines[0], CurrentX * 2);
	for (Line = 0; Line < (CurrentY / 2 - 1); ++Line)
	{
		YVal1 = pEvenLines[Line];
		YVal2 = pOddLines[Line];
		YVal3 = pEvenLines[Line + 1];
		Dest = lpCurOverlay + (Line * 2 + 1) * OverlayPitch;
		_asm
		{
			mov ecx, CurrentX
			mov eax, dword ptr [YVal1]
			mov ebx, dword ptr [YVal2]
			mov edx, dword ptr [YVal3]
			mov edi, dword ptr [Dest]
			shr ecx, 4       // there are ActiveX * 2 / 8 qwords, done 4 at a time

align 8
DoNext32Bytes:			
// Loop pass 1 of unrolled loop
			add edi, OverlayPitch

			movq mm0, qword ptr[eax] 
			movq mm1, qword ptr[ebx] 

			// copy the even line to destination, 32 bytes at a time
			movq mm2, qword ptr[edx]
			movq mm3, qword ptr[edx+8]
			movq mm4, qword ptr[edx+16]
			movq mm5, qword ptr[edx+24]

			movq qword ptr[edi], mm2
			movq qword ptr[edi+8], mm3
			movq qword ptr[edi+16], mm4
			movq qword ptr[edi+24], mm5

			// get intensities in mm3 - 4
			movq mm3, mm1
			movq mm4, mm2
			movq mm5, mm3

			sub edi, OverlayPitch

			pand mm3, YMask
			pand mm4, YMask
			pand mm5, YMask

			// get average in mm0
			psrlw mm0, 01
			psrlw mm2, 01
			pand  mm0, Mask
			pand  mm2, Mask
			paddw mm0, mm2

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

			movq mm7, mm6

			pand mm0, mm6

			pandn mm7, mm1

			por mm7, mm0

			movq qword ptr[edi], mm7

// Loop pass 2 of unrolled loop
			movq mm0, qword ptr[eax+8] 
			movq mm1, qword ptr[ebx+8] 
			movq mm2, qword ptr[edx+8]

			// get intensities in mm3 - 4
			movq mm3, mm1
			movq mm4, mm2
			movq mm5, mm3

			pand mm3, YMask
			pand mm4, YMask
			pand mm5, YMask

			// get average in mm0
			psrlw mm0, 01
			psrlw mm2, 01
			pand  mm0, Mask
			pand  mm2, Mask
			paddw mm0, mm2

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

			movq mm7, mm6

			pand mm0, mm6

			pandn mm7, mm1

			por mm7, mm0

			movq qword ptr[edi+8], mm7
	
// Loop pass 3 of unrolled loop

			movq mm0, qword ptr[eax+16] 
			movq mm1, qword ptr[ebx+16] 
			movq mm2, qword ptr[edx+16]

			// get intensities in mm3 - 4
			movq mm3, mm1
			movq mm4, mm2
			movq mm5, mm3

			pand mm3, YMask
			pand mm4, YMask
			pand mm5, YMask

			// get average in mm0
			psrlw mm0, 01
			psrlw mm2, 01
			pand  mm0, Mask
			pand  mm2, Mask
			paddw mm0, mm2

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

			movq mm7, mm6

			pand mm0, mm6

			pandn mm7, mm1

			por mm7, mm0

			movq qword ptr[edi+16], mm7
	
// Loop pass 4 of unrolled loop

			movq mm0, qword ptr[eax+24] 
			movq mm1, qword ptr[ebx+24] 
			movq mm2, qword ptr[edx+24]

			// get intensities in mm3 - 4
			movq mm3, mm1
			movq mm4, mm2
			movq mm5, mm3

			pand mm3, YMask
			pand mm4, YMask
			pand mm5, YMask

			// get average in mm0
			psrlw mm0, 01
			psrlw mm2, 01
			pand  mm0, Mask
			pand  mm2, Mask
			paddw mm0, mm2

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

			movq mm7, mm6

			pand mm0, mm6

			pandn mm7, mm1

			por mm7, mm0

			movq qword ptr[edi+24], mm7
	

			add eax, 32
			add ebx, 32
			add edx, 32
			add edi, 32
			dec ecx
			jne near DoNext32Bytes
			emms
		}
	}
	memcpyMMX(lpCurOverlay + (CurrentY - 1) * OverlayPitch, pOddLines[CurrentY / 2 - 1], CurrentX * 2);
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
long CompareFields(short** pLines1, short** pLines2)
{
	int Line;
	long LineFactor;
	long DiffFactor = 0;
	short* YVal1;
	short* YVal2;
	long ActiveX = CurrentX - 2 * InitialOverscan;
	const __int64 YMask    = 0x00ff00ff00ff00ff;
	__int64 wBitShift    = BitShift;

	for (Line = 80; Line < ((CurrentY - 80) / 2); ++Line)
	{
		YVal1 = pLines1[Line] + InitialOverscan;
		YVal2 = pLines2[Line] + InitialOverscan;
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

long CompareFields2(short** pLines1, short** pLines2)
{
	int Line;
	long LineFactor;
	long CombFactor = 0;
	short* YVal1;
	short* YVal2;
	long ActiveX = CurrentX - 2 * InitialOverscan;
	const __int64 YMask    = 0x00ff00ff00ff00ff;
	const __int64 qwOnes = 0x0001000100010001;
	__int64 wBitShift    = BitShift;

	__int64 qwThreshold;

	qwThreshold = DiffThreshold;
	qwThreshold += (qwThreshold << 48) + (qwThreshold << 32) + (qwThreshold << 16);

	for (Line = 100; Line < ((CurrentY - 100) / 2); ++Line)
	{
		YVal1 = pLines1[Line] + InitialOverscan;
		YVal2 = pLines2[Line] + InitialOverscan;
		_asm
		{
			mov ecx, ActiveX
			mov eax,dword ptr [YVal1]
			mov ebx,dword ptr [YVal2]
			shr ecx, 2       // there are ActiveX * 2 / 8 qwords
		    movq mm1, YMask
			pxor mm0, mm0    // mm0 = 0
align 8
Next8Bytes:
			movq mm3, qword ptr[eax] 
			movq mm4, qword ptr[ebx] 

			pand mm3, YMask
			pand mm4, YMask

			psubw mm3, mm4

			pmullw mm3, mm3

			pcmpgtw mm3, qwThreshold

			pand mm3, qwOnes

			paddw mm0, mm3

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
		CombFactor += (LineFactor & 0xFFFF);
		CombFactor += (LineFactor >> 16);
	}
	return CombFactor;
}
