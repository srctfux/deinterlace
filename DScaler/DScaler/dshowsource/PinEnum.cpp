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
 * @file PinEnum.cpp implementation of the CDShowPinEnum class.
 */


#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT
#include "PinEnum.h"
#include "exception.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDShowPinEnum::CDShowPinEnum(CComPtr<IBaseFilter> filter,PIN_DIRECTION pinDir)
:m_anydir(false)
{
    ASSERT(filter!=NULL);
    ASSERT(pinDir==PINDIR_INPUT || pinDir==PINDIR_OUTPUT);

    CComPtr<IEnumPins> pEnum;
    HRESULT hr=filter->EnumPins(&pEnum);
    if(FAILED(hr))
        throw CDShowPinEnumException("Failed to create pin enumerator",hr);
    
    ASSERT(pEnum!=NULL);
    init(pEnum);
    
    m_pinDir=pinDir;
}

CDShowPinEnum::CDShowPinEnum(CComPtr<IBaseFilter> filter)
:m_anydir(true)
{
    ASSERT(filter!=NULL);
    CComPtr<IEnumPins> pEnum;
    HRESULT hr=filter->EnumPins(&pEnum);
    if(FAILED(hr))
        throw CDShowPinEnumException("Failed to create pin enumerator",hr);
    
    ASSERT(pEnum!=NULL);
    init(pEnum);
}

CDShowPinEnum::~CDShowPinEnum()
{

}

//////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////

CComPtr<IPin> CDShowPinEnum::next()
{
    CComPtr<IPin> pin;
    HRESULT hr=S_OK;

    //only pins in specified direction?
    if(m_anydir)
    {
        hr=CDShowGenericEnum<IEnumPins,IPin>::next(&pin);
        if(FAILED(hr) || pin==NULL)
            return NULL;
            //throw CExPinEnum("next() failed",hr);
    }
    else
    {
        //step thru all pins until one with the right direction is found
        while(true)
        {
            hr=CDShowGenericEnum<IEnumPins,IPin>::next(&pin);
            if(FAILED(hr) || pin==NULL)
                return NULL;
                //throw CExPinEnum("next() failed",hr);
            
            PIN_DIRECTION pinDir;
            hr=pin->QueryDirection(&pinDir);
            if(SUCCEEDED(hr) && pinDir==m_pinDir)
            {
                break;
            }
            else
            {
                //releas the pin becaus it was eighter wrong direction or some error
                //if we dont, there will be an assert trigered in CComPtr<>
                pin.Release();
            }
        }
    }
    return pin;
}
#endif