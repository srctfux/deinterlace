/////////////////////////////////////////////////////////////////////////////
// DI_HalfHeight.c
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

#include "stdafx.h"
#include "deinterlace.h"
#include "cpu.h"


BOOL HalfHeightBoth(DEINTERLACE_INFO *info)
{
	int nLineTarget;

	for (nLineTarget = 0; nLineTarget < info->FieldHeight; nLineTarget++)
	{
		// copy latest field's rows to overlay, resulting in a half-height image.
		if (info->IsOdd)
		{
			memcpyMMX(info->Overlay + nLineTarget * info->OverlayPitch,
						info->OddLines[0][nLineTarget],
						info->LineLength * 2);
		}
		else
		{
			memcpyMMX(info->Overlay + nLineTarget * info->OverlayPitch,
						info->EvenLines[0][nLineTarget],
						info->LineLength * 2);
		}
	}
	return TRUE;
}

BOOL HalfHeightEvenOnly(DEINTERLACE_INFO *info)
{
	int nLineTarget;

	if (!info->IsOdd)
	{
		for (nLineTarget = 0; nLineTarget < info->FieldHeight; nLineTarget++)
		{
			// copy latest field's rows to overlay, resulting in a half-height image.
			memcpyMMX(info->Overlay + nLineTarget * info->OverlayPitch,
						info->EvenLines[0][nLineTarget],
						info->LineLength * 2);
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL HalfHeightOddOnly(DEINTERLACE_INFO *info)
{
	int nLineTarget;

	if (info->IsOdd)
	{
		for (nLineTarget = 0; nLineTarget < info->FieldHeight; nLineTarget++)
		{
		// copy latest field's rows to overlay, resulting in a half-height image.
			memcpyMMX(info->Overlay + nLineTarget * info->OverlayPitch,
						info->OddLines[0][nLineTarget],
						info->LineLength * 2);
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
