/////////////////////////////////////////////////////////////////////////////
// VBI_WSSdecode.h
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1998 Laurent Garnier.  All rights reserved.
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
//	GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 11 Mar 2001   Laurent Garnier       New Header file
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __WSSDECODE_H___
#define __WSSDECODE_H___

#define WSS625_SUBTITLE_NO			0
#define WSS625_SUBTITLE_INSIDE		1
#define WSS625_SUBTITLE_OUTSIDE		2

#define	WSS_STATUS_OK				1
#define	WSS_STATUS_ERROR			0
#define	WSS_STATUS_PONCTUAL_ERROR	2

// WSS data
typedef struct 
{
	int		AspectRatio;
	int		AspectMode;
	BOOL	FilmMode;
	BOOL	ColorPlus;
	BOOL	HelperSignals;
	BOOL	TeletextSubtitle;
	int		OpenSubtitles;
	BOOL	SurroundSound;
	BOOL	CopyrightAsserted;
	BOOL	CopyProtection;
} TWSS_DataStruct;

extern TWSS_DataStruct WSS_Data;

// WSS control data
typedef struct 
{
	int	DecodeStatus;		// Status of last decoding
	int	NbDecodeErr;		// Number of decoding errors
	int	NbDecodeOk;			// Number of correct decoding
	int	NbSuccessiveErr;	// Number of successive decoding errors
	int	MinPos;
	int	MaxPos;
	int	TotalPos;
	int	NbErrPos;
	int	AspectRatioWhenErr;
	int	AspectModeWhenErr;
} TWSS_CtrlDataStruct;

extern TWSS_CtrlDataStruct WSS_CtrlData;

extern void WSS_init ();
extern int WSS_DecodeLine(BYTE* vbiline);
extern BOOL WSS_GetRecommendedAR (int* pMode, int* pRatio);

#endif
