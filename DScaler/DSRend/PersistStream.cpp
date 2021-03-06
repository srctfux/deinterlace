/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Torbj?rn Jansson.  All rights reserved.
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
 * @file PersistStream.cpp implementation of the CPersistStream class.
 */

#include "stdafx.h"
#include "dsrend.h"
#include "PersistStream.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CPersistStream::CPersistStream()
:m_dwVersion(0),m_bIsDirty(false)
{

}

CPersistStream::~CPersistStream()
{

}

HRESULT CPersistStream::IsDirty()
{
    return m_bIsDirty ? S_OK : S_FALSE;
}

HRESULT CPersistStream::Load(IStream *pStm)
{
    HRESULT hr=pStm->Read(&m_dwVersion,sizeof(DWORD),NULL);
    if(FAILED(hr))
    {
        return hr;
    }
    return LoadFromStream(pStm);
}

HRESULT CPersistStream::Save(IStream *pStm, BOOL fClearDirty)
{
    DWORD dwVer=GetVersion();
    HRESULT hr=pStm->Write(&dwVer,sizeof(DWORD),NULL);
    if(FAILED(hr))
    {
        return hr;
    }
    hr=SaveToStream(pStm);
    if(SUCCEEDED(hr) && fClearDirty!=FALSE)
    {
        SetDirty(false);
    }
    return hr;
}

HRESULT CPersistStream::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    if(pcbSize==NULL)
    {
        return E_POINTER;
    }
    pcbSize->QuadPart=sizeof(DWORD)+GetSize();
    return S_OK;
}

void CPersistStream::SetDirty(bool bDirty)
{
    m_bIsDirty=bDirty;
}

DWORD CPersistStream::GetVersion()
{
    return 0;
}