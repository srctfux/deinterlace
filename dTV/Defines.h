/////////////////////////////////////////////////////////////////////////////
// defines.h
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
//
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Original Release
//                                     Translated most code from German
//                                     Combined Header files
//                                     Cut out all decoding
//                                     Cut out digital hardware stuff
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __DEFINES_H___
#define __DEFINES_H___

#include <windows.h>			/* required for all Windows applications */
#include <stdio.h>
#include <time.h>
#include "resource.h"
#include <stdlib.h>				/* atoi                                  */
#include <memory.h>
#include <io.h>
#include <fcntl.h>
#include <commctrl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ddraw.h>
#include <process.h>			/* for _beginthread                      */
#include <math.h>
#include <mmsystem.h>
#include <winioctl.h>
#include "dTVdrv.h"



// originally from bt848.h

typedef enum
{
    BT8X8_MODE_BYTE   = 0,
    BT8X8_MODE_WORD   = 1,
    BT8X8_MODE_DWORD  = 2
} BT8X8_MODE;


typedef enum
{
    BT8X8_AD_BAR0 = 0,
    BT8X8_AD_BAR1 = 1,
    BT8X8_AD_BAR2 = 2,
    BT8X8_AD_BAR3 = 3,
    BT8X8_AD_BAR4 = 4,
    BT8X8_AD_BAR5 = 5,
    BT8X8_AD_EPROM = 6,
    BT8X8_ITEMS = 7
} BT8X8_ADDR;

// options for BT8X8_Open
enum { BT8X8_OPEN_USE_INT =   0x1 };

enum {
    PCI_IDR  = 0x00,
    PCI_CR   = 0x04,
    PCI_SR   = 0x06,
    PCI_REV  = 0x08,
    PCI_CCR  = 0x09,
    PCI_LSR  = 0x0c,
    PCI_LTR  = 0x0d,
    PCI_HTR  = 0x0e,
    PCI_BISTR= 0x0f,
    PCI_BAR0 = 0x10,
    PCI_BAR1 = 0x14,
    PCI_BAR2 = 0x18,
    PCI_BAR3 = 0x1c,
    PCI_BAR4 = 0x20,
    PCI_BAR5 = 0x24,
    PCI_CIS  = 0x28,
    PCI_SVID = 0x2c,
    PCI_SID  = 0x2e,
    PCI_ERBAR= 0x30,
    PCI_ILR  = 0x3c,
    PCI_IPR  = 0x3d,
    PCI_MGR  = 0x3e,
    PCI_MLR  = 0x3f
};


typedef enum
{
    SAA7146_MODE_BYTE   = 0,
    SAA7146_MODE_WORD   = 1,
    SAA7146_MODE_DWORD  = 2
} SAA7146_MODE;


typedef enum
{
    SAA7146_AD_BAR0 = 0,
    SAA7146_AD_BAR1 = 1,
    SAA7146_AD_BAR2 = 2,
    SAA7146_AD_BAR3 = 3,
    SAA7146_AD_BAR4 = 4,
    SAA7146_AD_BAR5 = 5,
    SAA7146_AD_EPROM = 6,
    SAA7146_ITEMS = 7
} SAA7146_ADDR;

// options for SAA7146_Open
enum { SAA7146_OPEN_USE_INT =   0x1 };

typedef enum  {	WCreate,
                WDestroy,
                WMoveD,
                WMoveA,
		    	WHide,
                WTop,
                DBox,
                DLine,
		    	DText,
		    	Set_Font,
                SetColor,
		    	SetBlend,
		    	SetWBlend,
		    	SetCBlend,
		    	SetNonBlend,
                LoadBmp,
				BlitBmp,
				ReleaseBmp
              } OSDCOM;

typedef enum OSDPALTYPE
{
	NoPalet =  0,      /* No palette */
	Pal1Bit =  2,      /* 2 colors for 1 Bit Palette    */
	Pal2Bit =  4,      /* 4 colors for 2 bit palette    */
	Pal4Bit =  16,     /* 16 colors for 4 bit palette   */
	Pal8Bit =  256     /* 256 colors for 16 bit palette */
} OSDPALTYPE, *POSDPALTYPE;

typedef enum DISPTYPE
{
	BITMAP1,                  /* 1 bit bitmap */
	BITMAP2,                  /* 2 bit bitmap */
	BITMAP4,                  /* 4 bit bitmap */
	BITMAP8,                  /* 8 bit bitmap */
//	BITMAP1HR,                /* 1 Bit bitmap half resolution */
//	BITMAP2HR,                /* 2 bit bitmap half resolution */
//	BITMAP4HR,                /* 4 bit bitmap half resolution */
//	BITMAP8HR,                /* 8 bit bitmap half resolution */
//	YCRCB422,                 /* 4:2:2 YCRCB Graphic Display */
//	YCRCB444,                 /* 4:4:4 YCRCB Graphic Display */
//	YCRCB444HR,               /* 4:4:4 YCRCB graphic half resolution */
//	VIDEO_MPEG,               /* Full Size MPEG Video Display */
//	VIDEOHSIZE,               /* MPEG Video Display Half Resolution */
//	VIDEOQSIZE,               /* MPEG Video Display Quarter Resolution */
//	VIDEOESIZE,               /* MPEG Video Display Quarter Resolution */
//	CURSOR                    /* Cursor */
} DISPTYPE, *PDISPTYPE;

typedef enum  { TTX_Disable_Capture,
                TTX_Capture_Parity_Corrected,
				TTX_Capture_Hamming_8_4_Corrected,
				TTX_Capture_8Bit_Data
} TTX_Capture_Types;



typedef enum  { TTXEnable,
		    	TTXDisable,
		    	ModifyTable,
		    	ReqestP29_P830,
                NoPageMemory,
                NoPESMemory, 
                PESPacket,
                TTXPage,
                P29_P830,
                DroppedPages
              } TTXCOM;

typedef enum  { AudioState,
		    	AudioBuffState,
		    	VideoState1,
		    	VideoState2,
		    	VideoState3,
		    	CrashCounter,
				ReqVersion,
				ReqVCXO,
				ReqRegister
			  } REQCOM;

typedef enum  { SetVidMode,
			    SetTestMode,
			    LoadVidCode,
				SetMonitorType
			  } ENC;

typedef enum  { IncCrashCounter,
		    	dummy
			  } SYSTEM;

typedef enum  
{
	__Record,
	__Stop,
	__Play,
	__Pause,
	__Slow,
	__FF_IP,
	__Scan_I
} REC_PLAY;

typedef enum
{ 
	COMTYPE_NOCOM,		//0
	COMTYPE_TUNER,
	COMTYPE_DEMODULATOR,	//2
	COMTYPE_DESCRAMBLE,
	COMTYPE_PIDFILTER,	//4
	COMTYPE_MPEGDECODER,
	COMTYPE_OSD,		//6
	COMTYPE_BMP,
	COMTYPE_ENCODER,		//8
	COMTYPE_CONFACCESS,
	COMTYPE_I2C,		//a
	COMTYPE_AUDIODAC,
	COMTYPE_TELETEXT,		//c
	COMTYPE_REQUEST,
	COMTYPE_SYSTEM,		//e
	COMTYPE_REC_PLAY,
	COMTYPE_COMMON_IF,		//10
	COMTYPE_PID_FILTER
} COMTYPE;

typedef enum  
{ 
	VideoPID,
	WaitVideoPlay,
	WaitUnMute,
	InitFilt,
	AddFilt8,
	AddFilt16,
	DeleteFilt,
	FiltError,
	RecSection,
	NewVersion,
	CacheError,
	SetFiltBytes,
	SetRange,
	AddSecFilt,
	AddRangeFilt,
	AddFilt12,
	MultiPID,
	AddPIDFilter,
	DelPIDFilter,
	TestSec
} PIDCOM;



typedef enum  
{
	AudioDAC,
	CabADAC,
	ON22K,
	OFF22K,
	MainSwitch,
	ADSwitch,
	SendDiSEqC,
	OFDM_Channel,
	OFDM_Guard,
	OFDM_MpegBer,
	SetRegister,
	SetIrControl,
	LoadTTuner
} AUDCOM;


typedef enum QAM_TYPE
{	QAM_16,
	QAM_32,
	QAM_64,
	QAM_128,
	QAM_256
} QAM_TYPE, *PQAM_TYPE;

typedef enum GPIO_MODE
{
	GPIO_INPUT = 0x00,
	GPIO_IRQHI = 0x10,
	GPIO_IRQLO = 0x20,
	GPIO_IRQHL = 0x30,
	GPIO_OUTLO = 0x40,
	GPIO_OUTHI = 0x50
} GPIO_MODE;

typedef enum
{
	VIDEO_MODE = 0,
	FILM_22_PULLDOWN_ODD,
	FILM_22_PULLDOWN_EVEN,
	FILM_32_PULLDOWN_0,
	FILM_32_PULLDOWN_1,
	FILM_32_PULLDOWN_2,
	FILM_32_PULLDOWN_3,
	FILM_32_PULLDOWN_4,
	LAST_ONE
} ePULLDOWNMODES;

#define MAXVTDIALOG 8
#define MAXPROGS 4096
#define VBI_DATA_SIZE 2048*39
#define VBI_VT  1
#define VBI_VPS 2
#define VBI_IC  8
#define VBI_VD  16
#define VBI_WINBIS  32
#define FPSHIFT 16
#define FPFAC (1<<FPSHIFT)

/* Brooktree 848 registers */

#define BT848_DSTATUS          0x000
#define BT848_DSTATUS_PRES     (1<<7)
#define BT848_DSTATUS_HLOC     (1<<6)
#define BT848_DSTATUS_FIELD    (1<<5)
#define BT848_DSTATUS_NUML     (1<<4)
#define BT848_DSTATUS_CSEL     (1<<3)
#define BT848_DSTATUS_LOF      (1<<1)
#define BT848_DSTATUS_COF      (1<<0)

#define BT848_IFORM            0x004  
#define BT848_IFORM_HACTIVE    (1<<7)
#define BT848_IFORM_MUXSEL     (3<<5)
#define BT848_IFORM_MUX0       (2<<5)
#define BT848_IFORM_MUX1       (3<<5)
#define BT848_IFORM_MUX2       (1<<5)
#define BT848_IFORM_XTSEL      (3<<3)
#define BT848_IFORM_XT0        (1<<3)
#define BT848_IFORM_XT1        (2<<3)
#define BT848_IFORM_XTAUTO     (3<<3)
#define BT848_IFORM_XTBOTH     (3<<3)
#define BT848_IFORM_NTSC       1
#define BT848_IFORM_NTSC_JAP   2
#define BT848_IFORM_PAL_BDGHI  3
#define BT848_IFORM_PAL_M      4
#define BT848_IFORM_PAL_N      5
#define BT848_IFORM_SECAM      6
#define BT848_IFORM_AUTO       0
#define BT848_IFORM_NORM       7


#define BT848_FCNTR            0x0E8
#define BT848_PLL_F_LO         0x0F0
#define BT848_PLL_F_HI         0x0F4
#define BT848_PLL_XCI          0x0F8

#define BT848_TGCTRL           0x084  
#define BT848_TGCTRL_TGCKI_PLL 0x08
#define BT848_TGCTRL_TGCKI_NOPLL 0x00

#define BT848_TDEC             0x008  
#define BT848_TDEC_DEC_FIELD   (1<<7)
#define BT848_TDEC_FLDALIGN    (1<<6)
#define BT848_TDEC_DEC_RAT     (0x1f)

#define BT848_E_CROP           0x00C
#define BT848_O_CROP           0x08C

#define BT848_E_VDELAY_LO      0x010
#define BT848_O_VDELAY_LO      0x090

#define BT848_E_VACTIVE_LO     0x014
#define BT848_O_VACTIVE_LO     0x094

#define BT848_E_HDELAY_LO      0x018
#define BT848_O_HDELAY_LO      0x098

#define BT848_E_HACTIVE_LO     0x01C
#define BT848_O_HACTIVE_LO     0x09C

#define BT848_E_HSCALE_HI      0x020
#define BT848_O_HSCALE_HI      0x0A0

#define BT848_E_HSCALE_LO      0x024
#define BT848_O_HSCALE_LO      0x0A4

#define BT848_BRIGHT           0x028

#define BT848_E_CONTROL        0x02C
#define BT848_O_CONTROL        0x0AC
#define BT848_CONTROL_LNOTCH    (1<<7)
#define BT848_CONTROL_COMP      (1<<6)
#define BT848_CONTROL_LDEC      (1<<5)
#define BT848_CONTROL_CBSENSE   (1<<4)
#define BT848_CONTROL_CON_MSB   (1<<2)
#define BT848_CONTROL_SAT_U_MSB (1<<1)
#define BT848_CONTROL_SAT_V_MSB (1<<0)

#define BT848_CONTRAST_LO      0x030
#define BT848_SAT_U_LO         0x034
#define BT848_SAT_V_LO         0x038
#define BT848_HUE              0x03C

#define BT848_E_SCLOOP         0x040
#define BT848_O_SCLOOP         0x0C0
#define BT848_SCLOOP_LUMA_PEAK  (1<<7)
#define BT848_SCLOOP_CAGC       (1<<6)
#define BT848_SCLOOP_CKILL      (1<<5)
#define BT848_SCLOOP_HFILT_AUTO (0<<3)
#define BT848_SCLOOP_HFILT_CIF  (1<<3)
#define BT848_SCLOOP_HFILT_QCIF (2<<3)
#define BT848_SCLOOP_HFILT_ICON (3<<3)

#define BT848_WC_UP            0x044
#define BT848_VTOTAL_LO        0x0B0
#define BT848_VTOTAL_HI        0x0B4
#define BT848_DVSIF            0x0FC

#define BT848_OFORM            0x048
#define BT848_OFORM_RANGE      (1<<7)
#define BT848_OFORM_CORE0      (0<<5)
#define BT848_OFORM_CORE8      (1<<5)
#define BT848_OFORM_CORE16     (2<<5)
#define BT848_OFORM_CORE32     (3<<5)

#define BT848_E_VSCALE_HI      0x04C
#define BT848_O_VSCALE_HI      0x0CC
#define BT848_VSCALE_YCOMB     (1<<7)
#define BT848_VSCALE_COMB      (1<<6)
#define BT848_VSCALE_INT       (1<<5)
#define BT848_VSCALE_HI        15

#define BT848_E_VSCALE_LO      0x050
#define BT848_O_VSCALE_LO      0x0D0
#define BT848_TEST             0x054
#define BT848_ADELAY           0x060
#define BT848_BDELAY           0x064

#define BT848_ADC              0x068
#define BT848_ADC_RESERVED     (2<<6)
#define BT848_ADC_SYNC_T       (1<<5)
#define BT848_ADC_AGC_EN       (1<<4)
#define BT848_ADC_CLK_SLEEP    (1<<3)
#define BT848_ADC_Y_SLEEP      (1<<2)
#define BT848_ADC_C_SLEEP      (1<<1)
#define BT848_ADC_CRUSH        (1<<0)

#define BT848_E_VTC            0x06C
#define BT848_O_VTC            0x0EC
#define BT848_VTC_HSFMT        (1<<7)
#define BT848_VTC_VFILT_2TAP   0
#define BT848_VTC_VFILT_3TAP   1
#define BT848_VTC_VFILT_4TAP   2
#define BT848_VTC_VFILT_5TAP   3

#define BT848_SRESET           0x07C

#define BT848_COLOR_FMT             0x0D4
#define BT848_COLOR_FMT_O_RGB32     (0<<4)
#define BT848_COLOR_FMT_O_RGB24     (1<<4)
#define BT848_COLOR_FMT_O_RGB16     (2<<4)
#define BT848_COLOR_FMT_O_RGB15     (3<<4)
#define BT848_COLOR_FMT_O_YUY2      (4<<4)
#define BT848_COLOR_FMT_O_BtYUV     (5<<4)
#define BT848_COLOR_FMT_O_Y8        (6<<4)
#define BT848_COLOR_FMT_O_RGB8      (7<<4)
#define BT848_COLOR_FMT_O_YCrCb422  (8<<4)
#define BT848_COLOR_FMT_O_YCrCb411  (9<<4)
#define BT848_COLOR_FMT_O_RAW       (14<<4)
#define BT848_COLOR_FMT_E_RGB32     0
#define BT848_COLOR_FMT_E_RGB24     1
#define BT848_COLOR_FMT_E_RGB16     2
#define BT848_COLOR_FMT_E_RGB15     3
#define BT848_COLOR_FMT_E_YUY2      4
#define BT848_COLOR_FMT_E_BtYUV     5
#define BT848_COLOR_FMT_E_Y8        6
#define BT848_COLOR_FMT_E_RGB8      7
#define BT848_COLOR_FMT_E_YCrCb422  8
#define BT848_COLOR_FMT_E_YCrCb411  9
#define BT848_COLOR_FMT_E_RAW       14

#define BT848_COLOR_FMT_RGB32       0x00
#define BT848_COLOR_FMT_RGB24       0x11
#define BT848_COLOR_FMT_RGB16       0x22
#define BT848_COLOR_FMT_RGB15       0x33
#define BT848_COLOR_FMT_YUY2        0x44
#define BT848_COLOR_FMT_BtYUV       0x55
#define BT848_COLOR_FMT_Y8          0x66
#define BT848_COLOR_FMT_RGB8        0x77
#define BT848_COLOR_FMT_YCrCb422    0x88
#define BT848_COLOR_FMT_YCrCb411    0x99
#define BT848_COLOR_FMT_RAW         0xee

#define BT848_COLOR_CTL                0x0D8
#define BT848_COLOR_CTL_EXT_FRMRATE    (1<<7)
#define BT848_COLOR_CTL_COLOR_BARS     (1<<6)
#define BT848_COLOR_CTL_RGB_DED        (1<<5)
#define BT848_COLOR_CTL_GAMMA          (1<<4)
#define BT848_COLOR_CTL_WSWAP_ODD      (1<<3)
#define BT848_COLOR_CTL_WSWAP_EVEN     (1<<2)
#define BT848_COLOR_CTL_BSWAP_ODD      (1<<1)
#define BT848_COLOR_CTL_BSWAP_EVEN     (1<<0)

#define BT848_CAP_CTL                  0x0DC
#define BT848_CAP_CTL_DITH_FRAME       (1<<4)
#define BT848_CAP_CTL_CAPTURE_VBI_ODD  (1<<3)
#define BT848_CAP_CTL_CAPTURE_VBI_EVEN (1<<2)
#define BT848_CAP_CTL_CAPTURE_ODD      (1<<1)
#define BT848_CAP_CTL_CAPTURE_EVEN     (1<<0)

#define BT848_VBI_PACK_SIZE    0x0E0

#define BT848_VBI_PACK_DEL     0x0E4
#define BT848_VBI_PACK_DEL_VBI_HDELAY 0xfc
#define BT848_VBI_PACK_DEL_EXT_FRAME  2
#define BT848_VBI_PACK_DEL_VBI_PKT_HI 1

#define BT848_INT_STAT         0x100
#define BT848_INT_MASK         0x104

#define BT848_INT_ETBF         (1<<23)

#define BT848_INT_RISCS   (0xf<<28)
#define BT848_INT_RISC_EN (1<<27)
#define BT848_INT_RACK    (1<<25)
#define BT848_INT_FIELD   (1<<24)
#define BT848_INT_SCERR   (1<<19)
#define BT848_INT_OCERR   (1<<18)
#define BT848_INT_PABORT  (1<<17)
#define BT848_INT_RIPERR  (1<<16)
#define BT848_INT_PPERR   (1<<15)
#define BT848_INT_FDSR    (1<<14)
#define BT848_INT_FTRGT   (1<<13)
#define BT848_INT_FBUS    (1<<12)
#define BT848_INT_RISCI   (1<<11)
#define BT848_INT_GPINT   (1<<9)
#define BT848_INT_I2CDONE (1<<8)
#define BT848_INT_VPRES   (1<<5)
#define BT848_INT_HLOCK   (1<<4)
#define BT848_INT_OFLOW   (1<<3)
#define BT848_INT_HSYNC   (1<<2)
#define BT848_INT_VSYNC   (1<<1)
#define BT848_INT_FMTCHG  (1<<0)


#define BT848_GPIO_DMA_CTL             0x10C
#define BT848_GPIO_DMA_CTL_GPINTC      (1<<15)
#define BT848_GPIO_DMA_CTL_GPINTI      (1<<14)
#define BT848_GPIO_DMA_CTL_GPWEC       (1<<13)
#define BT848_GPIO_DMA_CTL_GPIOMODE    (3<<11)
#define BT848_GPIO_DMA_CTL_GPCLKMODE   (1<<10)
#define BT848_GPIO_DMA_CTL_PLTP23_4    (0<<6)
#define BT848_GPIO_DMA_CTL_PLTP23_8    (1<<6)
#define BT848_GPIO_DMA_CTL_PLTP23_16   (2<<6)
#define BT848_GPIO_DMA_CTL_PLTP23_32   (3<<6)
#define BT848_GPIO_DMA_CTL_PLTP1_4     (0<<4)
#define BT848_GPIO_DMA_CTL_PLTP1_8     (1<<4)
#define BT848_GPIO_DMA_CTL_PLTP1_16    (2<<4)
#define BT848_GPIO_DMA_CTL_PLTP1_32    (3<<4)
#define BT848_GPIO_DMA_CTL_PKTP_4      (0<<2)
#define BT848_GPIO_DMA_CTL_PKTP_8      (1<<2)
#define BT848_GPIO_DMA_CTL_PKTP_16     (2<<2)
#define BT848_GPIO_DMA_CTL_PKTP_32     (3<<2)
#define BT848_GPIO_DMA_CTL_RISC_ENABLE (1<<1)
#define BT848_GPIO_DMA_CTL_FIFO_ENABLE (1<<0)

#define BT848_I2C              0x110
#define BT848_I2C_DIV          (0xf<<4)
#define BT848_I2C_SYNC         (1<<3)
#define BT848_I2C_W3B	       (1<<2)
#define BT848_I2C_SCL          (1<<1)
#define BT848_I2C_SDA          (1<<0)


#define BT848_RISC_STRT_ADD    0x114
#define BT848_GPIO_OUT_EN      0x118
#define BT848_GPIO_OUT_EN_HIBYTE      0x11A
#define BT848_GPIO_REG_INP     0x11C
#define BT848_GPIO_REG_INP_HIBYTE     0x11E
#define BT848_RISC_COUNT       0x120
#define BT848_GPIO_DATA        0x200
#define BT848_GPIO_DATA_HIBYTE 0x202


/* Bt848 RISC commands */

/* only for the SYNC RISC command */
#define BT848_FIFO_STATUS_FM1  0x06
#define BT848_FIFO_STATUS_FM3  0x0e
#define BT848_FIFO_STATUS_SOL  0x02
#define BT848_FIFO_STATUS_EOL4 0x01
#define BT848_FIFO_STATUS_EOL3 0x0d
#define BT848_FIFO_STATUS_EOL2 0x09
#define BT848_FIFO_STATUS_EOL1 0x05
#define BT848_FIFO_STATUS_VRE  0x04
#define BT848_FIFO_STATUS_VRO  0x0c
#define BT848_FIFO_STATUS_PXV  0x00

#define BT848_RISC_RESYNC      (1<<15)

/* WRITE and SKIP */
/* disable which bytes of each DWORD */
#define BT848_RISC_BYTE0       (1<<12)
#define BT848_RISC_BYTE1       (1<<13)
#define BT848_RISC_BYTE2       (1<<14)
#define BT848_RISC_BYTE3       (1<<15)
#define BT848_RISC_BYTE_ALL    (0x0f<<12)
#define BT848_RISC_BYTE_NONE   0
/* cause RISCI */
#define BT848_RISC_IRQ         (1<<24)
/* RISC command is last one in this line */
#define BT848_RISC_EOL         (1<<26)
/* RISC command is first one in this line */
#define BT848_RISC_SOL         (1<<27)

#define BT848_RISC_WRITE       (0x01<<28)
#define BT848_RISC_SKIP        (0x02<<28)
#define BT848_RISC_WRITEC      (0x05<<28)
#define BT848_RISC_JUMP        (0x07<<28)
#define BT848_RISC_SYNC        (0x08<<28)

#define BT848_RISC_WRITE123    (0x09<<28)
#define BT848_RISC_SKIP123     (0x0a<<28)
#define BT848_RISC_WRITE1S23   (0x0b<<28)


// MSP34x0 definitions
#define MSP_CONTROL 0x00 // Software reset
#define MSP_TEST	0x01 // Internal use
#define MSP_WR_DEM	0x10 // Write demodulator
#define MSP_RD_DEM	0x11 // Read demodulator
#define MSP_WR_DSP	0x12 // Write DSP
#define MSP_RD_DSP	0x13 // Read DSP

#define I2C_DELAY 0
#define I2C_TIMING (0x7<<4)
#define I2C_COMMAND (I2C_TIMING | BT848_I2C_SCL | BT848_I2C_SDA)

#define VBI_SPL 2044
#define FLEN 50

#define MSP_MODE_AM_DETECT   0
#define MSP_MODE_FM_RADIO    2
#define MSP_MODE_FM_TERRA    3
#define MSP_MODE_FM_SAT      4
#define MSP_MODE_FM_NICAM1   5
#define MSP_MODE_FM_NICAM2   6

#define VIDEO_SOUND_MONO	1
#define VIDEO_SOUND_STEREO	2
#define VIDEO_SOUND_LANG1	3
#define VIDEO_SOUND_LANG2	4


#define CLEARLINES 288

#define ID_STATUSBAR    1700
#define ID_TEXTFIELD       ID_STATUSBAR+1
#define ID_KENNUNGFFIELD   ID_STATUSBAR+2
#define ID_CODEFIELD       ID_STATUSBAR+3
#define ID_FPSFIELD        ID_STATUSBAR+4
#define ID_AUDIOFIELD      ID_STATUSBAR+5

#define INIT_BT              1800
#define RESET_LIST           1802

#define MAXVTDIALOG 8

#define MAXPROGS 4096

#define NEUSIZE 128

#define EIT_INDEX_SIZE 32768

#define MAXFAVORITEN 128

#define MAXNIT 512
#define NEUNIT 96
#define MAXFILTER 64

#define FLEN 50

// Nagravision Stuff 
#define TESTPIXEL             32
#define ZEILENZAHL           287

#define            FORMAT_4_3               0x08
#define            FORMAT_14_9              0x01
#define            FORMAT_16_9              0x0B

#define TIMERANZAHL 12

#define    RP_None               0x00
#define    AudioPES              0x01
#define    AudioMp2              0x02
#define    AudioPCM              0x03
#define    VideoPES              0x04
#define    AV_PES                0x05

#define HILO(x) (x##_hi << 8 | x##_lo)

#define WriteDem(wAddr,wData) Audio_WriteMSP(MSP_WR_DEM,wAddr,wData) // I2C_MSP3400C_DEM
#define WriteDSP(wAddr,wData) Audio_WriteMSP(MSP_WR_DSP,wAddr,wData) // I2C_MSP3400C_DEM
#define ReadDem(wAddr) Audio_ReadMSP(MSP_RD_DEM,wAddr)  // I2C_MSP3400C_DFP
#define ReadDSP(wAddr) Audio_ReadMSP(MSP_RD_DSP,wAddr)  // I2C_MSP3400C_DFP

/* This macro is allowed for *constants* only, gcc must calculate it
   at compile time.  Remember -- no floats in kernel mode */
#define MSP_CARRIER(freq) ((int)((float)(freq/18.432)*(1<<24)))


#define get_descr(x) (((descr_gen_t *) x)->descriptor_tag)
#define get_descr_len(x) (((descr_gen_t *) x)->descriptor_length)

#define I2C_M_TEN	0x10	/* we have a ten bit chip address	*/
#define I2C_M_RD	0x01
#if 0
#define I2C_M_PROBE	0x20
#endif

#define DVB_S  0
#define DVB_C  1

/* Values that need to be changed for PAL or NTSC systems */
/* Example for NTSC: insmod saa7146 mode=1 */

/* Number of vertical active lines */
#define V_ACTIVE_LINES_PAL	576
#define V_ACTIVE_LINES_NTSC	480

/* Number of lines in a field for HPS to process */
#define V_FIELD_PAL	288
#define V_FIELD_NTSC	240

/* Number of lines of vertical offset before processing */
#define V_OFFSET_PAL	0x15
#define V_OFFSET_NTSC	0x16

/* Number of horizontal pixels to process */
#define H_PIXELS_PAL	720
#define H_PIXELS_NTSC	640

/* Horizontal offset of processing window */
#define H_OFFSET_PAL	0x40
#define H_OFFSET_NTSC	0x06

/************************************************************************/
/* UNSORTED								*/
/************************************************************************/

#define ME1    0x0000000800
#define PV1    0x0000000008

/************************************************************************/
/* I2C									*/
/************************************************************************/

/* time we wait after certain i2c-operations */
#define SAA7146_I2C_DELAY 		10

#define	SAA7146_IICSTA		0x090
#define	SAA7146_I2C_ABORT	(1<<7)
#define	SAA7146_I2C_SPERR	(1<<6)
#define	SAA7146_I2C_APERR	(1<<5)
#define	SAA7146_I2C_DTERR	(1<<4)
#define	SAA7146_I2C_DRERR	(1<<3)
#define	SAA7146_I2C_AL		(1<<2)
#define	SAA7146_I2C_ERR		(1<<1)
#define	SAA7146_I2C_BUSY	(1<<0)

#define	SAA7146_IICTRF		0x08c
#define	SAA7146_I2C_START	(0x3)
#define	SAA7146_I2C_CONT	(0x2)
#define	SAA7146_I2C_STOP	(0x1)
#define	SAA7146_I2C_NOP		(0x0)

#define SAA7146_I2C_BUS_BIT_RATE_6400	(0x5<<8)
#define SAA7146_I2C_BUS_BIT_RATE_3200	(0x1<<8)
#define SAA7146_I2C_BUS_BIT_RATE_480	(0x4<<8)
#define SAA7146_I2C_BUS_BIT_RATE_320	(0x6<<8)
#define SAA7146_I2C_BUS_BIT_RATE_240	(0x7<<8)
#define SAA7146_I2C_BUS_BIT_RATE_120	(0x0<<8)
#define SAA7146_I2C_BUS_BIT_RATE_80	(0x2<<8)
#define SAA7146_I2C_BUS_BIT_RATE_60	(0x3<<8)

/************************************************************************/
/* CLIPPING								*/
/************************************************************************/

/* some defines for the various clipping-modes */
#define SAA7146_CLIPPING_RECT		0x4
#define SAA7146_CLIPPING_RECT_INVERTED	0x5
#define SAA7146_CLIPPING_MASK		0x6
#define SAA7146_CLIPPING_MASK_INVERTED	0x7

/************************************************************************/
/* RPS									*/
/************************************************************************/

#define CMD_NOP		0x00000000  /* No operation */
#define CMD_CLR_EVENT	0x00000000  /* Clear event */
#define CMD_SET_EVENT	0x10000000  /* Set signal event */
#define CMD_PAUSE	0x20000000  /* Pause */
#define CMD_CHECK_LATE	0x30000000  /* Check late */
#define CMD_UPLOAD	0x40000000  /* Upload */
#define CMD_STOP	0x50000000  /* Stop */
#define CMD_INTERRUPT	0x60000000  /* Interrupt */
#define CMD_JUMP	0x80000000  /* Jump */
#define CMD_WR_REG	0x90000000  /* Write (load) register */
#define CMD_RD_REG	0xa0000000  /* Read (store) register */
#define CMD_WR_REG_MASK	0xc0000000  /* Write register with mask */

/************************************************************************/
/* OUTPUT FORMATS 							*/
/************************************************************************/

/* output formats; each entry holds three types of information */
/* composed is used in the sense of "not-planar" */

#define RGB15_COMPOSED	0x213
/* this means: yuv2rgb-conversation-mode=2, dither=yes(=1), format-mode = 3 */
#define RGB16_COMPOSED	0x210
#define RGB24_COMPOSED	0x201
#define RGB32_COMPOSED	0x202

#define YUV411_COMPOSED		0x003
/* this means: yuv2rgb-conversation-mode=0, dither=no(=0), format-mode = 3 */
#define YUV422_COMPOSED		0x000
#define YUV411_DECOMPOSED	0x00b
#define YUV422_DECOMPOSED	0x009

/************************************************************************/
/* MISC 								*/
/************************************************************************/

/* Bit mask constants */
#define MASK_00   0x00000001    /* Mask value for bit 0 */
#define MASK_01   0x00000002    /* Mask value for bit 1 */
#define MASK_02   0x00000004    /* Mask value for bit 2 */
#define MASK_03   0x00000008    /* Mask value for bit 3 */
#define MASK_04   0x00000010    /* Mask value for bit 4 */
#define MASK_05   0x00000020    /* Mask value for bit 5 */
#define MASK_06   0x00000040    /* Mask value for bit 6 */
#define MASK_07   0x00000080    /* Mask value for bit 7 */
#define MASK_08   0x00000100    /* Mask value for bit 8 */
#define MASK_09   0x00000200    /* Mask value for bit 9 */
#define MASK_10   0x00000400    /* Mask value for bit 10 */
#define MASK_11   0x00000800    /* Mask value for bit 11 */
#define MASK_12   0x00001000    /* Mask value for bit 12 */
#define MASK_13   0x00002000    /* Mask value for bit 13 */
#define MASK_14   0x00004000    /* Mask value for bit 14 */
#define MASK_15   0x00008000    /* Mask value for bit 15 */
#define MASK_16   0x00010000    /* Mask value for bit 16 */
#define MASK_17   0x00020000    /* Mask value for bit 17 */
#define MASK_18   0x00040000    /* Mask value for bit 18 */
#define MASK_19   0x00080000    /* Mask value for bit 19 */
#define MASK_20   0x00100000    /* Mask value for bit 20 */
#define MASK_21   0x00200000    /* Mask value for bit 21 */
#define MASK_22   0x00400000    /* Mask value for bit 22 */
#define MASK_23   0x00800000    /* Mask value for bit 23 */
#define MASK_24   0x01000000    /* Mask value for bit 24 */
#define MASK_25   0x02000000    /* Mask value for bit 25 */
#define MASK_26   0x04000000    /* Mask value for bit 26 */
#define MASK_27   0x08000000    /* Mask value for bit 27 */
#define MASK_28   0x10000000    /* Mask value for bit 28 */
#define MASK_29   0x20000000    /* Mask value for bit 29 */
#define MASK_30   0x40000000    /* Mask value for bit 30 */
#define MASK_31   0x80000000    /* Mask value for bit 31 */

#define MASK_B0   0x000000ff    /* Mask value for byte 0 */
#define MASK_B1   0x0000ff00    /* Mask value for byte 1 */
#define MASK_B2   0x00ff0000    /* Mask value for byte 2 */
#define MASK_B3   0xff000000    /* Mask value for byte 3 */

#define MASK_W0   0x0000ffff    /* Mask value for word 0 */
#define MASK_W1   0xffff0000    /* Mask value for word 1 */

#define MASK_PA   0xfffffffc    /* Mask value for physical address */
#define MASK_PR   0xfffffffe 	/* Mask value for protection register */
#define MASK_ER   0xffffffff    /* Mask value for the entire register */

#define MASK_NONE 0x00000000    /* No mask */

/************************************************************************/
/* REGISTERS 								*/
/************************************************************************/

#define BASE_ODD1         0x00  /* Video DMA 1 registers  */
#define BASE_EVEN1        0x04
#define PROT_ADDR1        0x08
#define PITCH1            0x0C
#define BASE_PAGE1        0x10  /* Video DMA 1 base page */
#define NUM_LINE_BYTE1    0x14

#define BASE_ODD2         0x18  /* Video DMA 2 registers */
#define BASE_EVEN2        0x1C
#define PROT_ADDR2        0x20
#define PITCH2            0x24
#define BASE_PAGE2        0x28  /* Video DMA 2 base page */
#define NUM_LINE_BYTE2    0x2C

#define BASE_ODD3         0x30  /* Video DMA 3 registers */
#define BASE_EVEN3        0x34
#define PROT_ADDR3        0x38
#define PITCH3            0x3C         
#define BASE_PAGE3        0x40  /* Video DMA 3 base page */
#define NUM_LINE_BYTE3    0x44

#define PCI_BT_V1         0x48  /* Video/FIFO 1 */
#define PCI_BT_V2         0x49  /* Video/FIFO 2 */
#define PCI_BT_V3         0x4A  /* Video/FIFO 3 */
#define PCI_BT_DEBI       0x4B  /* DEBI */
#define PCI_BT_A          0x4C  /* Audio */

#define DD1_INIT          0x50  /* Init setting of DD1 interface */

#define DD1_STREAM_B      0x54  /* DD1 B video data stream handling */
#define DD1_STREAM_A      0x56  /* DD1 A video data stream handling */

#define BRS_CTRL          0x58  /* BRS control register */
#define HPS_CTRL          0x5C  /* HPS control register */
#define HPS_V_SCALE       0x60  /* HPS vertical scale */
#define HPS_V_GAIN        0x64  /* HPS vertical ACL and gain */
#define HPS_H_PRESCALE    0x68  /* HPS horizontal prescale   */
#define HPS_H_SCALE       0x6C  /* HPS horizontal scale */
#define BCS_CTRL          0x70  /* BCS control */
#define CHROMA_KEY_RANGE  0x74
#define CLIP_FORMAT_CTRL  0x78  /* HPS outputs formats & clipping */

#define DEBI_CONFIG       0x7C
#define DEBI_COMMAND      0x80
#define DEBI_PAGE         0x84
#define DEBI_AD           0x88	

#define I2C_TRANSFER      0x8C	
#define I2C_STATUS        0x90	

#define BASE_A1_IN        0x94	/* Audio 1 input DMA */
#define PROT_A1_IN        0x98
#define PAGE_A1_IN        0x9C
  
#define BASE_A1_OUT       0xA0  /* Audio 1 output DMA */
#define PROT_A1_OUT       0xA4
#define PAGE_A1_OUT       0xA8

#define BASE_A2_IN        0xAC  /* Audio 2 input DMA */
#define PROT_A2_IN        0xB0
#define PAGE_A2_IN        0xB4

#define BASE_A2_OUT       0xB8  /* Audio 2 output DMA */
#define PROT_A2_OUT       0xBC
#define PAGE_A2_OUT       0xC0

#define RPS_PAGE0         0xC4  /* RPS task 0 page register */
#define RPS_PAGE1         0xC8  /* RPS task 1 page register */

#define RPS_THRESH0       0xCC  /* HBI threshold for task 0 */
#define RPS_THRESH1       0xD0  /* HBI threshold for task 1 */

#define RPS_TOV0          0xD4  /* RPS timeout for task 0 */
#define RPS_TOV1          0xD8  /* RPS timeout for task 1 */

#define IER               0xDC  /* Interrupt enable register */

#define GPIO_CTRL         0xE0  /* GPIO 0-3 register */

#define EC1SSR            0xE4  /* Event cnt set 1 source select */
#define EC2SSR            0xE8  /* Event cnt set 2 source select */
#define ECT1R             0xEC  /* Event cnt set 1 thresholds */
#define ECT2R             0xF0  /* Event cnt set 2 thresholds */

#define ACON1             0xF4
#define ACON2             0xF8

#define MC1               0xFC   /* Main control register 1 */
#define MC2               0x100  /* Main control register 2  */

#define RPS_ADDR0         0x104  /* RPS task 0 address register */
#define RPS_ADDR1         0x108  /* RPS task 1 address register */

#define ISR               0x10C  /* Interrupt status register */                                                             
#define PSR               0x110  /* Primary status register */
#define SSR               0x114  /* Secondary status register */

#define EC1R              0x118  /* Event counter set 1 register */
#define EC2R              0x11C  /* Event counter set 2 register */         

#define PCI_VDP1          0x120  /* Video DMA pointer of FIFO 1 */
#define PCI_VDP2          0x124  /* Video DMA pointer of FIFO 2 */
#define PCI_VDP3          0x128  /* Video DMA pointer of FIFO 3 */
#define PCI_ADP1          0x12C  /* Audio DMA pointer of audio out 1 */
#define PCI_ADP2          0x130  /* Audio DMA pointer of audio in 1 */
#define PCI_ADP3          0x134  /* Audio DMA pointer of audio out 2 */
#define PCI_ADP4          0x138  /* Audio DMA pointer of audio in 2 */
#define PCI_DMA_DDP       0x13C  /* DEBI DMA pointer */

#define LEVEL_REP         0x140,
#define A_TIME_SLOT1      0x180,  /* from 180 - 1BC */
#define A_TIME_SLOT2      0x1C0,  /* from 1C0 - 1FC */

/************************************************************************/
/* ISR-MASKS 								*/
/************************************************************************/

#define SPCI_PPEF       0x80000000  /* PCI parity error */
#define SPCI_PABO       0x40000000  /* PCI access error (target or master abort) */
#define SPCI_PPED       0x20000000  /* PCI parity error on 'real time data' */
#define SPCI_RPS_I1     0x10000000  /* Interrupt issued by RPS1 */
#define SPCI_RPS_I0     0x08000000  /* Interrupt issued by RPS0 */
#define SPCI_RPS_LATE1  0x04000000  /* RPS task 1 is late */
#define SPCI_RPS_LATE0  0x02000000  /* RPS task 0 is late */
#define SPCI_RPS_E1     0x01000000  /* RPS error from task 1 */
#define SPCI_RPS_E0     0x00800000  /* RPS error from task 0 */
#define SPCI_RPS_TO1    0x00400000  /* RPS timeout task 1 */
#define SPCI_RPS_TO0    0x00200000  /* RPS timeout task 0 */
#define SPCI_UPLD       0x00100000  /* RPS in upload */
#define SPCI_DEBI_S     0x00080000  /* DEBI status */
#define SPCI_DEBI_E     0x00040000  /* DEBI error */
#define SPCI_IIC_S      0x00020000  /* I2C status */
#define SPCI_IIC_E      0x00010000  /* I2C error */
#define SPCI_A2_IN      0x00008000  /* Audio 2 input DMA protection / limit */
#define SPCI_A2_OUT     0x00004000  /* Audio 2 output DMA protection / limit */
#define SPCI_A1_IN      0x00002000  /* Audio 1 input DMA protection / limit */
#define SPCI_A1_OUT     0x00001000  /* Audio 1 output DMA protection / limit */
#define SPCI_AFOU       0x00000800  /* Audio FIFO over- / underflow */
#define SPCI_V_PE       0x00000400  /* Video protection address */
#define SPCI_VFOU       0x00000200  /* Video FIFO over- / underflow */
#define SPCI_FIDA       0x00000100  /* Field ID video port A */
#define SPCI_FIDB       0x00000080  /* Field ID video port B */
#define SPCI_PIN3       0x00000040  /* GPIO pin 3 */
#define SPCI_PIN2       0x00000020  /* GPIO pin 2 */
#define SPCI_PIN1       0x00000010  /* GPIO pin 1 */
#define SPCI_PIN0       0x00000008  /* GPIO pin 0 */
#define SPCI_ECS        0x00000004  /* Event counter 1, 2, 4, 5 */
#define SPCI_EC3S       0x00000002  /* Event counter 3 */
#define SPCI_EC0S       0x00000001  /* Event counter 0 */


#define	DPRAM_BASE	0x4000
#define DPRAM_SIZE	0x2000
#define BOOT_STATE	(DPRAM_BASE + 0x7F8)
#define BOOT_SIZE		(DPRAM_BASE + 0x7FA)
#define BOOT_BASE		(DPRAM_BASE + 0x7FC)
#define BOOT_BLOCK	(DPRAM_BASE + 0x800)
#define BOOT_MAX_SIZE	0x800

#define IRQ_STATE		(DPRAM_BASE + 0x0F4)
#define IRQ_STATE_EXT	(DPRAM_BASE + 0x0F6)
#define MSGSTATE		(DPRAM_BASE + 0x0F8)
#define FILT_STATE	(DPRAM_BASE + 0x0FA)
#define COMMAND		(DPRAM_BASE + 0x0FC)
#define COM_BUFF		(DPRAM_BASE + 0x100)
#define COM_BUFF_SIZE	0x20

#define BUFF1_BASE	(DPRAM_BASE + 0x120)
#define BUFF1_SIZE	0xE0

#define DATA_BUFF_BASE	(DPRAM_BASE + 0x200)
#define DATA_BUFF_SIZE	0x1C00

#define Reserved		(DPRAM_BASE + 0x1E00)
#define Reserved_SIZE	0x1FC

#define HANDSHAKE_REG	(DPRAM_BASE + 0x1FF8)
#define IRQ_RX		(DPRAM_BASE + 0x1FFC)
#define IRQ_TX		(DPRAM_BASE + 0x1FFE)

#define DRAM_START_CODE		0x2e000404
#define DRAM_MAX_CODE_SIZE	0x00100000

#define RESET_LINE			2
#define DEBI_DONE_LINE		1

#define DAC_CS	0x8000
#define DAC_CDS	0x0000

#define DEBINOSWAP 0x4e0000
#define DEBISWAB   0x5e0000

#define BOOTSTATE_BUFFER_EMPTY	 0
#define BOOTSTATE_BUFFER_FULL	 1
#define BOOTSTATE_BOOT_COMPLETE	 2



#define DVB_ERR_NOSERVER		0
#define DVB_ERR_NONE			1
#define DVB_ERR_HARDWARE		2
#define DVB_ERR_ILLEGALDATA		3
#define DVB_ERR_NODATA			4
#define DVB_ERR_TIMEOUT			5
#define DVB_ERR_DENY			6
#define DVB_ERR_OVERFLOW		7
#define DVB_ERR_NOMEMORY		8
#define DVB_ERR_LOCK			9

#define	GPMQFull	0x0001			//Main Message Queue Full
#define	GPMQOver	0x0002			//Main Message Queue Overflow
#define	HPQFull	0x0004			//High Priority Msg Queue Full
#define	HPQOver	0x0008
#define	OSDQFull	0x0010			//OSD Queue Full
#define	OSDQOver	0x0020

#define DATA_TELETEXT            0x00 
#define DATA_FSECTION            0x01
#define DATA_IPMPE               0x02
#define DATA_MPEG_RECORD         0x03
#define DATA_DEBUG_MESSAGE       0x04
#define DATA_COMMON_INTERFACE    0x05
#define DATA_MPEG_PLAY           0x06
#define DATA_BMP_LOAD            0x07
#define DATA_IRCOMMAND           0x08
#define DATA_PIPING              0x09
#define DATA_STREAMING           0x0a


#define CI_CMD_ERROR             0x00
#define CI_CMD_ACK               0x01
#define CI_CMD_SYSTEM_READY      0x02
#define CI_CMD_KEYPRESS          0x03
#define CI_CMD_ON_TUNED          0x04
#define CI_CMD_ON_SWITCH_PROGRAM 0x05
#define CI_CMD_SECTION_ARRIVED   0x06
#define CI_CMD_SECTION_TIMEOUT   0x07
#define	CI_CMD_TIME              0x08
#define	CI_CMD_ENTER_MENU        0x09
#define	CI_CMD_FAST_PSI          0x0a
#define	CI_CMD_GET_SLOT_INFO     0x0b
#define	CI_CMD_HALT_ARM          0xe0

#define	CI_MSG_NONE              0x00
#define	CI_MSG_CI_INFO           0x01
#define	CI_MSG_MENU              0x02
#define	CI_MSG_LIST              0x03
#define	CI_MSG_TEXT              0x04
#define	CI_MSG_REQUEST_INPUT     0x05
#define	CI_MSG_INPUT_COMPLETE    0x06
#define	CI_MSG_LIST_MORE         0x07
#define	CI_MSG_MENU_MORE         0x08
#define	CI_MSG_CLOSE_MMI_IMM     0x09
#define	CI_MSG_SECTION_REQUEST   0x0a
#define	CI_MSG_CLOSE_FILTER      0x0b
#define	CI_PSI_COMPLETE          0x0c
#define	CI_MODULE_READY          0x0d
#define	CI_SWITCH_PRG_REPLY      0x0e
#define	CI_MSG_TEXT_MORE         0x0f

#define	CI_MSG_CA_PMT            0xe0
#define	CI_MSG_ERROR             0xf0



#define	I2C_ADR_VES1877		0x10	// Sat-Front End (alt und neu)
#define	I2C_ADR_VES1820		0x12	// Kabel-Front End
#define	I2C_ADR_FEC92314	0x20	// DVB-T Frontend
#define	I2C_ADR_OFDM92314	0x40	// DVB-T Frontend
#define	I2C_ADR_TERTUNER	0xC0	// DVB-T-Tuner
#define	I2C_ADR_SATTUNER	0xC2    // SAT-Tuner
#define	I2C_ADR_PHILIPS_TUNER 0xC2
#define	I2C_ADR_CABTUNER	0xC4  // Kabel-Tuner


#define PID_PAT 0x00	/* Program Association Table */
#define PID_BAT 0x01	/* Bouquet Association Table */
#define PID_CAT 0x01	/* Conditional Access Table */
#define PID_NIT 0x10	/* Network Information Table */
#define PID_SDT 0x11	/* Service Description Table */
#define PID_EIT 0x12	/* Event Information Table */
#define PID_RST 0x13	/* Running Status Table */
#define PID_TDT 0x14	/* Time Date Table */
#define PID_TOT 0x14	/* Time Offset Table */
#define PID_ST	0x14	/* Stuffung Table */
/* 0x15 - 0x1F */	/* Reserved for future use */

/* Table Identifier */

#define TID_PAT 0x00	/* Program Association Section */
#define TID_CAT 0x01	/* Conditional Access Section */
#define TID_PMT 0x02	/* Conditional Access Section */
/* 0x03 - 0x3F */	/* Reserved for future use */
#define TID_NIT_ACT 0x40	/* Network Information Section - actual */
#define TID_NIT_OTH 0x41	/* Network Information Section - other */
#define TID_SDT_ACT 0x42	/* Service Description Section - actual */
#define TID_SDT_OTH 0x46	/* Service Description Section - other */
#define TID_TDT 0x70	/* Time Date Section */
#define TID_TOT 0x73	/* Time Offset Section */

#define TID_BAT 0x01	/* Bouquet Association Section */
#define TID_BAT 0x01	/* Bouquet Association Section */

#define TID_EIT 0x12	/* Event Information Section */
#define TID_RST 0x13	/* Running Status Section */
#define TID_ST	0x14	/* Stuffung Section */
/* 0xFF */		/* Reserved for future use */

#define FRONT_TV   0
#define FRONT_DVBS 1
#define FRONT_DVBC 2
#define FRONT_DVBT 3

#define DVB_SYNC_SIGNAL        1
#define DVB_SYNC_CARRIER       2
#define DVB_SYNC_VITERBI       4
#define DVB_SYNC_FSYNC         8
#define DVB_SYNC_FRONT        16

#define FRONT_TP_CHANGED  1 
#define FRONT_FREQ_CHANGED 2
#define FRONT_RATE_CHANGED 4

#define TUNER_POR       0x80
#define TUNER_FL        0x40
#define TUNER_MODE      0x38
#define TUNER_AFC       0x07

#define MAXPROGS 4096

#define DESCR_NW_NAME		0x40
#define DESCR_SERVICE_LIST	0x41
#define DESCR_STUFFING		0x42
#define DESCR_SAT_DEL_SYS	0x43
#define DESCR_CABLE_DEL_SYS	0x44
#define DESCR_BOUQUET_NAME	0x47
#define DESCR_SERVICE		0x48
#define DESCR_COUNTRY_AVAIL	0x49
#define DESCR_LINKAGE		0x4A
#define DESCR_NVOD_REF		0x4B
#define DESCR_TIME_SHIFTED_SERVICE	0x4C
#define DESCR_SHORT_EVENT	0x4D
#define DESCR_EXTENDED_EVENT	0x4E
#define DESCR_TIME_SHIFTED_EVENT	0x4F
#define DESCR_COMPONENT		0x50
#define DESCR_MOSAIC		0x51
#define DESCR_STREAM_ID		0x52
#define DESCR_CA_IDENT		0x53
#define DESCR_CONTENT		0x54
#define DESCR_PARENTAL_RATING	0x55
#define DESCR_TELETEXT		0x56
#define DESCR_TELEPHONE		0x57
#define DESCR_LOCAL_TIME_OFF	0x58
#define DESCR_SUBTITLING	0x59
#define DESCR_TERR_DEL_SYS	0x5A
#define DESCR_ML_NW_NAME	0x5B
#define DESCR_ML_BQ_NAME	0x5C
#define DESCR_ML_SERVICE_NAME	0x5D
#define DESCR_ML_COMPONENT	0x5E
#define DESCR_PRIV_DATA_SPEC	0x5F
#define DESCR_SERVICE_MOVE	0x60
#define DESCR_SHORT_SMOOTH_BUF	0x61
#define DESCR_FREQUENCY_LIST	0x62
#define DESCR_PARTIAL_TP_STREAM	0x63
#define DESCR_DATA_BROADCAST	0x64
#define DESCR_CA_SYSTEM		0x65
#define DESCR_DATA_BROADCAST_ID	0x66

#define MAXFILTER 64

#define MAXNIT 512
#define NEUNIT 96

#define EIT_INDEX_SIZE 32768

#define    RP_None               0x00
#define    AudioPES              0x01
#define    AudioMp2              0x02
#define    AudioPCM              0x03
#define    VideoPES              0x04
#define    AV_PES                0x05

#define AV_PES_HEADER_LEN 8
// AV_PES block types:
#define AV_PES_VIDEO 1
#define AV_PES_AUDIO 2

#define CI_MODULE_PRESENT 1
#define CI_MODULE_OK      2
#define TIMERANZAHL 12

#define I2C_MEM_SIZE		0x002000	/* 2048 */
#define	RPS_MEM_SIZE		0x000200	/* 512 Bytes */
#define	DEBI_MEM_SIZE		0x008000	/* 32 KB */
#define	RECORD_PLAY_MEM		0x001800	/* 6144 B */

#define MAXKANAELE 4

#define ZEILENZAHL           287

#define INDEXSIZE           14000


#define TESTZEILEN            16

// Anzahl der Möglichkeiten
#define KOMBINATIONEN      32768
// Kleinste Testzeile
#define KLEINSTE_ZEILE        96
// Größte Testzeile
#define GROESSTE_ZEILE    ZEILENZAHL-KLEINSTE_ZEILE-1

#define LARGE_WIDTH 15
#define LARGE_HEIGHT 18

#define SMALL_WIDTH 10
#define SMALL_HEIGHT 12

#define VTTimeout 60000


#define VT_LARGE_BITMAP_HEIGHT (LARGE_HEIGHT*25)
#define VT_LARGE_BITMAP_WIDTH (LARGE_WIDTH*40)

#define VT_SMALL_BITMAP_HEIGHT (SMALL_HEIGHT*25)
#define VT_SMALL_BITMAP_WIDTH (SMALL_WIDTH*40)


#define ROUNDUP(w) (((w)+3)&~3)
#define _BitmapDataP(bmp)	(bmp->bmiHeader.biBitCount==8?(((BYTE*)(bmp))+sizeof(BITMAPINFOHEADER)+sizeof(WORD)*256):(((BYTE*)(bmp))+sizeof(BITMAPINFOHEADER)))
#define _BitmapDataR(bmp)	(((BYTE*)(bmp))+sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)*256)
#define _BitmapPalR(bmp)	((RGBQUAD*)(((BYTE*)(bmp))+sizeof(BITMAPINFOHEADER)))

#define _BitmapLargeRowR(bmp,row) (_BitmapDataR(bmp)+ROUNDUP(bmp->bmiHeader.biWidth)*(bmp->bmiHeader.biHeight-(row)*LARGE_HEIGHT))
#define _BitmapLargeChar(bmp,c)	(_BitmapLargeRowR(bmp,(c)/32)+((c)&0x1f)*LARGE_WIDTH)

#define _BitmapLargeRowP(bmp,row) ((_BitmapDataP(bmp))+VT_LARGE_BITMAP_WIDTH*((row)*LARGE_HEIGHT))
#define _BitmapLargeText(bmp,row,c) (_BitmapLargeRowP(bmp,row)+((c)*LARGE_WIDTH*2))


#define _BitmapSmallRowR(bmp,row) (_BitmapDataR(bmp)+ROUNDUP(bmp->bmiHeader.biWidth)*(bmp->bmiHeader.biHeight-(row)*SMALL_HEIGHT))
#define _BitmapSmallChar(bmp,c)	(_BitmapSmallRowR(bmp,(c)/32)+((c)&0x1f)*SMALL_WIDTH)

#define _BitmapSmallRowP(bmp,row) ((_BitmapDataP(bmp))+VT_SMALL_BITMAP_WIDTH*((row)*SMALL_HEIGHT))
#define _BitmapSmallText(bmp,row,c) (_BitmapSmallRowP(bmp,row)+((c)*SMALL_WIDTH*2))


#define _MakePage(mag,page) (((mag)?(mag):8)*100+((page)>>4)*10+((page)&0xf))


#define MAXVTDIALOG 8

#define EA_DOUBLE	1	// double height char
#define EA_HDOUBLE	2	// single height char in double height line
#define EA_BLINK	4	// blink
#define EA_CONCEALED	8	// concealed
#define EA_GRAPHIC	16	// graphic symbol
#define EA_SEPARATED	32	// use separated graphic symbol

#define E_DEF_FG	7
#define E_DEF_BG	0
#define E_DEF_ATTR	0


#define VCCLEARLINES 287

#endif