/////////////////////////////////////////////////////////////////////////////
// $Id: DShowFileSource.cpp,v 1.2 2002-04-03 19:54:28 tobbej Exp $
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
// Revision 1.1  2002/02/07 22:05:43  tobbej
// new classes for file input
// rearanged class inheritance a bit
//
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
#include "PinEnum.h"

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
	//the simple case, RenderStream is able to properly connect the filters
	hr=m_pBuilder->RenderStream(NULL,NULL,m_pFileSource,NULL,filter);
	if(FAILED(hr))
	{
		//that didnt work, try to manualy connect the pins on the source filter
		CDShowPinEnum outPins(m_pFileSource,PINDIR_OUTPUT);
		CComPtr<IPin> outPin;
		bool bSucceeded=false;
		while(outPin=outPins.next(),bSucceeded==false && outPin!=NULL)
		{
			CDShowPinEnum inPins(filter,PINDIR_INPUT);
			CComPtr<IPin> inPin;
			while(inPin=inPins.next(),inPin!=NULL)
			{
				hr=m_pGraph->Connect(outPin,inPin);
				if(SUCCEEDED(hr))
				{
					//now the video is connected properly

					//render the other pins if any, it is not a big problem if it fails,
					//just continue with the stream we already got
					outPins.reset();
					CComPtr<IPin> pin;
					while(pin=outPins.next(),pin!=NULL)
					{
						CComPtr<IPin> dest;
						hr=pin->ConnectedTo(&dest);
						if(FAILED(hr))
						{
							m_pGraph->Render(pin);
						}
					}

					bSucceeded=true;
					break;
				}
			}
		}
		if(!bSucceeded)
		{
			throw CDShowException("Cant connect filesource to renderer",hr);
		}
	}
	//try to render audio, if this fails then this file probably dont have any audio
	hr=m_pBuilder->RenderStream(NULL,&MEDIATYPE_Audio,m_pFileSource,NULL,NULL);
	
	m_bIsConnected=true;
}

#endif