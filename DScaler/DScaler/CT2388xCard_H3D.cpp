/////////////////////////////////////////////////////////////////////////////
// $Id: CT2388xCard_H3D.cpp,v 1.6 2002-09-26 16:32:33 adcockj Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.5  2002/09/22 17:47:04  adcockj
// Fixes for holo3d
//
// Revision 1.4  2002/09/19 22:10:08  adcockj
// Holo3D Fixes for PAL
//
// Revision 1.3  2002/09/16 19:34:18  adcockj
// Fix for auto format change
//
// Revision 1.2  2002/09/11 19:33:06  adcockj
// a few tidy ups
//
// Revision 1.1  2002/09/11 18:19:37  adcockj
// Prelimainary support for CT2388x based cards
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "CT2388xCard.h"
#include "CT2388x_Defines.h"
#include "DebugLog.h"

enum eHolo3DInputs
{
    H3D_COMPONENT,
    H3D_RGsB,
    H3D_SVIDEO,
    H3D_SDI,
    H3D_COMPOSITE1,
    H3D_COMPOSITE2,
    H3D_COMPOSITE3,
    H3D_COMPOSITE4,
};

void CCT2388xCard::InitH3D()
{
    m_SAA7118 = new CSAA7118();

    m_SAA7118->Attach(m_I2CBus);

    if(m_SAA7118->GetVersion() > 0)
    {
        m_SAA7118->DumpSettings();
    }

    FILE* hFile;

    hFile = fopen("FPGA.txt", "w");
    if(!hFile)
    {
        return;
    }

    for(DWORD i(0x390000); i <= 0x39000F; ++i)
    {
        fprintf(hFile, "%06x\t%02x\n", i, ReadByte(i));
    }

    fclose(hFile);

    m_SAA7118->SetRegister(0x01, 0x47);
    m_SAA7118->SetRegister(0x03, 0x10);
    m_SAA7118->SetRegister(0x04, 0x90);
    m_SAA7118->SetRegister(0x05, 0x90);
    m_SAA7118->SetRegister(0x06, 0xeb);
    m_SAA7118->SetRegister(0x07, 0xe0);
    m_SAA7118->SetRegister(0x08, 0x98);
    m_SAA7118->SetRegister(0x11, 0x00);
    m_SAA7118->SetRegister(0x12, 0x00);
    m_SAA7118->SetRegister(0x13, 0x00);
    m_SAA7118->SetRegister(0x14, 0x00);
    m_SAA7118->SetRegister(0x15, 0x11);
    m_SAA7118->SetRegister(0x16, 0xfe);
    m_SAA7118->SetRegister(0x17, 0xc0);
    m_SAA7118->SetRegister(0x18, 0x40);
    m_SAA7118->SetRegister(0x19, 0x80);

    m_SAA7118->SetRegister(0x23, 0x00);
    m_SAA7118->SetRegister(0x24, 0x90);
    m_SAA7118->SetRegister(0x25, 0x90);
    m_SAA7118->SetRegister(0x29, 0x00);
    m_SAA7118->SetRegister(0x2D, 0x00);
    m_SAA7118->SetRegister(0x2E, 0x00);
    m_SAA7118->SetRegister(0x2F, 0x00);

    m_SAA7118->SetRegister(0x40, 0x40);
    for(BYTE j(0x41); j <= 0x57; ++j)
    {
        m_SAA7118->SetRegister(j, 0xFF);
    }
    m_SAA7118->SetRegister(0x58, 0x00);
    m_SAA7118->SetRegister(0x59, 0x47);
    m_SAA7118->SetRegister(0x5C, 0x00);
    m_SAA7118->SetRegister(0x5D, 0x3E);
    m_SAA7118->SetRegister(0x5E, 0x00);
    m_SAA7118->SetRegister(0x5F, 0x00);

}

void CCT2388xCard::H3DInputSelect(int nInput)
{
    StandardInputSelect(nInput);

    switch(nInput)
    {
    case H3D_COMPONENT:
        m_SAA7118->SetRegister(0x02, 0xef);
        WriteByte(0x390002, 0x04);
        break;
    case H3D_RGsB:
        m_SAA7118->SetRegister(0x02, 0xff);
        WriteByte(0x390002, 0x04);
        break;
    case H3D_SVIDEO:
        m_SAA7118->SetRegister(0x02, 0xc8);
        WriteByte(0x390002, 0x04);
        break;
    case H3D_SDI:
        WriteByte(0x390002, 0x14);
        break;
    case H3D_COMPOSITE1:
        m_SAA7118->SetRegister(0x02, 0xc5);
        WriteByte(0x390002, 0x04);
        break;
    case H3D_COMPOSITE2:
        m_SAA7118->SetRegister(0x02, 0xdf);
        WriteByte(0x390002, 0x04);
        break;
    case H3D_COMPOSITE3:
        m_SAA7118->SetRegister(0x02, 0xd5);
        WriteByte(0x390002, 0x04);
        break;
    case H3D_COMPOSITE4:
        m_SAA7118->SetRegister(0x02, 0xd0);
        WriteByte(0x390002, 0x04);
        break;
    }
}

void CCT2388xCard::H3DSetFormat(int nInput, eVideoFormat TVFormat, BOOL IsProgressive)
{
    BYTE ChrominanceControl(0x81);
    BYTE ChrominanceControl2(0x00);
    BYTE ChrominanceGainControl(0x2A);
    BYTE LuminaceControl(0x40);

    if(GetTVFormat(TVFormat)->wCropHeight == 576)
    {
        m_SAA7118->SetRegister(0x5A, 0x03);
        m_SAA7118->SetRegister(0x5A, 0x03);
        WriteByte(0x390007, 0xd0);
    }
    else
    {
        m_SAA7118->SetRegister(0x5A, 0x06);
        m_SAA7118->SetRegister(0x5A, 0x83);
        WriteByte(0x390007, 0x50);
    }

    switch(nInput)
    {
    case H3D_COMPONENT:
    case H3D_RGsB:
    case H3D_SDI:
        // doesn't really matter for these inputs
        // but we'll set the default
        ChrominanceControl = 0x89;
	    m_SAA7118->SetRegister(0x29, 0x40);
        break;
    case H3D_SVIDEO:
    case H3D_COMPOSITE1:
    case H3D_COMPOSITE2:
    case H3D_COMPOSITE3:
    case H3D_COMPOSITE4:
	    m_SAA7118->SetRegister(0x29, 0x00);
        switch(TVFormat)
        {
        case VIDEOFORMAT_PAL_M:
            ChrominanceControl |= 3 << 4;
            ChrominanceControl2 = 0x06;
            break;
        case VIDEOFORMAT_PAL_60:
            ChrominanceControl |= 1 << 4;
            ChrominanceControl2 = 0x06;
            break;
        case VIDEOFORMAT_NTSC_50:
            ChrominanceControl |= 1 << 4;
            ChrominanceControl |= 1 << 3;
            ChrominanceControl2 = 0x0E;
            break;
        case VIDEOFORMAT_PAL_N_COMBO:
            ChrominanceControl |= 2 << 4;
            ChrominanceControl2 = 0x06;
            break;
        case VIDEOFORMAT_SECAM_B:
        case VIDEOFORMAT_SECAM_D:
        case VIDEOFORMAT_SECAM_G:
        case VIDEOFORMAT_SECAM_H:
        case VIDEOFORMAT_SECAM_K:
        case VIDEOFORMAT_SECAM_K1:
        case VIDEOFORMAT_SECAM_L:
        case VIDEOFORMAT_SECAM_L1:
            ChrominanceControl = 0xD0;
            ChrominanceControl2 = 0x00;
            ChrominanceGainControl = 0x80;
            LuminaceControl = 0x1B;
            break;
        case VIDEOFORMAT_NTSC_M_Japan:
            ChrominanceControl |= 4 << 4;
            ChrominanceControl |= 1 << 3;
            ChrominanceControl2 = 0x0E;
            break;
        case VIDEOFORMAT_PAL_B:
        case VIDEOFORMAT_PAL_D:
        case VIDEOFORMAT_PAL_G:
        case VIDEOFORMAT_PAL_H:
        case VIDEOFORMAT_PAL_I:
            ChrominanceControl |= 0 << 4;
            ChrominanceControl2 = 0x06;
            break;
        case VIDEOFORMAT_NTSC_M:
        default:
            ChrominanceControl |= 0 << 4;
            ChrominanceControl |= 1 << 3;
            ChrominanceControl2 = 0x0E;
            break;
        }
        break;
    }

    if(nInput == H3D_SVIDEO)
    {
        LuminaceControl |= 0x80;
    }
    
    m_SAA7118->SetRegister(0x09, LuminaceControl);
    m_SAA7118->SetRegister(0x0E, ChrominanceControl);
    m_SAA7118->SetRegister(0x0F, ChrominanceGainControl);
    m_SAA7118->SetRegister(0x10, ChrominanceControl2);

    if(IsProgressive)
    {
        WriteByte(0x390005, 0x4a);
        WriteByte(0x390008, 0x02);
        WriteByte(0x39000e, 0x10);
        WriteByte(0x39000f, 0x01);
    }
    else
    {
        WriteByte(0x390005, 0x0a);
        WriteByte(0x390008, 0x03);
        WriteByte(0x39000e, 0x00);
        WriteByte(0x39000f, 0x02);
    }
}


void CCT2388xCard::SetH3DBrightness(BYTE Brightness)
{
    switch(m_CurrentInput)
    {
    case H3D_COMPONENT:
    case H3D_RGsB:
        m_SAA7118->SetComponentBrightness(Brightness);
        SetVIPBrightness(0x80);
        break;
    case H3D_SVIDEO:
    case H3D_COMPOSITE1:
    case H3D_COMPOSITE2:
    case H3D_COMPOSITE3:
    case H3D_COMPOSITE4:
        m_SAA7118->SetBrightness(Brightness);
        SetVIPBrightness(0x80);
        break;
    case H3D_SDI:
    default:
        SetVIPBrightness(Brightness);
        break;
    }
}

void CCT2388xCard::SetH3DHue(BYTE Hue)
{
    switch(m_CurrentInput)
    {
    case H3D_COMPONENT:
    case H3D_RGsB:
        break;
    case H3D_SVIDEO:
    case H3D_COMPOSITE1:
    case H3D_COMPOSITE2:
    case H3D_COMPOSITE3:
    case H3D_COMPOSITE4:
        m_SAA7118->SetHue(Hue);
        break;
    case H3D_SDI:
    default:
        break;
    }
}

void CCT2388xCard::SetH3DContrast(BYTE Contrast)
{
    switch(m_CurrentInput)
    {
    case H3D_COMPONENT:
    case H3D_RGsB:
        m_SAA7118->SetComponentContrast(Contrast);
        SetVIPContrast(0x80);
        break;
    case H3D_SVIDEO:
    case H3D_COMPOSITE1:
    case H3D_COMPOSITE2:
    case H3D_COMPOSITE3:
    case H3D_COMPOSITE4:
        m_SAA7118->SetContrast(Contrast);
        SetVIPContrast(0x80);
        break;
    case H3D_SDI:
    default:
        SetVIPContrast(Contrast);
        break;
    }
}

void CCT2388xCard::SetH3DSaturationU(BYTE Saturation)
{
    switch(m_CurrentInput)
    {
    case H3D_COMPONENT:
    case H3D_RGsB:
        m_SAA7118->SetComponentSaturation(Saturation);
        SetVIPSaturation(0x80);
        break;
    case H3D_SVIDEO:
    case H3D_COMPOSITE1:
    case H3D_COMPOSITE2:
    case H3D_COMPOSITE3:
    case H3D_COMPOSITE4:
        m_SAA7118->SetSaturation(Saturation);
        SetVIPSaturation(0x80);
        break;
    case H3D_SDI:
    default:
        SetVIPSaturation(Saturation);
        break;
    }
}

void CCT2388xCard::SetH3DSaturationV(BYTE NotUsed)
{
    // we'll do nothing with this but it needs to be here
}
