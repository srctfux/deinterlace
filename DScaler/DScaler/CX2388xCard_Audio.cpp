/////////////////////////////////////////////////////////////////////////////
// $Id$
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
//
// This code is based on a version of dTV modified by Michael Eskin and
// others at Connexant.  Those parts are probably (c) Connexant 2002
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file CX2388xCard.cpp CCX2388xCard Implementation (Audio)
 */

#include "stdafx.h"

#ifdef WANT_CX2388X_SUPPORT

#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "CX2388xCard.h"
#include "CX2388x_Defines.h"
#include "TVFormats.h"

const CCX2388xCard::TAudioRegList CCX2388xCard::m_RegList_BTSC[]=
{
    {AUD_INIT,                    SEL_BTSC},
    {AUD_INIT_LD,                0x00000001},
    {AUD_SOFT_RESET,            0x00000001},
    {AUD_AFE_12DB_EN,            0x00000001},
    {AUD_OUT1_SEL,                0x00000013},
    {AUD_OUT1_SHIFT,            0x00000000},
    {AUD_POLY0_DDS_CONSTANT,    0x0012010c},
    {AUD_DMD_RA_DDS,            0x00c3e7aa},
    {AUD_DBX_IN_GAIN,            0x00004734},
    {AUD_DBX_WBE_GAIN,            0x00004640},
    {AUD_DBX_SE_GAIN,            0x00008d31},
    {AUD_DCOC_0_SRC,            0x0000001a},
    {AUD_IIR1_4_SEL,            0x00000021},
    {AUD_DCOC_PASS_IN,            0x00000003},
    {AUD_DCOC_0_SHIFT_IN0,        0x0000000a},
    {AUD_DCOC_0_SHIFT_IN1,        0x00000008},
    {AUD_DCOC_1_SHIFT_IN0,        0x0000000a},
    {AUD_DCOC_1_SHIFT_IN1,        0x00000008},
    {AUD_DN0_FREQ,                0x0000283b},
    {AUD_DN2_SRC_SEL,            0x00000008},
    {AUD_DN2_FREQ,                0x00003000},
    {AUD_DN2_AFC,                0x00000002},
    {AUD_DN2_SHFT,                0x00000000},
    {AUD_IIR2_2_SEL,            0x00000020},
    {AUD_IIR2_2_SHIFT,            0x00000000},
    {AUD_IIR2_3_SEL,            0x0000001f},
    {AUD_IIR2_3_SHIFT,            0x00000000},
    {AUD_CRDC1_SRC_SEL,            0x000003ce},
    {AUD_CRDC1_SHIFT,            0x00000000},
    {AUD_CORDIC_SHIFT_1,        0x00000007},
    {AUD_DCOC_1_SRC,            0x0000001b},
    {AUD_DCOC1_SHIFT,            0x00000000},
    {AUD_RDSI_SEL,                0x00000008},
    {AUD_RDSQ_SEL,                0x00000008},
    {AUD_RDSI_SHIFT,            0x00000000},
    {AUD_RDSQ_SHIFT,            0x00000000},
    {AUD_POLYPH80SCALEFAC,        0x00000003},
    {0, 0},
};

const CCX2388xCard::TAudioRegList CCX2388xCard::m_RegList_BTSC_SAP[]=
{
    {AUD_INIT,                    SEL_SAP},
    {AUD_INIT_LD,                0x00000001},
    {AUD_SOFT_RESET,            0x00000001},
    {AUD_AFE_12DB_EN,            0x00000001},
    {AUD_DBX_IN_GAIN,            0x00007200},
    {AUD_DBX_WBE_GAIN,            0x00006200},
    {AUD_DBX_SE_GAIN,            0x00006200},
    {AUD_IIR1_1_SEL,            0x00000000},
    {AUD_IIR1_3_SEL,            0x00000001},
    {AUD_DN1_SRC_SEL,            0x00000007},
    {AUD_IIR1_4_SHIFT,            0x00000006},
    {AUD_IIR2_1_SHIFT,            0x00000000},
    {AUD_IIR2_2_SHIFT,            0x00000000},
    {AUD_IIR3_0_SHIFT,            0x00000000},
    {AUD_IIR3_1_SHIFT,            0x00000000},
    {AUD_IIR3_0_SEL,            0x0000000d},
    {AUD_IIR3_1_SEL,            0x0000000e},
    {AUD_DEEMPH1_SRC_SEL,        0x00000014},
    {AUD_DEEMPH1_SHIFT,            0x00000000},
    {AUD_DEEMPH1_G0,            0x00004000},
    {AUD_DEEMPH1_A0,            0x00000000},
    {AUD_DEEMPH1_B0,            0x00000000},
    {AUD_DEEMPH1_A1,            0x00000000},
    {AUD_DEEMPH1_B1,            0x00000000},
    {AUD_OUT0_SEL,                0x0000003f},
    {AUD_OUT1_SEL,                0x0000003f},
    {AUD_DN1_AFC,                0x00000002},
    {AUD_DCOC_0_SHIFT_IN0,        0x0000000a},
    {AUD_DCOC_0_SHIFT_IN1,        0x00000008},
    {AUD_DCOC_1_SHIFT_IN0,        0x0000000a},
    {AUD_DCOC_1_SHIFT_IN1,        0x00000008},
    {AUD_IIR1_0_SEL,            0x0000001d},
    {AUD_IIR1_2_SEL,            0x0000001e},
    {AUD_IIR2_1_SEL,            0x00000002},
    {AUD_IIR2_2_SEL,            0x00000004},
    {AUD_IIR3_2_SEL,            0x0000000f},
    {AUD_DCOC2_SHIFT,            0x00000001},
    {AUD_IIR3_2_SHIFT,            0x00000001},
    {AUD_DEEMPH0_SRC_SEL,        0x00000014},
    {AUD_CORDIC_SHIFT_1,        0x00000006},
    {AUD_POLY0_DDS_CONSTANT,    0x000e4db2},
    {AUD_DMD_RA_DDS,            0x00f696e6},
    {AUD_IIR2_3_SEL,            0x00000025},
    {AUD_IIR1_4_SEL,            0x00000021},
    {AUD_DN1_FREQ,                0x0000c965},
    {AUD_DCOC_PASS_IN,            0x00000003},
    {AUD_DCOC_0_SRC,            0x0000001a},
    {AUD_DCOC_1_SRC,            0x0000001b},
    {AUD_DCOC1_SHIFT,            0x00000000},
    {AUD_RDSI_SEL,                0x00000009},
    {AUD_RDSQ_SEL,                0x00000009},
    {AUD_RDSI_SHIFT,            0x00000000},
    {AUD_RDSQ_SHIFT,            0x00000000},
    {AUD_POLYPH80SCALEFAC,        0x00000003},
    {0, 0},
};

const CCX2388xCard::TAudioRegList CCX2388xCard::m_RegList_EIAJ[]=
{
    //\todo handle StereoType

    // increase level of input by 12dB
    {AUD_AFE_12DB_EN,          0x0001},

    // EIAJ stereo -> RAM -> DAC standard setup
    // xtal = 28.636 MHz

    // initialize EIAJ
    {AUD_INIT,                 SEL_EIAJ},
    {AUD_INIT_LD,              0x0001},
    {AUD_SOFT_RESET,           0x0001},

    {AUD_CTL,                  EN_DAC_ENABLE | EN_DMTRX_SUMDIFF | EN_EIAJ_AUTO_STEREO},

    // fix pilot detection
    {AUD_HP_PROG_IIR4_1,       0x0019},
    {AUD_IIR4_0_CA0,           0x000392ad},
    {AUD_IIR4_0_CA1,           0x00007543},
    {AUD_IIR4_0_CA2,           0x00030dc3},
    {AUD_IIR4_0_CB0,           0x00006439},
    {AUD_IIR4_0_CB1,           0x00031e98},
    {AUD_IIR4_1_CA0,           0x00004137},
    {AUD_IIR4_1_CA1,           0x0000692d},
    {AUD_IIR4_1_CA2,           0x0000bca5},
    {AUD_IIR4_1_CB0,           0x00003bd4},
    {AUD_IIR4_1_CB1,           0x000097f3},
    {AUD_IIR4_2_CA0,           0x00004137},
    {AUD_IIR4_2_CA1,           0x0000692d},
    {AUD_IIR4_2_CA2,           0x0000bca5},
    {AUD_IIR4_2_CB0,           0x00003bd4},
    {AUD_IIR4_2_CB1,           0x000097f3},
    {AUD_DN2_FREQ,             0x00003d71},

    // turn down gain to iir4's...
    {AUD_IIR4_0_SHIFT,         0x0007},
    {AUD_IIR4_1_SHIFT,         0x0000},
    {AUD_IIR4_2_SHIFT,         0x0006},

    // fast-in, fast-out, stereo & dual
    {AUD_THR_FR,               0x0000},
    {AUD_PILOT_BQD_1_K0,       0x00008000},
    {AUD_PILOT_BQD_1_K1,       0x00000000},
    {AUD_PILOT_BQD_1_K2,       0x00000000},
    {AUD_PILOT_BQD_1_K3,       0x00000000},
    {AUD_PILOT_BQD_1_K4,       0x00000000},
    {AUD_PILOT_BQD_2_K0,       0x00c00000},
    {AUD_PILOT_BQD_2_K1,       0x00000000},
    {AUD_PILOT_BQD_2_K2,       0x00000000},
    {AUD_PILOT_BQD_2_K3,       0x00000000},
    {AUD_PILOT_BQD_2_K4,       0x00000000},
    {AUD_C2_UP_THR,            0x2600},
    {AUD_C2_LO_THR,            0x0c00},
    {AUD_C1_UP_THR,            0x3400},
    {AUD_C1_LO_THR,            0x2c00},
    {AUD_MODE_CHG_TIMER,       0x0050},
    {AUD_START_TIMER,          0x0200},
    {AUD_AFE_12DB_EN,          0x0001},
    {AUD_CORDIC_SHIFT_0,       0x0006},

    // slow-in, fast-out stereo only
    //{AUD_THR_FR,               0x0000},
    //{AUD_PILOT_BQD_1_K0,       0x0000c000},
    //{AUD_PILOT_BQD_1_K1,       0x00000000},
    //{AUD_PILOT_BQD_1_K2,       0x00000000},
    //{AUD_PILOT_BQD_1_K3,       0x00000000},
    //{AUD_PILOT_BQD_1_K4,       0x00000000},
    //{AUD_PILOT_BQD_2_K0,       0x00c00000},
    //{AUD_C1_UP_THR,            0x0120},
    //{AUD_C1_LO_THR,            0x0114},
    //{AUD_C2_UP_THR,            0x0000},
    //{AUD_C2_LO_THR,            0x0000},
    //{AUD_MODE_CHG_TIMER,       0x0050},
    //{AUD_START_TIMER,          0x0200},
    //{AUD_AFE_16DB_EN,          0x0000},
    //{AUD_CORDIC_SHIFT_0,       0x0006},

    {AUD_RATE_ADJ1,            0x0100},
    {AUD_RATE_ADJ2,            0x0200},
    {AUD_RATE_ADJ3,            0x0300},
    {AUD_RATE_ADJ4,            0x0400},
    {AUD_RATE_ADJ5,            0x0500},
    {AUD_POLY0_DDS_CONSTANT,   0x121116},
    {AUD_DEEMPH0_SHIFT,        0x0000},
    {AUD_DEEMPH1_SHIFT,        0x0000},
    {AUD_DEEMPH0_G0,           0x0d9f}, // change 5/4/01
    {AUD_PHASE_FIX_CTL,        0x0009}, // change 5/4/01
    {AUD_CORDIC_SHIFT_1,       0x0006}, // change 5/4/01

    // Completely ditch AFC feedback
    {AUD_DCOC_0_SRC,           0x0021},
    {AUD_DCOC_1_SRC,           0x001a},
    {AUD_DCOC1_SHIFT,          0x0000},
    {AUD_DCOC_1_SHIFT_IN0,     0x000a},
    {AUD_DCOC_1_SHIFT_IN1,     0x0008},
    {AUD_DCOC_PASS_IN,         0x0000},
    {AUD_IIR1_4_SEL,           0x0023},

    // Completely ditch L-R AFC feedback
    {AUD_DN1_AFC,              0x0000},
    {AUD_DN1_FREQ,             0x4000},
    {AUD_DCOC_2_SRC,           0x001b},
    {AUD_DCOC2_SHIFT,          0x0000},
    {AUD_DCOC_2_SHIFT_IN0,     0x000a},
    {AUD_DCOC_2_SHIFT_IN1,     0x0008},
    {AUD_DEEMPH1_SRC_SEL,      0x0025},

    // setup Audio PLL
    //{AUD_PLL_PRESCALE,         0x0002},
    //{AUD_PLL_INT,              0x001f},

    // de-assert Audio soft reset
    {AUD_SOFT_RESET,           0x0000},
    {0, 0},
};

const CCX2388xCard::TAudioRegList CCX2388xCard::m_RegList_Nicam_Common[]=
{
    {AUD_INIT,                    SEL_NICAM},
    {AUD_INIT_LD,                0x00000001},
    {AUD_SOFT_RESET,            0x00000001},
    {AUD_AFE_12DB_EN,            0x00000001},
    {AUD_RATE_ADJ1,                0x00000010},
    {AUD_RATE_ADJ2,                0x00000040},
    {AUD_RATE_ADJ3,                0x00000100},
    {AUD_RATE_ADJ4,                0x00000400},
    {AUD_RATE_ADJ5,                0x00001000},
    {AUD_DEEMPHDENOM2_R,        0x00000000},
    {AUD_ERRLOGPERIOD_R,        0x00000fff},
    {AUD_ERRINTRPTTHSHLD1_R,    0x000003ff},
    {AUD_ERRINTRPTTHSHLD2_R,    0x000000ff},
    {AUD_ERRINTRPTTHSHLD3_R,    0x0000003f},
    {AUD_POLYPH80SCALEFAC,        0x00000003},
    {AUD_PDF_DDS_CNST_BYTE2,    0x06},
    {AUD_PDF_DDS_CNST_BYTE1,    0x82},
    {AUD_QAM_MODE,                0x05},
    {0, 0},
};

const CCX2388xCard::TAudioRegList CCX2388xCard::m_RegList_Nicam_Deemph1[]=
{
    {AUD_DEEMPHGAIN_R,            0x000023c2},
    {AUD_DEEMPHNUMER1_R,        0x0002a7bc},
    {AUD_DEEMPHNUMER2_R,        0x0003023e},
    {AUD_DEEMPHDENOM1_R,        0x0000f3d0},
    {AUD_DEEMPHDENOM2_R,        0x00000000},
    {0, 0},
};

const CCX2388xCard::TAudioRegList CCX2388xCard::m_RegList_Nicam_Deemph2[]=
{
    {AUD_DEEMPHGAIN_R,            0x0000c600},
    {AUD_DEEMPHNUMER1_R,        0x00066738},
    {AUD_DEEMPHNUMER2_R,        0x00066739},
    {AUD_DEEMPHDENOM1_R,        0x0001e88c},
    {AUD_DEEMPHDENOM2_R,        0x0001e88c},
    {0, 0},
};

const CCX2388xCard::TAudioRegList CCX2388xCard::m_RegList_AM_L[]=
{
    {AUD_INIT_LD,               0x00000001},
    {AUD_SOFT_RESET,            0x00000001},
    {AUD_ERRLOGPERIOD_R,        0x00000064},
    {AUD_ERRINTRPTTHSHLD1_R,    0x00000fff},
    {AUD_ERRINTRPTTHSHLD2_R,    0x0000001f},
    {AUD_ERRINTRPTTHSHLD3_R,    0x0000000f},
    {AUD_PDF_DDS_CNST_BYTE2,    0x06},
    {AUD_PDF_DDS_CNST_BYTE1,    0x82},
    {AUD_PDF_DDS_CNST_BYTE0,    0x12},
    {AUD_QAM_MODE,                0x05},
    {AUD_PHACC_FREQ_8MSB,        0x34},
    {AUD_PHACC_FREQ_8LSB,        0x4c},
    {AUD_DEEMPHGAIN_R,            0x00006680},
    {AUD_DEEMPHNUMER1_R,        0x000353de},
    {AUD_DEEMPHNUMER2_R,        0x000001b1},
    {AUD_DEEMPHDENOM1_R,        0x0000f3d0},
    {AUD_DEEMPHDENOM2_R,        0x00000000},
    {AUD_FM_MODE_ENABLE,        0x00000007},
    {AUD_POLYPH80SCALEFAC,        0x00000003},
    {AUD_AFE_12DB_EN,            0x00000001},
    {AAGC_GAIN,                    0x00000000},
    {AAGC_HYST,                    0x00000018},
    {AAGC_DEF,                    0x00000020},
    {AUD_DN0_FREQ,                0x00000000},
    {AUD_POLY0_DDS_CONSTANT,    0x000e4db2},
    {AUD_DCOC_0_SRC,            0x00000021},
    {AUD_IIR1_0_SEL,            0x00000000},
    {AUD_IIR1_0_SHIFT,            0x00000007},
    {AUD_IIR1_1_SEL,            0x00000002},
    {AUD_IIR1_1_SHIFT,            0x00000000},
    {AUD_DCOC_1_SRC,            0x00000003},
    {AUD_DCOC1_SHIFT,            0x00000000},
    {AUD_DCOC_PASS_IN,            0x00000000},
    {AUD_IIR1_2_SEL,            0x00000023},
    {AUD_IIR1_2_SHIFT,            0x00000000},
    {AUD_IIR1_3_SEL,            0x00000004},
    {AUD_IIR1_3_SHIFT,            0x00000007},
    {AUD_IIR1_4_SEL,            0x00000005},
    {AUD_IIR1_4_SHIFT,            0x00000007},
    {AUD_IIR3_0_SEL,            0x00000007},
    {AUD_IIR3_0_SHIFT,            0x00000000},
    {AUD_DEEMPH0_SRC_SEL,        0x00000011},
    {AUD_DEEMPH0_SHIFT,            0x00000000},
    {AUD_DEEMPH0_G0,            0x00007000},
    {AUD_DEEMPH0_A0,            0x00000000},
    {AUD_DEEMPH0_B0,            0x00000000},
    {AUD_DEEMPH0_A1,            0x00000000},
    {AUD_DEEMPH0_B1,            0x00000000},
    {AUD_DEEMPH1_SRC_SEL,        0x00000011},
    {AUD_DEEMPH1_SHIFT,            0x00000000},
    {AUD_DEEMPH1_G0,            0x00007000},
    {AUD_DEEMPH1_A0,            0x00000000},
    {AUD_DEEMPH1_B0,            0x00000000},
    {AUD_DEEMPH1_A1,            0x00000000},
    {AUD_DEEMPH1_B1,            0x00000000},
    {AUD_OUT0_SEL,                0x0000003f},
    {AUD_OUT1_SEL,                0x0000003f},
    {AUD_DMD_RA_DDS,            0x00f5c285},
    {AUD_PLL_INT,                0x0000001e},
    {AUD_PLL_DDS,                0x00000000},
    {AUD_PLL_FRAC,                0x0000e542},
    {0, 0},
};

const CCX2388xCard::TAudioRegList CCX2388xCard::m_RegList_A1_I[]=
{
    {AUD_INIT,                    SEL_A2},
    {AUD_INIT_LD,                0x00000001},
    {AUD_SOFT_RESET,            0x00000001},
    {AUD_ERRLOGPERIOD_R,        0x00000064},
    {AUD_ERRINTRPTTHSHLD1_R,    0x00000fff},
    {AUD_ERRINTRPTTHSHLD2_R,    0x0000001f},
    {AUD_ERRINTRPTTHSHLD3_R,    0x0000000f},
    {AUD_PDF_DDS_CNST_BYTE2,    0x06},
    {AUD_PDF_DDS_CNST_BYTE1,    0x82},
    {AUD_PDF_DDS_CNST_BYTE0,    0x12},
    {AUD_QAM_MODE,                0x05},
    {AUD_PHACC_FREQ_8MSB,        0x3a},
    {AUD_PHACC_FREQ_8LSB,        0x93},
    {AUD_DMD_RA_DDS,            0x002a4f2f},
    {AUD_PLL_INT,                0x0000001e},
    {AUD_PLL_DDS,                0x00000004},
    {AUD_PLL_FRAC,                0x0000e542},
    {AUD_RATE_ADJ1,                0x00000100},
    {AUD_RATE_ADJ2,                0x00000200},
    {AUD_RATE_ADJ3,                0x00000300},
    {AUD_RATE_ADJ4,                0x00000400},
    {AUD_RATE_ADJ5,                0x00000500},
    {AUD_THR_FR,                0x00000000},
    {AUD_PILOT_BQD_1_K0,        0x0000755b},
    {AUD_PILOT_BQD_1_K1,        0x00551340},
    {AUD_PILOT_BQD_1_K2,        0x006d30be},
    {AUD_PILOT_BQD_1_K3,        0xffd394af},
    {AUD_PILOT_BQD_1_K4,        0x00400000},
    {AUD_PILOT_BQD_2_K0,        0x00040000},
    {AUD_PILOT_BQD_2_K1,        0x002a4841},
    {AUD_PILOT_BQD_2_K2,        0x00400000},
    {AUD_PILOT_BQD_2_K3,        0x00000000},
    {AUD_PILOT_BQD_2_K4,        0x00000000},
    {AUD_MODE_CHG_TIMER,        0x00000060},
    {AUD_AFE_12DB_EN,            0x00000001},
    {AAGC_HYST,                    0x0000000a},
    {AUD_CORDIC_SHIFT_0,        0x00000007},
    {AUD_CORDIC_SHIFT_1,        0x00000007},
    {AUD_C1_UP_THR,                0x00007000},
    {AUD_C1_LO_THR,                0x00005400},
    {AUD_C2_UP_THR,                0x00005400},
    {AUD_C2_LO_THR,                0x00003000},
    {AUD_DCOC_0_SRC,            0x0000001a},
    {AUD_DCOC0_SHIFT,            0x00000000},
    {AUD_DCOC_0_SHIFT_IN0,        0x0000000a},
    {AUD_DCOC_0_SHIFT_IN1,        0x00000008},
    {AUD_DCOC_PASS_IN,            0x00000003},
    {AUD_IIR3_0_SEL,            0x00000021},
    {AUD_DN2_AFC,                0x00000002},
    {AUD_DCOC_1_SRC,            0x0000001b},
    {AUD_DCOC1_SHIFT,            0x00000000},
    {AUD_DCOC_1_SHIFT_IN0,        0x0000000a},
    {AUD_DCOC_1_SHIFT_IN1,        0x00000008},
    {AUD_IIR3_1_SEL,            0x00000023},
    {AUD_DN0_FREQ,                0x000035a3},
    {AUD_DN2_FREQ,                0x000029c7},
    {AUD_CRDC0_SRC_SEL,            0x00000511},
    {AUD_IIR1_0_SEL,            0x00000001},
    {AUD_IIR1_1_SEL,            0x00000000},
    {AUD_IIR3_2_SEL,            0x00000003},
    {AUD_IIR3_2_SHIFT,            0x00000000},
    {AUD_IIR3_0_SEL,            0x00000002},
    {AUD_IIR2_0_SEL,            0x00000021},
    {AUD_IIR2_0_SHIFT,            0x00000002},
    {AUD_DEEMPH0_SRC_SEL,        0x0000000b},
    {AUD_DEEMPH1_SRC_SEL,        0x0000000b},
    {AUD_POLYPH80SCALEFAC,        0x00000001},
    {AUD_START_TIMER,            0x00000000},
    {0, 0},
};

const CCX2388xCard::TAudioRegList CCX2388xCard::m_RegList_A2_BGDKM_Common[]=
{
    {AUD_INIT,                    SEL_A2},
    {AUD_INIT_LD,                0x00000001},
    {AUD_SOFT_RESET,            0x00000001},
    {AUD_ERRLOGPERIOD_R,        0x00000064},
    {AUD_ERRINTRPTTHSHLD1_R,    0x00000fff},
    {AUD_ERRINTRPTTHSHLD2_R,    0x0000001f},
    {AUD_ERRINTRPTTHSHLD3_R,    0x0000000f},
    {AUD_PDF_DDS_CNST_BYTE2,    0x06},
    {AUD_PDF_DDS_CNST_BYTE1,    0x82},
    {AUD_PDF_DDS_CNST_BYTE0,    0x12},
    {AUD_QAM_MODE,                0x05},
    {AUD_PHACC_FREQ_8MSB,        0x34},
    {AUD_PHACC_FREQ_8LSB,        0x4c},
    {AUD_RATE_ADJ1,                0x00000100},
    {AUD_RATE_ADJ2,                0x00000200},
    {AUD_RATE_ADJ3,                0x00000300},
    {AUD_RATE_ADJ4,                0x00000400},
    {AUD_RATE_ADJ5,                0x00000500},
    {AUD_THR_FR,                0x00000000},
    {AAGC_HYST,                    0x0000001a},
    {AUD_PILOT_BQD_1_K0,        0x0000755b},
    {AUD_PILOT_BQD_1_K1,        0x00551340},
    {AUD_PILOT_BQD_1_K2,        0x006d30be},
    {AUD_PILOT_BQD_1_K3,        0xffd394af},
    {AUD_PILOT_BQD_1_K4,        0x00400000},
    {AUD_PILOT_BQD_2_K0,        0x00040000},
    {AUD_PILOT_BQD_2_K1,        0x002a4841},
    {AUD_PILOT_BQD_2_K2,        0x00400000},
    {AUD_PILOT_BQD_2_K3,        0x00000000},
    {AUD_PILOT_BQD_2_K4,        0x00000000},
    {AUD_MODE_CHG_TIMER,        0x00000040},
    {AUD_AFE_12DB_EN,            0x00000001},
    {AUD_CORDIC_SHIFT_0,        0x00000007},
    {AUD_CORDIC_SHIFT_1,        0x00000007},
    {AUD_DEEMPH0_G0,            0x00000380},
    {AUD_DEEMPH1_G0,            0x00000380},
    {AUD_DCOC_0_SRC,            0x0000001a},
    {AUD_DCOC0_SHIFT,            0x00000000},
    {AUD_DCOC_0_SHIFT_IN0,        0x0000000a},
    {AUD_DCOC_0_SHIFT_IN1,        0x00000008},
    {AUD_DCOC_PASS_IN,            0x00000003},
    {AUD_IIR3_0_SEL,            0x00000021},
    {AUD_DN2_AFC,                0x00000002},
    {AUD_DCOC_1_SRC,            0x0000001b},
    {AUD_DCOC1_SHIFT,            0x00000000},
    {AUD_DCOC_1_SHIFT_IN0,        0x0000000a},
    {AUD_DCOC_1_SHIFT_IN1,        0x00000008},
    {AUD_IIR3_1_SEL,            0x00000023},
    {AUD_RDSI_SEL,                0x00000017},
    {AUD_RDSI_SHIFT,            0x00000000},
    {AUD_RDSQ_SEL,                0x00000017},
    {AUD_RDSQ_SHIFT,            0x00000000},
    {AUD_PLL_INT,                0x0000001e},
    {AUD_PLL_DDS,                0x00000000},
    {AUD_PLL_FRAC,                0x0000e542},
    {AUD_POLYPH80SCALEFAC,        0x00000001},
    {AUD_START_TIMER,            0x00000000},
    {0, 0},
};

const CCX2388xCard::TAudioRegList CCX2388xCard::m_RegList_A2_BG[]=
{
    {AUD_DMD_RA_DDS,            0x002a4f2f},
    {AUD_C1_UP_THR,                0x00007000},
    {AUD_C1_LO_THR,                0x00005400},
    {AUD_C2_UP_THR,                0x00005400},
    {AUD_C2_LO_THR,                0x00003000},
    {0, 0},
};

const CCX2388xCard::TAudioRegList CCX2388xCard::m_RegList_A2_DK[]=
{
    {AUD_DMD_RA_DDS,            0x002a4f2f},
    {AUD_C1_UP_THR,                0x00007000},
    {AUD_C1_LO_THR,                0x00005400},
    {AUD_C2_UP_THR,                0x00005400},
    {AUD_C2_LO_THR,                0x00003000},
    {AUD_DN0_FREQ,                0x00003a1c},
    {AUD_DN2_FREQ,                0x0000d2e0},
    {0, 0},
};

const CCX2388xCard::TAudioRegList CCX2388xCard::m_RegList_A2_M[]=
{
    {AUD_DMD_RA_DDS,            0x002a0425},
    {AUD_C1_UP_THR,                0x00003c00},
    {AUD_C1_LO_THR,                0x00003000},
    {AUD_C2_UP_THR,                0x00006000},
    {AUD_C2_LO_THR,                0x00003c00},
    {AUD_DEEMPH0_A0,            0x00007a80},
    {AUD_DEEMPH1_A0,            0x00007a80},
    {AUD_DEEMPH0_G0,            0x00001200},
    {AUD_DEEMPH1_G0,            0x00001200},
    {AUD_DN0_FREQ,                0x0000283b},
    {AUD_DN1_FREQ,                0x00003418},
    {AUD_DN2_FREQ,                0x000029c7},
    {AUD_POLY0_DDS_CONSTANT,    0x000a7540},
    {0, 0},
};

const CCX2388xCard::TAudioRegList CCX2388xCard::m_RegList_TV_Deemph50[]=
{

    {AUD_DEEMPH0_G0,            0x00000380},
    {AUD_DEEMPH1_G0,            0x00000380},
    {AUD_DEEMPHGAIN_R,            0x000011e1},
    {AUD_DEEMPHNUMER1_R,        0x0002a7bc},
    {AUD_DEEMPHNUMER2_R,        0x0003023c},
    {0, 0},
};

const CCX2388xCard::TAudioRegList CCX2388xCard::m_RegList_TV_Deemph75[]=
{
    {AUD_DEEMPH0_G0,            0x00000480},
    {AUD_DEEMPH1_G0,            0x00000480},
    {AUD_DEEMPHGAIN_R,            0x00009000},
    {AUD_DEEMPHNUMER1_R,        0x000353de},
    {AUD_DEEMPHNUMER2_R,        0x000001b1},
    {0, 0},
};

const CCX2388xCard::TAudioRegList CCX2388xCard::m_RegList_FM_Deemph75[]=
{
    {AUD_DEEMPH0_G0,            0x0000091b},
    {AUD_DEEMPH0_A0,            0x00006b68},
    {AUD_DEEMPH0_B0,            0x000011ec},
    {AUD_DEEMPH0_A1,            0x0003fc66},
    {AUD_DEEMPH0_B1,            0x0000399a},

    {AUD_DEEMPH1_G0,            0x00000aa0},
    {AUD_DEEMPH1_A0,            0x00006b68},
    {AUD_DEEMPH1_B0,            0x000011ec},
    {AUD_DEEMPH1_A1,            0x0003fc66},
    {AUD_DEEMPH1_B1,            0x0000399a},
    {0, 0},
};

const CCX2388xCard::TAudioRegList CCX2388xCard::m_RegList_FM_Deemph50[]=
{
    {AUD_DEEMPH0_G0,            0x00000c45},
    {AUD_DEEMPH0_A0,            0x00006262},
    {AUD_DEEMPH0_B0,            0x00001c29},
    {AUD_DEEMPH0_A1,            0x0003fc66},
    {AUD_DEEMPH0_B1,            0x0000399a},

    {AUD_DEEMPH1_G0,            0x00000d80},
    {AUD_DEEMPH1_A0,            0x00006262},
    {AUD_DEEMPH1_B0,            0x00001c29},
    {AUD_DEEMPH1_A1,            0x0003fc66},
    {AUD_DEEMPH1_B1,            0x0000399a},
    {0, 0},
};

void CCX2388xCard::SetAudioRegisters(const TAudioRegList* pAudioList)
{
    for(int i = 0; pAudioList[i].dwRegister; i++)
    {
        switch (pAudioList[i].dwRegister)
        {
        case AUD_PDF_DDS_CNST_BYTE0:
        case AUD_PDF_DDS_CNST_BYTE1:
        case AUD_PDF_DDS_CNST_BYTE2:
        case AUD_QAM_MODE:
        case AUD_PHACC_FREQ_8MSB:
        case AUD_PHACC_FREQ_8LSB:
            WriteByte(pAudioList[i].dwRegister, (BYTE)pAudioList[i].dwValue);
            break;

        default:
            WriteDword(pAudioList[i].dwRegister, pAudioList[i].dwValue);
            break;
        }
    }
}

void CCX2388xCard::AudioInit(int nInput, eVideoFormat TVFormat, eCX2388xAudioStandard Standard, eCX2388xStereoType StereoType)
{
    // stop the audio and wait for buffers to clear
    WriteDword(MO_AUD_DMACNTRL, 0x00000000);
    ::Sleep(100);

    if(Standard == AUDIO_STANDARD_AUTO)
    {
        switch (TVFormat)
        {
        case VIDEOFORMAT_NTSC_M:
        case VIDEOFORMAT_PAL_M:
            Standard = AUDIO_STANDARD_BTSC;
            break;

        case VIDEOFORMAT_NTSC_M_Japan:
            Standard = AUDIO_STANDARD_EIAJ;
            break;

        default:
            Standard = AUDIO_STANDARD_A2;
            break;
        }
    }

    m_CurrentAudioStandard    = Standard;
    m_CurrentStereoType        = StereoType;

    switch(Standard)
    {
    case AUDIO_STANDARD_BTSC:
        AudioInitBTSC(TVFormat, StereoType);
        break;
    case AUDIO_STANDARD_EIAJ:
        AudioInitEIAJ(TVFormat, StereoType);
        break;
    case AUDIO_STANDARD_A2:
        AudioInitA2(TVFormat, StereoType);
        break;
    case AUDIO_STANDARD_BTSC_SAP:
        AudioInitBTSCSAP(TVFormat, StereoType);
        break;
    case AUDIO_STANDARD_NICAM:
        AudioInitNICAM(TVFormat, StereoType);
        break;
    case AUDIO_STANDARD_FM:
    default:
        AudioInitFM(TVFormat, StereoType);
        break;
    }

    // start the audio running
    AudioInitDMA();
}

void CCX2388xCard::SetAudioVolume(WORD nVolume)
{
    // Unmute the audio and set volume
    DWORD dwval = 63 - MulDiv(nVolume, 63, 1000);
    WriteDword(AUD_VOL_CTL,dwval);
}

void CCX2388xCard::SetAudioBalance(WORD nBalance)
{
    short val = (short) nBalance;

    if(val <=0)
    {
        val = ((-val) >> 1);
    }
    else
    {
        val = (val >> 1);
        val |= (1 << 6);
    }
    WriteDword(AUD_BAL_CTL, val);
}


void CCX2388xCard::SetAudioMute()
{
    // Mute the audio
    DWORD dwval = ReadDword(AUD_VOL_CTL) & 0x1FF;
    dwval |= EN_DAC_MUTE_EN;
    WriteDword(AUD_VOL_CTL, dwval);
}

void CCX2388xCard::SetAudioUnMute(WORD nVolume)
{
    // Unmute the audio and set volume
    DWORD dwval = 63 - MulDiv(nVolume, 63, 1000);
    WriteDword(AUD_VOL_CTL,dwval);
}

void CCX2388xCard::AudioInitDMA()
{
    WriteDword(MO_AUDD_LNGTH,SRAM_FIFO_AUDIO_BUFFER_SIZE);
    WriteDword(MO_AUDR_LNGTH,SRAM_FIFO_AUDIO_BUFFER_SIZE);
    WriteDword(MO_AUD_DMACNTRL, 0x00000003);
}

void CCX2388xCard::AudioInitBTSC(eVideoFormat TVFormat, eCX2388xStereoType StereoType)
{
    SetAudioRegisters(m_RegList_BTSC);

    DWORD dwVal = EN_DAC_ENABLE|EN_FMRADIO_EN_RDS;

    switch (StereoType)
    {
    case STEREOTYPE_MONO:
    case STEREOTYPE_ALT1:
    case STEREOTYPE_ALT2:
        dwVal |= EN_BTSC_FORCE_MONO;
        break;

    case STEREOTYPE_STEREO:
        dwVal |= EN_BTSC_FORCE_STEREO;
        break;

    case STEREOTYPE_AUTO:
        dwVal |= EN_BTSC_AUTO_STEREO;
        break;
    }

    WriteDword(AUD_CTL,            dwVal     );
    WriteDword(AUD_SOFT_RESET,    0x00000000);
}

void CCX2388xCard::AudioInitBTSCSAP(eVideoFormat TVFormat, eCX2388xStereoType StereoType)
{
    SetAudioRegisters(m_RegList_BTSC_SAP);
    WriteDword(AUD_CTL,            EN_DAC_ENABLE|EN_FMRADIO_EN_RDS|EN_BTSC_FORCE_SAP);
    WriteDword(AUD_SOFT_RESET,    0x00000000);
}

void CCX2388xCard::AudioInitFM(eVideoFormat TVFormat, eCX2388xStereoType StereoType)
{
    // from v4l
    WriteDword(AUD_INIT,        SEL_FMRADIO);
    WriteDword(AUD_INIT_LD,        0x00000001);
    WriteDword(AUD_AFE_12DB_EN,    0x00000001);

    // don't know an better way
    switch(TVFormat)
    {
    case VIDEOFORMAT_NTSC_M:
    case VIDEOFORMAT_NTSC_M_Japan:
    case VIDEOFORMAT_NTSC_50:
        // (for US) Set De-emphasis filter coefficients for 75 usec
        SetAudioRegisters(m_RegList_FM_Deemph75);
        break;

    default:
        // (for Europe) Set De-emphasis filter coefficients for 50 usec
        SetAudioRegisters(m_RegList_FM_Deemph50);
        break;
    }

    WriteDword(AUD_CTL,            EN_DAC_ENABLE|EN_FMRADIO_AUTO_STEREO);
    WriteDword(AUD_SOFT_RESET,    0x00000000);
}

void CCX2388xCard::AudioInitEIAJ(eVideoFormat TVFormat, eCX2388xStereoType StereoType)
{
    SetAudioRegisters(m_RegList_EIAJ);
}

void CCX2388xCard::AudioInitNICAM(eVideoFormat TVFormat, eCX2388xStereoType StereoType)
{
    SetAudioRegisters(m_RegList_Nicam_Common);

    // Deemphasis 1:
    SetAudioRegisters(m_RegList_Nicam_Deemph1);

    // Deemphasis 2: please test this for other TVFormat
    //SetAudioRegisters(m_RegList_Nicam_Deemph2);

    // set QAM
    switch (TVFormat)
    {
    // 6.552 MHz
    case VIDEOFORMAT_PAL_I:
        WriteByte(AUD_PDF_DDS_CNST_BYTE0,    0x12);
        WriteByte(AUD_PHACC_FREQ_8MSB,        0x3a);
        WriteByte(AUD_PHACC_FREQ_8LSB,        0x93);
        break;

    // 5.85 MHz
    default:
        WriteByte(AUD_PDF_DDS_CNST_BYTE0,    0x16);
        WriteByte(AUD_PHACC_FREQ_8MSB,        0x34);
        WriteByte(AUD_PHACC_FREQ_8LSB,        0x4c);
        break;
    }

    DWORD dwTemp = EN_DAC_ENABLE|EN_DMTRX_LR|EN_DMTRX_BYPASS;

    switch(StereoType)
    {
    case STEREOTYPE_MONO:
    case STEREOTYPE_ALT1:
        dwTemp |= EN_NICAM_FORCE_MONO1;
        break;

    case STEREOTYPE_ALT2:
        dwTemp |= EN_NICAM_FORCE_MONO2;
        break;

    case STEREOTYPE_STEREO:
        dwTemp |= EN_NICAM_FORCE_STEREO;
        break;

    case STEREOTYPE_AUTO:
        dwTemp |= EN_NICAM_AUTO_STEREO;
        break;
    }

    WriteDword(AUD_CTL,            dwTemp);
    WriteDword(AUD_SOFT_RESET,    0x00000000);  // Causes a pop every time/**/
}

void CCX2388xCard::AudioInitA2(eVideoFormat TVFormat, eCX2388xStereoType StereoType)
{
    switch (TVFormat)
    {
    case VIDEOFORMAT_SECAM_L:
    case VIDEOFORMAT_SECAM_L1:          // not sure
        SetAudioRegisters(m_RegList_AM_L);
        StereoType = STEREOTYPE_MONO;    // always AM mono
        break;

    case VIDEOFORMAT_PAL_I:
        SetAudioRegisters(m_RegList_A1_I);
        SetAudioRegisters(m_RegList_TV_Deemph50);
        StereoType = STEREOTYPE_MONO;    // always FM mono
        break;

        case VIDEOFORMAT_PAL_B:
    case VIDEOFORMAT_PAL_G:
        SetAudioRegisters(m_RegList_A2_BGDKM_Common);
        SetAudioRegisters(m_RegList_A2_BG);
        SetAudioRegisters(m_RegList_TV_Deemph50);
        break;

    case VIDEOFORMAT_PAL_D:
    case VIDEOFORMAT_SECAM_K:
    case VIDEOFORMAT_SECAM_D:
        SetAudioRegisters(m_RegList_A2_BGDKM_Common);
        SetAudioRegisters(m_RegList_A2_DK);
        SetAudioRegisters(m_RegList_TV_Deemph50);
        break;

    case VIDEOFORMAT_NTSC_M:         // not sure
        SetAudioRegisters(m_RegList_A2_BGDKM_Common);
        SetAudioRegisters(m_RegList_A2_M);
        SetAudioRegisters(m_RegList_TV_Deemph75);
        break;
    }

    DWORD dwTemp = EN_DAC_ENABLE|EN_FMRADIO_EN_RDS|EN_DMTRX_SUMDIFF;
    switch(StereoType)
    {
    case STEREOTYPE_MONO:
    case STEREOTYPE_ALT1:
        dwTemp |= EN_A2_FORCE_MONO1;
        break;

    case STEREOTYPE_ALT2:
        dwTemp |= EN_A2_FORCE_MONO2;
        break;

    case STEREOTYPE_STEREO:
        dwTemp |= EN_A2_FORCE_STEREO;
        break;

    case STEREOTYPE_AUTO:
        dwTemp |= EN_A2_AUTO_STEREO;
        break;
    }

    WriteDword(AUD_CTL,            dwTemp);
    WriteDword(AUD_SOFT_RESET,    0x00000000);  // Causes a pop every time
}

eCX2388xAudioStandard CCX2388xCard::GetCurrentAudioStandard()
{
    return m_CurrentAudioStandard;
}

eCX2388xStereoType CCX2388xCard::GetCurrentStereoType()
{
    return m_CurrentStereoType;
}

#endif // WANT_CX2388X_SUPPORT
