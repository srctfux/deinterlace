/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 Torbj?rn Jansson.  All rights reserved.
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
 * @fileDShowSeeking.cpp implementation of the CDShowSeeking class.
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT

#include "dscaler.h"
#include "DShowSeeking.h"
#include "exception.h"

CDShowSeeking::CDShowSeeking(CComPtr<IGraphBuilder> &pGraph)
{
    _ASSERTE(pGraph!=NULL);
    HRESULT hr=pGraph.QueryInterface(&m_pSeeking);
    if(FAILED(hr))
    {
        m_pSeeking.Release();
        throw CDShowException("Failed to find IMediaSeeking",hr);
    }

    //we can only work with TIME_FORMAT_MEDIA_TIME
    if(m_pSeeking->IsFormatSupported(&TIME_FORMAT_MEDIA_TIME)==S_OK)
    {
        hr=m_pSeeking->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
        if(FAILED(hr))
        {
            m_pSeeking.Release();
            throw CDShowException("IMediaSeeking::SetTimeFormat failed",hr);
        }
    }
    else
    {
        m_pSeeking.Release();
        throw CDShowException("IMediaSeeking doesn't support the required time format");
    }
}

CDShowSeeking::~CDShowSeeking()
{

}

DWORD CDShowSeeking::GetCaps()
{
    DWORD dwCaps;
    HRESULT hr=m_pSeeking->GetCapabilities(&dwCaps);
    if(FAILED(hr))
    {
        throw CDShowException("IMediaSeeking::GetCapabilities failed",hr);
    }
    return dwCaps;
}

LONGLONG CDShowSeeking::GetCurrentPos()
{
    LONGLONG CurrentPos;
    HRESULT hr=m_pSeeking->GetCurrentPosition(&CurrentPos);
    if(FAILED(hr))
    {
        throw CDShowException("IMediaSeeking::GetCurrentPosition failed",hr);
    }
    return CurrentPos;
}

LONGLONG CDShowSeeking::GetDuration()
{
    LONGLONG Duration;
    HRESULT hr=m_pSeeking->GetDuration(&Duration);
    if(FAILED(hr))
    {
        throw CDShowException("IMediaSeeking::GetDuration failed",hr);
    }
    return Duration;
}

void CDShowSeeking::SeekTo(LONGLONG position)
{
    LONGLONG pos=position;
    HRESULT hr=m_pSeeking->SetPositions(&pos,AM_SEEKING_AbsolutePositioning,NULL,AM_SEEKING_NoPositioning);
    if(FAILED(hr))
    {
        throw CDShowException("IMediaSeeking::SetPositions failed",hr);
    }
}

#endif