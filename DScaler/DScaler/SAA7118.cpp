/////////////////////////////////////////////////////////////////////////////
//
// $Id$
//
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 John Adcock.  All rights reserved.
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

/**
 * @file SAA7118.cpp CSAA7118 Implementation
 */

#include "stdafx.h"
#include "SAA7118.h"
#include "DebugLog.h"

CSAA7118::CSAA7118()
{
}

void CSAA7118::SetBrightness(BYTE Brightness)
{
    WriteToSubAddress(0x0a, &Brightness, 1);
}

void CSAA7118::SetComponentBrightness(BYTE Brightness)
{
    WriteToSubAddress(0x2a, &Brightness, 1);
}

void CSAA7118::SetContrast(BYTE Contrast)
{
    // Contrast limited to 0-128
    Contrast = Contrast >> 1;
    WriteToSubAddress(0x0b, &Contrast, 1);
}

void CSAA7118::SetComponentContrast(BYTE Contrast)
{
    // Contrast limited to 0-128
    Contrast = Contrast >> 1;
    WriteToSubAddress(0x2b, &Contrast, 1);
}

void CSAA7118::SetHue(BYTE Hue)
{
    Hue = (BYTE)(signed char)(Hue - 0x80);
    WriteToSubAddress(0x0d, &Hue, 1);
}

void CSAA7118::SetSaturation(BYTE Saturation)
{
    // Saturation limited to 0-128
    Saturation = Saturation >> 1;
    WriteToSubAddress(0x0c, &Saturation, 1);
}


void CSAA7118::SetComponentSaturation(BYTE Saturation)
{
    // Saturation limited to 0-128
    Saturation = Saturation >> 1;
    WriteToSubAddress(0x2c, &Saturation, 1);
}

BYTE CSAA7118::GetVersion()
{
    BYTE Result(0);
    ReadFromSubAddress(0x00, &Result, 1);
    return Result;
}

void CSAA7118::SetRegister(BYTE Register, BYTE Value)
{
    WriteToSubAddress(Register, &Value, 1);
}

BYTE CSAA7118::GetRegister(BYTE Register)
{
    BYTE Result(0);
    ReadFromSubAddress(Register, &Result, 1);
    return Result;
}

void CSAA7118::DumpSettings(LPCSTR Filename)
{
    FILE* hFile;

    hFile = fopen(Filename, "w");
    if(!hFile)
    {
        return;
    }

    int i;
    for(i = 0; i < 0x2d; ++i)
    {
        fprintf(hFile, "%02x\t%02x\n", i, GetRegister(i));
    }

    for(i = 0x40; i <= 0x62; ++i)
    {
        fprintf(hFile, "%02x\t%02x\n", i, GetRegister(i));
    }

    for(i = 0x80; i <= 0xBF; ++i)
    {
        fprintf(hFile, "%02x\t%02x\n", i, GetRegister(i));
    }

    fclose(hFile);
}

BYTE CSAA7118::GetDefaultAddress()const
{
    return 0x42>>1;
}
