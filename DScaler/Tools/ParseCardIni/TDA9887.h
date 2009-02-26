/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 Torsten Seeboth. All rights reserved.
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


#ifndef _TDA_9887_H_
#define _TDA_9887_H_

//
// TDA defines
//

#define I2C_TDA9887_0                    0x86    // MAD1
#define I2C_TDA9887_1                    0x96    // MAD3
#define I2C_TDA9887_2                    0x84    // MAD2
#define I2C_TDA9887_3                    0x94    // MAD4


//// first reg
#define TDA9887_VideoTrapBypassOFF        0x00    // bit b0
#define TDA9887_VideoTrapBypassON        0x01    // bit b0

#define TDA9887_AutoMuteFmInactive        0x00    // bit b1
#define TDA9887_AutoMuteFmActive        0x02    // bit b1

#define TDA9887_Intercarrier            0x00    // bit b2
#define TDA9887_QSS                        0x04    // bit b2

#define TDA9887_PositiveAmTV            0x00    // bit b3:4
#define TDA9887_FmRadio                    0x08    // bit b3:4
#define TDA9887_NegativeFmTV            0x10    // bit b3:4

#define TDA9887_ForcedMuteAudioON        0x20    // bit b5
#define TDA9887_ForcedMuteAudioOFF        0x00    // bit b5

#define TDA9887_OutputPort1Active        0x00    // bit b6
#define TDA9887_OutputPort1Inactive        0x40    // bit b6
#define TDA9887_OutputPort2Active        0x00    // bit b7
#define TDA9887_OutputPort2Inactive        0x80    // bit b7

//// second reg
#define TDA9887_TakeOverPointMin        0x00    // bit c0:4
#define TDA9887_TakeOverPointDefault    0x10    // bit c0:4
#define TDA9887_TakeOverPointMax        0x1f    // bit c0:4
#define TDA9887_DeemphasisOFF            0x00    // bit c5
#define TDA9887_DeemphasisON            0x20    // bit c5
#define TDA9887_Deemphasis75            0x00    // bit c6
#define TDA9887_Deemphasis50            0x40    // bit c6
#define TDA9887_AudioGain0                0x00    // bit c7
#define TDA9887_AudioGain6                0x80    // bit c7

//// third reg
#define TDA9887_AudioIF_4_5                0x00    // bit e0:1
#define TDA9887_AudioIF_5_5                0x01    // bit e0:1
#define TDA9887_AudioIF_6_0                0x02    // bit e0:1
#define TDA9887_AudioIF_6_5                0x03    // bit e0:1

#define TDA9887_VideoIF_58_75            0x00    // bit e2:4
#define TDA9887_VideoIF_45_75            0x04    // bit e2:4
#define TDA9887_VideoIF_38_90            0x08    // bit e2:4
#define TDA9887_VideoIF_38_00            0x0C    // bit e2:4
#define TDA9887_VideoIF_33_90            0x10    // bit e2:4
#define TDA9887_VideoIF_33_40            0x14    // bit e2:4
#define TDA9887_RadioIF_45_75            0x18    // bit e2:4
#define TDA9887_RadioIF_38_90            0x1C    // bit e2:4

#define TDA9887_TunerGainNormal            0x00    // bit e5
#define TDA9887_TunerGainLow            0x20    // bit e5

#define TDA9887_Gating_18                0x00    // bit e6
#define TDA9887_Gating_36                0x40    // bit e6

#define TDA9887_AgcOutON                0x80    // bit e7
#define TDA9887_AgcOutOFF                0x00    // bit e7



// TV format groupings used by TDA9887
enum eTDA9887Format
{
    TDA9887_FORMAT_NONE        = -1,
    TDA9887_FORMAT_PAL_BG    = 0,
    TDA9887_FORMAT_PAL_I,
    TDA9887_FORMAT_PAL_DK,
    TDA9887_FORMAT_PAL_MN,
    TDA9887_FORMAT_SECAM_L,
    TDA9887_FORMAT_SECAM_DK,
    TDA9887_FORMAT_NTSC_M,
    TDA9887_FORMAT_NTSC_JP,
    TDA9887_FORMAT_RADIO,
    TDA9887_FORMAT_LASTONE,
};

// Bits for SetCardSpecifics(...)'s TTDA9887CardSpecifics
enum
{
    TDA9887_SM_CARRIER_QSS                = 0x20,    // != Intercarrier
    TDA9887_SM_OUTPUTPORT1_INACTIVE        = 0x40,    // != Active
    TDA9887_SM_OUTPUTPORT2_INACTIVE        = 0x80,    // != Active
    TDA9887_SM_TAKEOVERPOINT_MASK        = 0x1F,
    TDA9887_SM_TAKEOVERPOINT_OFFSET        = 0,

    TDA9887_SM_TAKEOVERPOINT_DEFAULT    = 0x10,    // 0 dB
    TDA9887_SM_TAKEOVERPOINT_MIN        = 0x00,    // -16 dB
    TDA9887_SM_TAKEOVERPOINT_MAX        = 0x1F,    // +15 dB
};

// Only the modes specified in the enum above can be changed with
// SetModes(...).  To change a mode, add the respective constant to the
// 'mask' value then specify the new value in 'bits'.  For example, to
// use the QSS carrier mode and active OutputPort2 mode, the
// following will be used:
//
// mask = TDA9887_SM_CARRIER_QSS|TDA9887_SM_OUTPUTPORT2_INACTIVE;
// value = TDA9887_SM_CARRIER_QSS;
//
// If no changes are made, modes listed in k_TDAStandardtSettings are
// used.

// Input structure for SetModes(...).
typedef struct
{
    BYTE    mask;
    BYTE    bits;
} TTDA9887Modes;

// Input structure for SetModes(...).
typedef struct
{
    eTDA9887Format    format;
    BYTE            mask;
    BYTE            bits;
} TTDA9887FormatModes;


#endif
