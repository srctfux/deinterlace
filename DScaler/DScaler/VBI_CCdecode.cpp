/////////////////////////////////////////////////////////////////////////////
// $Id: VBI_CCdecode.cpp,v 1.12 2002-11-28 21:29:52 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 Mike Baker.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//  This file is subject to the terms of the GNU General Public License as
//  published by the Free Software Foundation.  A copy of this license is
//  included with this software distribution in the file COPYING.  If you
//  do not have a copy, you may obtain a copy by writing to the Free
//  Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//
//  GNU General Public License for more details
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.11  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.10  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.9  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.8  2001/09/05 15:08:43  adcockj
// Updated Loging
//
// Revision 1.7.2.2  2001/08/21 09:43:01  adcockj
// Brought branch up to date with latest code fixes
//
// Revision 1.7.2.1  2001/08/18 17:09:30  adcockj
// Got to compile, still lots to do...
//
// Revision 1.7  2001/08/02 18:08:17  adcockj
// Made all logging code use new levels
//
// Revision 1.6  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.5  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "VBI_CCdecode.h"
#include "DebugLog.h"
#include "DScaler.h"
#include "Providers.h"

//XDSdecode
char          Info[8][25][256]; 
char          NewInfo[8][25][256];
char*         pInfo = NewInfo[0][0];
int           Mode;
int           Type;
char          InfoCheckSum;

//ccdecode
char*         Ratings[] = {"(NOT RATED)","TV-Y","TV-Y7","TV-G","TV-PG","TV-14","TV-MA","(NOT RATED)"};
int           RowData[] = {10,0,0,1,2,3,11,12,13,14,4,5,6,7,8,9};
char*         SpecialChars[] = {"�","�","�","�","�","�","�","� ","�"," ","�","�","�","�","�","�"};
char*         Modes[]={"current","future","channel","miscellanious","public service","reserved","invalid","invalid","invalid","invalid"};
int           LastCode;
int           CCDisplayMode=1;       //cc1 or cc2
char          CCBuf[3][256];      //cc is 32 columns per row, this allows for extra characters

int parityok(int n) // check parity for 2 bytes packed in n 
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

int decodebit(unsigned char* data, int threshold, int NumPixels)
{
    int i, sum = 0;
    for (i = 0; i < NumPixels; i++)
    {
        sum += data[i];
    }
    return (sum > threshold * NumPixels);
}

int FindClock(unsigned char* vbiline, int ClockPixels)
{
    int i;
    DWORD MinTotal = 0;
    int MinCount = 0;
    DWORD MaxTotal = 0;
    int MaxCount = 0;
    int LastPixel = (int)(ClockPixels * 6.5); 
    int Remainder;

    for(i = 0; i < LastPixel; i++)
    {
        Remainder = i % ClockPixels;
        if(Remainder < ClockPixels / 4  || Remainder > 3 * ClockPixels / 4)
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

int decode(unsigned char* vbiline)
{
    int i, tmp, packedbits = 0;
    int ClockMax = -1;
    int ClockPos = -1;
    DWORD Threshold = 0;
    int ClockCur;
    TTVFormat* TVFormat = GetTVFormat(Providers_GetCurrentSource()->GetFormat());
    
    i=0;

    while (i < 120)
    {
        ClockCur = FindClock(vbiline + i, TVFormat->CC_Clock);
        if(ClockCur > ClockMax)
        {
            ClockMax = ClockCur;
            ClockPos = i;
        }
        i++;
    }

    if(ClockMax < 10)
    {
        return -1;
    }

    tmp = ClockPos + TVFormat->CC_Gap - 3 * TVFormat->CC_Clock / 2;

    for(i = 0; i < TVFormat->CC_Clock; i++)
    {
        Threshold += vbiline[tmp + i];
    }
    Threshold /= TVFormat->CC_Clock;
    Threshold += ClockMax / 2; 
    tmp = ClockPos + TVFormat->CC_Gap;

    if(!decodebit(&vbiline[tmp], Threshold, TVFormat->CC_Clock / 2))
    {
        // no start bit
        return -1;
    }
    for (i = 0; i < 16; i++)
    {
        tmp += TVFormat->CC_Clock;
        if(decodebit(&vbiline[tmp], Threshold, TVFormat->CC_Clock / 2))
        {
            packedbits |= 1<<i;
        }
    }
    
    return packedbits & parityok(packedbits);
} // decode 

int XDSdecode(int data)
{
    int b1, b2, length;
    if (data == -1)
        return -1;
    
    b1 = data & 0x7F;
    b2 = (data>>8) & 0x7F;

    if (b1 < 15) // start packet 
    {
        Mode = b1;
        Type = b2;
        InfoCheckSum = b1 + b2 + 15;
        if (Mode > 8 || Type > 20)
        {
//          LOG(5, "%% Unsupported Mode %s(%d) [%d]\n",Modes[(Mode-1)>>1],Mode,Type);
            Mode=0; Type=0;
        }
        pInfo = NewInfo[Mode][Type];
    }
    else if (b1 == 15) // eof (next byte is checksum)
    {
#if 0 //debug
        if (Mode == 0)
        {
            length=pInfo - NewInfo[0][0];
            pInfo[1]=0;
            LOG(5, "LEN: %d\n",length);
            for (y=0;y<length;y++)
                LOG(5, " %03d",NewInfo[0][0][y]);
            LOG(5, " --- %s\n",NewInfo[0][0]);
        }
#endif
        if (Mode == 0) return 0;
        if (b2 != 128-((InfoCheckSum%128)&0x7F)) return 0;

        length = pInfo - NewInfo[Mode][Type];

        //don't bug the user with repeated data
        //only parse it if it's different
        if (strncmp(Info[Mode][Type],NewInfo[Mode][Type],length-1))
        {
            pInfo = Info[Mode][Type];
            memcpy(Info[Mode][Type],NewInfo[Mode][Type],length+1);
            LOG(5, "\33[33m%%");
            switch ((Mode<<8) + Type)
            {
                case 0x0101:
                    LOG(5, "TIMECODE: %d/%02d %d:%02d",
                    pInfo[3]&0x0f,pInfo[2]&0x1f,pInfo[1]&0x1f,pInfo[0]&0x3f);
                case 0x0102:
                    if ((pInfo[1]&0x3f)>5)
                        break;
                    LOG(5, "LENGTH: %d:%02d:%02d of %d:%02d:00",
                    pInfo[3]&0x3f,pInfo[2]&0x3f,pInfo[4]&0x3f,pInfo[1]&0x3f,pInfo[0]&0x3f);
                    break;
                case 0x0103:
                    pInfo[length] = 0;
                    LOG(5, "TITLE: %s",pInfo);
                    break;
                case 0x0105:
                    LOG(5, "RATING: %s (%d)",Ratings[pInfo[0]&0x07],pInfo[0]);
                    if ((pInfo[0]&0x07)>0)
                    {
                        if (pInfo[0]&0x20) LOG(5, "VIOLENCE");
                        if (pInfo[0]&0x10) LOG(5, "SEXUAL");
                        if (pInfo[0]&0x08) LOG(5, "LANGUAGE");
                    }
                    break;
                case 0x0501:
                    pInfo[length] = 0;
                    LOG(5, "NETWORK: %s",pInfo);
                    break;
                case 0x0502:
                    pInfo[length] = 0;
                    LOG(5, "CALL: %s",pInfo);
                    break;
                case 0x0701:
                    LOG(5, "CUR.TIME: %d:%02d %d/%02d/%04d UTC",pInfo[1]&0x1F,pInfo[0]&0x3f,pInfo[3]&0x0f,pInfo[2]&0x1f,(pInfo[5]&0x3f)+1990);
                    break;
                case 0x0704: //timezone
                    LOG(5, "TIMEZONE: UTC-%d",pInfo[0]&0x1f);
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
                    pInfo[length+1] = 0;
                    LOG(5, "DESC: %s",pInfo);
                    break;
            }
            LOG(5, "\33[0m\n");
        }
        Mode = 0; Type = 0;
    }
    else if( (pInfo - NewInfo[Mode][Type]) < 250 ) // must be a data packet, check if we're in a supported Mode
    {
        pInfo[0] = b1; pInfo++;
        pInfo[0] = b2; pInfo++;
        InfoCheckSum += b1 + b2;
    }
    return 0;
}

void webtv_check(char* buf,int len)
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
        LOG(5, "\33[35mWEBTV: %s\33[0m\n",buf-nbytes-1);
    }
}


int CCdecode(int data, BOOL CaptionMode, int Channel)
{
    int b1, b2, len, x;
    static TCCScreen Screens[2];
    static int ScreenToWrite = 0;
    static int CursorPos = 0;
    static int CursorRow = 14;
    static char ForeColor = 0;
    static eCaptionMode Mode = TEXT;
    static int ModeRows = 0;
    static int LastIndent = -1;
    static TCCChar CurrentState =
        { TRUE, ' ', CC_WHITE, CC_BLACK, FALSE, FALSE, FALSE,};
    static TCCChar ResetState =
        { TRUE, ' ', CC_WHITE, CC_BLACK, FALSE, FALSE, FALSE,};
    static BOOL bCaptureText = FALSE;
    static int BadCount = 0;

    BOOL bPaintNow = FALSE;

    if (data == -1) //invalid data.
    {
        // if we get three in a row then reset the 
        // display
        BadCount++;
        if(BadCount == 3)
        {
            memset(&Screens[0],0,sizeof(TCCScreen));
            bPaintNow = TRUE;
            bCaptureText = FALSE;
        }
    }
    else
    {
        BadCount = 0;
        b1 = data & 0x7f;
        b2 = (data>>8) & 0x7f;

        if (b1&0x60) // text (characters not repeated)
        {
            if(bCaptureText)
            {
                // copy first character
                if(CursorPos < CC_CHARS_PER_LINE - 1)
                {
                    memcpy(&Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos], &CurrentState, sizeof(TCCChar));
                    Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].Text = b1;
                    CursorPos++;
                }
                else 
                {
					// Not enough room - Shift characters over one row
                    for(x = 1; x < CC_CHARS_PER_LINE;x++)
                    {
                        memcpy(&Screens[ScreenToWrite].ScreenData[CursorRow][x - 1], 
                            &Screens[ScreenToWrite].ScreenData[CursorRow][x], 
                            sizeof(TCCChar));
                    }
                    memcpy(&Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos], &CurrentState, sizeof(TCCChar));
                    Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].Text = b1;
                };
                
                if(b2 > 31)
                {
                    // copy second character
                    if(CursorPos < CC_CHARS_PER_LINE - 1)
                    {
                        memcpy(&Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos], &CurrentState, sizeof(TCCChar));
                        Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].Text = b2;
                        CursorPos++;
                    }
                    else 
                    {
					    // Not enough room - Shift characters over one row
                        for(x = 1; x < CC_CHARS_PER_LINE;x++)
                        {
                            memcpy(&Screens[ScreenToWrite].ScreenData[CursorRow][x - 1], 
                                &Screens[ScreenToWrite].ScreenData[CursorRow][x], 
                                sizeof(TCCChar));
                        }
                        memcpy(&Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos], &CurrentState, sizeof(TCCChar));
                        Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].Text = b2;
                    }
                }
                bPaintNow = (ScreenToWrite == 0);
            }
        }
        else if ((b1&0x10) && (b2>0x1F) && (data != LastCode)) //codes are always transmitted twice (ignore the second occurance)
        {
            // if we have been asked for the other channel
            // need to ignore any text that follows
            if(((b1>>3)&1) != Channel)
            {
                bCaptureText = FALSE;
                return 0;
            }
            len = strlen(CCBuf[CCDisplayMode]);

            if (b2 & 0x40)  //preamble address code (row & indent)
            {
                if(bCaptureText == TRUE)
                {
                    // reset state
                    memcpy(&CurrentState, &ResetState, sizeof(TCCChar));

                    // row information ignored in text Mode
                    if(Mode != ROLL_UP)
                    {
                        CursorRow = RowData[((b1<<1)&0x0E)|((b2>>5)&0x01)];
                    }
                    
                    CursorPos = 0;

                    // get underline flag
                    CurrentState.bUnderline = b2&0x1;

                    // get indent Info
                    if (b2&0x10)
                    {
                        //row contains indent flag
                        CurrentState.ForeColor = CC_WHITE;
                        CursorPos = (b2&0x0E) << 1; 
                        // there seems to be a problem with the pop on captions
                        // when you get the second or third line of data

                        for(x=0; x<CursorPos; x++)
                        {
                            Screens[ScreenToWrite].ScreenData[CursorRow][x].bIsActive = FALSE;
                        }
                        LastIndent = CursorPos;
                    }
                    else
                    {
                        // get color and indent Info
                        if(((b2&0x0E) >> 1) == 0x07)
                        {
                            CurrentState.bItalics = TRUE;
                        }
                        else
                        {
                            CurrentState.ForeColor = (eCCColor)((b2&0x0E) >> 1);
                        }
                    }
                }               
            }
            else
            {
                switch (b1 & 0x07)
                {
                    case 0x00:  //attribute
                        if(bCaptureText == TRUE)
                        {
                            CurrentState.BackColor = (eCCColor)((b2&0x0E) >> 1);
                            memcpy(&Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos], &CurrentState, sizeof(TCCChar));
                            Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].Text = ' ';
                            if(CursorPos < CC_CHARS_PER_LINE - 1) CursorPos++;
                        }
                        break;
                    case 0x01:  //midrow or char
                        if(bCaptureText == TRUE)
                        {
                            switch (b2&0x70)
                            {
                                case 0x20: //midrow attribute change
                                    switch (b2&0x0e)
                                    {
                                        case 0x0e: //italics on
                                            CurrentState.bItalics = TRUE;
                                            break;
                                        default:
                                            CurrentState.ForeColor = (eCCColor)((b2&0x0E) >> 1);
                                            CurrentState.bItalics = FALSE;
                                            break;
                                    }
                                    if (b2&0x01) //underline
                                        CurrentState.bUnderline = TRUE;
                                    else
                                        CurrentState.bUnderline = FALSE;
                                    memcpy(&Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos], &CurrentState, sizeof(TCCChar));
                                    Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].Text = ' ';
                                    if(CursorPos < CC_CHARS_PER_LINE - 1) CursorPos++;
                                    break;
                                case 0x30: //special character..
                                    memcpy(&Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos], &CurrentState, sizeof(TCCChar));
                                    Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].Text = *(SpecialChars[b2&0x0f]);
                                    if(CursorPos < CC_CHARS_PER_LINE - 1) CursorPos++;
                                    break;
                            }
                        }
                        break;
                    case 0x04:  //misc
                    case 0x05:  //misc + F
                        switch (b2)
                        {
                        // start pop on captioning
                        case 0x20:
                            if(CaptionMode)
                            {
                                ScreenToWrite = 1;
                                Mode = POP_ON;
								// Only reset on the first pop_on command
                                if (!bCaptureText) memset(&Screens[1],0,sizeof(TCCScreen));
                                memcpy(&CurrentState, &ResetState, sizeof(TCCChar));
                                CursorPos = 0;
                                CursorRow = 14;
                                bPaintNow = FALSE;
                                LastIndent = -1;
                                bCaptureText = TRUE;
                            }
                            break;

                        case 0x21: //backspace
                            if(bCaptureText == TRUE)
                            {
                                if(CursorPos > 0)
                                {
                                    CursorPos--;
                                }
                                Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].bIsActive = FALSE;
                                bPaintNow = TRUE;
                            }
                            break;

                        case 0x24: //delete to end of row
                            if(bCaptureText == TRUE)
                            {
                                for(x = CursorPos; x < CC_CHARS_PER_LINE; x++)
                                {
                                    Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].bIsActive = FALSE;
                                }
                                bPaintNow = TRUE;
                            }
                            break;

                        // reset screen becase we've gone into roll up Mode
                        case 0x25: //2 row caption
                        case 0x26: //3 row caption
                        case 0x27: //4 row caption
                            if(CaptionMode)
                            {
                                Mode = ROLL_UP;
                                ModeRows = b2 - 0x23;
                                ScreenToWrite = 0;
                                memcpy(&CurrentState, &ResetState, sizeof(TCCChar));
                                CursorPos = 0;
                                CursorRow = 14;
                                bPaintNow = TRUE;
                                bCaptureText = TRUE;
                            }
                            break;

                        case 0x28: //flash on
                            if(bCaptureText == TRUE)
                            {
                                CurrentState.bFlash = TRUE;
                                memcpy(&Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos], &CurrentState, sizeof(TCCChar));
                                Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].Text = '\0';
                                if(CursorPos < CC_CHARS_PER_LINE - 1) CursorPos++;
                            }
                            break;

                        // reset screen because we've gone into paint on Mode
                        case 0x29: //resume direct caption
                            if(CaptionMode)
                            {
                                ScreenToWrite = 0;
                                Mode = PAINT_ON;
                                memset(&Screens[0],0,sizeof(TCCScreen));
                                memcpy(&CurrentState, &ResetState, sizeof(TCCChar));
                                CursorPos = 0;
                                CursorRow = 14;
                                bPaintNow = TRUE;
                                bCaptureText = TRUE;
                            }
                            break;

                        case 0x2C: //erase displayed memory
                            if(CaptionMode)
                            {
                                memset(&Screens[0],0,sizeof(TCCScreen));
                                bPaintNow = TRUE;
                            }
                            break;

                        case 0x2E: //erase non-displayed memory
                            if(CaptionMode)
                            {
                                memset(&Screens[1],0,sizeof(TCCScreen));
                                LastIndent = -1;
                            }
                            break;

                        case 0x2A: //text restart
                            if(!CaptionMode)
                            {
                                Mode = TEXT;
                                ModeRows = 9;
                                memset(&Screens[0],0,sizeof(TCCScreen));
                                ScreenToWrite = 0;
                                memcpy(&CurrentState, &ResetState, sizeof(TCCChar));
                                CursorPos = 0;
                                CursorRow = 6;
                                bPaintNow = TRUE;
                                bCaptureText = TRUE;
                            }
                            break;

                        case 0x2B: //resume text display
                            if(!CaptionMode)
                            {
                                bCaptureText = TRUE;
                            }
                            break;
                            
                        case 0x2D: //carriage return
                            if(bCaptureText == TRUE)
                            {
                                if(Mode == TEXT || Mode == ROLL_UP)
                                {
                                    if(CursorRow < 14)
                                    {
                                        CursorRow++;
                                    }
                                    else
                                    {
                                        for(x = 14 - ModeRows + 1; x < 14; x++)
                                        {
                                            memcpy(Screens[ScreenToWrite].ScreenData[x], Screens[ScreenToWrite].ScreenData[x + 1], CC_CHARS_PER_LINE * sizeof(TCCChar));
                                        }
                                        CursorRow = 14;
                                    }
                                    memset(Screens[ScreenToWrite].ScreenData[x], 0 ,CC_CHARS_PER_LINE * sizeof(TCCChar));
                                    memcpy(&CurrentState, &ResetState, sizeof(TCCChar));
                                    CursorPos = 0;
                                }
                            }
                            break;
                        
                        case 0x2F: //end caption + swap memory
                            if(CaptionMode)
                            {
                                memcpy(Screens, Screens + 1, sizeof(TCCScreen));
                                memcpy(&CurrentState, &ResetState, sizeof(TCCChar));
                                ScreenToWrite = 0;
                                CursorPos = 0;
                                CursorRow = 14;
                                bPaintNow = TRUE;
                                LastIndent = -1;
                            }
                            break;
                    }
                    break;
                case 0x07:  //misc (TAB)
                    if(bCaptureText == TRUE)
                    {
                        switch(b2)
                        {
                        case 0x23:
                            Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].bIsActive = FALSE;
                            if(CursorPos < CC_CHARS_PER_LINE - 1) CursorPos++;
                            // FALL 
                        case 0x22:
                            Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].bIsActive = FALSE;
                            if(CursorPos < CC_CHARS_PER_LINE - 1) CursorPos++;
                            // FALL 
                        case 0x21:
                            Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].bIsActive = FALSE;
                            if(CursorPos < CC_CHARS_PER_LINE - 1) CursorPos++;
                            break;

                        case 0x2F:
                            CurrentState.bUnderline = TRUE;
                            // FALL 
                        case 0x2E:
                            CurrentState.ForeColor = CC_BLACK;
                            memcpy(&Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos], &CurrentState, sizeof(TCCChar));
                            Screens[ScreenToWrite].ScreenData[CursorRow][CursorPos].Text = ' ';
                            if(CursorPos < CC_CHARS_PER_LINE - 1) CursorPos++;
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }
    if(bPaintNow)
    {
        PostMessage(hWnd, WM_COMMAND, IDM_OSD_CC_TEXT, (LPARAM)Screens);
    }
    LastCode=data;
    return 0;
}

int CC_DecodeLine(BYTE* vbiline, eCCMode CCMode, BOOL IsOdd)
{
    if(!IsOdd)
    {
        switch(CCMode)
        {
        case CCMODE_CC1:
            CCdecode(decode(vbiline), TRUE, 0);
            break;
        case CCMODE_CC2:
            CCdecode(decode(vbiline), TRUE, 1);
            break;
        case CCMODE_TEXT1:
            CCdecode(decode(vbiline), FALSE, 0);
            break;
        case CCMODE_TEXT2:
            CCdecode(decode(vbiline), FALSE, 1);
            break;
        default:
            break;
        }
    }
    else
    {
        switch(CCMode)
        {
        case CCMODE_CC3:
            CCdecode(decode(vbiline), TRUE, 0);
            break;
        case CCMODE_CC4:
            CCdecode(decode(vbiline), TRUE, 1);
            break;
        case CCMODE_TEXT3:
            CCdecode(decode(vbiline), FALSE, 0);
            break;
        case CCMODE_TEXT4:
            CCdecode(decode(vbiline), FALSE, 1);
            break;
        default:
            break;
        }
    }
    return 0;
}

int XDS_DecodeLine(BYTE* vbiline)
{
    XDSdecode(decode(vbiline));
    return 0;
}

COLORREF CC_GetColor(eCCColor Color)
{
    switch(Color)
    {
    case CC_GREEN:
        return RGB(0,255,0);
    case CC_BLUE:
        return RGB(0,0,255);
    case CC_CYAN:
        return RGB(0,255,255);
    case CC_RED:
        return RGB(255,0,0);
    case CC_YELLOW:
        return RGB(255,255,0);
    case CC_MAGENTA:
        return RGB(255,0,255);
    case CC_BLACK:
        return RGB(0,0,0);
    default:
    case CC_WHITE:
        return RGB(255,255,255);
    }
}


void CC_PaintChars(HWND hWnd, TCCChar* Char, char* szLine, HDC hDC, RECT* PaintRect, int nFontsize)
{
    SIZE sizeText;
    HFONT hTmp, hOSDfont;

    if (hDC != NULL)
    {
        hOSDfont = CreateFont(nFontsize, 0, 0, 0, 
                                (Char->bFlash)? FW_BOLD:0, 
                                Char->bItalics, 
                                Char->bUnderline, 
                                0, 0, 0, 0, 
                                NONANTIALIASED_QUALITY, 
                                DEFAULT_PITCH | FF_DONTCARE, 
                                "Arial");
        if (!hOSDfont) return;

        hTmp = (HFONT)SelectObject(hDC, hOSDfont);
        
        if(hTmp)
        {
            GetTextExtentPoint32(hDC, szLine, strlen(szLine), &sizeText);

            SetBkMode(hDC, TRANSPARENT);
            SetTextColor(hDC, CC_GetColor(Char->BackColor));

            TextOut(hDC, PaintRect->left - 2, PaintRect->top, szLine, strlen(szLine));
            TextOut(hDC, PaintRect->left + 2, PaintRect->top, szLine, strlen(szLine));
            TextOut(hDC, PaintRect->left, PaintRect->top - 2, szLine, strlen(szLine));
            TextOut(hDC, PaintRect->left, PaintRect->top + 2, szLine, strlen(szLine));
            
            SetTextColor(hDC, CC_GetColor(Char->ForeColor));
            TextOut(hDC, PaintRect->left, PaintRect->top, szLine, strlen(szLine));

            PaintRect->left += sizeText.cx;

            SelectObject(hDC, hTmp);
            DeleteObject(hOSDfont);
        }
    }
}


void CC_PaintLine(HWND hWnd, TCCChar* Line, HDC hDC, RECT* PaintRect, int nFontsize)
{
    int i;
    BOOL bAnyText = FALSE;
    int Count = 0;
    int LineWidth = (PaintRect->right - PaintRect->left);
    char szString[CC_CHARS_PER_LINE + 1];
    TCCChar* LastChar = NULL;
    int StrPos = 0;

    for(i = 0; i < CC_CHARS_PER_LINE; i++)
    {
        if(Line[i].bIsActive)
        {
            if(!bAnyText && Count == 0)
            {
                bAnyText = TRUE;
                PaintRect->left = PaintRect->left + LineWidth * (i + 2) / (CC_CHARS_PER_LINE + 4);
            }
            
            if(LastChar != NULL &&
                (LastChar->BackColor != Line[i].BackColor ||
                 LastChar->ForeColor != Line[i].ForeColor ||
                 LastChar->bItalics != Line[i].bItalics ||
                 LastChar->bUnderline != Line[i].bUnderline ||
                 LastChar->bFlash != Line[i].bFlash))
            {
                szString[StrPos] = '\0';
                CC_PaintChars(hWnd, LastChar, szString, hDC, PaintRect, nFontsize);
                StrPos = 0;
            }
            
            // cope with changes from straight ASCII
            switch(Line[i].Text)
            {
            case '`':
                szString[StrPos++] = '�';
                break;
            case '*':
                szString[StrPos++] = '�';
                break;
            case '\\':
                szString[StrPos++] = '�';
                break;
            case '^':
                szString[StrPos++] = '�';
                break;
            case '_':
                szString[StrPos++] = '�';
                break;
            case '{':
                szString[StrPos++] = '�';
                break;
            case '|':
                szString[StrPos++] = '�';
                break;
            case '}':
                szString[StrPos++] = '�';
                break;
            case '~':
                szString[StrPos++] = '�';
                break;
            default:
                szString[StrPos++] = Line[i].Text;
                break;
            }
            LastChar = Line + i;
            Count++;
        }
        else
        {
            if(StrPos > 0)
            {
                szString[StrPos] = '\0';
                CC_PaintChars(hWnd, LastChar, szString, hDC, PaintRect, nFontsize);
                StrPos = 0;
            }
            bAnyText = FALSE;
            LastChar = NULL;
        }
    }

    if(StrPos > 0)
    {
        szString[StrPos] = '\0';
        CC_PaintChars(hWnd, LastChar, szString, hDC, PaintRect, nFontsize);
    }
}

void CC_PaintScreen(HWND hWnd, TCCScreen* Screen, HDC hDC, RECT* PaintRect)
{
    int i;
    RECT LineRect;
    int nFontsize;
    
    if (hDC != NULL)
    {
        nFontsize = (PaintRect->bottom - PaintRect->top) / 17;
        if(nFontsize <= 1)
        {
            return;
        }

        for(i = 0; i < 15; i++)
        {
            LineRect.top = PaintRect->top + ((PaintRect->bottom - PaintRect->top) * (i + 1)) / 17;
            LineRect.bottom = PaintRect->top + ((PaintRect->bottom - PaintRect->top) * (i + 2)) / 17;
            LineRect.left = PaintRect->left;
            LineRect.right = PaintRect->right;
            CC_PaintLine(hWnd, &Screen->ScreenData[i][0], hDC, &LineRect, nFontsize);
        }
    }
}

