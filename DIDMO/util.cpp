/////////////////////////////////////////////////////////////////////////////
// $Id: util.cpp,v 1.1.1.1 2001-07-30 16:14:44 tobbej Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Torbjörn Jansson.  All rights reserved.
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
//  GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <dmo.h>
#include <amvideo.h>
#include <uuids.h>
#include "util.h"

bool TypesMatch(const DMO_MEDIA_TYPE *pmt1, const DMO_MEDIA_TYPE *pmt2)
{
	LPBITMAPINFOHEADER pb1;
	LPBITMAPINFOHEADER pb2;
	if(pmt1->majortype==pmt2->majortype && pmt1->subtype==pmt2->subtype)
	{
		if(pmt1->formattype==FORMAT_VideoInfo)
		{
			pb1=&(((VIDEOINFOHEADER *)pmt1->pbFormat)->bmiHeader);
			pb2=&(((VIDEOINFOHEADER *)pmt2->pbFormat)->bmiHeader);
			
			//abs() is needed becaus overlaymixer uses negative height/width
			//and an incorect DMO_MEDIA_TYPE.lSampleSize
			if(pb1->biBitCount==pb2->biBitCount &&
				abs(pb1->biWidth)==abs(pb2->biWidth) &&
				abs(pb1->biHeight)==abs(pb2->biHeight) &&
				pb1->biCompression==pb2->biCompression &&
				pb1->biSizeImage==pb2->biSizeImage)
				return true;
		}
		
		//more exact
		if(pmt1->lSampleSize==pmt2->lSampleSize &&
			pmt1->formattype==pmt2->formattype &&
			pmt1->cbFormat==pmt2->cbFormat &&
			0 == memcmp(pmt1->pbFormat, pmt2->pbFormat, pmt1->cbFormat))
			return true;
	}
	return false;
}
