/////////////////////////////////////////////////////////////////////////////
// $Id: HistoryData.h,v 1.1 2001-08-08 15:37:02 tobbej Exp $
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
// Revision 1.1.1.1  2001/07/30 16:14:44  tobbej
// initial import of new dmo filter
//
//
//////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_HISTORYDATA_H__9E04B30E_EF9D_4AC0_BADD_08A3EBAA6EB0__INCLUDED_)
#define AFX_HISTORYDATA_H__9E04B30E_EF9D_4AC0_BADD_08A3EBAA6EB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <dmo.h>
#include <limits.h>

static const REFERENCE_TIME INVALID_TIME = _I64_MAX;

class CHistoryData
{
public:
	CHistoryData();
	void Reset();

	CComPtr<IMediaBuffer> m_Buffer;	//the buffer
	DWORD m_flags;					//flags
	REFERENCE_TIME m_rtTimestamp;	//timestamt
	REFERENCE_TIME m_rtTimelength;	//length
};

#endif // !defined(AFX_HISTORYDATA_H__9E04B30E_EF9D_4AC0_BADD_08A3EBAA6EB0__INCLUDED_)
