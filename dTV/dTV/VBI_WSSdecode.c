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
#include "math.h"
#include "VBI_WSSdecode.h"
#include "vbi.h"
#include "bt848.h"
#include "AspectRatio.h"
#define DOLOGGING
#include "DebugLog.h"

#define	WSS_MAX_SUCCESSIVE_ERR		10

// Structure of WSS signal for 625-line systems
#define	WSS625_RUNIN_CODE_LENGTH	29
#define	WSS625_START_CODE_LENGTH	24
#define	WSS625_BEFORE_DATA_LENGTH	(WSS625_RUNIN_CODE_LENGTH+WSS625_START_CODE_LENGTH)
#define	WSS625_DATA_BIT_LENGTH		6
#define	WSS625_NB_DATA_BITS			14
#define	WSS625_START_POS_MIN		110
#define	WSS625_START_POS_MAX		150

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

#define AR_NONANAMORPHIC 1
#define AR_ANAMORPHIC    2

#define ROUND(d)	(int)floor((d)+0.5)

extern int decodebit(unsigned char *data, int threshold, int NumPixels);

// WSS decoded data
int		WSSAspectRatio = -1;
int		WSSAspectMode = -1;
BOOL	WSSFilmMode = FALSE;
BOOL	WSSColorPlus = FALSE;
BOOL	WSSHelperSignals = FALSE;
BOOL	WSSTeletextSubtitle = FALSE;
int		WSSOpenSubtitles = WSS625_SUBTITLE_NO;
BOOL	WSSSurroundSound = FALSE;
BOOL	WSSCopyrightAsserted = FALSE;
BOOL	WSSCopyProtection = FALSE;

// WSS control data
BOOL	WSSDecodeOk = FALSE;		// Status of last decoding
int		WSSNbDecodeErr = 0;			// Number of decoding errors
int		WSSNbDecodeOk = 0;			// Number of correct decoding
int		WSSMinPos = WSS625_START_POS_MAX;
int		WSSMaxPos = WSS625_START_POS_MIN;
int		WSSTotalPos = 0;
static int	WSSNbSuccessiveErr = 0;	// Number of successive decoding errors

// Sequence values for run-in code
static int WSS625_runin[WSS625_RUNIN_CODE_LENGTH] = { 1,1,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1 };

// Sequence values for start code
static int WSS625_start[WSS625_START_CODE_LENGTH] = { 0,0,0,1,1,1,1,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,1 };

// Sequence values for a data bit = 0
static int WSS625_0[WSS625_DATA_BIT_LENGTH] = { 0,0,0,1,1,1 };

// Sequence values for a data bit = 0
static int WSS625_1[WSS625_DATA_BIT_LENGTH] = { 1,1,1,0,0,0 };

// Clear WSS decoded data
static void WSS_clear_data ()
{
	WSSAspectRatio = -1;
	WSSAspectMode = -1;
	WSSFilmMode = FALSE;
	WSSColorPlus = FALSE;
	WSSHelperSignals = FALSE;
	WSSTeletextSubtitle = FALSE;
	WSSOpenSubtitles = WSS625_SUBTITLE_NO;
	WSSSurroundSound = FALSE;
	WSSCopyrightAsserted = FALSE;
	WSSCopyProtection = FALSE;
}

// Clear WSS decoded data and WSS control data
void WSS_init ()
{
	// Clear WSS control data
	WSSDecodeOk = FALSE;
	WSSNbDecodeErr = 0;
	WSSNbDecodeOk = 0;
	WSSMinPos = WSS625_START_POS_MAX;
	WSSMaxPos = WSS625_START_POS_MIN;
	WSSTotalPos = 0;
	WSSNbSuccessiveErr = 0;

	// Clear WSS decoded data
	WSS_clear_data ();
}

static void log_databit(int bit_pos, int decoded_bit, BYTE* vbiline, double ClockPixels)
{
	int	i, pos;

	for (i = 0 ; i < WSS625_DATA_BIT_LENGTH ; i++)
	{
		pos = ROUND (ClockPixels * i);
		LOG("WSS bit b%d : %d => %x %x %x %x %x %x %x", bit_pos, decoded_bit, vbiline[pos], vbiline[pos+1], vbiline[pos+2], vbiline[pos+3], vbiline[pos+4], vbiline[pos+5], vbiline[pos+6]);
	}
}

static BOOL decode_sequence(BYTE* vbiline, int *DecodedVals, int NbVal, int Threshold, double ClockPixels)
{
	int	i;
	int	NbPixels = ROUND (ClockPixels);

	for (i = 0 ; i < NbVal ; i++)
	{
		if (decodebit (vbiline + ROUND (ClockPixels * i), Threshold, NbPixels) != DecodedVals[i])
			break;
	}
	return ( (i == NbVal) ? TRUE : FALSE );
}

static BOOL WSS625_DecodeLine(BYTE* vbiline)
{
	int		i, j;
	BOOL	DecodeOk = FALSE;
	double	ClockPixels;
	int		Threshold = 0;
	int		pos;
	int		bits[WSS625_NB_DATA_BITS];
	int		packedbits;
	int		nb;

	ClockPixels = 7.09379;
	Threshold = VBI_thresh;
	if (Threshold < 10)
	{
//		LOG("WSS signal threshold < 10");
		return FALSE;
	}

	for (i = WSS625_START_POS_MIN ; i <= WSS625_START_POS_MAX ; i++)
	{
		// run-in code decoding
		pos = i;
		if (decode_sequence (vbiline + pos, WSS625_runin, WSS625_RUNIN_CODE_LENGTH, Threshold, ClockPixels))
		{
//			LOG("WSS run-in code detected (i = %d, Threshold = %x)", i, Threshold);

			// Start code decoding
			pos = i + ROUND (ClockPixels * WSS625_RUNIN_CODE_LENGTH);
			if (decode_sequence (vbiline + pos, WSS625_start, WSS625_START_CODE_LENGTH, Threshold, ClockPixels))
			{
//				LOG("WSS start code detected");

				// Data bits decoding
				nb = 0;
				for (j = 0 ; j < WSS625_NB_DATA_BITS ; j++)
				{
					pos = i + ROUND (ClockPixels * (WSS625_BEFORE_DATA_LENGTH + j * WSS625_DATA_BIT_LENGTH));
					if (decode_sequence (vbiline + pos, WSS625_0, WSS625_DATA_BIT_LENGTH, Threshold, ClockPixels))
					{
						bits[j] = 0;
						nb++;
//						LOG("WSS b%d = 0 (i = %d, Threshold = %x)", j, i, Threshold);
					}
					else if (decode_sequence (vbiline + pos, WSS625_1, WSS625_DATA_BIT_LENGTH, Threshold, ClockPixels))
					{
						bits[j] = 1;
						nb++;
//						LOG("WSS b%d = 1 (i = %d, Threshold = %x)", j, i, Threshold);
					}
					else
					{
						bits[j] = -1;
//						LOG("WSS b%d = ?", j);
					}
				}
				if (nb == WSS625_NB_DATA_BITS)
				{
					DecodeOk = TRUE;
					break;
				}
			}
		}
	}

//	if (! DecodeOk)
//	{
//		i = 0;
//		LOG("WSS start pos = %d, Threshold = %x, Min = %x, Max = %x", i, Threshold, min, max);
//		for (j = 0 ; j < 250 ; j++)
//		{
//			pos = i + ROUND (ClockPixels * j);
//			LOG("WSS pos = %d : %x %x %x %x %x %x %x", pos, vbiline[pos], vbiline[pos + 1], vbiline[pos + 2], vbiline[pos + 3], vbiline[pos + 4], vbiline[pos + 5], vbiline[pos + 6]);
//		}
//	}
//	else
//	{
//		LOG("WSS start pos = %d, Threshold = %x", i, Threshold);
//		for (j = 0 ; j < WSS625_NB_DATA_BITS ; j++)
//		{
//			pos = i + ROUND (ClockPixels * (WSS625_BEFORE_DATA_LENGTH + j * WSS625_DATA_BIT_LENGTH));
//			log_databit (j, bits[j], vbiline + pos, ClockPixels);
//			LOG("WSS bit b%d = %d",j,bits[j]);
//		}
//	}

	if (DecodeOk)
	{
		// Decoding statistics
		WSSTotalPos += i;
		if (i < WSSMinPos)
			WSSMinPos = i;
		if (i > WSSMaxPos)
			WSSMaxPos = i;

		packedbits = 0;
		for (j = 0 ; j < WSS625_NB_DATA_BITS ; j++)
		{
			packedbits |= bits[j]<<j;
		}
		switch (packedbits & 0x000f)
		{
		case WSS625_RATIO_133:
			WSSAspectMode = AR_NONANAMORPHIC;
			WSSAspectRatio = 1333;
			break;
		case WSS625_RATIO_177_ANAMORPHIC:
			WSSAspectMode = AR_ANAMORPHIC;
			WSSAspectRatio = 1778;
			break;
		case WSS625_RATIO_155:
		case WSS625_RATIO_155_LETTERBOX_CENTER:
		case WSS625_RATIO_155_LETTERBOX_TOP:
			WSSAspectMode = AR_NONANAMORPHIC;
			WSSAspectRatio = 1555;
			break;
		case WSS625_RATIO_177_LETTERBOX_CENTER:
		case WSS625_RATIO_177_LETTERBOX_TOP:
			WSSAspectMode = AR_NONANAMORPHIC;
			WSSAspectRatio = 1778;
			break;
		case WSS625_RATIO_BIG_LETTERBOX_CENTER:
			WSSAspectMode = AR_NONANAMORPHIC;
			WSSAspectRatio = -1;
			break;
		default:
			WSSAspectMode = -1;
			WSSAspectRatio = -1;
			break;
		}
		WSSFilmMode = (packedbits & 0x0010) ? TRUE : FALSE;
		WSSColorPlus = (packedbits & 0x0020) ? TRUE : FALSE;
		WSSHelperSignals = (packedbits & 0x0040) ? TRUE : FALSE;
		WSSTeletextSubtitle = (packedbits & 0x0100) ? TRUE : FALSE;
		WSSOpenSubtitles = packedbits & 0x0600;
		WSSSurroundSound = (packedbits & 0x0800) ? TRUE : FALSE;
		WSSCopyrightAsserted = (packedbits & 0x1000) ? TRUE : FALSE;
		WSSCopyProtection = (packedbits & 0x2000) ? TRUE : FALSE;
	}

	return DecodeOk;
}

static BOOL WSS525_DecodeLine(BYTE* vbiline)
{
	BOOL	DecodeOk = FALSE;
	int		packedbits;

	// !!!!!!!!!!!!!!!!!!!!
	// !!! Code missing !!!
	// !!!!!!!!!!!!!!!!!!!!

	if (DecodeOk)
	{
		packedbits = 0;
		switch (packedbits & 0x0003)
		{
		case WSS525_RATIO_133:
		case WSS525_RATIO_133_LETTERBOX:
			WSSAspectMode = AR_NONANAMORPHIC;
			WSSAspectRatio = 1333;
			break;
		case WSS525_RATIO_177_ANAMORPHIC:
			WSSAspectMode = AR_ANAMORPHIC;
			WSSAspectRatio = 1778;
			break;
		default:
			WSSAspectMode = -1;
			WSSAspectRatio = -1;
			break;
		}
	}

	return DecodeOk;
}

int WSS_DecodeLine(BYTE* vbiline)
{
	int		PrevAspectMode = WSSAspectMode;
	int		PrevAspectRatio = WSSAspectRatio;
	int		NewAspectMode;
	int		NewAspectRatio;
	BOOL	bSwitch = FALSE;

	switch (BT848_GetTVFormat()->wCropHeight)
	{
	// 625-line systems
	case 576:
		WSSDecodeOk = WSS625_DecodeLine(vbiline);
		break;

	// 525-line systems
	case 400:
		WSSDecodeOk = WSS525_DecodeLine(vbiline);
		break;

	default:
		WSSDecodeOk = FALSE;
		break;
	}

	if (! WSSDecodeOk)
	{
		WSSNbDecodeErr++;
		WSSNbSuccessiveErr++;
		// Clear WSS decoded data
		// after two many successive decoding errors
		if (WSSNbSuccessiveErr > WSS_MAX_SUCCESSIVE_ERR)
		{
			WSS_clear_data();
			WSSNbSuccessiveErr = 0;
		}
		return -1;
	}
	else
	{
		WSSNbDecodeOk++;
		WSSNbSuccessiveErr = 0;

		// Manage WSS ratio information
		if ( (WSSAspectMode != -1)
		  && (WSSAspectRatio != -1) )
		{
			if ( (WSSAspectMode != PrevAspectMode)
			  || (WSSAspectRatio != PrevAspectRatio) )
			{
				bSwitch = TRUE;
				NewAspectMode = WSSAspectMode;
				NewAspectRatio = WSSAspectRatio;
			}
			else
			{
				NewAspectMode = WSSAspectMode;
				if (WSSAspectMode != aspectSettings.aspect_mode)
				{
					bSwitch = TRUE;
				}
				if (WSSAspectRatio > aspectSettings.source_aspect)
				{
					bSwitch = TRUE;
					NewAspectRatio = WSSAspectRatio;
				}
				else
				{
					NewAspectRatio = aspectSettings.source_aspect;
				}
			}
			if (bSwitch
			 && ( (NewAspectMode != aspectSettings.aspect_mode)
			   || (NewAspectRatio != aspectSettings.source_aspect) ) )
			{
				SwitchToRatio (NewAspectMode, NewAspectRatio);
			}
		}
		return 0;
	}
}
