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

typedef enum
{
	VIDEO_MODE_BOB = 0,
	VIDEO_MODE_WEAVE,
	VIDEO_MODE_2FRAME,
	SIMPLE_WEAVE,
	INTERPOLATE_BOB,
	BTV_PLUGIN,
	FILM_22_PULLDOWN_ODD,
	FILM_22_PULLDOWN_EVEN,
	FILM_32_PULLDOWN_0,
	FILM_32_PULLDOWN_1,
	FILM_32_PULLDOWN_2,
	FILM_32_PULLDOWN_3,
	FILM_32_PULLDOWN_4,
	EVEN_ONLY,
	ODD_ONLY,
	BLENDED_CLIP,
	PULLDOWNMODES_LAST_ONE
} ePULLDOWNMODES;

#define IS_PULLDOWN_MODE(x) ((x) == FILM_22_PULLDOWN_ODD || \
							 (x) == FILM_22_PULLDOWN_EVEN || \
							 (x) == FILM_32_PULLDOWN_0 || \
							 (x) == FILM_32_PULLDOWN_1 || \
							 (x) == FILM_32_PULLDOWN_2 || \
							 (x) == FILM_32_PULLDOWN_3 || \
							 (x) == FILM_32_PULLDOWN_4)
#define IS_VIDEO_MODE(x) ((x) == VIDEO_MODE_WEAVE || \
                          (x) == VIDEO_MODE_BOB || \
						  (x) == VIDEO_MODE_2FRAME)
#define IS_HALF_HEIGHT(x) ((x) == EVEN_ONLY || \
						   (x) == ODD_ONLY)

#define MAXVTDIALOG 8
#define MAXPROGS 4096
#define VBI_DATA_SIZE 2048*39
#define VBI_VT  1
#define VBI_VPS 2
#define VBI_IC  8
#define VBI_VD  16
#define VBI_CC  32
#define FPSHIFT 16
#define FPFAC (1<<FPSHIFT)
#define VBI_SPL 2044

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

#define MAXMODESWITCHES 50	// Maximum number of switches to track in TrackModeSwitches()

// Nagravision Stuff
#define TESTPIXEL             32
#define ZEILENZAHL           287

#define            FORMAT_4_3               0x08
#define            FORMAT_14_9              0x01
#define            FORMAT_16_9              0x0B

#define    RP_None               0x00
#define    AudioPES              0x01
#define    AudioMp2              0x02
#define    AudioPCM              0x03
#define    VideoPES              0x04
#define    AV_PES                0x05

#define HILO(x) (x##_hi << 8 | x##_lo)


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

#define LEVEL_REP         0x140,
#define A_TIME_SLOT1      0x180,  /* from 180 - 1BC */
#define A_TIME_SLOT2      0x1C0,  /* from 1C0 - 1FC */

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

#define BTV_VER1_WIDTH 768
#define BTV_VER1_HEIGHT 576

#define ABS(x) ((x) < 0 ? -(x) : (x))

#endif
