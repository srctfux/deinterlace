/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Card_Audio.cpp,v 1.8 2002-10-08 12:24:46 atnak Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Atsushi Nakagawa.  All rights reserved.
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
// This software was based on v4l2 device driver for philips
// saa7134 based TV cards.  Those portions are
// Copyright (c) 2001,02 Gerd Knorr <kraxel@bytesex.org> [SuSE Labs]
//
// This software was based on BT848Card_Audio.cpp.  Those portions are
// Copyright (c) 2001 John Adcock.
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 09 Sep 2002   Atsushi Nakagawa      Initial Release
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.7  2002/10/04 23:40:46  atnak
// proper support for audio channels mono,stereo,lang1,lang2 added
//
// Revision 1.6  2002/10/03 23:36:23  atnak
// Various changes (major): VideoStandard, AudioStandard, CSAA7134Common, cleanups, tweaks etc,
//
// Revision 1.5  2002/09/16 17:51:58  atnak
// Added controls for L/R/Nicam audio volume
//
// Revision 1.4  2002/09/14 19:40:48  atnak
// various changes
//
// Revision 1.3  2002/09/10 12:14:35  atnak
// Some changes to eAudioStandard stuff
//
//
//////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "SAA7134Card.h"
#include "SAA7134_Defines.h"

#include "DebugLog.h"



void CSAA7134Card::InitAudio()
{
    m_AudioSampleRate = AUDIOSAMPLERATE_48000Hz;
    
    // mute all
    WriteByte(SAA7134_AUDIO_MUTE_CTRL, 0xFF);

    WriteByte(SAA7134_MONITOR_SELECT,           0x00);
    WriteByte(SAA7134_NICAM_ERROR_LOW,          0x00);
    WriteByte(SAA7134_NICAM_ERROR_HIGH,         0x00);

    // auto gain control enabled
    WriteByte(SAA7134_AGC_GAIN_SELECT,          0x00);
    
    WriteByte(SAA7134_SIF_SAMPLE_FREQ,          0x00);

    WriteByte(SAA7134_DEMODULATOR,              0x00);
    WriteByte(SAA7134_DCXO_IDENT_CTRL,          0x00);
    WriteByte(SAA7134_FM_DEEMPHASIS,            0x22);
    WriteByte(SAA7134_STEREO_DAC_OUTPUT_SELECT, 0xA0);

    SetAudioFMDematrix(AUDIOFMDEMATRIX_AUTOSWITCHING);

    WriteByte(SAA7134_DSP_OUTPUT_SELECT,        0x80);

    OrDataByte(SAA7134_ANALOG_IO_SELECT, SAA7134_ANALOG_IO_SELECT_VSEL1);
    OrDataByte(SAA7134_ANALOG_IO_SELECT, SAA7134_ANALOG_IO_SELECT_VSEL2);

    // no audio capture through DMA
    MaskDataDword(SAA7134_NUM_SAMPLES, 0x00, SAA7134_NUM_SAMPLES_MASK);
    WriteByte(SAA7134_AUDIO_FORMAT_CTRL, 0xDD);

    SetAudioSource(AUDIOINPUTSOURCE_LINE1);

    // normal output gain
    MaskDataByte(SAA7134_CHANNEL1_LEVEL, 0x00, 0x1F);
    MaskDataByte(SAA7134_CHANNEL2_LEVEL, 0x00, 0x1F);

    // no I2S output
    WriteByte(SAA7134_I2S_OUTPUT_FORMAT, 0x00);
    WriteByte(SAA7134_I2S_OUTPUT_SELECT, 0x00);
    WriteByte(SAA7134_I2S_OUTPUT_LEVEL, 0x00);

    SetAudioLockToVideo(FALSE);
}


void CSAA7134Card::SetAudioStandard(eAudioStandard AudioStandard)
{
    BYTE IdentCtrl      = 0;
    BYTE Demodulator    = 0;
    BYTE AudioPLLCtrl   = 0;

    DWORD Carrier1 = m_AudioStandards[AudioStandard].Carrier1;
    DWORD Carrier2 = m_AudioStandards[AudioStandard].Carrier2;

    switch (m_AudioStandards[AudioStandard].FIRType)
    {
    case FIR_BG_DK_DUAL_FM:
        // nothing specifc to do
        break;

    case FIR_SAT_DUAL_FM:
        // nothing specifc to do
        break;

    case FIR_BG_DK_NICAM:
    case FIR_I_NICAM:
    case FIR_L_NICAM:
        AudioPLLCtrl = SAA7134_AUDIO_PLL_CTRL_SWLOOP;
        break;

    case FIR_M_DUAL_FM:
        IdentCtrl = (0xFF & SAA7134_DCXO_IDENT_CTRL_IDAREA);
        break;
    }

    if (m_AudioStandards[AudioStandard].Carrier1Mode == AUDIOCHANNELMODE_AM)
    {
        Demodulator = (0xFF & SAA7134_DEMODULATOR_CH1MODE);
    }

    switch (m_AudioStandards[AudioStandard].Carrier2Mode)
    {
    case AUDIOCHANNELMODE_FM:
        Demodulator |= 0x00;
        break;

    case AUDIOCHANNELMODE_AM:
        Demodulator |= 0x01;
        break;

    case AUDIOCHANNELMODE_NICAM:
        Demodulator |= 0x10;
        break;
    }

    WriteDword(SAA7134_CARRIER1_FREQ, Carrier1);
    WriteDword(SAA7134_CARRIER2_FREQ, Carrier2);

    SetCh1FMDeemphasis(m_AudioStandards[AudioStandard].Ch1FMDeemphasis);
    SetCh2FMDeemphasis(m_AudioStandards[AudioStandard].Ch2FMDeemphasis);

    MaskDataByte(SAA7134_DCXO_IDENT_CTRL, IdentCtrl,
        SAA7134_DCXO_IDENT_CTRL_IDAREA);

    MaskDataByte(SAA7134_DEMODULATOR, Demodulator,
        SAA7134_DEMODULATOR_CH2MOD0 |
        SAA7134_DEMODULATOR_CH1MODE |
        SAA7134_DEMODULATOR_CH2MOD1);

    MaskDataByte(SAA7134_AUDIO_PLL_CTRL, AudioPLLCtrl,
        SAA7134_AUDIO_PLL_CTRL_SWLOOP);

    m_AudioStandard = AudioStandard;
}


void CSAA7134Card::SetAudioCarrier1Freq(DWORD Carrier)
{
    WriteDword(SAA7134_CARRIER1_FREQ, Carrier);
}


void CSAA7134Card::SetAudioCarrier2Freq(DWORD Carrier)
{
    WriteDword(SAA7134_CARRIER2_FREQ, Carrier);
}


void CSAA7134Card::SetAudioCarrier1Mode(eAudioCarrierMode Mode)
{
    if (Mode == AUDIOCHANNELMODE_AM)
    {
        AndDataByte(SAA7134_DEMODULATOR, ~SAA7134_DEMODULATOR_CH1MODE);
    }
    else
    {
        OrDataByte(SAA7134_DEMODULATOR, SAA7134_DEMODULATOR_CH1MODE);
    }
}


void CSAA7134Card::SetAudioCarrier2Mode(eAudioCarrierMode Mode)
{
    BYTE Demodulator = 0x00;

    switch (Mode)
    {
    case AUDIOCHANNELMODE_FM:
        Demodulator = 0x00;
        break;

    case AUDIOCHANNELMODE_AM:
        Demodulator = 0x01;
        break;

    case AUDIOCHANNELMODE_NICAM:
        Demodulator = 0x10;
        break;
    }

    MaskDataByte(SAA7134_DEMODULATOR, Demodulator,
        SAA7134_DEMODULATOR_CH2MOD0 |
        SAA7134_DEMODULATOR_CH2MOD1);
}


void CSAA7134Card::SetAudioLockToVideo(BOOL bLockAudio)
{
    if (bLockAudio)
    {
        AndDataByte(SAA7134_AUDIO_PLL_CTRL, ~SAA7134_AUDIO_PLL_CTRL_APLL);
    }
    else
    {
        OrDataByte(SAA7134_AUDIO_PLL_CTRL, SAA7134_AUDIO_PLL_CTRL_APLL);
    }
}

void CSAA7134Card::UpdateAudioClocksPerField(eVideoStandard VideoStandard)
{
    DWORD AudioClock = 0x000000;
    DWORD AudioClocksPerField = 0x00000;

    if (m_AudioInputSource == AUDIOINPUTSOURCE_DAC ||
        m_AudioSampleRate != AUDIOSAMPLERATE_44100Hz)
    {
        switch (GetCardSetup()->AudioCrystal)
        {
        case AUDIOCRYSTAL_32110Hz:
            AudioClock = 0x187DE7;
            break;

        case AUDIOCRYSTAL_24576Hz:
            AudioClock = 0x200000;
            break;
        }

        if ((VideoStandard != VIDEOSTANDARD_AUTODETECT &&
            m_VideoStandards[VideoStandard].Is25fps) ||
            Is25fpsSignalDetected())
        {
            AudioClocksPerField = 0x1E000;
        }
        else
        {
            AudioClocksPerField = 0x19066;
        }
    }
    else
    {
        switch (GetCardSetup()->AudioCrystal)
        {
        case AUDIOCRYSTAL_32110Hz:
            AudioClock = 0x1C2097;
            break;

        case AUDIOCRYSTAL_24576Hz:
            AudioClock = 0x24C000;
            break;
        }

        if (m_VideoStandards[VideoStandard].Is25fps)
        {
            AudioClocksPerField = 0x22740;
        }
        else
        {
            AudioClocksPerField = 0x1CBD5;
        }
    }

    MaskDataDword(SAA7134_AUDIO_CLOCK, AudioClock,
        SAA7134_AUDIO_CLOCK_MASK);

    MaskDataDword(SAA7134_AUDIO_CLOCKS_PER_FIELD, AudioClocksPerField,
        SAA7134_AUDIO_CLOCKS_PER_FIELD_MASK);
}


void CSAA7134Card::SetCh1FMDeemphasis(eAudioFMDeemphasis FMDeemphasis)
{
    BYTE Ch1FMDeemphasis = 0x00;

    switch (FMDeemphasis)
    {
    case AUDIOFMDEEMPHASIS_OFF:
        Ch1FMDeemphasis = 0x04;
        break;

    case AUDIOFMDEEMPHASIS_50_MICROS:
        Ch1FMDeemphasis = 0x00;
        break;

    case AUDIOFMDEEMPHASIS_60_MICROS:
        Ch1FMDeemphasis = 0x01;
        break;

    case AUDIOFMDEEMPHASIS_75_MICROS:
        Ch1FMDeemphasis = 0x02;
        break;

    case AUDIOFMDEEMPHASIS_J17:
        Ch1FMDeemphasis = 0x03;
        break;

    case AUDIOFMDEEMPHASIS_ADAPTIVE:
        Ch1FMDeemphasis = 0x0C;
        break;
    }

    MaskDataByte(SAA7134_FM_DEEMPHASIS, Ch1FMDeemphasis, 0x0F);
}


void CSAA7134Card::SetCh2FMDeemphasis(eAudioFMDeemphasis FMDeemphasis)
{
    BYTE Ch2FMDeemphasis = 0x00;

    switch (FMDeemphasis)
    {
    case AUDIOFMDEEMPHASIS_OFF:
        Ch2FMDeemphasis = 0x40;
        break;

    case AUDIOFMDEEMPHASIS_50_MICROS:
        Ch2FMDeemphasis = 0x00;
        break;

    case AUDIOFMDEEMPHASIS_60_MICROS:
        Ch2FMDeemphasis = 0x10;
        break;

    case AUDIOFMDEEMPHASIS_75_MICROS:
        Ch2FMDeemphasis = 0x20;
        break;

    case AUDIOFMDEEMPHASIS_J17:
        Ch2FMDeemphasis = 0x30;
        break;

    case AUDIOFMDEEMPHASIS_ADAPTIVE:
        Ch2FMDeemphasis = 0xC0;
        break;
    }

    MaskDataByte(SAA7134_FM_DEEMPHASIS, Ch2FMDeemphasis, 0xF0);
}


void CSAA7134Card::SetAudioFMDematrix(eAudioFMDematrix FMDematrix)
{
    BYTE FMDematrixSelect = 0x00;

    switch (FMDematrix)
    {
    case AUDIOFMDEMATRIX_AUTOSWITCHING:
        FMDematrixSelect = 0x80;
        break;

    case AUDIOFMDEMATRIX_MONO1:
        FMDematrixSelect = 0x00;
        break;

    case AUDIOFMDEMATRIX_MONO2:
        FMDematrixSelect = 0x01;
        break;

    case AUDIOFMDEMATRIX_DUAL:
        FMDematrixSelect = 0x02;
        break;

    case AUDIOFMDEMATRIX_DUAL_SWAPPED:
        FMDematrixSelect = 0x03;
        break;

    case AUDIOFMDEMATRIX_STEREO_EUROPE:
        FMDematrixSelect = 0x04;
        break;

    case AUDIOFMDEMATRIX_STEREO_KOREA__6DB:
        FMDematrixSelect = 0x05;
        break;

    case AUDIOFMDEMATRIX_STEREO_KOREA:
        FMDematrixSelect = 0x06;
        break;

    }

    WriteByte(SAA7134_FM_DEMATRIX, FMDematrixSelect);
}


void CSAA7134Card::SetFilterBandwidth(eAudioFilterBandwidth FilterBandwidth)
{
    switch (FilterBandwidth)
    {
    case AUDIOFILTERBANDWIDTH_NARROW_NARROW:
        break;

    case AUDIOFILTERBANDWIDTH_XWIDE_NARROW:
        break;

    case AUDIOFILTERBANDWIDTH_MEDIUM_MEDIUM:
        break;

    case AUDIOFILTERBANDWIDTH_WIDE_WIDE:
        break;

    default:
        break;
    }
}

void CSAA7134Card::SetAudioSampleRate(eAudioSampleRate SampleRate)
{
    BYTE SampleFrequency = 0;

    if (m_AudioInputSource == AUDIOINPUTSOURCE_DAC)
    {
        SampleFrequency = 0x00;
    }
    else
    {
        switch (SampleRate)
        {
        case AUDIOSAMPLERATE_32000Hz: SampleFrequency = 0x01; break;
        case AUDIOSAMPLERATE_44100Hz: SampleFrequency = 0x02; break;
        case AUDIOSAMPLERATE_48000Hz: SampleFrequency = 0x03; break;
        }
    }

    MaskDataByte(SAA7134_SIF_SAMPLE_FREQ, SampleFrequency,
        SAA7134_SIF_SAMPLE_FREQ_SFS);

    m_AudioSampleRate = SampleRate;

    UpdateAudioClocksPerField(m_VideoStandard);
}


void CSAA7134Card::SetAudioSource(eAudioInputSource InputSource)
{
    BYTE LineSelect;

    switch (InputSource)
    {
    case AUDIOINPUTSOURCE_DAC: LineSelect = 0x02; break;
    case AUDIOINPUTSOURCE_LINE1: LineSelect = 0x00; break;
    case AUDIOINPUTSOURCE_LINE2: LineSelect = 0x01; break;
    }

    MaskDataByte(SAA7134_ANALOG_IO_SELECT, LineSelect,
        SAA7134_ANALOG_IO_SELECT_OCS);

    m_AudioInputSource = InputSource;

    if (InputSource == AUDIOINPUTSOURCE_LINE2)
    {
        OrDataByte(SAA7134_ANALOG_IO_SELECT, SAA7134_ANALOG_IO_SELECT_ICS);
    }
    else
    {
        AndDataByte(SAA7134_ANALOG_IO_SELECT, ~SAA7134_ANALOG_IO_SELECT_ICS);
    }

    SetAudioSampleRate(m_AudioSampleRate);

    // TODO: some cards need GPIO for audio selecting
    /*
    BYTE GpioMask = 0x0;  // FlyVideo 3000 
    BYTE InputGpio = 0x0; // FlyVideo 3000

    if (!GpioMask)
        return;

    MaskDataDword(SAA7134_GPIO_GPMODE0,   GpioMask, GpioMask);
    MaskDataDword(SAA7134_GPIO_GPSTATUS0, InputGpio, GpioMask);
    StatGPIO();
    */
}


BOOL CSAA7134Card::IsAudioChannelDetected(eAudioChannel AudioChannel)
{
    BYTE Status;

    if (IsNICAMAudioStandard(m_AudioStandard))
    {
        Status = (ReadByte(SAA7134_NICAM_STATUS) & SAA7134_NICAM_STATUS_SIN);

        switch (AudioChannel)
        {
        case AUDIOCHANNEL_MONO:
            return Status == 0x00 || Status == 0x02;

        case AUDIOCHANNEL_STEREO:
            return Status == 0x02;

        case AUDIOCHANNEL_LANGUAGE1:
        case AUDIOCHANNEL_LANGUAGE2:
            return Status == 0x01;
        }
    }
    else
    {
        Status = (ReadByte(SAA7134_IDENT_SIF) & SAA7134_IDENT_SIF_IDP) >> 5;

        switch (AudioChannel)
        {
        case AUDIOCHANNEL_MONO:
            return (Status & (1 << 1)) == 0;

        case AUDIOCHANNEL_STEREO:
            return (Status & (1 << 2)) != 0;

        case AUDIOCHANNEL_LANGUAGE1:
        case AUDIOCHANNEL_LANGUAGE2:
            return (Status & (1 << 1)) != 0;
        }
    }

    // NEVER_GET_HERE;
    return FALSE;
}


void CSAA7134Card::SetAudioChannel(eAudioChannel AudioChannel)
{
    if (AudioChannel == AUDIOCHANNEL_MONO)
    {
        // Select Left/Left
        MaskDataByte(SAA7134_DSP_OUTPUT_SELECT, 0x10,
            SAA7134_DSP_OUTPUT_SELECT_CSM);

        // Disable automatic stereo adjustment
        AndDataByte(SAA7134_DSP_OUTPUT_SELECT, ~SAA7134_DSP_OUTPUT_SELECT_AASDMA);

        MaskDataByte(SAA7134_STEREO_DAC_OUTPUT_SELECT, 0x00,
            SAA7134_STEREO_DAC_OUTPUT_SELECT_SDOS);    
    }
    else
    {
        MaskDataByte(SAA7134_STEREO_DAC_OUTPUT_SELECT, 0x00,
            SAA7134_STEREO_DAC_OUTPUT_SELECT_SDOS);

        // Enable automatic stereo adjustment and the card
        // will sort itself out
        OrDataByte(SAA7134_DSP_OUTPUT_SELECT, SAA7134_DSP_OUTPUT_SELECT_AASDMA);

        if (AudioChannel == AUDIOCHANNEL_LANGUAGE1)
        {
            MaskDataByte(SAA7134_DSP_OUTPUT_SELECT, 0x10,
                SAA7134_DSP_OUTPUT_SELECT_CSM);
        }
        else if (AudioChannel == AUDIOCHANNEL_LANGUAGE2)
        {
            MaskDataByte(SAA7134_DSP_OUTPUT_SELECT, 0x20,
                SAA7134_DSP_OUTPUT_SELECT_CSM);
        }
    }
}


CSAA7134Card::eAudioChannel CSAA7134Card::GetAudioChannel()
{
    // If automatic select is enabled, things work a bit different
    if (ReadByte(SAA7134_DSP_OUTPUT_SELECT) & SAA7134_DSP_OUTPUT_SELECT_AASDMA)
    {
        if (IsAudioChannelDetected(AUDIOCHANNEL_STEREO))
        {
            return AUDIOCHANNEL_STEREO;
        }
        else if (IsAudioChannelDetected(AUDIOCHANNEL_LANGUAGE2))
        {
            if (ReadByte(SAA7134_DSP_OUTPUT_SELECT) & 0x20)
            {
                return AUDIOCHANNEL_LANGUAGE2;
            }
            return AUDIOCHANNEL_LANGUAGE1;
        }
        return AUDIOCHANNEL_MONO;
    }

    switch (ReadByte(SAA7134_DSP_OUTPUT_SELECT) &
        SAA7134_DSP_OUTPUT_SELECT_CSM)
    {
    case 0x00:  // L / R
    case 0x30:  // R / L
    case 0x40:  // (L+R)/2 / (L+R)/2
        if (IsAudioChannelDetected(AUDIOCHANNEL_STEREO))
        {
            return AUDIOCHANNEL_STEREO;
        }
        break;

    case 0x10:
        if (IsAudioChannelDetected(AUDIOCHANNEL_LANGUAGE1))
        {
            return AUDIOCHANNEL_LANGUAGE1;
        }
        break;

    case 0x20:
        if (IsAudioChannelDetected(AUDIOCHANNEL_LANGUAGE2))
        {
            return AUDIOCHANNEL_LANGUAGE2;
        }
        break;

    default:
        // NEVER_GET_HERE;
        break;
    }
    return AUDIOCHANNEL_MONO;
}


void CSAA7134Card::SetAudioMute()
{
    // if SAA7134
    WriteByte(SAA7134_AUDIO_MUTE_CTRL, 0xFF);

    // if SAA7130 select MUTE line
}

void CSAA7134Card::SetAudioUnMute(long nVolume, eAudioInputSource InputSource)
{
    // if SAA7134
    MaskDataByte(SAA7134_AUDIO_MUTE_CTRL, 0x00, SAA7134_AUDIO_MUTE_CTRL_MUTSOUT);

    // SetAudioSource(Input);
}

// Unlatched. has some problems with distorted sound
void CSAA7134Card::SetAudioVolume(BYTE nGain)
{
    // nGain = -15..0..15, 0 = normal

    // Dual FM Level adjust
    WriteByte(SAA7134_CHANNEL1_LEVEL, nGain & 0x1F);
    WriteByte(SAA7134_CHANNEL2_LEVEL, nGain & 0x1F);
    // NICAM Levle adjust
    WriteByte(SAA7134_NICAM_LEVEL_ADJUST, nGain & 0x1F);
}

void CSAA7134Card::SetAudioLeftVolume(BYTE nGain)
{
    WriteByte(SAA7134_CHANNEL1_LEVEL, nGain & 0x1F);
}

void CSAA7134Card::SetAudioRightVolume(BYTE nGain)
{
    WriteByte(SAA7134_CHANNEL2_LEVEL, nGain & 0x1F);
}

void CSAA7134Card::SetAudioNicamVolume(BYTE nGain)
{
    WriteByte(SAA7134_NICAM_LEVEL_ADJUST, nGain & 0x1F);
}

int CSAA7134Card::GetAudioLeftVolume()
{
    BYTE Gain = ReadByte(SAA7134_CHANNEL1_LEVEL) & 0x1F;

    if ((Gain & 0x10) > 0)
    {
        return Gain - 32;
    }
    return Gain;
}

int CSAA7134Card::GetAudioRightVolume()
{
    BYTE Gain = ReadByte(SAA7134_CHANNEL2_LEVEL) & 0x1F;

    if ((Gain & 0x10) > 0)
    {
        return Gain - 32;
    }
    return Gain;
}

int CSAA7134Card::GetAudioNicamVolume()
{
    BYTE Gain = ReadByte(SAA7134_NICAM_LEVEL_ADJUST) & 0x1F;

    if ((Gain & 0x10) > 0)
    {
        return Gain - 32;
    }
    return Gain;
}

void CSAA7134Card::SetAudioBalance(WORD nBalance)
{
    // TODO: Need to implement
}

void CSAA7134Card::SetAudioBass(WORD nBass)
{
    // TODO: Need to implement
    // SAA7134 doesn't have Bass but SAA7133 & SAA7135 does
}

void CSAA7134Card::SetAudioTreble(WORD nTreble)
{
    // TODO: Need to implement
    // SAA7134 doesn't have Treble but SAA7133 & SAA7135 does
}


int CSAA7134Card::GetInputAudioLine(int nInput)
{
    if(nInput < m_TVCards[m_CardType].NumInputs && nInput >= 0)
    {
        return m_TVCards[m_CardType].Inputs[nInput].AudioLineSelect;
    }
    return 0;
}

LPCSTR CSAA7134Card::GetAudioStandardName(eAudioStandard audioStandard)
{
    return m_AudioStandards[audioStandard].Name;
}

