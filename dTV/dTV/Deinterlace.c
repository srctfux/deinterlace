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
// 09 Nov 2000   Tom Barry		       Added Blended Clipping Deinterlace method
//
// 30 Dec 2000   Mark Rejhon           Split out deinterlace routines
//                                     into separate modules
//
// 05 Jan 2001   John Adcock           Added flip frequencies to DeintMethods
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 09 Jan 2001   John Adcock           Split out memcpySSE as separate function
//                                     Changed DeintMethods to reflect the two
//                                     film mode functions replacing the one before
//                                     Moved CombFactor and CompareFields to new file
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "deinterlace.h"
#include "cpu.h"
#include "bt848.h"
#include "dTV.h"
#include "OutThreads.h"
#include "FD_50Hz.h"
#include "FD_60Hz.h"
#include "AspectRatio.h"
#include "Status.h"

DEINTERLACE_METHOD FilmDeintMethods[FILMPULLDOWNMODES_LAST_ONE] =
{
	// FILM_22_PULLDOWN_ODD
	{"2:2 Pulldown Flip on Odd", "2:2 Odd", NULL, FALSE, TRUE, FilmModePALOdd, 25, 30, 0, NULL, 0, NULL, NULL, 2, 0, 0, -1,},
	// FILM_22_PULLDOWN_EVEN
	{"2:2 Pulldown Flip on Even", "2:2 Even", NULL, FALSE, TRUE, FilmModePALEven, 25, 30, 0, NULL, 0, NULL, NULL, 2, 0, 0, -1,},
	// FILM_32_PULLDOWN_0
	{"3:2 Pulldown Skip 1st Full Frame", "3:2 1st", NULL, FALSE, TRUE, FilmModeNTSC1st, 1000, 24, 0, NULL, 0, NULL, NULL, 2, 0, 0, -1,},
	// FILM_32_PULLDOWN_1
	{"3:2 Pulldown Skip 2nd Full Frame", "3:2 2nd", NULL, FALSE, TRUE, FilmModeNTSC2nd, 1000, 24, 0, NULL, 0, NULL, NULL, 2, 0, 0, -1,},
	// FILM_32_PULLDOWN_2
	{"3:2 Pulldown Skip 3rd Full Frame", "3:2 3rd", NULL, FALSE, TRUE, FilmModeNTSC3rd, 1000, 24, 0, NULL, 0, NULL, NULL, 2, 0, 0, -1,},
	// FILM_32_PULLDOWN_3
	{"3:2 Pulldown Skip 4th Full Frame", "3:2 4th", NULL, FALSE, TRUE, FilmModeNTSC4th, 1000, 24, 0, NULL, 0, NULL, NULL, 2, 0, 0, -1,},
	// FILM_32_PULLDOWN_4
	{"3:2 Pulldown Skip 5th Full Frame", "3:2 5th", NULL, FALSE, TRUE, FilmModeNTSC5th, 1000, 24, 0, NULL, 0, NULL, NULL, 2, 0, 0, -1,},
};

long NumVideoModes = 0;
DEINTERLACE_METHOD* VideoDeintMethods[100] = {NULL,};
BOOL bIsFilmMode = FALSE;

long gVideoPulldownMode = 0;
long gFilmPulldownMode = 0;

DEINTERLACE_METHOD* GetCurrentDeintMethod()
{
	if(bIsFilmMode)
	{
		return FilmDeintMethods + gFilmPulldownMode;
	}
	else
	{
		return VideoDeintMethods[gVideoPulldownMode];
	}
}

DEINTERLACE_METHOD* GetVideoDeintMethod(int Mode)
{
	if(Mode < NumVideoModes)
	{
		return VideoDeintMethods[gVideoPulldownMode];
	}
	else
	{
		return NULL;
	}
}

DEINTERLACE_METHOD* GetFilmDeintMethod(eFILMPULLDOWNMODES Mode)
{
	if(Mode < FILMPULLDOWNMODES_LAST_ONE)
	{
		return FilmDeintMethods + gFilmPulldownMode;
	}
	else
	{
		return NULL;
	}
}


BOOL IsFilmMode()
{
	return bIsFilmMode;
}

BOOL InHalfHeightMode()
{
	if(bIsFilmMode)
	{
		return FALSE;
	}
	else
	{
		return VideoDeintMethods[gVideoPulldownMode]->bIsHalfHeight;
	}
}

eFILMPULLDOWNMODES GetFilmMode()
{
	if(bIsFilmMode)
	{
		return gFilmPulldownMode;
	}
	else
	{
		return FILMPULLDOWNMODES_LAST_ONE;
	}
}

void SetFilmDeinterlaceMode(int mode)
{
	if (gFilmPulldownMode != mode || bIsFilmMode == FALSE)
	{
		DWORD CurrentTickCount = GetTickCount();

		if (nInitialTicks == -1)
		{
			nInitialTicks = CurrentTickCount;
			nLastTicks = CurrentTickCount;
		}
		else
		{
			if(bIsFilmMode == TRUE)
			{
				FilmDeintMethods[gFilmPulldownMode].ModeTicks += CurrentTickCount - nLastTicks;
			}
			else
			{
				VideoDeintMethods[gVideoPulldownMode]->ModeTicks += CurrentTickCount - nLastTicks;
			}
		}
		gFilmPulldownMode = mode;
		bIsFilmMode = TRUE;
		nLastTicks = CurrentTickCount;
		StatusBar_ShowText(STATUS_PAL, GetDeinterlaceModeName());
		nTotalDeintModeChanges++;
		FilmDeintMethods[gFilmPulldownMode].ModeChanges++;
		SetHalfHeight(FilmDeintMethods[gFilmPulldownMode].bIsHalfHeight);
	}
}

void SetVideoDeinterlaceMode(int mode)
{
	if (gVideoPulldownMode != mode || bIsFilmMode == TRUE)
	{
		DWORD CurrentTickCount = GetTickCount();

		if (nInitialTicks == -1)
		{
			nInitialTicks = CurrentTickCount;
			nLastTicks = CurrentTickCount;
		}
		else
		{
			if(bIsFilmMode == TRUE)
			{
				FilmDeintMethods[gFilmPulldownMode].ModeTicks += CurrentTickCount - nLastTicks;
			}
			else
			{
				VideoDeintMethods[gVideoPulldownMode]->ModeTicks += CurrentTickCount - nLastTicks;
			}
		}
		gVideoPulldownMode = mode;
		bIsFilmMode = FALSE;
		nLastTicks = CurrentTickCount;
		StatusBar_ShowText(STATUS_PAL, GetDeinterlaceModeName());
		nTotalDeintModeChanges++;
		VideoDeintMethods[gVideoPulldownMode]->ModeChanges++;
		SetHalfHeight(VideoDeintMethods[gVideoPulldownMode]->bIsHalfHeight);
	}
}

void SetVideoDeinterlaceIndex(int index)
{
	int i;
	for(i = 0; i < NumVideoModes; i++)
	{
		if(VideoDeintMethods[i]->nMethodIndex == index)
		{
			SetVideoDeinterlaceMode(i);
			return;
		}
	}
}

char* GetDeinterlaceModeName()
{
	if(bIsFilmMode)
	{
		return FilmDeintMethods[gFilmPulldownMode].szName;
	}
	else
	{
		return VideoDeintMethods[gVideoPulldownMode]->szName;
	}
}

void PrepareDeinterlaceMode()
{
	bIsFilmMode = FALSE;
	if(BT848_GetTVFormat()->Is25fps)
	{
		SetVideoDeinterlaceIndex(Setting_GetValue(FD50_GetSetting(PALFILMFALLBACKMODE)));
	}
	else
	{
		SetVideoDeinterlaceIndex(Setting_GetValue(FD60_GetSetting(NTSCFILMFALLBACKMODE)));
	}
	// If that didn't work then go into whatever they loaded up first
	if(gVideoPulldownMode == -1)
	{
		SetVideoDeinterlaceMode(0);
	}
}

void IncrementDeinterlaceMode()
{
	long Mode;
	if(bIsFilmMode)
	{
		Mode = gFilmPulldownMode;
		Mode++;
		if(Mode == FILMPULLDOWNMODES_LAST_ONE)
		{
			SetVideoDeinterlaceMode(0);
		}
		else
		{
			SetFilmDeinterlaceMode(Mode);
		}
	}
	else
	{
		Mode = gVideoPulldownMode;
		Mode++;
		if(Mode == NumVideoModes)
		{
			SetFilmDeinterlaceMode(FILM_22_PULLDOWN_ODD);
		}
		else
		{
			SetVideoDeinterlaceMode(Mode);
		}
	}
}

void DecrementDeinterlaceMode()
{
	long Mode;
	if(bIsFilmMode)
	{
		Mode = gFilmPulldownMode;
		Mode--;
		if(Mode < 0)
		{
			SetVideoDeinterlaceMode(NumVideoModes - 1);
		}
		else
		{
			SetFilmDeinterlaceMode(Mode);
		}
	}
	else
	{
		Mode = gVideoPulldownMode;
		Mode--;
		if(Mode < 0)
		{
			SetFilmDeinterlaceMode(FILM_32_PULLDOWN_4);
		}
		else
		{
			SetVideoDeinterlaceMode(Mode);
		}
	}
}

BOOL ProcessDeinterlaceSelection(HWND hWnd, WORD wMenuID)
{
	int i;
	for(i = 0; i < NumVideoModes; i++)
	{
		if(wMenuID == VideoDeintMethods[i]->MenuId)
		{
			SetVideoDeinterlaceMode(i);
			return TRUE;
		}
	}
	return FALSE;
}

void LoadPlugin(LPCSTR szFileName)
{
	GETDEINTERLACEPLUGININFO* pfnGetDeinterlacePluginInfo;
	DEINTERLACE_METHOD* pMethod;
	HMODULE hPlugInMod;

	hPlugInMod = LoadLibrary(szFileName);
	if(hPlugInMod == NULL)
	{
		return;
	}
	
	pfnGetDeinterlacePluginInfo = (GETDEINTERLACEPLUGININFO*)GetProcAddress(hPlugInMod, "GetDeinterlacePluginInfo");
	if(pfnGetDeinterlacePluginInfo == NULL)
	{
		return;
	}

	pMethod = pfnGetDeinterlacePluginInfo(CpuFeatureFlags);
	if(pMethod != NULL)
	{
		VideoDeintMethods[NumVideoModes] = pMethod;
		pMethod->hModule = hPlugInMod;
		NumVideoModes++;
	}
}

void UnloadDeinterlacePlugins()
{
	int i;
	for(i = 0; i < NumVideoModes; i++)
	{
		VideoDeintMethods[i]->pfnPluginExit();
		FreeLibrary(VideoDeintMethods[i]->hModule);
		VideoDeintMethods[i] = NULL;
	}
	NumVideoModes = 0;
}

int MethodCompare( const void *arg1, const void *arg2 )
{
	DEINTERLACE_METHOD* pMethod1 = *(DEINTERLACE_METHOD**)arg1;
	DEINTERLACE_METHOD* pMethod2 = *(DEINTERLACE_METHOD**)arg2;

	if(pMethod1->nMethodIndex < pMethod2->nMethodIndex)
	{
		return -1;
	}
	else if(pMethod1->nMethodIndex > pMethod2->nMethodIndex)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void AddUIForDeintPlugin(HMENU hMenu, DEINTERLACE_METHOD* DeintMethod)
{
	static MenuId = 6000;
	DeintMethod->MenuId = MenuId++;
	if(DeintMethod->szMenuName != NULL)
	{
		AppendMenu(hMenu, MF_STRING | MF_ENABLED, DeintMethod->MenuId, DeintMethod->szMenuName);
	}
	else
	{
		AppendMenu(hMenu, MF_STRING | MF_ENABLED, DeintMethod->MenuId, DeintMethod->szName);
	}
}

BOOL LoadDeinterlacePlugins()
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFindFile;
	int i;
	HMENU hMenu;

	hFindFile = FindFirstFile("DI_*.dll", &FindFileData);

	if (hFindFile != INVALID_HANDLE_VALUE)
	{
		BOOL RetVal = TRUE;
    	while(RetVal != 0)
		{
			LoadPlugin(FindFileData.cFileName);
			RetVal = FindNextFile(hFindFile, &FindFileData);
		}
	}

	// put the plug-ins into index order
	// this should prevent confusion in the UI
	if(NumVideoModes > 1)
	{
		qsort((void*) VideoDeintMethods, NumVideoModes, sizeof(DEINTERLACE_METHOD*), MethodCompare);
	}
	if(NumVideoModes > 0)
	{
		hMenu = GetMenu(hWnd);
		if(hMenu == NULL) return FALSE;
		hMenu = GetSubMenu(hMenu, 4);
		if(hMenu == NULL) return FALSE;
		hMenu = GetSubMenu(hMenu, 1);
		if(hMenu == NULL) return FALSE;

		for(i = 0; i < NumVideoModes; i++)
		{
			AddUIForDeintPlugin(hMenu, VideoDeintMethods[i]);
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


/////////////////////////////////////////////////////////////////////////////
// memcpyMMX
// Uses MMX instructions to move memory around
// does as much as we can in 64 byte chunks (128-byte on SSE machines)
// using MMX instructions
// then copies any extra bytes
// assumes there will be at least 64 bytes to copy
// This code was originally from Borg's bTV plugin SDK 
/////////////////////////////////////////////////////////////////////////////
void memcpyMMX(void *Dest, void *Src, size_t nBytes)
{
	__asm {
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

#ifdef USE_SSE
/////////////////////////////////////////////////////////////////////////////
// memcpySSE
// On SSE machines, we can use the 128-bit floating-point registers and
// bypass write caching to copy a bit faster.  The destination has to be
// 16-byte aligned.  
/////////////////////////////////////////////////////////////////////////////
void memcpySSE(void *Dest, void *Src, size_t nBytes)
{
	__asm {
		mov		esi, dword ptr[Src]
		mov		edi, dword ptr[Dest]
		mov		ecx, nBytes
		shr     ecx, 7                      // nBytes / 128
align 8
CopyLoopSSE:
		// movaps should be slightly more efficient
		// as the data is 16 bit aligned
		movaps	xmm0, xmmword ptr[esi]
		movaps	xmm1, xmmword ptr[esi+16*1]
		movaps	xmm2, xmmword ptr[esi+16*2]
		movaps	xmm3, xmmword ptr[esi+16*3]
		movaps	xmm4, xmmword ptr[esi+16*4]
		movaps	xmm5, xmmword ptr[esi+16*5]
		movaps	xmm6, xmmword ptr[esi+16*6]
		movaps	xmm7, xmmword ptr[esi+16*7]
		movntps	xmmword ptr[edi], xmm0
		movntps	xmmword ptr[edi+16*1], xmm1
		movntps	xmmword ptr[edi+16*2], xmm2
		movntps	xmmword ptr[edi+16*3], xmm3
		movntps	xmmword ptr[edi+16*4], xmm4
		movntps	xmmword ptr[edi+16*5], xmm5
		movntps	xmmword ptr[edi+16*6], xmm6
		movntps	xmmword ptr[edi+16*7], xmm7
		add		esi, 128
		add		edi, 128
		loop CopyLoopSSE
		mov		ecx, nBytes
		and     ecx, 127
		cmp     ecx, 0
		je EndCopyLoopSSE
align 8
CopyLoop2SSE:
		mov dl, byte ptr[esi] 
		mov byte ptr[edi], dl
		inc esi
		inc edi
		dec ecx
		jne near CopyLoop2SSE
EndCopyLoopSSE:
		emms
	}
}
#endif

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
// there are no settings at the moment but here is a good place to set
// up the DeintModeNames array used where modes are to be selected
/////////////////////////////////////////////////////////////////////////////
SETTING* Deinterlace_GetSetting(long nIndex, long Setting)
{
	if(nIndex < 0 || nIndex >= NumVideoModes)
	{
		return NULL;
	}
	if(Setting > -1 && Setting < VideoDeintMethods[nIndex]->nSettings)
	{
		return &(VideoDeintMethods[nIndex]->pSettings[Setting]);
	}
	else
	{
		return NULL;
	}
}

LONG Deinterlace_HandleSettingsMsg(HWND hWnd, UINT message, UINT wParam, LONG lParam, BOOL* bDone)
{
	return 0;
}


void Deinterlace_ReadSettingsFromIni()
{
	int i,j;
	for(i = 0; i < NumVideoModes; i++)
	{
		for(j = 0; j < VideoDeintMethods[i]->nSettings; j++)
		{
			Setting_ReadFromIni(&(VideoDeintMethods[i]->pSettings[j]));
		}
	}
}

void Deinterlace_WriteSettingsToIni()
{
	int i,j;
	for(i = 0; i < NumVideoModes; i++)
	{
		for(j = 0; j < VideoDeintMethods[i]->nSettings; j++)
		{
			Setting_WriteToIni(&(VideoDeintMethods[i]->pSettings[j]));
		}
	}
}

void Deinterlace_SetMenu(HMENU hMenu)
{
	int i;

	if(bIsFilmMode)
	{
		CheckMenuItem(hMenu, IDM_22PULLODD, (gFilmPulldownMode == FILM_22_PULLDOWN_ODD) ?MF_CHECKED:MF_UNCHECKED);
		CheckMenuItem(hMenu, IDM_22PULLEVEN, (gFilmPulldownMode == FILM_22_PULLDOWN_EVEN) ?MF_CHECKED:MF_UNCHECKED);
		CheckMenuItem(hMenu, IDM_32PULL1, (gFilmPulldownMode == FILM_32_PULLDOWN_0) ?MF_CHECKED:MF_UNCHECKED);
		CheckMenuItem(hMenu, IDM_32PULL2, (gFilmPulldownMode == FILM_32_PULLDOWN_1) ?MF_CHECKED:MF_UNCHECKED);
		CheckMenuItem(hMenu, IDM_32PULL3, (gFilmPulldownMode == FILM_32_PULLDOWN_2) ?MF_CHECKED:MF_UNCHECKED);
		CheckMenuItem(hMenu, IDM_32PULL4, (gFilmPulldownMode == FILM_32_PULLDOWN_3) ?MF_CHECKED:MF_UNCHECKED);
		CheckMenuItem(hMenu, IDM_32PULL5, (gFilmPulldownMode == FILM_32_PULLDOWN_4) ?MF_CHECKED:MF_UNCHECKED);
	}
	else
	{
		CheckMenuItem(hMenu, IDM_22PULLODD, MF_UNCHECKED);
		CheckMenuItem(hMenu, IDM_22PULLEVEN, MF_UNCHECKED);
		CheckMenuItem(hMenu, IDM_32PULL1, MF_UNCHECKED);
		CheckMenuItem(hMenu, IDM_32PULL2, MF_UNCHECKED);
		CheckMenuItem(hMenu, IDM_32PULL3, MF_UNCHECKED);
		CheckMenuItem(hMenu, IDM_32PULL4, MF_UNCHECKED);
		CheckMenuItem(hMenu, IDM_32PULL5, MF_UNCHECKED);
	}

	for(i = 0; i < NumVideoModes; i++)
	{
		if(gVideoPulldownMode == i)
		{
			CheckMenuItem(hMenu, VideoDeintMethods[i]->MenuId, MF_CHECKED);
		}
		else
		{
			CheckMenuItem(hMenu, VideoDeintMethods[i]->MenuId, MF_UNCHECKED);
		}
	}
}
