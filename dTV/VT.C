/////////////////////////////////////////////////////////////////////////////
// vt.c
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
// 24 Jul 2000   John Adcock           Original Release
//                                     Translated most code from German
//                                     Combined Header files
//                                     Cut out all decoding
//                                     Cut out digital hardware stuff
//
/////////////////////////////////////////////////////////////////////////////

#include "resource.h"
#include "vt.h"
#include "other.h"

struct TVT VTFrame[800];

int SubPage=0;
struct TVDatBlockz VDATBlockz[8];

struct TInterCast InterCast;

struct TVDat VDat;

unsigned char vdat[10];

char VT_BASE_DIR[128];

BOOL VT_REVEAL = FALSE;
int  VT_LATIN = 1;
BOOL VT_NL = TRUE;
BOOL VT_HEADERS = TRUE;
BOOL VT_STRIPPED = TRUE;
BOOL VT_ALWAYS_EXPORT = FALSE;

HANDLE OutThread;
HANDLE VBI_Event=NULL;
BOOL bStopVBI;

int VBI_lpf = 16;   // lines per field
int VBI_bpl = 2048;   // bytes per line
BYTE VBI_thresh;
BYTE VBI_off;
BYTE VBI_vcbuf[25];
BYTE VBI_vc2buf[25];

BYTE VBI_CURRENT_MAG;
int VBI_CURRENT_PAGE=-1;
int VBI_CURRENT_SUB=-1;
BOOL VBI_CURRENT_PAGE_ERASE=FALSE;

int IC_BYTES_TOTAL=0;
char IC_BASE_DIR[128];
char VD_DIR[128];
char Current_VD_DIR[128];

unsigned int VBI_spos;

int VBI_FPS;

unsigned int vtstep, vpsstep, vdatstep;

char VPS_tmpName[9]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
char VPS_lastname[9]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
char VPS_chname[9]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
int VPS_namep=0;

unsigned char revham[16] = { 
  0x00, 0x08, 0x04, 0x0c, 0x02, 0x0a, 0x06, 0x0e,
  0x01, 0x09, 0x05, 0x0d, 0x03, 0x0b, 0x07, 0x0f };

unsigned char unhamtab[256] = {
  0x01, 0x0f, 0x01, 0x01, 0x0f, 0x00, 0x01, 0x0f, 
  0x0f, 0x02, 0x01, 0x0f, 0x0a, 0x0f, 0x0f, 0x07, 
  0x0f, 0x00, 0x01, 0x0f, 0x00, 0x00, 0x0f, 0x00, 
  0x06, 0x0f, 0x0f, 0x0b, 0x0f, 0x00, 0x03, 0x0f, 
  0x0f, 0x0c, 0x01, 0x0f, 0x04, 0x0f, 0x0f, 0x07, 
  0x06, 0x0f, 0x0f, 0x07, 0x0f, 0x07, 0x07, 0x07, 
  0x06, 0x0f, 0x0f, 0x05, 0x0f, 0x00, 0x0d, 0x0f, 
  0x06, 0x06, 0x06, 0x0f, 0x06, 0x0f, 0x0f, 0x07, 
  0x0f, 0x02, 0x01, 0x0f, 0x04, 0x0f, 0x0f, 0x09, 
  0x02, 0x02, 0x0f, 0x02, 0x0f, 0x02, 0x03, 0x0f, 
  0x08, 0x0f, 0x0f, 0x05, 0x0f, 0x00, 0x03, 0x0f, 
  0x0f, 0x02, 0x03, 0x0f, 0x03, 0x0f, 0x03, 0x03, 
  0x04, 0x0f, 0x0f, 0x05, 0x04, 0x04, 0x04, 0x0f, 
  0x0f, 0x02, 0x0f, 0x0f, 0x04, 0x0f, 0x0f, 0x07, 
  0x0f, 0x05, 0x05, 0x05, 0x04, 0x0f, 0x0f, 0x05, 
  0x06, 0x0f, 0x0f, 0x05, 0x0f, 0x0e, 0x03, 0x0f, 
  0x0f, 0x0c, 0x01, 0x0f, 0x0a, 0x0f, 0x0f, 0x09, 
  0x0a, 0x0f, 0x0f, 0x0b, 0x0a, 0x0a, 0x0a, 0x0f, 
  0x08, 0x0f, 0x0f, 0x0b, 0x0f, 0x00, 0x0d, 0x0f, 
  0x0f, 0x0b, 0x0b, 0x0b, 0x0a, 0x0f, 0x0f, 0x0b, 
  0x0c, 0x0c, 0x0f, 0x0c, 0x0f, 0x0c, 0x0d, 0x0f, 
  0x0f, 0x0c, 0x0f, 0x0f, 0x0a, 0x0f, 0x0f, 0x07, 
  0x0f, 0x0c, 0x0d, 0x0f, 0x0d, 0x0f, 0x0d, 0x0d, 
  0x06, 0x0f, 0x0f, 0x0b, 0x0f, 0x0e, 0x0d, 0x0f, 
  0x08, 0x0f, 0x0f, 0x09, 0x0f, 0x09, 0x09, 0x09, 
  0x0f, 0x02, 0x0f, 0x0f, 0x0a, 0x0f, 0x0f, 0x09, 
  0x08, 0x08, 0x08, 0x0f, 0x08, 0x0f, 0x0f, 0x09, 
  0x08, 0x0f, 0x0f, 0x0b, 0x0f, 0x0e, 0x03, 0x0f, 
  0x0f, 0x0c, 0x0f, 0x0f, 0x04, 0x0f, 0x0f, 0x09, 
  0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0e, 0x0f, 0x0f, 
  0x08, 0x0f, 0x0f, 0x05, 0x0f, 0x0e, 0x0d, 0x0f, 
  0x0f, 0x0e, 0x0f, 0x0f, 0x0e, 0x0e, 0x0f, 0x0e, 
};

int VT_Cache=0;

BYTE VT_Header_Line[40];



void Start_VBI()
{
	//DWORD LinkThreadID;

	if (VBI_Event == NULL)
	{
		VBI_Event = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	ResetEvent(VBI_Event);

	bStopVBI = FALSE;
	ResetEvent(VBI_Event);
	//CloseHandle(CreateThread((LPSECURITY_ATTRIBUTES) NULL, (DWORD) 0, DoVBI, NULL, (DWORD) 0, (LPDWORD) & LinkThreadID));
}

void Stop_VBI()
{
	bStopVBI = TRUE;
	SetEvent(VBI_Event);
	Sleep(20);
	bStopVBI = TRUE;
	SetEvent(VBI_Event);
	CloseHandle(VBI_Event);
	VBI_Event = NULL;
}

DWORD WINAPI DoVBI(LPVOID lpThreadParameter)
{
	BYTE *pVBI;
	DWORD VBI_Tic_Count = 0;
	int vbi_frames;
	int line;
	int norm = 0;
	double vtfreq;
	double f = 0.0;
	double vpsfreq = 5.0;		// does NTSC have VPS???
	double vcfreq = 0.77;
	double freq;
	double vdatfreq = 2.0;

	ProzessorMask = 1 << (VBIProzessor);
	SetThreadAffinityMask(GetCurrentThread(), ProzessorMask);

	vtfreq = norm ? 5.72725 : 6.9375;

	/* if no frequency given, use standard ones for Bt848 and PAL/NTSC */
	if (f == 0.0)
		freq = norm ? 28.636363 : 35.468950;
	else
		freq = f;

	vtstep = (int) ((freq / vtfreq) * FPFAC + 0.5);
	/* VPS is shift encoded, so just sample first "state" */
	vpsstep = 2 * (int) ((freq / vpsfreq) * FPFAC + 0.5);
	vdatstep = (int) ((freq / vdatfreq) * FPFAC + 0.5);

	vbi_frames = 0;
	VBI_Tic_Count = GetTickCount();

	while(!bStopVBI)
	{
		vbi_frames++;
		if (WaitForSingleObject(VBI_Event, INFINITE) != WAIT_OBJECT_0)
			continue;
		ResetEvent(VBI_Event);
		if (!VideoPresent)
			continue;

		pVBI = (LPBYTE) Vbi_dma[CurrentFrame]->dwUser;

		for (line = 0; line < VBI_lpf * 2; line++)
		{
			VBI_decode_line(pVBI + line * 2048, line);
		}

		if (VBI_Tic_Count + 960 <= GetTickCount())
		{
			VBI_FPS = vbi_frames + 1;
			if (VBI_FPS > 25)
				VBI_FPS = 25;
			vbi_frames = 0;
			VBI_Tic_Count = GetTickCount();
		}

	}
	ExitThread(0);
	return 0;
}

void VBI_decode_line(unsigned char *VBI_Buffer, int line)
{
	unsigned char data[45];
	int i, p;
	BOOL VDat_Load;

	if (line >= VBI_lpf)
		line -= VBI_lpf;

	VBI_AGC(VBI_Buffer, 120, 450, 1);

	/* all kinds of data with videotext data format: videotext, intercast, ... */
	if (((VBI_Flags & VBI_VT) || (VBI_Flags & VBI_IC)) && (line < 16))
	{
		// search for first 1 bit (VT always starts with 55 55 27 !!!)
		p = 50;
		while ((VBI_Buffer[p] < VBI_thresh) && (p < 350))
			p++;
		VBI_spos = (p << FPSHIFT) + vtstep / 2;

		/* ignore first bit for now */
		data[0] = VBI_Scan(VBI_Buffer, vtstep);
		//cout << HEX(2) << (int)data[0] << endl;
		if ((data[0] & 0xfe) == 0x54)
		{
			data[1] = VBI_Scan(VBI_Buffer, vtstep);
			switch (data[1])
			{
			case 0xd5:			/* oops, missed first 1-bit: backup 2 bits */
				VBI_spos -= 2 * vtstep;
				data[1] = 0x55;
			case 0x55:
				data[2] = VBI_Scan(VBI_Buffer, vtstep);
				switch (data[2])
				{
				case 0xd8:		/* this shows up on some channels?!?!? */
					for (i = 3; i < 45; i++)
					{
						data[i] = VBI_Scan(VBI_Buffer, vtstep);
					}
					return;
				case 0x27:
					for (i = 3; i < 45; i++)
					{
						data[i] = VBI_Scan(VBI_Buffer, vtstep);
					}
					VBI_decode_vt(data);
					return;
				default:
					break;
				}
			default:
				break;
			}
		}
	}

	/* VPS information with channel name, time, VCR programming info, etc. */
	if ((VBI_Flags & VBI_VPS) && (line == 9))
	{
		p = 150;
		while ((VBI_Buffer[p] < VBI_thresh) && (p < 260))
			p++;
		p += 2;
		VBI_spos = p << FPSHIFT;
		if ((data[0] = VBI_Scan(VBI_Buffer, vpsstep)) != 0xff)
			return;
		if ((data[1] = VBI_Scan(VBI_Buffer, vpsstep)) != 0x5d)
			return;
		for (i = 2; i < 16; i++)
		{
			data[i] = VBI_Scan(VBI_Buffer, vpsstep);
		}
		VBI_decode_vps(data);
	}

	/* Video_Dat_Stuff  */
	if ((VBI_Flags & VBI_VD) && ((line == 17) || (line == 18)))
	{
		p = 100;
		VBI_off = 0;
		while ((VBI_Buffer[p] < 100) && (p < 260))
			p++;
		if (p < 200)
		{
			VBI_spos = p << FPSHIFT;
			VBI_spos += vdatstep / 2;
			VDat_Load = TRUE;
			for (i = 0; i < 10; i++)
			{
				if (VBI_VDatScan(VBI_Buffer, vdatstep, i) != 0)
				{
					strcpy(VDat.Error, "Kein StartBit gefunden");
					VDat_Load = FALSE;
					if (VDat.Index > 0)
						VideoDat_Flush();
					break;
				}
			}					// 10 Bytes geladen

			if (VDat_Load == TRUE)
			{
				if (VD_RAW == TRUE)
				{
					VDat.BlocksOK++;
					strcpy(VDat.Error, "                    ");
					for (i = 0; i < 10; i++)
						VDat.XDATA[VDat.Index++] = vdat[i];
					if (VDat.Index > 1000)
						VideoDat_Flush();
				}
				else
					Work_VideoDat(vdat);
			}
			else
			{
				if (VDat.Index > 0)
					VideoDat_Flush();
				VDat.BlocksError++;
			}

		}
		else
		{
			strcpy(VDat.Error, "Kein Videodat-Signal gefunden");
			if (VDat.Index > 0)
				VideoDat_Flush();
			VDat.BlocksError = 0;
			VDat.BlocksOK = 0;

		}
	}
}

void VBI_decode_vps(unsigned char *data)
{

	unsigned char *info;

	info = data;
	if ((info[3] & 0x80))
	{
		VPS_chname[VPS_namep] = 0;
		if (VPS_namep == 8)
		{
			if (strcpy(VPS_chname, VPS_tmpName) == 0)
				memcpy(VPS_lastname, VPS_chname, 9);	// VPS-Channel-Name
			strcpy(VPS_tmpName, VPS_chname);
		}
		VPS_namep = 0;
	}
	VPS_chname[VPS_namep++] = info[3] & 0x7f;
	if (VPS_namep >= 9)
		VPS_namep = 0;
	if (ShowVPSInfo != NULL)
		SetDlgItemText(ShowVPSInfo, TEXT1, VPS_lastname);
}

// VIDEODAT // 
int VBI_VDatScan(BYTE * VBI_Buffer, unsigned int step, int BytePos)
{
	int j;

	/* check for start bit */
	if (!((VBI_Buffer[VBI_spos >> FPSHIFT] + VBI_off) & 0x80))
		return -1;

	/* Resync Glaube ich nicht notwendig 
	   for (j=0; j<(step>>FPSHIFT); j++)
	   if (!((VBI_Buffer[(VBI_spos>>FPSHIFT)-j]+VBI_off)&0x80))
	   break;
	   VBI_spos=VBI_spos-(j<<FPSHIFT)+step/2;
	 */
	VBI_spos += step;
	vdat[BytePos] = 0x00;

	for (j = 7; j >= 0; j--)
	{
		vdat[BytePos] |= (((VBI_Buffer[VBI_spos >> FPSHIFT] + VBI_off) & 0x80) >> j);
		VBI_spos += step;
	}
	/* check for stop bit */
	if (!((VBI_Buffer[VBI_spos >> FPSHIFT] + VBI_off) & 0x80))
	{
		// kein StoppBit gefunden 
		// Return -1 ???? ( Als nix machen ) ???
	}

	VBI_spos += step;
	vdat[BytePos] ^= 0xff;
	return 0;
}

void Work_VideoDat(unsigned char *Buffer)
{
	int i, j;
	unsigned short CheckSum = 0;

	VDat.BlocksOK++;

	for (i = 0; i < 10; i++)
	{
		if (VDat.Status == 0)
		{
			if (Buffer[i] == 0x56)
				VDat.Status = 1;
			VDat.LastKillPos = 0;
		}
		else if (VDat.Status == 1)
		{
			if (Buffer[i] == 0xAE)
				VDat.Status = 2;
			else
				VDat.Status = 0;
		}
		else if (VDat.Status == 2)
		{
			VDat.ADR = Buffer[i];
			if (VDat.ADR != 0xff)
				VDat.Status = 3;
			else
				VDat.Status = 0;
		}
		else if (VDat.Status == 3)
		{
			VDat.SYS = Buffer[i];
			VDat.Interleave = (Buffer[i] & 0x70) / 16;
			//Ermittelt die Blocklaenge...

			switch (Buffer[i] & 0x0F)
			{
			case 0:
				VDat.Lenght = 1;
				break;
			case 1:
				VDat.Lenght = 2;
				break;
			case 2:
				VDat.Lenght = 4;
				break;
			case 3:
				VDat.Lenght = 8;
				break;
			case 4:
				VDat.Lenght = 16;
				break;
			case 5:
				VDat.Lenght = 32;
				break;
			case 6:
				VDat.Lenght = 64;
				break;
			case 7:
				VDat.Lenght = 128;
				break;
			case 8:
				VDat.Lenght = 256;
				break;
			case 9:
				VDat.Lenght = 512;
				break;
			}
			VDat.Lenght -= 1;
			VDat.Status = 4;
		}
		else if (VDat.Status == 4)
		{
			VDat.LBN = Buffer[i];
			VDat.Status = 5;
		}
		else if (VDat.Status == 5)
		{
			VDat.KEY = Buffer[i];
			VDat.Pos = 0;
			memset(&VDat.XDATA[0], 0x00, 512);
			VDat.Status = 6;
		}
		else if (VDat.Status == 6)
		{
			VDat.XDATA[VDat.Pos++] = Buffer[i];
			//56 AE FF Folge erkennen...
			if (VDat.XDATA[VDat.Pos - 1] == 0xff)
			{
				if (VDat.Pos > 2)
				{
					if (VDat.XDATA[VDat.Pos - 2] == 0xAE)
					{
						if (VDat.XDATA[VDat.Pos - 3] == 0x56)
						{
							if (VDat.LastKillPos != (VDat.Pos - 1))
							{
								//Dann muss das FF entfernt werden, aber nur wenn
								//nicht schon ein FF weg gemacht wurde.!!!!
								VDat.Pos--;
//                                  VDat.Lenght--;
								VDat.LastKillPos = VDat.Pos;
							}
						}
					}
				}
			}
			if (VDat.Pos < VDat.Lenght)
				VDat.Status = 6;
			else
				VDat.Status = 7;
		}
		else if (VDat.Status == 7)
		{
			VDat.CSL = Buffer[i];
			VDat.Status = 8;
		}
		else if (VDat.Status == 8)
		{
			VDat.CSH = Buffer[i];
			CheckSum = 0;
			CheckSum += VDat.ADR;
			CheckSum += VDat.SYS;
			CheckSum += VDat.LBN;
			CheckSum += VDat.KEY;
			for (j = 0; j < VDat.Lenght; j++)
				CheckSum += VDat.XDATA[j];

			VDat.BlockSoFar++;
			if ((VDat.CSL == (CheckSum & 0x00FF)) && (VDat.CSH == ((CheckSum & 0xFF00) >> 8)))
			{
				VDat.Fehler_Frei = TRUE;
			}
			else
			{
				VDat.CRCError++;
				VDat.Fehler_Frei = FALSE;
			}

			if (VDat.KEY <= 4)
			{					/* Seit 0.02 Weg! || (LastInter != Block.interleave) */
				if (VDat.Blockzaehler < 8)
				{
					VDATBlockz[VDat.Blockzaehler].KEY = VDat.KEY;
					VDATBlockz[VDat.Blockzaehler].LBN = VDat.LBN;
					VDATBlockz[VDat.Blockzaehler].Fehler_Frei = VDat.Fehler_Frei;
					for (j = 0; j < VDat.Lenght; j++)
						VDATBlockz[VDat.Blockzaehler].XDATA[j] = VDat.XDATA[j];
					VDATBlockz[VDat.Blockzaehler].Lenght = VDat.Lenght;
					VDat.Blockzaehler++;
				}
				else
				{
					VBI_VDat_Blockaustausch();
					VDat.Blockzaehler = 0;
					VDATBlockz[VDat.Blockzaehler].KEY = VDat.KEY;
					VDATBlockz[VDat.Blockzaehler].LBN = VDat.LBN;
					VDATBlockz[VDat.Blockzaehler].Fehler_Frei = VDat.Fehler_Frei;
					for (j = 0; j < VDat.Lenght; j++)
						VDATBlockz[VDat.Blockzaehler].XDATA[j] = VDat.XDATA[j];
					VDATBlockz[VDat.Blockzaehler].Lenght = VDat.Lenght;
					VDat.Blockzaehler++;
				}
			}

			if (VDat.KEY == 0)
				VDat.eLBN = 0x00;
//      VDat.Fehler_Frei = FALSE;
			VDat.Status = 0;
		}
	}
}

void VBI_VDat_Blockaustausch()
{
	int i, j, k, z;

	for (i = 0; i < 8; i++)
	{
		if (VDATBlockz[i].Fehler_Frei == FALSE)
		{
			for (j = 0; j < 8; j++)
			{
				if (VDATBlockz[i].LBN == VDATBlockz[j].LBN)
				{
					VDATBlockz[i].KEY = VDATBlockz[j].KEY;
					VDATBlockz[i].LBN = VDATBlockz[j].LBN;
					VDATBlockz[i].Fehler_Frei = VDATBlockz[j].Fehler_Frei;
					VDATBlockz[i].Lenght = VDATBlockz[j].Lenght;
					for (k = 0; k < VDATBlockz[i].Lenght; k++)
						VDATBlockz[i].XDATA[k] = VDATBlockz[j].XDATA[k];
				}
			}
		}
	}
	// Nach dem Decodeing alle KEYs auf 99 setzten...
	if (VBI_VDAT_DecodeBlockz() == 0)
		for (z = 0; z < 8; z++)
			VDATBlockz[z].KEY = 99;
}

int VBI_VDAT_DecodeBlockz()
{
	//Decodiert die Datenbloecke...
	int i;
	char FileName[255];

	for (i = 0; i < 8; i++)
	{
		if (VDat.eLBN == VDATBlockz[i].LBN)
		{

			switch (VDATBlockz[i].KEY)
			{
			case 0:			//Start of Transmission
				VBI_VDAT_SOTInfo(VDATBlockz[i], &SOTInfoRec);	//Neu !!
				VDat.eLBN++;
				break;
			case 1:			//End of Transmission
				VDat.eLBN++;
				break;
			case 2:			//Start of File
				VBI_VDat_Filename(VDATBlockz[i], &VDat.FileName[0]);
				VDat.FileSize = VBI_VDat_FileSize(VDATBlockz[i]);
				sprintf(FileName, "%s\\%s", Current_VD_DIR, VDat.FileName);
				VDat.FilePtr = fopen(FileName, "wb");
				if (VDat.FilePtr == NULL)
				{
					strcpy(VDat.Error, "VideoDat-Datei kann nicht zum schreiben geöffnet werden");
				}
				VDat.eLBN++;
				break;
			case 3:			//End of File
				if (VDat.FilePtr != NULL)
					fclose(VDat.FilePtr);
				VDat.eLBN++;
				break;
			case 4:			//DATA
				if (VDat.FilePtr != NULL)
				{

					if (VBI_VDat_WriteData(VDATBlockz[i]) == FALSE)
					{
						fclose(VDat.FilePtr);
						strcpy(VDat.Error, "Schreibfehler bei der VideoDat-Datei");
					}
				}
				VDat.eLBN++;
				break;
			}
		}
	}
	return (0);
}

 //Ermittelt den Dateinamen...
void VBI_VDat_Filename(struct TVDatBlockz Block, char *fNames)
{
	char fName[13];
	int t = 0;

	while (t != 13)
	{
		fName[t] = Block.XDATA[t + 13];
		t++;
		fName[t] = 0x00;
		if (Block.XDATA[t + 13] == 0x00)
			t = 13;
	}
	fName[12] = 0x00;
	strcpy(fNames, fName);
}

unsigned int VBI_VDat_FileSize(struct TVDatBlockz Block)
{
	unsigned char a, b, c, d;
	unsigned int value;

	a = Block.XDATA[4];
	b = Block.XDATA[5];
	c = Block.XDATA[6];
	d = Block.XDATA[7];

	value = 0;
	value += d << 24;			/// BOCK in eurer Software !! //Warning ignorieren, wird sowieso bald geändert!
	value += c << 16;
	value += b << 8;
	value += a;

	return (value);
}

BOOL VBI_VDat_WriteData(struct TVDatBlockz Block)
{
	//Daten sichern...
	long BytesToWrite;

	fseek(VDat.FilePtr, 0L, SEEK_END);
	BytesToWrite = ftell(VDat.FilePtr);

	if ((BytesToWrite + 255) > (signed int) VDat.FileSize)
		BytesToWrite = VDat.FileSize - BytesToWrite;
	else
		BytesToWrite = 255;

	if (fwrite(Block.XDATA, 1, (size_t) BytesToWrite, VDat.FilePtr) != (unsigned int) BytesToWrite)
		return (FALSE);
	return (TRUE);
}

void VBI_VDAT_SOTInfo(struct TVDatBlockz Block, struct SOTREC *Info)
{
	unsigned char sizeGN;		//Länge des GeneralNames...
	unsigned char sizeEC;		//Länge des Exec-Komandos...
	unsigned char sizeKO;		//Länge des Kommentars...
	FILE *txtFile;
	char FName[255];

	Info->date = (unsigned int) Block.XDATA[0];	//Murks Murks... Date.
	Info->fanz = (unsigned short) Block.XDATA[4];	//Murks Murks... Anzahl der Dateien.

	sizeGN = Block.XDATA[6];
	strncpy(Info->GeneralName, &Block.XDATA[7], sizeGN);
	Info->GeneralName[sizeGN] = 0x00;

	sizeEC = Block.XDATA[6 + sizeGN + 1];
	if (sizeEC != 0)
	{
		strncpy(Info->ExecCommand, &Block.XDATA[7 + sizeGN + 1], sizeEC);
		Info->ExecCommand[sizeEC] = 0x00;
	}

	sizeKO = Block.XDATA[6 + sizeGN + 1 + sizeEC + 1];
	if (sizeKO != 0)
	{
		strncpy(Info->Kommentar, &Block.XDATA[7 + sizeGN + 1 + sizeEC + 1], sizeKO);
		Info->Kommentar[sizeKO] = 0x00;
	}

	//Verzeichnis Anlegen, in das wir schreiben wollen...
	sprintf(Current_VD_DIR, "%s//%s", VD_DIR, Info->GeneralName);
	CreateDirectory(Current_VD_DIR, NULL);

	sprintf(FName, "%s//VideoDat.txt", Current_VD_DIR);
	if ((txtFile = fopen(FName, "w")) == NULL)
	{
		strcpy(Current_VD_DIR, VD_DIR);
		sprintf(FName, "%s//VideoDat.txt", Current_VD_DIR);
		if ((txtFile = fopen(FName, "w")) == NULL)
			return;
	}

	fprintf(txtFile, "***********************************************************************\n");
	fprintf(txtFile, "\n");
	fprintf(txtFile, "                     dTV - VideoDat\n");
	fprintf(txtFile, "\n");
	fprintf(txtFile, "***********************************************************************\n");
	fprintf(txtFile, "\n");
	fprintf(txtFile, "Datum der Sendung             : %d \n", Info->date);
	fprintf(txtFile, "Anzahl der Dateien            : %d \n", Info->fanz);
	fprintf(txtFile, "Name des Verzechnisses        : %s \n", Info->GeneralName);
	fprintf(txtFile, "Auszuführendes Kommando       : %s \n", Info->ExecCommand);
	fprintf(txtFile, "Kommentar                     : %s \n", Info->Kommentar);
	fprintf(txtFile, "\n");
	fclose(txtFile);
}

void VideoDat_Flush(void)
{
	FILE *sFile;
	char FileName[255];

	if (VDat.Index > 0)
	{
		sprintf(FileName, "%s//%s", VD_DIR, VDat.RawName);

		sFile = fopen(FileName, "ab");
		if (sFile == NULL)
		{
			strcpy(VDat.Error, "Datei kann zum schreiben nicht geöffnet werden");
			VDat.Index = 0;
			return;
		}

		if (fwrite(VDat.XDATA, 1, VDat.Index, sFile) != (unsigned int) VDat.Index)
		{
			strcpy(VDat.Error, "Datei kann nicht geschrieben werden");
		}
		else
			strcpy(VDat.Error, "");

		fclose(sFile);
		VDat.Index = 0;
	}
}

unsigned char VBI_Scan(BYTE * VBI_Buffer, unsigned int step)
{
	int j;
	unsigned char dat;

	for (j = 7, dat = 0; j >= 0; j--, VBI_spos += step)
		dat |= ((VBI_Buffer[VBI_spos >> FPSHIFT] + VBI_off) & 0x80) >> j;
	return dat;
}

// unham 2 bytes into 1, report 2 bit errors but ignore them
unsigned char unham(unsigned char *d)
{
	unsigned char c1, c2;

	c1 = unhamtab[d[0]];
	c2 = unhamtab[d[1]];
	return (c2 << 4) | (c1);
}

// unham, but with reversed nibble order for VC
unsigned char unham2(unsigned char *d)
{
	unsigned char c1, c2;

	c1 = unhamtab[d[0]];
	c2 = unhamtab[d[1]];
	return (c1 << 4) | (c2);
}

void InterCast_Exit(void)
{

	if (InterCast.fbuf != NULL)
	{
		free(InterCast.fbuf);
		InterCast.fbuf = NULL;
	}
}

void InterCast_Init(void)
{
	InterCast.fbuf = NULL;
	InterCast.fbufSize = 0;
	InterCast.esc = 0;
	InterCast.done = 0;
	InterCast.pnum = 0;
	InterCast.ok = 0;
	InterCast.datap = 0;
	InterCast.lastci = 0xff;
}

void VideoDat_Init(void)
{
	VDat.Pos = 0;
	VDat.eLBN = 0x00;
	VDat.CRCError = 0;
	VDat.BlockSoFar = 0;
	VDat.Lenght = 0;
	VDat.Interleave = 0;
	VDat.Blockzaehler = 0;
	VDat.Status = 0;
	VDat.LastKillPos = 0;
	VDat.Pos = 0;
	VDat.FileName[0] = 0x00;
	VDat.FileSize = 0;
	VDat.FilePtr = NULL;
	VDat.Index = 0;
	VDat.BlocksError = 0;
	VDat.BlocksOK = 0;
	strcpy(Current_VD_DIR, VD_DIR);
}

void VideoDat_Exit(void)
{
	VideoDat_Flush();
	VDat.BlocksError = 0;
	VDat.BlocksOK = 0;
}

void VBI_IC_adata(unsigned char c)
{
	if (InterCast.ok)
	{
		if (InterCast.datap >= 0x100)
			InterCast.ok = 0;
		else
			InterCast.data[InterCast.datap++] = c;
	}
}

void VBI_IC_Procpack()
{

	unsigned char *pack;
	char name[255];
	FILE *sFile;
	unsigned char *Save;

	if (!InterCast.ok)
		return;
	InterCast.ok = 0;
	InterCast.length = InterCast.datap - 10;

	if (InterCast.length > (unsigned int) InterCast.datap || InterCast.length != InterCast.data[6])
	{
		strcpy(InterCast.Error, "Packet-Längen Fehler");
		InterCast.BytesSoFar = 0;
		return;
	}
	//    hdump10 (data,7);
	//   hdump10 (data+7,datap-10); cout << endl;
	InterCast.ci = InterCast.data[4];
	if (InterCast.data[2])
	{
		pack = InterCast.data + 7;
		if (!InterCast.ci)
		{
			/* XXX: this works on little endian only!!! */
			InterCast.packnum = *((unsigned int *) (pack + 0x0c));
			InterCast.packtot = *((unsigned int *) (pack + 0x10));
			InterCast.packlen = *((unsigned int *) (pack + 0x14));
			InterCast.total = *((unsigned int *) (pack + 0x18));
			InterCast.BytesSoFar += InterCast.packlen;

			if (InterCast.packnum == 1)
			{
				InterCast.lastci = 0x0f;
				InterCast.done = 0;
				/* I keep the whole file in memory for now */
				/* go ahead and change this but note that you will need to buffer
				   the last 2 packages since name and other information might
				   be on a package boundary! 
				 */
				if (InterCast.fbufSize < (signed) InterCast.total)
				{
					Save = InterCast.fbuf;
					InterCast.fbufSize = InterCast.total;
					InterCast.fbuf = (unsigned char *) realloc((void *) InterCast.fbuf, InterCast.total);
					if (InterCast.fbuf == NULL)
					{
						strcpy(InterCast.Error, "Memory Realloc Fail");
						free(Save);
						InterCast_Exit();
						InterCast_Init();
						return;
					}
				}
			}
			if (InterCast.lastci == 0x0f)
				InterCast.ciok = 1;
			InterCast.lastci = 0;
			if (InterCast.ciok && (InterCast.length > 0x38))
			{
				memcpy(InterCast.fbuf + InterCast.done, pack + 0x38, InterCast.length - 0x38);
				InterCast.done += InterCast.length - 0x38;
			}
		}
		else
		{
			if (InterCast.lastci + 1 != InterCast.ci)
				InterCast.ciok = 0;
			InterCast.lastci = InterCast.ci;
			if (InterCast.ciok)
			{
//    LOG(4,cout << ".";cout.flush());
				memcpy(InterCast.fbuf + InterCast.done, pack, InterCast.length);
				InterCast.done += InterCast.length;
			}
			else
			{
				//   LOG(4,cout << "x";cout.flush(););
			}
		}
		if (InterCast.total && (InterCast.done == InterCast.total))
		{
			unsigned int npos, ipos;
			int nlen;

			/* At fbuf+total-6 is a pointer to an info structure */
			ipos = *((unsigned int *) (InterCast.fbuf + InterCast.total - 6));
			InterCast.flength = *((unsigned int *) (InterCast.fbuf + ipos + 2));

			nlen = (int) InterCast.fbuf[ipos + 0x34];
			if (InterCast.fbuf[ipos + 0x32] & 0x80)
				npos = ipos + 0x35;
			else
				npos = *((unsigned int *) (InterCast.fbuf + ipos + 0x38));
			/* Only save the file if pointers pass some sanity checks */
			if (npos + nlen < InterCast.total && ipos < InterCast.total && InterCast.flength <= InterCast.total)
			{
				strncpy(name, (char *) InterCast.fbuf + npos, nlen);
				name[nlen] = 0;	// just to be sure ...
				sprintf(InterCast.fname, "%s\\%s", IC_BASE_DIR, name);
				InterCast.BytesSoFar = 0;
				sFile = fopen(InterCast.fname, "wb");
				if (sFile == NULL)
				{
					strcpy(InterCast.Error, "Datei kann zum schreiben nicht geöffnet werden");
				}
				else
				{
					if (fwrite(InterCast.fbuf, 1, InterCast.flength, sFile) != InterCast.flength)
					{
						strcpy(InterCast.Error, "Datei kann nicht geschrieben werden");
					}
					else
						strcpy(InterCast.Error, "");

					fclose(sFile);
				}

			}
			else
			{
				InterCast.BytesSoFar = 0;
				strcpy(InterCast.Error, "Fehler bei IC-CrossCheck");
			}
			InterCast.done = 0;
			InterCast.total = 0;
			InterCast.ciok = 0;
			InterCast.lastci = 0xff;
		}

	}
}

void VBI_IC_Procbyte(unsigned char c)
{
	if (!InterCast.esc)
	{
		// 0x10 escapes 0x02, 0x03 and itself
		if (c == 0x10)
		{
			InterCast.esc = 1;
			return;
		}
		// unescaped 0x02 starts a package
		if (c == 0x02)
		{
			InterCast.datap = 0;
			InterCast.ok = 1;
		}
		VBI_IC_adata(c);
		// unescaped 0x03 ends a package
		if (c == 0x03)
			VBI_IC_Procpack();
	}
	else
	{
		InterCast.esc = 0;
		VBI_IC_adata(c);
	}
}

void VBI_IC_Procblocks(void)
{
	int i, j;
	unsigned char *block;

	for (i = 0; i < 14; i++)
	{
		block = InterCast.blocks + 28 * i;
		for (j = 0; j < 26; j++)
			VBI_IC_Procbyte(block[j]);
	}
	memset(InterCast.blocks, 0, 16 * 28);
}

void VBI_IC_Setblock(int nr, unsigned char *block)
{
	memcpy(InterCast.blocks + 28 * nr, block, 28);
	if (nr == 0x0f)
		VBI_IC_Procblocks();
}

void VBI_AGC(BYTE * Buffer, int start, int stop, int step)
{
	int i, min = 255, max = 0;

	for (i = start; i < stop; i += step)
	{
		if (Buffer[i] < min)
			min = Buffer[i];
		else if (Buffer[i] > max)
			max = Buffer[i];
	}
	VBI_thresh = (max + min) / 2;
	VBI_off = 128 - VBI_thresh;
}

void VBI_decode_vt(unsigned char *dat)
{
	int i, FL, NR;
	unsigned char mag, pack, mpag, ftal, ft, al, page;
	unsigned int addr;
	unsigned int pnum = 0;
	unsigned short sub;
	int j;
	WORD ctrl;
	static unsigned char amag=0x00;


	struct TVTPage *Save;
	int nPage;
	int nPage1;

	/* dat: 55 55 27 %MPAG% */
	mpag = unham(dat + 3);
	mag = mpag & 7;

	pack = (mpag >> 3) & 0x1f;

	if (ShowVTInfo != NULL)
	{
		if (amag != mag)
		{
			SetDlgItemInt(ShowVTInfo, TEXT3, mag, FALSE);
			amag = mag;
		}
	}

	switch (pack)
	{
	case 0:
		if (VBI_Flags & VBI_VT)
		{

			//hdump(udat,4); cout << " HD\n";

			/* dat: 55 55 27 %MPAG% %PAGE% %SUB% 
			   00 01 02  03 04  05 06 07-0a 
			 */
/*ALT
			
	page=unham(dat+5);
    if ( page == 0x9f ) break;
    sub=(unham(dat+9)<<8)|unham(dat+7);
    if ( mag == 0 ) mag=8;
    nPage=(page/16);
	nPage1=page-(nPage*16);
	pnum=100*mag+nPage*10+nPage1;

*/
// NEU
			page = unham(dat + 5);

			if (page == 0x9f)
				break;

			if (mag == 0)
				mag = 8;
			nPage = (page / 16);

			if (nPage > 10)
				break;			//DF um ungültige Seiten zu verhindern (z.B. f1 bei 251)

			nPage1 = page - (nPage * 16);

			if (nPage1 > 10)
				break;			//DF um ungültige Seiten zu verhindern
			pnum = 100 * mag + nPage * 10 + nPage1;

			sub = (unham(dat + 9) << 8) | unham(dat + 7);
// ENDE NEU

			VBI_CURRENT_MAG = mag;

			if (sub & 0x80)
				VBI_CURRENT_PAGE_ERASE = TRUE;
			else
				VBI_CURRENT_PAGE_ERASE = FALSE;

			if (sub & 0x8000)
			{
				i = 0;
				while ((i < UTCount) && (i < 8) && (UTPages[i] != pnum))
					i++;
				if (i < 12)
				{
					if (i >= UTCount)
					{
						UTPages[UTCount] = pnum;
						UTCount++;
					}
				}
			}

			j = sub;
			sub = sub & 0x3F;
			nPage = (sub / 16);
			nPage1 = sub - (nPage * 16);

			sub = nPage * 10 + nPage1;

			if (ShowVTInfo != NULL)
			{
				SetDlgItemInt(ShowVTInfo, TEXT1, VT_Cache, FALSE);
				SetDlgItemInt(ShowVTInfo, TEXT2, pnum, FALSE);
				SetDlgItemInt(ShowVTInfo, TEXT4, sub, FALSE);
				SetDlgItemInt(ShowVTInfo, TEXT5, j, FALSE);
			}

			pnum -= 100;

			if ((pnum >= 0) && (pnum < 800))
			{

				ctrl = (unhamtab[dat[3]] & 0x7) + ((unhamtab[dat[8]] >> 3) << 3) + ((unhamtab[dat[10]] >> 2) << 4) + (unhamtab[dat[11]] << 6) + (unhamtab[dat[12]] << 10);

				VBI_CURRENT_PAGE = pnum;
				if (sub > 0)
					sub--;
				VBI_CURRENT_SUB = sub;

				if (VTFrame[VBI_CURRENT_PAGE].SubCount == 0)
				{
					if (sub < 64)
					{
						VTFrame[VBI_CURRENT_PAGE].SubCount = sub + 1;
						VTFrame[VBI_CURRENT_PAGE].SubPage = calloc(sub + 1, sizeof(struct TVTPage));

						if (VTFrame[VBI_CURRENT_PAGE].SubPage == NULL)
							break;
						VT_Cache = VT_Cache + sub + 1;
						for (i = 0; i < sub + 1; i++)
						{
							VTFrame[VBI_CURRENT_PAGE].SubPage[i].Fill = FALSE;
							VTFrame[VBI_CURRENT_PAGE].SubPage[i].bUpdated = 0x00;
							for (j = 0; j < 25; j++)
								VTFrame[VBI_CURRENT_PAGE].SubPage[i].LineUpdate[j] = 0;

						}
					}
				}

				if ((sub < 64) && (VTFrame[VBI_CURRENT_PAGE].SubCount < sub))
				{
					Save = VTFrame[VBI_CURRENT_PAGE].SubPage;
					VTFrame[VBI_CURRENT_PAGE].SubPage = realloc(Save, sizeof(struct TVTPage) * (sub + 1));

					if (VTFrame[VBI_CURRENT_PAGE].SubPage == NULL)
					{
						VTFrame[VBI_CURRENT_PAGE].SubPage = Save;
						break;
					}

					for (i = VTFrame[VBI_CURRENT_PAGE].SubCount; i < sub + 1; i++)
					{
						VTFrame[VBI_CURRENT_PAGE].SubPage[i].Fill = FALSE;
						VTFrame[VBI_CURRENT_PAGE].SubPage[i].bUpdated = 0x00;
						memset(&VTFrame[VBI_CURRENT_PAGE].SubPage[i].LineUpdate[0], 0, 25);
					}
					VT_Cache = VT_Cache + sub + 1 - VTFrame[VBI_CURRENT_PAGE].SubCount;
					VTFrame[VBI_CURRENT_PAGE].SubCount = sub + 1;
				}

				if (VTFrame[VBI_CURRENT_PAGE].SubCount > sub)
				{
					VTFrame[VBI_CURRENT_PAGE].SubPage[sub].wCtrl = ctrl;
					if (VBI_CURRENT_PAGE_ERASE == TRUE)
					{
						memset(&VTFrame[VBI_CURRENT_PAGE].SubPage[sub].Frame[1], 0x00, 24 * 40);
					}
					memcpy(&VTFrame[VBI_CURRENT_PAGE].SubPage[sub].Frame[0], dat + 5, 40);
					memcpy(&VT_Header_Line[0], dat + 5, 40);
					VTFrame[VBI_CURRENT_PAGE].SubPage[sub].bUpdated = 1;
					VTFrame[VBI_CURRENT_PAGE].SubPage[sub].Fill = TRUE;
				}
			}
			else
			{
				if (ShowVTInfo != NULL)
				{
					SetDlgItemInt(ShowVTInfo, TEXT7, i + 100, FALSE);
				}

			}
		}
		break;

	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
	case 19:
	case 20:
	case 21:
	case 22:
	case 23:
	case 24:

/* ALT
	  if (VBI_Flags&VBI_VT ) {

	  if (( VBI_CURRENT_PAGE >= 0 ) && ( VBI_CURRENT_PAGE < 800 ) &&
		( VBI_CURRENT_SUB >= 0 ) && ( VBI_CURRENT_SUB < 64 ) && 
		  ( VBI_CURRENT_MAG == mag )) {
		  if ( VTFrame[VBI_CURRENT_PAGE].SubCount > VBI_CURRENT_SUB ) {
		memcpy(&VTFrame[VBI_CURRENT_PAGE].SubPage[VBI_CURRENT_SUB].Frame[pack],dat+5,40);
	    VTFrame[VBI_CURRENT_PAGE].SubPage[VBI_CURRENT_SUB].bUpdated=1;
		VTFrame[VBI_CURRENT_PAGE].SubPage[VBI_CURRENT_SUB].Fill=TRUE;
		VTFrame[VBI_CURRENT_PAGE].SubPage[VBI_CURRENT_SUB].LineUpdate[pack]=1;
        if ( pack == 24 ) VBI_CURRENT_PAGE = -1; 
		}
	}
}
*/
// NEU
		if (VBI_Flags & VBI_VT)
		{

			if ((VBI_CURRENT_PAGE >= 0) && (VBI_CURRENT_PAGE < 800) && (VBI_CURRENT_SUB >= 0) && (VBI_CURRENT_SUB < 64) && (VBI_CURRENT_MAG == mag))
			{
				if (VTFrame[VBI_CURRENT_PAGE].SubCount > VBI_CURRENT_SUB)
				{

					memcpy(&VTFrame[VBI_CURRENT_PAGE].SubPage[VBI_CURRENT_SUB].Frame[pack], dat + 5, 40);

					//DF...
					if (pack == 1)
					{
//                  FILE *f;
						int n, subs;
						char t[50], tmp[50];

						//f=fopen("c:\\temp\\sub.txt", "a+");
						memcpy(t, dat + 5, 40);

						for (n = 0; n < 40; n++)
							tmp[n] = (t[n] & 0x7f);
						tmp[n] = 0;
						//fprintf(f, "[%d] frame[%d]=%40s\n", VBI_CURRENT_PAGE, pack, tmp);
						//fprintf(f, "%3s --> %d", tmp+36, atoi(tmp+38));
						//fclose(f);

// Noch mehr Subpages? 
						subs = atoi(tmp + 38);	// direkt aus dem Videotext 2. Zeile die Anzahl der Subpages lesen

						if ((subs > 0) && (subs < 64) && (VTFrame[VBI_CURRENT_PAGE].SubCount < subs))
						{		//DF??? <
							Save = VTFrame[VBI_CURRENT_PAGE].SubPage;
							VTFrame[VBI_CURRENT_PAGE].SubPage = realloc(Save, sizeof(struct TVTPage) * (subs));

							if (VTFrame[VBI_CURRENT_PAGE].SubPage == NULL)
							{
								VTFrame[VBI_CURRENT_PAGE].SubPage = Save;
								break;
							}

							for (i = VTFrame[VBI_CURRENT_PAGE].SubCount; i < subs; i++)
							{
								VTFrame[VBI_CURRENT_PAGE].SubPage[i].Fill = FALSE;
								VTFrame[VBI_CURRENT_PAGE].SubPage[i].bUpdated = 0x00;
								memset(&VTFrame[VBI_CURRENT_PAGE].SubPage[i].LineUpdate[0], 0, 25);
							}
							VT_Cache = VT_Cache + subs - VTFrame[VBI_CURRENT_PAGE].SubCount;
							VTFrame[VBI_CURRENT_PAGE].SubCount = subs;
						}		// if subs>SubCount
					}			// if (pack==1)

					VTFrame[VBI_CURRENT_PAGE].SubPage[VBI_CURRENT_SUB].bUpdated = 1;
					VTFrame[VBI_CURRENT_PAGE].SubPage[VBI_CURRENT_SUB].Fill = TRUE;
					VTFrame[VBI_CURRENT_PAGE].SubPage[VBI_CURRENT_SUB].LineUpdate[pack] = 1;
					if (pack == 24)
						VBI_CURRENT_PAGE = -1;
				}
			}
		}

// ENDE NEU

		//    vtch.setline(dat+5, pack, mag);
		break;
	case 25:
		/*    cout << "page " << HEX(4) << int(pnum)  << " ";
		   cout << "AltHeader:"; adump(dat+5,40); cout << "\n";
		 */
		break;
	case 26:					// PDC 
	case 27:
	case 28:
	case 29:
		break;
	case 30:
		StorePacket30(dat);
		//    LOG(16,cout << "pack 30\n";)
		break;
	case 31:
		//cout << "mag" << int(mag) << "\n";
		ftal = unham(dat + 5);
		al = ftal >> 4;			/* address length */
		ft = ftal & 0x0f;
		for (addr = 0, i = 0; i < al; i++)
			addr = (addr << 4) | unhamtab[dat[7 + i]];

		switch (addr)
		{
		case 0x07:
			//cout << " ??:" << HEX(2) << int(0xf&unhamtab[dat[9+al]]);

			break;

		case 0x0500:
			if (VBI_Flags & VBI_IC)
			{
				/* here we usually have the following structure: (03-0e are hammed)
				   dat: 00 01 02  03 04  05 06  07-0c    0d  0e  0f-2a 2b-2c 
				   SYNC    MPAG    FTAL   ADDR    NR  FL   DATA   CRC  
				   55 55 27   31    00 06  000500   1-f 08     
				 */
				FL = (int) unhamtab[dat[8 + al]];	/* flags? */
				NR = (int) unhamtab[dat[7 + al]];	/* line number */

				VBI_IC_Setblock(NR, dat + 9 + al);
			}

			break;
		case 0x0f00:			/* also used by ZDF and DSF, data format unknown */
			break;
		default:
			break;
		}
/*
    LOG(2,
	cout << "ICH:\n";  
//	hdump10(dat+9+al,40-al-4); 
	hdump10(dat,45); 
	cout << "\n";
	);
*/
		break;
	default:
		VBI_CURRENT_PAGE = -1;
		//    cout << "Packet " << dec << int(pack) << "\n";
		break;
	}
}

void StorePacket30(BYTE * p)
{
	DWORD d, b;
	BYTE h, m, s, a, CNI0, CNI1, CNI2, CNI3;
	char Text[20];
	int n;

	if (*p != 0x55)
		return;					// Some error, the data should be here...
	p += 5;

	if (ShowPDCInfo != NULL)
	{

	}

	if (unhamtab[*p] == 0)		// TSDP
	{
		p++;
		Packet30.HomePage.nPage = unham(p);
		Packet30.HomePage.nSubcode = (((unhamtab[p[5]] & 0x3) << 12) + (unhamtab[p[4]] << 8) + ((unhamtab[p[3]] & 0x7) << 4) + unhamtab[p[2]]);

		Packet30.HomePage.nMag = ((unhamtab[p[5]] >> 1) & 0x6) + ((unhamtab[p[3]] >> 3) & 0x1);
		p += 6;
		Packet30.NetId = (p[1] << 8) + p[0];
		p += 2;
		Packet30.UTC.Offset = ((*p >> 1) & 0x1f) * (*p & 0x40) ? -1 : 1;
		p++;
		d = (((*p) - 0x01) << 16) + (((*(p + 1)) - 0x11) << 8) + ((*(p + 2)) - 0x11);
		Packet30.UTC.JulianDay = (((d & 0xF0000) >> 16) * 10000) + (((d & 0x0F000) >> 12) * 1000) + (((d & 0x00F00) >> 8) * 100) + (((d & 0x000F0) >> 4) * 10) + (d & 0x0000F);
		p += 3;
		h = (*p) - 0x11;
		m = (*(p + 1)) - 0x11;
		s = (*(p + 2)) - 0x11;
		Packet30.UTC.Hour = (h >> 4) * 10 + (h & 0x0f);
		Packet30.UTC.Min = (m >> 4) * 10 + (m & 0x0f);
		Packet30.UTC.Sec = (s >> 4) * 10 + (s & 0x0f);
		p += 3;
		for (n = 0; n < 4; n++)
			Packet30.Unknown[n] = p[n] & 0x7f;
		Packet30.Unknown[n] = '\0';
		p += 4;
		for (n = 0; n < 20; n++)
			Packet30.Identifier[n] = p[n] & 0x7f;
		Packet30.Identifier[n] = '\0';

		if (ShowPDCInfo != NULL)
		{
			SetDlgItemText(ShowPDCInfo, TEXT1, Packet30.Identifier);
			SetDlgItemText(ShowPDCInfo, TEXT2, Packet30.Unknown);
			sprintf(Text, "%d", Packet30.NetId);
			SetDlgItemText(ShowPDCInfo, TEXT3, Text);
			sprintf(Text, "%d", Packet30.HomePage.nMag);
			SetDlgItemText(ShowPDCInfo, TEXT4, Text);
			sprintf(Text, "%d", Packet30.HomePage.nPage);
			SetDlgItemText(ShowPDCInfo, TEXT5, Text);
			sprintf(Text, "%d", Packet30.HomePage.nSubcode);
			SetDlgItemText(ShowPDCInfo, TEXT6, Text);
			sprintf(Text, "%02x", Packet30.UTC.Offset);
			SetDlgItemText(ShowPDCInfo, TEXT7, Text);
			sprintf(Text, "%d", Packet30.UTC.JulianDay);
			SetDlgItemText(ShowPDCInfo, TEXT8, Text);
			sprintf(Text, "%02d", Packet30.UTC.Hour);
			SetDlgItemText(ShowPDCInfo, TEXT9, Text);
			sprintf(Text, "%02d", Packet30.UTC.Min);
			SetDlgItemText(ShowPDCInfo, TEXT10, Text);
			sprintf(Text, "%02d", Packet30.UTC.Sec);
			SetDlgItemText(ShowPDCInfo, TEXT11, Text);
		}
	}
	else if (unhamtab[*p] == 2)	// PDC
	{
		p++;
		Packet30.HomePage.nPage = unham(p);
		Packet30.HomePage.nSubcode = (((unhamtab[p[5]] & 0x3) << 12) + (unhamtab[p[4]] << 8) + ((unhamtab[p[3]] & 0x7) << 4) + unhamtab[p[2]]);

		Packet30.HomePage.nMag = ((unhamtab[p[5]] >> 1) & 0x6) + ((unhamtab[p[3]] >> 3) & 0x1);
		p += 6;
		a = revham[unhamtab[p[0]]];
		Packet30.PDC.LCI = GetBit(a, 2, 3);
		Packet30.PDC.LUF = GetBit(a, 1, 1);
		Packet30.PDC.PRF = GetBit(a, 0, 1);
		a = revham[unhamtab[p[1]]];
		Packet30.PDC.PCS = GetBit(a, 2, 3);
		Packet30.PDC.MI = GetBit(a, 1, 1);
		CNI0 = revham[unhamtab[p[2]]];
		b = (revham[unhamtab[p[3]]] << 28) + (revham[unhamtab[p[4]]] << 24) +
			(revham[unhamtab[p[5]]] << 20) + (revham[unhamtab[p[6]]] << 16) +
			(revham[unhamtab[p[7]]] << 12) + (revham[unhamtab[p[8]]] << 8) + (revham[unhamtab[p[9]]] << 4) + (revham[unhamtab[p[10]]]);
		CNI2 = GetBit(b, 30, 3);
		Packet30.PDC.day = GetBit(b, 25, 0x1f);
		Packet30.PDC.month = GetBit(b, 21, 0xf);
		Packet30.PDC.hour = GetBit(b, 16, 0x1f);
		Packet30.PDC.minute = GetBit(b, 10, 0x3f);
		CNI1 = GetBit(b, 6, 0xf);
		CNI3 = GetBit(b, 0, 0x3f);
		Packet30.PDC.PTY = (revham[unhamtab[p[11]]] << 4) + revham[unhamtab[p[12]]];;
		Packet30.PDC.CNI = (CNI0 << 12) + (CNI1 << 8) + (CNI2 << 4) + CNI3;
		p += 13;
		for (n = 0; n < 20; n++)
			Packet30.Identifier[n] = p[n] & 0x7f;
		Packet30.Identifier[n] = '\0';

		if (ShowPDCInfo != NULL)
		{
			sprintf(Text, "%d", Packet30.PDC.LCI);
			SetDlgItemText(ShowPDCInfo, TEXT12, Text);
			sprintf(Text, "%d", Packet30.PDC.LUF);
			SetDlgItemText(ShowPDCInfo, TEXT13, Text);
			sprintf(Text, "%d", Packet30.PDC.PRF);
			SetDlgItemText(ShowPDCInfo, TEXT14, Text);
			sprintf(Text, "%d", Packet30.PDC.PCS);
			SetDlgItemText(ShowPDCInfo, TEXT15, Text);
			sprintf(Text, "%d", Packet30.PDC.MI);
			SetDlgItemText(ShowPDCInfo, TEXT16, Text);
			sprintf(Text, "%d", Packet30.PDC.day);
			SetDlgItemText(ShowPDCInfo, TEXT17, Text);
			sprintf(Text, "%d", Packet30.PDC.month);
			SetDlgItemText(ShowPDCInfo, TEXT18, Text);
			sprintf(Text, "%d", Packet30.PDC.hour);
			SetDlgItemText(ShowPDCInfo, TEXT19, Text);
			sprintf(Text, "%d", Packet30.PDC.minute);
			SetDlgItemText(ShowPDCInfo, TEXT20, Text);
			sprintf(Text, "%d", Packet30.PDC.CNI);
			SetDlgItemText(ShowPDCInfo, TEXT21, Text);
			sprintf(Text, "%d", Packet30.PDC.PTY);
			SetDlgItemText(ShowPDCInfo, TEXT22, Text);
		}
	}
}

int VT_GetPage(int nPage, BOOL * ErsteSub, int *SubNr)
{
	int i;

	if (nPage < 100 || nPage > 899)
		return (-1);
	if (VTFrame[nPage - 100].SubCount == 0)
		return (-1);
	if (*ErsteSub == TRUE)
	{
		i = 0;
		while ((i < VTFrame[nPage - 100].SubCount) && (VTFrame[nPage - 100].SubPage[i].Fill == FALSE))
			i++;
		*SubNr = i;
	}
	*ErsteSub = FALSE;
	return nPage - 100;
}

void VT_DoUpdate_Page(int Page, int SubPage, int Dialog, BOOL bForceUpdate, int VTDisplayMode, BOOL Large)
{

	BOOL bGraph, bHoldGraph, bSepGraph, bBox, bFlash, bDouble, bConceal, bHasDouble;
	BYTE nLastGraph;
	unsigned short CurrentFg, CurrentBkg, RealBkg;
	WORD wCharHeight, wCharWidth;
	BYTE c, ch;
	int endrow;
	int n, row, x, y;
	char tmp[41];
	BITMAPINFO *pCharSet;
	char tmp2[9];
	int VT_Bitmap_width;

	BYTE *src, *dest;
	unsigned short *dest1;

	BOOL bHideTopLine, bForceShowTopLine;
	unsigned short Black, ForceBlack, ForceTransparent;

	if (VTFrame[Page].SubCount == 0)
	{
		Black = VTColourTable[0];	// 
		bHideTopLine = FALSE;
		ForceTransparent = VTColourTable[8];
		ForceBlack = VTColourTable[0];
	}
	else
	{

		if ((VTFrame[Page].SubPage[SubPage].wCtrl & (3 << 4)) || VTDisplayMode == 1)
			Black = VTColourTable[8];
		else
			Black = VTColourTable[0];	// 0

		ForceTransparent = VTColourTable[8];
		ForceBlack = VTColourTable[0];

		if (VTFrame[Page].SubPage[SubPage].wCtrl & (3 << 4))
			bHideTopLine = TRUE;
		else
			bHideTopLine = FALSE;
	}

	if (Large == TRUE)
	{
		wCharWidth = LARGE_WIDTH;
		wCharHeight = LARGE_HEIGHT;
		pCharSet = VTCharSetLarge;
		VT_Bitmap_width = VT_LARGE_BITMAP_WIDTH;
	}
	else
	{
		wCharWidth = SMALL_WIDTH;
		wCharHeight = SMALL_HEIGHT;
		pCharSet = VTCharSetSmall;
		VT_Bitmap_width = VT_SMALL_BITMAP_WIDTH;
	}

	bForceShowTopLine = TRUE;

	bHasDouble = FALSE;
	endrow = 25;

	if (VTFrame[Page].SubCount == 0)
	{
		endrow = 1;
	}
	else
	{
		if (VTFrame[Page].SubPage[SubPage].bUpdated == 0)
			endrow = 1;
	}

	for (row = 0; row < endrow; row++)
	{
		if (bHasDouble)
		{
			bHasDouble = FALSE;
			continue;
		}
		bGraph = bHoldGraph = bSepGraph = bBox = bFlash = bDouble = bConceal = bHasDouble = FALSE;
		nLastGraph = 32;

		if (VTFrame[Page].SubCount == 0)
		{

			sprintf(tmp2, "  P%-3d \x7", VTDialog[Dialog].Page);
			for (n = 0; n < 40; n++)
			{
				tmp[n] = VT_Header_Line[n] & 0x7f;
				if (tmp[n] == 0x0d)
					bHasDouble = TRUE;
				strncpy(tmp, tmp2, 8);
			}

		}
		else
		{

			for (n = 0; n < 40; n++)
			{
				tmp[n] = VTFrame[Page].SubPage[SubPage].Frame[row][n] & 0x7f;
				if (tmp[n] == 0x0d)
					bHasDouble = TRUE;
			}
			tmp[n] = '\0';

			if (row == 0)
			{
				sprintf(tmp2, "  P%-3d \x7", VTDialog[Dialog].Page);
				strncpy(tmp, tmp2, 8);
				if (VTFrame[Page].SubPage[SubPage].Fill == FALSE)
				{
					for (n = 0; n < 40; n++)
					{
						tmp[n] = VT_Header_Line[n] & 0x7f;
						if (tmp[n] == 0x0d)
							bHasDouble = TRUE;
						strncpy(tmp, tmp2, 8);
					}
				}
				else
				{
					for (n = 30; n < 40; n++)
					{
						tmp[n] = VT_Header_Line[n] & 0x7f;
						if (tmp[n] == 0x0d)
							bHasDouble = TRUE;
					}

				}
			}
		}

		RealBkg = ForceBlack;
		if (Page != 0)
		{
			if (row == 0 && ((VTFrame[Page].SubCount == 0) || (bForceShowTopLine || (!bHideTopLine && VTDisplayMode != 1))))
				CurrentBkg = ForceBlack;
			else
				CurrentBkg = Black;

			if ((bHideTopLine && !bForceShowTopLine) && row == 0 && (VTFrame[Page].SubCount > 0))
				memset(tmp, 32, 40);
		}
		else
			CurrentBkg = ForceBlack;

		CurrentFg = VTColourTable[7];
		for (n = 0; n < 40; n++)
		{
			c = tmp[n];
			ch = c;
			if (c < 32)
			{
				if (c < 8)
				{
					CurrentFg = VTColourTable[c];
					bGraph = FALSE;
				}
				if (c >= 0x10 && c <= 0x17)
				{
					bGraph = TRUE;
					CurrentFg = VTColourTable[c - 0x10];
				}
				if (c == 0x1d)
				{
					CurrentBkg = (bBox || Black != ForceTransparent) ? CurrentFg : ForceTransparent;
					RealBkg = CurrentFg;
				}
				if (c == 0x1c)
				{
					CurrentBkg = bBox ? ForceBlack : Black;
					RealBkg = ForceBlack;
				}
				if (c == 0x08)
					bFlash = TRUE;
				if (c == 0x09)
					bFlash = FALSE;
				if (c == 0x0c)
					bDouble = FALSE;
				if (c == 0x0d)
					bDouble = TRUE;
				if (c == 0x18)
					bConceal = TRUE;
				if (c == 0x19)
					bSepGraph = FALSE;
				if (c == 0x1a)
					bSepGraph = TRUE;
				if (c == 0x1e)
					bHoldGraph = TRUE;
				if (c == 0x1f)
					bHoldGraph = FALSE;
				ch = bHoldGraph ? nLastGraph : 32;
			}
			if ((CurrentFg == VTColourTable[7]) && (VTFrame[Page].SubCount == 0) && (!row && n > 7))
				CurrentFg = VTColourTable[2];
			nLastGraph = 32;
			if (bGraph && (ch & 0x20))
			{
				nLastGraph = ch;
				ch = (ch & 0x1f) | ((ch & 0x40) >> 1);
				ch += 96;
				if (bSepGraph)
					ch += 64;
			}
			else
				ch -= 32;

			if ((ch >= 16) && (ch <= 25))
				VTDialog[Dialog].AsciiBuffer[row][n] = ch + 32;
			else
				VTDialog[Dialog].AsciiBuffer[row][n] = 0x00;

			if (bDouble == TRUE)
			{
				if ((ch >= 16) && (ch <= 25))
					VTDialog[Dialog].AsciiBuffer[row + 1][n] = ch + 32;
				else
					VTDialog[Dialog].AsciiBuffer[row + 1][n] = 0x00;
			}

			if (Large == TRUE)
				src = _BitmapLargeChar(pCharSet, ch);
			else
				src = _BitmapSmallChar(pCharSet, ch);
			dest = _BitmapDataP(VTScreen[Dialog]);
			dest += VTScreen[Dialog]->bmiHeader.biSizeImage;
			dest -= ((40 - n) * (wCharWidth * 2)) + ((wCharWidth / 2) * 2);
			dest -= (row * VT_Bitmap_width * wCharHeight * 2);

			for (y = 0; y < ((bDouble && (row < 24)) ? wCharHeight * 2 : wCharHeight); y++)
			{
				if (!bDouble || (bDouble && (!(y & 1))))
					src -= ROUNDUP(pCharSet->bmiHeader.biWidth);

				for (x = 0; x < wCharWidth; x++)
				{
					(unsigned char *) dest1 = (dest + x * 2);
					*(dest1) = (unsigned short) (*(src + x) ? CurrentFg : CurrentBkg);
				}
				dest -= VT_Bitmap_width * 2;

			}
			if (!bDouble && bHasDouble && row < 24)
			{
				for (y = 0; y < wCharHeight; y++)
				{
					for (x = 0; x < wCharWidth; x++)
					{
						(unsigned char *) dest1 = (dest + x * 2);
						*(dest1) = CurrentBkg;
					}
					dest -= VT_Bitmap_width * 2;
				}

			}

			if (c < 32)
			{
				if (c == 0x0a)	// Box off
				{
					CurrentBkg = (Black == ForceTransparent) ? ForceTransparent : RealBkg;
					bBox = FALSE;
				}
				if (c == 0x0b)	// Box on
				{
					CurrentBkg = RealBkg;
					bBox = TRUE;
				}
			}
		}
	}
}

void VT_ChannelChange()
{
	int i;

	for (i = 0; i < 12; i++)
		UTPages[i] = 0;
	UTCount = 0;

	if (VBI_Flags & VBI_VT)
	{
		VBI_Flags -= VBI_VT;
		Sleep(2);
		Sleep(0);
		Sleep(2);
		Sleep(0);
		Sleep(2);
		Sleep(2);
		Sleep(0);
		Sleep(2);
		Sleep(0);
		Sleep(2);
		Sleep(2);
		Sleep(0);
		Sleep(2);
		Sleep(0);
		Sleep(2);
	}
	else
		return;

	for (i = 0; i < 800; i++)
	{
		VTFrame[i].SubCount = 0;
		if (VTFrame[i].SubPage != NULL)
			free(VTFrame[i].SubPage);
		VTFrame[i].SubPage = NULL;
	}
	VT_Cache = 0;
	VBI_Flags += VBI_VT;
}

BOOL APIENTRY VideoTextProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{

	int Slot;
	int i;
	HDC wHDC;					/* display-context variable  */
	PAINTSTRUCT wps;			/* paint structure           */
	RECT rect;
	int x, y;
	char Buffer[10];
	char Text[40];

	switch (message)
	{
	case WM_INITDIALOG:

		SetDlgItemInt(hDlg, IDC_EDIT1, 100, FALSE);
		Slot = New_Dialog_Slot(hDlg);
		if (Slot < 0)
		{
			MessageBox(hWnd, "All Videotext Dialogs occupied", "dTV", MB_ICONSTOP | MB_OK);
			EndDialog(hDlg, 0);
			return (TRUE);

		}

		if (VTScreen[Slot] != NULL)
			free(VTScreen[Slot]);
		VTScreen[Slot] = NULL;

		if (VTLarge == TRUE)
		{
			VTScreen[Slot] = (BITMAPINFO *) calloc(1, sizeof(BITMAPINFOHEADER) + sizeof(WORD) * 256 + VT_LARGE_BITMAP_WIDTH * 2 * VT_LARGE_BITMAP_HEIGHT);
			VTScreen[Slot]->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			VTScreen[Slot]->bmiHeader.biWidth = VT_LARGE_BITMAP_WIDTH;
			VTScreen[Slot]->bmiHeader.biHeight = VT_LARGE_BITMAP_HEIGHT;
			VTScreen[Slot]->bmiHeader.biPlanes = 1;
			VTScreen[Slot]->bmiHeader.biBitCount = 16;
			VTScreen[Slot]->bmiHeader.biCompression = BI_RGB;
			VTScreen[Slot]->bmiHeader.biSizeImage = VTScreen[Slot]->bmiHeader.biWidth * VTScreen[Slot]->bmiHeader.biHeight * VTScreen[Slot]->bmiHeader.biBitCount / 8;
			VTScreen[Slot]->bmiHeader.biXPelsPerMeter = 0;
			VTScreen[Slot]->bmiHeader.biYPelsPerMeter = 0;
			VTScreen[Slot]->bmiHeader.biClrUsed = 0;
			VTScreen[Slot]->bmiHeader.biClrImportant = 0;
			VTDialog[Slot].Large = TRUE;
		}
		else
		{
			VTScreen[Slot] = (BITMAPINFO *) calloc(1, sizeof(BITMAPINFOHEADER) + sizeof(WORD) * 256 + VT_SMALL_BITMAP_WIDTH * 2 * VT_SMALL_BITMAP_HEIGHT);
			VTScreen[Slot]->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			VTScreen[Slot]->bmiHeader.biWidth = VT_SMALL_BITMAP_WIDTH;
			VTScreen[Slot]->bmiHeader.biHeight = VT_SMALL_BITMAP_HEIGHT;
			VTScreen[Slot]->bmiHeader.biPlanes = 1;
			VTScreen[Slot]->bmiHeader.biBitCount = 16;
			VTScreen[Slot]->bmiHeader.biCompression = BI_RGB;
			VTScreen[Slot]->bmiHeader.biSizeImage = VTScreen[Slot]->bmiHeader.biWidth * VTScreen[Slot]->bmiHeader.biHeight * VTScreen[Slot]->bmiHeader.biBitCount / 8;
			VTScreen[Slot]->bmiHeader.biXPelsPerMeter = 0;
			VTScreen[Slot]->bmiHeader.biYPelsPerMeter = 0;
			VTScreen[Slot]->bmiHeader.biClrUsed = 0;
			VTScreen[Slot]->bmiHeader.biClrImportant = 0;
			VTDialog[Slot].Large = FALSE;
		}
		VTDialog[Slot].SubPage = 0;
		VTDialog[Slot].PageChange = TRUE;
		SetTimer(hDlg, 1, 333, NULL);
		return (TRUE);

	case WM_TIMER:
		if (wParam != 1)
			return (TRUE);

		sprintf(Text, "VT-Cache %d", VT_Cache);
		SetDlgItemText(hDlg, IDC_TEXT21, Text);

		Slot = Get_Dialog_Slot(hDlg);
		if (Slot < 0)
		{
			MessageBox(hDlg, "Error :Unknown Videotext Dialog", "dTV", MB_ICONSTOP | MB_OK);
			EndDialog(hDlg, 0);
			return (TRUE);
		}
		sprintf(Text, "%02d/%02d", VTDialog[Slot].SubPage + 1, VTFrame[VTDialog[Slot].FramePos].SubCount);
		SetDlgItemText(hDlg, IDC_TEXT20, Text);

		if ((VTDialog[Slot].Page >= 100) && (VTDialog[Slot].Page <= 899))
		{

			VTDialog[Slot].FramePos = VT_GetPage(VTDialog[Slot].Page, &VTDialog[Slot].PageChange, &VTDialog[Slot].SubPage);

			if ((VTDialog[Slot].FramePos >= 0) && (VTFrame[VTDialog[Slot].FramePos].SubPage[VTDialog[Slot].SubPage].bUpdated != 0))
			{

				VT_DoUpdate_Page(VTDialog[Slot].FramePos, VTDialog[Slot].SubPage, Slot, TRUE, 0, VTDialog[Slot].Large);
				VTFrame[VTDialog[Slot].FramePos].SubPage[VTDialog[Slot].SubPage].bUpdated = 0;
				sprintf(Text, "Seite %d gefunden", VTDialog[Slot].Page);
				SetDlgItemText(hDlg, IDC_TEXT, Text);
				SetDlgItemText(hDlg, IDC_EDIT1, "");

				sprintf(Text, "dTV VT %d", VTDialog[Slot].Page);
				SetWindowText(hDlg, Text);

				rect.top = 50;
				rect.left = 10;
				rect.bottom = 501;
				rect.right = 637;
				InvalidateRect(hDlg, &rect, TRUE);
			}
			else
			{
				if (VTDialog[Slot].FramePos < 0)
				{
					VT_DoUpdate_Page(VTDialog[Slot].Page - 100, 0, Slot, TRUE, 0, VTDialog[Slot].Large);
				}
				else
					VT_DoUpdate_Page(VTDialog[Slot].FramePos, VTDialog[Slot].SubPage, Slot, TRUE, 0, VTDialog[Slot].Large);
				rect.top = 50;
				rect.left = 10;
				rect.bottom = 70;
				rect.right = 637;
				InvalidateRect(hDlg, &rect, TRUE);

			}

		}
		break;

	case IDM_VT_PAGE_MINUS:
		Slot = Get_Dialog_Slot(hDlg);
		if (VTDialog[Slot].Page > 100)
		{
			SetDlgItemInt(hDlg, IDC_EDIT1, VTDialog[Slot].Page - 1, FALSE);
			VTDialog[Slot].PageChange = TRUE;
		}
		return (TRUE);

	case IDM_VT_PAGE_PLUS:
		Slot = Get_Dialog_Slot(hDlg);
		if (VTDialog[Slot].Page > 100)
		{
			SetDlgItemInt(hDlg, IDC_EDIT1, VTDialog[Slot].Page + 1, FALSE);
			VTDialog[Slot].PageChange = TRUE;
		}
		return (TRUE);

	case WM_LBUTTONUP:
		y = HIWORD(lParam);
		x = LOWORD(lParam);
		if ((x >= 83) && (y >= 13) && (x <= 107) && (y <= 28))
		{
			Slot = Get_Dialog_Slot(hDlg);
			if (VTDialog[Slot].Page > 100)
			{
				SetDlgItemInt(hDlg, IDC_EDIT1, VTDialog[Slot].Page - 1, FALSE);
				VTDialog[Slot].PageChange = TRUE;
			}
			return (TRUE);
		}
		if ((x >= 113) && (y >= 13) && (x <= 138) && (y <= 28))
		{
			Slot = Get_Dialog_Slot(hDlg);
			if (VTDialog[Slot].Page < 899)
			{
				SetDlgItemInt(hDlg, IDC_EDIT1, VTDialog[Slot].Page + 1, FALSE);
				VTDialog[Slot].PageChange = TRUE;
			}
			return (TRUE);

		}
		if ((x >= 143) && (y >= 13) && (x <= 168) && (y <= 28))
		{
			Slot = Get_Dialog_Slot(hDlg);
			SetDlgItemInt(hDlg, IDC_EDIT1, 100, FALSE);
			VTDialog[Slot].PageChange = TRUE;
			return (TRUE);
		}
		if ((x >= 174) && (y >= 4) && (x <= 198) && (y <= 19))
		{
			Slot = Get_Dialog_Slot(hDlg);
			i = VTDialog[Slot].SubPage;
			if (VTFrame[VTDialog[Slot].FramePos].SubCount == 0)
				return (TRUE);
			i--;
			if (i < 0)
				i = VTFrame[VTDialog[Slot].FramePos].SubCount - 1;
			while ((i >= 0) && (VTFrame[VTDialog[Slot].FramePos].SubPage[i].Fill == FALSE))
				i--;
			if (i >= 0)
				VTDialog[Slot].SubPage = i;
			SetDlgItemInt(hDlg, IDC_EDIT1, VTDialog[Slot].Page, FALSE);
			return (TRUE);
		}

		if ((x >= 174) && (y >= 20) && (x <= 198) && (y <= 35))
		{
			Slot = Get_Dialog_Slot(hDlg);
			i = VTDialog[Slot].SubPage;
			if (VTFrame[VTDialog[Slot].FramePos].SubCount == 0)
				return (TRUE);
			i++;
			if (i >= VTFrame[VTDialog[Slot].FramePos].SubCount)
				i = 0;
			while ((i < VTFrame[VTDialog[Slot].FramePos].SubCount) && (VTFrame[VTDialog[Slot].FramePos].SubPage[i].Fill == FALSE))
				i++;
			if (i < VTFrame[VTDialog[Slot].FramePos].SubCount)
				VTDialog[Slot].SubPage = i;
			SetDlgItemInt(hDlg, IDC_EDIT1, VTDialog[Slot].Page, FALSE);

			return (TRUE);
		}

		Slot = Get_Dialog_Slot(hDlg);

		if (VTDialog[Slot].Large == FALSE)
		{
			if ((y >= 50) && (y <= 345) && (x >= 10) && (x <= 406))
			{
				x = ((x - 10) / SMALL_WIDTH) - 1;
				y = (y - 50) / SMALL_HEIGHT;
				if ((x >= 0) && (x <= 39) && (y >= 0) && (y <= 24))
				{
					i = 0;
					Buffer[i] = 0x00;
					while ((x >= 0) && (VTDialog[Slot].AsciiBuffer[y][x] != 0x00))
						x--;
					if (x < 0)
						return (TRUE);
					if (VTDialog[Slot].AsciiBuffer[y][x] == 0x00)
						x++;
					while ((i < 9) && (x + i < 40) && (VTDialog[Slot].AsciiBuffer[y][x + i] != 0x00))
					{
						Buffer[i] = VTDialog[Slot].AsciiBuffer[y][x + i];
						i++;
						Buffer[i] = 0x00;
					}
					if (i != 3)
						return (TRUE);
					i = atoi(Buffer);
					if ((i < 100) || (i > 899))
						return (TRUE);

					if (i != VTDialog[Slot].Page)
						VTDialog[Slot].PageChange = TRUE;
					VTDialog[Slot].Page = i;
					sprintf(Text, "Suche Seite %d", VTDialog[Slot].Page);
					SetDlgItemText(hDlg, IDC_TEXT, Text);
					VTDialog[Slot].FramePos = VT_GetPage(VTDialog[Slot].Page, &VTDialog[Slot].PageChange, &VTDialog[Slot].SubPage);
				}
			}
			return (TRUE);
		}

		if (VTDialog[Slot].Large == TRUE)
		{
			if ((y >= 50) && (y <= 498) && (x >= 10) && (x <= 606))
			{
				x = ((x - 10) / LARGE_WIDTH) - 1;
				y = (y - 50) / LARGE_HEIGHT;
				if ((x >= 0) && (x <= 39) && (y >= 0) && (y <= 24))
				{
					i = 0;
					Buffer[i] = 0x00;
					while ((x >= 0) && (VTDialog[Slot].AsciiBuffer[y][x] != 0x00))
						x--;
					if (x < 0)
						return (TRUE);
					if (VTDialog[Slot].AsciiBuffer[y][x] == 0x00)
						x++;
					while ((i < 9) && (x + i < 40) && (VTDialog[Slot].AsciiBuffer[y][x + i] != 0x00))
					{
						Buffer[i] = VTDialog[Slot].AsciiBuffer[y][x + i];
						i++;
						Buffer[i] = 0x00;
					}
					if (i != 3)
						return (TRUE);
					i = atoi(Buffer);
					if ((i < 100) || (i > 899))
						return (TRUE);
					if (i != VTDialog[Slot].Page)
						VTDialog[Slot].PageChange = TRUE;
					VTDialog[Slot].Page = i;
					sprintf(Text, "Suche Seite %d", VTDialog[Slot].Page);
					SetDlgItemText(hDlg, IDC_TEXT, Text);
					VTDialog[Slot].FramePos = VT_GetPage(VTDialog[Slot].Page - 100, &VTDialog[Slot].PageChange, &VTDialog[Slot].SubPage);
				}
			}
		}

		break;
	case WM_PAINT:
		wHDC = BeginPaint(hDlg, &wps);
		Slot = Get_Dialog_Slot(hDlg);
		if (Slot < 0)
		{
			MessageBox(hDlg, "Error: Unknown Videotext Dialog", "dTV", MB_ICONSTOP | MB_OK);
			EndDialog(hDlg, 0);
			return (TRUE);
		}
		if (VTDialog[Slot].Large == TRUE)
		{
			SetDIBitsToDevice(wHDC, 10, 50, VT_LARGE_BITMAP_WIDTH, VT_LARGE_BITMAP_HEIGHT, 0, 0, 0, VT_LARGE_BITMAP_HEIGHT, _BitmapDataP(VTScreen[Slot]), VTScreen[Slot], DIB_PAL_COLORS);
		}
		else
		{
			SetDIBitsToDevice(wHDC, 10, 50, VT_SMALL_BITMAP_WIDTH, VT_SMALL_BITMAP_HEIGHT, 0, 0, 0, VT_SMALL_BITMAP_HEIGHT, _BitmapDataP(VTScreen[Slot]), VTScreen[Slot], DIB_PAL_COLORS);
		}
		EndPaint(hDlg, &wps);
		break;

	case WM_COMMAND:

		if ((HWND) lParam == GetDlgItem(hDlg, IDC_EDIT1))
		{
			Slot = Get_Dialog_Slot(hDlg);
			if (Slot < 0)
			{
				return (TRUE);
			}
			i = GetDlgItemInt(hDlg, IDC_EDIT1, NULL, FALSE);
			if ((i >= 100) && (i <= 899))
			{
				if (i != VTDialog[Slot].Page)
					VTDialog[Slot].PageChange = TRUE;
				VTDialog[Slot].Page = i;
				sprintf(Text, "Suche Seite %d", VTDialog[Slot].Page);
				SetDlgItemText(hDlg, IDC_TEXT, Text);
				VTDialog[Slot].FramePos = VT_GetPage(VTDialog[Slot].Page, &VTDialog[Slot].PageChange, &VTDialog[Slot].SubPage);
				if (VTDialog[Slot].FramePos >= 0)
				{
					VTFrame[VTDialog[Slot].FramePos].SubPage[VTDialog[Slot].SubPage].bUpdated = 1;
					VT_DoUpdate_Page(VTDialog[Slot].FramePos, VTDialog[Slot].SubPage, Slot, TRUE, 0, VTDialog[Slot].Large);
					VTFrame[VTDialog[Slot].FramePos].SubPage[VTDialog[Slot].SubPage].bUpdated = 0;
					sprintf(Text, "%02d/%02d", VTDialog[Slot].SubPage + 1, VTFrame[VTDialog[Slot].FramePos].SubCount);
					SetDlgItemText(hDlg, IDC_TEXT20, Text);
					sprintf(Text, "Seite %d gefunden", VTDialog[Slot].Page);
					SetDlgItemText(hDlg, IDC_TEXT, Text);
					SetDlgItemText(hDlg, IDC_EDIT1, "");
					sprintf(Text, "dTV VT %d", VTDialog[Slot].Page);
					SetWindowText(hDlg, Text);

					rect.top = 50;
					rect.left = 10;
					rect.bottom = 501;
					rect.right = 637;
					InvalidateRect(hDlg, &rect, FALSE);
				}

			}
			break;
		}

		if (LOWORD(wParam) == IDC_EXPORT)
		{
			Slot = Get_Dialog_Slot(hDlg);
			Export_VT_Page(hDlg, VTDialog[Slot].Page, VTDialog[Slot].SubPage);

		}

		if (LOWORD(wParam) == IDCANCEL)
		{
			KillTimer(hDlg, 1);
			Slot = Del_Dialog_Slot(hDlg);
			if (Slot < 0)
			{
				MessageBox(hDlg, "Error: Unknown Videotext Dialog", "dTV", MB_ICONSTOP | MB_OK);
			}
			else
			{
				if (VTScreen[Slot] != NULL)
					free(VTScreen[Slot]);
				VTScreen[Slot] = NULL;
			}
			VTDialog[Slot].Dialog = NULL;
			EndDialog(hDlg, TRUE);
		}

		break;
	}
	return (FALSE);
	UNREFERENCED_PARAMETER(lParam);
}

int Get_Dialog_Slot(HWND hwnd)
{
	int i;

	for (i = 0; i < MAXVTDIALOG; i++)
	{
		if (hwnd == VTDialog[i].Dialog)
			return (i);
	}

	return (-1);
}

int New_Dialog_Slot(HWND hwnd)
{
	int i;

	for (i = 0; i < MAXVTDIALOG; i++)
	{
		if (VTDialog[i].Dialog == NULL)
		{
			VTDialog[i].Dialog = hwnd;
			VTDialog[i].Page = 0;
			VTDialog[i].SubPage = 0;
			VTDialog[i].FramePos = -1;
			return (i);
		}
	}

	return (-1);
}

int Del_Dialog_Slot(HWND hwnd)
{
	int i;

	for (i = 0; i < MAXVTDIALOG; i++)
	{
		if (hwnd == VTDialog[i].Dialog)
		{
			VTDialog[i].Dialog = NULL;
			VTDialog[i].Page = 0;
			VTDialog[i].FramePos = -1;
			return (i);
		}
	}

	return (-1);
}

void Export_VT_Page(HWND hwnd, int Page, int SubPage)
{
	struct fmt_page PageInfo;
	char FileName[255];

	if (VTFrame[Page - 100].SubCount == 0)
		return;
	if (SubPage > VTFrame[Page - 100].SubCount)
		return;
	if (VTFrame[Page - 100].SubPage[SubPage].Fill == FALSE)
		return;

	sprintf(FileName, "%s\\VT%03d-%02d%.html", VT_BASE_DIR, Page, SubPage);

	format_page(Page - 100, SubPage, VT_REVEAL, &PageInfo);
	html_output(hwnd, &FileName[0], VT_LATIN, VT_NL, VT_HEADERS, VT_STRIPPED, &PageInfo);
}

void format_page(int Page, int SubPage, BOOL reveal, struct fmt_page *pg)
{
	char buf[16];
	int x, y;
	unsigned char c;
	unsigned char *p;
	unsigned char held_mosaic_char = ' ';
	unsigned char dbl = 0;
	unsigned char new_gfx = 0, here_gfx = 0;
	unsigned char new_bg = 0, here_bg = 0;
	unsigned char new_fg = 7, here_fg = 7;
	unsigned char new_bln = 0, here_bln = 0;
	unsigned char new_dbl = 0, here_dbl = 0;
	unsigned char new_con = 0, here_con = 0;
	unsigned char here_sep = 0;
	unsigned char new_hld = 0, here_hld = 0;
	unsigned char held_sep = 0;

	p = &VTFrame[Page].SubPage[SubPage].Frame[0][0];

	pg->dbl = 0;
	sprintf(buf, "\2%3x.%02x\7", Page, SubPage & 0xff);
	strncpy(p, buf, 8);

	for (y = 0; y < 25; y++)
	{
		held_mosaic_char = ' ';
		dbl = 0;
		new_gfx = 0;
		here_gfx = 0;
		new_bg = 0;
		here_bg = 0;
		new_fg = 7;
		here_fg = 7;
		new_bln = 0;
		here_bln = 0;
		new_dbl = 0;
		here_dbl = 0;
		new_con = 0;
		here_con = 0;
		here_sep = 0;
		new_hld = 0;
		here_hld = 0;
		held_sep = 0;
		//    for (x = 0; x < 40; ++x)
		for (x = 0; x < 40; ++x)
		{

			pg->data[y][x].ch = ' ';	// case 0x00 ... 0x1f
			switch (c = *p++)
			{
			case 0x00:
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04:
			case 0x05:
			case 0x06:
			case 0x07:
				//Alpha Colour Codes (foreground)
				//Set-After
				new_gfx = 0;
				new_fg = c;		// in this case (c == (c & 0x07))
				new_con = 0;
				break;
			case 0x08:
				//Flash
				//Set-After
				new_bln = EA_BLINK;
				break;
			case 0x09:
				//Steady
				//Set-At
				new_bln = here_bln = 0;
				break;
			case 0x0a:
				//End Box
				//Set-After
				//!!! not processed
				break;
			case 0x0b:
				//Start Box  (used with End Box to mark visible 
				//            part of page on transparent ones
				//            eg. newsflash or subtitle )
				//Set-After
				//!!! not processed
				break;
			case 0x0c:
				//Normal Size
				//Set-At
				new_dbl = here_dbl = 0;
				break;
			case 0x0d:
				//Double Height
				//Set-After
				dbl = 1;
				new_dbl = EA_DOUBLE;
				break;
			case 0x0e:
				//Double Width (for Presentation Level 2.5 and 3.5)
				//Set-After
				//!!! not processed
				break;
			case 0x0f:
				//Double Size (for Presentation Level 2.5 and 3.5)
				//Set-After
				//!!! not processed
				break;
			case 0x10:
			case 0x11:
			case 0x12:
			case 0x13:
			case 0x14:
			case 0x15:
			case 0x16:
			case 0x17:
				//Mosaic Colour Codes
				//Set-After
				new_gfx = EA_GRAPHIC;
				new_fg = c & 0x07;
				new_con = 0;
				break;
			case 0x18:
				//Conceal
				//Set-At
				here_con = new_con = EA_CONCEALED;
				break;
			case 0x19:
				//Contiguous Mosaic Graphics
				//Set-At
				here_sep = 0;
				break;
			case 0x1a:
				//Separated Mosaic Graphics
				//Set-At
				here_sep = EA_SEPARATED;
				break;
			case 0x1b:
				//ESC (or Switch) 
				//Set-After
				//!!! not processed
				break;
			case 0x1c:
				//Black Background
				//Set-At
				here_bg = new_bg = 0;
				break;
			case 0x1d:
				//New Background
				//Set-At
				here_bg = new_bg = here_fg;
				break;
			case 0x1e:
				//Hold Mosaics
				//Set-At
				here_hld = new_hld = 1;
				break;
			case 0x1f:
				//Release Mosaics
				//Set-After
				new_hld = 0;
				break;

				// start of "... processing ..."

			default:			//noncontrol characters 
				pg->data[y][x].ch = c;
				/* **************************** */
				/* special treating of sep
				 * when in Hold-Mosaics mode 
				 * see the specs for details...
				 */
				if (c & (1 << 5))	//why "6" bit set is (1<<5) ????????
				{
					held_mosaic_char = c;
					held_sep = here_sep;
				}
				if (here_hld)	//when in "hold" set 'here_sep' here:
					pg->data[y][x].attr |= here_sep;
				/* **************************** */
			}					//end of switch

			if (here_hld)
				c = held_mosaic_char;

			if ((here_gfx) && ((c & 0xa0) == 0x20))
				pg->data[y][x].ch = c + ((c & 0x40) ? 32 : -32);

			if ((here_con) && (reveal == FALSE))
				pg->data[y][x].ch = ' ';

			pg->data[y][x].fg = here_fg;
			pg->data[y][x].bg = here_bg;

			if (here_hld)		// special treating again...
				pg->data[y][x].attr = (here_dbl | here_gfx | here_bln | here_con);
			else				// when not in "hold" set 'here_sep' here
				pg->data[y][x].attr = (here_dbl | here_gfx | here_bln | here_con | here_sep);

			// end of "... processing ..."
			here_gfx = new_gfx;
			here_fg = new_fg;
			here_bg = new_bg;
			here_bln = new_bln;
			here_dbl = new_dbl;
			here_con = new_con;
			here_hld = new_hld;
		}
		if (dbl)
		{
			pg->dbl |= 1 << y;
			for (x = 0; x < 40; ++x)
			{
				if (~pg->data[y][x].attr & EA_DOUBLE)
					pg->data[y][x].attr |= EA_HDOUBLE;
				pg->data[y + 1][x] = pg->data[y][x];
				pg->data[y + 1][x].ch = ' ';
			}
			y++;
			p += 40;
		}

	}
}

///////////////////////////////////////////////////////

#define HTML_BLACK   "#000000"
#define HTML_RED     "#FF0000"
#define HTML_GREEN   "#00FF00"
#define HTML_YELLOW  "#FFFF00"
#define HTML_BLUE    "#0000FF"
#define HTML_MAGENTA "#FF00FF"
#define HTML_CYAN    "#00FFFF"
#define HTML_WHITE   "#FFFFFF"

int html_output(HWND hwnd, char *name, int latin1, BOOL HtmlNewLine, BOOL HtmlHeaders, BOOL StrippedHtml, struct fmt_page *pg)
{
	const char *html_colours[] = 
	{ 
		HTML_BLACK,
		HTML_RED,
		HTML_GREEN,
		HTML_YELLOW,
		HTML_BLUE,
		HTML_MAGENTA,
		HTML_CYAN,
		HTML_WHITE
	};
	FILE *fp;
	int x, y;

	fp = fopen(name, "w");
	if (fp == NULL)
	{
		MessageBox(hwnd, "Error :HTML Export File cannot be opened", "dTV", MB_ICONSTOP | MB_OK);
		return -1;
	}

	if (HtmlHeaders == TRUE)
	{
		if (StrippedHtml == FALSE)
		{
			fputs("<!doctype html public \"-//w3c//dtd html 4.0 transitional//en\">", fp);
			if (HtmlNewLine == TRUE)
				fputc('\n', fp);
		}
		fputs("<html><head>", fp);
		if (HtmlNewLine == TRUE)
			fputc('\n', fp);
		if (StrippedHtml == FALSE)
		{
			fputs("<meta http-equiv=\"Content-Type\" content=\"text/html;", fp);
			fprintf(fp, "charset=iso-8859-%d\">", (latin1 ? 1 : 2));
			if (HtmlNewLine == TRUE)
				fputc('\n', fp);
			fputs("<meta name=\"GENERATOR\" content=\"alevt-cap\">", fp);
			if (HtmlNewLine == TRUE)
				fputc('\n', fp);
		}
		else
		{
			fprintf(fp, "<meta charset=iso-8859-%d\">", (latin1 ? 1 : 2));
			if (HtmlNewLine == TRUE)
				fputc('\n', fp);
		}

		fputs("<head>", fp);
		fputs("<body text=\"#FFFFFF\" bgcolor=\"#000000\">", fp);
		if (HtmlNewLine == TRUE)
			fputc('\n', fp);
	}							//bare

	fputs("<tt><b>", fp);
	if (HtmlNewLine == TRUE)
		fputc('\n', fp);

	// write tables in form of HTML format
	for (y = 0; y < 25; ++y)
	{
		int last_nonblank = 0;
		int first_unprinted = 0;
		int last_space = 1;

		// previous char was &nbsp
		// is used for deciding to put semicolon or not
		int nbsp = 0;

		// for output filled with ' ' up to 40 chars
		// set last_nonblank=39
		for (x = 0; x < 40; ++x)
		{
			if (pg->data[y][x].attr & EA_GRAPHIC)
			{
				pg->data[y][x].ch = '#';
			}

			if (pg->data[y][x].ch != ' ')
			{
				last_nonblank = x;	// <-----
			}
		}

		for (x = 0; x <= last_nonblank; ++x)
		{
			if (pg->data[y][x].ch == ' ')
			{
				// if single space between blinking/colour words
				// then make the space blinking/colour too
				if ((x) && (x < 39))
				{
					if ((pg->data[y][x - 1].ch != ' ') && (pg->data[y][x + 1].ch != ' ') && (pg->data[y][x - 1].attr & EA_BLINK) && (pg->data[y][x + 1].attr & EA_BLINK))
					{
						pg->data[y][x].attr |= EA_BLINK;
					}
					else
					{
						pg->data[y][x].attr &= ~EA_BLINK;
					}

					if ((pg->data[y][x - 1].ch != ' ') && (pg->data[y][x + 1].ch != ' ') && (pg->data[y][x - 1].fg == pg->data[y][x + 1].fg))
					{
						pg->data[y][x].fg = pg->data[y][x - 1].fg;
					}
					else
						pg->data[y][x].fg = 7;
				}
				else
				{
					pg->data[y][x].attr &= ~EA_BLINK;
					pg->data[y][x].fg = 7;
				}
			}
			else
			{
				// if foreground is black set the foreground to previous 
				// background colour to let it be visible
				if (!pg->data[y][x].fg)
				{
					pg->data[y][x].fg = pg->data[y][x].bg;
				}
			}
			//check if attributes changed, 
			//if yes then print chars and update first_unprinted
			//if not then go to next char
			if (x)
			{
				if ((((pg->data[y][x].attr & EA_BLINK) == (pg->data[y][x - 1].attr & EA_BLINK)) && (pg->data[y][x].fg == pg->data[y][x - 1].fg)) && (x != last_nonblank))

				{
					continue;
				}
			}
			else
				continue;

			{
				int z = first_unprinted;

				//      last_space=0;
				for (; (pg->data[y][z].ch == ' ') && (z < x); z++)
				{
					if (last_space)
					{
						fprintf(fp, "&nbsp");
						last_space = 0;
						nbsp = 1;
					}
					else
					{
						fputc(' ', fp);
						last_space = 1;
						nbsp = 0;
					}
				}

				first_unprinted = z;

				if (z == x)
					continue;

				if (pg->data[y][first_unprinted].attr & EA_BLINK)
				{
					fprintf(fp, "<blink>");
					nbsp = 0;
				}

				if (pg->data[y][first_unprinted].fg != 7)
				{
					fprintf(fp, "<font color=%s>", html_colours[pg->data[y][first_unprinted].fg]);
					nbsp = 0;
				}
				for (; (z < x) || (z == last_nonblank); z++)
				{

					if (pg->data[y][z].ch == ' ')
					{
						for (; (pg->data[y][z].ch == ' ') && (z < x); z++)
						{
							if (last_space)
							{
								fprintf(fp, "&nbsp");
								last_space = 0;
								nbsp = 1;
							}
							else
							{
								fputc(' ', fp);
								last_space = 1;
								nbsp = 0;
							}
						}
						z--;
					}
					else
					{
						//if previous nbsp --> put semicolon!!!
						if (nbsp)
							fputc(';', fp);
						fputc(pg->data[y][z].ch, fp);
						last_space = 0;
						nbsp = 0;
					}
				}
				if (pg->data[y][first_unprinted].fg != 7)
				{
					fprintf(fp, "</font>");
				}
				if (pg->data[y][first_unprinted].attr & EA_BLINK)
					fprintf(fp, "</blink>");

				first_unprinted = z;
			}
		}
		fputs("<br>", fp);
		if (HtmlNewLine == TRUE)
			fputc('\n', fp);
	}
	fputs("</b></tt>", fp);
	if (HtmlHeaders == TRUE)
		fputs("</body></html>", fp);
	fclose(fp);
	return 0;
}

//////////////////////////////////////////////////////////////////////
