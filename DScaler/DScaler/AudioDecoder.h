//
// $Id: AudioDecoder.h,v 1.1 2001-12-05 21:45:10 ittarnavsky Exp $
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
/////////////////////////////////////////////////////////////////////////////

#if !defined(__AUDIODECODER_H__)
#define __AUDIODECODER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Bt848_Defines.h"
#include "TVFormats.h"

enum eAudioInput
{
    AUDIOINPUT_TUNER = 0,
    AUDIOINPUT_RADIO,
    AUDIOINPUT_EXTERNAL,
    AUDIOINPUT_INTERNAL,
    /// XXX: do we need this?
    AUDIOINPUT_MUTE,
    /// XXX: do we need this?
    AUDIOINPUT_STEREO 
};

class CAudioDecoder
{
public:
    CAudioDecoder();
    virtual ~CAudioDecoder();
    virtual void SetVideoFormat(eVideoFormat videoFormat);
    virtual eVideoFormat GetVideoFormat();
    virtual void SetSoundChannel(eSoundChannel soundChannel);
    virtual eSoundChannel GetSoundChannel();
    virtual eSoundChannel IsAudioChannelDetected(eSoundChannel desiredAudioChannel);
    virtual void SetAudioInput(eAudioInput audioInput);
    virtual eAudioInput GetAudioInput();
protected:
    eSoundChannel m_SoundChannel;
    eVideoFormat m_VideoFormat;
    eAudioInput m_AudioInput;
};

#endif // !defined(__AUDIODECODER_H__)
