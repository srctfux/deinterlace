/////////////////////////////////////////////////////////////////////////////
// $Id: DShowFileSource.cpp,v 1.9 2005-03-11 14:54:41 adcockj Exp $
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
// Revision 1.8  2002/09/24 17:18:14  tobbej
// support for files with multiple audio streams when using a user specified audio renderer
// changed some log messages
//
// Revision 1.7  2002/09/21 16:33:30  tobbej
// render all audio streams, this makes suround work
//
// Revision 1.6  2002/09/14 17:04:24  tobbej
// implemented audio output device selection.
// added a safety check for .grf files when copying settings from dsrend
//
// Revision 1.5  2002/08/11 14:03:16  tobbej
// preserve dsrend filter settings when opening grf files
//
// Revision 1.4  2002/08/01 20:22:13  tobbej
// improved error messages when opening files.
// corrected some smal problems when opening .grf files
//
// Revision 1.3  2002/07/29 17:42:53  tobbej
// support for opening graphedit saved filter graphs
//
// Revision 1.2  2002/04/03 19:54:28  tobbej
// modified connect() to try a little more before giving up on the file
//
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
#include "..\..\..\DSRend\DSRend.h"
#include "debuglog.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CDShowFileSource::CDShowFileSource(IGraphBuilder *pGraph,string filename)
:CDShowBaseSource(pGraph),m_file(filename),m_bIsConnected(false)
{
	USES_CONVERSION;
	HRESULT hr=m_pBuilder.CoCreateInstance(CLSID_CaptureGraphBuilder2);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to create capture graph builder",hr);
	}
	hr=m_pBuilder->SetFiltergraph(m_pGraph);
	if(FAILED(hr))
	{
		throw CDShowException("SetFiltergraph failed on capture graph builder",hr);
	}
	
	CString tmp;
	tmp=m_file.substr((m_file.size()<4 ? 0 : m_file.size()-4)).c_str();
	if(_tcsicmp(tmp.GetBuffer(0),_T(".grf"))!=0)
	{
		hr=m_pGraph->AddSourceFilter(A2W(filename.c_str()),NULL,&m_pFileSource);
		if(FAILED(hr))
		{
			throw CDShowException("Failed to add file",hr);
		}
	}
}

CDShowFileSource::~CDShowFileSource()
{

}

void CDShowFileSource::Connect(CComPtr<IBaseFilter> VideoFilter)
{
	USES_CONVERSION;
	HRESULT hr;
	
	//is this a grf file? grf files needs special handling
	CString tmp;
	tmp=m_file.substr((m_file.size()<4 ? 0 : m_file.size()-4)).c_str();
	if(_tcsicmp(tmp.GetBuffer(0),_T(".grf"))!=0)
	{
		//the simple case, RenderStream is able to properly connect the filters
		hr=m_pBuilder->RenderStream(NULL,NULL,m_pFileSource,NULL,VideoFilter);
		if(FAILED(hr))
		{
			//that didnt work, try to manualy connect the pins on the source filter
			CDShowPinEnum outPins(m_pFileSource,PINDIR_OUTPUT);
			CComPtr<IPin> outPin;
			bool bSucceeded=false;
			while(outPin=outPins.next(),bSucceeded==false && outPin!=NULL)
			{
				CDShowPinEnum inPins(VideoFilter,PINDIR_INPUT);
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
				throw CDShowUnsupportedFileException("Can't connect filesource to renderer",hr);
			}
		}

		//try to render audio, if this fails then this file probably don't have any audio
		bool bAudioRendered=false;
		int AudioStreamCount=0;
		/*
		Connect all audio streams, not sure if this is a good idea, but it 
		looks like IGraphBuilder::RenderFile also tries to connect all audio
		streams.
		This will always add one extra unconnected audio renderer when using
		a user specified audio renderer
		*/
		while(hr=m_pBuilder->RenderStream(NULL,&MEDIATYPE_Audio,m_pFileSource,NULL,GetNewAudioRenderer()),SUCCEEDED(hr))
		{
			bAudioRendered=true;
			AudioStreamCount++;
		}
		if(bAudioRendered)
		{
			LOG(2,"DShowFileSource: %d Audio streams rendered",AudioStreamCount);
		}
		else
		{
			LOG(2,"DShowFileSource: Unsupported audio or no audio found, error code: 0x%x",hr);
		}
	}
	else
	{
		hr=m_pGraph->RenderFile(A2W(m_file.c_str()),NULL);
		if(FAILED(hr))
		{
			throw CDShowException("Faild to render grapedit .grf file",hr);
		}

		CDShowGenericEnum<IEnumFilters,IBaseFilter> filterEnum;
		HRESULT hr=m_pGraph->EnumFilters(&filterEnum.m_pEnum);
		if(FAILED(hr))
		{
			throw CDShowException("Failed to get filter enumerator!!!",hr);
		}
		
		bool bFound=false;
		CComPtr<IDSRendFilter> pDSRend;
		CComPtr<IBaseFilter> pFilter;
		while(hr=filterEnum.next(&pFilter),hr==S_OK && pFilter!=NULL)
		{
			if(pFilter.IsEqualObject(VideoFilter))
			{
				pFilter.Release();
				continue;
			}

			hr=pFilter.QueryInterface(&pDSRend);
			if(SUCCEEDED(hr))
			{
				bFound=true;
				//replace the dsrend filter in the grf file with our renderer.
				//this might need to be changed to allow settings on the 
				//dsrend filter from the grf file to be preserved
				//(not implemented yet on the filter)

				CDShowPinEnum InPins(pFilter,PINDIR_INPUT);
				CComPtr<IPin> pInPin=InPins.next();
				if(pInPin==NULL)
				{
					throw CDShowException("DSRend filter do not have an input pin!!! (bug)");
				}
				CComPtr<IPin> pOutPin;
				hr=pInPin->ConnectedTo(&pOutPin);
				if(FAILED(hr))
				{
					if(hr==VFW_E_NOT_CONNECTED)
					{
						throw CDShowException("The dsrend filter is not connected in the grf file",hr);
					}
					else
					{
						throw CDShowException("Failed to find filter that is connected to dscaler renderer",hr);
					}
				}
				
				//preserve the mediatype on the connection
				AM_MEDIA_TYPE mt;
				memset(&mt,0,sizeof(AM_MEDIA_TYPE));
				hr=pOutPin->ConnectionMediaType(&mt);
				ASSERT(SUCCEEDED(hr));

				//preserve dsrend filter settings
				CComPtr<IPersistStream> pPStrmOld;
				hr=pFilter.QueryInterface(&pPStrmOld);
				if(FAILED(hr))
				{
					throw CDShowException("Coud not find IPersistStream on old dsrend (bug)",hr);
				}
				CComPtr<IPersistStream> pPStrmNew;
				hr=VideoFilter.QueryInterface(&pPStrmNew);
				if(FAILED(hr))
				{
					throw CDShowException("Coud not find IPersistStream on new dsrend (bug)",hr);
				}
				
				ULARGE_INTEGER ulSize;
				hr=pPStrmOld->GetSizeMax(&ulSize);
				if(FAILED(hr))
				{
					throw CDShowException("IPersistStream::GetSizeMax failed (bug)",hr);
				}
				
				//make sure that the new and old renderer filters is of the same type
				GUID OldGUID;
				GUID NewGUID;
				if(FAILED(pPStrmOld->GetClassID(&OldGUID)) || FAILED(pPStrmNew->GetClassID(&NewGUID)))
				{
					LOG(2,"DShowFileSource: Failed to get ClassID of new or old renderer filter");
				}
				else
				{
					if(IsEqualGUID(OldGUID,NewGUID))
					{	
						CComPtr<IStream> pStream;
						HGLOBAL hg=GlobalAlloc(GMEM_MOVEABLE,(SIZE_T)ulSize.QuadPart);
						if(hg!=NULL)
						{
							if(CreateStreamOnHGlobal(hg,TRUE,&pStream)==S_OK)
							{
								hr=pPStrmOld->Save(pStream,FALSE);
								if(SUCCEEDED(hr))
								{
									LARGE_INTEGER pos;
									pos.QuadPart=0;
									hr=pStream->Seek(pos,STREAM_SEEK_SET,NULL);
									hr=pPStrmNew->Load(pStream);
								}
							}
						}
					}
					else
					{
						LOG(2,"DShowFileSource: Old and new renderer filter is not of the same type, will not copy setting");
					}
				}


				hr=m_pGraph->RemoveFilter(pFilter);
				
				//connect to the right dsrend filter
				CDShowPinEnum InPins2(VideoFilter,PINDIR_INPUT);
				CComPtr<IPin> pInPin2=InPins2.next();
				hr=pOutPin->Connect(pInPin2,&mt);
				if(mt.cbFormat>0 && mt.pbFormat!=NULL)
				{
					CoTaskMemFree(mt.pbFormat);
					mt.cbFormat=0;
					mt.pbFormat=NULL;
				}
				if(mt.pUnk!=NULL)
				{
					mt.pUnk->Release();
					mt.pUnk=NULL;
				}
				if(FAILED(hr))
				{
					throw CDShowException("Failed to connect dsrend filter",hr);
				}
				break;
			}
			pFilter.Release();
		}
		if(!bFound)
		{
			throw CDShowException("This filter graph file does not contain a dscaler renderer filter");
		}
	}
	
	m_bIsConnected=true;
}

#endif