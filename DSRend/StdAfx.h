/////////////////////////////////////////////////////////////////////////////
// $Id: StdAfx.h,v 1.2 2002-07-06 16:44:48 tobbej Exp $
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
// Revision 1.1.1.1  2002/02/03 10:52:53  tobbej
// First import of new direct show renderer filter
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file StdAfx.h include file for standard system include files,
 *      or project specific include files that are used frequently,
 *      but are changed infrequently
 */

#if !defined(AFX_STDAFX_H__302465F6_417F_4A05_AE72_7C575630C302__INCLUDED_)
#define AFX_STDAFX_H__302465F6_417F_4A05_AE72_7C575630C302__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_FREE_THREADED

//ATL debuging debuging
#ifdef _DEBUG

//hmm... why does _ATL_DEBUG_INTERFACES create problems with the input pin?
//looks like _QIThunk is deleted when it shoud not, why?
////#define _ATL_DEBUG_INTERFACES	//reference count debuging
//#define ATL_TRACE_LEVEL 2
//#define _ATL_DEBUG_QI			//queryinterface debugging
#endif

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>

//DShow stuff
#include <strmif.h>		//if you get IAMFilterMiscFlags is undefined, you are using the wrong version of strmif.h (the one that comes with vc, not directx sdk)
#include <amvideo.h>
#include <uuids.h>
#include <vfwmsgs.h>
#include <evcode.h>		//event codes

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__302465F6_417F_4A05_AE72_7C575630C302__INCLUDED)
