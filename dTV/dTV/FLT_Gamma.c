/////////////////////////////////////////////////////////////////////////////
// FLT_TNoise.c
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

#include "stdafx.h"
#include "deinterlace.h"
#include "cpu.h"
#include "FLT_Gamma.h"


char GammaTable[256];

BOOL Filter_Gamma(DEINTERLACE_INFO *info)
{
	short *Pixels;
	short *Table;
	int y;
	int Cycles;

	// Need to have the current and next-to-previous fields to do the filtering.
	if ((info->IsOdd && info->OddLines[0] == NULL) ||
		(! info->IsOdd && info->EvenLines[0] == NULL))
	{
		return FALSE;
	}

	Cycles = info->LineLength / 2;

	for(y = 0; y < 255; y++)
	{
		GammaTable[y] = 255 - y;
	}

	Table = (short*)GammaTable;

	for (y = 0; y < info->FieldHeight; y++)
	{
		if (info->IsOdd)
		{
			Pixels = info->OddLines[0][y];
		}
		else
		{
			Pixels = info->EvenLines[0][y];
		}

		_asm
		{
			mov ecx, Cycles
			mov edx, dword ptr[Pixels]
			mov ebx, dword ptr[Table]
LOOP_LABEL:
			mov al,byte ptr[edx]
			xlatb
			mov byte ptr[edx], al
			add edx, 2			
			loop LOOP_LABEL
		}

	}
	return TRUE;
}

