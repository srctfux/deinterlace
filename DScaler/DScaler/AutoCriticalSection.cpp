/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Torbj?rn Jansson.  All rights reserved.
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

/**
 * @file AutoCriticalSection.cpp implementation of the CAutoCriticalSection class.
 */

#include "stdafx.h"
#include "dscaler.h"
#include "AutoCriticalSection.h"

CAutoCriticalSection::CAutoCriticalSection(CRITICAL_SECTION &pCriticalSection)
:m_pCriticalSection(&pCriticalSection)
{
    _ASSERTE(m_pCriticalSection!=NULL);
    EnterCriticalSection(m_pCriticalSection);
}

CAutoCriticalSection::~CAutoCriticalSection()
{
    LeaveCriticalSection(m_pCriticalSection);
}
