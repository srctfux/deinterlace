/////////////////////////////////////////////////////////////////////////////
// DI_ScalerBob.c
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
// 04 Jan 2001   John Adcock           Split into separate module
//
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "dTV_Deinterlace.h"

BOOL DeinterlaceScalerBob(DEINTERLACE_INFO *info)
{
	int nLineTarget;

	for (nLineTarget = 0; nLineTarget < info->FieldHeight; nLineTarget++)
	{
		// copy latest field's rows to overlay, resulting in a half-height image.
		if (info->IsOdd)
		{
			info->pMemcpy(info->Overlay + nLineTarget * info->OverlayPitch,
						info->OddLines[0][nLineTarget],
						info->LineLength * 2);
		}
		else
		{
			info->pMemcpy(info->Overlay + nLineTarget * info->OverlayPitch,
						info->EvenLines[0][nLineTarget],
						info->LineLength * 2);
		}
	}
	return TRUE;
}

DEINTERLACE_METHOD ScalerBobMethod =
{
	"Scaler Bob", 
	NULL,
	"S&caler BOB",
	TRUE,
	FALSE,
	DeinterlaceScalerBob,
	50, 
	60,
	0,
	NULL,
	5,
	NULL,
	NULL,
	INDEX_SCALER_BOB,
	0,
	0,
	-1,
};


__declspec(dllexport) DEINTERLACE_METHOD* GetDeinterlacePluginInfo(long CpuFeatureFlags)
{
	return &ScalerBobMethod;
}

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}

