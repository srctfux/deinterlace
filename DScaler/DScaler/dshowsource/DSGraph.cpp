/////////////////////////////////////////////////////////////////////////////
// $Id: DSGraph.cpp,v 1.23 2002-09-14 17:03:11 tobbej Exp $
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
// Change Log
//
// Date          Developer             Changes
//
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.22  2002/09/07 13:32:34  tobbej
// save/restore video format settings to ini file
//
// Revision 1.21  2002/09/04 17:12:01  tobbej
// moved parts of start() to ConnectGraph()
// some other changes for new video format dialog
//
// Revision 1.20  2002/08/21 20:29:20  kooiman
// Fixed settings and added setting for resolution. Fixed videoformat==lastone in dstvtuner.
//
// Revision 1.19  2002/08/20 16:19:57  tobbej
// removed graph loging from debug build
//
// Revision 1.18  2002/08/04 17:25:49  tobbej
// improved error messages when creating filter graph
//
// Revision 1.17  2002/08/01 20:24:19  tobbej
// implemented AvgSyncOffset counter in dsrend
//
// Revision 1.16  2002/07/06 16:50:08  tobbej
// new field buffering
// some small changes to resolution changing
//
// Revision 1.15  2002/05/24 15:15:11  tobbej
// changed filter properties dialog to include progpertypages from the pins
//
// Revision 1.14  2002/05/11 15:22:00  tobbej
// fixed object reference leak when opening filter settings
// added filter graph loging in debug build
//
// Revision 1.13  2002/05/02 19:50:39  tobbej
// changed dshow source filter submenu to use new tree based dialog
//
// Revision 1.12  2002/05/01 20:38:40  tobbej
// fixed memory leak
//
// Revision 1.11  2002/04/16 15:26:54  tobbej
// fixed filter reference leak when geting filter names (filters submenu)
// added waitForNextField
//
// Revision 1.10  2002/04/07 14:52:13  tobbej
// fixed race when changing resolution
// improved error handling
//
// Revision 1.9  2002/04/03 19:52:30  tobbej
// added some more logging to help track the filters submenu problem
//
// Revision 1.8  2002/03/26 19:48:59  adcockj
// Improved error handling in DShow code
//
// Revision 1.7  2002/03/17 21:43:23  tobbej
// added input resolution submenu
//
// Revision 1.6  2002/03/15 23:07:16  tobbej
// changed dropped frames counter to include dropped frames in source filter.
// added functions to enable/disable graph clock.
// started to make changing resolution posibel.
//
// Revision 1.5  2002/02/13 17:01:42  tobbej
// new filter properties menu
//
// Revision 1.4  2002/02/07 22:09:11  tobbej
// changed for new file input
//
// Revision 1.3  2002/02/05 17:27:46  tobbej
// update dropped/drawn fields stats
//
// Revision 1.2  2002/02/03 11:02:34  tobbej
// various updates for new filter
//
// Revision 1.1  2001/12/17 19:30:24  tobbej
// class for managing the capture graph
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DSGraph.cpp implementation of the CDShowGraph class.
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT

#include "dscaler.h"
#include "DSGraph.h"
#include "debug.h"
#include "CaptureDevice.h"
#include "DShowFileSource.h"
#include "PinEnum.h"
#include "DebugLog.h"
#include "..\..\..\DSRend\DSRend_i.c"
#include <dvdmedia.h>
#include "DevEnum.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDShowGraph::CDShowGraph(string device,string deviceName,string AudioDevice)
:m_pSource(NULL),m_GraphState(State_Stopped)
{
	initGraph();
	CreateRenderer(AudioDevice);

	m_pSource=new CDShowCaptureDevice(m_pGraph,device,deviceName);

#ifdef _DEBUG
	//this makes it posibel to connect graphedit to the graph
	AddToRot(m_pGraph,&m_hROT);
#endif
}

CDShowGraph::CDShowGraph(string filename,string AudioDevice)
:m_pSource(NULL),m_GraphState(State_Stopped)
{
	initGraph();
	CreateRenderer(AudioDevice);

	m_pSource=new CDShowFileSource(m_pGraph,filename);

#ifdef _DEBUG
	AddToRot(m_pGraph,&m_hROT);
#endif
}

CDShowGraph::~CDShowGraph()
{
	if(m_DSRend!=NULL)
	{
		//make shure that all fields are marked as free
		//(prevents deadlock and problems in dsrend filter)
		m_DSRend->FreeFields();
	}
	if(m_pSource!=NULL)
	{
		delete m_pSource;
		m_pSource=NULL;
	}

#ifdef _DEBUG
	RemoveFromRot(m_hROT);
	/*m_pGraph->SetLogFile(NULL);
	if(m_hLogFile!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hLogFile);
	}*/
#endif
}

void CDShowGraph::initGraph()
{
	HRESULT hr=m_pGraph.CoCreateInstance(CLSID_FilterGraph);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to create Filter Graph",hr);
	}

	hr=m_pBuilder.CoCreateInstance(CLSID_CaptureGraphBuilder2);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to create Capture graph builder",hr);
	}
	
	//connect capture graph builder with the graph
	m_pBuilder->SetFiltergraph(m_pGraph);

	//get mediacontrol so we can start and stop the filter graph
	hr=m_pGraph.QueryInterface(&m_pControl);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to get IMediaControl interface",hr);
	}

#ifdef _DEBUG
	/*m_hLogFile=CreateFile("DShowGraphLog.txt",GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(m_hLogFile!=INVALID_HANDLE_VALUE)
	{
		hr=m_pGraph->SetLogFile((DWORD_PTR)m_hLogFile);
		ASSERT(SUCCEEDED(hr));
	}*/
#endif


}

void CDShowGraph::CreateRenderer(string AudioDevice)
{
	HRESULT hr=m_renderer.CoCreateInstance(CLSID_DSRendFilter);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to create dscaler input renderer filter\nMake sure that the filter is properly registered",hr);
	}
	
	hr=m_pGraph->AddFilter(m_renderer,L"DScaler video renderer");
	if(FAILED(hr))
	{
		throw CDShowException("Failed to add video renderer to filter graph",hr);
	}
	hr=m_renderer.QueryInterface(&m_DSRend);
	if(FAILED(hr))
	{
		throw CDShowException("QueryInterface failed on video renderer",hr);
	}
	hr=m_DSRend->SetFieldHistory(MAX_PICTURE_HISTORY);
	if(FAILED(hr))
	{
		throw CDShowException("SetFieldHistory failed",hr);
	}

	//find IDSRendSettings interface
	CDShowPinEnum pins(m_renderer,PINDIR_INPUT);
	CComPtr<IPin> pInPin=pins.next();
	if(pInPin==NULL)
	{
		throw CDShowException("DScaler input renderer filter don't have any input pins!! (bug)");
	}
	hr=pInPin.QueryInterface(&m_pDSRendSettings);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to find IDSRendSettings",hr);
	}

	if(AudioDevice.size()>0)
	{
		CDShowDevEnum::createDevice(AudioDevice,IID_IBaseFilter,&m_pAudioRenderer);

		hr=m_pGraph->AddFilter(m_pAudioRenderer,L"Audio Renderer");
		if(FAILED(hr))
		{
			throw CDShowException("Failed to add audio renderer to filter graph",hr);
		}
	}
}

bool CDShowGraph::GetFields(long *pcFields, FieldBuffer *ppFields,BufferInfo &info,DWORD dwLateness)
{
	if(m_DSRend==NULL)
	{
		return false;
	}
	ASSERT(ppFields!=NULL && pcFields!=NULL);
	HRESULT hr=m_DSRend->GetFields(ppFields,pcFields,&info,400,dwLateness);
	if(FAILED(hr))
	{
		CString tmpstr;
		DWORD len=AMGetErrorText(hr,tmpstr.GetBufferSetLength(MAX_ERROR_TEXT_LEN),MAX_ERROR_TEXT_LEN);
		tmpstr.ReleaseBuffer(len);
		LOG(3, "GetFields failed - Error Code: '0x%x' Error Text: '%s'", hr,(LPCSTR)tmpstr);
		LOGD("GetFields failed - Error Code: '0x%x' Error Text: '%s'\n", hr,(LPCSTR)tmpstr);
		return false;
	}
	return true;
}

void CDShowGraph::ConnectGraph()
{
	if(m_pSource!=NULL)
	{
		//check if the dsrend is unconnected
		bool IsUnConnected=false;
		CDShowPinEnum pins(m_renderer,PINDIR_INPUT);
		CComPtr<IPin> InPin=pins.next();
		ASSERT(InPin!=NULL);
		CComPtr<IPin> OutPin;
		HRESULT hr=InPin->ConnectedTo(&OutPin);
		if(hr==VFW_E_NOT_CONNECTED)
		{
			IsUnConnected=true;
		}
		
		if(IsUnConnected || !m_pSource->isConnected())
		{
			m_pSource->Connect(m_renderer,m_pAudioRenderer);
		}
		buildFilterList();
	}
}

void CDShowGraph::start()
{
	if(m_pSource!=NULL)
	{
		ConnectGraph();
		HRESULT hr=m_pControl->Run();
		if(FAILED(hr))
		{
			throw CDShowException("Failed to start filter graph",hr);
		}
		m_GraphState=State_Running;
	}
	else
	{
		m_GraphState=State_Stopped;
	}
}

void CDShowGraph::pause()
{
	if(m_pSource!=NULL)
	{
		if(!m_pSource->isConnected())
		{
			m_pSource->Connect(m_renderer,m_pAudioRenderer);
		}

		HRESULT hr=m_pControl->Pause();
		if(FAILED(hr))
		{
			throw CDShowException("Failed to pause filter graph",hr);
		}
		m_GraphState=State_Paused;
	}
}

void CDShowGraph::stop()
{
	if(m_DSRend!=NULL)
	{
		m_DSRend->FreeFields();
	}
	HRESULT hr=m_pControl->Stop();
	if(FAILED(hr))
	{
		throw CDShowException("Failed to stop filter graph",hr);
	}
	m_GraphState=State_Stopped;
}

CDShowBaseSource* CDShowGraph::getSourceDevice()
{
	return m_pSource;
}

void CDShowGraph::getConnectionMediatype(AM_MEDIA_TYPE *pmt)
{
	ASSERT(pmt!=NULL);
	
	if(m_renderer==NULL)
	{
		throw CDShowException("Null pointer!!");
	}
	
	CDShowPinEnum pins(m_renderer,PINDIR_INPUT);
	CComPtr<IPin> inPin=pins.next();
	if(inPin==NULL)
	{
		throw CDShowException("Cant find input pin on video renderer");
	}
	
	HRESULT hr=inPin->ConnectionMediaType(pmt);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to get media type",hr);
	}
}

void CDShowGraph::buildFilterList()
{
	m_filters.erase(m_filters.begin(),m_filters.end());
	
	CDShowGenericEnum<IEnumFilters,IBaseFilter> filterEnum;
	HRESULT hr=m_pGraph->EnumFilters(&filterEnum.m_pEnum);
	if(FAILED(hr))
	{
		CString tmpstr;
		DWORD len=AMGetErrorText(hr,tmpstr.GetBufferSetLength(MAX_ERROR_TEXT_LEN),MAX_ERROR_TEXT_LEN);
		tmpstr.ReleaseBuffer(len);
		LOG(1, "Failed to get filter enumerator!!! : Error Code: '0x%x' Error Text: '%s'",hr,(LPCSTR)tmpstr);
		return;
	}
	CComPtr<IBaseFilter> pFilter;
	while(hr=filterEnum.next(&pFilter),hr==S_OK && pFilter!=NULL)
	{
		CFilterPages tmp;
		tmp.m_pFilter=pFilter;
		
		//check for any propertypages on the pins
		try
		{
			CComPtr<IPin> pPin;
			CDShowPinEnum pins(pFilter);
			while(pPin=pins.next(),pPin!=NULL)
			{
				//add only if the pin has propertypages
				CComPtr<ISpecifyPropertyPages> pPages;
				hr=pPin.QueryInterface(&pPages);
				if(SUCCEEDED(hr))
				{
					//just because a pin supports ISpecifyPropertyPages doesnt
					//nessesarily mean that it has any pages.
					CAUUID pages;
					hr=pPages->GetPages(&pages);
					if(SUCCEEDED(hr) && pages.cElems>0)
					{
						tmp.m_SubPage.push_back(pPin);
						pPages.Release();
						CoTaskMemFree(pages.pElems);
					}
				}
			}
		}
		catch(CDShowException e)
		{
		}

		m_filters.push_back(tmp);
		pFilter.Release();
	}
}

bool CDShowGraph::getFilterPropertyPage(int index,CTreeSettingsPage **ppPage,bool &bHasSubPages)
{
	USES_CONVERSION;
	if(ppPage==NULL)
		return false;
	
	if(index>=0 && index<m_filters.size())
	{

		CComPtr<IBaseFilter> pFilter=m_filters[index].m_pFilter;
		bHasSubPages=m_filters[index].m_SubPage.size()>0;
		FILTER_INFO info;
		HRESULT hr=pFilter->QueryFilterInfo(&info);
		if(SUCCEEDED(hr))
		{
			if(info.pGraph!=NULL)
			{
				info.pGraph->Release();
				info.pGraph=NULL;
			}
			CComPtr<ISpecifyPropertyPages> pSpecifyPages;
			hr=pFilter.QueryInterface(&pSpecifyPages);
			if(SUCCEEDED(hr))
			{
				CAUUID pages;
				hr=pSpecifyPages->GetPages(&pages);
				if(SUCCEEDED(hr) && pages.cElems>0)
				{
					IUnknown *pUnk;
					hr=pFilter->QueryInterface(IID_IUnknown,(void**)&pUnk);
					//a com object cant exist without IUnknown
					ASSERT(SUCCEEDED(hr));
					
					*ppPage=new CTreeSettingsOleProperties(W2A(info.achName),1,&pUnk,pages.cElems,pages.pElems,MAKELCID(MAKELANGID(LANG_NEUTRAL,SUBLANG_SYS_DEFAULT),SORT_DEFAULT));
					pUnk->Release();
					pUnk=NULL;
					CoTaskMemFree(pages.pElems);
				}
				else
				{
					*ppPage=new CTreeSettingsPage(W2A(info.achName),IDD_TREESETTINGS_NOPROPERTIES);
				}
			}
			else
			{
				*ppPage=new CTreeSettingsPage(W2A(info.achName),IDD_TREESETTINGS_NOPROPERTIES);
			}
			
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool CDShowGraph::getFilterSubPage(int filterIndex,int subIndex,CTreeSettingsPage **ppPage)
{
	USES_CONVERSION;
	if(filterIndex>=0 && filterIndex<m_filters.size())
	{
		if(subIndex>=0 && subIndex<m_filters[filterIndex].m_SubPage.size())
		{
			PIN_INFO pinInfo;
			CComPtr<IPin> pPin=m_filters[filterIndex].m_SubPage[subIndex];
			HRESULT hr=pPin->QueryPinInfo(&pinInfo);
			if(SUCCEEDED(hr))
			{
				if(pinInfo.pFilter!=NULL)
				{
					pinInfo.pFilter->Release();
					pinInfo.pFilter=NULL;
				}
				CComPtr<ISpecifyPropertyPages> pSpecifyPages;
				hr=pPin.QueryInterface(&pSpecifyPages);
				if(SUCCEEDED(hr))
				{
					CAUUID pages;
					hr=pSpecifyPages->GetPages(&pages);
					if(SUCCEEDED(hr) && pages.cElems>0)
					{
						IUnknown *pUnk;
						hr=pPin->QueryInterface(IID_IUnknown,(void**)&pUnk);
						ASSERT(SUCCEEDED(hr));
						
						*ppPage=new CTreeSettingsOleProperties(W2A(pinInfo.achName),1,&pUnk,pages.cElems,pages.pElems,MAKELCID(MAKELANGID(LANG_NEUTRAL,SUBLANG_SYS_DEFAULT),SORT_DEFAULT));
						pUnk->Release();
						pUnk=NULL;
						CoTaskMemFree(pages.pElems);
						return true;
					}
				}
			}
		}
	}
	return false;
}

long CDShowGraph::getDroppedFrames()
{
	HRESULT hr;
	if(m_pQualProp==NULL)
	{
		hr=m_renderer->QueryInterface(IID_IQualProp,(void**)&m_pQualProp);
		if(FAILED(hr))
		{
			throw CDShowException("Failed to get IQualProp interface on renderer filter (most likely a bug)",hr);
		}
	}
	
	int frames;
	hr=m_pQualProp->get_FramesDroppedInRenderer(&frames);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to get dropped frames count",hr);
	}
	return frames+m_pSource->getNumDroppedFrames();
}

CDShowGraph::eChangeRes_Error CDShowGraph::ChangeRes(CDShowGraph::CVideoFormat fmt)
{
	if(m_renderer==NULL)
	{
		return eChangeRes_Error::ERROR_NO_GRAPH;
	}

	CDShowPinEnum RendPins(m_renderer,PINDIR_INPUT);
	CComPtr<IPin> InPin;
	InPin=RendPins.next();
	//if this assert is trigered there is most likely s bug in the renderer filter
	ASSERT(InPin!=NULL);
	//get the upstream pin
	CComPtr<IPin> OutPin;
	HRESULT hr=InPin->ConnectedTo(&OutPin);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to find pin",hr);
	}
	//get IAMStreamConfig on the output pin
	m_pStreamCfg=NULL;
	hr=OutPin.QueryInterface(&m_pStreamCfg);
	if(FAILED(hr))
	{
		throw CDShowException("Query interface for IAMStreamConfig failed",hr);
	}

	FILTER_STATE oldState=getState();
	if(oldState!=State_Stopped)
	{
		//the only time the format can be changed is when the graph is stopped.
		stop();
	}

	//get current mediatype
	AM_MEDIA_TYPE *mt=NULL;
	hr=m_pStreamCfg->GetFormat(&mt);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to get old mediatype",hr);
	}
	
	//create the new media type
	AM_MEDIA_TYPE newType;
	memset(&newType,0,sizeof(AM_MEDIA_TYPE));
	BITMAPINFOHEADER *pbmiHeader=NULL;

	newType.majortype=MEDIATYPE_Video;
	newType.subtype=MEDIASUBTYPE_YUY2;
	newType.bFixedSizeSamples=TRUE;
	
	//copy some info from the old media type and initialize the new format block
	if(mt->pbFormat!=NULL)
	{
		if(mt->formattype==FORMAT_VideoInfo)
		{
			VIDEOINFOHEADER *newInfoHeader=(VIDEOINFOHEADER*)CoTaskMemAlloc(sizeof(VIDEOINFOHEADER));
			memset(newInfoHeader,0,sizeof(VIDEOINFOHEADER));
			newType.formattype=FORMAT_VideoInfo;
			newType.cbFormat=sizeof(VIDEOINFOHEADER);
			newType.pbFormat=(BYTE*)newInfoHeader;

			VIDEOINFOHEADER *videoInfo=(VIDEOINFOHEADER*)mt->pbFormat;
			newInfoHeader->dwBitRate=videoInfo->dwBitRate;
			newInfoHeader->dwBitErrorRate=videoInfo->dwBitErrorRate;
			newInfoHeader->AvgTimePerFrame=videoInfo->AvgTimePerFrame;
			newInfoHeader->bmiHeader=videoInfo->bmiHeader;
			pbmiHeader=&newInfoHeader->bmiHeader;
		}
		else if(mt->formattype==FORMAT_VideoInfo2)
		{
			VIDEOINFOHEADER2 *newInfoHeader=(VIDEOINFOHEADER2*)CoTaskMemAlloc(sizeof(VIDEOINFOHEADER2));
			memset(newInfoHeader,0,sizeof(VIDEOINFOHEADER2));
			newType.formattype=FORMAT_VideoInfo2;
			newType.cbFormat=sizeof(VIDEOINFOHEADER2);
			newType.pbFormat=(BYTE*)newInfoHeader;

			VIDEOINFOHEADER2 *videoInfo2=(VIDEOINFOHEADER2*)mt->pbFormat;
			newInfoHeader->dwBitRate=videoInfo2->dwBitRate;
			newInfoHeader->dwBitErrorRate=videoInfo2->dwBitErrorRate;
			newInfoHeader->AvgTimePerFrame=videoInfo2->AvgTimePerFrame;
			newInfoHeader->bmiHeader=videoInfo2->bmiHeader;
			pbmiHeader=&newInfoHeader->bmiHeader;
		}
	}
	
	//if pbFormat is null then there is something strange going on with the renderer filter
	//most likely a bug
	ASSERT(newType.pbFormat!=NULL);

	pbmiHeader->biSize=sizeof(BITMAPINFOHEADER);
	pbmiHeader->biWidth=fmt.m_Width;
	pbmiHeader->biHeight=fmt.m_Height;
	pbmiHeader->biSizeImage=fmt.m_Width*fmt.m_Height*pbmiHeader->biBitCount/8;

	//change dsrend filter settings acording to fmt
	hr=m_pDSRendSettings->put_ForceYUY2(fmt.m_bForceYUY2 ? TRUE : FALSE);
	if(FAILED(hr))
	{
		//@todo handle error
	}
	
	hr=m_pDSRendSettings->put_FieldFormat(fmt.m_FieldFmt);
	if(FAILED(hr))
	{
		//@todo handle error
	}
	
	eChangeRes_Error result=eChangeRes_Error::ERROR_FAILED_TO_CHANGE_BACK;
	//change the format
	hr=m_pStreamCfg->SetFormat(&newType);
	if(FAILED(hr))
	{
		//retry with RGB24
		newType.subtype=MEDIASUBTYPE_RGB24;
		hr=m_pStreamCfg->SetFormat(&newType);
		if(FAILED(hr))
		{
			//reconnect using the old mediatype
			///@todo reset ForceYUY2 and FieldFormat first
			CComPtr<IPin> tmp;
			hr=InPin->ConnectedTo(&tmp);
			if(hr==VFW_E_NOT_CONNECTED)
			{
				//reconnect
				hr=OutPin->Connect(InPin,mt);
				if(SUCCEEDED(hr))
				{
					//failed to change mediatype, but was able to reconnect using old mediatype          
					result=eChangeRes_Error::ERROR_CHANGED_BACK;
				}
			}
			else
			{
				hr=m_pStreamCfg->SetFormat(mt);
				if(SUCCEEDED(hr))
				{
					//was able to change back to old format
					result=eChangeRes_Error::ERROR_CHANGED_BACK;
				}
			}
			
			/*if(bBackToOldResolution && (mt->pbFormat!=NULL))
			{
				if(mt->formattype==FORMAT_VideoInfo)
				{
					VIDEOINFOHEADER *videoInfo=(VIDEOINFOHEADER*)mt->pbFormat;
					fmt.m_Width = videoInfo->bmiHeader.biWidth;
					fmt.m_Height = videoInfo->bmiHeader.biHeight;
				}
				else if(mt->formattype==FORMAT_VideoInfo2)
				{
					VIDEOINFOHEADER2 *videoInfo=(VIDEOINFOHEADER2*)mt->pbFormat;
					fmt.m_Width = videoInfo->bmiHeader.biWidth;
					fmt.m_Height = videoInfo->bmiHeader.biHeight;
				}
			}*/
			//throw CDShowException("Failed to change resolution, and coud not change back to old resolution",hr);
		}
		else
		{
			result=eChangeRes_Error::SUCCESS;
		}
	}
	else
	{
		result=eChangeRes_Error::SUCCESS;
	}
	
	//free mediatypes
	if(mt->pUnk!=NULL)
	{
		mt->pUnk->Release();
		mt->pUnk=NULL;
	}
	if(mt->pbFormat!=NULL && mt->cbFormat>0)
	{
		CoTaskMemFree(mt->pbFormat);
		mt->pbFormat=NULL;
		mt->cbFormat=0;
	}
	if(newType.pbFormat!=NULL && mt->cbFormat>0)
	{
		CoTaskMemFree(newType.pbFormat);
		newType.pbFormat=NULL;
		newType.cbFormat=NULL;
	}

	//restore old graph state
	if(oldState==State_Running)
	{
		start();
	}
	else if(oldState==State_Paused)
	{
		pause();
	}

	return result;
}

bool CDShowGraph::IsValidRes(CDShowGraph::CVideoFormat fmt)
{
	if(m_renderer==NULL)
	{
		return false;
	}

	if(m_pStreamCfg==NULL)
	{
		try
		{
			findStreamConfig();
		}
		catch(CDShowException e)
		{
            LOG(1, "DShow Exception - %s", (LPCSTR)e.getErrorText());
			return false;
		}
	}
	int iCount,iSize;
	HRESULT hr=m_pStreamCfg->GetNumberOfCapabilities(&iCount,&iSize);
	if(FAILED(hr))
		return false;

	for(int i=0;i<iCount;i++)
	{
		AM_MEDIA_TYPE *pMT=NULL;
		VIDEO_STREAM_CONFIG_CAPS vCaps;
		hr=m_pStreamCfg->GetStreamCaps(i,&pMT,(BYTE*)&vCaps);
		if(SUCCEEDED(hr))
		{
			//free the mediatype
			if(pMT->pUnk!=NULL)
			{
				pMT->pUnk->Release();
				pMT->pUnk=NULL;
			}
			if(pMT->cbFormat>0 && pMT->pbFormat!=NULL)
			{
				CoTaskMemFree(pMT->pbFormat);
				pMT->pbFormat=NULL;
			}
			
			//maybe check VIDEO_STREAM_CONFIG_CAPS::VideoStandard too

			//check width and height
			if(fmt.m_Width<=vCaps.MaxOutputSize.cx && fmt.m_Height<=vCaps.MaxOutputSize.cy && 
				fmt.m_Width>=vCaps.MinOutputSize.cx && fmt.m_Height>=vCaps.MinOutputSize.cy &&
				fmt.m_Width%vCaps.OutputGranularityX==0 && fmt.m_Height%vCaps.OutputGranularityY==0)
			{
				return true;
			}
		}
	}
	return false;
}
void CDShowGraph::findStreamConfig()
{
	CDShowPinEnum rendPins(m_renderer,PINDIR_INPUT);
	CComPtr<IPin> inPin;

	inPin=rendPins.next();
	
	//if this assert is trigered there is most likely s bug in the renderer filter
	ASSERT(inPin!=NULL);

	//get the upstream pin
	CComPtr<IPin> outPin;
	HRESULT hr=inPin->ConnectedTo(&outPin);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to find pin",hr);
	}

	//get IAMStreamConfig on the output pin
	m_pStreamCfg=NULL;
	hr=outPin.QueryInterface(&m_pStreamCfg);
	if(FAILED(hr))
	{
		throw CDShowException("Query interface for IAMStreamConfig failed",hr);
	}
}

void CDShowGraph::disableClock()
{
	//prevent multiple disableClock calls
	if(m_pOldRefClk!=NULL)
		return;

	CComPtr<IMediaFilter> pMFilter;
	HRESULT hr=m_pGraph.QueryInterface(&pMFilter);
	if(FAILED(hr))
	{
		throw CDShowException("QueryInterface for IMediaFilter failed on filter graph",hr);
	}
	
	//save old reference clock
	hr=pMFilter->GetSyncSource(&m_pOldRefClk);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to get old reference clock",hr);
	}
	
	//disable the current one
	hr=pMFilter->SetSyncSource(NULL);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to set reference clock",hr);
	}
}

void CDShowGraph::restoreClock()
{
	if(m_pOldRefClk==NULL)
		return;
	
	CComPtr<IMediaFilter> pMFilter;
	HRESULT hr=m_pGraph.QueryInterface(&pMFilter);
	if(FAILED(hr))
	{
		throw CDShowException("QueryInterface for IMediaFilter failed on filter graph",hr);
	}

	//restor the reference clock
	hr=pMFilter->SetSyncSource(m_pOldRefClk);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to set reference clock",hr);
	}
	m_pOldRefClk=NULL;

}

bool CDShowGraph::CVideoFormat::operator==(CVideoFormat &fmt)
{
	if(m_Width==fmt.m_Width && 
		m_Height==fmt.m_Height && 
		m_bForceYUY2==fmt.m_bForceYUY2 &&
		m_FieldFmt==fmt.m_FieldFmt)
	{
		return true;
	}
	else
	{
		return false;
	}
}

CDShowGraph::CVideoFormat::operator std::string()
{
	std::stringstream str;
	str << m_Name << "#" << m_Width << "#" << m_Height << "#" << (m_bForceYUY2 ? 1 : 0) << "#" << m_FieldFmt;
	std::string tmp=str.str();
	return tmp;
}

void CDShowGraph::CVideoFormat::operator=(std::string &str)
{
	std::vector<std::string> strlist;
	std::string::size_type LastPos=0;
	std::string::size_type pos;
	while(pos=str.find("#",LastPos),pos!=std::string::npos)
	{
		strlist.push_back(str.substr(LastPos,pos-LastPos));
		LastPos=pos+1;
	}
	if(LastPos<str.size())
	{
		strlist.push_back(str.substr(LastPos));
	}

	ASSERT(strlist.size()==5);
	m_Name=strlist[0];
	m_Width=atol(strlist[1].c_str());
	m_Height=atol(strlist[2].c_str());
	m_bForceYUY2=atol(strlist[3].c_str())!=0;
	m_FieldFmt=(DSREND_FIELD_FORMAT)atol(strlist[4].c_str());
}
#endif
