/////////////////////////////////////////////////////////////////////////////
// $Id: Event.cpp,v 1.1.1.1 2002-02-03 10:52:53 tobbej Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Torbjörn Jansson.  All rights reserved.
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
/////////////////////////////////////////////////////////////////////////////

/**
 * @file Event.cpp implementation of the CEvent class.
 */

#include "stdafx.h"
#include "Event.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEvent::CEvent(BOOL bManualReset,BOOL bInitialState,LPCTSTR lpszName)
{
	m_hEvent=CreateEvent(NULL,bManualReset,bInitialState,lpszName);
	ATLASSERT(m_hEvent!=NULL);
}

CEvent::~CEvent()
{
	CloseHandle(m_hEvent);
}

BOOL CEvent::setEvent()
{
	return ::SetEvent(m_hEvent);
}

BOOL CEvent::resetEvent()
{
	return ::ResetEvent(m_hEvent);
}

bool CEvent::wait(DWORD dwMilliseconds)
{
	return (WaitForSingleObject(m_hEvent,dwMilliseconds)==WAIT_OBJECT_0);
}