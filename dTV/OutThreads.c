/////////////////////////////////////////////////////////////////////////////
// OutThread.c
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
//
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Original Release
//                                     Translated most code from German
//                                     Combined Header files
//                                     Cut out all decoding
//                                     Cut out digital hardware stuff
//
// 09 Aug 2000   John Adcock           Fixed bug at end of GetCombFactor assember
//
/////////////////////////////////////////////////////////////////////////////

#include "OutThreads.h"
#include "other.h"
#include "bt848.h"
#include "bTVPlugin.h"


short pPALplusCode[] = {  18,  27,  36,  45,  54,  63,  72,  81,  90, 100, 110, 120, 134, 149};
short pPALplusData[] = { 160, 178, 196, 214, 232, 250, 268, 286, 304, 322, 340, 358, 376, 394};
short nLevelLow      =  45;
short nLevelHigh     = 135;

BOOL bStopThread = FALSE;
HANDLE OutThread;

struct TPulldownDetect
{
	ePULLDOWNMODES Mode;
	WORD nFlags1;
	WORD nFlags2;
};

struct TPulldownDetect PulldownDetect[] =
{
	{VIDEO_MODE, 0, 0,},
	// if it's 101 010 101 then 2:2 flip on odd
	{FILM_22_PULLDOWN_ODD, 0525, 0252,},
	// if it's 010 101 010 then 2:2 flip on even
	{FILM_22_PULLDOWN_EVEN, 0252, 0525,},
	// if it's 101 101 011 then 3:2 flip on first
	{FILM_32_PULLDOWN_0, 0553, 0224,},
	// if it's 110 101 101 then 3:2 flip on Second
	{FILM_32_PULLDOWN_1, 0655, 0122,},
	// if it's 010 110 101 then 3:2 flip on Third
	{FILM_32_PULLDOWN_2, 0265, 0512,},
	// if it's 011 010 110 then 3:2 flip on Forth
	{FILM_32_PULLDOWN_3, 0326, 0451,},
	// if it's 101 011 010 then 3:2 flip on Fifth
	{FILM_32_PULLDOWN_4, 0532, 0245,},
};

void Start_Thread()
{
	int i;
	DWORD LinkThreadID;

	CurrentFrame = 0;

	for (i = 0; i < 5; i++)
	{
		pDisplay[i] = Display_dma[i]->dwUser;
	}

	bStopThread = FALSE;

	if(TVSettings[TVTYPE].Is25fps)
	{
		OutThread = CreateThread((LPSECURITY_ATTRIBUTES) NULL,	// No security.
								 (DWORD) 0,	// Same stack size.
								 YUVOutThreadPAL,	// Thread procedure.
								 NULL,	// Parameter.
								 (DWORD) 0,	// Start immediatly.
								 (LPDWORD) & LinkThreadID);	// Thread ID.
	}
	else
	{
		OutThread = CreateThread((LPSECURITY_ATTRIBUTES) NULL,	// No security.
								 (DWORD) 0,	// Same stack size.
								 YUVOutThreadNTSC,	// Thread procedure.
								 NULL,	// Parameter.
								 (DWORD) 0,	// Start immediatly.
								 (LPDWORD) & LinkThreadID);	// Thread ID.
	}
}

void Stop_Thread()
{
	DWORD ExitCode;
	int i;
	BOOL Thread_Stopped = FALSE;

	if (OutThread != NULL)
	{
		i = 5;
		SetThreadPriority(OutThread, THREAD_PRIORITY_NORMAL);
		bStopThread = TRUE;
		while(i > 0)
		{
			if (GetExitCodeThread(OutThread, &ExitCode) == TRUE)
			{
				if (ExitCode != STILL_ACTIVE)
				{
					Thread_Stopped = TRUE;
				}
			}
			else
			{
				Thread_Stopped = TRUE;
			}
			Sleep(100);
			i--;
		}

		if (Thread_Stopped == FALSE)
		{
			TerminateThread(OutThread, 0);
		}
		Sleep(50);
		CloseHandle(OutThread);
		OutThread = NULL;
	}
}

void Set_Capture(int nState)
{
	int nFlags = 0;

	if (Capture_Video == TRUE)
	{
		nFlags |= BT848_CAP_CTL_CAPTURE_EVEN | BT848_CAP_CTL_CAPTURE_ODD;
	}
	if (Capture_VBI == TRUE)
	{
		nFlags |= BT848_CAP_CTL_CAPTURE_VBI_EVEN | BT848_CAP_CTL_CAPTURE_VBI_ODD;
	}

	MaskDataByte(BT848_CAP_CTL, 0, 0x0f);

	switch (nState)
	{
	case 0:

		nFlags = nFlags |= BT848_CAP_CTL_CAPTURE_EVEN | BT848_CAP_CTL_CAPTURE_ODD;
		break;
	case 1:
		nFlags = nFlags &= ~(BT848_CAP_CTL_CAPTURE_EVEN | BT848_CAP_CTL_CAPTURE_ODD);
		break;
	case 2:
		nFlags = nFlags |= BT848_CAP_CTL_CAPTURE_VBI_EVEN | BT848_CAP_CTL_CAPTURE_VBI_ODD;
		break;
	case 3:
		nFlags = nFlags &= ~(BT848_CAP_CTL_CAPTURE_VBI_EVEN | BT848_CAP_CTL_CAPTURE_VBI_ODD);
		break;
	case 4:
		nFlags = 0;
		break;
	case 5:
		InterCast.esc = 0;
		InterCast.done = 0;
		InterCast.pnum = 0;
		InterCast.ok = 0;
		InterCast.datap = 0;
		InterCast.lastci = 0xff;
		break;
	}

	CurrentCapture = nFlags;

	if (!(nFlags & (BT848_CAP_CTL_CAPTURE_EVEN | BT848_CAP_CTL_CAPTURE_ODD)))
	{
		//  Stop The Output Thread
		if (OutThread != NULL)
		{
			Stop_Thread();
		}
	}
	
	SetRiscJumpsDecode(nFlags);

	MaskDataByte(BT848_CAP_CTL, (BYTE) nFlags, (BYTE) 0x0f);
	if (nFlags & 0x0f)
	{
		SetDMA(TRUE);
	}
	else
	{
		SetDMA(FALSE);
	}

	if (nFlags & (BT848_CAP_CTL_CAPTURE_EVEN | BT848_CAP_CTL_CAPTURE_ODD))
	{
		//  Start the OutputThread 
		if (OutThread == NULL)
		{
			Start_Thread();
		}
	}
}


void SetupProcessorAndThread()
{
	DWORD rc;
	ProzessorMask = 1 << (AusgabeProzessor);
	rc = SetThreadAffinityMask(GetCurrentThread(), ProzessorMask);
	
	if (ThreadClassId == 0)
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
	else if (ThreadClassId == 1)
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
	else if (ThreadClassId == 2)
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
	else if (ThreadClassId == 3)
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
	else if (ThreadClassId == 4)
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
}

long __inline Sq(long x) {return x*x;}

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
		dec ecx
		mov byte ptr[edi], dl
		inc esi
		inc edi
		loop CopyLoop2
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
// assumes there will be at least 64 bytes to copy
/////////////////////////////////////////////////////////////////////////////
void memcpyBOBMMX(void *Dest1, void *Dest2, void *Src, size_t nBytes)
{
	size_t nCharsLeft = nBytes & 0x3F;
	__asm
	{
		mov		esi, dword ptr[Src]
		mov		ebx, dword ptr[Dest2]
		mov		edi, dword ptr[Dest1]
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
		loop CopyLoop
		mov		ecx, nBytes
		and     ecx, 63
		cmp     ecx, 0
		je EndCopyLoop
align 16
CopyLoop2:
		mov dl, byte ptr[esi] 
		dec ecx
		mov byte ptr[edi], dl
		mov byte ptr[ebx], dl
		inc esi
		inc edi
		inc ebx
		loop CopyLoop2
EndCopyLoop:
		emms
	}
}

ePULLDOWNMODES GuessCorrectPALPulldownMode(WORD Flags1, WORD Flags2)
{
	ePULLDOWNMODES i;
	for(i = 1; i < 3; i++)
	{
		if((Flags1 & PulldownDetect[i].nFlags1) == PulldownDetect[i].nFlags1)
		{
			return i;
		}
	}
	return VIDEO_MODE;
}

ePULLDOWNMODES GuessCorrectNTSCPulldownMode(WORD Flags1, WORD Flags2)
{
	ePULLDOWNMODES i;
	for(i = 3; i < PULLDOWNMODES_LAST_ONE; i++)
	{
		if((Flags1 & PulldownDetect[i].nFlags1) == PulldownDetect[i].nFlags1 &&
			(Flags2 & PulldownDetect[i].nFlags2) == 0 )
		{
			return i;
		}
	}
	return VIDEO_MODE;
}

void UpdatePALPulldownMode(struct TPulldowmMode* PulldownMode, long* CombFactors)
{
	WORD Flags1 = 0;
	WORD Flags2 = 0;
	int i;

	for(i = 0; i < 9; i++)
	{
		int Diff = CombFactors[i + 1] - CombFactors[i];
		if(Diff * 100 / (CombFactors[i + 1]  + 1) < PulldownThresholdHigh)
		{
			Flags1 |= 1 << i;
		}
		if(Diff * 100 / (CombFactors[i + 1]  + 1) > PulldownThresholdLow)
		{
			Flags2 |= 1 << i;
		}
	}

	if(gPulldownMode == VIDEO_MODE)
	{
		// if we are in video mode then look for the same
		// pattern being repeated PulldownRepeatCount times
		ePULLDOWNMODES BestGuess = GuessCorrectPALPulldownMode(Flags1, Flags2);
		if(BestGuess != 0 && BestGuess == PulldownMode->LastGuess)
		{
			if((Flags2 & PulldownDetect[gPulldownMode].nFlags2) == 0)
			{
				PulldownMode->LastGuessCount++;
			}
		}
		else if(BestGuess != 0)
		{
			PulldownMode->LastGuessCount = 1;
			PulldownMode->LastGuess = BestGuess;
		}
		if(PulldownMode->LastGuessCount > PulldownRepeatCount)
		{
			gPulldownMode = PulldownMode->LastGuess;
			PulldownMode->nCount = PulldownMode->LastGuessCount;
			PulldownMode->LastGuessCount = 0;
			UpdatePulldownStatus();
		}
	}
	else
	{
		// is it not OK to carry on using what we started with
		// then update the structure
		if((Flags1 & PulldownDetect[gPulldownMode].nFlags1) != PulldownDetect[gPulldownMode].nFlags1)
		{
			ePULLDOWNMODES BestGuess = GuessCorrectPALPulldownMode(Flags1, Flags2);
			if(BestGuess == PulldownMode->LastGuess)
			{
				if((Flags2 & PulldownDetect[gPulldownMode].nFlags2) == 0)
				{
					PulldownMode->LastGuessCount++;
				}
			}
			else
			{
				PulldownMode->LastGuessCount = 0;
			}
			PulldownMode->nCount--;
			if(PulldownMode->nCount == 0)
			{
				// if we have got some sort of new lock then jump straight to it
				if(PulldownMode->LastGuessCount > 2)
				{
					gPulldownMode = PulldownMode->LastGuess;
					PulldownMode->nCount = PulldownMode->LastGuessCount;
					PulldownMode->LastGuessCount = 1;
					UpdatePulldownStatus();
				}
				else
				{
					gPulldownMode = VIDEO_MODE;
					PulldownMode->nCount = PulldownMode->LastGuessCount;
					UpdatePulldownStatus();
				}
			}
		}
		else
		{
			PulldownMode->LastGuess = gPulldownMode;
			PulldownMode->LastGuessCount = 0;
			if(PulldownMode->nCount < PulldownRepeatCount)
			{
				if((Flags2 & PulldownDetect[gPulldownMode].nFlags2) == 0)
				{
					PulldownMode->nCount++;
				}
			}
		}
	}
}

void UpdateNTSCPulldownMode(struct TPulldowmMode* PulldownMode, long* CombFactors)
{
	WORD Flags1 = 0;
	WORD Flags2 = 0;
	int i;

	for(i = 0; i < 9; i++)
	{
		int Diff = CombFactors[i + 1] - CombFactors[i];
		if(Diff * 100 / (CombFactors[i + 1]  + 1) < PulldownThresholdLow)
		{
			Flags1 |= 1 << i;
		}
		else if(Diff * 100 / (CombFactors[i + 1] + 1) > PulldownThresholdHigh)
		{
			Flags2 |= 1 << i;
		}
	}

	if(gPulldownMode == VIDEO_MODE)
	{
		// if we are in video mode then look for the same
		// pattern being repeated PulldownRepeatCount times
		ePULLDOWNMODES BestGuess = GuessCorrectNTSCPulldownMode(Flags1, Flags2);
		if(BestGuess == PulldownMode->LastGuess)
		{
			PulldownMode->LastGuessCount++;
		}
		else
		{
			PulldownMode->LastGuessCount = 1;
			PulldownMode->LastGuess = BestGuess;
		}
		if(PulldownMode->LastGuessCount > PulldownRepeatCount)
		{
			gPulldownMode = PulldownMode->LastGuess;
			PulldownMode->nCount = PulldownMode->LastGuessCount;
			PulldownMode->LastGuessCount = 0;
			UpdatePulldownStatus();
		}
	}
	else
	{
		// is it not OK to carry on using what we started with
		// then update the structure
		if((Flags1 & !PulldownDetect[gPulldownMode].nFlags1) != !PulldownDetect[gPulldownMode].nFlags1 &&
			(Flags2 & !PulldownDetect[gPulldownMode].nFlags2) > 0)
		{
			ePULLDOWNMODES BestGuess = GuessCorrectNTSCPulldownMode(Flags1, Flags2);
			if(BestGuess == PulldownMode->LastGuess)
			{
				PulldownMode->LastGuessCount++;
				PulldownMode->LastGuess = BestGuess;
			}
			else
			{
				PulldownMode->LastGuessCount = 0;
			}
			PulldownMode->nCount--;
			if(PulldownMode->nCount == 0)
			{
				// if we have got some sort of new lock then jump straight to it
				if(PulldownMode->LastGuessCount > 2)
				{
					gPulldownMode = PulldownMode->LastGuess;
					PulldownMode->nCount = PulldownMode->LastGuessCount;
					PulldownMode->LastGuessCount = 1;
					UpdatePulldownStatus();
				}
				else
				{
					gPulldownMode = VIDEO_MODE;
					PulldownMode->nCount = PulldownMode->LastGuessCount;
					UpdatePulldownStatus();
				}
			}
		}
		else
		{
			PulldownMode->LastGuess = gPulldownMode;
			PulldownMode->LastGuessCount = 0;
			if(PulldownMode->nCount < PulldownRepeatCount)
			{
				PulldownMode->nCount++;
			}
		}
	}
}

BOOL DoWeWantToFlip()
{
	BOOL RetVal;
	switch(gPulldownMode)
	{
	case VIDEO_MODE:
		RetVal = TRUE;
		break;
	case FILM_22_PULLDOWN_ODD:
		RetVal = bIsOddField;
		break;
	case FILM_22_PULLDOWN_EVEN:
		RetVal = !bIsOddField;
		break;
	case FILM_32_PULLDOWN_0:
		switch(CurrentFrame)
		{
		case 0:
			RetVal = FALSE;
			break;
		case 1:
			RetVal = !bIsOddField;
			break;
		case 2:
			RetVal = !bIsOddField;
			break;
		case 3:
			RetVal = bIsOddField;
			break;
		case 4:
			RetVal = bIsOddField;
			break;
		}
		break;
	case FILM_32_PULLDOWN_1:
		switch(CurrentFrame)
		{
		case 0:
			RetVal = bIsOddField;
			break;
		case 1:
			RetVal = FALSE;
			break;
		case 2:
			RetVal = !bIsOddField;
			break;
		case 3:
			RetVal = !bIsOddField;
			break;
		case 4:
			RetVal = bIsOddField;
			break;
		}
		break;
	case FILM_32_PULLDOWN_2:
		switch(CurrentFrame)
		{
		case 0:
			RetVal = bIsOddField;
			break;
		case 1:
			RetVal = bIsOddField;
			break;
		case 2:
			RetVal = FALSE;
			break;
		case 3:
			RetVal = !bIsOddField;
			break;
		case 4:
			RetVal = !bIsOddField;
			break;
		}
		break;
	case FILM_32_PULLDOWN_3:
		switch(CurrentFrame)
		{
		case 0:
			RetVal = !bIsOddField;
			break;
		case 1:
			RetVal = bIsOddField;
			break;
		case 2:
			RetVal = bIsOddField;
			break;
		case 3:
			RetVal = FALSE;
			break;
		case 4:
			RetVal = !bIsOddField;
			break;
		}
		break;
	case FILM_32_PULLDOWN_4:
		switch(CurrentFrame)
		{
		case 0:
			RetVal = !bIsOddField;
			break;
		case 1:
			RetVal = !bIsOddField;
			break;
		case 2:
			RetVal = bIsOddField;
			break;
		case 3:
			RetVal = bIsOddField;
			break;
		case 4:
			RetVal = FALSE;
			break;
		}
		break;
	default:
		RetVal = FALSE;
		break;
	}
	return RetVal;
}

void UpdatePulldownStatus()
{
	switch(gPulldownMode)
	{
	case VIDEO_MODE:
		if(bUseBTVPlugin == TRUE)
		{
			SetWindowText(hwndPalField, "Using bTV Plugin");
		}
		else
		{
			SetWindowText(hwndPalField, "Simple Weave");
		}
		break;
	case FILM_22_PULLDOWN_ODD:
		SetWindowText(hwndPalField, "2:2 Pulldown Flip on Odd");
		break;
	case FILM_22_PULLDOWN_EVEN:
		SetWindowText(hwndPalField, "2:2 Pulldown Flip on Even");
		break;
	case FILM_32_PULLDOWN_0:
		SetWindowText(hwndPalField, "3:2 Pulldown Skip 1st Full Frame");
		break;
	case FILM_32_PULLDOWN_1:
		SetWindowText(hwndPalField, "3:2 Pulldown Skip 2nd Full Frame");
		break;
	case FILM_32_PULLDOWN_2:
		SetWindowText(hwndPalField, "3:2 Pulldown Skip 3rd Full Frame");
		break;
	case FILM_32_PULLDOWN_3:
		SetWindowText(hwndPalField, "3:2 Pulldown Skip 4th Full Frame");
		break;
	case FILM_32_PULLDOWN_4:
		SetWindowText(hwndPalField, "3:2 Pulldown Skip 5th Full Frame");
		break;
	default:
		SetWindowText(hwndPalField, "Unknown Pulldown Mode");
		break;
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

void Weave(short** pOddLines, short** pEvenLines, BYTE* lpOverlay)
{
	int i;

	for (i = 0; i < CurrentY / 2; i++)
	{
		memcpyMMX(lpOverlay, pEvenLines[i], CurrentX * 2);
		lpOverlay += OverlayPitch;

		memcpyMMX(lpOverlay, pOddLines[i], CurrentX * 2);
		lpOverlay += OverlayPitch;
	}
}

DWORD WINAPI YUVOutThreadPAL(LPVOID lpThreadParameter)
{
	BYTE* ScreenPtr;
	BYTE* ScreenPtr2;
	char Text[128];
	int i, j;
	int nLineTarget;
	short* pSource;
	int nFrame = 0;
	DWORD dwLastCount;
	BYTE* lpCurOverlay = lpOverlayBack;
	//long CombFactors[10];
	short* ppEvenLines[5][CLEARLINES];
	short* ppOddLines[5][CLEARLINES];
	BYTE* pDestEven[CLEARLINES];
	BYTE* pDestOdd[CLEARLINES];
	//struct TPulldowmMode PulldownMode;
	int LastEvenFrame = 0;
	int LastOddFrame = 0;
	int CombNum = 0;

	if (lpDDOverlay == NULL || lpDDOverlay == NULL || lpOverlayBack == NULL || lpOverlay == NULL)
	{
		ExitThread(-1);
	}

	// Sets processor Affinity and Thread priority according to menu selection
	SetupProcessorAndThread();

	// Set up 5 sets of pointers to the start of odd and even lines
	// we will always go up to the limit so that we can use bTV
	for (j = 0; j < 5; j++)
	{
		for (i = 0; i < BTV_VER1_HEIGHT; i += 2)
		{
			ppOddLines[j][i / 2] = (short *) pDisplay[j] + (i + 1) * 1024;
			ppEvenLines[j][i / 2] = (short *) pDisplay[j] + i * 1024;
		}
	}

	// make sure we start with a clean sheet of paper
	Black_Surface();
	Black_Overlays();

	// display the current pulldown mode
	UpdatePulldownStatus();
	
	dwLastCount = GetTickCount();

	while(!bStopThread)
	{
		nFrame++;
		Get_Thread_Status();

		// set up desination pointers
		// may need to optimize this later
		for (i = 0; i < BTV_VER1_HEIGHT; i += 2)
		{
			pDestEven[i / 2] = lpCurOverlay + i * OverlayPitch;
			pDestOdd[i / 2] = lpCurOverlay + (i + 1) * OverlayPitch;
		}

		if(bIsOddField)
		{
			//CombFactors[CurrentFrame * 2 + 1] = GetCombFactor(ppEvenLines[CurrentFrame], ppOddLines[CurrentFrame]);
			if(CurrentFrame == 4)
			{
				// update the gPulldownMode based on last set of 5 frames
				// doesn't check for dropped frames but this shouldn't matter
				// too much
				//UpdatePALPulldownMode(&PulldownMode, CombFactors);
			}
			// if we have dropped a field then do BOB 
			if(LastEvenFrame != CurrentFrame)
			{
				for (nLineTarget = 0; nLineTarget < CurrentY / 2; nLineTarget++)
				{
					memcpyBOBMMX(pDestEven[nLineTarget], 
								pDestOdd[nLineTarget], 
								ppOddLines[CurrentFrame][nLineTarget], 
								CurrentX * 2);
				}
			}
			else if(gPulldownMode == VIDEO_MODE)
			{
				if(bUseBTVPlugin == TRUE)
				{
					BTVParams.IsOddField = 1;
					BTVParams.ppCurrentField = ppOddLines[CurrentFrame];
					BTVParams.ppLastField = ppEvenLines[CurrentFrame];
					BTVParams.ppEvenDest = pDestEven;
					BTVParams.ppOddDest = pDestOdd;
					BTVPluginDoField(&BTVParams);
				}
				else
				{
					Weave(ppOddLines[CurrentFrame], ppEvenLines[CurrentFrame], lpCurOverlay);
				}
			}
			else
			{
				for (nLineTarget = 0; nLineTarget < CurrentY / 2; nLineTarget++)
				{
					// copy latest data to destination buffer
					memcpyMMX(pDestOdd[nLineTarget], 
						ppOddLines[CurrentFrame][nLineTarget], 
						CurrentX * 2);
				}
			}
			LastOddFrame = CurrentFrame;
		}
		else
		{
			//CombFactors[CurrentFrame * 2] = GetCombFactor(ppOddLines[(CurrentFrame + 4) % 5], ppEvenLines[CurrentFrame] + 1);

			// if we have dropped a field then do BOB
			if(LastOddFrame != ((CurrentFrame + 4) % 5))
			{
				for (nLineTarget = 0; nLineTarget < CurrentY / 2; nLineTarget++)
				{
					// copy latest field data to both odd and even rows
					pSource = ppEvenLines[CurrentFrame][nLineTarget];
					ScreenPtr = lpCurOverlay + (nLineTarget * 2) * OverlayPitch;
					ScreenPtr2 = lpCurOverlay + (nLineTarget * 2 + 1) * OverlayPitch;
					memcpyBOBMMX(pDestEven[nLineTarget], 
								pDestOdd[nLineTarget], 
								ppEvenLines[CurrentFrame][nLineTarget],
								CurrentX * 2);
				}
			}
			else if(gPulldownMode == VIDEO_MODE)
			{
				if(bUseBTVPlugin == TRUE)
				{
					BTVParams.IsOddField = 0;
					BTVParams.ppCurrentField = ppEvenLines[CurrentFrame];
					BTVParams.ppLastField = ppOddLines[LastOddFrame];
					BTVParams.ppEvenDest = pDestEven;
					BTVParams.ppOddDest = pDestOdd;
					BTVPluginDoField(&BTVParams);
				}
				else
				{
					Weave(ppOddLines[LastOddFrame], ppEvenLines[CurrentFrame], lpCurOverlay);
				}
			}
			else
			{
				for (nLineTarget = 0; nLineTarget < CurrentY / 2; nLineTarget++)
				{
					// copy latest data to destination buffer
					memcpyMMX(pDestEven[nLineTarget], 
								ppEvenLines[CurrentFrame][nLineTarget], 
								CurrentX  * 2);
				}
			}
			LastEvenFrame = CurrentFrame;
		}

		if(DoWeWantToFlip())
		{
			IDirectDrawSurface_Flip(lpDDOverlay, lpDDOverlayBack, DDFLIP_WAIT);
			if(lpCurOverlay == lpOverlay)
			{
				lpCurOverlay = lpOverlayBack;
			}
			else
			{
				lpCurOverlay = lpOverlay;
			}
		}
	
		if (bDisplayStatusBar == TRUE)
		{
			if (dwLastCount + 1000 < GetTickCount())
			{
				sprintf(Text, "%d FPS", nFrame);
				SetWindowText(hwndFPSField, Text);
				nFrame = 0;
				dwLastCount = GetTickCount();
			}
		}
	}
	ExitThread(0);
	return 0;
}

DWORD WINAPI YUVOutThreadNTSC(LPVOID lpThreadParameter)
{
	BYTE* ScreenPtr;
	BYTE* ScreenPtr2;
	char Text[128];
	int i, j;
	int nLineTarget;
	short* pSource;
	int nFrame = 0;
	DWORD dwLastCount;
	BYTE* lpCurOverlay = lpOverlayBack;
	//long CombFactors[10];
	short* ppEvenLines[5][CLEARLINES];
	short* ppOddLines[5][CLEARLINES];
	BYTE* pDestEven[CLEARLINES];
	BYTE* pDestOdd[CLEARLINES];
	//struct TPulldowmMode PulldownMode;
	int LastEvenFrame = 0;
	int LastOddFrame = 0;
	int CombNum = 0;

	if (lpDDOverlay == NULL || lpDDOverlay == NULL || lpOverlayBack == NULL || lpOverlay == NULL)
	{
		ExitThread(-1);
	}

	// Sets processor Affinity and Thread priority according to menu selection
	SetupProcessorAndThread();

	// Set up 5 sets of pointers to the start of odd and even lines
	// we will always go up to the limit so that we can use bTV
	for (j = 0; j < 5; j++)
	{
		for (i = 0; i < BTV_VER1_HEIGHT; i += 2)
		{
			ppOddLines[j][i / 2] = (short *) pDisplay[j] + (i + 1) * 1024;
			ppEvenLines[j][i / 2] = (short *) pDisplay[j] + i * 1024;
		}
	}

	// make sure we start with a clean sheet of paper
	Black_Surface();
	Black_Overlays();

	// display the current pulldown mode
	UpdatePulldownStatus();
	
	dwLastCount = GetTickCount();

	while(!bStopThread)
	{
		nFrame++;
		Get_Thread_Status();

		// set up desination pointers
		// may need to optimize this later
		for (i = 0; i < BTV_VER1_HEIGHT; i += 2)
		{
			pDestEven[i / 2] = lpCurOverlay + i * OverlayPitch;
			pDestOdd[i / 2] = lpCurOverlay + (i + 1) * OverlayPitch;
		}

		if(bIsOddField)
		{
			//CombFactors[CurrentFrame * 2 + 1] = GetCombFactor(ppEvenLines[CurrentFrame], ppOddLines[CurrentFrame]);
			if(CurrentFrame == 4)
			{
				// update the gPulldownMode based on last set of 5 frames
				// doesn't check for dropped frames but this shouldn't matter
				// too much
				//UpdateNTSCPulldownMode(&PulldownMode, CombFactors);
			}
			// if we have dropped a field then do BOB 
			if(LastEvenFrame != CurrentFrame)
			{
				for (nLineTarget = 0; nLineTarget < CurrentY / 2; nLineTarget++)
				{
					memcpyBOBMMX(pDestEven[nLineTarget], 
								pDestOdd[nLineTarget], 
								ppOddLines[CurrentFrame][nLineTarget], 
								CurrentX * 2);
				}
			}
			else if(gPulldownMode == VIDEO_MODE)
			{
				if(bUseBTVPlugin == TRUE)
				{
					BTVParams.IsOddField = 1;
					BTVParams.ppCurrentField = ppOddLines[CurrentFrame];
					BTVParams.ppLastField = ppEvenLines[CurrentFrame];
					BTVParams.ppEvenDest = pDestEven;
					BTVParams.ppOddDest = pDestOdd;
					BTVPluginDoField(&BTVParams);
				}
				else
				{
					Weave(ppOddLines[CurrentFrame], ppEvenLines[CurrentFrame], lpCurOverlay);
				}
			}
			else
			{
				for (nLineTarget = 0; nLineTarget < CurrentY / 2; nLineTarget++)
				{
					// copy latest data to destination buffer
					memcpyMMX(pDestOdd[nLineTarget], 
						ppOddLines[CurrentFrame][nLineTarget], 
						CurrentX * 2);
				}
			}
			LastOddFrame = CurrentFrame;
		}
		else
		{
			//CombFactors[CurrentFrame * 2] = GetCombFactor(ppOddLines[(CurrentFrame + 4) % 5], ppEvenLines[CurrentFrame] + 1);

			// if we have dropped a field then do BOB
			if(LastOddFrame != ((CurrentFrame + 4) % 5))
			{
				for (nLineTarget = 0; nLineTarget < CurrentY / 2; nLineTarget++)
				{
					// copy latest field data to both odd and even rows
					pSource = ppEvenLines[CurrentFrame][nLineTarget];
					ScreenPtr = lpCurOverlay + (nLineTarget * 2) * OverlayPitch;
					ScreenPtr2 = lpCurOverlay + (nLineTarget * 2 + 1) * OverlayPitch;
					memcpyBOBMMX(pDestEven[nLineTarget], 
								pDestOdd[nLineTarget], 
								ppEvenLines[CurrentFrame][nLineTarget],
								CurrentX * 2);
				}
			}
			else if(gPulldownMode == VIDEO_MODE)
			{
				if(bUseBTVPlugin == TRUE)
				{
					BTVParams.IsOddField = 0;
					BTVParams.ppCurrentField = ppEvenLines[CurrentFrame];
					BTVParams.ppLastField = ppOddLines[LastOddFrame];
					BTVParams.ppEvenDest = pDestEven;
					BTVParams.ppOddDest = pDestOdd;
					BTVPluginDoField(&BTVParams);
				}
				else
				{
					Weave(ppOddLines[LastOddFrame], ppEvenLines[CurrentFrame], lpCurOverlay);
				}
			}
			else
			{
				for (nLineTarget = 0; nLineTarget < CurrentY / 2; nLineTarget++)
				{
					// copy latest data to destination buffer
					memcpyMMX(pDestEven[nLineTarget], 
								ppEvenLines[CurrentFrame][nLineTarget], 
								CurrentX  * 2);
				}
			}
			LastEvenFrame = CurrentFrame;
		}

		if(DoWeWantToFlip())
		{
			IDirectDrawSurface_Flip(lpDDOverlay, lpDDOverlayBack, DDFLIP_WAIT);
			if(lpCurOverlay == lpOverlay)
			{
				lpCurOverlay = lpOverlayBack;
			}
			else
			{
				lpCurOverlay = lpOverlay;
			}
		}
	
		if (bDisplayStatusBar == TRUE)
		{
			if (dwLastCount + 1000 < GetTickCount())
			{
				sprintf(Text, "%d FPS", nFrame);
				SetWindowText(hwndFPSField, Text);
				nFrame = 0;
				dwLastCount = GetTickCount();
			}
		}
	}
	ExitThread(0);
	return 0;
}
