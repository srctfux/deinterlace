/////////////////////////////////////////////////////////////////////////////
// $Id: Deinterlace.cpp,v 1.2 2001-09-19 17:50:07 tobbej Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Torbjörn Jansson.  All rights reserved.
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
// Revision 1.1  2001/08/08 15:37:02  tobbej
// moved dmo filter to new directory
//
// Revision 1.2  2001/08/07 20:22:35  tobbej
// added new button in propertypage to show plugin ui
// fixed Activate function
//
// Revision 1.1.1.1  2001/07/30 16:14:44  tobbej
// initial import of new dmo filter
//
//
//////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"

#define FIX_LOCK_NAME
#include <dmo.h>
#include <dmoimpl.h>
#include <uuids.h>		//DirectShow media type guids
#include <amvideo.h>	//VIDEOINFOHEADER definition
#include "util.h"

#include "DeinterlaceDMO.h"
#include "Deinterlace.h"
#include "memcpy.h"

/////////////////////////////////////////////////////////////////////////////
//

STDMETHODIMP CDeinterlace::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IDeinterlace
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (IsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}


HRESULT CDeinterlace::InternalGetInputStreamInfo(DWORD dwInputStreamIndex, DWORD *pdwFlags)
{
	//ATLTRACE("%s(%d) : InternalGetInputStreamInfo\n",__FILE__,__LINE__);

    if(pdwFlags==NULL)
		return E_POINTER;
	
	*pdwFlags = DMO_INPUT_STREAMF_WHOLE_SAMPLES |
		DMO_INPUT_STREAMF_SINGLE_SAMPLE_PER_BUFFER |
		DMO_INPUT_STREAMF_FIXED_SAMPLE_SIZE |
		DMO_INPUT_STREAMF_HOLDS_BUFFERS;
	return S_OK;
}

HRESULT CDeinterlace::InternalGetOutputStreamInfo(DWORD dwOutputStreamIndex, DWORD *pdwFlags)
{
	//ATLTRACE("%s(%d) : InternalGetOutputStreamInfo\n",__FILE__,__LINE__);

	if(pdwFlags==NULL)
		return E_POINTER;

	*pdwFlags = DMO_OUTPUT_STREAMF_WHOLE_SAMPLES |
		DMO_OUTPUT_STREAMF_SINGLE_SAMPLE_PER_BUFFER |
		DMO_OUTPUT_STREAMF_FIXED_SAMPLE_SIZE;
	return S_OK;
}

HRESULT CDeinterlace::InternalCheckInputType(DWORD dwInputStreamIndex, const DMO_MEDIA_TYPE *pmt)
{
	//ATLTRACE("%s(%d) : InternalCheckInputType\n",__FILE__,__LINE__);

	//if output is set, accept only same format
	if(OutputTypeSet(0))
	{
		if(TypesMatch(OutputType(0),pmt))
			return S_OK;
		else
			return DMO_E_INVALIDTYPE;
	}
	else
	{
		return CanPerformDeinterlace(pmt);
	}
}

HRESULT CDeinterlace::InternalCheckOutputType(DWORD dwOutputStreamIndex, const DMO_MEDIA_TYPE *pmt)
{
	//ATLTRACE("%s(%d) : InternalCheckOutputType\n",__FILE__,__LINE__);

	//if input set, accept only same format
	if(InputTypeSet(0))
	{
		if(TypesMatch(InputType(0),pmt))
			return S_OK;
		else
			return DMO_E_INVALIDTYPE;
	}
	else
	{
		return CanPerformDeinterlace(pmt);
	}
}


HRESULT CDeinterlace::InternalGetInputType(DWORD dwInputStreamIndex, DWORD dwTypeIndex,
                             DMO_MEDIA_TYPE *pmt)
{
	//ATLTRACE("%s(%d) : InternalGetInputType\n",__FILE__,__LINE__);
    if(dwInputStreamIndex!=0)
		return DMO_E_INVALIDSTREAMINDEX;
	if(pmt==NULL)
		return E_POINTER;
	
	//if output is already set, then prefered media type is the same as output
	if(OutputTypeSet(0))
	{
		if(dwTypeIndex==0)
		{
			HRESULT hr=MoCopyMediaType(pmt,OutputType(0));
			if(FAILED(hr))
				return hr;
			else
				return S_OK;
		}
		else
			return DMO_E_NO_MORE_ITEMS;
	}

	if(dwTypeIndex==0)
	{
		HRESULT hr=MoInitMediaType(pmt,0);
		pmt->majortype=MEDIATYPE_Video;
		pmt->subtype=MEDIASUBTYPE_RGB24;
		return S_OK;
	}
	else if(dwTypeIndex==1)
	{
		HRESULT hr=MoInitMediaType(pmt,0);
		pmt->majortype=MEDIATYPE_Video;
		pmt->subtype=MEDIASUBTYPE_YUY2;
		return S_OK;
	}
	else
		return DMO_E_NO_MORE_ITEMS;
}

HRESULT CDeinterlace::InternalGetOutputType(DWORD dwOutputStreamIndex, DWORD dwTypeIndex,
							DMO_MEDIA_TYPE *pmt)
{
	//ATLTRACE("%s(%d) : InternalGetOutputType\n",__FILE__,__LINE__);
	if(dwOutputStreamIndex!=0)
		return DMO_E_INVALIDSTREAMINDEX;
	if(pmt==NULL)
		return E_POINTER;

	if(InputTypeSet(0))
	{
		if(dwTypeIndex==0)
		{
			HRESULT hr=MoCopyMediaType(pmt,InputType(0));
			if(FAILED(hr))
				return hr;
			else
				return S_OK;
		}
		else
			return DMO_E_NO_MORE_ITEMS;
	}
	
	if(dwTypeIndex==0)
	{
		HRESULT hr=MoInitMediaType(pmt,0);
		pmt->majortype=MEDIATYPE_Video;
		pmt->subtype=MEDIASUBTYPE_RGB24;
		return S_OK;
	}
	else if(dwTypeIndex==1)
	{
		HRESULT hr=MoInitMediaType(pmt,0);
		pmt->majortype=MEDIATYPE_Video;
		pmt->subtype=MEDIASUBTYPE_YUY2;
		return S_OK;
	}
	else
		return DMO_E_NO_MORE_ITEMS;
}

HRESULT CDeinterlace::InternalGetInputSizeInfo(DWORD dwInputStreamIndex, DWORD *pcbSize,
								DWORD *pcbMaxLookahead, DWORD *pcbAlignment)
{
	//ATLTRACE("%s(%d) : InternalGetInputSizeInfo\n",__FILE__,__LINE__);
	if(pcbSize==NULL || pcbMaxLookahead==NULL || pcbAlignment==NULL)
		return E_POINTER;
	
	if(dwInputStreamIndex!=0)
		return DMO_E_INVALIDSTREAMINDEX;

	*pcbSize = 1;
	*pcbMaxLookahead=MAX_FIELD_HISTORY-1;	//maximum number of buffers we will hold
	*pcbAlignment = 1;
	return S_OK;
}

HRESULT CDeinterlace::InternalGetOutputSizeInfo(DWORD dwOutputStreamIndex, DWORD *pcbSize,
								DWORD *pcbAlignment)
{
	//ATLTRACE("%s(%d) : InternalGetOutputSizeInfo\n",__FILE__,__LINE__);
	if(pcbSize==NULL || pcbAlignment==NULL)
		return E_POINTER;

	if(dwOutputStreamIndex!=0)
		return DMO_E_INVALIDSTREAMINDEX;
	
	if(!InputTypeSet(0))
		return DMO_E_TYPE_NOT_SET;
	
	*pcbAlignment=1;
	
	//cant use InputType(0)->lSampleSize since overlay mixer uses the wrong value for lSampleSize
	*pcbSize=((VIDEOINFOHEADER*)InputType(0)->pbFormat)->bmiHeader.biSizeImage;
	return S_OK;
}

HRESULT CDeinterlace::InternalGetInputMaxLatency(DWORD dwInputStreamIndex, REFERENCE_TIME *prtMaxLatency)
{
	//ATLTRACE("%s(%d) : InternalGetInputMaxLatency\n",__FILE__,__LINE__);
	return E_NOTIMPL;
}

HRESULT CDeinterlace::InternalSetInputMaxLatency(DWORD dwInputStreamIndex, REFERENCE_TIME rtMaxLatency)
{
	//ATLTRACE("%s(%d) : InternalSetInputMaxLatency\n",__FILE__,__LINE__);
	return E_NOTIMPL;
}

HRESULT CDeinterlace::InternalFlush()
{
	//ATLTRACE("%s(%d) : InternalFlush\n",__FILE__,__LINE__);

	//release all buffers
	for(int i=0;i<MAX_FIELD_HISTORY;i++)
		m_InputHistory[i].Reset();

	m_LastFrameProcessed=true;
	return S_OK;
}

HRESULT CDeinterlace::InternalDiscontinuity(DWORD dwInputStreamIndex)
{
	//ATLTRACE("%s(%d) : InternalDiscontinuity\n",__FILE__,__LINE__);
	//m_NextTimeStamp=-1;
	m_LastFrameProcessed=true;

	return S_OK;
}

HRESULT CDeinterlace::InternalAllocateStreamingResources()
{
	//ATLTRACE("%s(%d) : InternalAllocateStreamingResources\n",__FILE__,__LINE__);
	
	LPBITMAPINFOHEADER pbi;
	LPBITMAPINFOHEADER pbiOut;

	//retrieve BITMAPINFOHEADER for input and output
	pbi=&(((VIDEOINFOHEADER *)InputType(0)->pbFormat)->bmiHeader);
	pbiOut=&(((VIDEOINFOHEADER *)OutputType(0)->pbFormat)->bmiHeader);

	//fill in DEINTERLACE_INFO struct with data that wont change during streaming
	m_DIInfo.OverlayPitch=pbiOut->biWidth*pbiOut->biBitCount/8;
	m_DIInfo.LineLength=pbi->biWidth*pbi->biBitCount/8;
	m_DIInfo.FrameWidth=pbi->biWidth;
	m_DIInfo.FrameHeight=pbi->biHeight;
	m_DIInfo.FieldHeight=pbi->biHeight/2;
	m_DIInfo.CpuFeatureFlags=CpuFeatureFlags;

	m_DIInfo.CurrentFrame=0;
	m_DIInfo.bRunningLate=FALSE;
	m_DIInfo.bMissedFrame=FALSE;
	m_DIInfo.bDoAccurateFlips=FALSE;
	m_DIInfo.IsOdd=FALSE;
	//m_DIInfo.SleepInterval=0;
	memset(&m_DIInfo.SourceRect,0,sizeof(m_DIInfo.SourceRect));

	if(CpuFeatureFlags & FEATURE_SSE)
	{
		m_DIInfo.pMemcpy = memcpySSE;
	}
	else
	{
		m_DIInfo.pMemcpy = memcpyMMX;
	}
	m_InputSize=0;

	return S_OK;
}

HRESULT CDeinterlace::InternalFreeStreamingResources()
{
	//ATLTRACE("%s(%d) : InternalFreeStreamingResources\n",__FILE__,__LINE__);
	return S_OK;
}

HRESULT CDeinterlace::InternalProcessInput(DWORD dwInputStreamIndex, IMediaBuffer *pBuffer,
								DWORD dwFlags, REFERENCE_TIME rtTimestamp,
								REFERENCE_TIME rtTimelength)
{
	//ATLTRACE("%s(%d) : InternalProcessInput\n",__FILE__,__LINE__);
	//ATLTRACE("%s(%d) :  Flags: %#.Lx TimeStamp: %#.1I64x TimeLength: %#.I64x\n",__FILE__,__LINE__,dwFlags,rtTimestamp,rtTimelength);
	
	//move one step in history and adjust m_InputSize
	m_InputSize=MAX_FIELD_HISTORY;
	for(int i=MAX_FIELD_HISTORY-2;i>=0;i--)
	{
		if(m_InputHistory[i].m_Buffer==NULL)
			m_InputSize=i+1;
		m_InputHistory[i+1]=m_InputHistory[i];
	}
	
	//store new frame
	m_InputHistory[0].m_Buffer=pBuffer;
	m_InputHistory[0].m_flags=dwFlags;
	m_InputHistory[0].m_rtTimestamp=rtTimestamp;
	m_InputHistory[0].m_rtTimelength=rtTimelength;
	
	
	DWORD cbInputSize;
	BYTE *pbInputBuffer;
	short *OddLines[MAX_FIELD_HISTORY][1024];
	short *EvenLines[MAX_FIELD_HISTORY][1024];
	
	//update filed history
	memset(m_DIInfo.EvenLines, 0, MAX_FIELD_HISTORY * sizeof(short**));
	memset(m_DIInfo.OddLines, 0, MAX_FIELD_HISTORY * sizeof(short**));
	for(int cHist=0;cHist<MAX_FIELD_HISTORY;cHist++)
	{
		if(m_InputHistory[cHist].m_Buffer==NULL)
			continue;
		
		HRESULT hr=m_InputHistory[cHist].m_Buffer->GetBufferAndLength(&pbInputBuffer,&cbInputSize);
		if(FAILED(hr))
			return hr;

		for(int i=0;i<m_DIInfo.FieldHeight;i++)
		{
			EvenLines[cHist][i] = (short*)(pbInputBuffer+(2*i)*m_DIInfo.LineLength);
			OddLines[cHist][i] = (short*)(pbInputBuffer+(2*i+1)*m_DIInfo.LineLength);
		}
		m_DIInfo.OddLines[cHist] = OddLines[cHist];
		m_DIInfo.EvenLines[cHist] = EvenLines[cHist];
	}
	
	
	m_LastFrameProcessed=false;
	return S_OK;
}

HRESULT CDeinterlace::InternalProcessOutput(DWORD dwFlags,
									DWORD cOutputBufferCount,
									DMO_OUTPUT_DATA_BUFFER *pOutputBuffers,
									DWORD *pdwStatus)
{
	//output buffer
	PBYTE pbOutputBuffer;
	DWORD cbOutputBuffer;
	
	//ATLTRACE("%s(%d) : InternalProcessOutput\n",__FILE__,__LINE__);
	//ATLTRACE("%s(%d) :  dwFlags: %#.Lx cOutputBufferCount: %#.Lx\n",__FILE__,__LINE__,dwFlags,cOutputBufferCount);
	
	//make sure we only process the latest frame once
	//(thers an extra InternalProcessOutput after InternalDiscontinuity gets called)
	if(m_LastFrameProcessed==true)
		return S_FALSE;

	//retrieve pointer to destination buffer
	HRESULT hr = pOutputBuffers[0].pBuffer->GetBufferAndLength(&pbOutputBuffer, &cbOutputBuffer);
	if(FAILED(hr))
		return hr;
	hr = pOutputBuffers[0].pBuffer->GetMaxLength(&cbOutputBuffer);
	if(FAILED(hr))
		return hr;
	
	BOOL bResult;

	DWORD cbInputSize;
	BYTE *pbInputBuffer;
	hr=m_InputHistory[0].m_Buffer->GetBufferAndLength(&pbInputBuffer,&cbInputSize);
	if(FAILED(hr))
		return hr;

	//all data in input buffer must fit in output buffer
	ATLASSERT(cbOutputBuffer>=cbInputSize);

	//is there a plugin loaded?
	if(m_DIPlugin!=NULL)
	{
		//do we have enough fileds?
		if(m_InputSize*2<m_DIPlugin->nFieldsRequired)
		{
			m_LastFrameProcessed=true;
			return S_FALSE;
		}
		
		m_DIInfo.Overlay=pbOutputBuffer;
		
		//this needs fixing
		switch(m_mode)
		{
		case DI_ALWAYS_ODD:
			m_DIInfo.IsOdd=TRUE;
			break;
		case DI_ALWAYS_EVEN:
			m_DIInfo.IsOdd=FALSE;
			break;
		case DI_FIELDINPUT:
			m_DIInfo.IsOdd=m_DIInfo.IsOdd==FALSE ? TRUE : FALSE;
			break;
		case DI_FRAMEINPUT:
		default:
			m_DIInfo.IsOdd=FALSE;
		}
		m_DIInfo.CombFactor=-1;
		m_DIInfo.FieldDiff=-1;
		
		//run plugin
		bResult=m_DIPlugin->pfnAlgorithm(&m_DIInfo);
	}
	else
	{
		//no plugin loaded, just copy frame
		//shoud probably use optimized memcpy
		memcpy(pbOutputBuffer,pbInputBuffer,cbInputSize);
		//memcpyMMX(pbOutputBuffer,pbInputBuffer,cbInputSize);
		bResult=TRUE;
	}
	
	//success?
	if(bResult==TRUE)
	{
		m_LastFrameProcessed=true;
		hr = pOutputBuffers[0].pBuffer->SetLength(cbInputSize);
		
		//ställ in tidstämpel, använd den älsta
		//dvs se till att man inte missar nån timestamp
		//ATLTRACE("%s(%d) :  Flags: %#.Lx TimeStamp: %#.1I64x TimeLength: %#.1I64x\n",__FILE__,__LINE__,m_InputHistory[m_InputSize-1].m_flags,m_InputHistory[m_InputSize-1].m_rtTimestamp,m_InputHistory[m_InputSize-1].m_rtTimelength);
		
		pOutputBuffers[0].dwStatus=m_InputHistory[m_InputSize-1].m_flags;
		//pOutputBuffers[0].dwStatus|=DMO_OUTPUT_DATA_BUFFERF_SYNCPOINT;
		
		pOutputBuffers[0].rtTimestamp=m_InputHistory[m_InputSize-1].m_rtTimestamp;
		pOutputBuffers[0].rtTimelength=m_InputHistory[m_InputSize-1].m_rtTimelength;
		
		
		//release one buffer, since we have processed one frame we can release the oldest one
		//if we dont release a buffer we might not get anymore input
		
		//if((m_DIPlugin!=NULL && m_InputSize*2>=m_DIPlugin->nFieldsRequired) || m_DIPlugin==NULL)
		m_InputHistory[m_InputSize-1].Reset();
		
		return S_OK;
	}
	else
	{
		//if everything goes well, we shodnt end up here
		m_LastFrameProcessed=true;
		return S_FALSE;
	}
}

HRESULT CDeinterlace::InternalAcceptingInput(DWORD dwInputStreamIndex)
{
	//ATLTRACE("%s(%d) : InternalAcceptingInput: %s\n",__FILE__,__LINE__,m_LastFrameProcessed==true ? "true" : "false");
	return m_LastFrameProcessed==true ? S_OK : S_FALSE;
}

HRESULT CDeinterlace::CanPerformDeinterlace(const DMO_MEDIA_TYPE *pmt)
{
	//check if its a video format
	if(pmt->majortype==MEDIATYPE_Video)
	{
		//retrieve bitmapinfoheader pointers
		LPBITMAPINFOHEADER pbi;
		if(pmt->formattype==FORMAT_VideoInfo)
			pbi=&(((VIDEOINFOHEADER *)pmt->pbFormat)->bmiHeader);
		else
			return DMO_E_INVALIDTYPE;
		
		if(pmt->subtype==MEDIASUBTYPE_YUY2 || pmt->subtype==MEDIASUBTYPE_RGB24)
			if(pbi->biBitCount==16 || pbi->biBitCount==24)
				return S_OK;
	}
	return DMO_E_INVALIDTYPE;
}


STDMETHODIMP CDeinterlace::LoadPlugin(LPCSTR szFileName)
{
	ATLTRACE("%s(%d) : LoadPlugin\n",__FILE__,__LINE__);
	
	USES_CONVERSION;
	GETDEINTERLACEPLUGININFO* pfnGetDeinterlacePluginInfo;
	DEINTERLACE_METHOD* pMethod;
	HMODULE hPlugInMod;
	HRESULT hr;

	hPlugInMod=LoadLibrary(szFileName);
	if(hPlugInMod==NULL)
	{
		char *str=new char[strlen(szFileName)+30];
		wsprintf(str,"Failed to load plugin %s",szFileName);
		CreateErrorInfoObj(str);
		delete [] str;
		return E_FAIL;
	}

	pfnGetDeinterlacePluginInfo=(GETDEINTERLACEPLUGININFO*)GetProcAddress(hPlugInMod, "GetDeinterlacePluginInfo");
	if(pfnGetDeinterlacePluginInfo==NULL)
	{
		CreateErrorInfoObj("GetProcAddress() failed, Are you sure this is a valid plugin?");
		return E_FAIL;
	}

	pMethod=pfnGetDeinterlacePluginInfo(CpuFeatureFlags);
	if(pMethod!=NULL)
	{
		//check that the plugin has the right api version
		if(pMethod->SizeOfStructure!=sizeof(DEINTERLACE_METHOD) || pMethod->DeinterlaceStructureVersion!=DEINTERLACE_CURRENT_VERSION)
		{
			CreateErrorInfoObj("This version of the plugin is not supported");
			FreeLibrary(hPlugInMod);
			return E_FAIL;
		}

		if(pMethod->bNeedFieldDiff==TRUE ||
			pMethod->bNeedCombFactor==TRUE ||
			pMethod->bIsHalfHeight==TRUE)
		{
			CreateErrorInfoObj("Plugins that need FieldDiff, CombFactor or IsHalfHeight is currently not supported");
			FreeLibrary(hPlugInMod);
			return E_FAIL;
		}
		
		//is a plugin already loaded?
		if(m_DIPlugin!=NULL)
		{
			//remove it
			hr=UnloadPlugin();
			if(FAILED(hr))
			{
				FreeLibrary(hPlugInMod);
				return hr;
			}
		}
		
		//lock dmo
		LockIt lck(this);
		
		m_DIPlugin=pMethod;
		m_DIPlugin->hModule=hPlugInMod;

		if(m_DIPlugin->pfnPluginStart!=NULL)
		{
			//FIXME: last param, status callbacks
			m_DIPlugin->pfnPluginStart(0, NULL,NULL);
		}
		
		return S_OK;
	}
	
	return E_FAIL;
}

STDMETHODIMP CDeinterlace::UnloadPlugin()
{
	ATLTRACE("%s(%d) : UnloadPlugin\n",__FILE__,__LINE__);

	if(m_DIPlugin==NULL)
		return S_OK;
	
	//lock dmo
	LockIt lck(this);
	
	//call plugin exit function if present
	if(m_DIPlugin->pfnPluginExit != NULL)
		m_DIPlugin->pfnPluginExit();
	
	if(!FreeLibrary(m_DIPlugin->hModule))
	{
		CreateErrorInfoObj("FreeLibrary() failed");
		return E_FAIL;
	}
	m_DIPlugin=NULL;

	return S_OK;
}

STDMETHODIMP CDeinterlace::IsPluginLoaded(VARIANT_BOOL *pIsLoaded)
{
	ATLTRACE("%s(%d) : IsPluginLoaded\n",__FILE__,__LINE__);
	if(pIsLoaded==NULL)
		return E_POINTER;
	
	*pIsLoaded=m_DIPlugin!=NULL ? OATRUE : OAFALSE;
	return S_OK;
}

STDMETHODIMP CDeinterlace::GetPluginName(unsigned char **szName)
{
	ATLTRACE("%s(%d) : GetPluginName\n",__FILE__,__LINE__);
	
	LockIt lck(this);
	
	
	if(m_DIPlugin==NULL)
		return E_FAIL;
	
	//CComBSTR str(m_DIPlugin->szName);
	//*name=str.Copy();
	*szName=(unsigned char*)m_DIPlugin->szName;
	
	return S_OK;
}

STDMETHODIMP CDeinterlace::GetSetting(int nIndex, DISETTING *pSetting)
{
	ATLTRACE("%s(%d) : GetSetting\n",__FILE__,__LINE__);
	
	LockIt lck(this);
	
	if(m_DIPlugin==NULL)
		return E_FAIL;

	if(pSetting==NULL)
		return E_POINTER;
	
	if(nIndex>m_DIPlugin->nSettings || nIndex<0)
		return E_INVALIDARG;
	
	pSetting->szDisplayName=(unsigned char *)m_DIPlugin->pSettings[nIndex].szDisplayName;
	pSetting->Type=(DISETTING::DISETTING_TYPE)m_DIPlugin->pSettings[nIndex].Type;
	pSetting->Default=m_DIPlugin->pSettings[nIndex].Default;
	pSetting->MinValue=m_DIPlugin->pSettings[nIndex].MinValue;
	pSetting->MaxValue=m_DIPlugin->pSettings[nIndex].MaxValue;
	pSetting->StepValue=m_DIPlugin->pSettings[nIndex].StepValue;
	
	return S_OK;
}

STDMETHODIMP CDeinterlace::GetSettingCount(long *pCount)
{
	ATLTRACE("%s(%d) : GetSettingCount\n",__FILE__,__LINE__);
	
	LockIt lck(this);
	
	if(pCount==NULL)
		return E_POINTER;
	if(m_DIPlugin==NULL)
		return E_FAIL;
	
	*pCount=m_DIPlugin->nSettings;
	return S_OK;
}

STDMETHODIMP CDeinterlace::put_SettingValue(int nIndex, long lValue)
{
	ATLTRACE("%s(%d) : PutSettingValue\n",__FILE__,__LINE__);
	
	LockIt lck(this);
	
	if(m_DIPlugin==NULL)
		return E_FAIL;
	if(nIndex>m_DIPlugin->nSettings || nIndex<0)
		return E_INVALIDARG;
	
	if(lValue<m_DIPlugin->pSettings[nIndex].MinValue || lValue>m_DIPlugin->pSettings[nIndex].MaxValue)
		return E_FAIL;

	*m_DIPlugin->pSettings[nIndex].pValue=lValue;
	if(m_DIPlugin->pSettings[nIndex].pfnOnChange!=NULL)
		m_DIPlugin->pSettings[nIndex].pfnOnChange(lValue);
	
	return S_OK;
}

STDMETHODIMP CDeinterlace::get_SettingValue(int nIndex, long *pValue)
{
	ATLTRACE("%s(%d) : GetSettingValue\n",__FILE__,__LINE__);
	
	LockIt lck(this);
	
	if(m_DIPlugin==NULL)
		return E_FAIL;
	if(pValue==NULL)
		return E_POINTER;
	if(nIndex>m_DIPlugin->nSettings || nIndex<0)
		return E_INVALIDARG;
	
	*pValue=*m_DIPlugin->pSettings[nIndex].pValue;
	return S_OK;
}

STDMETHODIMP CDeinterlace::ShowPluginUI(long *hwndParent)
{
	ATLTRACE("%s(%d) : ShowUI\n",__FILE__,__LINE__);

	if(m_DIPlugin==NULL)
		return E_FAIL;
	if(m_DIPlugin->pfnPluginShowUI==NULL)
		return E_FAIL;
	
	//show plugin UI
	m_DIPlugin->pfnPluginShowUI((HWND)hwndParent);

	return S_OK;
}

STDMETHODIMP CDeinterlace::PluginHasUI(VARIANT_BOOL *hasUI)
{
	USES_CONVERSION;
	if(m_DIPlugin==NULL)
	{
		CComPtr<ICreateErrorInfo> cerrinfo;
		CComPtr<IErrorInfo> errinfo;

		if(SUCCEEDED(CreateErrorInfo(&cerrinfo)))
		{
			if(SUCCEEDED(cerrinfo.QueryInterface(&errinfo)))
			{
				cerrinfo->SetDescription(T2OLE("No plugin is loaded"));
				SetErrorInfo(0,errinfo);
			}
		}
		return E_FAIL;
	}
	
	if(m_DIPlugin->pfnPluginShowUI==NULL)
	{
		*hasUI=OAFALSE;
	}
	else
	{
		*hasUI=OATRUE;
	}
	return S_OK;
}

STDMETHODIMP CDeinterlace::get_Mode(FieldFrameMode *pVal)
{
	if(pVal==NULL)
		return E_POINTER;
	*pVal=m_mode;
	return S_OK;
}

STDMETHODIMP CDeinterlace::put_Mode(FieldFrameMode newVal)
{
	switch(newVal)
	{
	case DI_ALWAYS_ODD:
	case DI_ALWAYS_EVEN:
	case DI_FIELDINPUT:
		m_mode=newVal;
		break;
	case DI_FRAMEINPUT:
		CreateErrorInfoObj("This mode is not yet implemented");
		return E_NOTIMPL;
	}
	return S_OK;
}

void CDeinterlace::CreateErrorInfoObj(LPTSTR errorText)
{
	USES_CONVERSION;
	CComPtr<ICreateErrorInfo> cerrinfo;
	CComPtr<IErrorInfo> errinfo;

	if(SUCCEEDED(CreateErrorInfo(&cerrinfo)))
	{
		if(SUCCEEDED(cerrinfo.QueryInterface(&errinfo)))
		{
			cerrinfo->SetDescription(T2OLE(errorText));
			SetErrorInfo(0,errinfo);
		}
	}
}
