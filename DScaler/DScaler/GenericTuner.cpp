//
// $Id: GenericTuner.cpp,v 1.6 2002-01-16 19:16:20 adcockj Exp $
//
/////////////////////////////////////////////////////////////////////////////
//
// copyleft 2001 itt@myself.com
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: not supported by cvs2svn $
// Revision 1.5  2001/12/18 23:36:01  adcockj
// Split up the MSP chip support into two parts to avoid probelms when deleting objects
//
// Revision 1.4  2001/12/05 21:45:11  ittarnavsky
// added changes for the AudioDecoder and AudioControls support
//
// Revision 1.3  2001/11/29 17:30:52  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.2  2001/11/26 13:02:27  adcockj
// Bug Fixes and standards changes
//
// Revision 1.1  2001/11/25 02:03:21  ittarnavsky
// initial checkin of the new I2C code
//
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GenericTuner.h"

/// \todo not at all OO
#define TUNERDEF(TID,VFMT,T1,T2,VHFL,VHFH,UHF,CFG,IFPC) \
        m_TunerId = (TID); \
        m_VideoFormat = (VFMT); \
        m_Thresh1 = (T1); \
        m_Thresh2 = (T2); \
        m_VHF_L = (VHFL); \
        m_VHF_H = (VHFH); \
        m_UHF = (UHF); \
        m_Config = (CFG); \
        m_IFPCoff = (IFPC)

CGenericTuner::CGenericTuner(eTunerId tunerId)
{
    switch (tunerId)
    {
    default:
        {
            TUNERDEF(TUNER_ABSENT, VIDEOFORMAT_NTSC_M,
                0,0,0,0,0,0,0);
            break;
        }
    case TUNER_PHILIPS_PAL_I:
        { 
            TUNERDEF(TUNER_PHILIPS_PAL_I, VIDEOFORMAT_PAL_I, 
                16*140.25, 16*463.25, 0xa0, 0x90, 0x30, 0x8e, 623);
            break;
        }
    case TUNER_PHILIPS_NTSC:
        { 
            TUNERDEF(TUNER_PHILIPS_NTSC, VIDEOFORMAT_NTSC_M, 
                16*157.25, 16*451.25, 0xA0, 0x90, 0x30, 0x8e, 732);
            break;
        }
    case TUNER_PHILIPS_SECAM:
        { 
            TUNERDEF(TUNER_PHILIPS_SECAM, VIDEOFORMAT_SECAM_D, 
                16*168.25, 16*447.25, 0xA7, 0x97, 0x37, 0x8e, 623);
            break;
        }
    case TUNER_PHILIPS_PAL:
        { 
            TUNERDEF(TUNER_PHILIPS_PAL, VIDEOFORMAT_PAL_B, 
                16*168.25, 16*447.25, 0xA0, 0x90, 0x30, 0x8e, 623);
            break;
        }
    case TUNER_TEMIC_4002FH5_PAL:
        { 
            TUNERDEF(TUNER_TEMIC_4002FH5_PAL, VIDEOFORMAT_PAL_B,
                16*140.25, 16*463.25, 0x02, 0x04, 0x01, 0x8e, 623);
            break;
        }
    case TUNER_TEMIC_4032FY5_NTSC:
        {
            TUNERDEF(TUNER_TEMIC_4032FY5_NTSC, VIDEOFORMAT_NTSC_M, 
                16*157.25, 16*463.25, 0x02, 0x04, 0x01, 0x8e, 732);
            break;
        }
    case TUNER_TEMIC_4062FY5_PAL_I:
        {
            TUNERDEF(TUNER_TEMIC_4062FY5_PAL_I, VIDEOFORMAT_PAL_I, 
                16*170.00, 16*450.00, 0x02, 0x04, 0x01, 0x8e, 623);
            break;
        }
    case TUNER_TEMIC_4036FY5_NTSC:
        {
            TUNERDEF(TUNER_TEMIC_4036FY5_NTSC, VIDEOFORMAT_NTSC_M, 
                16*157.25, 16*463.25, 0xa0, 0x90, 0x30, 0x8e, 732);
            break;
        }
    case TUNER_ALPS_TSBH1_NTSC:
        {
            TUNERDEF(TUNER_ALPS_TSBH1_NTSC, VIDEOFORMAT_NTSC_M, 
                16*137.25, 16*385.25, 0x01, 0x02, 0x08, 0x8e, 732);
            break;
        }
    case TUNER_ALPS_TSBE1_PAL:
        {
            TUNERDEF(TUNER_ALPS_TSBE1_PAL, VIDEOFORMAT_PAL_B, 
                16*137.25, 16*385.25, 0x01, 0x02, 0x08, 0x8e, 732);
            break;
        }
    case TUNER_ALPS_TSBB5_PAL_I:
        {
            TUNERDEF(TUNER_ALPS_TSBB5_PAL_I, VIDEOFORMAT_PAL_I, 
                16*133.25, 16*351.25, 0x01, 0x02, 0x08, 0x8e, 632);
            break;
        }
    case TUNER_ALPS_TSBE5_PAL:
        {
            TUNERDEF(TUNER_ALPS_TSBE5_PAL, VIDEOFORMAT_PAL_B, 
                16*133.25, 16*351.25, 0x01, 0x02, 0x08, 0x8e, 622);
            break;
        }
    case TUNER_ALPS_TSBC5_PAL:
        {
            TUNERDEF(TUNER_ALPS_TSBC5_PAL, VIDEOFORMAT_PAL_B, 
                16*133.25, 16*351.25, 0x01, 0x02, 0x08, 0x8e, 608);
            break;
        }
    case TUNER_TEMIC_4006FH5_PAL:
        {
            TUNERDEF(TUNER_TEMIC_4006FH5_PAL, VIDEOFORMAT_PAL_B, 
                16*170.00,16*450.00, 0xa0, 0x90, 0x30, 0x8e, 623);
            break;
        }
    case TUNER_PHILIPS_1236D_NTSC_INPUT1:
        {
            TUNERDEF(TUNER_PHILIPS_1236D_NTSC_INPUT1, VIDEOFORMAT_NTSC_M, 
                2516, 7220, 0xA3, 0x93, 0x33, 0xCE, 732);
            break;
        }
    case TUNER_PHILIPS_1236D_NTSC_INPUT2:
        {
            TUNERDEF(TUNER_PHILIPS_1236D_NTSC_INPUT2, VIDEOFORMAT_NTSC_M, 
                2516, 7220, 0xA2, 0x92, 0x32, 0xCE, 732);
            break;
        }
    case TUNER_ALPS_TSCH6_NTSC:
        {
            TUNERDEF(TUNER_ALPS_TSCH6_NTSC, VIDEOFORMAT_NTSC_M,
                16*137.25, 16*385.25, 0x14, 0x12, 0x11, 0x8e, 732);
            break;
        }
    case TUNER_TEMIC_4016FY5_PAL:
        {
            TUNERDEF(TUNER_TEMIC_4016FY5_PAL, VIDEOFORMAT_PAL_B,
                16*136.25, 16*456.25, 0xa0, 0x90, 0x30, 0x8e, 623);
            break;
        }
    case TUNER_PHILIPS_MK2_NTSC:
        {
            TUNERDEF(TUNER_PHILIPS_MK2_NTSC, VIDEOFORMAT_NTSC_M,
                16*160.00,16*454.00,0xa0,0x90,0x30,0x8e,732);
            break;
        }
    case TUNER_TEMIC_4066FY5_PAL_I:
        {
            TUNERDEF(TUNER_TEMIC_4066FY5_PAL_I, VIDEOFORMAT_PAL_I,
                16*169.00, 16*454.00, 0xa0,0x90,0x30,0x8e,623);
            break;
        }
    case TUNER_TEMIC_4006FN5_PAL:
        {
            TUNERDEF(TUNER_TEMIC_4006FN5_PAL, VIDEOFORMAT_PAL_B,
                16*169.00, 16*454.00, 0xa0,0x90,0x30,0x8e,623);
            break;
        }
    case TUNER_TEMIC_4009FR5_PAL:
        { 
            TUNERDEF(TUNER_TEMIC_4009FR5_PAL, VIDEOFORMAT_PAL_B,
                16*141.00, 16*464.00, 0xa0,0x90,0x30,0x8e,623);
            break;
        }
    case TUNER_TEMIC_4039FR5_NTSC:
        {
            TUNERDEF(TUNER_TEMIC_4039FR5_NTSC, VIDEOFORMAT_NTSC_M,
                16*158.00, 16*453.00, 0xa0,0x90,0x30,0x8e,732);
            break;
        }
    case TUNER_TEMIC_4046FM5_MULTI:
        { 
            TUNERDEF(TUNER_TEMIC_4046FM5_MULTI, VIDEOFORMAT_PAL_B,
                16*169.00, 16*454.00, 0xa0,0x90,0x30,0x8e,623);
            break;
        }
    case TUNER_PHILIPS_PAL_DK:
        { 
            TUNERDEF(TUNER_PHILIPS_PAL_DK, VIDEOFORMAT_PAL_D,
                16*170.00, 16*450.00, 0xa0, 0x90, 0x30, 0x8e, 623);
            break;
        }
    case TUNER_PHILIPS_MULTI:
        { 
            TUNERDEF(TUNER_PHILIPS_MULTI, VIDEOFORMAT_PAL_B,
                16*170.00,16*450.00,0xa0,0x90,0x30,0x8e,623);
            break;
        }
    case TUNER_LG_I001D_PAL_I:
        { 
            TUNERDEF(TUNER_LG_I001D_PAL_I, VIDEOFORMAT_PAL_I,
                16*170.00,16*450.00,0xa0,0x90,0x30,0x8e,623);
            break;
        }
    case TUNER_LG_I701D_PAL_I:
        { 
            TUNERDEF(TUNER_LG_I701D_PAL_I, VIDEOFORMAT_PAL_I,
                16*170.00,16*450.00,0xa0,0x90,0x30,0x8e,623);
            break;
        }
    case TUNER_LG_R01F_NTSC:
        { 
            TUNERDEF(TUNER_LG_R01F_NTSC, VIDEOFORMAT_NTSC_M,
                16*210.00,16*497.00,0xa0,0x90,0x30,0x8e,732);
            break;
        }
    case TUNER_LG_B01D_PAL:
        { 
            TUNERDEF(TUNER_LG_B01D_PAL, VIDEOFORMAT_PAL_B,
                16*170.00,16*450.00,0xa0,0x90,0x30,0x8e,623);
            break;
        }
    case TUNER_LG_B11D_PAL:
        { 
            TUNERDEF(TUNER_LG_B11D_PAL, VIDEOFORMAT_PAL_B,
                16*170.00,16*450.00,0xa0,0x90,0x30,0x8e,623);
            break;
        }
    case TUNER_TEMIC_4009FN5_PAL:
        { 
            TUNERDEF(TUNER_TEMIC_4009FN5_PAL, VIDEOFORMAT_PAL_B,
                16*141.00, 16*464.00, 0xa0,0x90,0x30,0x8e,623);
            break;
        }
    case TUNER_SHARP_2U5JF5540_NTSC:
        {
            TUNERDEF(TUNER_SHARP_2U5JF5540_NTSC, VIDEOFORMAT_NTSC_M_Japan,
                16*137.25, 16*317.25, 0x01, 0x02, 0x08, 0x8e, 940);
            break;
        }
    case TUNER_LG_TAPCH701P_NTSC:
        {
            TUNERDEF(TUNER_LG_TAPCH701P_NTSC, VIDEOFORMAT_NTSC_M, 
                16*165.00, 16*450.00, 0x01, 0x02, 0x08, 0x8e, 732);
            break;
        }

    }
}

bool CGenericTuner::SetTVFrequency(long nFrequency, eVideoFormat videoFormat)
{
    BYTE config;
    WORD div;

    if (nFrequency < m_Thresh1)
    {
        config = m_VHF_L;
    }
    else if (nFrequency < m_Thresh2)
    {
        config = m_VHF_H;
    }
    else
    {
        config = m_UHF;
    }
    
    div = nFrequency + m_IFPCoff;
    
    // handle Mode on Philips SECAM tuners
    // they can also recive PAL if the Mode is set properly
    if (m_TunerId == TUNER_PHILIPS_SECAM)
    {
        if (IsSECAMVideoFormat(videoFormat))
        {
            config |= 0x02;
        }
        else
        {
            config &= ~0x02;
        }
    }
    
    div &= 0x7fff;

    BYTE buffer[] = {(BYTE) m_DeviceAddress << 1, (BYTE) ((div >> 8) & 0x7f), (BYTE) (div & 0xff), m_Config, config};

    bool result = m_I2CBus->Write(buffer, sizeof(buffer));
    return result;
}

eTunerId CGenericTuner::GetTunerId()
{
    return m_TunerId;
}

eVideoFormat CGenericTuner::GetDefaultVideoFormat()
{
    return m_VideoFormat;
}

bool CGenericTuner::HasRadio() const
{
    return false;
}

bool CGenericTuner::SetRadioFrequency(long nFrequency)
{
    return true;
}

BYTE CGenericTuner::GetDefaultAddress()const
{
    return 0xC0>>1;
}
