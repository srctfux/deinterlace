/////////////////////////////////////////////////////////////////////////////
// Tuner.h
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
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
//
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 11 Aug 2000   John Adcock           Moved Tuner Functions in here
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __TUNER_H___
#define __TUNER_H___

BOOL Tuner_Init();
BOOL Tuner_SetFrequency(int wFrequency);
const char* Tuner_Status();
void Load_Country_Settings();
void Load_Country_Specific_Settings(int LPos);

typedef struct
{
    char Name[128];
} TCountries;

typedef struct
{
	char Name[128];
	int MinChannel;
	int MaxChannel;
	unsigned long freq[512];
} TChannels;

extern TCountries Countries[35];
extern TChannels Channels;

#endif
