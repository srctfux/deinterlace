/////////////////////////////////////////////////////////////////////////////
// $Id: CX2388xSource_Audio.cpp,v 1.4 2004-02-27 20:51:00 to_see Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 John Adcock.  All rights reserved.
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
//
// This code is based on a version of dTV modified by Michael Eskin and
// others at Connexant.  Those parts are probably (c) Connexant 2002
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.3  2003/10/27 10:39:51  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.2  2002/12/10 14:53:16  adcockj
// Sound fixes for cx2388x
//
// Revision 1.1  2002/10/31 15:55:50  adcockj
// Moved audio code from Connexant dTV version
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file CX2388xSource.cpp CCX2388xSource Implementation (Audio)
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "CX2388xSource.h"
#include "CX2388x_Defines.h"
#include "DScaler.h"
#include "OutThreads.h"
#include "AspectRatio.h"
#include "DebugLog.h"
#include "SettingsPerChannel.h"
#include "status.h"

void CCX2388xSource::Mute()
{
    m_pCard->SetAudioMute();
	m_bMuted = TRUE;
}

void CCX2388xSource::UnMute()
{
    m_pCard->SetAudioUnMute(m_Volume->GetValue());
	m_bMuted = FALSE;
}

void CCX2388xSource::VolumeOnChange(long NewValue, long OldValue)
{
    m_pCard->SetAudioVolume(NewValue);    
	EventCollector->RaiseEvent(this, EVENT_VOLUME, OldValue, NewValue);
}

void CCX2388xSource::BalanceOnChange(long NewValue, long OldValue)
{
    m_pCard->SetAudioBalance(NewValue);
}

void CCX2388xSource::AudioStandardOnChange(long NewValue, long OldValue)
{
	StopUpdateAudioStatus();
    m_pCard->AudioInit(
                        m_VideoSource->GetValue(), 
                        (eVideoFormat)m_VideoFormat->GetValue(), 
                        (eCX2388xAudioStandard)NewValue,
                        (eCX2388xStereoType)m_StereoType->GetValue()
                      );
	
	StartUpdateAudioStatus();
}

void CCX2388xSource::StereoTypeOnChange(long NewValue, long OldValue)
{
	StopUpdateAudioStatus();
	m_pCard->AudioInit(
                        m_VideoSource->GetValue(), 
                        (eVideoFormat)m_VideoFormat->GetValue(), 
                        (eCX2388xAudioStandard)m_AudioStandard->GetValue(),
                        (eCX2388xStereoType)NewValue
                      );
    
	StartUpdateAudioStatus();
}

void CCX2388xSource::StartUpdateAudioStatus()
{
	m_AutoDetectA2StereoCounter		= 0;
	m_AutoDetectA2BilingualCounter	= 0;
	SetTimer(hWnd, TIMER_CX2388X, TIMER_CX2388X_MS, NULL);
}

void CCX2388xSource::StopUpdateAudioStatus()
{
	KillTimer(hWnd, TIMER_CX2388X);
}

// called every 250ms
void CCX2388xSource::UpdateAudioStatus()
{
	eSoundChannel SoundChannel = SOUNDCHANNEL_MONO;

	if(IsInTunerMode())
	{
		if(IsVideoPresent())
		{
			if(m_bMuted)
			{
				UnMute();
				EventCollector->RaiseEvent(this, EVENT_MUTE, -1, m_bMuted);
			}
			
			switch(m_pCard->GetCurrentAudioStandard())
			{
			case AUDIO_STANDARD_A2:
				switch(m_pCard->GetCurrentStereoType())
				{
				case STEREOTYPE_AUTO:
					SoundChannel = AutoDetectA2Sound();
					break
						;
				case STEREOTYPE_MONO:
					SoundChannel = SOUNDCHANNEL_MONO;
					break;

				case STEREOTYPE_STEREO:
					SoundChannel = SOUNDCHANNEL_STEREO;
					break;

				case STEREOTYPE_ALT1:
					SoundChannel = SOUNDCHANNEL_LANGUAGE1;
					break;

				case STEREOTYPE_ALT2:
					SoundChannel = SOUNDCHANNEL_LANGUAGE2;
					break;
				}
				
				break;

			case AUDIO_STANDARD_AUTO:
			case AUDIO_STANDARD_BTSC:
			case AUDIO_STANDARD_EIAJ:
			case AUDIO_STANDARD_BTSC_SAP:
			case AUDIO_STANDARD_NICAM:
			case AUDIO_STANDARD_FM:
				break;	// \todo: add more support
			}
		}

		// no Video present
		else
		{
			if(!m_bMuted)
			{
				Mute();
				EventCollector->RaiseEvent(this, EVENT_MUTE, -1, m_bMuted);
			}
			
			m_AutoDetectA2StereoCounter		= 0;
			m_AutoDetectA2BilingualCounter	= 0;
		}
	}

	// not in tuner mode, show always Stereo
	else
	{
		SoundChannel = SOUNDCHANNEL_STEREO;
	}
	
	char szSoundChannel[256] = "";

    switch(SoundChannel)
    {
    case SOUNDCHANNEL_MONO:
		strcpy(szSoundChannel,"Mono");
		break;

    case SOUNDCHANNEL_STEREO:
		strcpy(szSoundChannel,"Stereo");
		break;

    case SOUNDCHANNEL_LANGUAGE1:
		strcpy(szSoundChannel,"Language 1");
		break;

    case SOUNDCHANNEL_LANGUAGE2:
		strcpy(szSoundChannel,"Language 2");
		break;
    }

    StatusBar_ShowText(STATUS_AUDIO, szSoundChannel);
    EventCollector->RaiseEvent(this, EVENT_SOUNDCHANNEL, -1, SoundChannel);
}

eSoundChannel CCX2388xSource::AutoDetectA2Sound()
{
	eSoundChannel SoundChannelA2 = SOUNDCHANNEL_MONO;
	DWORD dwVal = m_pCard->GetAudioStatusRegister();

	switch(dwVal & 0x03)
	{
	case 0: // Stereo detected
		if(m_AutoDetectA2StereoCounter < 10)
		{
			m_AutoDetectA2StereoCounter++;
		}
		break;

	case 2:  // Mono detected
		if(m_AutoDetectA2StereoCounter > 0)
		{
			m_AutoDetectA2StereoCounter--;
		}
		break;

	// \todo bilingual support
	}

	if(m_AutoDetectA2StereoCounter > 5)
	{
		m_pCard->SetAutoA2StereoToStereo();
		SoundChannelA2 = SOUNDCHANNEL_STEREO;
	}

	else
	{
		m_pCard->SetAutoA2StereoToMono();
		SoundChannelA2 = SOUNDCHANNEL_MONO;
	}

	return SoundChannelA2;
}

