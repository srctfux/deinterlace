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

/////////////////////////////////////////////////////////////////////////////
// memcpyMMX
// Uses MMX instructions to move memory around
// does as much as we can in 64 byte chunks
// using MMX instructions
// then copies any extra bytes
// assumes there will be at least 64 bytes to copy
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
align 16
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
align 16
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
align 16
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
align 16
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
/////////////////////////////////////////////////////////////////////////////
void VideoDeinterlaceMMX(void *Dest, void *SrcUp, void *SrcSame, void *SrcDown, size_t nBytes)
{
	__int64 Mask = 0x7f7f7f7f7f7f7f7f;
	__asm
	{
		mov		esi, dword ptr[SrcUp]
		mov		edx, dword ptr[SrcDown]
		mov		ebx, dword ptr[SrcSame]
		mov		edi, dword ptr[Dest]
		mov		ecx, nBytes
		shr     ecx, 5                      // nBytes / 32

align 16
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
align 16
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

void DeinterlaceOdd(short** pOddLines, short** pEvenLines, BYTE* lpCurOverlay)
{
	int nLineTarget;
	short* pSource;
	BYTE* ScreenPtr;

	for (nLineTarget = 1; nLineTarget < CurrentY / 2 - 1; nLineTarget++)
	{
		pSource = pOddLines[nLineTarget];
		ScreenPtr = lpCurOverlay + (nLineTarget * 2 + 1) * OverlayPitch;
		memcpyMMX(ScreenPtr, pSource, CurrentX * 2);

		pSource = pEvenLines[nLineTarget];
		ScreenPtr = lpCurOverlay + (nLineTarget * 2) * OverlayPitch;
		memcpyMMX(ScreenPtr, pSource, CurrentX * 2);
	}
	ScreenPtr = lpCurOverlay + CurrentY * OverlayPitch;
	memcpyMMX(ScreenPtr, pEvenLines[CurrentY / 2], CurrentX * 2);
}

void DeinterlaceEven(short** pOddLines, short** pEvenLines, BYTE* lpCurOverlay)
{
	int Line;
	int Column;
	short* pSource;
	BYTE* ScreenPtr;
	short* YVal1;
	short* YVal2;
	short* YVal3;
	BYTE* Dest;
	
	const __int64 YMask    = 0x00ff00ff00ff00ff;
	const __int64 UVMask    = 0xff00ff00ff00ff00;
	__int64 qwEdgeDetect;
	__int64 qwThreshold;

	qwEdgeDetect = (25 << 48) + (25 << 32) + (25 << 16) + 100;
	qwThreshold = (27 << 48) + (27 << 32) + (27 << 16) + 100;

	// copy first even and odd line anyway
	memcpyMMX(lpCurOverlay, pEvenLines[0], CurrentX * 2);
	memcpyMMX(lpCurOverlay + OverlayPitch, pOddLines[0], CurrentX * 2);
	for (Line = 1; Line < (CurrentY / 2 - 1); ++Line)
	{
		YVal1 = pOddLines[Line];
		YVal2 = pEvenLines[Line + 1];
		YVal3 = pOddLines[Line + 1];
		Dest = lpCurOverlay + (Line * 2 + 1) * OverlayPitch;
		for(Column = 0; Column < CurrentY /2; Column++)
		{
			_asm
			{
				mov edi,dword ptr [Dest]
				movq mm7,qword ptr[YMask]  
				movq mm6,qword ptr[qwEdgeDetect]  

				movq mm0, qword ptr[YVal1] 
				movq mm1, qword ptr[YVal2] 
				movq mm2, qword ptr[YVal3]

				// copy the odd line to destination
				movq mm2, qword ptr[edi]
				sub edi, OverlayPitch

				// get intensities in mm3 - 4
				movq mm3, mm1
				movq mm4, mm2
				movq mm5, mm3

				pand mm3, mm7
				pand mm4, mm7
				pand mm5, mm7

				// work out (O1 - E) * (O2 - E) - EdgeDetect * (O1 - O2) ^ 2 >> 12
				// result will be in mm0

				movq mm0, mm3
				psubw mm0, mm4		//mm0 = O1 - E

				movq mm1, mm5
				psubw mm1, mm4		//mm0 = O2 - e

				pmullw mm0, mm1		// mm0 = (O1 - E) * (O2 - E)

				movq mm1, mm3
				psubw mm1, mm5		// mm1 = (O1 - O2)
				pmullw mm1, mm1		// mm1 = (O1 - O2) ^ 2
				psrlw mm1, 12		// mm1 = (O1 - O2) ^ 2 >> 12
				pmullw mm1, mm6		// mm1  = EdgeDetect * (O1 - O2) ^ 2 >> 12

				psubw mm0, mm1      // mm1 is what we want

				
				pcmpgtw mm0, qword ptr[qwThreshold]
				
				paddw mm3, mm4
				pand mm6, mm1
				paddw mm3, mm6
				
				add eax, 8
				pmaddwd mm3, mm3
				add ebx, 8
				paddd mm0, mm3
				add edx, 8


				movd eax, mm0
				psrlq mm0,32
				movd ecx, mm0
				add ecx, eax
				emms
			}
		}
		pSource = pOddLines[Line];
		ScreenPtr = lpCurOverlay + (Line * 2 + 1) * OverlayPitch;
		memcpyMMX(ScreenPtr, pSource, CurrentX * 2);

		pSource = pEvenLines[Line];
		ScreenPtr = lpCurOverlay + (Line * 2) * OverlayPitch;
		memcpyMMX(ScreenPtr, pSource, CurrentX * 2);
	}
	ScreenPtr = lpCurOverlay + CurrentY * OverlayPitch;
	memcpyMMX(ScreenPtr, pEvenLines[CurrentY / 2], CurrentX * 2);
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
///////////////////////////////////////////////////////////////////////////////
long GetCombFactor(short** pLines1, short** pLines2)
{
	int Line;
	long LineFactor;
	long CombFactor = 0;
	short* YVal1;
	short* YVal2;
	short* YVal3;

	const __int64 YMask    = 0x00ff00ff00ff00ff;

	for (Line = 1; Line < (CurrentY / 2 - 2); ++Line)
	{
		YVal1 = pLines1[Line];
		YVal2 = pLines2[Line];
		YVal3 = pLines1[Line + 1];
		_asm
		{
			mov ecx, CurrentX
			mov eax,dword ptr [YVal1]
			mov ebx,dword ptr [YVal2]
			mov edx,dword ptr [YVal3]
			shr ecx, 1
		    movq mm1,qword ptr[YMask]  
			pxor mm0, mm0    // mm0 = 0
align 16
Next8Bytes:
			movq mm5, qword ptr[ebx] 
			pxor mm3, mm3    // mm3 = 0
			pand mm5, mm1
			movq mm4, qword ptr[eax] 
			psubw mm3, mm5
			pand mm4, mm1
			psubw mm3, mm5

			movq mm6, qword ptr[edx]
			paddw mm3, mm4
			pand mm6, mm1
			paddw mm3, mm6
			
			add eax, 8
			pmaddwd mm3, mm3
			add ebx, 8
			paddd mm0, mm3
			add edx, 8

			loop Next8Bytes

			movd eax, mm0
			psrlq mm0,32
			movd ecx, mm0
			add ecx, eax
			mov dword ptr[LineFactor], ecx
			emms
		}
		CombFactor += (long)sqrt(LineFactor);
	}
	return CombFactor;
}
