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

// Possible ratio values for 625-line systems
#define	WSS625_RATIO_133					0x08
#define	WSS625_RATIO_155					0x0e
#define	WSS625_RATIO_177_ANAMORPHIC			0x07
#define	WSS625_RATIO_155_LETTERBOX_CENTER	0x01
#define	WSS625_RATIO_155_LETTERBOX_TOP		0x02
#define	WSS625_RATIO_177_LETTERBOX_CENTER	0x0b
#define	WSS625_RATIO_177_LETTERBOX_TOP		0x04
#define	WSS625_RATIO_BIG_LETTERBOX_CENTER	0x0d

#define WSS625_SUBTITLE_NO			0
#define WSS625_SUBTITLE_INSIDE		1
#define WSS625_SUBTITLE_OUTSIDE		2

// Possible ratio values for 525-line systems
#define	WSS525_RATIO_133					0x00
#define	WSS525_RATIO_177_ANAMORPHIC			0x01
#define	WSS525_RATIO_133_LETTERBOX			0x02

// Last WSS data decoded
extern int	WSSAspectRatio;
extern int	WSSAspectMode;
extern BOOL	WSSFilmMode;
extern BOOL	WSSColorPlus;
extern BOOL	WSSHelperSignals;
extern BOOL	WSSTeletextSubtitle;
extern int	WSSOpenSubtitles;
extern BOOL	WSSSurroundSound;
extern BOOL	WSSCopyrightAsserted;
extern BOOL	WSSCopyProtection;

extern BOOL	WSSDecodeOk;	// Status of last decoding
extern int	WSSNbDecodeErr;	// Number of decoding errors
extern int	WSSNbDecodeOk;	// Number of correct decoding
extern int	WSSMinPos;
extern int	WSSMaxPos;
extern int	WSSAvgPos;

int WSS_DecodeLine(BYTE* vbiline);

#endif
