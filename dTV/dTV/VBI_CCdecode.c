/////////////////////////////////////////////////////////////////////////////
// VBI_CCdecode.c
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 Mike Baker.  All rights reserved.
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
//  This is the CLOSED CAPTION DECODER.
//
// taken from ccdecode.c by Mike Baker (mbm@linux.com)
// (based on code by timecop@japan.co.jp)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 29 Jan 2001   John Adcock           Took code from latest version of ccdecode
//                                     Made compatable with dTV
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VBI_CCdecode.h"
#define DOLOGGING
#include "DebugLog.h"
#include "dTV.h"
#include "BT848.h"


/* #define XWIN 1   /* visual debugging */

#ifdef XWIN
	#include <X11/X.h>
	#include <X11/Xlib.h>
	#include <X11/Xutil.h>
	#include <X11/Xproto.h>
	Display *dpy;
	Window Win,Root;
	char dpyname[256] = "";
	GC WinGC;
	GC WinGC0;
	GC WinGC1;
	int x;
#endif


//XDSdecode
char	info[8][25][256]; 
char	newinfo[8][25][256];
char	*infoptr=newinfo[0][0];
int	mode,type;
char	infochecksum;

//ccdecode
char    *ratings[] = {"(NOT RATED)","TV-Y","TV-Y7","TV-G","TV-PG","TV-14","TV-MA","(NOT RATED)"};
int     rowdata[] = {11,-1,1,2,3,4,12,13,14,15,5,6,7,8,9,10};
char	*specialchar[] = {"®","°","½","¿","(TM)","¢","£","o/~ ","à"," ","è","â","ê","î","ô","û"};
char	*modes[]={"current","future","channel","miscellanious","public service","reserved","invalid","invalid","invalid","invalid"};
int	lastcode;
int	ccmode=1;		//cc1 or cc2
char	ccbuf[3][256];		//cc is 32 columns per row, this allows for extra characters
int	keywords=0;
char	*keyword[32];


//args (this should probably be put into a structure later)
char rawline=-1;

int sen;
int inval;

int parityok(int n)	/* check parity for 2 bytes packed in n */
{
    int mask=0;
    int j, k;

    for (k = 1, j = 0; j < 7; j++)
	{
		if (n & (1<<j))
		{
			k++;
		}
	}
    if ((k & 1) == ((n>>7)&1))
	{
	    mask |= 0x00FF;
	}
    for (k = 1, j = 8; j < 15; j++)
	{
		if (n & (1<<j))
		{
			k++;
		}
	}
    if ((k & 1) == ((n>>15)&1))
	{
	    mask |= 0xFF00;
	}
   return mask;
}

int decodebit(unsigned char *data, int threshold)
{
    int i, sum = 0;
    for (i = 0; i < 23; i++)
	{
		sum += data[i];
	}
    return (sum > threshold*23);
}

int FindClock(unsigned char *vbiline, double ClockPixels)
{
	int i;
	DWORD MinTotal = 0;
	int MinCount = 0;
	DWORD MaxTotal = 0;
	int MaxCount = 0;
	double Integer;
	double Remainder;
	for(i = 0; i < (int)(ClockPixels * 6.5); i++)
	{
		Remainder = modf((double)i / ClockPixels, &Integer);
		if(Remainder < 0.25 || Remainder > 0.75)
		{
			MinTotal += vbiline[i];
			MinCount++;
		}
		else
		{
			MaxTotal += vbiline[i];
			MaxCount++;
		}
	}
	if(MinCount == 0 || MaxCount == 0)
	{
		return 0;	
	}
	else
	{
		return (MaxTotal / MaxCount) - (MinTotal / MinCount);
	}
}

int decode(unsigned char *vbiline)
{
    int max[7], min[7], val[7], i, clk, tmp, packedbits = 0;
	double ClockPixels;
	int ClockMax = -1;
	int ClockPos = -1;
	int ClockCur;
	static int LastCode = 0;
    
    for (clk=0; clk<7; clk++)
	{
		max[clk] = min[clk] = val[clk] = -1;
	}
    clk = tmp = 0;

	ClockPixels = 8.0 * BT848_GetTVFormat()->Fsc / BT848_GetTVFormat()->CC_Clock;

    i=35;

    while (i < 50)
	{
		ClockCur = FindClock(vbiline + i, ClockPixels);
		if(ClockCur > ClockMax)
		{
			ClockMax = ClockCur;
			ClockPos = i;
		}
		i++;
	}

	if(ClockMax < 45)
	{
		if(LastCode != 0)
		{
			LOGD("Nothing found %d\n", ClockMax);
		}
		LastCode = 0;
		return -1;
	}

    tmp = ClockPos + 512;
	if(!decodebit(&vbiline[tmp], 0x90))
	{
		// no start bit
		LastCode = 0;
		return -1;
	}
	tmp += 57;
    for (i = 0; i < 16; i++)
	{
		if(decodebit(&vbiline[tmp + i * 57], 0x70))
		{
			packedbits |= 1<<i;
		}
	}
	
	LastCode = packedbits;
    return packedbits & parityok(packedbits);
	//LOGD("%c%c\n", packedbits & 0x7F, (packedbits>>8) & 0x7F);
	//LastCode = packedbits;
	//return packedbits;
} /* decode */

int XDSdecode(int data)
{
	int b1, b2, length;
	if (data == -1)
		return -1;
	
	b1 = data & 0x7F;
	b2 = (data>>8) & 0x7F;

	if (b1 < 15) // start packet 
	{
		mode = b1;
		type = b2;
		infochecksum = b1 + b2 + 15;
		if (mode > 8 || type > 20)
		{
//			LOGD("%% Unsupported mode %s(%d) [%d]\n",modes[(mode-1)>>1],mode,type);
			mode=0; type=0;
		}
		infoptr = newinfo[mode][type];
	}
	else if (b1 == 15) // eof (next byte is checksum)
	{
#if 0 //debug
		if (mode == 0)
		{
			length=infoptr - newinfo[0][0];
			infoptr[1]=0;
			LOGD("LEN: %d\n",length);
			for (y=0;y<length;y++)
				LOGD(" %03d",newinfo[0][0][y]);
			LOGD(" --- %s\n",newinfo[0][0]);
		}
#endif
		if (mode == 0) return 0;
		if (b2 != 128-((infochecksum%128)&0x7F)) return 0;

		length = infoptr - newinfo[mode][type];

		//don't bug the user with repeated data
		//only parse it if it's different
		if (strncmp(info[mode][type],newinfo[mode][type],length-1))
		{
			infoptr = info[mode][type];
			memcpy(info[mode][type],newinfo[mode][type],length+1);
			LOGD("\33[33m%%");
			switch ((mode<<8) + type)
			{
				case 0x0101:
					LOGD(" TIMECODE: %d/%02d %d:%02d",
					infoptr[3]&0x0f,infoptr[2]&0x1f,infoptr[1]&0x1f,infoptr[0]&0x3f);
				case 0x0102:
					if ((infoptr[1]&0x3f)>5)
						break;
					LOGD("   LENGTH: %d:%02d:%02d of %d:%02d:00",
					infoptr[3]&0x3f,infoptr[2]&0x3f,infoptr[4]&0x3f,infoptr[1]&0x3f,infoptr[0]&0x3f);
					break;
				case 0x0103:
					infoptr[length] = 0;
					LOGD("    TITLE: %s",infoptr);
					break;
				case 0x0105:
					LOGD("   RATING: %s (%d)",ratings[infoptr[0]&0x07],infoptr[0]);
					if ((infoptr[0]&0x07)>0)
					{
						if (infoptr[0]&0x20) LOGD(" VIOLENCE");
						if (infoptr[0]&0x10) LOGD(" SEXUAL");
						if (infoptr[0]&0x08) LOGD(" LANGUAGE");
					}
					break;
				case 0x0501:
					infoptr[length] = 0;
					LOGD("  NETWORK: %s",infoptr);
					break;
				case 0x0502:
					infoptr[length] = 0;
					LOGD("     CALL: %s",infoptr);
					break;
				case 0x0701:
					LOGD(" CUR.TIME: %d:%02d %d/%02d/%04d UTC",infoptr[1]&0x1F,infoptr[0]&0x3f,infoptr[3]&0x0f,infoptr[2]&0x1f,(infoptr[5]&0x3f)+1990);
					break;
				case 0x0704: //timezone
					LOGD(" TIMEZONE: UTC-%d",infoptr[0]&0x1f);
					break;
				case 0x0104: //program genere
					break;
				case 0x0110:
				case 0x0111:
				case 0x0112:
				case 0x0113:
				case 0x0114:
				case 0x0115:
				case 0x0116:
				case 0x0117:
					infoptr[length+1] = 0;
					LOGD("     DESC: %s",infoptr);
					break;
			}
			LOGD("\33[0m\n");
		}
		mode = 0; type = 0;
	}
	else if( (infoptr - newinfo[mode][type]) < 250 ) // must be a data packet, check if we're in a supported mode
	{
		infoptr[0] = b1; infoptr++;
		infoptr[0] = b2; infoptr++;
		infochecksum += b1 + b2;
	}
	return 0;
}

void webtv_check(char * buf,int len)
{
	unsigned long   sum;
	unsigned long   nwords;
	unsigned short  csum=0;
	char temp[9];
	int nbytes=0;
	
	while (buf[0]!='<' && len > 6)  //search for the start
	{
		buf++; len--;
	}
	
	if (len == 6) //failure to find start
		return;
				
	
	while (nbytes+6 <= len)
	{
		//look for end of object checksum, it's enclosed in []'s and there shouldn't be any [' after
		if (buf[nbytes] == '[' && buf[nbytes+5] == ']' && buf[nbytes+6] != '[')
			break;
		else
			nbytes++;
	}
	if (nbytes+6>len) //failure to find end
		return;
	
	nwords = nbytes >> 1; sum = 0;

	//add up all two byte words
	while (nwords-- > 0) {
		sum += *buf++ << 8;
		sum += *buf++;
	}
	if (nbytes & 1) {
		sum += *buf << 8;
	}
	csum = (unsigned short)(sum >> 16);
	while(csum !=0) {
		sum = csum + (sum & 0xffff);
		csum = (unsigned short)(sum >> 16);
	}
	sprintf(temp,"%04X\n",(int)~sum&0xffff);
	buf++;
	if(!strncmp(buf,temp,4))
	{
		buf[5]=0;
		LOGD("\33[35mWEBTV: %s\33[0m\n",buf-nbytes-1);
	}
}

int CCdecode(int data)
{
	int b1, b2, row, len, x,y;
	if (data == -1) //invalid data. flush buffers to be safe.
	{
		memset(ccbuf[1],0,255);
		memset(ccbuf[2],0,255);
		return -1;
	}
	b1 = data & 0x7f;
	b2 = (data>>8) & 0x7f;
	len = strlen(ccbuf[ccmode]);

	if (b1&0x60 && data != lastcode) // text
	{
		ccbuf[ccmode][len++]=b1;
		if (b2&0x60) ccbuf[ccmode][len++]=b2;
		if (b1 == ']' || b2 == ']')
			webtv_check(ccbuf[ccmode],len);
		//LOGD("%c%c", b1, b2);
	}
	else if ((b1&0x10) && (b2>0x1F) && (data != lastcode)) //codes are always transmitted twice (apparently not, ignore the second occurance)
	{
		ccmode=((b1>>3)&1)+1;
		len = strlen(ccbuf[ccmode]);

		if (b2 & 0x40)	//preamble address code (row & indent)
		{
			row=rowdata[((b1<<1)&14)|((b2>>5)&1)];
			if (len!=0)
			{
				ccbuf[ccmode][len++]='\n';
				LOGD("\n");
			}

			if (b2&0x10) //row contains indent flag
				for (x=0;x<(b2&0x0F)<<1;x++)
					ccbuf[ccmode][len++]=' ';
		}
		else
		{
			switch (b1 & 0x07)
			{
				case 0x00:	//attribute
					LOGD("<ATTRIBUTE %d %d>\n",b1,b2);
					break;
				case 0x01:	//midrow or char
					switch (b2&0x70)
					{
						case 0x20: //midrow attribute change
							switch (b2&0x0e)
							{
								case 0x00: //italics off
									strcat(ccbuf[ccmode],"\33[0m ");
									break;
								case 0x0e: //italics on
									strcat(ccbuf[ccmode],"\33[36m ");
									break;
							}
							if (b2&0x01) //underline
								strcat(ccbuf[ccmode],"\33[4m");
							else
								strcat(ccbuf[ccmode],"\33[24m");
							break;
						case 0x30: //special character..
							strcat(ccbuf[ccmode],specialchar[b2&0x0f]);
							break;
					}
					break;
				case 0x04:	//misc
				case 0x05:	//misc + F
//					LOGD("ccmode %d cmd %02x\n",ccmode,b2);
					switch (b2)
					{
						case 0x21: //backspace
							ccbuf[ccmode][len--]=0;
							break;
							
						/* these codes are insignifigant if we're ignoring positioning */
						case 0x25: //2 row caption
						case 0x26: //3 row caption
						case 0x27: //4 row caption
						case 0x29: //resume direct caption
						case 0x2B: //resume text display
						case 0x2C: //erase displayed memory
							break;
							
						case 0x2D: //carriage return
							if (ccmode==2)
								break;
						case 0x2F: //end caption + swap memory
						case 0x20: //resume caption (new caption)
							if (!strlen(ccbuf[ccmode]))
									break;
							for (x=0;x< (int)strlen(ccbuf[ccmode]);x++)
								for (y=0;y<keywords;y++)
									if (!strnicmp(keyword[y], ccbuf[ccmode]+x, strlen(keyword[y])))
										LOGD("\a");
							LOGD("%s\33[m\n",ccbuf[ccmode]);
							/* FALL */
						case 0x2A: //text restart
						case 0x2E: //erase non-displayed memory
							memset(ccbuf[ccmode],0,255);
							break;
					}
					break;
				case 0x07:	//misc (TAB)
					for(x=0;x<(b2&0x03);x++)
						ccbuf[ccmode][len++]=' ';
					break;
			}
		}
	}
	lastcode=data;
	return 0;
}

int RAW(int data)
{
	int b1, b2;
	if (data == -1)
		return -1;
	b1 = data & 0x7f;
	b2 = (data>>8) & 0x7f;
	return 0;
}

int sentance(int data)
{
	static char szCCText[2][31];
	static int Pos = 0;
	static int Buff = 0;
	int b1, b2;
	if (data == -1)
		return -1;
	b1 = data & 0x7f;
	b2 = (data>>8) & 0x7f;
	inval++;
	if (data==lastcode)
	{
		if (sen==1)
		{
			szCCText[Buff][Pos++] = ' ';
			sen=0;
		}
		if (inval>10 && sen)
		{
			szCCText[Buff][Pos++] = '\0';
			PostMessage(hWnd, WM_COMMAND, IDM_OSD_CC_TEXT, (LPARAM)szCCText[Buff]);
			Buff = !Buff;
			Pos = 0;
			sen=0;
		}
		return 0;
	}
	lastcode=data;

	if (b1&96)
	{
		inval=0;
		if (sen==2 && b1!='.' && b2!='.' && b1!='!' && b2!='!' && b1!='?' && b2!='?' && b1!=')' && b2!=')')
		{
			szCCText[Buff][Pos++] = '\0';
			PostMessage(hWnd, WM_COMMAND, IDM_OSD_CC_TEXT, (LPARAM)szCCText[Buff]);
			Buff = !Buff;
			Pos = 0;
			sen=1;
		}
		else if (b1=='.' || b2=='.' || b1=='!' || b2=='!' || b1=='?' || b2=='?' || b1==')' || b2==')')
			sen=2;
		else
			sen=1;
		if(Pos < 30)
		{
			szCCText[Buff][Pos++] = tolower(b1);
			szCCText[Buff][Pos++] = tolower(b2);
		}
		else
		{
			szCCText[Buff][Pos++] = '\0';
			PostMessage(hWnd, WM_COMMAND, IDM_OSD_CC_TEXT, (LPARAM)szCCText[Buff]);
			Buff = !Buff;
			Pos = 0;
		}
	}
	return 0;
}

int CC_DecodeLine(BYTE* vbiline)
{
	//CCdecode(decode(vbiline));
	sentance(decode(vbiline));
	return 0;
}

int XDS_DecodeLine(BYTE* vbiline)
{
	XDSdecode(decode(vbiline));
	return 0;
}
