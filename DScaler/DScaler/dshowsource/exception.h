/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file exception.h interface for exception classes.
 */

#if !defined(AFX_EXCEPTION_H__3DCA2AE5_2EC6_405F_AE26_D7D1E8E0E2D0__INCLUDED_)
#define AFX_EXCEPTION_H__3DCA2AE5_2EC6_405F_AE26_D7D1E8E0E2D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**
 * Exception baseclass.
 */
class CDShowException : public std::exception
{
public:
    CDShowException(const char* msg,HRESULT hr = S_OK);
    HRESULT getErrNo() {return m_err;};

private:
    HRESULT m_err;            //error code
};

#endif // !defined(AFX_EXCEPTION_H__3DCA2AE5_2EC6_405F_AE26_D7D1E8E0E2D0__INCLUDED_)