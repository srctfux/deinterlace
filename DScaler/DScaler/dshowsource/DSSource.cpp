/////////////////////////////////////////////////////////////////////////////
// $Id: DSSource.cpp,v 1.5 2002-02-03 11:00:43 tobbej Exp $
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
// Revision 1.4  2001/12/18 13:12:12  adcockj
// Interim check-in for redesign of card specific settings
//
// Revision 1.3  2001/12/17 19:39:38  tobbej
// implemented the picture history and field management
// crossbar support.
//
// Revision 1.2  2001/12/14 14:11:13  adcockj
// Added #ifdef to allow compilation without SDK
//
// Revision 1.1  2001/12/09 22:01:48  tobbej
// experimental dshow support, doesnt work yet
// define WANT_DSHOW_SUPPORT if you want to try it
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DSSource.cpp implementation of the CDSSource class.
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT
#include "dscaler.h"
#include "..\DScalerRes\resource.h"
#include "DSSource.h"
#include <dvdmedia.h>		//VIDEOINFOHEADER2

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/// @todo better error handling
CDSSource::CDSSource(string device,string deviceName) :
	CSource(0,IDC_DSHOWSOURCEMENU),
	m_pDSGraph(NULL),
	m_device(device),
	m_deviceName(deviceName),
	m_currentX(0),
	m_currentY(0),
	m_cbFieldSize(0),
	m_bProcessingFirstField(true),
	m_pictureHistoryPos(0)

{
	CreateSettings(device.c_str());

	for(int i=0;i<MAX_PICTURE_HISTORY;i++)
	{
		m_pictureHistory[i].IsFirstInSeries=FALSE;
		m_pictureHistory[i].pData=NULL;
		m_pictureHistory[i].Flags= i%2 ? PICTURE_INTERLACED_ODD : PICTURE_INTERLACED_EVEN;
	}
}

CDSSource::~CDSSource()
{
	if(m_pDSGraph!=NULL)
	{
		delete m_pDSGraph;
		m_pDSGraph=NULL;
	}

	for(int i=0;i<MAX_PICTURE_HISTORY;i++)
	{
		if(m_pictureHistory[i].pData!=NULL)
		{
			delete m_pictureHistory[i].pData;
		}
	}
}

ISetting* CDSSource::GetBrightness()
{
	if(m_pDSGraph==NULL)
		return NULL;
	
	if(m_pDSGraph->getCaptureDevice()->hasVideoProcAmp())
	{
		long min;
		long max;
		long def;
		long value;

		try
		{
			m_pDSGraph->getCaptureDevice()->getRange(VideoProcAmp_Brightness,&min,&max,NULL,&def);
			m_pDSGraph->getCaptureDevice()->get(VideoProcAmp_Brightness,&value);
			m_Brightness->SetMax(max);
			m_Brightness->SetMin(min);

			//why does ChangeDefault() change the value itself too?
			m_Brightness->ChangeDefault(def);
			m_Brightness->SetValue(value);
			return m_Brightness;
		}
		catch(CDShowException e)
		{}
	}
	return NULL;
}

void CDSSource::BrightnessOnChange(long Brightness, long OldValue)
{
	if(Brightness==OldValue)
		return;
	if(m_pDSGraph==NULL)
		return;
	try
	{
		m_pDSGraph->getCaptureDevice()->set(VideoProcAmp_Brightness,Brightness,VideoProcAmp_Flags_Manual);
	}
	catch(CDShowException e)
	{
		AfxMessageBox(e.getErrorText(),MB_OK|MB_ICONEXCLAMATION);
	}
}

ISetting* CDSSource::GetContrast()
{
	if(m_pDSGraph==NULL)
		return NULL;

	if(m_pDSGraph->getCaptureDevice()->hasVideoProcAmp())
	{
		long min;
		long max;
		long def;
		long value;

		try
		{
			m_pDSGraph->getCaptureDevice()->getRange(VideoProcAmp_Contrast,&min,&max,NULL,&def);
			m_pDSGraph->getCaptureDevice()->get(VideoProcAmp_Contrast,&value);
			m_Contrast->SetMax(max);
			m_Contrast->SetMin(min);
			m_Contrast->ChangeDefault(def);
			m_Contrast->SetValue(value);
			return m_Contrast;
		}
		catch(CDShowException e)
		{}
	}
	return NULL;
}

void CDSSource::ContrastOnChange(long Contrast, long OldValue)
{
	if(Contrast==OldValue)
		return;
	if(m_pDSGraph==NULL)
		return;
	try
	{
		m_pDSGraph->getCaptureDevice()->set(VideoProcAmp_Contrast,Contrast,VideoProcAmp_Flags_Manual);
	}
	catch(CDShowException e)
	{
		AfxMessageBox(e.getErrorText(),MB_OK|MB_ICONEXCLAMATION);
	}
}

ISetting* CDSSource::GetHue()
{
	if(m_pDSGraph==NULL)
		return NULL;

	if(m_pDSGraph->getCaptureDevice()->hasVideoProcAmp())
	{
		long min;
		long max;
		long def;
		long value;
		
		try
		{
			m_pDSGraph->getCaptureDevice()->getRange(VideoProcAmp_Hue,&min,&max,NULL,&def);
			m_pDSGraph->getCaptureDevice()->get(VideoProcAmp_Hue,&value);
			m_Hue->SetMax(max);
			m_Hue->SetMin(min);
			m_Hue->ChangeDefault(def);
			m_Hue->SetValue(value);
			return m_Hue;
		}
		catch(CDShowException e)
		{}
	}
	return NULL;
}

void CDSSource::HueOnChange(long Hue, long OldValue)
{
	if(Hue==OldValue)
		return;
	if(m_pDSGraph==NULL)
		return;
	try
	{
		m_pDSGraph->getCaptureDevice()->set(VideoProcAmp_Hue,Hue,VideoProcAmp_Flags_Manual);
	}
	catch(CDShowException e)
	{
		AfxMessageBox(e.getErrorText(),MB_OK|MB_ICONEXCLAMATION);
	}
}

ISetting* CDSSource::GetSaturation()
{
	if(m_pDSGraph==NULL)
		return NULL;

	if(m_pDSGraph->getCaptureDevice()->hasVideoProcAmp())
	{
		long min;
		long max;
		long def;
		long value;
		
		try
		{
			m_pDSGraph->getCaptureDevice()->getRange(VideoProcAmp_Saturation,&min,&max,NULL,&def);
			m_pDSGraph->getCaptureDevice()->get(VideoProcAmp_Saturation,&value);
			m_Saturation->SetMax(max);
			m_Saturation->SetMin(min);
			m_Saturation->ChangeDefault(def);
			m_Saturation->SetValue(value);
			return m_Saturation;
		}
		catch(CDShowException e)
		{}
	}
	return NULL;
}

void CDSSource::SaturationOnChange(long Saturation, long OldValue)
{
	if(Saturation==OldValue)
		return;
	if(m_pDSGraph==NULL)
		return;
	try
	{
		m_pDSGraph->getCaptureDevice()->set(VideoProcAmp_Saturation,Saturation,VideoProcAmp_Flags_Manual);
	}
	catch(CDShowException e)
	{
		AfxMessageBox(e.getErrorText(),MB_OK|MB_ICONEXCLAMATION);
	}
}

void CDSSource::CreateSettings(LPCSTR IniSection)
{
	m_Brightness = new CBrightnessSetting(this, "Brightness", 0, 0, 0, IniSection);
	m_Settings.push_back(m_Brightness);

	m_Contrast = new CContrastSetting(this, "Contrast", 0, 0, 0, IniSection);
	m_Settings.push_back(m_Contrast);

	m_Hue = new CHueSetting(this, "Hue", 0, 0, 0, IniSection);
	m_Settings.push_back(m_Hue);

	m_Saturation = new CSaturationSetting(this, "Saturation", 0, 0, 0, IniSection);
	m_Settings.push_back(m_Saturation);

}

BOOL CDSSource::HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
{
	if(LOWORD(wParam)>=IDM_CROSSBAR_INPUT0 && LOWORD(wParam)<=IDM_CROSSBAR_INPUT_MAX)
	{
		try
		{
			if(m_pDSGraph!=NULL)
			{
				CDShowBaseCrossbar *pCrossbar=m_pDSGraph->getCaptureDevice()->getCrossbar();
				if(pCrossbar!=NULL)
				{
					//if changing a video input, set the related pin too
					bool isVideo=pCrossbar->GetInputType(LOWORD(wParam)-IDM_CROSSBAR_INPUT0)<4096;
					pCrossbar->SetInputIndex(LOWORD(wParam)-IDM_CROSSBAR_INPUT0,isVideo);
				}
			}
		}
		catch(CDShowException &e)
		{
			AfxMessageBox(CString("Failed to change input\n\n")+e.getErrorText(),MB_OK|MB_ICONERROR);
		}

		return TRUE;
	}
	switch(LOWORD(wParam))
	{
	case IDM_DSHOW_RENDERERPROPERTIES:
		if(m_pDSGraph!=NULL)
		{
			m_pDSGraph->showRendererProperies(hWnd);
		}
		return TRUE;
		break;
	}
	
	return FALSE;
}

void CDSSource::Start()
{
	m_pictureHistoryPos=0;
	try
	{
		CWaitCursor wait;
		if(m_pDSGraph==NULL)
		{
			m_pDSGraph=new CDShowGraph(m_device,m_deviceName);
		}
		m_pDSGraph->start();
		
		//get the stored settings
		ReadFromIni();
	}
	catch(CDShowException &e)
	{
		AfxMessageBox(e.getErrorText(),MB_OK|MB_ICONERROR);
	}
}

void CDSSource::Stop()
{
	if(m_pDSGraph!=NULL)
	{
		try
		{
			WriteToIni(TRUE);
			m_pDSGraph->stop();
			delete m_pDSGraph;
			m_pDSGraph=NULL;
		}
		catch(CDShowException &e)
		{
			AfxMessageBox(e.getErrorText(),MB_OK|MB_ICONERROR);
		}
	}
	//shoud probably free the memory allocated by the picture history array
}

void CDSSource::Reset()
{
	Stop();
	Start();
}

eVideoFormat CDSSource::GetFormat()
{
	return VIDEOFORMAT_PAL_B;
}

BOOL CDSSource::IsInTunerMode()
{
	return FALSE;
}

int CDSSource::GetWidth()
{
	return m_currentX;
}

int CDSSource::GetHeight()
{
	return m_currentY;
}

BOOL CDSSource::HasTuner()
{
	return FALSE;
}

BOOL CDSSource::SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat)
{
	return FALSE;
}

BOOL CDSSource::IsVideoPresent()
{
	return FALSE;
}

int CDSSource::FindMenuID(CMenu *menu,UINT menuID)
{
	ASSERT(menu);
	ASSERT(::IsMenu(menu->GetSafeHmenu()));

	int count=menu->GetMenuItemCount();
	for(int i=0;i<count;i++)
	{
		if(menu->GetMenuItemID(i)==menuID)
			return i;
	}

	return -1;
}

void CDSSource::SetMenu(HMENU hMenu)
{
	if(m_pDSGraph==NULL)
	{
		return;
	}
	CMenu topMenu;
	topMenu.Attach(m_hMenu);
	CMenu *menu=topMenu.GetSubMenu(0);
	
	//setup input selection menus
	CString menuText;
	CDShowBaseCrossbar *pCrossbar=m_pDSGraph->getCaptureDevice()->getCrossbar();
	if(pCrossbar!=NULL)
	{
		int vidPos=0;//FindMenuID(menu,IDM_DSHOW_VIDEOINPUT);
		int audPos=3;//FindMenuID(menu,IDM_DSHOW_AUDIOINPUT);
		CMenu vidSubMenu;
		CMenu audSubMenu;
		
		//create video input submenu and insert it
		vidSubMenu.CreateMenu();
		menu->GetMenuString(vidPos,menuText,MF_BYPOSITION);
		menu->ModifyMenu(vidPos,MF_POPUP|MF_BYPOSITION,(UINT) vidSubMenu.GetSafeHmenu(),menuText);

		//same for audio input menu
		audSubMenu.CreateMenu();
		menu->GetMenuString(audPos,menuText,MF_BYPOSITION);
		menu->ModifyMenu(audPos,MF_POPUP|MF_BYPOSITION,(UINT) audSubMenu.GetSafeHmenu(),menuText);

		for(int i=0;i<pCrossbar->GetInputCount();i++)
		{
			ASSERT((IDM_CROSSBAR_INPUT0+i)<=IDM_CROSSBAR_INPUT_MAX);
			bool bSelected=pCrossbar->isInputSelected(i);

			//is it an audio or video input?
			if(pCrossbar->GetInputType(i)<4096)
			{
				vidSubMenu.AppendMenu(MF_STRING,IDM_CROSSBAR_INPUT0+i,pCrossbar->GetInputName(i));
				vidSubMenu.CheckMenuItem(IDM_CROSSBAR_INPUT0+i,bSelected ? MF_CHECKED:MF_UNCHECKED);
			}
			else
			{
				audSubMenu.AppendMenu(MF_STRING,IDM_CROSSBAR_INPUT0+i,pCrossbar->GetInputName(i));
				audSubMenu.CheckMenuItem(IDM_CROSSBAR_INPUT0+i,bSelected ? MF_CHECKED:MF_UNCHECKED);
			}
		}
	}
	
	if(m_pDSGraph->getCaptureDevice()->hasVideoDec())
	{
		CMenu formatMenu;
		
		long formats=0;
		long selectedFormat=0;
		try
		{
			formats=m_pDSGraph->getCaptureDevice()->getSupportedTVFormats();
			selectedFormat=m_pDSGraph->getCaptureDevice()->getTVFormat();
		}
		catch(CDShowException e)
		{}
		
		//make sure there is atleast one format to be selected
		if(formats!=0)
		{
			formatMenu.CreateMenu();
			menu->GetMenuString(1,menuText,MF_BYPOSITION);
			menu->ModifyMenu(1,MF_POPUP|MF_BYPOSITION,(UINT)formatMenu.GetSafeHmenu(),menuText);
			
			int i=0;
			if(formats&AnalogVideo_NTSC_M)
			{
				formatMenu.AppendMenu(MF_STRING,0+i,"NTSC_M");
				formatMenu.CheckMenuItem(0+i,selectedFormat&AnalogVideo_NTSC_M ? MF_CHECKED:MF_UNCHECKED);
				i++;
			}
			if(formats&AnalogVideo_NTSC_M_J)
			{
				formatMenu.AppendMenu(MF_STRING,0+i,"NTSC_M_J");
				formatMenu.CheckMenuItem(0+i,selectedFormat&AnalogVideo_NTSC_M_J ? MF_CHECKED:MF_UNCHECKED);
				i++;
			}
			if(formats&AnalogVideo_NTSC_433)
			{
				formatMenu.AppendMenu(MF_STRING,0+i,"NTSC_433");
				formatMenu.CheckMenuItem(0+i,selectedFormat&AnalogVideo_NTSC_433 ? MF_CHECKED:MF_UNCHECKED);
				i++;
			}
			if(formats&AnalogVideo_PAL_B)
			{
				formatMenu.AppendMenu(MF_STRING,0+i,"PAL_B");
				formatMenu.CheckMenuItem(0+i,selectedFormat&AnalogVideo_PAL_B ? MF_CHECKED:MF_UNCHECKED);
				i++;
			}
			if(formats&AnalogVideo_PAL_D)
			{
				formatMenu.AppendMenu(MF_STRING,0+i,"PAL_D");
				formatMenu.CheckMenuItem(0+i,selectedFormat&AnalogVideo_PAL_D ? MF_CHECKED:MF_UNCHECKED);
				i++;
			}
			if(formats&AnalogVideo_PAL_H)
			{
				formatMenu.AppendMenu(MF_STRING,0+i,"PAL_H");
				formatMenu.CheckMenuItem(0+i,selectedFormat&AnalogVideo_PAL_H ? MF_CHECKED:MF_UNCHECKED);
				i++;
			}
			if(formats&AnalogVideo_PAL_I)
			{
				formatMenu.AppendMenu(MF_STRING,0+i,"PAL_I");
				formatMenu.CheckMenuItem(0+i,selectedFormat&AnalogVideo_PAL_I ? MF_CHECKED:MF_UNCHECKED);
				i++;
			}
			if(formats&AnalogVideo_PAL_M)
			{
				formatMenu.AppendMenu(MF_STRING,0+i,"PAL_M");
				formatMenu.CheckMenuItem(0+i,selectedFormat&AnalogVideo_PAL_M ? MF_CHECKED:MF_UNCHECKED);
				i++;
			}
			if(formats&AnalogVideo_PAL_N)
			{
				formatMenu.AppendMenu(MF_STRING,0+i,"PAL_N");
				formatMenu.CheckMenuItem(0+i,selectedFormat&AnalogVideo_PAL_N ? MF_CHECKED:MF_UNCHECKED);
				i++;
			}
			if(formats&AnalogVideo_PAL_60)
			{
				formatMenu.AppendMenu(MF_STRING,0+i,"PAL_60");
				formatMenu.CheckMenuItem(0+i,selectedFormat&AnalogVideo_PAL_60 ? MF_CHECKED:MF_UNCHECKED);
				i++;
			}
			if(formats&AnalogVideo_SECAM_B)
			{
				formatMenu.AppendMenu(MF_STRING,0+i,"SECAM_B");
				formatMenu.CheckMenuItem(0+i,selectedFormat&AnalogVideo_SECAM_B ? MF_CHECKED:MF_UNCHECKED);
				i++;
			}
			if(formats&AnalogVideo_SECAM_D)
			{
				formatMenu.AppendMenu(MF_STRING,0+i,"SECAM_D");
				formatMenu.CheckMenuItem(0+i,selectedFormat&AnalogVideo_SECAM_D ? MF_CHECKED:MF_UNCHECKED);
				i++;
			}
			if(formats&AnalogVideo_SECAM_G)
			{
				formatMenu.AppendMenu(MF_STRING,0+i,"SECAM_G");
				formatMenu.CheckMenuItem(0+i,selectedFormat&AnalogVideo_SECAM_G ? MF_CHECKED:MF_UNCHECKED);
				i++;
			}
			if(formats&AnalogVideo_SECAM_H)
			{
				formatMenu.AppendMenu(MF_STRING,0+i,"SECAM_H");
				formatMenu.CheckMenuItem(0+i,selectedFormat&AnalogVideo_SECAM_H ? MF_CHECKED:MF_UNCHECKED);
				i++;
			}
			if(formats&AnalogVideo_SECAM_K)
			{
				formatMenu.AppendMenu(MF_STRING,0+i,"SECAM_K");
				formatMenu.CheckMenuItem(0+i,selectedFormat&AnalogVideo_SECAM_K ? MF_CHECKED:MF_UNCHECKED);
				i++;
			}
			if(formats&AnalogVideo_SECAM_K1)
			{
				formatMenu.AppendMenu(MF_STRING,0+i,"SECAM_K1");
				formatMenu.CheckMenuItem(0+i,selectedFormat&AnalogVideo_SECAM_K1 ? MF_CHECKED:MF_UNCHECKED);
				i++;
			}
			if(formats&AnalogVideo_SECAM_L)
			{
				formatMenu.AppendMenu(MF_STRING,0+i,"SECAM_L");
				formatMenu.CheckMenuItem(0+i,selectedFormat&AnalogVideo_SECAM_L ? MF_CHECKED:MF_UNCHECKED);
				i++;
			}
			if(formats&AnalogVideo_SECAM_L1)
			{
				formatMenu.AppendMenu(MF_STRING,0+i,"SECAM_L1");
				formatMenu.CheckMenuItem(0+i,selectedFormat&AnalogVideo_SECAM_L1 ? MF_CHECKED:MF_UNCHECKED);
				i++;
			}
			if(formats&AnalogVideo_PAL_N_COMBO)
			{
				formatMenu.AppendMenu(MF_STRING,0+i,"PAL_N_COMBO");
				formatMenu.CheckMenuItem(0+i,selectedFormat&AnalogVideo_PAL_N_COMBO ? MF_CHECKED:MF_UNCHECKED);
				i++;
			}

		}


	}

	topMenu.Detach();
}

void CDSSource::HandleTimerMessages(int TimerId)
{

}

LPCSTR CDSSource::GetMenuLabel()
{
	return NULL;
}

LPCSTR CDSSource::GetStatus()
{
	return "This source dont work yet";
}

void CDSSource::GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming)
{
	if(m_pDSGraph==NULL)
	{
		return;
	}
	
	//which field in the history is the first one to be returned
	int historyStart=0;

	//get a new frame or not
	if(m_bProcessingFirstField)
	{
		//get the current media type
		AM_MEDIA_TYPE mediaType;
		memset(&mediaType,0,sizeof(mediaType));
		try
		{
			m_pDSGraph->getConnectionMediatype(&mediaType);
		}
		catch(CDShowException &e)
		{
			AfxMessageBox(e.getErrorText(),MB_OK|MB_ICONERROR);
			return;
		}
		
		BITMAPINFOHEADER *bmi=NULL;
		if(mediaType.formattype==FORMAT_VideoInfo)
		{
			VIDEOINFOHEADER *videoInfo=(VIDEOINFOHEADER*)mediaType.pbFormat;
			bmi=&(videoInfo->bmiHeader);
		}
		else if(mediaType.formattype==FORMAT_VideoInfo2)
		{
			VIDEOINFOHEADER2 *videoInfo2=(VIDEOINFOHEADER2*)mediaType.pbFormat;
			bmi=&(videoInfo2->bmiHeader);
		}
		
		//info to return if we fail
		pInfo->bRunningLate=TRUE;
		pInfo->bMissedFrame=TRUE;
		pInfo->FrameWidth=10;
		pInfo->FrameHeight=10;
		CComPtr<IMediaSample> pSample;
		if(!m_pDSGraph->getNextSample(pSample))
		{
			return;
		}

		LONG size=pSample->GetActualDataLength();
		BYTE *pData;
		HRESULT hr=pSample->GetPointer(&pData);
		if(FAILED(hr))
		{
			//oops, coudn't get any pointer
			return;
		}

		//is the buffers large enough?
		LONG fieldSize=bmi->biSizeImage/2;
		if(m_cbFieldSize<fieldSize)
		{
			//no, fix it
			for(int i=0;i<MAX_PICTURE_HISTORY;i++)
			{
				if(m_pictureHistory[i].pData!=NULL)
				{
					delete m_pictureHistory[i].pData;
					m_pictureHistory[i].pData=NULL;
				}
				m_pictureHistory[i].pData=new BYTE[fieldSize];
			}
			m_cbFieldSize=fieldSize;
			m_pictureHistory[0].IsFirstInSeries=TRUE;
		}
		else
		{
			m_pictureHistory[0].IsFirstInSeries=FALSE;
		}
		ASSERT(m_cbFieldSize>=size/2);
		
		
		//split the media sample into even and odd field
		long lineSize=bmi->biWidth*bmi->biBitCount/8;
		for(int i=0;i<(bmi->biHeight/2);i++)
		{
			pInfo->pMemcpy(m_pictureHistory[m_pictureHistoryPos].pData+i*lineSize,pData+(2*i)*lineSize,lineSize);
			pInfo->pMemcpy(m_pictureHistory[m_pictureHistoryPos+1].pData+i*lineSize,pData+(2*i+1)*lineSize,lineSize);
		}

		m_pictureHistory[m_pictureHistoryPos].Flags=PICTURE_INTERLACED_EVEN;
		m_pictureHistory[m_pictureHistoryPos+1].Flags=PICTURE_INTERLACED_ODD;

		m_currentX=bmi->biWidth;
		m_currentY=bmi->biHeight;
		m_bytePerPixel=bmi->biBitCount/8;
		pInfo->FrameWidth=bmi->biWidth;
		pInfo->FrameHeight=bmi->biHeight;
		pInfo->LineLength=bmi->biWidth * m_bytePerPixel;
		pInfo->FieldHeight=bmi->biHeight / 2;
		pInfo->InputPitch=pInfo->LineLength;
		pInfo->bMissedFrame=FALSE;
		pInfo->bRunningLate=FALSE;
		
		historyStart=m_pictureHistoryPos;
		
		//update m_pictureHistoryPos
		m_pictureHistoryPos+=2;
		if(m_pictureHistoryPos>=MAX_PICTURE_HISTORY)
		{
			m_pictureHistoryPos=0;
		}

		//free format block if any
		if(mediaType.cbFormat!=0)
		{
			CoTaskMemFree((PVOID)mediaType.pbFormat);
		}
		
		m_bProcessingFirstField=false;
	}
	else
	{
		pInfo->FrameHeight=m_currentY;
		pInfo->FrameWidth=m_currentX;
		pInfo->LineLength=m_currentX * m_bytePerPixel;
		pInfo->FieldHeight=pInfo->FrameHeight/2;
		pInfo->InputPitch=pInfo->LineLength;
		pInfo->bMissedFrame=FALSE;

		historyStart=m_pictureHistoryPos-1;
		m_bProcessingFirstField=true;
	}
	
	int pos=historyStart;
	for(int i=0;i<MAX_PICTURE_HISTORY;i++)
	{
		pos--;
		if(pos<0)
		{
			pos=MAX_PICTURE_HISTORY-1;
		}

		ASSERT(pos>=0 && pos<MAX_PICTURE_HISTORY);
		pInfo->PictureHistory[i]=&m_pictureHistory[pos];
	}
}
#endif