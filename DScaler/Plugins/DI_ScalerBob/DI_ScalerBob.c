/////////////////////////////////////////////////////////////////////////////
// $Id: DI_ScalerBob.c,v 1.4 2001-07-13 16:13:33 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//  This file is subject to the terms of the GNU General Public License as
//  published by the Free Software Foundation.  A copy of this license is
//  included with this software distribution in the file COPYING.  If you
//  do not have a copy, you may obtain a copy by writing to the Free
//  Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 04 Jan 2001   John Adcock           Split into separate module
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DS_Deinterlace.h"

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
                        info->LineLength);
        }
        else
        {
            info->pMemcpy(info->Overlay + nLineTarget * info->OverlayPitch,
                        info->EvenLines[0][nLineTarget],
                        info->LineLength);
        }
    }
    // need to clear up MMX registers
    _asm
    {
        emms
    }
    return TRUE;
}

DEINTERLACE_METHOD ScalerBobMethod =
{
    sizeof(DEINTERLACE_METHOD),
    DEINTERLACE_CURRENT_VERSION,
    "Scaler Bob", 
    NULL,
    TRUE,
    FALSE,
    DeinterlaceScalerBob,
    50, 
    60,
    0,
    NULL,
    INDEX_SCALER_BOB,
    NULL,
    NULL,
    NULL,
    NULL,
    1,
    0,
    0,
    -1,
    NULL,
    0,
    FALSE,
    FALSE,
};


__declspec(dllexport) DEINTERLACE_METHOD* GetDeinterlacePluginInfo(long CpuFeatureFlags)
{
    return &ScalerBobMethod;
}

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}

