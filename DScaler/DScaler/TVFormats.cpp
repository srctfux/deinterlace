/////////////////////////////////////////////////////////////////////////////
// $Id: TVFormats.cpp,v 1.6 2001-12-05 21:45:11 ittarnavsky Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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
// Revision 1.5  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.4  2001/11/18 10:07:00  temperton
// Bug fix.
//
// Revision 1.3  2001/11/02 17:03:59  adcockj
// Merge in PAL_NC change again
//
// Revision 1.2  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.1.2.1  2001/08/17 16:35:14  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "TVFormats.h"
#include "BT848_Defines.h"


static TTVFormat TVFormats[VIDEOFORMAT_LASTONE] =
{
    // PAL-B
    { 
        576, 1135, 0x7f, 0x72, (BT848_IFORM_PAL_BDGHI|BT848_IFORM_XT1),
        186, 922, 0x24, 0, TRUE, 511, 19,
        4.43361875, TRUE, 71, 626, 15,
        16,
    },
    // PAL-D
    { 
        576, 1135, 0x7f, 0x72, (BT848_IFORM_PAL_BDGHI|BT848_IFORM_XT1),
        186, 922, 0x24, 0, TRUE, 511, 19,
        4.43361875, TRUE, 71, 626, 15,
        16,
    },
    // PAL-G
    { 
        576, 1135, 0x7f, 0x72, (BT848_IFORM_PAL_BDGHI|BT848_IFORM_XT1),
        186, 922, 0x24, 0, TRUE, 511, 19,
        4.43361875, TRUE, 71, 626, 15,
        16,
    },
    // PAL-H
    { 
        576, 1135, 0x7f, 0x72, (BT848_IFORM_PAL_BDGHI|BT848_IFORM_XT1),
        186, 922, 0x24, 0, TRUE, 511, 19,
        4.43361875, TRUE, 71, 626, 15,
        16,
    },
    // PAL-I
    { 
        576, 1135, 0x7f, 0x72, (BT848_IFORM_PAL_BDGHI|BT848_IFORM_XT1),
        186, 922, 0x24, 0, TRUE, 511, 19,
        4.43361875, TRUE, 71, 626, 15,
        16,
    },
    // PAL-M
    {
        480,  910, 0x68, 0x5c, (BT848_IFORM_PAL_M|BT848_IFORM_XT0),
        137, 754, 0x1a, 0, FALSE, 400, 13,
        3.579545,  FALSE, 57, 512, 11,
        10,
    },
    // PAL-N
    {
        576, 1135, 0x7f, 0x72, (BT848_IFORM_PAL_N|BT848_IFORM_XT1),
        186, 922, 0x20, 0, TRUE, 511, 19,
        4.43361875, TRUE,  71, 626, 15,
        16,
    },
    // PAL-60
    {
        480, 1135, 0x7f, 0x72, (BT848_IFORM_PAL_BDGHI|BT848_IFORM_XT1),
        186, 922, 0x20, 0, FALSE, 400, 16,
        4.43361875, TRUE, 70, 626, 14,
        13,
    },
    // PAL-NC thanks to Eduardo Jos� Tagle
    { 
        576, 916, 0x68, 0x5c, (BT848_IFORM_PAL_NC|BT848_IFORM_XT0), 
        149, 745, 0x20, 0, TRUE, 511, 19, 
        3.579545, FALSE, 71, 626, 15, 
        16, 
    }, 
    // SECAM B
    {
        576, 1135, 0x7f, 0xb0, (BT848_IFORM_SECAM|BT848_IFORM_XT1),
        186, 922, 0x22, 0, TRUE, 511, 19,
        4.43361875, TRUE, 71, 633, 15,
        16,
    },
    // SECAM D
    {
        576, 1135, 0x7f, 0xb0, (BT848_IFORM_SECAM|BT848_IFORM_XT1),
        186, 922, 0x22, 0, TRUE, 511, 19,
        4.43361875, TRUE, 71, 633, 15,
        16,
    },
    // SECAM G
    {
        576, 1135, 0x7f, 0xb0, (BT848_IFORM_SECAM|BT848_IFORM_XT1),
        186, 922, 0x22, 0, TRUE, 511, 19,
        4.43361875, TRUE, 71, 633, 15,
        16,
    },
    // SECAM H
    {
        576, 1135, 0x7f, 0xb0, (BT848_IFORM_SECAM|BT848_IFORM_XT1),
        186, 922, 0x22, 0, TRUE, 511, 19,
        4.43361875, TRUE, 71, 633, 15,
        16,
    },
    // SECAM K
    {
        576, 1135, 0x7f, 0xb0, (BT848_IFORM_SECAM|BT848_IFORM_XT1),
        186, 922, 0x22, 0, TRUE, 511, 19,
        4.43361875, TRUE, 71, 633, 15,
        16,
    },
    // SECAM K1
    {
        576, 1135, 0x7f, 0xb0, (BT848_IFORM_SECAM|BT848_IFORM_XT1),
        186, 922, 0x22, 0, TRUE, 511, 19,
        4.43361875, TRUE, 71, 633, 15,
        16,
    },
    // SECAM L
    {
        576, 1135, 0x7f, 0xb0, (BT848_IFORM_SECAM|BT848_IFORM_XT1),
        186, 922, 0x22, 0, TRUE, 511, 19,
        4.43361875, TRUE, 71, 633, 15,
        16,
    },
    // SECAM L1
    {
        576, 1135, 0x7f, 0xb0, (BT848_IFORM_SECAM|BT848_IFORM_XT1),
        186, 922, 0x22, 0, TRUE, 511, 19,
        4.43361875, TRUE, 71, 633, 15,
        16,
    },
    // NTSC M
    {
        480, 910, 0x68, 0x5c, (BT848_IFORM_NTSC|BT848_IFORM_XT0),
        137, 754, 0x1a, 0, FALSE, 400, 13,
        3.579545,  FALSE, 57, 512, 11, 
        10,
    },
    // NTSC M Japan
    {
        480,  910, 0x70, 0x5c, (BT848_IFORM_NTSC_JAP|BT848_IFORM_XT0),
        135, 754, 0x1a, 0, FALSE, 400, 13,
        3.579545, FALSE, 57, 512, 11, 
        10,
    },
    // NTSC-50
    {       
        576, 910, 0x68, 0x5c, (BT848_IFORM_NTSC|BT848_IFORM_XT0),
        137, 754, 0x24, 0, TRUE, 511, 19,
        3.579545,  FALSE, 71, 626, 15,      
        16, 
    },
};

const char *VideoFormatNames[VIDEOFORMAT_LASTONE] = 
{
    "PAL-B",
    "PAL-D",
    "PAL-G",
    "PAL-H",
    "PAL-I",
    "PAL-M",
    "PAL-N",
    "PAL-60",
    "PAL-N-COMBO",

    "SECAM-B",
    "SECAM-D",
    "SECAM-G",
    "SECAM-H",
    "SECAM-K",
    "SECAM-K1",
    "SECAM-L",
    "SECAM-L1",

    "NTSC-M",
    "NTSC-M-Japan",
    "NTSC-50",
};

TTVFormat* GetTVFormat(eVideoFormat Format)
{
    return &TVFormats[Format];
}

bool IsPALVideoFormat(eVideoFormat Format)
{
    bool result = false;
    switch (Format)
    {
    case VIDEOFORMAT_PAL_B:
    case VIDEOFORMAT_PAL_D:
    case VIDEOFORMAT_PAL_G:
    case VIDEOFORMAT_PAL_H:
    case VIDEOFORMAT_PAL_I:
    case VIDEOFORMAT_PAL_M:
    case VIDEOFORMAT_PAL_N:
    case VIDEOFORMAT_PAL_60:
    case VIDEOFORMAT_PAL_N_COMBO:
        result = true;
        break;
    }
    return result;
}

bool IsNTSCVideoFormat(eVideoFormat Format)
{
    bool result = false;
    switch (Format)
    {
    case VIDEOFORMAT_NTSC_M:
    case VIDEOFORMAT_NTSC_M_Japan:
    case VIDEOFORMAT_NTSC_50:
        result = true;
        break;
    }
    return result;
}

bool IsSECAMVideoFormat(eVideoFormat Format)
{
    bool result = false;
    switch (Format)
    {
    case VIDEOFORMAT_SECAM_B:
    case VIDEOFORMAT_SECAM_D:
    case VIDEOFORMAT_SECAM_G:
    case VIDEOFORMAT_SECAM_H:
    case VIDEOFORMAT_SECAM_K:
    case VIDEOFORMAT_SECAM_K1:
    case VIDEOFORMAT_SECAM_L:
    case VIDEOFORMAT_SECAM_L1:
        result = true;
        break;
    }
    return result;
}