/////////////////////////////////////////////////////////////////////////////
// VBI_VideoText.c
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
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bt848.h"
#include "VBI_VideoText.h"
#include "VBI_CCdecode.h"
#include "VBI.h"
#include "dTV.h"

struct TVT VTFrame[800];
struct TVTDialog VTDialog[MAXVTDIALOG];

int SubPage=0;

BYTE VBI_vcbuf[25];
BYTE VBI_vc2buf[25];

BYTE VBI_CURRENT_MAG;
int VBI_CURRENT_PAGE=-1;
int VBI_CURRENT_SUB=-1;
BOOL VBI_CURRENT_PAGE_ERASE=FALSE;

unsigned int VBI_spos;

int VBI_FPS;

extern unsigned int vtstep;
extern unsigned int vpsstep;

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

unsigned short UTCount = 0;
unsigned short UTPages[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };


#define GetBit(val,bit,mask) (BYTE)(((val)>>(bit))&(mask))

BITMAPINFO* VTCharSetLarge = NULL;
BITMAPINFO* VTCharSetSmall = NULL;
BITMAPINFO* VTScreen[MAXVTDIALOG];

/// VideoText
unsigned short VTColourTable[9] =
{
	0,		//Black
	31744,	//Red
	992,	//Green
	32736,	//Yellow
	31,		//Blue
	15375,	//Invisible
	15871,	//Cyan
	32767,	//White
	32767,	//Transparent
};

void VBI_VT_Init()
{
	HGLOBAL hGlobal;
	int i;

	hGlobal = LoadResource(hInst, FindResource(hInst, "VTCHARLARGE", RT_BITMAP));
	VTCharSetLarge = (BITMAPINFO *) LockResource(hGlobal);
	hGlobal = LoadResource(hInst, FindResource(hInst, "VTCHARSMALL", RT_BITMAP));
	VTCharSetSmall = (BITMAPINFO *) LockResource(hGlobal);

	for (i = 0; i < 800; i++)
	{
		VTFrame[i].SubPage = NULL;
		VTFrame[i].SubCount = 0;
	}

	for (i = 0; i < MAXVTDIALOG; i++)
	{
		VTDialog[i].Dialog = NULL;
	}
	VTScreen[0] = NULL;
	VT_ChannelChange();
}

void VBI_VT_Exit()
{
	int i;

	for (i = 0; i < 800; i++)
	{
		if (VTFrame[i].SubPage != NULL)
		{
			free(VTFrame[i].SubPage);
		}
		VTFrame[i].SubPage = NULL;
		VTFrame[i].SubCount = 0;
	}
	for (i = 0; i < MAXVTDIALOG; i++)
	{
		free(VTScreen[i]);
	}
	
	DeleteObject(VTCharSetLarge);
	DeleteObject(VTCharSetSmall);
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

void VBI_decode_vt(unsigned char *dat)
{
	int i;
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
		if (1)
		{

			//hdump(udat,4); cout << " HD\n";

			/* dat: 55 55 27 %MPAG% %PAGE% %SUB%
			   00 01 02  03 04  05 06 07-0a
			 */


			page = unham(dat + 5);

			if (page == 0x9f)
				break;

			if (mag == 0)
				mag = 8;
			nPage = (page / 16);

			if (nPage > 10)
				break;

			nPage1 = page - (nPage * 16);

			if (nPage1 > 10)
				break;
			pnum = 100 * mag + nPage * 10 + nPage1;

			sub = (unham(dat + 9) << 8) | unham(dat + 7);

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
	  if (1) {

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
		if (1)
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
	int n;

	if (*p != 0x55)
		return;					// Some error, the data should be here...
	p += 5;

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

	for (i = 0; i < 800; i++)
	{
		VTFrame[i].SubCount = 0;
		if (VTFrame[i].SubPage != NULL)
			free(VTFrame[i].SubPage);
		VTFrame[i].SubPage = NULL;
	}
	VT_Cache = 0;
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
			ErrorBox("All Videotext Dialogs occupied");
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
			ErrorBoxDlg(hDlg, "Error :Unknown Videotext Dialog");
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

//////////////////////////////////////////////////////////////////////
void VT_DecodeLine(BYTE* VBI_Buffer)
{
	unsigned char data[45];
	int i, p;

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

void VTS_DecodeLine(BYTE* VBI_Buffer)
{
	unsigned char data[45];
	int i, p;

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