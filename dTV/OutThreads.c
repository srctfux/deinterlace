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
/////////////////////////////////////////////////////////////////////////////

#include "OutThreads.h"
#include "other.h"
#include "bt848.h"


short pPALplusCode[] = {  18,  27,  36,  45,  54,  63,  72,  81,  90, 100, 110, 120, 134, 149};
short pPALplusData[] = { 160, 178, 196, 214, 232, 250, 268, 286, 304, 322, 340, 358, 376, 394};
short nLevelLow      =  45;
short nLevelHigh     = 135;

BOOL bStopThread = FALSE;
HANDLE OutThread;

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

	OutThread = CreateThread((LPSECURITY_ATTRIBUTES) NULL,	// No security.
							 (DWORD) 0,	// Same stack size.
							 YUVOutThread,	// Thread procedure.
							 NULL,	// Parameter.
							 (DWORD) 0,	// Start immediatly.
							 (LPDWORD) & LinkThreadID);	// Thread ID.
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
			mov dword ptr[LineFactor], eax
			emms
		}
		CombFactor += (long)sqrt(LineFactor);
	}
	return CombFactor;
}

/////////////////////////////////////////////////////////////////////////////
// CopyLine
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

void UpdatePulldownMode(int* PulldownRepeatCount, long* CombFactors)
{

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
		SetWindowText(hwndPalField, "Video Source Mode");
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

DWORD WINAPI YUVOutThread(LPVOID lpThreadParameter)
{
	BYTE *ScreenPtr;
	char Text[128];
	int i, j;
	int nLineTarget;
	short* pSource;
	int nFrame = 0;
	DWORD dwLastCount;
	BYTE* lpCurOverlay = lpOverlayBack;
	long CombFactors[10];
	short *pLinesAddressEven[5*CLEARLINES];
	short *pLinesAddressOdd[5*CLEARLINES];
	int PulldownRepeatCount;
	int LastEvenFrame = -1;
	int LastOddFrame = -1;

	if (lpDDOverlay == NULL || lpDDOverlay == NULL || lpOverlayBack == NULL || lpOverlay == NULL)
	{
		ExitThread(-1);
	}

	// Sets processor Affinity and Thread priority according to menu selection
	SetupProcessorAndThread();

	// Set up 5 sets of pointers to the start of odd and even lines
	for (j = 0; j < 5; j++)
	{
		for (i = 0; i < CurrentY; i += 2)
		{
			pLinesAddressOdd[(j * CLEARLINES) + (i / 2)] = (short *) pDisplay[j] + (i + 1) * 1024;
			pLinesAddressEven[(j * CLEARLINES) + (i / 2)] = (short *) pDisplay[j] + i * 1024;
		}
	}

	Black_Surface();

	// blank out front and back buffers
	for (nLineTarget = 0; nLineTarget < CurrentY / 2; nLineTarget++)
	{
		ScreenPtr = lpOverlay + (nLineTarget * 2) * OverlayPitch;
		memset(ScreenPtr, 0, CurrentX  * 2);
		ScreenPtr = lpOverlay + (nLineTarget * 2 + 1) * OverlayPitch;
		memset(ScreenPtr, 0, CurrentX  * 2);
		ScreenPtr = lpOverlayBack + (nLineTarget * 2) * OverlayPitch;
		memset(ScreenPtr, 0, CurrentX  * 2);
		ScreenPtr = lpOverlayBack + (nLineTarget * 2 + 1) * OverlayPitch;
		memset(ScreenPtr, 0, CurrentX  * 2);
	}

	UpdatePulldownStatus();
	
	dwLastCount = GetTickCount();

	while(!bStopThread)
	{
		nFrame++;
		Get_Thread_Status();

		// move down old comb factors
		memcpy(CombFactors, CombFactors + 1, 9 * sizeof(long));
		if(bIsOddField)
		{
			CombFactors[9] = GetCombFactor(pLinesAddressEven + LastEvenFrame * CLEARLINES, pLinesAddressOdd + CurrentFrame * CLEARLINES);
		}
		else
		{
			CombFactors[9] = GetCombFactor(pLinesAddressOdd + CurrentFrame * CLEARLINES, pLinesAddressEven + CurrentFrame * CLEARLINES + 1);
		}

		// Copy new field data onto current back overlay
		// we always do this even if we are in 3:2 pulldown mode
		// so that the frame rate will be as smooth as possible
		// we should do roughly the same amount of work for each field
		// in video mode do weave for the time being
		for (nLineTarget = 0; nLineTarget < CurrentY / 2; nLineTarget++)
		{
			if(bIsOddField)
			{
				// copy latest data to destination buffer
				pSource = pLinesAddressOdd[CurrentFrame * CLEARLINES + nLineTarget];
				ScreenPtr = lpCurOverlay + (nLineTarget * 2 + 1) * OverlayPitch;
				memcpyMMX(ScreenPtr, pSource, CurrentX * 2);
				// if we have dropped a frame then do BOB 
				if(LastEvenFrame != CurrentFrame)
				{
					ScreenPtr = lpCurOverlay + (nLineTarget * 2) * OverlayPitch;
					memcpyMMX(ScreenPtr, pSource, CurrentX * 2);
				}
				// if we are in video mode we flip every time so we have to copy the
				// previous field
				else if(gPulldownMode == VIDEO_MODE)
				{
					pSource = pLinesAddressEven[LastEvenFrame * CLEARLINES + nLineTarget];
					ScreenPtr = lpCurOverlay + (nLineTarget * 2) * OverlayPitch;
					memcpyMMX(ScreenPtr, pSource, CurrentX * 2);
				}
			}
			else
			{
				// copy latest data to destination buffer
				pSource = pLinesAddressEven[CurrentFrame * CLEARLINES + nLineTarget];
				ScreenPtr = lpCurOverlay + (nLineTarget * 2) * OverlayPitch;
				memcpyMMX(ScreenPtr, pSource, CurrentX  * 2);
				// if we have dropped a frame then do BOB
				if(LastOddFrame != ((CurrentFrame + 4) % 5))
				{
					ScreenPtr = lpCurOverlay + (nLineTarget * 2 + 1) * OverlayPitch;
					memcpyMMX(ScreenPtr, pSource, CurrentX  * 2);
				}
				// if we are in video mode we flip every time so we have to copy the
				// previous field
				else if(gPulldownMode == VIDEO_MODE)
				{
					pSource = pLinesAddressOdd[CurrentFrame * CLEARLINES + nLineTarget];
					ScreenPtr = lpCurOverlay + (nLineTarget * 2 + 1) * OverlayPitch;
					memcpyMMX(ScreenPtr, pSource, CurrentX  * 2);
				}
			}
		}
		
		// doesn't do anything at the moment
		UpdatePulldownMode(&PulldownRepeatCount, CombFactors);

		if(bIsOddField)
		{
			LastOddFrame = CurrentFrame;
		}
		else
		{
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
