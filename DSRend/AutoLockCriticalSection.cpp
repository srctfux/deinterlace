/////////////////////////////////////////////////////////////////////////////
// $Id: AutoLockCriticalSection.cpp,v 1.2 2002-07-06 16:43:26 tobbej Exp $
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
// Revision 1.1  2002/06/03 18:19:41  tobbej
// moved CAutoLockCriticalSection
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file AutoLockCriticalSection.cpp implementation of the CAutoLockCriticalSection class.
 */

#include "stdafx.h"
#include "AutoLockCriticalSection.h"

CAutoLockCriticalSection::CAutoLockCriticalSection(CComAutoCriticalSection *pLock)
{
	m_pLock=pLock;
	m_pLock->Lock();
}

CAutoLockCriticalSection::~CAutoLockCriticalSection()
{
	m_pLock->Unlock();
}
