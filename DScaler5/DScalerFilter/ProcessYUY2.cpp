///////////////////////////////////////////////////////////////////////////////
// $Id: ProcessYUY2.cpp,v 1.3 2003-05-09 16:02:47 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
// DScalerFilter.dll - DirectShow filter for deinterlacing and video processing
// Copyright (c) 2003 John Adcock
///////////////////////////////////////////////////////////////////////////////
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
///////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.2  2003/05/09 07:03:26  adcockj
// Bug fixes for new format code
//
// Revision 1.1  2003/05/08 15:58:38  adcockj
// Better error handling, threading and format support
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Process.h"

///////////////////////////////////////////////////////////////////////////////
// Process the YUY2 samples
// 4:2:2 samples go:
// Y0 U0 Y1 V0 Y2 U2 Y3 V2 etc
///////////////////////////////////////////////////////////////////////////////

void FillScreenWithAlternatingLines(int Lines, BITMAPINFOHEADER* InputBMI, BITMAPINFOHEADER* OutputBMI, BYTE* pInBuffer, BYTE* pOutBuffer)
{
    for(int i(0); i < Lines; ++i)
    {
        // black out first 8 pixels if we have a 
        // 704 width source that we are extending up to 720
        if(InputBMI->biWidth == 704)
        {
            for(int i(0); i < 8; ++i)
            {
                *(pOutBuffer++) = 0;
                *(pOutBuffer++) = 128;
            }
        }

        if(i & 1)
        {
            for(int j(0); j < InputBMI->biWidth; ++j)
            {
                *(pOutBuffer++) = 16;
                *(pOutBuffer++) = 128;
            }
        }
        else
        {
            for(int j(0); j < InputBMI->biWidth; ++j)
            {
                *(pOutBuffer++) = 235;
                *(pOutBuffer++) = 128;
            }
        }
        
        // black out last 8 pixels if we have a 
        // 704 width source that we are extending up to 720
        if(InputBMI->biWidth == 704)
        {
            for(int i(0); i < 8; ++i)
            {
                *(pOutBuffer++) = 0;
                *(pOutBuffer++) = 128;
            }
            pOutBuffer -= 720 * 2;
        }
        else
        {
            pOutBuffer -= InputBMI->biWidth * 2;
        }

        pOutBuffer += OutputBMI->biWidth * 2;
        pInBuffer += InputBMI->biWidth * 2;
    }

}


void ProcessYUY2(int Lines, BITMAPINFOHEADER* InputBMI, BITMAPINFOHEADER* OutputBMI, BYTE* pInBuffer, BYTE* pOutBuffer)
{
    // test code -f or trying to work out what format the VMR9 is expecting
    //FillScreenWithAlternatingLines(Lines, InputBMI, OutputBMI, pInBuffer, pOutBuffer);
    //return;

    for(int i(0); i < Lines; ++i)
    {
        // black out first 8 pixels if we have a 
        // 704 width source that we are extending up to 720
        if(InputBMI->biWidth == 704)
        {
            for(int i(0); i < 8; ++i)
            {
                *(pOutBuffer++) = 0;
                *(pOutBuffer++) = 128;
            }
        }
        
        memcpy(pOutBuffer, pInBuffer, InputBMI->biWidth * 2);
        
        // black out last 8 pixels if we have a 
        // 704 width source that we are extending up to 720
        if(InputBMI->biWidth == 704)
        {
            pOutBuffer += 704 * 2;
            for(int i(0); i < 8; ++i)
            {
                *(pOutBuffer++) = 0;
                *(pOutBuffer++) = 128;
            }
            pOutBuffer -= 720 * 2;
        }

        pOutBuffer += OutputBMI->biWidth * 2;
        pInBuffer += InputBMI->biWidth * 2;
    }
}
