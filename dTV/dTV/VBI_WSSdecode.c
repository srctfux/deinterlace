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

#define	WSS625_RUNIN_CODE_LENGTH	29
#define	WSS625_START_CODE_LENGTH	24
#define	WSS625_BEFORE_DATA_LENGTH	(WSS625_RUNIN_CODE_LENGTH+WSS625_START_CODE_LENGTH)
#define	WSS625_DATA_BIT_LENGTH		6

#define	WSS625_NB_DATA_BITS			14

#define ROUND(d)	(int)floor((d)+0.5)

extern int decodebit(unsigned char *data, int threshold, int NumPixels);

// Last WSS data decoded
int		WSSRatio = -1;
BOOL	WSSFilmMode = FALSE;
BOOL	WSSColorPlus = FALSE;
BOOL	WSSHelperSignals = FALSE;
BOOL	WSSTeletextSubtitle = FALSE;
int		WSSOpenSubtitles = WSS625_SUBTITLE_NO;
BOOL	WSSSurroundSound = FALSE;
BOOL	WSSCopyrightAsserted = FALSE;
BOOL	WSSCopyProtection = FALSE;

BOOL	WSSDecodeOk = FALSE;	// Status of last decoding
int		WSSNbDecodeErr = 0;		// Number of decoding errors
int		WSSNbDecodeOk = 0;		// Number of correct decoding
int		WSSMinPos = 0;
int		WSSMaxPos = 0;
int		WSSAvgPos = 0;
int		WSSTotalPos = 0;

// Sequence values for run-in code
int WSS625_runin[WSS625_RUNIN_CODE_LENGTH] = { 1,1,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1 };

// Sequence values for start code
int WSS625_start[WSS625_START_CODE_LENGTH] = { 0,0,0,1,1,1,1,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,1 };

// Sequence values for a data bit = 0
int WSS625_0[WSS625_DATA_BIT_LENGTH] = { 0,0,0,1,1,1 };

// Sequence values for a data bit = 0
int WSS625_1[WSS625_DATA_BIT_LENGTH] = { 1,1,1,0,0,0 };

void log_databit(int bit_pos, int decoded_bit, BYTE* vbiline, double ClockPixels)
{
	int	i, pos;

	for (i = 0 ; i < WSS625_DATA_BIT_LENGTH ; i++)
	{
		pos = ROUND (ClockPixels * i);
		LOG("WSS bit b%d : %d => %x %x %x %x %x %x %x", bit_pos, decoded_bit, vbiline[pos], vbiline[pos+1], vbiline[pos+2], vbiline[pos+3], vbiline[pos+4], vbiline[pos+5], vbiline[pos+6]);
	}
}

BOOL decode_sequence(BYTE* vbiline, int *DecodedVals, int NbVal, int Threshold, double ClockPixels)
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

BOOL WSS625_DecodeLine(BYTE* vbiline)
{
	int		OldRatio;
	int		OldFilmMode;
	int		OldColorPlus;
	int		OldHelperSignals;
	int		OldTeletextSubtitle;
	int		OldOpenSubtitles;
	int		OldSurroundSound;
	int		OldCopyrightAsserted;
	int		OldCopyProtection;
	int		i, j;
	BOOL	DecodeOk = FALSE;
	double	ClockPixels;
	int		Threshold = 0;
	int		pos;
	int		bits[WSS625_NB_DATA_BITS];
	int		packedbits;
	int		nb;

//	int		min = 255;
//	int		max = 0;

	ClockPixels = 7.09379;
	Threshold = VBI_thresh;
//	for (i = 50 ; i < 1500 ; i++)
//	{
//		if (vbiline[i] > max)	max = vbiline[i];
//		if (vbiline[i] < min)	min = vbiline[i];
//	}
//	Threshold = (max - min) / 2 + min;
	if (Threshold < 10)
	{
		WSSNbDecodeErr++;
//		LOG("WSS signal threshold < 10");
		return FALSE;
	}

	OldRatio = WSSRatio;
	OldFilmMode = WSSFilmMode;
	OldColorPlus = WSSColorPlus;
	OldHelperSignals = WSSHelperSignals;
	OldTeletextSubtitle = WSSTeletextSubtitle;
	OldOpenSubtitles = WSSOpenSubtitles;
	OldSurroundSound = WSSSurroundSound;
	OldCopyrightAsserted = WSSCopyrightAsserted;
	OldCopyProtection = WSSCopyProtection;

	for (i = 50 ; i < 200 ; i++)
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
		WSSNbDecodeOk++;
		WSSTotalPos += i;
		if (WSSNbDecodeOk == 1)
		{
			WSSMinPos = i;
			WSSMaxPos = i;
		}
		else
		{
			if (i < WSSMinPos)
				WSSMinPos = i;
			if (i > WSSMaxPos)
				WSSMaxPos = i;
		}
		WSSAvgPos = ROUND(((double)WSSTotalPos) / ((double)WSSNbDecodeOk));

		packedbits = 0;
		for (j = 0 ; j < WSS625_NB_DATA_BITS ; j++)
		{
			packedbits |= bits[j]<<j;
		}

		WSSRatio = packedbits & 0x000f;
		WSSFilmMode = (packedbits & 0x0010) ? TRUE : FALSE;
		WSSColorPlus = (packedbits & 0x0020) ? TRUE : FALSE;
		WSSHelperSignals = (packedbits & 0x0040) ? TRUE : FALSE;
		WSSTeletextSubtitle = (packedbits & 0x0100) ? TRUE : FALSE;
		WSSOpenSubtitles = packedbits & 0x0600;
		WSSSurroundSound = (packedbits & 0x0800) ? TRUE : FALSE;
		WSSCopyrightAsserted = (packedbits & 0x1000) ? TRUE : FALSE;
		WSSCopyProtection = (packedbits & 0x2000) ? TRUE : FALSE;
	}
	else
	{
		WSSNbDecodeErr++;
//		LOG("WSS nb decoding errors = %d", WSSNbDecodeErr);
	}

	if (OldRatio != WSSRatio)
	{
		switch (WSSRatio)
		{
		case WSS625_RATIO_133:
			SwitchToRatio(1,1333);
			LOG("WSS ratio (%x) : 1.33", WSSRatio);
			break;
		case WSS625_RATIO_177_ANAMORPHIC:
			SwitchToRatio(2,1778);
			LOG("WSS ratio (%x) : 1.77 anamorphic", WSSRatio);
			break;
		case WSS625_RATIO_155:
		case WSS625_RATIO_155_LETTERBOX_CENTER:
		case WSS625_RATIO_155_LETTERBOX_TOP:
			SwitchToRatio(1,1555);
			LOG("WSS ratio (%x) : 1.55 letterbox", WSSRatio);
			break;
		case WSS625_RATIO_177_LETTERBOX_CENTER:
		case WSS625_RATIO_177_LETTERBOX_TOP:
			SwitchToRatio(1,1778);
			LOG("WSS ratio (%x) : 1.77 letterbox", WSSRatio);
			break;
		case WSS625_RATIO_BIG_LETTERBOX_CENTER:
		default:
			LOG("WSS ratio (%x) : ???", WSSRatio);
			break;
		}
	}

	if (OldFilmMode != WSSFilmMode)
	{
		if (WSSFilmMode)
			LOG("WSS mode = film mode");
		else
			LOG("WSS mode = camera mode");
	}

	if (OldColorPlus != WSSColorPlus)
	{
		if (WSSColorPlus)
			LOG("WSS color encoding = Motion Adaptative ColorPlus");
		else
			LOG("WSS color encoding = normal Pal");
	}

	if (OldHelperSignals != WSSHelperSignals)
	{
		if (WSSHelperSignals)
			LOG("WSS helper signals = yes");
		else
			LOG("WSS helper signals = no");
	}

	if (OldTeletextSubtitle != WSSTeletextSubtitle)
	{
		if (WSSTeletextSubtitle)
			LOG("WSS teletext subtitles = yes");
		else
			LOG("WSS teletext subtitles = no");
	}

	if (OldOpenSubtitles != WSSOpenSubtitles)
	{
		switch (WSSOpenSubtitles)
		{
		case WSS625_SUBTITLE_NO:
			LOG("WSS open subtitles = no");
			break;
		case WSS625_SUBTITLE_INSIDE:
			LOG("WSS open subtitles = inside active picture");
			break;
		case WSS625_SUBTITLE_OUTSIDE:
			LOG("WSS open subtitles = outside active picture");
			break;
		default:
			LOG("WSS open subtitles = ???");
			break;
		}
	}

	if (OldSurroundSound != WSSSurroundSound)
	{
		if (WSSSurroundSound)
			LOG("WSS surround sound = yes");
		else
			LOG("WSS surround sound = no");
	}

	if (OldCopyrightAsserted != WSSCopyrightAsserted)
	{
		if (WSSCopyrightAsserted)
			LOG("WSS copyright asserted = yes");
		else
			LOG("WSS copyright asserted = no");
	}

	if (OldCopyProtection != WSSCopyProtection)
	{
		if (WSSCopyProtection)
			LOG("WSS copy protection = yes");
		else
			LOG("WSS copy protection = no");
	}

	return DecodeOk;
}

BOOL WSS525_DecodeLine(BYTE* vbiline)
{
	int		OldRatio;
	BOOL	DecodeOk = FALSE;

	OldRatio = WSSRatio;

	WSSRatio = OldRatio;

	if (OldRatio != WSSRatio)
	{
		switch (WSSRatio)
		{
		case WSS525_RATIO_133:
		case WSS525_RATIO_133_LETTERBOX:
			SwitchToRatio(1,1333);
			LOG("WSS ratio (%x) : 1.33", WSSRatio);
			break;
		case WSS525_RATIO_177_ANAMORPHIC:
			SwitchToRatio(2,1778);
			LOG("WSS ratio (%x) : 1.77 anamorphic", WSSRatio);
			break;
		default:
			LOG("WSS ratio (%x) : ???", WSSRatio);
			break;
		}
	}

	return DecodeOk;
}

int WSS_DecodeLine(BYTE* vbiline)
{
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
		return -1;
	}
	else
	{
		return 0;
	}
}
