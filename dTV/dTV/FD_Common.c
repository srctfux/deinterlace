/////////////////////////////////////////////////////////////////////////////
// FD_Common.c
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock. All rights reserved.
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
// Refinements made by Mark Rejhon and Steve Grimm
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Original Release
//
// 02 Jan 2001   John Adcock           Fixed bug at end of GetCombFactor assember
//
// 07 Jan 2001   John Adcock           Fixed PAL detection bug
//                                     Changed GetCombFactor to work on a primary
//                                     and secondary set of fields.
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 09 Jan 2001   John Adcock           Split out into new file
//                                     Changed functions to use DEINTERLACE_INFO
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OutThreads.h"
#include "FD_Common.h"
#define DOLOGGING
#include "DebugLog.h"


// Settings
// Default values which can be overwritten by the INI file
long BitShift = 13;
long CombEdgeDetect = 625;
long CombJaggieThreshold = 73;
long DiffThreshold = 224;


///////////////////////////////////////////////////////////////////////////////
// GetCombFactor
//
// This routine basically calculates how close the pixels in pSecondaryLines
// are the interpelated pixels between pPrimaryLines
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
long GetCombFactor(DEINTERLACE_INFO *pInfo)
{
	int Line;
	DWORD LineFactor;
	long CombFactor = 0;
	short* YVal1;
	short* YVal2;
	short* YVal3;
	long ActiveX = pInfo->SourceRect.right - pInfo->SourceRect.left;
	const __int64 YMask    = 0x00ff00ff00ff00ff;
	const __int64 qwOnes = 0x0001000100010001;

	__int64 qwEdgeDetect;
	__int64 qwThreshold;

	// If we've already computed the comb factor, just return it.
	if (pInfo->CombFactor > -1)
		return pInfo->CombFactor;

	// If one of the fields is missing, treat them as very different.
	if (pInfo->OddLines[0] == NULL || pInfo->EvenLines[0] == NULL)
		return 0x7fffffff;

	qwEdgeDetect = CombEdgeDetect;
	qwEdgeDetect += (qwEdgeDetect << 48) + (qwEdgeDetect << 32) + (qwEdgeDetect << 16);
	qwThreshold = CombJaggieThreshold;
	qwThreshold += (qwThreshold << 48) + (qwThreshold << 32) + (qwThreshold << 16);

	for (Line = pInfo->SourceRect.top / 2; Line < pInfo->SourceRect.bottom / 2 - 1; ++Line)
	{
		if(pInfo->IsOdd)
		{
			YVal1 = pInfo->OddLines[0][Line] + (pInfo->SourceRect.left & ~1);
			YVal2 = pInfo->EvenLines[0][Line + 1] + (pInfo->SourceRect.left & ~1);
			YVal3 = pInfo->OddLines[0][Line + 1] + (pInfo->SourceRect.left & ~1);
		}
		else
		{
			YVal1 = pInfo->EvenLines[0][Line] + (pInfo->SourceRect.left & ~1);
			YVal2 = pInfo->OddLines[0][Line] + (pInfo->SourceRect.left & ~1);
			YVal3 = pInfo->EvenLines[0][Line + 1] + (pInfo->SourceRect.left & ~1);
		}

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
			mov LineFactor, ecx
			emms
		}
		CombFactor += (LineFactor & 0xFFFF);
		CombFactor += (LineFactor >> 16);
	}

	pInfo->CombFactor = CombFactor;
	LOG(" Frame %d %c CF = %d", pInfo->CurrentFrame, pInfo->IsOdd ? 'O' : 'E', pInfo->CombFactor);
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
long CompareFields(DEINTERLACE_INFO *pInfo)
{
	int Line;
	DWORD LineFactor;
	long DiffFactor = 0;
	short* YVal1;
	short* YVal2;
	long ActiveX = pInfo->SourceRect.right - pInfo->SourceRect.left;
	const __int64 YMask    = 0x00ff00ff00ff00ff;
	__int64 wBitShift    = BitShift;
	short** pLines1;
	short** pLines2;

	// If we've already computed the field difference, just return it.
	if (pInfo->FieldDiff > -1)
		return pInfo->FieldDiff;

	if(pInfo->IsOdd)
	{
		pLines1 = pInfo->OddLines[1];
		pLines2 = pInfo->OddLines[0];
	}
	else
	{
		pLines1 = pInfo->EvenLines[1];
		pLines2 = pInfo->EvenLines[0];
	}

	// If we skipped a field, treat the new one as maximally different.
	if (pLines1 == NULL || pLines2 == NULL)
		return 0x7fffffff;

	for (Line = pInfo->SourceRect.top / 2; Line < pInfo->SourceRect.bottom / 2; ++Line)
	{
		YVal1 = pLines1[Line] + (pInfo->SourceRect.left & ~1);
		YVal2 = pLines2[Line] + (pInfo->SourceRect.left & ~1);
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
			mov LineFactor, ecx
			emms
		}
		DiffFactor += (long)sqrt(LineFactor);
	}

	pInfo->FieldDiff = DiffFactor;
	LOG(" Frame %d %c FD = %d", pInfo->CurrentFrame, pInfo->IsOdd ? 'O' : 'E', pInfo->FieldDiff);
	return DiffFactor;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING FD_CommonSettings[FD_COMMON_SETTING_LASTONE] =
{
	{
		"Bit Shift", SLIDER, 0, &BitShift,
		13, 0, 15, 1, NULL,
		"Pulldown", "BitShift", NULL,

	},
	{
		"Comb Edge Detect", SLIDER, 0, &CombEdgeDetect,
		625, 0, 10000, 5, NULL,
		"Pulldown", "EdgeDetect", NULL,

	},
	{
		"Comb Jaggie Threshold", SLIDER, 0, &CombJaggieThreshold,
		73, 0, 5000, 5, NULL,
		"Pulldown", "JaggieThreshold", NULL,

	},
	{
		"DiffThreshold", SLIDER, 0, &DiffThreshold,
		224, 0, 5000, 5, NULL,
		"Pulldown", "DiffThreshold", NULL,

	},
};

SETTING* FD_Common_GetSetting(FD_COMMON_SETTING Setting)
{
	if(Setting > -1 && Setting < FD_COMMON_SETTING_LASTONE)
	{
		return &(FD_CommonSettings[Setting]);
	}
	else
	{
		return NULL;
	}
}

void FD_Common_ReadSettingsFromIni()
{
	int i;
	for(i = 0; i < FD_COMMON_SETTING_LASTONE; i++)
	{
		Setting_ReadFromIni(&(FD_CommonSettings[i]));
	}
}

void FD_Common_WriteSettingsToIni()
{
	int i;
	for(i = 0; i < FD_COMMON_SETTING_LASTONE; i++)
	{
		Setting_WriteToIni(&(FD_CommonSettings[i]));
	}
}
