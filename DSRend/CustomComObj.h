/////////////////////////////////////////////////////////////////////////////
// $Id: CustomComObj.h,v 1.1.1.1 2002-02-03 10:52:53 tobbej Exp $
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
 * @file CustomComObj.h Declaration of the CCustomComObject
 */

/**
 * Custom CComObject class.
 * This class is similar to the ATL class CComObject, its used on the input pin to delegate 
 * addref/release to the filter itself.
 * It is also used to send a pointer to the filter to the input pin
 */
template<typename base,typename paramType>
class CCustomComObject : public base
{
public:
	CCustomComObject(paramType pFilter)
		:base(pFilter)
	{
	}

	~CCustomComObject()
	{
		FinalRelease();
	}
	
	ULONG STDMETHODCALLTYPE AddRef()
	{
		//delegate addref to filter
		return m_pFilter->AddRef();
	}

	ULONG STDMETHODCALLTYPE Release()
	{
		//delegate release to filter
		return m_pFilter->Release();
	}
	
	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject)
	{
		return _InternalQueryInterface(iid, ppvObject);
	}
};