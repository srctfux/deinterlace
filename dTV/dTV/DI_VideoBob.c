/////////////////////////////////////////////////////////////////////////////
// DI_VideoBob.c
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
#include "cpu.h"
#include "deinterlace.h"
#include "DI_VideoBob.h"

long EdgeDetect = 625;
long JaggieThreshold = 73;

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
BOOL DeinterlaceFieldBob(DEINTERLACE_INFO *info)
{
	int Line;
	short* YVal1;
	short* YVal2;
	short* YVal3;
	BYTE* Dest;
	short **pOddLines = info->OddLines[0];
	short **pEvenLines = info->EvenLines[0];
	DWORD LineLength = info->LineLength;
	
	__int64 qwEdgeDetect;
	__int64 qwThreshold;
	const __int64 Mask = 0x7f7f7f7f7f7f7f7f;
	const __int64 YMask    = 0x00ff00ff00ff00ff;

	qwEdgeDetect = EdgeDetect;
	qwEdgeDetect += (qwEdgeDetect << 48) + (qwEdgeDetect << 32) + (qwEdgeDetect << 16);
	qwThreshold = JaggieThreshold;
	qwThreshold += (qwThreshold << 48) + (qwThreshold << 32) + (qwThreshold << 16);


	// copy first even line no matter what, and the first odd line if we're
	// processing an odd field.
	memcpyMMX(info->Overlay, pEvenLines[0], LineLength);
	if (info->IsOdd)
		memcpyMMX(info->Overlay + info->OverlayPitch, pOddLines[0], LineLength);

	for (Line = 0; Line < info->FieldHeight - 1; ++Line)
	{
		if (info->IsOdd)
		{
			YVal1 = pOddLines[Line];
			YVal2 = pEvenLines[Line + 1];
			YVal3 = pOddLines[Line + 1];
			Dest = info->Overlay + (Line * 2 + 2) * info->OverlayPitch;
		}
		else
		{
			YVal1 = pEvenLines[Line];
			YVal2 = pOddLines[Line];
			YVal3 = pEvenLines[Line + 1];
			Dest = info->Overlay + (Line * 2 + 1) * info->OverlayPitch;
		}

		// For ease of reading, the comments below assume that we're operating on an odd
		// field (i.e., that bIsOdd is true).  The exact same processing is done when we
		// operate on an even field, but the roles of the odd and even fields are reversed.
		// It's just too cumbersome to explain the algorithm in terms of "the next odd
		// line if we're doing an odd field, or the next even line if we're doing an
		// even field" etc.  So wherever you see "odd" or "even" below, keep in mind that
		// half the time this function is called, those words' meanings will invert.

		// Copy the odd line to the overlay verbatim.
		memcpyMMX(Dest + info->OverlayPitch, YVal3, LineLength);

		_asm
		{
			mov ecx, LineLength
			mov eax, dword ptr [YVal1]
			mov ebx, dword ptr [YVal2]
			mov edx, dword ptr [YVal3]
			mov edi, dword ptr [Dest]
			shr ecx, 3       // there are LineLength / 8 qwords

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
	if (! info->IsOdd)
	{
		memcpyMMX(info->Overlay + (info->FrameHeight - 1) * info->OverlayPitch,
				  pOddLines[info->FieldHeight - 1],
				  LineLength);
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING DI_VideoBobSettings[DI_VIDEOBOB_SETTING_LASTONE] =
{
	{
		"Weave Edge Detect", SLIDER, 0, &EdgeDetect,
		625, 0, 10000, 5, 1,
		NULL,
		"Deinterlace", "EdgeDetect", NULL,
	},
	{
		"Weave Jaggie Threshold", SLIDER, 0, &JaggieThreshold,
		73, 0, 5000, 5, 1,
		NULL,
		"Deinterlace", "JaggieThreshold", NULL,
	},
};

SETTING* DI_VideoBob_GetSetting(DI_VIDEOBOB_SETTING Setting)
{
	if(Setting > -1 && Setting < DI_VIDEOBOB_SETTING_LASTONE)
	{
		return &(DI_VideoBobSettings[Setting]);
	}
	else
	{
		return NULL;
	}
}

void DI_VideoBob_ReadSettingsFromIni()
{
	int i;
	for(i = 0; i < DI_VIDEOBOB_SETTING_LASTONE; i++)
	{
		Setting_ReadFromIni(&(DI_VideoBobSettings[i]));
	}
}

void DI_VideoBob_WriteSettingsToIni()
{
	int i;
	for(i = 0; i < DI_VIDEOBOB_SETTING_LASTONE; i++)
	{
		Setting_WriteToIni(&(DI_VideoBobSettings[i]));
	}
}
