/////////////////////////////////////////////////////////////////////////////
// VBI_WSSdecode.c
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 Laurent Garnier.  All rights reserved.
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
//
//	GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
//
//  This is the WideScreen Signaling DECODER.
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 11 Mar 2001   Laurent Garnier       New file
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VBI_WSSdecode.h"
#include "bt848.h"
#include "AspectRatio.h"
#define DOLOGGING
#include "DebugLog.h"

// Possible ratio values for 625-line systems
#define	WSS625_RATIO_133					0x08
#define	WSS625_RATIO_155					0x0e
#define	WSS625_RATIO_177_ANAMORPHIC			0x07
#define	WSS625_RATIO_155_LETTERBOX_CENTER	0x01
#define	WSS625_RATIO_155_LETTERBOX_TOP		0x02
#define	WSS625_RATIO_177_LETTERBOX_CENTER	0x0b
#define	WSS625_RATIO_177_LETTERBOX_TOP		0x04
#define	WSS625_RATIO_BIG_LETTERBOX_CENTER	0x0d

// Possible ratio values for 525-line systems
#define	WSS525_RATIO_133					0x00
#define	WSS525_RATIO_177_ANAMORPHIC			0x01
#define	WSS525_RATIO_133_LETTERBOX			0x02

extern void SwitchToRatio(int nMode, int nRatio);

// Last value decoded
unsigned char	WSSCurrentRatio = -1;

// #define TEST_WSS

#ifdef TEST_WSS
// Only for preliminary tests
#define	CST_TST	500
int	cpt = 0;
unsigned char Ratios625[8] = {
	WSS625_RATIO_133,
	WSS625_RATIO_155,
	WSS625_RATIO_177_ANAMORPHIC,
	WSS625_RATIO_155_LETTERBOX_CENTER,
	WSS625_RATIO_155_LETTERBOX_TOP,
	WSS625_RATIO_177_LETTERBOX_CENTER,
	WSS625_RATIO_177_LETTERBOX_TOP,
	WSS625_RATIO_BIG_LETTERBOX_CENTER,
};
unsigned char Ratios525[3] = {
	WSS525_RATIO_133,
	WSS525_RATIO_177_ANAMORPHIC,
	WSS525_RATIO_133_LETTERBOX,
};
#endif

int WSS625_DecodeLine(BYTE* vbiline)
{
	unsigned char	NewRatio;

#ifdef TEST_WSS
	// Only for preliminary tests
	if (cpt >= (CST_TST*8))
	{
		cpt = 0;
	}
	cpt++;
	if ((cpt % CST_TST) == 0)
	{
		NewRatio = Ratios625[(cpt / CST_TST) - 1];
	}
	else
	{
		NewRatio = WSSCurrentRatio;
	}
#else
	NewRatio = WSSCurrentRatio;
#endif

	if (NewRatio == WSSCurrentRatio)	return (0);

	WSSCurrentRatio = NewRatio;

	switch (WSSCurrentRatio)
	{
	case WSS625_RATIO_133:
		SwitchToRatio(1,1333);
		LOG("WSS ratio (%x): 1.33", WSSCurrentRatio);
		break;
	case WSS625_RATIO_177_ANAMORPHIC:
		SwitchToRatio(2,1778);
		LOG("WSS ratio (%x) : 1.77 anamorphic", WSSCurrentRatio);
		break;
	case WSS625_RATIO_155:
	case WSS625_RATIO_155_LETTERBOX_CENTER:
	case WSS625_RATIO_155_LETTERBOX_TOP:
		SwitchToRatio(1,1555);
		LOG("WSS ratio (%x) : 1.55 letterbox", WSSCurrentRatio);
		break;
	case WSS625_RATIO_177_LETTERBOX_CENTER:
	case WSS625_RATIO_177_LETTERBOX_TOP:
		SwitchToRatio(1,1778);
		LOG("WSS ratio (%x) : 1.77 letterbox", WSSCurrentRatio);
		break;
	case WSS625_RATIO_BIG_LETTERBOX_CENTER:
	default:
		LOG("WSS ratio (%x) : ???", WSSCurrentRatio);
		break;
	}
	return 0;
}

int WSS525_DecodeLine(BYTE* vbiline)
{
	unsigned char	NewRatio;

#ifdef TEST_WSS
	// Only for preliminary tests
	if (cpt >= (CST_TST*3))
	{
		cpt = 0;
	}
	cpt++;
	if ((cpt % CST_TST) == 0)
	{
		NewRatio = Ratios525[(cpt / CST_TST) - 1];
	}
	else
	{
		NewRatio = WSSCurrentRatio;
	}
#else
	NewRatio = WSSCurrentRatio;
#endif

	if (NewRatio == WSSCurrentRatio)	return (0);

	WSSCurrentRatio = NewRatio;

	switch (WSSCurrentRatio)
	{
	case WSS525_RATIO_133:
	case WSS525_RATIO_133_LETTERBOX:
		SwitchToRatio(1,1333);
		LOG("WSS ratio (%x): 1.33", WSSCurrentRatio);
		break;
	case WSS525_RATIO_177_ANAMORPHIC:
		SwitchToRatio(2,1778);
		LOG("WSS ratio (%x) : 1.77 anamorphic", WSSCurrentRatio);
		break;
	default:
		LOG("WSS ratio (%x) : ???", WSSCurrentRatio);
		break;
	}
	return 0;
}

int WSS_DecodeLine(BYTE* vbiline)
{
	// 625-line systems
	if (BT848_GetTVFormat()->wCropHeight == 576)
	{
		return (WSS625_DecodeLine(vbiline));
	}
	// 525-line systems
	else if (BT848_GetTVFormat()->wCropHeight == 480)
	{
		return (WSS525_DecodeLine(vbiline));
	}
	return 0;
}
