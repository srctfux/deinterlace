/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Common.h,v 1.8 2002-11-07 18:54:21 atnak Exp $
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
// Change Log
//
// Date          Developer             Changes
//
// 01 Oct 2002   Atsushi Nakagawa      Initial Release
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.7  2002/10/29 03:07:19  atnak
// Added SAA713x TreeSettings Page
//
// Revision 1.6  2002/10/26 06:59:30  atnak
// Minor change to video standards definition
//
// Revision 1.5  2002/10/26 04:41:44  atnak
// Clean up + added auto card detection
//
// Revision 1.4  2002/10/20 07:41:04  atnak
// custom audio standard setup + etc
//
// Revision 1.3  2002/10/16 11:38:46  atnak
// cleaned up audio standard stuff
//
// Revision 1.2  2002/10/04 23:40:46  atnak
// proper support for audio channels mono,stereo,lang1,lang2 added
//
// Revision 1.1  2002/10/03 23:42:06  atnak
// SAA7134Common.h SAA7134Common.cpp SAA7134Card_Video.cpp added
//
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __SAA7134COMMON_H___
#define __SAA7134COMMON_H___

#include "TVFormats.h"


class CSAA7134Common
{
protected:

    enum
    {
        kMAX_VBILINES               = 19,
        kMAX_VIDLINES               = 288,

        // DO NOT change
        kMAX_FRAMEBUFFERS           = 2,
        kMAX_FIELDBUFFERS           = kMAX_FRAMEBUFFERS * 2,
        kMAX_VBI_PAGETABLES         = kMAX_FRAMEBUFFERS,
        kMAX_VID_PAGETABLES         = kMAX_FRAMEBUFFERS,
        kMAX_PAGETABLES             = kMAX_VID_PAGETABLES + kMAX_VBI_PAGETABLES,
    };

    /// SAA713x Card Ids
    enum eSAA7134CardId
    {
        SAA7134CARDID_UNKNOWN = 0,
        SAA7134CARDID_PROTEUSPRO,
        SAA7134CARDID_FLYVIDEO3000,
        SAA7134CARDID_FLYVIDEO2000,
        SAA7134CARDID_EMPRESS,
        SAA7134CARDID_MONSTERTV,
        SAA7134CARDID_TEVIONMD9717,
        SAA7134CARDID_TVSTATIONRDS,
        SAA7134CARDID_CINERGY400,
        SAA7134CARDID_MEDION5044,
        SAA7134CARDID_KWTV713XRF,
        SAA7134CARDID_LASTONE,
    };

    enum eTaskID
    {
        TASKID_A                = 0,
        TASKID_B                = 1
    };

    enum eRegionID
    {
        REGIONID_INVALID        = -1,
        REGIONID_VIDEO_A        = 0,
        REGIONID_VIDEO_B        = 1,
        REGIONID_VBI_A          = 2,
        REGIONID_VBI_B          = 3
    };

    /// Bit masks used in TFieldID
    enum
    {
        FIELDID_FRAMESHIFT        = 1,
        FIELDID_FIELDMASK         = 0x01,
        FIELDID_SECONDFIELD       = FIELDID_FIELDMASK
    };

    enum
    {
        SAA7134_DEFAULT_BRIGHTNESS          = 0x80,
        SAA7134_DEFAULT_CONTRAST            = 0x44,
        SAA7134_DEFAULT_SATURATION          = 0x40,
        SAA7134_DEFAULT_HUE                 = 0x00,
        SAA7134_DEFAULT_OVERSCAN            = 4,

        SAA7134_DEFAULT_PAL_SATURATION      = 0x3D,
        SAA7134_DEFAULT_PAL_OVERSCAN        = 8,
        SAA7134_DEFAULT_NTSC_SATURATION     = 0x43,
        SAA7134_DEFAULT_NTSC_OVERSCAN       = 12,
    };

    enum eVideoStandard
    {
        VIDEOSTANDARD_INVALID = -1,
        VIDEOSTANDARD_AUTODETECT = 0,   // Automatic detection by card
        VIDEOSTANDARD_PAL_BGDHI,        // 50Hz, PAL BGDHI (4.434Mhz)
        VIDEOSTANDARD_PAL_N_COMBO,      // 50Hz, Combination-PAL N (3.582MHz)
        VIDEOSTANDARD_PAL_60,           // 60Hz, PAL 4.43 (4.434Mhz)
        VIDEOSTANDARD_PAL_M,            // 60Hz, PAL M (3.576MHz)
        VIDEOSTANDARD_SECAM,            // 50Hz, SECAM
        VIDEOSTANDARD_NTSC_M,           // 60Hz, NTSC M (3.58MHz)
        VIDEOSTANDARD_NTSC_60,          // 60Hz, NTSC 4.43 (4.434Mhz)
        VIDEOSTANDARD_NTSC_Japan,       // 60Hz, NTSC-Japan (3.58MHz)
        VIDEOSTANDARD_NTSC_50,          // 50Hz, NTSC 4.43 (4.434Mhz)
        VIDEOSTANDARD_NTSC_N,           // 50Hz, NTSC N (3.58MHz)
    };

    enum eHPLLMode
    {
        HPLLMODE_TV = 0,
        HPLLMODE_VCR,
        HPLLMODE_FAST_TRACKING,
        HPLLMODE_LASTONE
    };

    enum eVSyncRecovery
    {
        VSYNCRECOVERY_NORMAL = 0,
        VSYNCRECOVERY_FAST_TRACKING,
        VSYNCRECOVERY_FREE_RUNNING,
        VSYNCRECOVERY_IMMEDIATE_MODE,
    };

    enum eAudioInputSource
    {
        /// standard tuner line
        AUDIOINPUTSOURCE_DAC = 0,
        /// internal line 1 input
        AUDIOINPUTSOURCE_LINE1,
        /// internal line 2 input
        AUDIOINPUTSOURCE_LINE2,
    };

    enum eAudioStandard
    {
        AUDIOSTANDARD_BG_DUAL_FM = 0,
        AUDIOSTANDARD_DK1_DUAL_FM,
        AUDIOSTANDARD_DK2_DUAL_FM,
        AUDIOSTANDARD_DK_FM_MONO,
        AUDIOSTANDARD_DK3_DUAL_FM,
        AUDIOSTANDARD_M_DUAL_FM,
        AUDIOSTANDARD_BG_NICAM_FM,
        AUDIOSTANDARD_L_NICAM_AM,
        AUDIOSTANDARD_I_NICAM_FM,
        AUDIOSTANDARD_DK_NICAM_FM,
        AUDIOSTANDARD_DK_NICAM_FM_HDEV2,
        AUDIOSTANDARD_DK_NICAM_FM_HDEV3,
        AUDIOSTANDARD_M_BTSC,
        AUDIOSTANDARD_M_BTSC_MONO,
        AUDIOSTANDARD_M_EIAJ,
        AUDIOSTANDARD_FM_RADIO,
        AUDIOSTANDARD_SAT,
        AUDIOSTANDARD_SAT_MONO,
        AUDIOSTANDARD_SAT_ADR,
        AUDIOSTANDARD_LASTONE
    };

    enum eAudioChannel
    {
        AUDIOCHANNEL_MONO = 0,
        AUDIOCHANNEL_STEREO,
        AUDIOCHANNEL_LANGUAGE1,
        AUDIOCHANNEL_LANGUAGE2,
    };

    enum eAudioCarrierMode
    {
        AUDIOCHANNELMODE_NONE = 0,
        AUDIOCHANNELMODE_FM,
        AUDIOCHANNELMODE_FM_KOREA,          // Korean stereo standard
        AUDIOCHANNELMODE_AM,
        AUDIOCHANNELMODE_NICAM,
        AUDIOCHANNELMODE_BTSC,              // mono only on SAA7134
        AUDIOCHANNELMODE_EIAJ,              // mono only on SAA7134
    };

    enum eAudioFMDeemphasis
    {
        AUDIOFMDEEMPHASIS_OFF = 0,
        AUDIOFMDEEMPHASIS_50_MICROS,
        AUDIOFMDEEMPHASIS_60_MICROS,
        AUDIOFMDEEMPHASIS_75_MICROS,
        AUDIOFMDEEMPHASIS_J17,
        AUDIOFMDEEMPHASIS_ADAPTIVE
    };

    enum eAudioFMDematrix
    {
        AUDIOFMDEMATRIX_AUTOSWITCHING = 0,
        AUDIOFMDEMATRIX_MONO1,
        AUDIOFMDEMATRIX_MONO2,
        AUDIOFMDEMATRIX_DUAL,
        AUDIOFMDEMATRIX_DUAL_SWAPPED,
        AUDIOFMDEMATRIX_STEREO_EUROPE,
        AUDIOFMDEMATRIX_STEREO_KOREA__6DB,
        AUDIOFMDEMATRIX_STEREO_KOREA
    };

    enum eAudioFilterBandwidth
    {
        AUDIOFILTERBANDWIDTH_NARROW_NARROW = 0,
        AUDIOFILTERBANDWIDTH_XWIDE_NARROW,
        AUDIOFILTERBANDWIDTH_MEDIUM_MEDIUM,
        AUDIOFILTERBANDWIDTH_WIDE_WIDE,
    };

    enum eAudioSampleRate
    {
        AUDIOSAMPLERATE_32000Hz = 0,
        AUDIOSAMPLERATE_44100Hz,
        AUDIOSAMPLERATE_48000Hz
    };

    #define AUDIO_CARRIER(freq) ((int)((float)(freq/12.288)*(1<<24)))
        enum eAudioCarrier
        {
            AUDIO_CARRIER_4_5         = AUDIO_CARRIER(4.5),
            AUDIO_CARRIER_4_724212    = AUDIO_CARRIER(4.724212),
            AUDIO_CARRIER_5_5         = AUDIO_CARRIER(5.5),
            AUDIO_CARRIER_5_7421875   = AUDIO_CARRIER(5.7421875),
            AUDIO_CARRIER_5_85        = AUDIO_CARRIER(5.85),
            AUDIO_CARRIER_6_0         = AUDIO_CARRIER(6.0),
            AUDIO_CARRIER_6_12        = AUDIO_CARRIER(6.12),
            AUDIO_CARRIER_6_2578125   = AUDIO_CARRIER(6.2578125),
            AUDIO_CARRIER_6_5         = AUDIO_CARRIER(6.5),
            AUDIO_CARRIER_6_552       = AUDIO_CARRIER(6.552),
            AUDIO_CARRIER_6_7421875   = AUDIO_CARRIER(6.7421875),
            AUDIO_CARRIER_7_02        = AUDIO_CARRIER(7.02),
            AUDIO_CARRIER_7_20        = AUDIO_CARRIER(7.20),
            AUDIO_CARRIER_10_7        = AUDIO_CARRIER(10.7),
        };
    #undef AUDIO_CARRIER

    enum eAutomaticVolume
    {
        AUTOMATICVOLUME_OFF = 0,
        AUTOMATICVOLUME_SHORTDECAY,
        AUTOMATICVOLUME_MEDIUMDECAY,
        AUTOMATICVOLUME_LONGDECAY,
    };

protected:

    typedef struct
    {
        WORD                wFieldWidth;
        WORD                wFieldHeight;
        WORD                wHDelay;
        WORD                wVDelay;
        BOOL                Is25fps;
        WORD                wVBIStartLine;
        WORD                wVBIStopLine;
        BYTE                CSTDMask;
    } TVideoStandardDefinition;


    /// Used to store audio standard settings
    typedef struct
    {
        char*               Name;
        eAudioCarrier       Carrier1;
        eAudioCarrier       Carrier2;
        eAudioCarrierMode   Carrier1Mode;
        eAudioCarrierMode   Carrier2Mode;
        eAudioFMDeemphasis  Ch1FMDeemphasis;
        eAudioFMDeemphasis  Ch2FMDeemphasis;
    } TAudioStandardDefinition;


    /// Used by Source and Card to identify fields
    typedef char TFieldID;


protected:

    BOOL        IsRegionIDVideo(eRegionID RegionID);
    BOOL        IsRegionIDVBI(eRegionID RegionID);

    eTaskID     RegionID2TaskID(eRegionID RegionID);
    eRegionID   TaskID2VideoRegion(eTaskID TaskID);
    eRegionID   TaskID2VBIRegion(eTaskID TaskID);

    TFieldID    GetNextFieldID(TFieldID FieldID);
    TFieldID    GetPrevFieldID(TFieldID FieldID);
    int         GetFieldDistance(TFieldID FromID, TFieldID ToID);

    BOOL IsPALVideoStandard(eVideoStandard VideoStandard);
    BOOL IsSECAMVideoStandard(eVideoStandard VideoStandard);
    BOOL IsNTSCVideoStandard(eVideoStandard VideoStandard);

    eVideoStandard TVFormat2VideoStandard(eVideoFormat TVFormat);
    eAudioStandard TVFormat2AudioStandard(eVideoFormat VideoFormat);

    BOOL IsDualFMAudioStandard(eAudioStandard AudioStandard);
    BOOL IsNICAMAudioStandard(eAudioStandard AudioStandard);

    int GetMaxAudioCarrierNames();
    int GetMaxFMDeemphasisNames();


protected:

    // Lists the items displayed in the UI
    static eAudioCarrier            m_AudioCarrierList[];
    static const char*              m_AudioFMDeemphasisSzList[];

    static const char*              m_AutomaticVolumeSzList[];

    // Video standards (TV Formats) table, uses eVideoFormat
    static TVideoStandardDefinition m_VideoStandards[];

    // Audio standards table
    static TAudioStandardDefinition m_AudioStandards[];
};


#endif