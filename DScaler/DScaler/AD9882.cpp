/////////////////////////////////////////////////////////////////////////////
//
// $Id$
//
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2006 Curtiss-Wright Controls, Inc..  All rights reserved.
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
//
// $Log: not supported by cvs2svn $
// Revision 1.1  2006/12/28 14:18:35  adcockj
// Added patch for Curtiss-Wright cards from Bill Randle
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file AD9882.cpp CAD9882 Implementation
 */

#include "stdafx.h"
#include "AD9882.h"
#include "DebugLog.h"

CAD9882::CAD9882()
{
}

BYTE CAD9882::GetVersion()
{
    BYTE Result(0);
    ReadFromSubAddress(0x00, &Result, 1);
    return Result;
}

void CAD9882::SetRegister(BYTE Register, BYTE Value)
{
    WriteToSubAddress(Register, &Value, 1);
}

BYTE CAD9882::GetRegister(BYTE Register)
{
    BYTE Result(0);
    ReadFromSubAddress(Register, &Result, 1);
    return Result;
}

void CAD9882::DumpSettings(LPCSTR Filename)
{
    FILE* hFile;

    hFile = fopen(Filename, "w");
    if(!hFile)
    {
        return;
    }

    fprintf(hFile, "AD01 = 0x%02x\n", GetRegister(0x01));
    fprintf(hFile, "AD02 = 0x%02x\n", GetRegister(0x02));
    fprintf(hFile, "AD03 = 0x%02x\n", GetRegister(0x03));
    fprintf(hFile, "AD04 = 0x%02x\n", GetRegister(0x04));
    fprintf(hFile, "AD07 = 0x%02x\n", GetRegister(0x07));
    fprintf(hFile, "AD08 = 0x%02x\n", GetRegister(0x08));
    fprintf(hFile, "AD09 = 0x%02x\n", GetRegister(0x09));
    fprintf(hFile, "AD0a = 0x%02x\n", GetRegister(0x0a));
    fprintf(hFile, "AD0b = 0x%02x\n", GetRegister(0x0b));
    fprintf(hFile, "AD0c = 0x%02x\n", GetRegister(0x0c));
    fprintf(hFile, "AD0d = 0x%02x\n", GetRegister(0x0d));
    fprintf(hFile, "AD0e = 0x%02x\n", GetRegister(0x0e));
    fprintf(hFile, "AD0f = 0x%02x\n", GetRegister(0x0f));
    fprintf(hFile, "AD10 = 0x%02x\n", GetRegister(0x10));
    fprintf(hFile, "AD11 = 0x%02x\n", GetRegister(0x11));
    fprintf(hFile, "AD12 = 0x%02x\n", GetRegister(0x12));
    fprintf(hFile, "AD13 = 0x%02x\n", GetRegister(0x13));
    fprintf(hFile, "AD14 = 0x%02x\n", GetRegister(0x14));
    fprintf(hFile, "AD15 = 0x%02x\n", GetRegister(0x15));
    fprintf(hFile, "AD16 = 0x%02x\n", GetRegister(0x16));

    fclose(hFile);
}

BYTE CAD9882::GetDefaultAddress()const
{
    return 0x98>>1;
}

void CAD9882::Suspend()
{
    // powerdown AD9882
    SetRegister(0x14, 0xe0);
}

void CAD9882::Wakeup()
{
    // remove powerdown
    SetRegister(0x14, 0xe2);
}

// calculate PLL values given input resolution and veritcal frequency
void CAD9882::calcAD9882PLL(int htotal, int vtotal, int vf, int ilace,
        int &vco_range, int &cpump)
{
    double ipump;
    int hf, pixelfreq;
    int vco_gain, postdiv;
    const int ct_inv = 12195122;

    hf = vf * vtotal;
    if (ilace)
    {
        hf /= 2;
    }
    pixelfreq = hf * htotal;
    vco_gain = (pixelfreq < 140000000 ? 150 : 180);
    if (pixelfreq < 42000000)
    {
        postdiv = 4;
        vco_range = 0;
    }
    else if (pixelfreq < 84000000)
    {
        postdiv = 2;
        vco_range = 1;
    }
    else if (pixelfreq < 140000000)
    {
        postdiv = 1;
        vco_range = 2;
    }
    else
    {
        postdiv = 1;
        vco_range = 3;
    }
    ipump = (hf * 6.28) / 15.5;
    ipump *= ipump;
    ipump /= ct_inv;
    ipump *= (htotal * postdiv);
    ipump /= vco_gain;
    if (ipump < 75)
    {
        cpump = 0;
    }
    else if (ipump < 125)
    {
        cpump = 1;
    }
    else if (ipump < 200)
    {
        cpump = 2;
    }
    else if (ipump < 300)
    {
        cpump = 3;
    }
    else if (ipump < 425)
    {
        cpump = 4;
    }
    else if (ipump < 625)
    {
        cpump = 5;
    }
    else if (ipump < 1125)
    {
        cpump = 6;
    }
    else
    {
        cpump = 7;
    }
}

void CAD9882::SetPLL(WORD Value)
{
    SetRegister(0x01, (unsigned char)((Value>>4) & 0xff));
    SetRegister(0x02, (unsigned char)((Value<<4) & 0xff));
}

void CAD9882::SetVCO(BYTE Value)
{
    BYTE tmp = GetRegister(0x03);
    SetRegister(0x03, (unsigned char)((Value << 6) | (tmp & 0x3f)));
}

void CAD9882::SetPump(BYTE Value)
{
    BYTE tmp = GetRegister(0x03);
    SetRegister(0x03, (unsigned char)((tmp & 0xc0) | (Value << 3)));
}

void CAD9882::SetSOGThresh(BYTE Value)
{
    BYTE tmp = GetRegister(0x0f);
    SetRegister(0x0f, (unsigned char)((Value << 3) | (tmp & 0x07)));
}

void CAD9882::SetSOG(BOOLEAN YesNo)
{
    BYTE tmp = GetRegister(0x10);
    if (YesNo)
    {
        tmp |= 0x18;
    }
    else
    {
        tmp &= 0xe7;
    }
    SetRegister(0x10, tmp);
}

void CAD9882::SetPhase(BYTE Value)
{
    SetRegister(0x04, (Value & 0x1f) << 3);
}

void CAD9882::SetPreCoast(BYTE Value)
{
    SetRegister(0x12, Value);
}

void CAD9882::SetPostCoast(BYTE Value)
{
    SetRegister(0x13, Value);
}

void CAD9882::SetHSync(BYTE Value)
{
    SetRegister(0x07, Value);
}

void CAD9882::SetSyncSep(BYTE Value)
{
    SetRegister(0x0e, Value);
}

void CAD9882::SetCoastSel(BOOLEAN IntExt)
{
    BYTE tmp = GetRegister(0x11);
    if (IntExt)
    {
        tmp |= 0x08;
    }
    else
    {
        tmp &= ~0x08;
    }
    SetRegister(0x11, tmp);
}

void CAD9882::SetCoastOvr(BOOLEAN ManAuto)
{
    BYTE tmp = GetRegister(0x11);
    if (ManAuto)
    {
        tmp |= 0x04;
    }
    else
    {
        tmp &= ~0x04;
    }
    SetRegister(0x11, tmp);
}

void CAD9882::SetCoastPol(BOOLEAN HighLow)
{
    BYTE tmp = GetRegister(0x11);
    if (HighLow)
    {
        tmp |= 0x02;
    }
    else
    {
        tmp &= ~0x02;
    }
    SetRegister(0x11, tmp);
}

