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
 * @file DSProvider.cpp implementation of the CDSProvider class.
 */


#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT
//#include "dscaler.h"
#include "DSProvider.h"
#include "devenum.h"
#include "CaptureDevice.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CDSProvider::CDSProvider()
{
    std::string ErrMsg;
    if(!CanUseDShow(ErrMsg))
    {
        //the only reason for the extra \n is to avoid text on the osd being drawn outside the window
        //(== some text will not be visible)
        std::string msg="Can't use DirectShow support because\n"+ErrMsg+"\nThe DirectShow input sources will be disabled";
        ErrorBox(msg.c_str());
        return;
    }
    
    try
    {
        try
        {
            CDShowDevEnum devenum(CLSID_VideoInputDeviceCategory);
            //get all video capture devices
            while(devenum.getNext()==true)
            {
                string deviceName=devenum.getProperty("FriendlyName");
                CDSCaptureSource *tmpsrc=new CDSCaptureSource(devenum.getDisplayName(),deviceName);
                m_DSSources.push_back(tmpsrc);
                m_SourceNames[m_DSSources.size()-1]=deviceName;
            }
        }
        catch(CDShowDevEnumException&)
        {
            //creation of device enumerator failed, this probably means that CLSID_VideoInputDeviceCategory is empty/non existant
        }

        //add one file source
        CDSFileSource *src=new CDSFileSource();
        m_DSSources.push_back(src);
        m_SourceNames[m_DSSources.size()-1]="Media file";
    }
    catch(std::runtime_error& e)
    {
        ErrorBox(e.what());
    }
    catch(CDShowException& e)
    {
        ErrorBox(e.getErrorText());
    }
    catch(...)
    {
        ErrorBox("Unexpected Error");
    }
}

CDSProvider::~CDSProvider()
{
}

string CDSProvider::GetSourceName(int SourceIndex)
{
    ASSERT(SourceIndex>=0 && SourceIndex<m_DSSources.size());
    return m_SourceNames[SourceIndex];
}

int CDSProvider::GetNumberOfSources()
{
    return m_DSSources.size();
}

SmartPtr<CSource> CDSProvider::GetSource(int SourceIndex)
{
    ASSERT(SourceIndex>=0 && SourceIndex<m_DSSources.size());

    if(SourceIndex>=0 && SourceIndex<m_DSSources.size())
    {
        return m_DSSources[SourceIndex];
    }
    else
    {
        return NULL;
    }
}

bool CDSProvider::CanUseDShow(std::string &FailMsg)
{
    ///@todo add a check for directx version

    //check dsrend.dll filter
    CComPtr<IBaseFilter> filter;
    HRESULT hr=filter.CoCreateInstance(CLSID_DSRendFilter);
    if(FAILED(hr))
    {
        ///@todo maybe try to register dsrend.dll and only return false if it failed
        FailMsg="the 'DScaler renderer filter' is not properly installed";
        return false;
    }
    CComPtr<IDSRendFilter> DSRendIf;
    hr=filter.QueryInterface(&DSRendIf);
    if(FAILED(hr))
    {
        FailMsg="the 'DScaler renderer filter' does not support the necessary interface\n(maybe the filter is of the wrong version)";
        return false;
    }
    return true;
}

#endif