/////////////////////////////////////////////////////////////////////////////
// VBI_CCdecode.c
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
//
//	GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
//
//  This is the CLOSED CAPTION DECODER.
//
//  This software is based on code by timecop@japan.co.jp
//  http://www.ne.jp/asahi/linux/timecop/
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Added Header block
//                                     removed xwindows calls
//                                     put definitions in header
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VBI_CCdecode.h"

char *ccode = " !\"#$%&'()á+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[é]íóúabcdefghijklmnopqrstuvwxyzç÷Ññþ";

char *bufz;

int old_secs=-1,old_hours=-1;


// very over the top memory usage
// still it's on;y temporay
char outbuf[65536];
char tmpbuf[65536];
char rembuf[65536];
char midbuf[65535];

/*
 * Wrap a string at word boundries instead of static character points
 * by ldj (Nov 1998)
 */
int CC_WrapAtWord(char *src, char *dest, char *rem)
{
	int a=0, b=0;
	if(strlen(src)<WRAP_POINT+1)
	{
		strcpy(dest,src);
		return -1;
	}
	for(a=0; a<WRAP_POINT-1; a++)
	{
		if(src[a]==' ')
		{
			b=a;
		}
	}
	strncpy(dest,src,b);
	strcpy(rem,src+(b+1));
	return b;
}

BOOL CC_IsParityOK(int n)	/* check parity for 2 bytes packed in n */
{
    int j, k;
    for (k = 0, j = 0; j < 7; j++)
	{
		if (n & (1<<j))
		{
			k++;
		}
	}
    if ((k & 1) && (n & 0x80))
	{
		return FALSE;
	}
    for (k = 0, j = 8; j < 15; j++)
	{
		if (n & (1<<j)) k++;
	}
    if ((k & 1) && (n && 0x8000))
	{
		return FALSE;
	}
	return TRUE;
}

int CC_DecodeBit(BYTE* data, int threshold)
{
    int i, sum = 0;
    for (i = 0; i < 32; i++)
	{
		sum += data[i];
	}
    return (sum > (threshold<<5));
}

int CC_DecodeLine(BYTE* vbiline)
{
	int max[7], min[7], val[7], i, clk, tmp, sample, packedbits = 0;
	int done = 0, mode = 0, lastchar = 0;
	int b1, b2, ret;

    for (clk=0; clk<7; clk++)
	{
		max[clk] = min[clk] = val[clk] = -1;
	}
    clk = tmp = 0;
    i=57;
    while (i < 600 && clk < 7)
	{	/* find and lock all 7 clocks */
		sample = vbiline[i];
		if (max[clk] < 0)
		{ /* find maximum value before drop */
			if (sample > 70 && sample > val[clk])
			{
				(val[clk] = sample, tmp = i);	/* mark new maximum found */
			}
			else if (val[clk] - sample > 30)
			{ /* far enough */
				(max[clk] = tmp, i = tmp + 10);
			}
		}
		else
		{ /* find minimum value after drop */
			if (sample < val[clk])
			{
				(val[clk] = sample, tmp = i);	/* mark new minimum found */
			}
			else if (sample - val[clk] > 30)	/* searched far enough */
			{
				(min[clk++] = tmp, i = tmp + 10);
			}
		}
		i++;
    }
    if (clk != 7)		/* failure to locate clock lead-in */
	{
		return 0;
	}

    /* calculate threshold */
    sample = vbiline[min[0]] + ((vbiline[max[0]] - vbiline[min[0]])>>1);

    /* found clock lead-in, now find start */
    for (i=min[6],tmp=0;tmp<27 && i<800;i++)
	{
		if (vbiline[i] > sample)
		{
			tmp++;
		}
	    else
		{
			tmp = 0;
		}
	}
    if (i==800)		/* failure to locate start bit */
	{
	    return 0;
	}
    tmp = i + 37;		/* tmp = data bit zero */
    for (i = 0; i < 16; i++)
	{
		if(CC_DecodeBit(&vbiline[tmp + i * 57], sample))
		{
		    packedbits |= 1<<i;
		}
	}
    if (!CC_IsParityOK(packedbits))
	{
		return 0;
	}

	b1 = packedbits & 0x7f;
	b2 = (packedbits>>8) & 0x7f;

	if ((b1 & 96))
	{
		if (b1 > 31)
		{
		    strncat(outbuf,&ccode[b1 - 32], 1);
		    if (b1 > 32) lastchar = ccode[b1-32];
		}
		if (b2 > 31)
		{
		    strncat(outbuf,&ccode[b2 - 32], 1);
		    if (b2 > 32) lastchar = ccode[b2-32];
		}
	}
	if (!(b1 & 96) && b1 && *outbuf)
	{
		fprintf(stderr," %d\n",strlen(outbuf));

		ret=CC_WrapAtWord(outbuf,tmpbuf,rembuf);
		OutputDebugString(tmpbuf);
		while(ret!=-1)
		{
			strcpy(midbuf,rembuf);
			memset(tmpbuf,0,sizeof(tmpbuf));
			ret=CC_WrapAtWord(midbuf,tmpbuf,rembuf);
			OutputDebugString(tmpbuf);
		}
		*outbuf=0;
		mode = 0;
	}
	else if (outbuf[strlen(outbuf)-1] != ' ')
	{
		strncat(outbuf,ccode, 1);
	}
	return 1;
}

