//
// $Id: AudioDecoder.cpp,v 1.4 2002-01-23 22:52:01 robmuller Exp $
//
/////////////////////////////////////////////////////////////////////////////
//
// copyleft 2001 itt@myself.com
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: not supported by cvs2svn $
// Revision 1.3  2002/01/23 12:20:32  robmuller
// Added member function HandleTimerMessages(int TimerId).
//
// Revision 1.2  2001/12/19 19:24:44  ittarnavsky
// prepended SOUNDCHANNEL_ to all members of the eSoundChannel enum
//
// Revision 1.1  2001/12/05 21:45:10  ittarnavsky
// added changes for the AudioDecoder and AudioControls support
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AudioDecoder.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAudioDecoder::CAudioDecoder()
{
    m_VideoFormat = VIDEOFORMAT_NTSC_M;
    m_SoundChannel = SOUNDCHANNEL_MONO;
    m_AudioInput = AUDIOINPUT_TUNER;
}

CAudioDecoder::~CAudioDecoder()
{

}

eVideoFormat CAudioDecoder::GetVideoFormat()
{
    return m_VideoFormat;
}

void CAudioDecoder::SetVideoFormat(eVideoFormat videoFormat)
{
    m_VideoFormat = videoFormat;
}

eSoundChannel CAudioDecoder::GetSoundChannel()
{
    return m_SoundChannel;
}

void CAudioDecoder::SetSoundChannel(eSoundChannel soundChannel)
{
    m_SoundChannel = soundChannel;
}

eSoundChannel CAudioDecoder::IsAudioChannelDetected(eSoundChannel desiredAudioChannel)
{
    return desiredAudioChannel;
}

eAudioInput CAudioDecoder::GetAudioInput()
{
    return m_AudioInput;
}

void CAudioDecoder::SetAudioInput(eAudioInput audioInput)
{
    m_AudioInput = audioInput;
}

void CAudioDecoder::HandleTimerMessages(int TimerId)
{

}

LPCSTR CAudioDecoder::GetAudioName()
{
    return "Unknown";
}
