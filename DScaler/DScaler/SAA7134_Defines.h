/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134_Defines.h,v 1.4 2002-09-14 19:40:48 atnak Exp $
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
// Revision 1.3  2002/09/10 12:25:05  atnak
// add some more registers info
//
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __SAA7134_DEFINES_H___
#define __SAA7134_DEFINES_H___


/*
 * philips saa7134 registers
 */

/* ------------------------------------------------------------------ */
/*
 * PCI ID's
 */
#ifndef PCI_DEVICE_ID_PHILIPS_SAA7130
# define PCI_DEVICE_ID_PHILIPS_SAA7130 0x7130
#endif
#ifndef PCI_DEVICE_ID_PHILIPS_SAA7134
# define PCI_DEVICE_ID_PHILIPS_SAA7134 0x7134
#endif

/* ------------------------------------------------------------------ */
/*
 *  registers -- 32 bit
 */

/* DMA channels, n = 0 ... 6 */
#define SAA7134_RS_E_BA(n)                  (0x200|(n)<<4) // evens base address
#define SAA7134_RS_O_BA(n)                  (0x204|(n)<<4) // odds base address
#define SAA7134_RS_PITCH(n)                 (0x208|(n)<<4) // bytes to move per line
#define SAA7134_RS_CONTROL_0(n)             (0x20C|(n)<<4) // 20 bits, pglist page?
#define SAA7134_RS_CONTROL_1(n)             (0x20D|(n)<<4)
#define SAA7134_RS_CONTROL_2(n)             (0x20E|(n)<<4)
#define   SAA7134_RS_CONTROL_2_BURST_2          (1<<5)
#define   SAA7134_RS_CONTROL_2_BURST_4          (2<<5)
#define   SAA7134_RS_CONTROL_2_BURST_8          (3<<5)
#define   SAA7134_RS_CONTROL_2_BURST_16         (4<<5)
#define   SAA7134_RS_CONTROL_2_BURST_32         (5<<5)
#define   SAA7134_RS_CONTROL_2_BURST_64         (6<<5)
#define   SAA7134_RS_CONTROL_2_BURST_MAX        (7<<5)
#define   SAA7134_RS_CONTROL_2_ME               (1<<4)
#define SAA7134_RS_CONTROL_3(n)             (0x20F|(n)<<4)
#define   SAA7134_RS_CONTROL_3_WSWAP            (1<<1)
#define   SAA7134_RS_CONTROL_3_BSWAP            (1<<0)

#define SAA7134_RS_CONTROL(n)               (0x20C|(n)<<4)
#define   SAA7134_RS_CONTROL_WSWAP              (1<<25)
#define   SAA7134_RS_CONTROL_BSWAP              (1<<24)
#define   SAA7134_RS_CONTROL_BURST_2            (0x01<<21)
#define   SAA7134_RS_CONTROL_BURST_4            (0x02<<21)
#define   SAA7134_RS_CONTROL_BURST_8            (0x03<<21)
#define   SAA7134_RS_CONTROL_BURST_16           (0x04<<21)
#define   SAA7134_RS_CONTROL_BURST_32           (0x05<<21)
#define   SAA7134_RS_CONTROL_BURST_64           (0x06<<21)
#define   SAA7134_RS_CONTROL_BURST_MAX          (0x07<<21)
#define   SAA7134_RS_CONTROL_ME                 (1<<20)

#define SAA7134_FIFO_SIZE                   0x2A0
#define SAA7134_THRESHOULD                  0x2A4

// main control
#define SAA7134_MAIN_CTRL                   0x2A8
#define   SAA7134_MAIN_CTRL_VPLLE               (1<<15)
#define   SAA7134_MAIN_CTRL_APLLE               (1<<14)
#define   SAA7134_MAIN_CTRL_EXOSC               (1<<13)
#define   SAA7134_MAIN_CTRL_EVFE1               (1<<12)
#define   SAA7134_MAIN_CTRL_EVFE2               (1<<11)
#define   SAA7134_MAIN_CTRL_ESFE                (1<<10)
#define   SAA7134_MAIN_CTRL_EBADC               (1<<9)
#define   SAA7134_MAIN_CTRL_EBDAC               (1<<8)
#define   SAA7134_MAIN_CTRL_TE6                 (1<<6)
#define   SAA7134_MAIN_CTRL_TE5                 (1<<5)
#define   SAA7134_MAIN_CTRL_TE4                 (1<<4)
#define   SAA7134_MAIN_CTRL_TE3                 (1<<3)
#define   SAA7134_MAIN_CTRL_TE2                 (1<<2)
#define   SAA7134_MAIN_CTRL_TE1                 (1<<1)
#define   SAA7134_MAIN_CTRL_TE0                 (1<<0)

// DMA status
#define SAA7134_DMA_STATUS                  0x2AC

// audio / video status
#define SAA7134_AV_STATUS                   0x2C0
#define   SAA7134_AV_STATUS_STEREO              (1<<17)
#define   SAA7134_AV_STATUS_DUAL                (1<<16)
#define   SAA7134_AV_STATUS_PILOT               (1<<15)
#define   SAA7134_AV_STATUS_SMB                 (1<<14)
#define   SAA7134_AV_STATUS_DMB                 (1<<13)
#define   SAA7134_AV_STATUS_VDSP                (1<<12)
#define   SAA7134_AV_STATUS_IIC_STATUS          (3<<10)
#define   SAA7134_AV_STATUS_MVM                 (7<<7)
#define   SAA7134_AV_STATUS_FIDT                (1<<6)
#define   SAA7134_AV_STATUS_INTL                (1<<5)
#define   SAA7134_AV_STATUS_RDCAP               (1<<4)
#define   SAA7134_AV_STATUS_PWR_ON              (1<<3)
#define   SAA7134_AV_STATUS_LOAD_ERR            (1<<2)
#define   SAA7134_AV_STATUS_TRIG_ERR            (1<<1)
#define   SAA7134_AV_STATUS_CONF_ERR            (1<<0)

// interrupt
#define SAA7134_IRQ1                        0x2C4
#define   SAA7134_IRQ1_INTE_RA3_7               (1<<31)
#define   SAA7134_IRQ1_INTE_RA3_6               (1<<30)
#define   SAA7134_IRQ1_INTE_RA3_5               (1<<29)
#define   SAA7134_IRQ1_INTE_RA3_4               (1<<28)
#define   SAA7134_IRQ1_INTE_RA3_3               (1<<27)
#define   SAA7134_IRQ1_INTE_RA3_2               (1<<26)
#define   SAA7134_IRQ1_INTE_RA3_1               (1<<25)
#define   SAA7134_IRQ1_INTE_RA3_0               (1<<24)
#define   SAA7134_IRQ1_INTE_RA2_7               (1<<23)
#define   SAA7134_IRQ1_INTE_RA2_6               (1<<22)
#define   SAA7134_IRQ1_INTE_RA2_5               (1<<21)
#define   SAA7134_IRQ1_INTE_RA2_4               (1<<20)
#define   SAA7134_IRQ1_INTE_RA2_3               (1<<19)
#define   SAA7134_IRQ1_INTE_RA2_2               (1<<18)
#define   SAA7134_IRQ1_INTE_RA2_1               (1<<17)
#define   SAA7134_IRQ1_INTE_RA2_0               (1<<16)
#define   SAA7134_IRQ1_INTE_RA1_7               (1<<15)
#define   SAA7134_IRQ1_INTE_RA1_6               (1<<14)
#define   SAA7134_IRQ1_INTE_RA1_5               (1<<13)
#define   SAA7134_IRQ1_INTE_RA1_4               (1<<12)
#define   SAA7134_IRQ1_INTE_RA1_3               (1<<11)
#define   SAA7134_IRQ1_INTE_RA1_2               (1<<10)
#define   SAA7134_IRQ1_INTE_RA1_1               (1<<9)
#define   SAA7134_IRQ1_INTE_RA1_0               (1<<8)
#define   SAA7134_IRQ1_INTE_RA0_7               (1<<7)
#define   SAA7134_IRQ1_INTE_RA0_6               (1<<6)
#define   SAA7134_IRQ1_INTE_RA0_5               (1<<5)
#define   SAA7134_IRQ1_INTE_RA0_4               (1<<4)
#define   SAA7134_IRQ1_INTE_RA0_3               (1<<3)
#define   SAA7134_IRQ1_INTE_RA0_2               (1<<2)
#define   SAA7134_IRQ1_INTE_RA0_1               (1<<1)
#define   SAA7134_IRQ1_INTE_RA0_0               (1<<0)
#define SAA7134_IRQ2                        0x2C8
#define   SAA7134_IRQ2_INTE_SC2                 (1<<10)
#define   SAA7134_IRQ2_INTE_SC1                 (1<<9)
#define   SAA7134_IRQ2_INTE_SC0                 (1<<8)
#define   SAA7134_IRQ2_INTE_DEC5                (1<<7)
#define   SAA7134_IRQ2_INTE_DEC4                (1<<6)
#define   SAA7134_IRQ2_INTE_DEC3                (1<<5)
#define   SAA7134_IRQ2_INTE_DEC2                (1<<4)
#define   SAA7134_IRQ2_INTE_DEC1                (1<<3)
#define   SAA7134_IRQ2_INTE_DEC0                (1<<2)
#define   SAA7134_IRQ2_INTE_PE                  (1<<1)
#define   SAA7134_IRQ2_INTE_AR                  (1<<0)
#define SAA7134_IRQ_REPORT                  0x2CC
#define   SAA7134_IRQ_REPORT_LOAD_ERR           (1<<13)
#define   SAA7134_IRQ_REPORT_CONF_ERR           (1<<12)
#define   SAA7134_IRQ_REPORT_TRIG_ERR           (1<<11)
#define   SAA7134_IRQ_REPORT_MMC                (1<<10)
#define   SAA7134_IRQ_REPORT_FIDT               (1<<9)
#define   SAA7134_IRQ_REPORT_INTL               (1<<8)
#define   SAA7134_IRQ_REPORT_RDCAP              (1<<7)
#define   SAA7134_IRQ_REPORT_PWR_ON             (1<<6)
#define   SAA7134_IRQ_REPORT_PE                 (1<<5)
#define   SAA7134_IRQ_REPORT_AR                 (1<<4)
#define   SAA7134_IRQ_REPORT_DONE_RA3           (1<<3)
#define   SAA7134_IRQ_REPORT_DONE_RA2           (1<<2)
#define   SAA7134_IRQ_REPORT_DONE_RA1           (1<<1)
#define   SAA7134_IRQ_REPORT_DONE_RA0           (1<<0)
#define SAA7134_IRQ_STATUS                  0x2D0


/* ------------------------------------------------------------------ */
/*
 *  registers -- 8 bit
 */

// video decoder
#define SAA7134_INCR_DELAY                      0x101
#define SAA7134_ANALOG_IN_CTRL1                 0x102
#define SAA7134_ANALOG_IN_CTRL2                 0x103
#define SAA7134_ANALOG_IN_CTRL3                 0x104
#define SAA7134_ANALOG_IN_CTRL4                 0x105
#define SAA7134_HSYNC_START                     0x106
#define SAA7134_HSYNC_STOP                      0x107
#define SAA7134_SYNC_CTRL                       0x108
// guessed these by trying them out
#define     SAA7134_SYNC_CTRL_RESYNC                (1<<0)  // resyncs while set
#define     SAA7134_SYNC_CTRL_V_LOCK                (1<<1)
#define     SAA7134_SYNC_CTRL_H_CYCLE               (1<<2)
#define     SAA7134_SYNC_CTRL_STANDARD_SIGNAL       (1<<3)
#define     SAA7134_SYNC_CTRL_UNKNWN2               (1<<4)
#define     SAA7134_SYNC_CTRL_UNKNWN3               (1<<5)
#define     SAA7134_SYNC_CTRL_UNKNWN4               (1<<6)
#define     SAA7134_SYNC_CTRL_UNKNWN5               (1<<7)
#define SAA7134_LUMA_CTRL                       0x109
// guessed these - unverified
#define     SAA7134_LUMA_CTRL_BLUR                  (0x0F)  // blurs 1..15
#define     SAA7134_LUMA_CTRL_UNKNWN1               (1<<4)
#define     SAA7134_LUMA_CTRL_V_CHROMA_SHIFT        (1<<5)  // cleans up vert colour shift
#define     SAA7134_LUMA_CTRL_2D_COMB_FILTER        (1<<6)
#define     SAA7134_LUMA_CTRL_SVIDEO                (1<<7)

#define SAA7134_DEC_LUMA_BRIGHT                 0x10a // Decoder Brightness
#define SAA7134_DEC_LUMA_CONTRAST               0x10b // Decoder Contrast
#define SAA7134_DEC_CHROMA_SATURATION           0x10c // Decoder Saturation
#define SAA7134_DEC_CHROMA_HUE                  0x10d // Decoder Hue
#define SAA7134_CHROMA_CTRL1                    0x10e
#define SAA7134_CHROMA_GAIN                     0x10f
#define     SAA7134_CHROMA_GAIN_VALUE               (0x7F)  // 0 .. 127
#define     SAA7134_CHROMA_GAIN_DISABLE_AUTO        (1<<7)  // Set for auto
#define SAA7134_CHROMA_CTRL2                    0x110
#define SAA7134_MODE_DELAY_CTRL                 0x111

#define SAA7134_ANALOG_ADC                      0x114
#define SAA7134_VGATE_START                     0x115
#define SAA7134_VGATE_STOP                      0x116
#define SAA7134_MISC_VGATE_MSB                  0x117
#define SAA7134_RAW_DATA_GAIN                   0x118
#define SAA7134_RAW_DATA_OFFSET                 0x119
#define SAA7134_STATUS_VIDEO                    0x11E
#define SAA7134_STATUS_VIDEO_HIBYTE             0x11F
#define     SAA7134_STATUS_VIDEO_DCSTD0             (1<<0)
#define     SAA7134_STATUS_VIDEO_DCSCT1             (1<<1)
#define     SAA7134_STATUS_VIDEO_WIPA               (1<<2)
#define     SAA7134_STATUS_VIDEO_GLIMB              (1<<3)
#define     SAA7134_STATUS_VIDEO_GLIMT              (1<<4)
#define     SAA7134_STATUS_VIDEO_SLTCA              (1<<5)
#define     SAA7134_STATUS_VIDEO_HLCK               (1<<6)
#define     SAA7134_STATUS_VIDEO_RDCAP              (1<<8)
#define     SAA7134_STATUS_VIDEO_COPRO              (1<<9)
#define     SAA7134_STATUS_VIDEO_COLSTR             (1<<10)
#define     SAA7134_STATUS_VIDEO_TYPE3              (1<<11)
#define     SAA7134_STATUS_VIDEO_FIDT               (1<<13)
#define     SAA7134_STATUS_VIDEO_HLVLN              (1<<14)
#define     SAA7134_STATUS_VIDEO_INTL               (1<<15)

#define SAA7134_SOURCE_TIMING1                  0x000
#define SAA7134_SOURCE_TIMING2                  0x001
#define SAA7134_REGION_ENABLE                   0x004
#define     SAA7134_REGION_ENABLE_VIDEO_A           (1<<0) /* unverified */
#define     SAA7134_REGION_ENABLE_VIDEO_B           (1<<4) /* unverified */
#define     SAA7134_REGION_ENABLE_VBI_A             (1<<1) /* unverified */
#define     SAA7134_REGION_ENABLE_VBI_B             (1<<5) /* unverified */
#define SAA7134_SCALER_STATUS                   0x006
#define SAA7134_SCALER_STATUS_HIBYTE            0x007
#define     SAA7134_SCALER_STATUS_VID_A             (1<<0)
#define     SAA7134_SCALER_STATUS_VBI_A             (1<<1)
#define     SAA7134_SCALER_STATUS_VID_B             (1<<4)
#define     SAA7134_SCALER_STATUS_VBI_B             (1<<5)
#define     SAA7134_SCALER_STATUS_TRERR             (1<<8)
#define     SAA7134_SCALER_STATUS_CFERR             (1<<9)
#define     SAA7134_SCALER_STATUS_LDERR             (1<<10)
#define     SAA7134_SCALER_STATUS_WASRST            (1<<11)
#define     SAA7134_SCALER_STATUS_FIDSCI            (1<<12)
#define     SAA7134_SCALER_STATUS_FIDSCO            (1<<13)
#define     SAA7134_SCALER_STATUS_D6_D5             (1<<14)
#define     SAA7134_SCALER_STATUS_TASK              (1<<15)

#define SAA7134_START_GREEN                     0x00C
#define SAA7134_START_BLUE                      0x00D
#define SAA7134_START_RED                       0x00E
// x = 0 .. 15
#define SAA7134_GREEN_PATH(x)                   (0x010|x)
#define SAA7134_BLUE_PATH(x)                    (0x020|x)
#define SAA7134_RED_PATH(x)                     (0x030|x)


#define SAA7134_TASK_A_MASK                     0x040
#define SAA7134_TASK_B_MASK                     0x080

#define SAA7134_TASK_CONDITIONS(t)              (0x00|t)
#define SAA7134_FIELD_HANDLING(t)               (0x01|t)
#define SAA7134_DATA_PATH(t)                    (0x02|t)

#define SAA7134_VBI_H_START(t)                  (0x04|t) 
#define SAA7134_VBI_H_START_HIBYTE(t)           (0x05|t)
#define SAA7134_VBI_H_STOP(t)                   (0x06|t)
#define SAA7134_VBI_H_STOP_HIBYTE(t)            (0x07|t)
#define SAA7134_VBI_V_START(t)                  (0x08|t)
#define SAA7134_VBI_V_START_HIBYTE(t)           (0x09|t)
#define SAA7134_VBI_V_STOP(t)                   (0x0a|t)
#define SAA7134_VBI_V_STOP_HIBYTE(t)            (0x0b|t)
#define SAA7134_VBI_H_LEN(t)                    (0x0c|t)
#define SAA7134_VBI_H_LEN_HIBYTE(t)             (0x0d|t)
#define SAA7134_VBI_V_LEN(t)                    (0x0e|t)
#define SAA7134_VBI_V_LEN_HIBYTE(t)             (0x0f|t)

#define SAA7134_VIDEO_H_START(t)                (0x14|t)
#define SAA7134_VIDEO_H_START_HIBYTE(t)         (0x15|t)
#define SAA7134_VIDEO_H_STOP(t)                 (0x16|t)
#define SAA7134_VIDEO_H_STOP_HIBYTE(t)          (0x17|t)
#define SAA7134_VIDEO_V_START(t)                (0x18|t)
#define SAA7134_VIDEO_V_START_HIBYTE(t)         (0x19|t)
#define SAA7134_VIDEO_V_STOP(t)                 (0x1A|t)
#define SAA7134_VIDEO_V_STOP_HIBYTE(t)          (0x1B|t)
#define SAA7134_VIDEO_PIXELS(t)                 (0x1C|t)
#define SAA7134_VIDEO_PIXELS_HIBYTE(t)          (0x1D|t)
#define SAA7134_VIDEO_LINES(t)                  (0x1E|t)
#define SAA7134_VIDEO_LINES_HIBYTE(t)           (0x1F|t)

#define SAA7134_H_PRESCALE(t)                   (0x20|t)
#define SAA7134_ACC_LENGTH(t)                   (0x21|t)
#define SAA7134_LEVEL_CTRL(t)                   (0x22|t)
#define SAA7134_FIR_PREFILTER_CTRL(t)           (0x23|t)
#define SAA7134_LUMA_BRIGHT(t)                  (0x24|t)
#define SAA7134_LUMA_CONTRAST(t)                (0x25|t)
#define SAA7134_CHROMA_SATURATION(t)            (0x26|t)
#define SAA7134_VBI_H_SCALE_INC(t)              (0x28|t)
#define SAA7134_VBI_H_SCALE_INC_HIBYTE(t)       (0x29|t)
#define SAA7134_VBI_PHASE_OFFSET_LUMA(t)        (0x2A|t)
#define SAA7134_VBI_PHASE_OFFSET_CHROMA(t)      (0x2B|t)
#define SAA7134_H_SCALE_INC(t)                  (0x2C|t)
#define SAA7134_H_SCALE_INC_HIBYTE(t)           (0x2D|t)
#define SAA7134_H_PHASE_OFF_LUMA(t)             (0x2E|t)
#define SAA7134_H_PHASE_OFF_CHROMA(t)           (0x2F|t)
#define SAA7134_V_SCALE_RATIO(t)                (0x30|t)
#define SAA7134_V_SCALE_RATIO_HIBYTE(t)         (0x31|t)
#define SAA7134_V_FILTER(t)                     (0x32|t)
#define     SAA7134_V_FILTER_MIRROR                 (1<<1)
        //  something's here, don't know what   (0x33|t)
#define SAA7134_V_PHASE_OFFSET0(t)              (0x34|t)
#define SAA7134_V_PHASE_OFFSET1(t)              (0x35|t)
#define SAA7134_V_PHASE_OFFSET2(t)              (0x36|t)
#define SAA7134_V_PHASE_OFFSET3(t)              (0x37|t)

// clipping & dma
#define SAA7134_OFMT_VIDEO_A                    0x300 // formats
#define SAA7134_OFMT_DATA_A                     0x301
#define SAA7134_OFMT_VIDEO_B                    0x302
#define SAA7134_OFMT_DATA_B                     0x303
#define SAA7134_ALPHA_NOCLIP                    0x304
#define SAA7134_ALPHA_CLIP                      0x305
#define SAA7134_UV_PIXEL                        0x308
#define SAA7134_CLIP_RED                        0x309
#define SAA7134_CLIP_GREEN                      0x30a
#define SAA7134_CLIP_BLUE                       0x30b

// added, unverified
// Clips points, n = 0..16
#define SAA7134_CLIP_H_ACTIVE(n)                (0x380|(n)<<8)
#define SAA7134_CLIP_H_NOIDEA(n)                (0x381|(n)<<8)
#define SAA7134_CLIP_H_POS(n)                   (0x382|(n)<<8)
#define SAA7134_CLIP_H_POS_HIBYTE(n)            (0x383|(n)<<8)
#define SAA7134_CLIP_V_ACTIVE(n)                (0x384|(n)<<8)
#define SAA7134_CLIP_V_NOIDEA(n)                (0x385|(n)<<8)
#define SAA7134_CLIP_V_POS(n)                   (0x386|(n)<<8)
#define SAA7134_CLIP_V_POS_HIBYTE(n)            (0x387|(n)<<8)


// audio
#define SAA7134_NICAM_ADD_DATA1                 0x140
#define SAA7134_NICAM_ADD_DATA2                 0x141
#define SAA7134_NICAM_STATUS                    0x142
#define     SAA7134_NICAM_STATUS_LANG2              (1<<0)
#define     SAA7134_NICAM_STATUS_STEREO             (1<<1)
#define     SAA7134_NICAM_STATUS_MONO               (1<<3)
#define SAA7134_AUDIO_STATUS                    0x143
#define SAA7134_NICAM_ERROR_COUNT               0x144
#define SAA7134_IDENT_SIF                       0x145
#define     SAA7134_IDENT_SIF_MONO                  (1<<5)
#define     SAA7134_IDENT_SIF_LANG2                 (1<<6)
#define     SAA7134_IDENT_SIF_STEREO                (1<<7)
#define SAA7134_LEVEL_READOUT                   0x146
#define SAA7134_LEVEL_READOUT_HIBYTE            0x147
#define SAA7134_NICAM_ERROR_LOW                 0x148
#define SAA7134_NICAM_ERROR_HIGH                0x149
#define SAA7134_DCXO_IDENT_CTRL                 0x14a
#define SAA7134_DEMODULATOR                     0x14b
#define SAA7134_AGC_GAIN_SELECT                 0x14c
#define SAA7134_CARRIER1_FREQ                   0x150
#define SAA7134_CARRIER1_FREQ_HIWORD            0x152
#define SAA7134_CARRIER2_FREQ                   0x154
#define SAA7134_CARRIER2_FREQ_HIWORD            0x156
#define SAA7134_NUM_SAMPLES_0                   0x158 // 24 bits
#define SAA7134_NUM_SAMPLES_1                   0x159
#define SAA7134_NUM_SAMPLES_2                   0x15A
#define SAA7134_AUDIO_FORMAT_CTRL               0x15B
#define SAA7134_MONITOR_SELECT                  0x160
#define SAA7134_FM_DEEMPHASIS                   0x161
#define SAA7134_FM_DEMATRIX                     0x162
#define SAA7134_CHANNEL_LEVEL_L                 0x163 // -8..7, 0 = normal
#define SAA7134_CHANNEL_LEVEL_R                 0x164 // -8..7, 0 = normal
#define SAA7134_NICAM_CONFIG                    0x165
#define SAA7134_NICAM_LEVEL_ADJUST              0x166
#define SAA7134_STEREO_DAC_OUTPUT_SELECT        0x167
#define SAA7134_I2S_OUTPUT_FORMAT               0x168
#define SAA7134_I2S_OUTPUT_SELECT               0x169
#define SAA7134_I2S_OUTPUT_LEVEL                0x16A
#define SAA7134_DSP_OUTPUT_SELECT               0x16B
#define SAA7134_AUDIO_MUTE_CTRL                 0x16C
#define SAA7134_SIF_SAMPLE_FREQ                 0x16D
#define SAA7134_ANALOG_IO_SELECT                0x16E
#define SAA7134_AUDIO_CLOCK_0                   0x170 // 24 bits
#define SAA7134_AUDIO_CLOCK_1                   0x171
#define SAA7134_AUDIO_CLOCK_2                   0x172
#define SAA7134_AUDIO_PLL_CTRL                  0x173
#define SAA7134_AUDIO_CLOCKS_PER_FIELD0         0x174
#define SAA7134_AUDIO_CLOCKS_PER_FIELD1         0x175
#define SAA7134_AUDIO_CLOCKS_PER_FIELD2         0x176

// i2c bus
#define SAA7134_I2C_ATTR_STATUS                 0x180
#define SAA7134_I2C_DATA                        0x181
#define SAA7134_I2C_CLOCK_SELECT                0x182
#define SAA7134_I2C_TIMER                       0x183

// video port output
#define SAA7134_VIDEO_PORT_CTRL0                0x190
#define SAA7134_VIDEO_PORT_CTRL1                0x191
#define SAA7134_VIDEO_PORT_CTRL2                0x192
#define SAA7134_VIDEO_PORT_CTRL3                0x193
#define SAA7134_VIDEO_PORT_CTRL4                0x194
#define SAA7134_VIDEO_PORT_CTRL5                0x195
#define SAA7134_VIDEO_PORT_CTRL6                0x196
#define SAA7134_VIDEO_PORT_CTRL7                0x197
#define SAA7134_VIDEO_PORT_CTRL8                0x198

// transport stream interface
#define SAA7134_TS_PARALLEL                     0x1A0
#define SAA7134_TS_PARALLEL_SERIAL              0x1A1
#define SAA7134_TS_SERIAL0                      0x1A2
#define SAA7134_TS_SERIAL1                      0x1A3
#define SAA7134_TS_DMA0                         0x1A4
#define SAA7134_TS_DMA1                         0x1A5
#define SAA7134_TS_DMA2                         0x1A6

// GPIO Controls
#define SAA7134_GPIO_GPRESCAN                   0x80
#define SAA7134_GPIO_27_25                      0x0E

#define SAA7134_GPIO_GPMODE0                    0x1B0
#define SAA7134_GPIO_GPMODE1                    0x1B1
#define SAA7134_GPIO_GPMODE2                    0x1B2
#define SAA7134_GPIO_GPMODE3                    0x1B3
#define SAA7134_GPIO_GPSTATUS0                  0x1B4
#define SAA7134_GPIO_GPSTATUS1                  0x1B5
#define SAA7134_GPIO_GPSTATUS2                  0x1B6
#define SAA7134_GPIO_GPSTATUS3                  0x1B7

// I2S output
#define SAA7134_I2S_AUDIO_OUTPUT                0x1C0

// test modes
#define SAA7134_SPECIAL_MODE                    0x1D0


class ISAA7134_Defines
{
protected:

    enum eAudioInputLine
    {
        /// standard tuner line
        AUDIOINPUTLINE_TUNER = 0,
        /// internal line 1 input
        AUDIOINPUTLINE_LINE1,
        /// internal line 2 input
        AUDIOINPUTLINE_LINE2,
    };


    enum eFIRType
    {
        FIR_BG_DK_NICAM,
        FIR_I_NICAM,
        FIR_L_NICAM,
        FIR_BG_DK_DUAL_FM,
        FIR_M_N_DUAL_FM,
        FIR_SAT_DUAL_FM,
    };

    enum eAudioMonoType
    {
        MONO_FM,
        MONO_AM,
    };

    enum eAudioStereoType
    {
        STEREO_NONE,
        STEREO_FM,
        STEREO_NICAM,
        STEREO_BTSC,
        STEREO_MONO_SAP,
        STEREO_SAT,
        STEREO_ADR,
    };

    enum eAudioStandard
    {
//      AUDIOSTANDARD_AUTODETECT = -1,  need to fix menu
        AUDIOSTANDARD_BG_DUAL_FM = 0,
        AUDIOSTANDARD_DK1_DUAL_FM,         
        AUDIOSTANDARD_DK2_DUAL_FM,         
        AUDIOSTANDARD_DK3_DUAL_FM,         
        AUDIOSTANDARD_BG_NICAM_FM,         
        AUDIOSTANDARD_I_NICAM_FM,          
        AUDIOSTANDARD_DK_NICAM_FM,
        AUDIOSTANDARD_L_NICAM_AM,
        AUDIOSTANDARD_NTCS_MONO,
        AUDIOSTANDARD_M_DUAL_FM,
        AUDIOSTANDARD_LAST_ONE
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

};


#endif