/////////////////////////////////////////////////////////////////////////////
// $Id: DShowFileSource.cpp,v 1.1 2002-02-07 22:05:43 tobbej Exp $
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
 * @file DShowFileSource.cpp implementation of the CDShowFileSource class.
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT

#include "dscaler.h"
#include "DShowFileSource.h"
#include "exception.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CDShowFileSource::CDShowFileSource(IGraphBuilder *pGraph,string filename)
:CDShowBaseSource(pGraph),m_file(filename),m_bIsConnected(false)
{
	USES_CONVERSION;

	HRESULT hr=m_pGraph->AddSourceFilter(A2W(filename.c_str()),NULL,&m_pFileSource);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to add file",hr);
	}

	hr=m_pBuilder.CoCreateInstance(CLSID_CaptureGraphBuilder2);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to create capture graph builder",hr);
	}
	hr=m_pBuilder->SetFiltergraph(m_pGraph);
	if(FAILED(hr))
	{
		throw CDShowException("SetFiltergraph failed on capture graph builder",hr);
	}
}

CDShowFileSource::~CDShowFileSource()
{

}

void CDShowFileSource::connect(CComPtr<IBaseFilter> filter)
{
	HRESULT hr;
	hr=m_pBuilder->RenderStream(NULL,NULL,m_pFileSource,NULL,filter);
	if(FAILED(hr))
	{
		throw CDShowException("Cant connect filesource to renderer",hr);
	}
	hr=m_pBuilder->RenderStream(NULL,&MEDIATYPE_Audio,m_pFileSource,NULL,NULL);
	//MEDIATYPE_Audio
	m_bIsConnected=true;
}

#endif