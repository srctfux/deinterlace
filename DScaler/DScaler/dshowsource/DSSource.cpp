/////////////////////////////////////////////////////////////////////////////
// $Id: DSSource.cpp,v 1.71 2004-12-14 21:25:15 laurentg Exp $
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
// Revision 1.70  2004/12/12 01:17:53  laurentg
// Extended choice of resolution
//
// Revision 1.69  2003/02/22 16:48:59  tobbej
// added some comments about requierments for OpenFile (to avoid crashing)
//
// Revision 1.68  2003/02/05 19:12:40  tobbej
// added support for capture devices where audio can be rendered from directshow
// modified audio setings dialog so audio rendering can be turned off (usefull for devices with both internal and external audio)
//
// Revision 1.67  2003/01/18 10:49:10  laurentg
// SetOverscan renamed SetAspectRatioData
//
// Revision 1.66  2003/01/15 20:55:33  tobbej
// added audio channel selection menu
// fixed so video format menu is greyed when in tuner mode
//
// Revision 1.65  2003/01/12 21:27:45  adcockj
// Added flags for per settings groups
//
// Revision 1.64  2003/01/12 16:19:36  adcockj
// Added SettingsGroup activity setting
// Corrected event sequence and channel change behaviour
//
// Revision 1.63  2003/01/11 15:22:28  adcockj
// Interim Checkin of setting code rewrite
//  - Remove CSettingsGroupList class
//  - Fixed bugs in format switching
//  - Some new CSettingGroup code
//
// Revision 1.62  2003/01/10 17:52:19  adcockj
// Removed SettingFlags
//
// Revision 1.61  2003/01/10 17:38:44  adcockj
// Interrim Check in of Settings rewrite
//  - Removed SETTINGSEX structures and flags
//  - Removed Seperate settings per channel code
//  - Removed Settings flags
//  - Cut away some unused features
//
// Revision 1.60  2003/01/08 21:47:13  laurentg
// New setting for analogue blanking by source
//
// Revision 1.59  2003/01/07 23:31:23  laurentg
// New overscan settings
//
// Revision 1.58  2003/01/06 21:34:31  tobbej
// implemented GetFormat (not working yet)
// implemented fm radio support
//
// Revision 1.57  2002/12/31 13:21:22  adcockj
// Fixes for SetDefault Problems (needs testing)
//
// Revision 1.56  2002/12/14 22:29:22  tobbej
// put back the code i removed
//
// Revision 1.55  2002/12/13 20:21:42  tobbej
// fixed muting problem, but now the first channel is not set properly
//
// Revision 1.54  2002/12/10 12:58:07  adcockj
// Removed NotifyInputChange and NotifyVideoFormatChange functions and replaced with
//  calls to EventCollector->RaiseEvent
//
// Revision 1.53  2002/12/05 21:02:53  tobbej
// fixed initial channel change so it tunes properly to the last used channel.
// renamed video format to resolution in settings dialog.
// changed so the default list of resolutions is always used if resolution menu is empty.
// removed some unused channel change notification code.
//
// Revision 1.52  2002/11/10 20:57:13  tobbej
// changed IsVideoPresent, i hope this makes channel scanning work better
//
// Revision 1.51  2002/10/29 19:32:22  tobbej
// new tuner class for direct tuning to a frequency
// implemented IsVideoPresent, channel scaning shoud work now
//
// Revision 1.50  2002/10/27 12:17:29  tobbej
// implemented ITuner
//
// Revision 1.49  2002/10/26 17:51:53  adcockj
// Simplified hide cusror code and removed PreShowDialogOrMenu & PostShowDialogOrMenu
//
// Revision 1.48  2002/10/26 08:38:59  tobbej
// fixed compile problems by reverting HasTuner and SetTunerFrequency
//
// Revision 1.47  2002/10/22 04:10:12  flibuste2
// -- Modified CSource to include virtual ITuner* GetTuner();
// -- Modified HasTuner() and GetTunerId() when relevant
//
// Revision 1.46  2002/10/07 20:35:17  kooiman
// Fixed cursor hide problem
//
// Revision 1.45  2002/10/02 10:53:40  kooiman
// Fix C++ casting for eventobject.
//
// Revision 1.44  2002/09/28 14:32:47  kooiman
// Base class this pointer apparently not equal to this of main class. fixed comparison.
//
// Revision 1.43  2002/09/28 13:36:15  kooiman
// Added sender object to events and added setting flag to treesettingsgeneric.
//
// Revision 1.42  2002/09/26 10:35:34  kooiman
// Use new event code.
//
// Revision 1.41  2002/09/24 17:16:28  tobbej
// spelling error
//
// Revision 1.40  2002/09/14 17:05:49  tobbej
// implemented audio output device selection
//
// Revision 1.39  2002/09/11 17:12:05  tobbej
// prevent audio input from changing video
//
// Revision 1.38  2002/09/11 16:42:33  tobbej
// fixed so resolutions submenu can be empty and not always replaced with defaults if all resolutions is removed
//
// Revision 1.37  2002/09/07 13:32:35  tobbej
// save/restore video format settings to ini file
//
// Revision 1.36  2002/09/04 17:10:24  tobbej
// renamed some variables
// new video format configuration dialog (resolution)
//
// Revision 1.35  2002/09/02 19:32:21  kooiman
// Fixed small bug. Thread doesn't crash anymore.
//
// Revision 1.34  2002/08/27 22:09:39  kooiman
// Add get/set input for DS capture source.
//
// Revision 1.33  2002/08/21 20:29:20  kooiman
// Fixed settings and added setting for resolution. Fixed videoformat==lastone in dstvtuner.
//
// Revision 1.32  2002/08/20 16:21:28  tobbej
// split CDSSource into 3 different classes
//
// Revision 1.31  2002/08/16 09:38:30  kooiman
// Tuner fixes.
//
// Revision 1.30  2002/08/15 14:20:12  kooiman
// Improved tuner support. Added setting for video input.
//
// Revision 1.29  2002/08/14 22:03:23  kooiman
// Added TV tuner support for DirectShow capture devices
//
// Revision 1.28  2002/08/13 21:04:43  kooiman
// Add IDString() to Sources for identification purposes.
//
// Revision 1.27  2002/08/01 20:23:43  tobbej
// improved error messages when opening files.
// implemented AvgSyncOffset counter in dsrend
//
// Revision 1.26  2002/07/07 20:17:53  tobbej
// fixed deadlock when stoping input source
//
// Revision 1.25  2002/07/06 16:48:11  tobbej
// new field buffering
//
// Revision 1.24  2002/06/22 15:03:16  laurentg
// New vertical flip mode
//
// Revision 1.23  2002/05/24 15:18:32  tobbej
// changed filter properties dialog to include progpertypages from the pins
// fixed input source status
// fixed dot infront of start/pause/stop menu entries
// changed overscan settings a bit
// experimented a bit with measuring time for dscaler to process one field
//
// Revision 1.22  2002/05/02 19:50:39  tobbej
// changed dshow source filter submenu to use new tree based dialog
//
// Revision 1.21  2002/04/16 15:33:53  tobbej
// added overscan for capture devices
// added audio mute/unmute when starting and stopping source
// added waitForNextField
//
// Revision 1.20  2002/04/15 22:57:26  laurentg
// Automatic switch to "square pixels" AR mode when needed
//
// Revision 1.19  2002/04/07 14:52:13  tobbej
// fixed race when changing resolution
// improved error handling
//
// Revision 1.18  2002/04/03 19:52:30  tobbej
// added some more logging to help track the filters submenu problem
//
// Revision 1.17  2002/03/26 19:48:59  adcockj
// Improved error handling in DShow code
//
// Revision 1.16  2002/03/17 21:55:10  tobbej
// fixed resolution submenu so its properly disabled when using file input or there is no available resolutions
//
// Revision 1.15  2002/03/17 21:43:23  tobbej
// added input resolution submenu
//
// Revision 1.14  2002/03/15 23:03:51  tobbej
// reset m_bProcessingFirstFiled when starting
//
// Revision 1.13  2002/02/22 09:07:14  tobbej
// fixed small race condition when calling notifysizechange, workoutoverlaysize might have used the old size
//
// Revision 1.12  2002/02/19 16:03:37  tobbej
// removed CurrentX and CurrentY
// added new member in CSource, NotifySizeChange
//
// Revision 1.11  2002/02/13 17:02:08  tobbej
// new filter properties menu
// fixed some problems in GetNextFiled
//
// Revision 1.10  2002/02/09 02:49:23  laurentg
// Overscan now stored in a setting of the source
//
// Revision 1.9  2002/02/07 22:08:23  tobbej
// changed for new file input
//
// Revision 1.8  2002/02/05 17:52:27  tobbej
// changed alignment calc
//
// Revision 1.7  2002/02/05 17:27:17  tobbej
// fixed alignment problems
// update dropped/drawn fields stats
//
// Revision 1.6  2002/02/03 20:05:58  tobbej
// made video format menu work
// fixed color controls
// enable/disable menu items
//
// Revision 1.5  2002/02/03 11:00:43  tobbej
// added support for picure controls
// fixed menu handling
// fixed GetNextField to work with dshow filter
//
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
 * @file DSSource.cpp implementation of the CDSCaptureSource class.
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT
#include "dscaler.h"
#include "..\DScalerRes\resource.h"
#include "DSSource.h"
#include "CaptureDevice.h"
#include "AspectRatio.h"
#include "DebugLog.h"
#include "AutoCriticalSection.h"
#include "Audio.h"
#include "SettingsPerChannel.h"
#include "ProgramList.h"
#include "TreeSettingsDlg.h"
#include "DSVideoFormatPage.h"
#include "DSAudioDevicePage.h"
#include "OutThreads.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

struct videoStandardsType
{
	AnalogVideoStandard format;
	LPSTR name;
};

///array of video standards
videoStandardsType videoStandards[] =
	{
		AnalogVideo_NTSC_M,"NTSC_M",
		AnalogVideo_NTSC_M_J,"NTSC_M",
		AnalogVideo_NTSC_433,"NTSC_433",
		AnalogVideo_PAL_B,"PAL_B",
		AnalogVideo_PAL_D,"PAL_D",
		AnalogVideo_PAL_G,"PAL_G",
		AnalogVideo_PAL_H,"PAL_H",
		AnalogVideo_PAL_I,"PAL_I",
		AnalogVideo_PAL_M,"PAL_M",
		AnalogVideo_PAL_N,"PAL_N",
		AnalogVideo_PAL_60,"PAL_60",
		AnalogVideo_SECAM_B,"SECAM_B",
		AnalogVideo_SECAM_D,"SECAM_D",
		AnalogVideo_SECAM_G,"SECAM_G",
		AnalogVideo_SECAM_H,"SECAM_H",
		AnalogVideo_SECAM_K,"SECAM_K",
		AnalogVideo_SECAM_K1,"SECAM_K1",
		AnalogVideo_SECAM_L,"SECAM_L",
		AnalogVideo_SECAM_L1,"SECAM_L1",
		AnalogVideo_PAL_N_COMBO,"PAL_N_COMBO",
		(AnalogVideoStandard)0,NULL
	};

CDSCaptureSource::CDSCaptureSource(string device,string deviceName) :
	CDSSourceBase(0,IDC_DSHOWSOURCEMENU),
	m_Device(device),
	m_DeviceName(deviceName),
	m_HaveInputList(FALSE)
{
	m_IDString = std::string("DS_") + device;
	CreateSettings(device.c_str());

    eEventType EventList[] = {EVENT_CHANNEL_CHANGE,EVENT_ENDOFLIST};
    EventCollector->Register(this, EventList);
}

CDSCaptureSource::~CDSCaptureSource()
{
	//save m_VideoFmt to ini file
	if(m_VideoFmt.size()>0)
	{
		std::string data;
		for(vector<CDShowGraph::CVideoFormat>::size_type i=0;i<m_VideoFmt.size();i++)
		{
			if(data.size()!=0)
			{
				data+="&";
			}
			data+=m_VideoFmt[i];
		}
		WritePrivateProfileString(m_Device.c_str(),_T("ResolutionData"),data.c_str(),GetIniFileForSettings());
		WritePrivateProfileInt(m_Device.c_str(),_T("ResolutionSize"),data.size(),GetIniFileForSettings());
	}
	else
	{
		WritePrivateProfileString(m_Device.c_str(),_T("ResolutionData"),"",GetIniFileForSettings());
		WritePrivateProfileInt(m_Device.c_str(),_T("ResolutionSize"),0,GetIniFileForSettings());
	}
}

BOOL CDSCaptureSource::IsAccessAllowed()
{
	return TRUE;
}

BOOL CDSCaptureSource::OpenMediaFile(LPCSTR FileName, BOOL NewPlayList)
{
	return FALSE;
}

ISetting* CDSCaptureSource::GetBrightness()
{
	if(m_pDSGraph==NULL)
	{
		return NULL;
	}

	CDShowBaseSource *pSrc=m_pDSGraph->getSourceDevice();
	if(pSrc==NULL)
	{
		return NULL;
	}

	CDShowCaptureDevice *pCap=NULL;
	if(pSrc->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
	{
		pCap=(CDShowCaptureDevice*)pSrc;
		if(pCap->hasVideoProcAmp())
		{
			long min;
			long max;
			long def;

			try
			{
				pCap->getRange(VideoProcAmp_Brightness,&min,&max,NULL,&def);
				m_Brightness->SetMax(max);
				m_Brightness->SetMin(min);

				m_Brightness->ChangeDefault(def, TRUE);
				return m_Brightness;
			}
			catch(CDShowException e)
			{
				LOG(1, "Exception in CDSCaptureSource::GetBrightness - %s", (LPCSTR)e.getErrorText());
			}
		}
	}
	return NULL;
}

void CDSCaptureSource::BrightnessOnChange(long Brightness, long OldValue)
{
	if(m_pDSGraph==NULL)
	{
		return;
	}

	try
	{
		CDShowCaptureDevice *pCap=NULL;
		if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
		{
			pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
			LOG(3,"DSCaptureSource: Set brightness to %d",Brightness);
			pCap->set(VideoProcAmp_Brightness,Brightness,VideoProcAmp_Flags_Manual);
		}
	}
	catch(CDShowException e)
	{
		ErrorBox(e.getErrorText());
	}
}

ISetting* CDSCaptureSource::GetContrast()
{
	if(m_pDSGraph==NULL)
	{
		return NULL;
	}

	CDShowBaseSource *pSrc=m_pDSGraph->getSourceDevice();
	if(pSrc==NULL)
	{
		return NULL;
	}

	CDShowCaptureDevice *pCap=NULL;
	if(pSrc->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
	{
		pCap=(CDShowCaptureDevice*)pSrc;
		if(pCap->hasVideoProcAmp())
		{
			long min;
			long max;
			long def;

			try
			{
				pCap->getRange(VideoProcAmp_Contrast,&min,&max,NULL,&def);
				m_Contrast->SetMax(max);
				m_Contrast->SetMin(min);

				m_Contrast->ChangeDefault(def, TRUE);
				return m_Contrast;
			}
			catch(CDShowException e)
			{
				LOG(1, "Exception in CDSCaptureSource::GetContrast - %s", (LPCSTR)e.getErrorText());
			}
		}
	}
	return NULL;
}

void CDSCaptureSource::ContrastOnChange(long Contrast, long OldValue)
{
	if(m_pDSGraph==NULL)
	{
		return;
	}

	try
	{
		CDShowCaptureDevice *pCap=NULL;
		if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
		{
			pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
			pCap->set(VideoProcAmp_Contrast,Contrast,VideoProcAmp_Flags_Manual);
		}
	}
	catch(CDShowException e)
	{
		ErrorBox(e.getErrorText());
	}
}

ISetting* CDSCaptureSource::GetHue()
{
	if(m_pDSGraph==NULL)
	{
		return NULL;
	}

	CDShowBaseSource *pSrc=m_pDSGraph->getSourceDevice();
	if(pSrc==NULL)
	{
		return NULL;
	}

	CDShowCaptureDevice *pCap=NULL;
	if(pSrc->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
	{
		pCap=(CDShowCaptureDevice*)pSrc;
		if(pCap->hasVideoProcAmp())
		{
			long min;
			long max;
			long def;

			try
			{
				pCap->getRange(VideoProcAmp_Hue,&min,&max,NULL,&def);
				m_Hue->SetMax(max);
				m_Hue->SetMin(min);

				m_Hue->ChangeDefault(def, TRUE);
				return m_Hue;
			}
			catch(CDShowException e)
			{
				LOG(1, "Exception in CDSCaptureSource::GetHue - %s", (LPCSTR)e.getErrorText());
			}
		}
	}
	return NULL;
}

void CDSCaptureSource::HueOnChange(long Hue, long OldValue)
{
	if(m_pDSGraph==NULL)
	{
		return;
	}

	try
	{
		CDShowCaptureDevice *pCap=NULL;
		if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
		{
			pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
			pCap->set(VideoProcAmp_Hue,Hue,VideoProcAmp_Flags_Manual);
		}
	}
	catch(CDShowException e)
	{
		ErrorBox(e.getErrorText());
	}
}

ISetting* CDSCaptureSource::GetSaturation()
{
	if(m_pDSGraph==NULL)
	{
		return NULL;
	}

	CDShowBaseSource *pSrc=m_pDSGraph->getSourceDevice();
	if(pSrc==NULL)
	{
		return NULL;
	}

	CDShowCaptureDevice *pCap=NULL;
	if(pSrc->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
	{
		pCap=(CDShowCaptureDevice*)pSrc;
		if(pCap->hasVideoProcAmp())
		{
			long min;
			long max;
			long def;

			try
			{
				pCap->getRange(VideoProcAmp_Saturation,&min,&max,NULL,&def);
				m_Saturation->SetMax(max);
				m_Saturation->SetMin(min);

				m_Saturation->ChangeDefault(def, TRUE);
				return m_Saturation;
			}
			catch(CDShowException e)
			{
				LOG(1, "Exception in CDSCaptureSource::GetSaturation - %s", (LPCSTR)e.getErrorText());
			}
		}
	}
	return NULL;
}

void CDSCaptureSource::SaturationOnChange(long Saturation, long OldValue)
{
	if(m_pDSGraph==NULL)
	{
		return;
	}

	try
	{
		CDShowCaptureDevice *pCap=NULL;
		if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
		{
			pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
			pCap->set(VideoProcAmp_Saturation,Saturation,VideoProcAmp_Flags_Manual);
		}
	}
	catch(CDShowException e)
	{
		ErrorBox(e.getErrorText());
	}
}

void CDSCaptureSource::CreateSettings(LPCSTR IniSection)
{
	CDSSourceBase::CreateSettings(IniSection);

    CSettingGroup *pVideoGroup = GetSettingsGroup("DS - Video", SETTING_BY_CHANNEL | SETTING_BY_FORMAT | SETTING_BY_INPUT, TRUE);
    CSettingGroup *pOverscanGroup = GetSettingsGroup("DS - Overscan", SETTING_BY_CHANNEL | SETTING_BY_FORMAT | SETTING_BY_INPUT, FALSE);

	//at this time we dont know what the min and max will be
	m_Brightness = new CBrightnessSetting(this, "Brightness", 0, LONG_MIN, LONG_MAX, IniSection, pVideoGroup);
	m_Settings.push_back(m_Brightness);

	m_Contrast = new CContrastSetting(this, "Contrast", 0, LONG_MIN, LONG_MAX, IniSection, pVideoGroup);
	m_Settings.push_back(m_Contrast);

	m_Hue = new CHueSetting(this, "Hue", 0, LONG_MIN, LONG_MAX, IniSection, pVideoGroup);
	m_Settings.push_back(m_Hue);

	m_Saturation = new CSaturationSetting(this, "Saturation", 0, LONG_MIN, LONG_MAX, IniSection, pVideoGroup);
	m_Settings.push_back(m_Saturation);

	m_TopOverscan = new CTopOverscanSetting(this, "Overscan at Top", 0, 0, 150, IniSection, pOverscanGroup);
	m_Settings.push_back(m_TopOverscan);

	m_BottomOverscan = new CBottomOverscanSetting(this, "Overscan at Bottom", 0, 0, 150, IniSection, pOverscanGroup);
	m_Settings.push_back(m_BottomOverscan);

	m_LeftOverscan = new CLeftOverscanSetting(this, "Overscan at Left", 0, 0, 150, IniSection, pOverscanGroup);
	m_Settings.push_back(m_LeftOverscan);

	m_RightOverscan = new CRightOverscanSetting(this, "Overscan at Right", 0, 0, 150, IniSection, pOverscanGroup);
	m_Settings.push_back(m_RightOverscan);

	m_VideoInput = new CVideoInputSetting(this, "VideoInput", 0, 0, LONG_MAX, IniSection);
	m_Settings.push_back(m_VideoInput);

	m_AudioInput = new CAudioInputSetting(this, "AudioInput", 0, 0, LONG_MAX, IniSection);
	m_Settings.push_back(m_AudioInput);

	m_Resolution = new CResolutionSetting(this, "Resolution", -1, -1, LONG_MAX, IniSection, pVideoGroup);
	m_Settings.push_back(m_Resolution);
	
	m_ConnectAudio = new CConnectAudioSetting(this,"ConnectAudio",TRUE,IniSection);
	m_Settings.push_back(m_ConnectAudio);

	//restore m_VideoFmt from ini file
	int ResolutionDataIniSize=GetPrivateProfileInt(IniSection,"ResolutionSize",-1,GetIniFileForSettings());
	if(ResolutionDataIniSize>0)
	{
		char *pcData=new char[ResolutionDataIniSize+1];
		DWORD result=GetPrivateProfileString(IniSection,"ResolutionData","",pcData,ResolutionDataIniSize+1,GetIniFileForSettings());
		if(result<ResolutionDataIniSize)
		{
			LOG(2,"DSCaptureSource: Reading too litle data, problem with ResolutionSize or ResolutionData in ini file");
		}
		
		string str=pcData;
		vector<std::string> strlist;
		string::size_type LastPos=0;
		string::size_type pos;
		while(pos=str.find("&",LastPos),pos!=string::npos)
		{
			strlist.push_back(str.substr(LastPos,pos-LastPos));
			LastPos=pos+1;
		}
		if(LastPos<str.size())
		{
			strlist.push_back(str.substr(LastPos));
		}
		for(vector<std::string>::size_type i=0;i<strlist.size();i++)
		{
			CDShowGraph::CVideoFormat fmt;
			fmt=strlist[i];
			m_VideoFmt.push_back(fmt);
		}

		delete pcData;
	}
	
	ReadFromIni();
	LOG(2,"DSCaptureSource: setting read from .ini");
}

int CDSCaptureSource::ChangeRes(int nResIndex)
{
	int resu = 1;
	try
	{
		CAutoCriticalSection lock(m_hOutThreadSync);

		ASSERT(nResIndex>=0 && nResIndex<m_VideoFmt.size());
		CDShowGraph::eChangeRes_Error err=m_pDSGraph->ChangeRes(m_VideoFmt[nResIndex]);
		switch(err)
		{
		case CDShowGraph::eChangeRes_Error::ERROR_NO_GRAPH:
			ErrorBox("Can't change resolution because there is no filter graph (bug)");
			break;
		case CDShowGraph::eChangeRes_Error::ERROR_CHANGED_BACK:
			ErrorBox("The selected resolution is not valid or coud not be used");
			break;
		case CDShowGraph::eChangeRes_Error::ERROR_FAILED_TO_CHANGE_BACK:
			ErrorBox("Failed to change resolution and faild to change back to previous resolution");
			//shod probably call Stop() or Reset() here since the
			//filter graph is most likely broken now
			break;
		case CDShowGraph::eChangeRes_Error::SUCCESS:
			m_Resolution->SetValue(nResIndex);
			resu = 0;
			break;
		}
		NotifySquarePixelsCheck();
	}
	catch(CDShowException &e)
	{
		ErrorBox(CString("Error when changeing resolution\n\n")+e.getErrorText());
	}
	catch(exception &e2)
	{
		ErrorBox(CString("Stl exception:\n\n")+e2.what());
	}
	return resu;
}

BOOL CDSCaptureSource::HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
{
	if(CDSSourceBase::HandleWindowsCommands(hWnd,wParam,lParam)==TRUE)
	{
		return TRUE;
	}
	
	if(LOWORD(wParam)==IDM_DSHOW_SETTINGS)
	{
		CTreeSettingsDlg dlg(CString("DirectShow Settings"));

		bool bConnectAudio=(m_ConnectAudio->GetValue()!=0);
		CDSAudioDevicePage AudioDevice(CString("Audio output"),m_AudioDevice,&bConnectAudio);
		CDSVideoFormatPage VidemFmt(CString("Resolution"),m_VideoFmt,m_Resolution);

		dlg.AddPage(&AudioDevice);
		dlg.AddPage(&VidemFmt);
		dlg.DoModal();
		
		m_ConnectAudio->SetValue(bConnectAudio);

		return TRUE;
	}

	if(m_pDSGraph==NULL)
	{
		return FALSE;
	}

	CDShowCaptureDevice *pCap=NULL;
	CDShowTVAudio *pTVAudio=NULL;
	if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
	{
		pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
		pTVAudio=pCap->GetTVAudio();
	}

	if(LOWORD(wParam)>=IDM_CROSSBAR_INPUT0 && LOWORD(wParam)<=IDM_CROSSBAR_INPUT_MAX)
	{
		try
		{
			if(pCap!=NULL)
			{
				CDShowBaseCrossbar *pCrossbar=pCap->getCrossbar();
				if(pCrossbar!=NULL)
				{
					PhysicalConnectorType type=pCrossbar->GetInputType(LOWORD(wParam)-IDM_CROSSBAR_INPUT0);
					if(type<0x1000)
					{
						m_VideoInput->SetValue(LOWORD(wParam)-IDM_CROSSBAR_INPUT0);
					}
					else
					{
						m_AudioInput->SetValue(LOWORD(wParam)-IDM_CROSSBAR_INPUT0);
					}
				}
			}
		}
		catch(CDShowException &e)
		{
			ErrorBox(CString("Failed to change input\n\n")+e.getErrorText());
		}

		return TRUE;
	}
	else if(LOWORD(wParam)>=IDM_DSVIDEO_STANDARD_0 && LOWORD(wParam)<=IDM_DSVIDEO_STANDARD_MAX)
	{
		try
		{
			if(pCap!=NULL)
			{
				pCap->PutTVFormat(videoStandards[LOWORD(wParam)-IDM_DSVIDEO_STANDARD_0].format);
			}
		}
		catch(CDShowException &e)
		{
			ErrorBox(CString("Failed to change video format\n\n")+e.getErrorText());
		}
		return TRUE;
	}
	else if(LOWORD(wParam)>=IDM_DSHOW_RES_0 && LOWORD(wParam<=IDM_DSHOW_RES_MAX))
	{
		TGUIRequest req;
		req.type = REQ_DSHOW_CHANGERES;
		req.param1 = LOWORD(wParam)-IDM_DSHOW_RES_0;
		ASSERT(req.param1>=0 && req.param1<m_VideoFmt.size());
		PutRequest(&req);
	}
	else if(LOWORD(wParam)==ID_DSHOW_AUDIOCHANNEL_MONO)
	{
		if(pTVAudio!=NULL)
		{
			try
			{
				pTVAudio->SetMode(AMTVAUDIO_MODE_MONO);
			}
			catch(CDShowException &e)
			{
				ErrorBox(CString("Error changing audio channel\n\n")+e.getErrorText());
			}
		}
	}
	else if(LOWORD(wParam)==ID_DSHOW_AUDIOCHANNEL_STEREO)
	{
		if(pTVAudio!=NULL)
		{
			try
			{
				pTVAudio->SetMode(AMTVAUDIO_MODE_STEREO);
			}
			catch(CDShowException &e)
			{
				ErrorBox(CString("Error changing audio channel\n\n")+e.getErrorText());
			}
		}
	}
	else if(LOWORD(wParam)==ID_DSHOW_AUDIOCHANNEL_LANGUAGEA)
	{
		if(pTVAudio!=NULL)
		{
			try
			{
				pTVAudio->SetMode(AMTVAUDIO_MODE_LANG_A);
			}
			catch(CDShowException &e)
			{
				ErrorBox(CString("Error changing audio channel\n\n")+e.getErrorText());
			}
		}
	}
	else if(LOWORD(wParam)==ID_DSHOW_AUDIOCHANNEL_LANGUAGEB)
	{
		if(pTVAudio!=NULL)
		{
			try
			{
				pTVAudio->SetMode(AMTVAUDIO_MODE_LANG_B);
			}
			catch(CDShowException &e)
			{
				ErrorBox(CString("Error changing audio channel\n\n")+e.getErrorText());
			}
		}
	}
	else if(LOWORD(wParam)==ID_DSHOW_AUDIOCHANNEL_LANGUAGEB)
	{
		if(pTVAudio!=NULL)
		{
			try
			{
				pTVAudio->SetMode(AMTVAUDIO_MODE_LANG_C);
			}
			catch(CDShowException &e)
			{
				ErrorBox(CString("Error changing audio channel\n\n")+e.getErrorText());
			}
		}
	}

	return FALSE;
}

eVideoFormat CDSCaptureSource::GetFormat()
{
	if(m_pDSGraph==NULL)
	{
		LOG(1,"CDSCaptureSource::GetFormat called when there is no filter graph");
		return VIDEOFORMAT_PAL_B;
	}

	CDShowBaseSource *pSrc=m_pDSGraph->getSourceDevice();
	if(pSrc==NULL)
	{
		return VIDEOFORMAT_PAL_B;
	}

	CDShowCaptureDevice *pCap=NULL;
	if(pSrc->getObjectType()!=DSHOW_TYPE_SOURCE_CAPTURE)
	{
		return VIDEOFORMAT_PAL_B;
	}
	pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();

	AnalogVideoStandard VideoStd=AnalogVideo_None;
	try
	{
		VideoStd=pCap->GetTVFormat();
	}
	catch(CDShowException &e)
	{
		LOG(1,"Exception in CDSCaptureSource::GetFormat - %s",(LPCTSTR)e.getErrorText());
	}

	return ConvertVideoStd(VideoStd);
}

BOOL CDSCaptureSource::IsInTunerMode()
{
	if(m_pDSGraph==NULL)
	{
		return FALSE;
	}
	CDShowCaptureDevice *pCap=NULL;
	if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
	{
		pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
	}

	if(pCap!=NULL)
	{
		CDShowBaseCrossbar *pCrossbar=pCap->getCrossbar();
		if(pCrossbar!=NULL)
		{
			try
			{
				long cIn,cOut;
				pCrossbar->GetPinCounts(cIn,cOut);
				for(long i=0;i<cIn;i++)
				{
					if (pCrossbar->IsInputSelected(i))
					{
						if (pCrossbar->GetInputType(i) == PhysConn_Video_Tuner)
						{
							LOG(3,"DSSource: IsInTunerMode.");
							return TRUE;
						}
					}
				}
			}
			catch(CDShowException &e)
			{
				LOG(1,"Exception in CDSCaptureSource::IsInTunerMode - %s",(LPCTSTR)e.getErrorText());
			}
			LOG(3,"DSSource: IsInTunerMode: No");
		}
	}
	return FALSE;
}

ITuner* CDSCaptureSource::GetTuner()
{
	return &m_Tuner;
}

AnalogVideoStandard CDSCaptureSource::ConvertVideoStd(eVideoFormat fmt)
{
	switch(fmt)
	{
	case VIDEOFORMAT_PAL_B:
		return AnalogVideo_PAL_B;
    case VIDEOFORMAT_PAL_D:
		return AnalogVideo_PAL_D;
    case VIDEOFORMAT_PAL_G:
		return AnalogVideo_PAL_G;
    case VIDEOFORMAT_PAL_H:
		return AnalogVideo_PAL_H;
    case VIDEOFORMAT_PAL_I:
		return AnalogVideo_PAL_I;
    case VIDEOFORMAT_PAL_M:
		return AnalogVideo_PAL_M;
    case VIDEOFORMAT_PAL_N:
		return AnalogVideo_PAL_N;
    case VIDEOFORMAT_PAL_60:
		return AnalogVideo_PAL_60;
    case VIDEOFORMAT_PAL_N_COMBO:
		return AnalogVideo_PAL_N_COMBO;
    case VIDEOFORMAT_SECAM_B:
		return AnalogVideo_SECAM_B;
    case VIDEOFORMAT_SECAM_D:
		return AnalogVideo_SECAM_D;
    case VIDEOFORMAT_SECAM_G:
		return AnalogVideo_SECAM_G;
    case VIDEOFORMAT_SECAM_H:
		return AnalogVideo_SECAM_H;
    case VIDEOFORMAT_SECAM_K:
		return AnalogVideo_SECAM_K;
    case VIDEOFORMAT_SECAM_K1:
		return AnalogVideo_SECAM_K1;
    case VIDEOFORMAT_SECAM_L:
		return AnalogVideo_SECAM_L;
    case VIDEOFORMAT_SECAM_L1:
		return AnalogVideo_SECAM_L1;
    case VIDEOFORMAT_NTSC_M:
		return AnalogVideo_NTSC_M;
    case VIDEOFORMAT_NTSC_M_Japan:
		return AnalogVideo_NTSC_M_J;
    case VIDEOFORMAT_NTSC_50:
		return AnalogVideo_NTSC_433;
	default:
		LOG(1,"CDSCaptureSource::ConvertVideoFmt: Unknown videoformat!");
		return AnalogVideo_None;
	}
}

eVideoFormat CDSCaptureSource::ConvertVideoStd(AnalogVideoStandard fmt)
{
	switch(fmt)
	{
	case AnalogVideo_PAL_B:
		return VIDEOFORMAT_PAL_B;
    case AnalogVideo_PAL_D:
		return VIDEOFORMAT_PAL_D;
    case AnalogVideo_PAL_G:
		return VIDEOFORMAT_PAL_G;
    case AnalogVideo_PAL_H:
		return VIDEOFORMAT_PAL_H;
    case AnalogVideo_PAL_I:
		return VIDEOFORMAT_PAL_I;
    case AnalogVideo_PAL_M:
		return VIDEOFORMAT_PAL_M;
    case AnalogVideo_PAL_N:
		return VIDEOFORMAT_PAL_N;
    case AnalogVideo_PAL_60:
		return VIDEOFORMAT_PAL_60;
    case AnalogVideo_PAL_N_COMBO:
		return VIDEOFORMAT_PAL_N_COMBO;
    case AnalogVideo_SECAM_B:
		return VIDEOFORMAT_SECAM_B;
    case AnalogVideo_SECAM_D:
		return VIDEOFORMAT_SECAM_D;
    case AnalogVideo_SECAM_G:
		return VIDEOFORMAT_SECAM_G;
    case AnalogVideo_SECAM_H:
		return VIDEOFORMAT_SECAM_H;
    case AnalogVideo_SECAM_K:
		return VIDEOFORMAT_SECAM_K;
    case AnalogVideo_SECAM_K1:
		return VIDEOFORMAT_SECAM_K1;
    case AnalogVideo_SECAM_L:
		return VIDEOFORMAT_SECAM_L;
    case AnalogVideo_SECAM_L1:
		return VIDEOFORMAT_SECAM_L1;
    case AnalogVideo_NTSC_M:
		return VIDEOFORMAT_NTSC_M;
    case AnalogVideo_NTSC_M_J:
		return VIDEOFORMAT_NTSC_M_Japan;
    case AnalogVideo_NTSC_433:
		return VIDEOFORMAT_NTSC_50;
	default:
		LOG(1,"CDSCaptureSource::ConvertVideoFmt: Unknown videoformat!");
		return VIDEOFORMAT_PAL_B;
	}	
}

BOOL CDSCaptureSource::SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat)
{
	if(m_pDSGraph==NULL)
	{
		return FALSE;
	}

	CDShowBaseSource *pSrc=m_pDSGraph->getSourceDevice();
	if(pSrc==NULL)
	{
		return FALSE;
	}

	CDShowCaptureDevice *pCap=NULL;
	if(pSrc->getObjectType()!=DSHOW_TYPE_SOURCE_CAPTURE)
	{
		return FALSE;
	}

	pCap=(CDShowCaptureDevice*)pSrc;
	CDShowDirectTuner *pTuner = pCap->GetTuner();
	if(pTuner==NULL)
	{
		return FALSE;
	}
	try
	{
		//fm radio
		if(VideoFormat==VIDEOFORMAT_LASTONE+1)
		{
			if(pTuner->GetAvailableModes()&AMTUNER_MODE_FM_RADIO)
			{
				pTuner->SetFrequency(FrequencyId,AMTUNER_MODE_FM_RADIO,AnalogVideo_None);
				return TRUE;
			}
			else
			{
				LOG(1,"CDSCaptureSource::SetTunerFrequency: Tuner does not support fm radio");
				return FALSE;
			}
		}
		else
		{
			if(pTuner->GetAvailableModes()&AMTUNER_MODE_TV)
			{
				AnalogVideoStandard format=ConvertVideoStd(VideoFormat);
				if(format&pCap->GetSupportedTVFormats())
				{
					pCap->PutTVFormat(format);
				}
				else
				{
					LOG(1,"CDSCaptureSource::SetTunerFrequency: Specified video format is not supported!!!");
				}
				pTuner->SetFrequency(FrequencyId,AMTUNER_MODE_TV,format);

				return TRUE;
			}
			else
			{
				LOG(1,"CDSCaptureSource::SetTunerFrequency: Tuner is not a tvtuner");
				return FALSE;
			}
		}
	}
	catch(CDShowException e)
	{
		LOG(1, "CDSCaptureSource::SetTunerFrequency: DShow Exception - %s", (LPCSTR)e.getErrorText());
		return FALSE;
	}
}

BOOL CDSCaptureSource::IsVideoPresent()
{
	if(m_pDSGraph==NULL)
	{
		return FALSE;
	}

	CDShowBaseSource *pSrc=m_pDSGraph->getSourceDevice();
	if(pSrc==NULL)
	{
		return FALSE;
	}

	CDShowCaptureDevice *pCap=NULL;
	if(pSrc->getObjectType()!=DSHOW_TYPE_SOURCE_CAPTURE)
	{
		return FALSE;
	}

	pCap=(CDShowCaptureDevice*)pSrc;
	/*CDShowDirectTuner *pTuner = pCap->GetTuner();
	if(pTuner==NULL)
	{
		return FALSE;
	}*/
	try
	{
		//this is comented out for now since it looks like it doesn't work
		//hopefully IsHorizontalLocked will work properly most of the time
		/*CDShowDirectTuner::eSignalType type;

		long signal=pTuner->GetSignalStrength(type);
		switch(type)
		{
		case CDShowDirectTuner::eSignalType::SIGNALTYPE_SIGNALSTRENGTH:
			LOG(2,"CDSCaptureSource::IsVideoPresent: Using SignalStrength to determine signal present");
			if(signal>0)
			{
				return TRUE;
			}
			break;
		case CDShowDirectTuner::eSignalType::SIGNALTYPE_PLL:
			LOG(2,"CDSCaptureSource::IsVideoPresent: Using PLL to determine signal present");
			if(signal==0)
			{
				return TRUE;
			}
			break;
		case CDShowDirectTuner::eSignalType::SIGNALTYPE_NONE:
			LOG(2,"CDSCaptureSource::IsVideoPresent: Using IsHorizontalLocked() to determine signal present");
			return pCap->IsHorizontalLocked();
			break;
		}*/
		return pCap->IsHorizontalLocked();
	}
	catch(CDShowException e)
	{
		LOG(1, "CDSCaptureSource::IsVideoPresent: DShow Exception - %s", (LPCSTR)e.getErrorText());
	}

	return FALSE;
}
eTunerId CDSCaptureSource::CDummyTuner::GetTunerId()
{
	//there is no value that fits dshow
	//any value besides TUNER_ABSENT will work
	return TUNER_USER_SETUP;
}
eVideoFormat CDSCaptureSource::CDummyTuner::GetDefaultVideoFormat()
{
	return VIDEOFORMAT_PAL_B;
}
bool CDSCaptureSource::CDummyTuner::HasRadio() const
{
	return false;
}
bool CDSCaptureSource::CDummyTuner::SetRadioFrequency(long nFrequency)
{
	return false;
}
bool CDSCaptureSource::CDummyTuner::SetTVFrequency(long nFrequency, eVideoFormat videoFormat)
{
	return false;
}
long CDSCaptureSource::CDummyTuner::GetFrequency()
{
	return 0;
}
eTunerLocked CDSCaptureSource::CDummyTuner::IsLocked()
{
	return TUNER_LOCK_NOTSUPPORTED;
}
eTunerAFCStatus CDSCaptureSource::CDummyTuner::GetAFCStatus(long &nFreqDeviation)
{
	return TUNER_AFC_NOTSUPPORTED;
}

void CDSCaptureSource::SetMenu(HMENU hMenu)
{
	if(m_pDSGraph==NULL)
	{
		return;
	}

	CMenu topMenu;
	topMenu.Attach(m_hMenu);
	CMenu *menu=topMenu.GetSubMenu(0);

	CDShowCaptureDevice *pCap=NULL;
	CDShowBaseCrossbar *pCrossbar=NULL;

	CDShowBaseSource *pSrc=m_pDSGraph->getSourceDevice();
	if(pSrc!=NULL)
	{
		if(pSrc->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
		{
			pCap=(CDShowCaptureDevice*)pSrc;
			pCrossbar=pCap->getCrossbar();
		}
	}

	//setup input selection menus
	CString menuText;
	if(pCrossbar!=NULL)
	{
		CMenu vidSubMenu;
		CMenu audSubMenu;

		//create video input submenu and insert it
		vidSubMenu.CreateMenu();
		menu->GetMenuString(0,menuText,MF_BYPOSITION);
		menu->ModifyMenu(0,MF_POPUP|MF_BYPOSITION,(UINT) vidSubMenu.GetSafeHmenu(),menuText);
		menu->EnableMenuItem(0,MF_BYPOSITION|MF_ENABLED);

		//same for audio input menu
		audSubMenu.CreateMenu();
		menu->GetMenuString(3,menuText,MF_BYPOSITION);
		menu->ModifyMenu(3,MF_POPUP|MF_BYPOSITION,(UINT) audSubMenu.GetSafeHmenu(),menuText);
		menu->EnableMenuItem(3,MF_BYPOSITION|MF_ENABLED);

		long cIn,cOut;
		pCrossbar->GetPinCounts(cIn,cOut);
		for(int i=0;i<cIn;i++)
		{
			ASSERT((IDM_CROSSBAR_INPUT0+i)<=IDM_CROSSBAR_INPUT_MAX);
			bool bSelected=pCrossbar->IsInputSelected(i);

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
	else
	{
		menu->EnableMenuItem(0,MF_BYPOSITION|MF_GRAYED);
		menu->EnableMenuItem(3,MF_BYPOSITION|MF_GRAYED);
	}

	//video standards menu
	if(IsInTunerMode()==FALSE && pCap!=NULL && pCap->hasVideoDec())
	{
		CMenu formatMenu;

		long formats=0;
		long selectedFormat=0;
		try
		{
			formats=pCap->GetSupportedTVFormats();
			selectedFormat=pCap->GetTVFormat();
		}
		catch(CDShowException e)
		{
            LOG(1, "Exception in CDSCaptureSource::SetMenu - %s", (LPCSTR)e.getErrorText());
        }

		//make sure there is at least one format to be selected
		if(formats!=0)
		{
			formatMenu.CreateMenu();
			menu->GetMenuString(1,menuText,MF_BYPOSITION);
			menu->ModifyMenu(1,MF_POPUP|MF_BYPOSITION,(UINT)formatMenu.GetSafeHmenu(),menuText);
			menu->EnableMenuItem(1,MF_BYPOSITION|MF_ENABLED);

			int i=0;
			while(videoStandards[i].format!=0)
			{
				ASSERT(i <= IDM_DSVIDEO_STANDARD_MAX-IDM_DSVIDEO_STANDARD_0);

				if(formats & videoStandards[i].format)
				{
					UINT check=selectedFormat & videoStandards[i].format ? MF_CHECKED:MF_UNCHECKED;
					formatMenu.AppendMenu(MF_STRING,IDM_DSVIDEO_STANDARD_0+i,videoStandards[i].name);
					formatMenu.CheckMenuItem(IDM_DSVIDEO_STANDARD_0+i,check);
				}
				i++;
			}
		}
		else
		{
			menu->EnableMenuItem(1,MF_BYPOSITION|MF_GRAYED);
		}
	}
	else
	{
		menu->EnableMenuItem(1,MF_BYPOSITION|MF_GRAYED);
	}

	//resolution submenu
	bool ResAdded=false;
	CMenu ResSubMenu;
	ResSubMenu.CreateMenu();

	for(vector<CDShowGraph::CVideoFormat>::size_type index=0;index<m_VideoFmt.size();index++)
	{
		ASSERT(IDM_DSHOW_RES_0+index<=IDM_DSHOW_RES_MAX);
		ResSubMenu.AppendMenu(MF_STRING|(m_Resolution->GetValue()==index?MF_CHECKED:MF_UNCHECKED),IDM_DSHOW_RES_0+index,m_VideoFmt[index].m_Name.c_str());
		ResAdded=true;
	}

	if(ResAdded)
	{
		menu->GetMenuString(2,menuText,MF_BYPOSITION);
		menu->ModifyMenu(2,MF_POPUP|MF_BYPOSITION,(UINT) ResSubMenu.GetSafeHmenu(),menuText);
		menu->EnableMenuItem(2,MF_BYPOSITION|MF_ENABLED);
	}
	else
	{
		//no resolution was added, gray the menu item
		menu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);
		ResSubMenu.DestroyMenu();
	}

	//set a radio checkmark infront of the current play/pause/stop menu entry
	FILTER_STATE state=m_pDSGraph->getState();
	UINT pos=8-state;
	menu->CheckMenuRadioItem(6,8,pos,MF_BYPOSITION);
	
	//audio channel submenu
	CDShowTVAudio *pTVAudio=pCap->GetTVAudio();
	if(pTVAudio!=NULL && pTVAudio->GetAvailableModes()!=0)
	{
		menu->EnableMenuItem(4,MF_BYPOSITION);
		long modes=pTVAudio->GetAvailableModes();
		TVAudioMode mode=pTVAudio->GetMode();
		if(modes&AMTVAUDIO_MODE_MONO)
		{
			menu->EnableMenuItem(ID_DSHOW_AUDIOCHANNEL_MONO,MF_BYCOMMAND);
			menu->CheckMenuItem(ID_DSHOW_AUDIOCHANNEL_MONO,MF_BYCOMMAND| (mode&AMTVAUDIO_MODE_MONO ? MF_CHECKED : MF_UNCHECKED));
			
		}
		if(modes&AMTVAUDIO_MODE_STEREO)
		{
			menu->EnableMenuItem(ID_DSHOW_AUDIOCHANNEL_STEREO,MF_BYCOMMAND);
			menu->CheckMenuItem(ID_DSHOW_AUDIOCHANNEL_STEREO,MF_BYCOMMAND| (mode&AMTVAUDIO_MODE_STEREO ? MF_CHECKED : MF_UNCHECKED));
		}
		if(modes&AMTVAUDIO_MODE_LANG_A)
		{
			menu->EnableMenuItem(ID_DSHOW_AUDIOCHANNEL_LANGUAGEA,MF_BYCOMMAND);
			menu->CheckMenuItem(ID_DSHOW_AUDIOCHANNEL_LANGUAGEA,MF_BYCOMMAND| (mode&AMTVAUDIO_MODE_LANG_A ? MF_CHECKED : MF_UNCHECKED));
		}
		if(modes&AMTVAUDIO_MODE_LANG_B)
		{
			menu->EnableMenuItem(ID_DSHOW_AUDIOCHANNEL_LANGUAGEB,MF_BYCOMMAND);
			menu->CheckMenuItem(ID_DSHOW_AUDIOCHANNEL_LANGUAGEB,MF_BYCOMMAND| (mode&AMTVAUDIO_MODE_LANG_B ? MF_CHECKED : MF_UNCHECKED));
		}
		if(modes&AMTVAUDIO_MODE_LANG_C)
		{
			menu->EnableMenuItem(ID_DSHOW_AUDIOCHANNEL_LANGUAGEC,MF_BYCOMMAND);
			menu->CheckMenuItem(ID_DSHOW_AUDIOCHANNEL_LANGUAGEC,MF_BYCOMMAND| (mode&AMTVAUDIO_MODE_LANG_C ? MF_CHECKED : MF_UNCHECKED));
		}
	}
	else
	{
		menu->EnableMenuItem(4,MF_BYPOSITION|MF_GRAYED);
	}

	topMenu.Detach();
}

void CDSCaptureSource::HandleTimerMessages(int TimerId)
{

}

LPCSTR CDSCaptureSource::GetMenuLabel()
{
	return NULL;
}

ISetting* CDSCaptureSource::GetTopOverscan()
{
	return m_TopOverscan;
}

ISetting* CDSCaptureSource::GetBottomOverscan()
{
	return m_BottomOverscan;
}

ISetting* CDSCaptureSource::GetLeftOverscan()
{
	return m_LeftOverscan;
}

ISetting* CDSCaptureSource::GetRightOverscan()
{
	return m_RightOverscan;
}

void CDSCaptureSource::SetAspectRatioData()
{
	AspectSettings.InitialTopOverscan = m_TopOverscan->GetValue();
	AspectSettings.InitialBottomOverscan = m_BottomOverscan->GetValue();
	AspectSettings.InitialLeftOverscan = m_LeftOverscan->GetValue();
	AspectSettings.InitialRightOverscan = m_RightOverscan->GetValue();
    AspectSettings.bAnalogueBlanking = FALSE;
}

void CDSCaptureSource::TopOverscanOnChange(long Overscan, long OldValue)
{
	AspectSettings.InitialTopOverscan = Overscan;
    WorkoutOverlaySize(TRUE);
}

void CDSCaptureSource::BottomOverscanOnChange(long Overscan, long OldValue)
{
	AspectSettings.InitialBottomOverscan = Overscan;
    WorkoutOverlaySize(TRUE);
}

void CDSCaptureSource::LeftOverscanOnChange(long Overscan, long OldValue)
{
	AspectSettings.InitialLeftOverscan = Overscan;
    WorkoutOverlaySize(TRUE);
}

void CDSCaptureSource::RightOverscanOnChange(long Overscan, long OldValue)
{
	AspectSettings.InitialRightOverscan = Overscan;
    WorkoutOverlaySize(TRUE);
}

LPCSTR CDSCaptureSource::GetStatus()
{
	return m_DeviceName.c_str();
}

void CDSCaptureSource::VideoInputOnChange(long NewValue, long OldValue)
{
	if(m_pDSGraph==NULL)
	{
		return;
	}

	//this shoud always succeed
	CDShowCaptureDevice *pCap=NULL;
	if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
	{
		pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
	}
	ASSERT(pCap!=NULL);

	try
	{
		if(pCap!=NULL)
		{
			CDShowBaseCrossbar *pCrossbar=pCap->getCrossbar();
			if(pCrossbar!=NULL)
			{
				PhysicalConnectorType OldInputType = pCrossbar->GetInputType(OldValue);
                EventCollector->RaiseEvent(this, EVENT_VIDEOINPUT_PRECHANGE, OldValue, NewValue);

				LOG(2,"DSCaptureSource: Set video input to %d", NewValue);

				//set the related pin too since this is a video pin,maybe this shoud be user configurable?
				pCrossbar->SetInputIndex(NewValue,true);

				PhysicalConnectorType NewInputType = pCrossbar->GetInputType(NewValue);
				
				/**
				 * @todo we also must figure out what the related pin is and then call
				 * AudioInputOnChange if it is an audio pin.
				 */

                EventCollector->RaiseEvent(this, EVENT_VIDEOINPUT_CHANGE, OldValue, NewValue);

				if(NewInputType == PhysConn_Video_Tuner)
				{
                    // set up channel
                    // this must happen after the VideoInput change is sent
					if(pCap->GetTuner()!=NULL)
					{
						Channel_SetCurrent();
					}
				}
			}
		}
	}
	catch(CDShowException &e)
	{
		ErrorBox(CString("Failed to change video input\n\n")+e.getErrorText());
	}
}

void CDSCaptureSource::AudioInputOnChange(long NewValue, long OldValue)
{
	if(m_pDSGraph==NULL)
	{
		return;
	}

	//this shoud always succeed
	CDShowCaptureDevice *pCap=NULL;
	if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
	{
		pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
	}
	ASSERT(pCap!=NULL);

	try
	{
		if(pCap!=NULL)
		{
			CDShowBaseCrossbar *pCrossbar=pCap->getCrossbar();
			if(pCrossbar!=NULL)
			{
				LOG(2,"DSCaptureSource: Set audio input to %d",NewValue);
				
				//prevent changing to a pin that is not an audio pin
				PhysicalConnectorType type=pCrossbar->GetInputType(NewValue);
				if(type>=0x1000)
				{
					pCrossbar->SetInputIndex(NewValue,false);
				}
				else
				{
					LOG(2,"DSCaptureSource: New audio input pin is not a audio pin, will not change audio input");
				}
			}
		}
	}
	catch(CDShowException &e)
	{
		ErrorBox(CString("Failed to change audio input\n\n")+e.getErrorText());
	}
}

void CDSCaptureSource::Start()
{
	try
	{
		if(m_pDSGraph==NULL)
		{
			m_pDSGraph=new CDShowGraph(m_Device,m_DeviceName,m_AudioDevice,m_ConnectAudio->GetValue()!=0);
		}

		m_pDSGraph->ConnectGraph();

		if(m_VideoFmt.size()==0)
		{
			CreateDefaultVideoFmt();
		}

		if(m_Resolution->GetValue()>=0 && m_Resolution->GetValue()<m_VideoFmt.size())
		{
			CDShowGraph::eChangeRes_Error err=m_pDSGraph->ChangeRes(m_VideoFmt[m_Resolution->GetValue()]);
			switch(err)
			{
				case CDShowGraph::eChangeRes_Error::ERROR_NO_GRAPH:
					LOG(2,"Error restoring resolution, no filter graph (bug)");
					break;
				case CDShowGraph::eChangeRes_Error::ERROR_CHANGED_BACK:
					LOG(2,"Error restoring resolution, failed to change to new resolution or format not supported");
					break;
				case CDShowGraph::eChangeRes_Error::ERROR_FAILED_TO_CHANGE_BACK:
					//oops we broke the filter graph, reset resolution 
					//setting so we don't break it the next time too.
					m_Resolution->SetValue(-1);
					LOG(2,"Error restoring resolution, broke filter graph");
					break;
				case CDShowGraph::eChangeRes_Error::SUCCESS:
					break;
			}
		}

		// Sets min/max/default values
		GetHue();
		GetBrightness();
		GetContrast();
		GetSaturation();
		GetTopOverscan();
		GetBottomOverscan();
		GetLeftOverscan();
		GetRightOverscan();

		VideoInputOnChange(m_VideoInput->GetValue(), m_VideoInput->GetValue());
		AudioInputOnChange(m_AudioInput->GetValue(), m_AudioInput->GetValue());
		
		CDSSourceBase::Start();
	}
	catch(CDShowException &e)
	{
		ErrorBox(e.getErrorText());
	}
}

void CDSCaptureSource::Stop()
{
	CAutoCriticalSection lock(m_hOutThreadSync);

	CDSSourceBase::Stop();

	//need to remove the graph since we dont want to risk having both
	//dscalers own drivers and directshow drivers active at the same time
	if(m_pDSGraph!=NULL)
	{
		delete m_pDSGraph;
		m_pDSGraph=NULL;
	}
}

void CDSCaptureSource::CreateDefaultVideoFmt()
{
	struct ResolutionType
	{
		long x;
		long y;
	}res[]=
	{
		768,576,
		720,576,
		720,480,
		720,288,
		720,240,
		704,576,
		640,480,
		640,360,
		640,288,
		640,240,
		480,360,
		352,288,
		352,240,
		320,240,
		240,180,
		240,176,
		176,144,
		160,120,
		120,96,
		88,72
	};

	m_VideoFmt.erase(m_VideoFmt.begin(),m_VideoFmt.end());
	for(int i=0;i<sizeof(res)/sizeof(ResolutionType);i++)
	{
		CDShowGraph::CVideoFormat fmt;
		fmt.m_Width=res[i].x;
		fmt.m_Height=res[i].y;
		CString name;
		name.Format("%ldx%ld",res[i].x,res[i].y);
		fmt.m_Name=name;

		if(m_pDSGraph->IsValidRes(fmt))
		{
			m_VideoFmt.push_back(fmt);
		}
	}
}

void CDSCaptureSource::ResolutionOnChange(long NewValue, long OldValue)
{
    //
}

void CDSCaptureSource::ConnectAudioOnChange(long NewValue, long OldValue)
{
    //
}

int CDSCaptureSource::NumInputs(eSourceInputType InputType)
{
	if(m_pDSGraph==NULL)
	{
		return 0;
	}
	//this shoud always succeed
	CDShowCaptureDevice *pCap=NULL;
	if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
	{
		pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
	}
	if(pCap == NULL) { return 0; }
	
	if(!m_HaveInputList)
	{
		m_VideoInputList.clear();
		m_AudioInputList.clear();
		try
		{
			CDShowBaseCrossbar *pCrossbar=pCap->getCrossbar();
			if(pCrossbar==NULL)
			{
				return 0;
			}
			
			long cIn,cOut;
			pCrossbar->GetPinCounts(cIn,cOut);
			for(int i=0;i<cIn;i++)
			{
				//is it an audio or video input?
				if(pCrossbar->GetInputType(i)<4096)
				{
					m_VideoInputList.push_back(i);
				}
				else
				{
					m_AudioInputList.push_back(i);
				}
			}
			m_HaveInputList = TRUE;
		}
		catch(CDShowException &e)
		{
			LOG(1,"DSCaptureSource: NumInputs: Error: %s",e.getErrorText());
			return 0;
		}
	}
	
	if(InputType == VIDEOINPUT)
	{
		return m_VideoInputList.size();
	}
	else
	{
		return m_AudioInputList.size();
	}
	return 0;
}

BOOL CDSCaptureSource::SetInput(eSourceInputType InputType, int Nr)
{
	if(!m_HaveInputList)
	{
		NumInputs(InputType);   // Make input list
	}
	if(InputType == VIDEOINPUT)
	{
		if((Nr>=0) && (Nr < m_VideoInputList.size()))
		{
			m_VideoInput->SetValue(m_VideoInputList[Nr]);
			/// \todo Should check if this failed
			return TRUE;
		}
		return FALSE;
	}
	else if(InputType == AUDIOINPUT)
	{
		if((Nr>=0) && (Nr < m_AudioInputList.size()))
		{
			m_AudioInput->SetValue(m_AudioInputList[Nr]);
			/// \todo Should check if this failed
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

int CDSCaptureSource::GetInput(eSourceInputType InputType)
{
	if(!m_HaveInputList)
	{
		NumInputs(InputType);   // Make input list
	}
	
	if(InputType == VIDEOINPUT)
	{
		int i;
		for(i = 0; i < m_VideoInputList.size(); i++)
		{
			if(m_VideoInputList[i] == m_VideoInput->GetValue())
			{
				return i;
			}
		}
	}
	else if(InputType == AUDIOINPUT)
	{
		int i;
		for(i = 0; i < m_AudioInputList.size(); i++)
		{
			if(m_AudioInputList[i] == m_AudioInput->GetValue())
			{
				return i;
			}
		}
	}
	return -1;
}

const char* CDSCaptureSource::GetInputName(eSourceInputType InputType, int Nr)
{
	if(m_pDSGraph==NULL)
	{
		return NULL;
	}
	//this shoud always succeed
	CDShowCaptureDevice *pCap=NULL;
	if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
	{
		pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
	}
	if (pCap == NULL)
	{
		return NULL;
	}
	
	if(!m_HaveInputList)
	{
		// Make input list
		NumInputs(InputType);
	}
	char *szName = NULL;
	if((InputType == VIDEOINPUT) || (InputType == AUDIOINPUT))
	{
		if((Nr>=0) && (((InputType == VIDEOINPUT) && (Nr < m_VideoInputList.size()))
			|| ((InputType == AUDIOINPUT) && (Nr < m_AudioInputList.size())))
			)
		{
			try
			{
				CDShowBaseCrossbar *pCrossbar=pCap->getCrossbar();
				if(pCrossbar==NULL)
				{
					return NULL;
				}
				
				int nInputNumber;
				if (InputType == VIDEOINPUT)
				{
					nInputNumber = m_VideoInputList[ Nr ];
				}
				else
				{
					nInputNumber = m_AudioInputList[ Nr ];
				}
				szName = pCrossbar->GetInputName(nInputNumber);
			}
			catch(CDShowException &e)
			{
				LOG(1,"DSCaptureSource: GetInputName: Error: %s",e.getErrorText());
			}
		}
	}
	return (const char*)szName;
}

BOOL CDSCaptureSource::InputHasTuner(eSourceInputType InputType, int Nr)
{
	if(m_pDSGraph==NULL)
	{
		return FALSE;
	}
	//this shoud always succeed
	CDShowCaptureDevice *pCap=NULL;
	if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
	{
		pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
	}
	if(pCap == NULL)
	{
		return FALSE;
	}
	
	
	if(InputType == VIDEOINPUT)
	{
		if(!m_HaveInputList)
		{
			NumInputs(InputType);
		}
		
		if((Nr>=0) && (Nr < m_VideoInputList.size()))
		{
			try
			{
				CDShowBaseCrossbar *pCrossbar=pCap->getCrossbar();
				if(pCrossbar==NULL)
				{
					return FALSE;
				}
				
				if(pCrossbar->GetInputType(m_VideoInputList[Nr]) == PhysConn_Video_Tuner)
				{
					return TRUE;
				}
			}
			catch(CDShowException &e)
			{
				LOG(1,"DSCaptureSource: InputHasTuner: Error: %s",e.getErrorText());
			}
		}
	}

	return FALSE;
}


#endif
