/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Torbj�rn Jansson.  All rights reserved.
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
// Revision 1.2  2002/07/29 17:51:40  tobbej
// added vertical mirror.
// fixed field ordering and even/odd flags, seems like it is working
//
// Revision 1.1  2002/07/15 18:18:13  tobbej
// support for rgb24 input
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file ColorConverter.h interface for the CColorConverter class.
 */

#if !defined(AFX_COLORCONVERTER_H__DBCCBA49_09AE_4A34_915D_B342980FE078__INCLUDED_)
#define AFX_COLORCONVERTER_H__DBCCBA49_09AE_4A34_915D_B342980FE078__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**
 * Takes care of convertin a mediasample to YUY2.
 */
class CColorConverter
{
public:
	///enum describing how to convert a sample
	typedef enum COVERSION_FORMAT
	{
		///covert all of the sample	(field input)
		CNV_ALL,
		///convert and copy even lines only
		CNV_EVEN,
		///convert and copy odd lines only
		CNV_ODD
	};

	CColorConverter();
	virtual ~CColorConverter();
	
	///@return true if conversion from specified mediatype can be preformed.
	bool CanCovert(const AM_MEDIA_TYPE *mt);
	bool SetFormat(const AM_MEDIA_TYPE *mt);
	
	/**
	 * 
	 * @param dst
	 * @param src
	 * @param cnv
	 * @param bVertMirror true if sample shoud be fliped when converting
	 */
	bool Convert(BYTE *dst,BYTE *src,COVERSION_FORMAT cnv,bool &bVertMirror);

private:
	///function pinter to color conversion function
	BYTE *(*m_pfnConv)(short *dest,BYTE *src,DWORD w);
	long m_width;
	long m_height;
	WORD m_bitcount;
	
	///true if current format is normaly upside down.
	bool m_bNeedVertMirror;
};

#endif // !defined(AFX_COLORCONVERTER_H__DBCCBA49_09AE_4A34_915D_B342980FE078__INCLUDED_)
