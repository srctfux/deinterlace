/////////////////////////////////////////////////////////////////////////////
// $Id: HistoryData.cpp,v 1.1.1.1 2001-07-30 16:14:44 tobbej Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Torbj�rn Jansson.  All rights reserved.
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
#include "HistoryData.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CHistoryData::CHistoryData()
:m_rtTimestamp(INVALID_TIME),m_rtTimelength(INVALID_TIME),m_flags(0)
{

}

void CHistoryData::Reset()
{
	m_Buffer=NULL;
	m_flags=0;
	m_rtTimestamp=INVALID_TIME;
	m_rtTimelength=INVALID_TIME;
}