/////////////////////////////////////////////////////////////////////////////
// $Id: Event.h,v 1.1.1.1 2002-02-03 10:52:53 tobbej Exp $
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
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file Event.h: interface for the CEvent class.
 */


#if !defined(AFX_EVENT_H__F3EE92CB_5ED0_4B79_9A68_03C761B533A6__INCLUDED_)
#define AFX_EVENT_H__F3EE92CB_5ED0_4B79_9A68_03C761B533A6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**
 * Class for managing a win32 event object.
 */
class CEvent
{
public:
	CEvent(BOOL bManualReset=FALSE,BOOL bInitialState=FALSE,LPCTSTR lpszName=NULL);
	virtual ~CEvent();
	
	/// sets the event to signaled state
	BOOL setEvent();
	/// sets the event to unsignaled state
	BOOL resetEvent();
	/// @return true if event was signaled before timeout
	bool wait(DWORD dwMilliseconds);
	
	/// @return handle to win32 event object
	HANDLE getHandle(){return m_hEvent;}

private:
	///handle to event
	HANDLE m_hEvent;
};

#endif // !defined(AFX_EVENT_H__F3EE92CB_5ED0_4B79_9A68_03C761B533A6__INCLUDED_)
