/////////////////////////////////////////////////////////////////////////////
// Tuner.c
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
// Change Log
//
// Date          Developer             Changes
//
// 11 Aug 2000   John Adcock           Moved Tuner Functions in here
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "i2c.h"
#include "Tuner.h"

BYTE TunerDeviceWrite, TunerDeviceRead;
char TunerStatus[30];
TVTUNERID TunerType = TUNER_ABSENT;

struct TChannels Channels;

BOOL Tuner_Init(int TunerNr)
{
	unsigned char j;

	if (TunerNr == TUNER_ABSENT)
		return (TRUE);

	j = 0xc0;
	TunerDeviceRead = j;
	TunerDeviceWrite = j;

	while ((j <= 0xce) && (I2CBus_AddDevice((BYTE) j) == FALSE))
	{
		j++;
		TunerDeviceRead = j;
		TunerDeviceWrite = j;
	}

	if (j > 0xce)
	{
		return (FALSE);
	}
	sprintf(TunerStatus, "Tuner I2C-Bus I/O 0x%02x", j);
	return (TRUE);
}

/*
 *	Set TSA5522 synthesizer frequency 
 */
BOOL Tuner_SetFrequency(int TunerTyp, int wFrequency)
{
	BYTE config;
	WORD div;
	BOOL bAck;

	if (TunerTyp == TUNER_ABSENT)
		return (TRUE);

	if (wFrequency < Tuners[TunerTyp].thresh1)
		config = Tuners[TunerTyp].VHF_L;
	else if (wFrequency < Tuners[TunerTyp].thresh2)
		config = Tuners[TunerTyp].VHF_H;
	else
		config = Tuners[TunerTyp].UHF;

	div = wFrequency + Tuners[TunerTyp].IFPCoff;

	div &= 0x7fff;
	I2CBus_Lock();				// Lock/wait
	if (!I2CBus_Write((BYTE) TunerDeviceWrite, (BYTE) ((div >> 8) & 0x7f), (BYTE) (div & 0xff), TRUE))
	{
		Sleep(1);
		if (!I2CBus_Write((BYTE) TunerDeviceWrite, (BYTE) ((div >> 8) & 0x7f), (BYTE) (div & 0xff), TRUE))
		{
			Sleep(1);
			if (!I2CBus_Write((BYTE) TunerDeviceWrite, (BYTE) ((div >> 8) & 0x7f), (BYTE) (div & 0xff), TRUE))
			{
				ErrorBox("Tuner Device : Error Writing (1)");
				I2CBus_Unlock();	// Unlock
				return (FALSE);
			}
		}
	}
	if (!(bAck = I2CBus_Write(TunerDeviceWrite, Tuners[TunerTyp].config, config, TRUE)))
	{
		Sleep(1);
		if (!(bAck = I2CBus_Write(TunerDeviceWrite, Tuners[TunerTyp].config, config, TRUE)))
		{
			Sleep(1);
			if (!(bAck = I2CBus_Write(TunerDeviceWrite, Tuners[TunerTyp].config, config, TRUE)))
			{
				ErrorBox("Tuner Device : Error Writing (2)");
			}
		}
	}
	I2CBus_Unlock();			// Unlock
	if (!bAck)
		return FALSE;
	return TRUE;
}

void Load_Country_Settings()
{
	FILE *iniFile;
	char zeile[128];
	char *Pos;
	char *Pos1;
	char *SemmelPos;
	unsigned int i;

	for (i = 0; i < 32; i++)
	{
		Countries[i].Name[0] = 0x00;
	}

	if ((iniFile = fopen("Channel.lst", "r")) == NULL)
	{
		ErrorBox("File Channel.lst not Found");
		return;
	}
	i = 0;

	while (fgets(zeile, sizeof(zeile), iniFile) != NULL)
	{
		if (i >= 35)
		{
			ErrorBox("File Channel.lst has more than 35 settings!\nThe extra ones are ingnored.");
			fclose(iniFile);
			return;
		}
		SemmelPos = strstr(zeile, ";");
		if (SemmelPos == NULL)
			SemmelPos = strstr(zeile, "\n");

		if (((Pos = strstr(zeile, "[")) != 0) && (SemmelPos > Pos) && ((Pos1 = strstr(zeile, "]")) != 0))
		{

			Pos++;
			Pos1 = &Countries[i].Name[0];
			i++;
			while (*Pos != ']')
			{
				*(Pos1++) = *(Pos++);
				*Pos1 = 0x00;
			}
		}
	}

	fclose(iniFile);

}

void Load_Country_Specific_Settings(int LPos)
{
	unsigned short i, j, k;
	FILE *iniFile;
	char zeile[128];
	char txt[128];
	char *Pos;
	char *Pos1;
	char *SemmelPos;

	if ((iniFile = fopen("Channel.lst", "r")) == NULL)
	{
		ErrorBox("File Channel.lst not found");
		return;
	}
	i = 0;
	k = 0;

	while (fgets(zeile, sizeof(zeile), iniFile) != NULL)
	{

		SemmelPos = strstr(zeile, ";");
		if (SemmelPos == NULL)
			SemmelPos = strstr(zeile, "\n");

		sprintf(txt, "[%s]", Countries[LPos].Name);

		if (strstr(zeile, txt) != 0)
		{
			strcpy(Channels.Name, Countries[LPos].Name);
			while (fgets(zeile, sizeof(zeile), iniFile) != NULL)
			{
				SemmelPos = strstr(zeile, ";");
				if (SemmelPos == NULL)
					SemmelPos = strstr(zeile, "\n");

				if (((Pos = strstr(zeile, "[")) != 0) && (SemmelPos > Pos) && ((Pos1 = strstr(zeile, "]")) != 0))
				{
					fclose(iniFile);
					return;
				}

				if (((Pos = strstr(zeile, "KanalLow=")) != 0) && (SemmelPos > Pos))
				{
					Pos = Pos + 9;
					j = 0;
					txt[j] = 0x00;
					while (Pos < SemmelPos)
					{
						if (*Pos != 0x20)
						{
							txt[j] = *Pos;
							j++;
							txt[j] = 0x00;
						}
						Pos++;
					}
					Channels.MinChannel = atoi(txt);
				}
				else if (((Pos = strstr(zeile, "KanalHigh=")) != 0) && (SemmelPos > Pos))
				{
					Pos = Pos + 10;
					j = 0;
					txt[j] = 0x00;
					while (Pos < SemmelPos)
					{
						if (*Pos != 0x20)
						{
							txt[j] = *Pos;
							j++;
							txt[j] = 0x00;
						}
						Pos++;
					}
					Channels.MaxChannel = atoi(txt);
				}
				else
				{
					Pos = &zeile[0];
					j = 0;
					txt[j] = 0x00;
					while (Pos < SemmelPos)
					{
						if ((*Pos >= '0') && (*Pos <= '9'))
						{
							txt[j] = *Pos;
							j++;
							txt[j] = 0x00;
						}
						Pos++;
					}
					if (txt[0] != 0x00)
					{
						Channels.freq[k] = atol(txt);
						Channels.freq[k] = Channels.freq[k] / 1000;
						k++;
					}
				}
			}
		}
	}
	fclose(iniFile);
	return;
}
