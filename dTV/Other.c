/////////////////////////////////////////////////////////////////////////////
// other.h
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
// This was in turn based on  Linux code by:
//                                                                  
// BT-Parts                                                         
//                                                                  
// Copyright (C) 1996,97,98 Ralph  Metzler (rjkm@thp.uni-koeln.de)  
//                         & Marcus Metzler (mocm@thp.uni-koeln.de) 
// msp34XX                                                          
//                                                                  
// Copyright (C) 1997,1998 Gerd Knorr <kraxel@goldbach.in-berlin.de>
//                                                                  
// Copyright (C) 1996,97,98 Ralph  Metzler (rjkm@thp.uni-koeln.de)  
//                         & Marcus Metzler (mocm@thp.uni-koeln.de) 
// msp34XX                                                          
//                                                                  
// Copyright (C) 1997,1998 Gerd Knorr <kraxel@goldbach.in-berlin.de>
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

#include "other.h"
#include "dTV.h"
#include "bt848.h"
#include "OutThreads.h"
#include "vt.h"

// file global variables
unsigned char SaveIFORM;

int SaveTuner;
int SaveVideoSource;


CRITICAL_SECTION m_cCrit;

PHYS    RiscBasePhysical; 
DWORD  *RiscBaseLinear;

WORD m_wWindowWidth;
WORD m_wWindowHeight;

DWORD *m_pRiscJump;

DWORD *m_pRiscVBIOdd[5];

DWORD *m_pRiscFrameEven[5];

DWORD *m_pRiscVBIEven[5];

DWORD *m_pRiscFrameOdd[5];

static const BYTE AudioChannel[][5] = 
{
	{ 0x02, 0x00, 0x00, 0x00, 0x0a}, /* MIRO */
	{ 0x00, 0x01, 0x02, 0x03, 0x04}, /* Hauppauge */
	{ 0x04, 0x00, 0x02, 0x03, 0x01}, /* STB */
	{ 0x00, 0x01, 0x02, 0x03, 0x04}, /* Intel??? */
	{ 0x00, 0x01, 0x02, 0x03, 0x04}, /* Diamond DTV2000 */
	{ 0x0c, 0x00, 0x0b, 0x0b, 0x00}, /* AVerMedia TVPhone */
};


int CAudioSource=0;

/*
0		Audio_Tuner,
1		Audio_Radio,
2		Audio_External,
3		Audio_Internal,
4		Audio_Off,
5		Audio_On,
0x80	Audio_Mute = 0x80,
0x81	Audio_UnMute = 0x81
*/	
int CurrentCapture;
// 0  Capture_Frame_On
// 1  Capture_Frame_Off
// 2  Capture_VBI_On
// 3  Capture_VBI_Off
// 4  Capture_Pause_On
// 5  Capture_Pause_Off


// Audio


BYTE AudioDeviceWrite, AudioDeviceRead;

BYTE TunerDeviceWrite, TunerDeviceRead;


LPDIRECTDRAWSURFACE     lpDDSurface = NULL;
// OverLay 
LPDIRECTDRAWSURFACE     lpDDOverlay = NULL;
LPDIRECTDRAWSURFACE     lpDDOverlayBack = NULL;
BYTE* lpOverlay = NULL;
BYTE* lpOverlayBack = NULL;
long OverlayPitch = 0;
BOOL Can_ColorKey=FALSE;

char BTVendorID[10];
char BTDeviceID[10];

static struct MSP_INIT_DATA_DEM
{
	int fir1[6];
	int fir2[6];
	int cdo1;
	int cdo2;
	int ad_cv;
	int mode_reg;
	int dfp_src;
	int dfp_matrix;
} msp_init_data[] = 
{
	/* AM (for carrier detect / msp3400) */
	{ { 75, 19, 36, 35, 39, 40 }, { 75, 19, 36, 35, 39, 40 },
	  MSP_CARRIER(5.5), MSP_CARRIER(5.5),
	  0x00d0, 0x0500,   0x0020, 0x3000},

	/* AM (for carrier detect / msp3410) */
	{ { -1, -1, -8, 2, 59, 126 }, { -1, -1, -8, 2, 59, 126 },
	  MSP_CARRIER(5.5), MSP_CARRIER(5.5),
	  0x00d0, 0x0100,   0x0020, 0x3000},

	/* FM Radio */
	{ { -8, -8, 4, 6, 78, 107 }, { -8, -8, 4, 6, 78, 107 },
	  MSP_CARRIER(10.7), MSP_CARRIER(10.7),
	  0x00d0, 0x0480, 0x0020, 0x3000 },

	/* Terrestial FM-mono + FM-stereo */
	{ {  3, 18, 27, 48, 66, 72 }, {  3, 18, 27, 48, 66, 72 },
	  MSP_CARRIER(5.5), MSP_CARRIER(5.5),
	  0x00d0, 0x0480,   0x0030, 0x3000},

	/* Sat FM-mono */
	{ {  1,  9, 14, 24, 33, 37 }, {  3, 18, 27, 48, 66, 72 },
	  MSP_CARRIER(6.5), MSP_CARRIER(6.5),
	  0x00c6, 0x0480,   0x0000, 0x3000},

	/* NICAM B/G, D/K */
	{ { -2, -8, -10, 10, 50, 86 }, {  3, 18, 27, 48, 66, 72 },
	  MSP_CARRIER(5.5), MSP_CARRIER(5.5),
	  0x00d0, 0x0040,   0x0120, 0x3000},

	/* NICAM I */
	{ {  2, 4, -6, -4, 40, 94 }, {  3, 18, 27, 48, 66, 72 },
	  MSP_CARRIER(6.0), MSP_CARRIER(6.0),
	  0x00d0, 0x0040,   0x0120, 0x3000},
};


int carrier_detect_main[4] = {
	/* main carrier */
	 MSP_CARRIER(4.5),   // 4.5   NTSC                 
	 MSP_CARRIER(5.5),   // 5.5   PAL B/G              
	 MSP_CARRIER(6.0),   // 6.0   PAL I                
	 MSP_CARRIER(6.5),   // 6.5   PAL D/K + SAT + SECAM
};

int carrier_detect[8] = {
	/* PAL B/G */
	 MSP_CARRIER(5.7421875), // 5.742 PAL B/G FM-stereo
	 MSP_CARRIER(5.85),      // 5.85  PAL B/G NICAM
	/* PAL SAT / SECAM */
	 MSP_CARRIER(5.85),      // 5.85  PAL D/K NICAM
	 MSP_CARRIER(6.2578125), //6.25  PAL D/K1 FM-stereo
	 MSP_CARRIER(6.7421875), //6.74  PAL D/K2 FM-stereo
	 MSP_CARRIER(7.02),      //7.02  PAL SAT FM-stereo s/b
	 MSP_CARRIER(7.20),      //7.20  PAL SAT FM-stereo s
	 MSP_CARRIER(7.38),      //7.38  PAL SAT FM-stereo b
};

int MSPMode;
int MSPStereo;
int MSPNewStereo;
int MSPAutoDetectValue;
BOOL MSPNicam;
char MSPVersion[16];
int MSPMajorMode;
int MSPMinorMode;
BOOL AutoStereoSelect=TRUE;

BOOL Init_TV_Karte(HWND hWnd)
{
	FILE *SettingFile;

	char VersionString[255];
	int ret;
	unsigned short i;

	strcpy(BTVendorID, "0x109e");
	strcpy(BTDeviceID, "0x036e");

	ret = BT8X8_Open(0x109e, 0x36e, TRUE, FALSE);
	if (ret == 0)
	{
		strcpy(BTTyp, "BT878");
		strcpy(VersionString, "BT878 found");
	}
	else if (ret == 3)
	{
		MessageBox(hWnd, "PCI-Card with Bt878 Cannot be locked", "dTV", MB_ICONSTOP | MB_OK);
		return (FALSE);
	}
	else
	{
		strcpy(BTVendorID, "0x109e");
		strcpy(BTDeviceID, "0x0350");
		ret = BT8X8_Open(0x109e, 0x350, TRUE, FALSE);
		if (ret == 0)
		{
			strcpy(BTTyp, "BT848");
			strcpy(VersionString, "BT848 found");
		}
		else if (ret == 3)
		{
			MessageBox(hWnd, "PCI-Card with Bt848 Cannot be locked", "dTV", MB_ICONSTOP | MB_OK);
			return (FALSE);
		}
		else
		{
			strcpy(BTVendorID, "0x109e");
			strcpy(BTDeviceID, "0x0351");
			ret = BT8X8_Open(0x109e, 0x351, TRUE, FALSE);
			if (ret == 0)
			{
				strcpy(BTTyp, "BT849");
				strcpy(VersionString, "BT849 found");
			}
			else if (ret == 3)
			{
				MessageBox(hWnd, "PCI-Card with Bt849 Cannot be locked", "dTV", MB_ICONSTOP | MB_OK);
				return (FALSE);
			}
			else
			{
				strcpy(BTVendorID, "0x109e");
				strcpy(BTDeviceID, "0x036F");
				ret = BT8X8_Open(0x109e, 0x36F, TRUE, FALSE);
				if (ret == 0)
				{
					strcpy(BTTyp, "BT878");
					strcpy(VersionString, "Anubis BT878");
				}
				else if (ret == 3)
				{
					MessageBox(hWnd, "PCI-Card with Anubis Bt849 Cannot be locked", "dTV", MB_ICONSTOP | MB_OK);
					return (FALSE);
				}
			}
		}
	}

	if (ret != 0)
	{
		strcpy(VersionString, "No BT8x8 Found");
		SetDlgItemText(SplashWnd, IDC_TEXT2, VersionString);
		return (FALSE);
	}

	if (bSaveSettings == TRUE)
	{
		if ((SettingFile = fopen("Setting.BT", "w")) != NULL)
		{
			fprintf(SettingFile, "BT848_COLOR_CTL %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_COLOR_CTL));
			fprintf(SettingFile, "BT848_CAP_CTL %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_CAP_CTL));
			fprintf(SettingFile, "BT848_VBI_PACK_SIZE %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_VBI_PACK_SIZE));
			fprintf(SettingFile, "BT848_VBI_PACK_DEL %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_VBI_PACK_DEL));
			fprintf(SettingFile, "BT848_GPIO_DMA_CTL %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_GPIO_DMA_CTL));
			fprintf(SettingFile, "BT848_IFORM %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_IFORM));

			fprintf(SettingFile, "BT848_E_SCLOOP %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_E_SCLOOP));
			fprintf(SettingFile, "BT848_O_SCLOOP %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_O_SCLOOP));
			fprintf(SettingFile, "BT848_ADELAY %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_ADELAY));
			fprintf(SettingFile, "BT848_BDELAY %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_BDELAY));

			fprintf(SettingFile, "BT848_E_HSCALE_HI %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_E_HSCALE_HI));
			fprintf(SettingFile, "BT848_E_HSCALE_LO %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_E_HSCALE_LO));
			fprintf(SettingFile, "BT848_E_VSCALE_HI %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_E_VSCALE_HI));
			fprintf(SettingFile, "BT848_E_VSCALE_LO %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_E_VSCALE_LO));
			fprintf(SettingFile, "BT848_E_HACTIVE_LO %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_E_HACTIVE_LO));
			fprintf(SettingFile, "BT848_E_HDELAY_LO %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_E_HDELAY_LO));
			fprintf(SettingFile, "BT848_E_VACTIVE_LO %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_E_VACTIVE_LO));
			fprintf(SettingFile, "BT848_E_VDELAY_LO %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_E_VDELAY_LO));
			fprintf(SettingFile, "BT848_E_CROP %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_E_CROP));

			fprintf(SettingFile, "BT848_O_HSCALE_HI %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_O_HSCALE_HI));
			fprintf(SettingFile, "BT848_O_HSCALE_LO %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_O_HSCALE_LO));
			fprintf(SettingFile, "BT848_O_VSCALE_HI %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_O_VSCALE_HI));
			fprintf(SettingFile, "BT848_O_VSCALE_LO %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_E_VSCALE_LO));
			fprintf(SettingFile, "BT848_O_HACTIVE_LO %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_O_HACTIVE_LO));
			fprintf(SettingFile, "BT848_O_HDELAY_LO %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_O_HDELAY_LO));
			fprintf(SettingFile, "BT848_O_VACTIVE_LO %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_O_VACTIVE_LO));
			fprintf(SettingFile, "BT848_O_VDELAY_LO %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_O_VDELAY_LO));
			fprintf(SettingFile, "BT848_O_CROP %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_O_CROP));

			fprintf(SettingFile, "BT848_PLL_F_LO %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_PLL_F_LO));
			fprintf(SettingFile, "BT848_PLL_F_HI %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_PLL_F_HI));
			fprintf(SettingFile, "BT848_PLL_XCI %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_PLL_XCI));

			fprintf(SettingFile, "BT848_BRIGHT %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_BRIGHT));
			fprintf(SettingFile, "BT848_CONTRAST_LO %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_CONTRAST_LO));
			fprintf(SettingFile, "BT848_SAT_V_LO %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_SAT_V_LO));
			fprintf(SettingFile, "BT848_SAT_U_LO %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_SAT_U_LO));
			fprintf(SettingFile, "BT848_GPIO_OUT_EN %04x\n", BT8X8_ReadWord(BT8X8_AD_BAR0, BT848_GPIO_OUT_EN));
			fprintf(SettingFile, "BT848_GPIO_OUT_EN_HIBYTE %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_GPIO_OUT_EN_HIBYTE));

			fprintf(SettingFile, "BT848_GPIO_REG_INP %04x\n", BT8X8_ReadWord(BT8X8_AD_BAR0, BT848_GPIO_REG_INP));
			fprintf(SettingFile, "BT848_GPIO_REG_INP_HIBYTE %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_GPIO_REG_INP_HIBYTE));

			fprintf(SettingFile, "BT848_GPIO_DATA %04x\n", BT8X8_ReadWord(BT8X8_AD_BAR0, BT848_GPIO_DATA));
			fprintf(SettingFile, "BT848_GPIO_DATA_HIBYTE %02x\n", BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_GPIO_DATA_HIBYTE));
			i = ((BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_GPIO_OUT_EN_HIBYTE)) << 16) + BT8X8_ReadWord(BT8X8_AD_BAR0, BT848_GPIO_OUT_EN);
			fprintf(SettingFile, "*********************************************\n");
			fprintf(SettingFile, "Ausgelesene Einträge für Eigenen KartenTyp\n");
			fprintf(SettingFile, "Eintrag für BT848_GPIO_OUT_EN  %9d     ( Schaltwert )\n", i);
			i = ((BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_GPIO_REG_INP_HIBYTE)) << 16) + BT8X8_ReadWord(BT8X8_AD_BAR0, BT848_GPIO_REG_INP);
			fprintf(SettingFile, "Eintrag für BT848_GPIO_REG_INP %9d     ( Input-Control )\n", i);
			i = ((BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_GPIO_DATA_HIBYTE)) << 16) + BT8X8_ReadWord(BT8X8_AD_BAR0, BT848_GPIO_DATA);
			fprintf(SettingFile, "Eintrag für BT848_GPIO_DATA    %9d     ( Eingangswunsch) \n", i);
			fprintf(SettingFile, "*********************************************\n");
			fclose(SettingFile);
		}
	}

	SetDlgItemText(SplashWnd, IDC_TEXT2, VersionString);

	strcpy(VersionString, "Interrupt OK");
	SetDlgItemText(SplashWnd, IDC_TEXT3, VersionString);
	return (TRUE);
}

BOOL Init_BT_Kernel_Memory(void)
{
	if (!Alloc_DMA(83968, &Risc_dma, ALLOC_MEMORY_CONTIG))
	{
		MessageBox(hWnd, "Risc Memory (83 KB Contiguous) not Allocated", "dTV", MB_ICONSTOP | MB_OK);
		return (FALSE);
	}

	RiscBaseLinear = Risc_dma->dwUser;
	RiscBasePhysical = GetPhysicalAddress(Risc_dma, Risc_dma->dwUser, 83968, NULL);
	return (TRUE);
}

BOOL Init_Memory(void)
{
	int i;
	for (i = 0; i < 5; i++)
	{
		if (!Alloc_DMA(VBI_DATA_SIZE, &Vbi_dma[i], i))
		{
			MessageBox(hWnd, "VideoText Memory ( 77 KB ) for DMA not allocated", "dTV", MB_ICONSTOP | MB_OK);
			return (FALSE);
		}
		if (!Alloc_Display_DMA(1024 * 576 * 2, i))
		{
			MessageBox(hWnd, "Display Memory (1 MB) for DMA not allocated", "dTV", MB_ICONSTOP | MB_OK);
			return (FALSE);
		}
		if (!Alloc_DMA(256, &Burst_dma[i], i))
		{
			MessageBox(hWnd, "Burst Memory (256 Bytes) for DMA not allocated", "dTV", MB_ICONSTOP | MB_OK);
			return (FALSE);
		}

		pBurstLine[i] = Burst_dma[i]->dwUser;
	}

	return (TRUE);
}

BOOL Init_BT_HardWare(HWND hWnd)
{
	int i;

	i = 0;

	m_pRiscJump = RiscBaseLinear;
	m_pRiscVBIOdd[0] = RiscBaseLinear + 64;
	m_pRiscVBIEven[0] = m_pRiscVBIOdd[0] + 160;
	m_pRiscFrameOdd[0] = m_pRiscVBIEven[0] + 160;
	m_pRiscFrameEven[0] = m_pRiscFrameOdd[0] + 1536;

	m_pRiscVBIOdd[1] = m_pRiscFrameEven[0] + 1536;
	m_pRiscVBIEven[1] = m_pRiscVBIOdd[1] + 160;
	m_pRiscFrameOdd[1] = m_pRiscVBIEven[1] + 160;
	m_pRiscFrameEven[1] = m_pRiscFrameOdd[1] + 1536;

	m_pRiscVBIOdd[2] = m_pRiscFrameEven[1] + 1536;
	m_pRiscVBIEven[2] = m_pRiscVBIOdd[2] + 160;
	m_pRiscFrameOdd[2] = m_pRiscVBIEven[2] + 160;
	m_pRiscFrameEven[2] = m_pRiscFrameOdd[2] + 1536;

	m_pRiscVBIOdd[3] = m_pRiscFrameEven[2] + 1536;
	m_pRiscVBIEven[3] = m_pRiscVBIOdd[3] + 160;
	m_pRiscFrameOdd[3] = m_pRiscVBIEven[3] + 160;
	m_pRiscFrameEven[3] = m_pRiscFrameOdd[3] + 1536;

	m_pRiscVBIOdd[4] = m_pRiscFrameEven[3] + 1536;
	m_pRiscVBIEven[4] = m_pRiscVBIOdd[4] + 160;
	m_pRiscFrameOdd[4] = m_pRiscVBIEven[4] + 160;
	m_pRiscFrameEven[4] = m_pRiscFrameOdd[4] + 1536;

	Reset_BT_HardWare();

	for(i = 0; i < 5; i++)
	{
		MakeVBITable(i, VBI_lpf);
	}

	if (Capture_Video == TRUE)
	{
		CheckMenuItem(GetMenu(hWnd), IDM_VIDEO, MF_CHECKED);
	}

	if (Capture_VBI == TRUE)
	{
		Start_VBI();
		CheckMenuItem(GetMenu(hWnd), IDM_VBI, MF_CHECKED);
		EnableMenuItem(GetMenu(hWnd), IDM_VBI_VT, MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), IDM_VBI_IC, MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), IDM_VBI_VD, MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), IDM_VBI_VPS, MF_ENABLED);
	}

	return (TRUE);
}

void Reset_BT_HardWare()
{
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_SRESET, 0);
	BT8X8_WriteDword(BT8X8_AD_BAR0, BT848_RISC_STRT_ADD, RiscLogToPhys(m_pRiscJump + 2));
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_CAP_CTL, 0x00);
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_VBI_PACK_SIZE, (VBI_SPL / 4) & 0xff);
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_VBI_PACK_DEL, (VBI_SPL / 4) >> 8);
	BT8X8_WriteWord(BT8X8_AD_BAR0, BT848_GPIO_DMA_CTL, 0xfc);
	if (InitialIFORM != 0)
	{
		BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_IFORM, InitialIFORM);
	}
	else
	{
		BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_IFORM, BT848_IFORM_MUX1 | BT848_IFORM_XTAUTO | BT848_IFORM_PAL_BDGHI);
	}
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_E_CONTROL, 0x00);
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_O_CONTROL, 0x00);

	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_E_SCLOOP, BT848_SCLOOP_CAGC);
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_O_SCLOOP, BT848_SCLOOP_CAGC);

	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_OFORM, BT848_OFORM_CORE0 | BT848_OFORM_RANGE);
	
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_E_VSCALE_HI, BT848_VSCALE_COMB);
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_O_VSCALE_HI, BT848_VSCALE_COMB);
	
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_ADC, BT848_ADC_RESERVED | BT848_ADC_CRUSH);

	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_COLOR_CTL, 0x00);
	
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_TDEC, 0x00);

	BT8X8_WriteDword(BT8X8_AD_BAR0, BT848_INT_STAT, (DWORD) 0x0fffffff);
	BT8X8_WriteDword(BT8X8_AD_BAR0, BT848_INT_MASK, (1 << 23) | BT848_INT_RISCI);

	Set_PLL(INIT_PLL);

	SetBrightness(InitialBrightness);
	SetContrast(InitialContrast);
	SetHue(InitialHue);
	SetSaturationU(InitialSaturationU);
	SetSaturationV(InitialSaturationV);
	SetVideoSource(VideoSource);
	SetColourFormat(ColourFormat);
}

void Get_Thread_Status()
{
	DWORD stat = BT8X8_ReadDword(BT8X8_AD_BAR0, BT848_INT_STAT);
	while(bIsOddField == ((stat & BT848_INT_FIELD) == BT848_INT_FIELD))
	{
		Sleep(5);
		stat = BT8X8_ReadDword(BT8X8_AD_BAR0, BT848_INT_STAT);
	}

	bIsOddField = ((stat & BT848_INT_FIELD) == BT848_INT_FIELD);

	switch(stat >> 28)
	{
	case 1:
		CurrentFrame = 0;
		break;
	case 2:
		CurrentFrame = 1;
		break;
	case 3:
		CurrentFrame = 2;
		break;
	case 4:
		CurrentFrame = 3;
		break;
	case 5:
		CurrentFrame = 4;
		break;
	default:
		// just leave the same as last time
		// the stat flag is only set after ODD fields
		break;
	}
}

PHYS RiscLogToPhys(DWORD * pLog)
{
	return (RiscBasePhysical + (pLog - RiscBaseLinear) * 4);
}

void MakeVBITable(int nIndex, int VBI_Lines)
{
	int i;
	DWORD *po = m_pRiscVBIOdd[nIndex];
	DWORD *pe = m_pRiscVBIEven[nIndex];
	PHYS pPhysVBI;
	LPBYTE pVBI = (LPBYTE) Vbi_dma[nIndex]->dwUser;

	int GotBytesPerLine;

	*(pe++) = (BT848_RISC_SYNC | BT848_FIFO_STATUS_FM1);
	*(pe++) = 0;
	for (i = 0; i < VBI_Lines; i++)
	{
		pPhysVBI = GetPhysicalAddress(Vbi_dma[nIndex], pVBI, VBI_SPL, &GotBytesPerLine);
		if (GotBytesPerLine < VBI_SPL)
		{
			*(pe++) = BT848_RISC_WRITE | BT848_RISC_SOL | GotBytesPerLine;
			*(pe++) = pPhysVBI;
			// Assumes lines aren't >8K long!
			pPhysVBI = GetPhysicalAddress(Vbi_dma[nIndex], pVBI + GotBytesPerLine, 0, 0);
			*(pe++) = BT848_RISC_WRITE | BT848_RISC_EOL | (VBI_SPL - GotBytesPerLine);
			*(pe++) = pPhysVBI;
		}
		else
		{
			*(pe++) = BT848_RISC_WRITE | BT848_RISC_SOL | BT848_RISC_EOL | VBI_SPL;
			*(pe++) = pPhysVBI;
		}
		pVBI += 2048;
	}
	if (Capture_Video == FALSE)
	{
		if(nIndex > 0)
		{
			*(pe++) = BT848_RISC_JUMP;
		}
		else
		{
			*(pe++) = BT848_RISC_JUMP;
		}
	}
	else
	{
		*(pe++) = BT848_RISC_JUMP;
	}

	*(pe++) = RiscLogToPhys(m_pRiscJump + 4 + nIndex * 12);
	*(po++) = (BT848_RISC_SYNC | BT848_FIFO_STATUS_FM1);
	*(po++) = 0;

	for (i = VBI_Lines; i < VBI_Lines * 2; i++)
	{
		pPhysVBI = GetPhysicalAddress(Vbi_dma[nIndex], pVBI, VBI_SPL, &GotBytesPerLine);
		if (GotBytesPerLine < VBI_SPL)
		{
			*(po++) = BT848_RISC_WRITE | BT848_RISC_SOL | GotBytesPerLine;
			*(po++) = pPhysVBI;
			// Assumes lines aren't >8K long!
			pPhysVBI = GetPhysicalAddress(Vbi_dma[nIndex], pVBI + GotBytesPerLine, 0, 0);
			*(po++) = BT848_RISC_WRITE | BT848_RISC_EOL | (VBI_SPL - GotBytesPerLine);
			*(po++) = pPhysVBI;
		}
		else
		{
			*(po++) = BT848_RISC_WRITE | BT848_RISC_SOL | BT848_RISC_EOL | VBI_SPL;
			*(po++) = pPhysVBI;
		}
		pVBI += 2048;
	}
	*(po++) = BT848_RISC_JUMP;
	*(po++) = RiscLogToPhys(m_pRiscJump + 10 + nIndex * 12);
}


void Set_PLL(int PLL)
{
	int i;

	if (PLL == 0)
	{
		BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_TGCTRL, BT848_TGCTRL_TGCKI_NOPLL);
		BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_PLL_XCI, 0x00);
		return;
	}
	else if (PLL == 1)
	{
		BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_PLL_F_LO, 0xf9);
		BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_PLL_F_HI, 0xdc);
		BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_PLL_XCI, 0x8E);
	}
	else if (PLL == 2)
	{
		BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_PLL_F_LO, 0x39);
		BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_PLL_F_HI, 0xB0);
		BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_PLL_XCI, 0x89);
	}

	for (i = 0; i < 100; i++)
	{
		if (BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_DSTATUS) & BT848_DSTATUS_CSEL)
		{
			BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_DSTATUS, 0x00);
		}
		else
		{
			BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_TGCTRL, BT848_TGCTRL_TGCKI_PLL);
			break;
		}
		Sleep(10);
	}

	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_WC_UP, 0xcf);
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_VTOTAL_LO, 0x00);
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_VTOTAL_HI, 0x00);
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_DVSIF, 0x00);
}

void SetDMA(BOOL bState)
{
	unsigned short x, y;

	x = BT8X8_ReadWord(BT8X8_AD_BAR0, BT848_GPIO_DMA_CTL);

	if (bState)
	{
		OrDataWord(BT848_GPIO_DMA_CTL, 3);
	}
	else
	{
		AndDataWord(BT848_GPIO_DMA_CTL, ~3);
	}

	y = BT8X8_ReadWord(BT8X8_AD_BAR0, BT848_GPIO_DMA_CTL);
}

BOOL SetColourFormat(int nColourFormat)
{
	if (nColourFormat > 10)
		return FALSE;

	if (nColourFormat == BT848_COLOR_FMT_RGB8)
	{
		AndDataByte(BT848_CAP_CTL, ~0x10);
	}
	else
	{
		OrDataByte(BT848_CAP_CTL, 0x10);
	}
	ColourFormat = nColourFormat;
	SetGeometry(m_wWindowWidth, m_wWindowHeight, TVTYPE, (ColourFormat << 4) | ColourFormat);

	return TRUE;
}

BOOL SetGeoSize(int wWidth, int wHeight)
{

	if (!SetGeometry(wWidth, wHeight, TVTYPE, (ColourFormat << 4) | ColourFormat))
	{
		return FALSE;
	}
	else
	{
		MakeVideoTableForDisplay();
	}
	return TRUE;
}

BOOL SetBrightness(unsigned char bBrightness)
{
	short Low;
	unsigned short *Lowptr;
	__int64 CurrentLowValue = 0x002d002d002d002d;

	Lowptr = (short *) &CurrentLowValue;
	Low = InitialLow;
	nLevelHigh = 135;
	if (bBrightness < 128)
	{
		Low = Low + bBrightness;
		nLevelHigh = nLevelHigh + bBrightness;
		nLevelLow = 45 + bBrightness;

	}
	else
	{
		Low = Low - (256 - bBrightness);
		nLevelHigh = nLevelHigh - (256 - bBrightness);
		nLevelLow = 45 - (256 - bBrightness);
	}

	if (Low < 2)
	{
		Low = 0x02;
	}
	if (nLevelHigh > 225)
	{
		nLevelHigh = 225;
	}
	*Lowptr = Low;
	Lowptr++;
	*Lowptr = Low;
	Lowptr++;
	*Lowptr = Low;
	Lowptr++;
	*Lowptr = Low;
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_BRIGHT, bBrightness);
	return TRUE;
}

BOOL SetHue(char bHue)
{
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_HUE, bHue);
	return TRUE;
}

BOOL SetContrast(int wContrast)
{
	BYTE bContHi;

	bContHi = (BYTE) (wContrast >> 6) & 4;
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_CONTRAST_LO, (BYTE) (wContrast & 0xff));
	MaskDataByte(BT848_E_CONTROL, bContHi, 4);
	MaskDataByte(BT848_O_CONTROL, bContHi, 4);
	return TRUE;
}

BOOL SetSaturationU(int wData)
{
	BYTE bDataHi;

	bDataHi = (BYTE) (wData >> 7) & 2;
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_SAT_U_LO, (BYTE) (wData & 0xff));
	MaskDataByte(BT848_E_CONTROL, bDataHi, 2);
	MaskDataByte(BT848_O_CONTROL, bDataHi, 2);
	return TRUE;
}

BOOL SetSaturationV(int wData)
{
	BYTE bDataHi;

	bDataHi = (BYTE) (wData >> 8) & 1;
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_SAT_V_LO, (BYTE) (wData & 0xff));
	MaskDataByte(BT848_E_CONTROL, bDataHi, 1);
	MaskDataByte(BT848_O_CONTROL, bDataHi, 1);
	return TRUE;
}

BOOL SetVideoSource(int nInput)
{

	// 0= Video_Tuner,
	// 1= Video_Ext1,
	// 2= Video_Ext2,
	// 3= Video_Ext3

	if (InitialIFORM == 0)
	{
		AndDataByte(BT848_IFORM, ~(3 << 5));
	}

	switch (nInput)
	{
	case 0:
	case 1:
	case 2:
		AndDataByte(BT848_E_CONTROL, ~BT848_CONTROL_COMP);
		AndDataByte(BT848_O_CONTROL, ~BT848_CONTROL_COMP);
		break;
	case 3:
		OrDataByte(BT848_E_CONTROL, BT848_CONTROL_COMP);
		OrDataByte(BT848_O_CONTROL, BT848_CONTROL_COMP);
		break;
	}

	VideoSource = nInput;
	if (InitialIFORM == 0)
	{
		MaskDataByte(BT848_IFORM, (BYTE) (((nInput + 2) & 3) << 5), (BYTE) (3 << 5));
	}
	return TRUE;
}

void SetGeometryEvenOdd(BOOL bOdd, BYTE bVtc, int wHScale, int wVScale, int wHActive, int wVActive, int wHDelay, int wVDelay, BYTE bCrop)
{
	int nOff = bOdd ? 0x80 : 0x00;

	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_E_VTC + nOff, bVtc);
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_E_HSCALE_HI + nOff, (BYTE) (wHScale >> 8));
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_E_HSCALE_LO + nOff, (BYTE) (wHScale & 0xFF));
	MaskDataByte(BT848_E_VSCALE_HI + nOff, (BYTE) (wVScale >> 8), 0x1F);
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_E_VSCALE_LO + nOff, (BYTE) (wVScale & 0xFF));
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_E_HACTIVE_LO + nOff, (BYTE) (wHActive & 0xFF));
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_E_HDELAY_LO + nOff, (BYTE) (wHDelay & 0xFF));
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_E_VACTIVE_LO + nOff, (BYTE) (wVActive & 0xFF));
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_E_VDELAY_LO + nOff, (BYTE) (wVDelay & 0xFF));
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_E_CROP + nOff, bCrop);
}

BOOL SetGeometry(int width, int height, int Type, int CF)
{
	int vscale, hscale;
	DWORD sr;
	int hdelay, vdelay;
	int hactive, vactive;
	BYTE crop, vtc;

	if (!width || !height)
	{
		return FALSE;
	}

	m_wWindowHeight = height;
	m_wWindowWidth = width;

	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_ADELAY, TVSettings[Type].bDelayA);
	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_BDELAY, TVSettings[Type].bDelayB);
	if (InitialIFORM == 0)
	{
		MaskDataByte(BT848_IFORM, TVSettings[Type].bIForm, BT848_IFORM_NORM | BT848_IFORM_XTBOTH);
	}

	BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_COLOR_FMT, (BYTE) CF);

	hactive = width;
	vtc = (hactive < 193) ? 2 : ((hactive < 385) ? 1 : 0);

	hscale = ((TVSettings[Type].wTotalWidth - TVSettings[Type].dwXsfNum) * 4096UL) / TVSettings[Type].dwXsfNum;
	vdelay = TVSettings[Type].wVDelay;
	hdelay = ((width * TVSettings[Type].wHDelayx1) / TVSettings[Type].wHActivex1) & 0x3fe;

	sr = (TVSettings[Type].wCropHeight * 512) / height - 512;
	vscale = (WORD) (0x10000UL - sr) & 0x1fff;
	vactive = TVSettings[Type].wCropHeight;
	crop = ((hactive >> 8) & 0x03) | ((hdelay >> 6) & 0x0c) | ((vactive >> 4) & 0x30) | ((vdelay >> 2) & 0xc0);

	SetGeometryEvenOdd(FALSE, vtc, hscale, vscale, hactive, vactive, hdelay, vdelay, crop);
	SetGeometryEvenOdd(TRUE, vtc, hscale, vscale, hactive, vactive, hdelay, vdelay, crop);
	return TRUE;
}

BOOL SetAudioSource(int nChannel)
{
	BYTE B3;
	unsigned short B1;
	int i;

/*		Audio_Tuner,
		Audio_Radio,
		Audio_External,
		Audio_Internal,
		Audio_Off,
		Audio_On,
		Audio_Mute = 0x80,
		Audio_UnMute = 0x81
*/
	if (USECARD == FALSE)
	{
		return (TRUE);
	}

	if (CardType == 6)
	{
		B1 = ManuellAudio[0];
		B3 = (ManuellAudio[0] >> 16);
		BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_GPIO_OUT_EN_HIBYTE, B3);
		BT8X8_WriteWord(BT8X8_AD_BAR0, BT848_GPIO_OUT_EN, B1);
		B1 = ManuellAudio[7];
		B3 = (ManuellAudio[7] >> 16);
		BT8X8_WriteWord(BT8X8_AD_BAR0, BT848_GPIO_REG_INP_HIBYTE, B3);	// 3Bytes
		BT8X8_WriteWord(BT8X8_AD_BAR0, BT848_GPIO_REG_INP, B1);	// 3Bytes
		B1 = ManuellAudio[nChannel + 1];
		B3 = (ManuellAudio[nChannel + 1] >> 16);
		BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_GPIO_DATA_HIBYTE, B3);
		BT8X8_WriteWord(BT8X8_AD_BAR0, BT848_GPIO_DATA, B1);	// 3Bytes
		return (TRUE);
	}

	switch (nChannel)
	{
	case 0x80:
		CAudioSource = (int) ((CAudioSource) | (int) 0x80);
		nChannel = AudioChannel[CardType][4];
		break;
	case 0x81:
		CAudioSource = (int) ((CAudioSource) & ~(int) 0x80);
		nChannel = AudioSource;
		break;
	case 0x04:
		nChannel = AudioChannel[CardType][4];
		break;
	case 0x05:
		nChannel = AudioSource;
		break;
	default:
		if (nChannel == 1)
		{
			Init_Audio(0x80, 0x81);
		}
		CAudioSource = (int) (CAudioSource & (int) 0x80);
		CAudioSource = (int) (CAudioSource | AudioChannel[CardType][nChannel]);
		break;
	}

	/* enable least significant GPIO output byte */
	BT8X8_WriteWord(BT8X8_AD_BAR0, BT848_GPIO_OUT_EN, 0xffff);

	i = 0;
	while ((i < 20) && (!(BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_DSTATUS) & BT848_DSTATUS_HLOC)))
	{
		i++;
		Sleep(50);
	}
	/* if audio mute or not in H-lock, turn audio off */
	if ((nChannel != 1) && ((CAudioSource & 0x80) || !(BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_DSTATUS) & BT848_DSTATUS_HLOC)))
	{
		nChannel = AudioChannel[CardType][4];
	}

	/* select direct input */
	BT8X8_WriteWord(BT8X8_AD_BAR0, BT848_GPIO_REG_INP, 0x00);	// 3Bytes
	MaskDataWord(BT848_GPIO_DATA, (unsigned short) (0xfff0 | nChannel), (unsigned short) 0x0ffff);	// 3Bytes
	return TRUE;
}

BOOL VideoPresent()
{
	return ((BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_DSTATUS) & (BT848_DSTATUS_PRES | BT848_DSTATUS_HLOC)) == (BT848_DSTATUS_PRES | BT848_DSTATUS_HLOC)) ? TRUE : FALSE;
}

void SetRiscJumpsDecode(int nFlags)
{
	m_pRiscJump = Risc_dma->dwUser;

	m_pRiscJump[0] = (DWORD) (BT848_RISC_SYNC | BT848_RISC_RESYNC | BT848_FIFO_STATUS_VRE);
	m_pRiscJump[1] = 0;

	m_pRiscJump[2] = BT848_RISC_JUMP;
	if (nFlags & BT848_CAP_CTL_CAPTURE_VBI_EVEN)
	{
		m_pRiscJump[3] = RiscLogToPhys(m_pRiscVBIEven[0]);
	}
	else
	{
		m_pRiscJump[3] = RiscLogToPhys(m_pRiscJump + 4);
	}

	m_pRiscJump[4] = BT848_RISC_JUMP;
	if (nFlags & BT848_CAP_CTL_CAPTURE_EVEN)
	{
		m_pRiscJump[5] = RiscLogToPhys(m_pRiscFrameEven[0]);
	}
	else
	{
		m_pRiscJump[5] = RiscLogToPhys(m_pRiscJump + 6);
	}

	m_pRiscJump[6] = (DWORD) (BT848_RISC_SYNC | BT848_RISC_RESYNC | BT848_FIFO_STATUS_VRO);
	m_pRiscJump[7] = 0;

	m_pRiscJump[8] = BT848_RISC_JUMP;
	if (nFlags & BT848_CAP_CTL_CAPTURE_VBI_ODD)
	{
		m_pRiscJump[9] = RiscLogToPhys(m_pRiscVBIOdd[0]);
	}
	else
	{
		m_pRiscJump[9] = RiscLogToPhys(m_pRiscJump + 10);
	}

	m_pRiscJump[10] = BT848_RISC_JUMP;
	if (nFlags & BT848_CAP_CTL_CAPTURE_ODD)
	{
		m_pRiscJump[11] = RiscLogToPhys(m_pRiscFrameOdd[0]);
	}
	else
	{
		m_pRiscJump[11] = RiscLogToPhys(m_pRiscJump + 12);
	}

	// 2. Bild
	m_pRiscJump[12] = (DWORD) (BT848_RISC_SYNC | BT848_RISC_RESYNC | BT848_FIFO_STATUS_VRE);
	m_pRiscJump[13] = 0;

	m_pRiscJump[14] = BT848_RISC_JUMP;
	if (nFlags & BT848_CAP_CTL_CAPTURE_VBI_EVEN)
	{
		m_pRiscJump[15] = RiscLogToPhys(m_pRiscVBIEven[1]);
	}
	else
	{
		m_pRiscJump[15] = RiscLogToPhys(m_pRiscJump + 16);
	}

	m_pRiscJump[16] = BT848_RISC_JUMP;
	if (nFlags & BT848_CAP_CTL_CAPTURE_EVEN)
	{
		m_pRiscJump[17] = RiscLogToPhys(m_pRiscFrameEven[1]);
	}
	else
	{
		m_pRiscJump[17] = RiscLogToPhys(m_pRiscJump + 18);
	}

	m_pRiscJump[18] = (DWORD) (BT848_RISC_SYNC | BT848_RISC_RESYNC | BT848_FIFO_STATUS_VRO);
	m_pRiscJump[19] = 0;

	m_pRiscJump[20] = BT848_RISC_JUMP;
	if (nFlags & BT848_CAP_CTL_CAPTURE_VBI_ODD)
	{
		m_pRiscJump[21] = RiscLogToPhys(m_pRiscVBIOdd[1]);
	}
	else
	{
		m_pRiscJump[21] = RiscLogToPhys(m_pRiscJump + 22);
	}

	m_pRiscJump[22] = BT848_RISC_JUMP;
	if (nFlags & BT848_CAP_CTL_CAPTURE_ODD)
	{
		m_pRiscJump[23] = RiscLogToPhys(m_pRiscFrameOdd[1]);
	}
	else
	{
		m_pRiscJump[23] = RiscLogToPhys(m_pRiscJump + 24);
	}

	// 3. Bild

	m_pRiscJump[24] = (DWORD) (BT848_RISC_SYNC | BT848_RISC_RESYNC | BT848_FIFO_STATUS_VRE);
	m_pRiscJump[25] = 0;

	m_pRiscJump[26] = BT848_RISC_JUMP;
	if (nFlags & BT848_CAP_CTL_CAPTURE_VBI_EVEN)
	{
		m_pRiscJump[27] = RiscLogToPhys(m_pRiscVBIEven[2]);
	}
	else
	{
		m_pRiscJump[27] = RiscLogToPhys(m_pRiscJump + 28);
	}

	m_pRiscJump[28] = BT848_RISC_JUMP;
	if (nFlags & BT848_CAP_CTL_CAPTURE_EVEN)
	{
		m_pRiscJump[29] = RiscLogToPhys(m_pRiscFrameEven[2]);
	}
	else
	{
		m_pRiscJump[29] = RiscLogToPhys(m_pRiscJump + 30);
	}

	m_pRiscJump[30] = (DWORD) (BT848_RISC_SYNC | BT848_RISC_RESYNC | BT848_FIFO_STATUS_VRO);
	m_pRiscJump[31] = 0;

	m_pRiscJump[32] = BT848_RISC_JUMP;
	if (nFlags & BT848_CAP_CTL_CAPTURE_VBI_ODD)
	{
		m_pRiscJump[33] = RiscLogToPhys(m_pRiscVBIOdd[2]);
	}
	else
	{
		m_pRiscJump[33] = RiscLogToPhys(m_pRiscJump + 34);
	}

	m_pRiscJump[34] = BT848_RISC_JUMP;
	if (nFlags & BT848_CAP_CTL_CAPTURE_ODD)
	{
		m_pRiscJump[35] = RiscLogToPhys(m_pRiscFrameOdd[2]);
	}
	else
	{
		m_pRiscJump[35] = RiscLogToPhys(m_pRiscJump);
	}

	// 4. Bild

	m_pRiscJump[36] = (DWORD) (BT848_RISC_SYNC | BT848_RISC_RESYNC | BT848_FIFO_STATUS_VRE);
	m_pRiscJump[37] = 0;

	m_pRiscJump[38] = BT848_RISC_JUMP;
	if (nFlags & BT848_CAP_CTL_CAPTURE_VBI_EVEN)
	{
		m_pRiscJump[39] = RiscLogToPhys(m_pRiscVBIEven[3]);
	}
	else
	{
		m_pRiscJump[39] = RiscLogToPhys(m_pRiscJump + 40);
	}

	m_pRiscJump[40] = BT848_RISC_JUMP;
	if (nFlags & BT848_CAP_CTL_CAPTURE_EVEN)
	{
		m_pRiscJump[41] = RiscLogToPhys(m_pRiscFrameEven[3]);
	}
	else
	{
		m_pRiscJump[41] = RiscLogToPhys(m_pRiscJump + 42);
	}

	m_pRiscJump[42] = (DWORD) (BT848_RISC_SYNC | BT848_RISC_RESYNC | BT848_FIFO_STATUS_VRO);
	m_pRiscJump[43] = 0;

	m_pRiscJump[44] = BT848_RISC_JUMP;
	if (nFlags & BT848_CAP_CTL_CAPTURE_VBI_ODD)
	{
		m_pRiscJump[45] = RiscLogToPhys(m_pRiscVBIOdd[3]);
	}
	else
	{
		m_pRiscJump[45] = RiscLogToPhys(m_pRiscJump + 46);
	}

	m_pRiscJump[46] = BT848_RISC_JUMP;
	if (nFlags & BT848_CAP_CTL_CAPTURE_ODD)
	{
		m_pRiscJump[47] = RiscLogToPhys(m_pRiscFrameOdd[3]);
	}
	else
	{
		m_pRiscJump[47] = RiscLogToPhys(m_pRiscJump);
	}

	// 5. Bild

	m_pRiscJump[48] = (DWORD) (BT848_RISC_SYNC | BT848_RISC_RESYNC | BT848_FIFO_STATUS_VRE);
	m_pRiscJump[49] = 0;

	m_pRiscJump[50] = BT848_RISC_JUMP;
	if (nFlags & BT848_CAP_CTL_CAPTURE_VBI_EVEN)
	{
		m_pRiscJump[51] = RiscLogToPhys(m_pRiscVBIEven[4]);
	}
	else
	{
		m_pRiscJump[51] = RiscLogToPhys(m_pRiscJump + 52);
	}

	m_pRiscJump[52] = BT848_RISC_JUMP;
	if (nFlags & BT848_CAP_CTL_CAPTURE_EVEN)
	{
		m_pRiscJump[53] = RiscLogToPhys(m_pRiscFrameEven[4]);
	}
	else
	{
		m_pRiscJump[53] = RiscLogToPhys(m_pRiscJump + 54);
	}

	m_pRiscJump[54] = (DWORD) (BT848_RISC_SYNC | BT848_RISC_RESYNC | BT848_FIFO_STATUS_VRO);
	m_pRiscJump[55] = 0;

	m_pRiscJump[56] = BT848_RISC_JUMP;
	if (nFlags & BT848_CAP_CTL_CAPTURE_VBI_ODD)
	{
		m_pRiscJump[57] = RiscLogToPhys(m_pRiscVBIOdd[4]);
	}
	else
	{
		m_pRiscJump[57] = RiscLogToPhys(m_pRiscJump + 58);
	}

	m_pRiscJump[58] = BT848_RISC_JUMP;
	if (nFlags & BT848_CAP_CTL_CAPTURE_ODD)
	{
		m_pRiscJump[59] = RiscLogToPhys(m_pRiscFrameOdd[4]);
	}
	else
	{
		m_pRiscJump[59] = RiscLogToPhys(m_pRiscJump);
	}

	BT8X8_WriteDword(BT8X8_AD_BAR0, BT848_RISC_STRT_ADD, RiscLogToPhys(m_pRiscJump + 2));
}

void MakeVideoTableForDisplay()
{
	DWORD *ro = m_pRiscFrameOdd[0];
	DWORD *re = m_pRiscFrameEven[0];
	DWORD **rp;
	LPBYTE pLinDisplay;

	PHYS pPhysDisplay;
	DWORD GotBytesPerLine;
	DWORD BytesPerLine = CurrentX * 2;	// Untested
	int i;
	int nBuffer;

	for(nBuffer = 0; nBuffer < 5; nBuffer++)
	{
		re = m_pRiscFrameEven[nBuffer];
		ro = m_pRiscFrameOdd[nBuffer];
		pLinDisplay = (LPBYTE) pDisplay[nBuffer];

		*(re++) = (BT848_RISC_SYNC | BT848_FIFO_STATUS_FM1);
		*(re++) = 0;
		*(ro++) = (BT848_RISC_SYNC | BT848_FIFO_STATUS_FM1);	//|(14<<20));
		*(ro++) = 0;

		for (i = 0; i < CurrentY; i++)
		{
			rp = (i & 1) ? &ro : &re;

			pPhysDisplay = GetPhysicalAddress(Display_dma[nBuffer], pLinDisplay, BytesPerLine, &GotBytesPerLine);
			if(pPhysDisplay == 0)
			{
				return;
			}
			if (GotBytesPerLine < BytesPerLine)
			{
				DWORD BytesToGet = BytesPerLine - GotBytesPerLine;
				*((*rp)++) = BT848_RISC_WRITE | BT848_RISC_SOL | GotBytesPerLine;
				*((*rp)++) = pPhysDisplay;
				// Assumes lines aren't >8K long!
				pPhysDisplay = GetPhysicalAddress(Display_dma[nBuffer], pLinDisplay + GotBytesPerLine, BytesToGet, &GotBytesPerLine);
				if(pPhysDisplay == 0 || BytesToGet > GotBytesPerLine)
				{
					return;
				}
				*((*rp)++) = BT848_RISC_WRITE | BT848_RISC_EOL | BytesToGet;
				*((*rp)++) = pPhysDisplay;
			}
			else
			{
				*((*rp)++) = BT848_RISC_WRITE | BT848_RISC_SOL | BT848_RISC_EOL | BytesPerLine;
				*((*rp)++) = pPhysDisplay;
			}
			pLinDisplay += 2048;
		}
		*(re++) = BT848_RISC_JUMP | ((0xF0) << 16);
		*(re++) = RiscLogToPhys(m_pRiscJump + 6 + nBuffer * 12);
		*(ro++) = BT848_RISC_JUMP | ((0xF1 + nBuffer) << 16);
		if(nBuffer != 4)
		{
			*(ro++) = RiscLogToPhys(m_pRiscJump + 12 + nBuffer * 12);
		}
		else
		{
			*(ro++) = RiscLogToPhys(m_pRiscJump);
		}
	}
}

void ExitDD(void)
{
	if (lpDD != NULL)
	{
		if (lpDDOverlay != NULL)
		{
			OverlayUpdate(NULL, NULL, DDOVER_HIDE, FALSE);
			IDirectDrawSurface_Release(lpDDOverlay);
		}
		lpDDOverlay = NULL;
		if (lpDDSurface != NULL)
		{
			IDirectDrawSurface_Release(lpDDSurface);
		}
		lpDDSurface = NULL;
		IDirectDraw_Release(lpDD);
		lpDD = NULL;
	}
}

void Black_Surface()
{
	int nPixel;
	int nZeile;
	HRESULT ddrval;
	DDSURFACEDESC ddsd;

	if (lpDDOverlay == NULL)
	{
		return;
	}

	memset(&ddsd, 0x00, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	ddrval = IDirectDrawSurface_Lock(lpDDOverlay, NULL, &ddsd, DDLOCK_WAIT, NULL);

	for (nZeile = 0; nZeile < (signed) ddsd.dwHeight; nZeile++)
	{
		for (nPixel = 0; nPixel < (signed) ddsd.dwWidth * 2; nPixel += 4)
		{
			*((int *) ddsd.lpSurface + (nZeile * ddsd.lPitch + nPixel) / 4) = 0x80008000;
		}
	}
	ddrval = IDirectDrawSurface_Unlock(lpDDOverlay, ddsd.lpSurface);
}

void Black_Overlays()
{
	BYTE *ScreenPtr;
	int nLineTarget;

	// blank out front and back buffers
	for (nLineTarget = 0; nLineTarget < CurrentY / 2; nLineTarget++)
	{
		ScreenPtr = lpOverlay + (nLineTarget * 2) * OverlayPitch;
		memset(ScreenPtr, 0, CurrentX  * 2);
		ScreenPtr = lpOverlay + (nLineTarget * 2 + 1) * OverlayPitch;
		memset(ScreenPtr, 0, CurrentX  * 2);
		ScreenPtr = lpOverlayBack + (nLineTarget * 2) * OverlayPitch;
		memset(ScreenPtr, 0, CurrentX  * 2);
		ScreenPtr = lpOverlayBack + (nLineTarget * 2 + 1) * OverlayPitch;
		memset(ScreenPtr, 0, CurrentX  * 2);
	}
}

BOOL OverlayUpdate(LPRECT pSrcRect, LPRECT pDestRect, DWORD dwFlags, BOOL ColorKey)
{
	HRESULT ddrval;
	DDOVERLAYFX DDOverlayFX;
	int i = 0;

	if ((lpDD == NULL) || (lpDDSurface == NULL) || (lpDDOverlay == NULL))
	{
		return (FALSE);
	}

	memset(&DDOverlayFX, 0x00, sizeof(DDOverlayFX));
	DDOverlayFX.dwSize = sizeof(DDOverlayFX);

	if (pSrcRect == NULL)
	{
		ddrval = IDirectDrawSurface_UpdateOverlay(lpDDOverlay, NULL, lpDDSurface, NULL, dwFlags, &DDOverlayFX);
		return (TRUE);
	}

	i = dwFlags;

	dwFlags |= DDOVER_KEYDESTOVERRIDE;
	DDOverlayFX.dckDestColorkey.dwColorSpaceHighValue = 0;
	DDOverlayFX.dckDestColorkey.dwColorSpaceLowValue = 0;

	ddrval = IDirectDrawSurface_UpdateOverlay(lpDDOverlay, pSrcRect, lpDDSurface, pDestRect, dwFlags, &DDOverlayFX);
	if (ddrval != DD_OK)
	{
		dwFlags = i;
		memset(&DDOverlayFX, 0x00, sizeof(DDOverlayFX));
		DDOverlayFX.dwSize = sizeof(DDOverlayFX);

		ddrval = IDirectDrawSurface_UpdateOverlay(lpDDOverlay, pSrcRect, lpDDSurface, pDestRect, dwFlags, &DDOverlayFX);
		if (ddrval != DD_OK)
		{
			ddrval = IDirectDrawSurface_UpdateOverlay(lpDDOverlay, pSrcRect, lpDDSurface, pDestRect, dwFlags, &DDOverlayFX);

			if (ddrval != DD_OK)
			{
				return (FALSE);
			}
		}
	}

	return TRUE;
}

BOOL CreateOverlay()
{
	DDSURFACEDESC ddsd;
	DDPIXELFORMAT PixelFormat;
	HRESULT ddrval;
	DDSCAPS caps;

	memset(&PixelFormat, 0x00, sizeof(PixelFormat));
	PixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	PixelFormat.dwFlags = DDPF_FOURCC;
	PixelFormat.dwFourCC = MAKEFOURCC('Y', 'U', 'Y', '2');;
	PixelFormat.dwYUVBitCount = 16;

	memset(&ddsd, 0x00, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY | DDSCAPS_FLIP | DDSCAPS_COMPLEX;

	// create a much bigger surface than we need
	// this ensures that we can use the bTV plugin 
	ddsd.dwWidth = BTV_VER1_WIDTH;
	ddsd.dwHeight = BTV_VER1_HEIGHT;
	ddsd.dwBackBufferCount = 1;

	ddsd.ddpfPixelFormat = PixelFormat;
	if (IDirectDraw_CreateSurface(lpDD, &ddsd, &lpDDOverlay, NULL) != DD_OK)
	{
		MessageBox(NULL, "Can't create Overlay Surface", "dTV", MB_ICONSTOP | MB_OK);
		lpDDOverlay = NULL;
		return FALSE;
	}
	ddrval = IDirectDrawSurface_Lock(lpDDOverlay, NULL, &ddsd, 0, NULL);
	OverlayPitch = ddsd.lPitch;
	lpOverlay = ddsd.lpSurface;
	ddrval = IDirectDrawSurface_Unlock(lpDDOverlay, ddsd.lpSurface);
	
	caps.dwCaps = DDSCAPS_BACKBUFFER;
	ddrval = IDirectDrawSurface_GetAttachedSurface(lpDDOverlay, &caps, &lpDDOverlayBack);
	if (FAILED(ddrval))
	{
		MessageBox(NULL, "Can't create Overlay Back Surface", "dTV", MB_ICONSTOP | MB_OK);
		lpDDOverlayBack = NULL;
		return (FALSE);
	}
	else
	{
		ddrval = IDirectDrawSurface_Lock(lpDDOverlayBack, NULL, &ddsd, 0, NULL);
		lpOverlayBack = ddsd.lpSurface;
		ddrval = IDirectDrawSurface_Unlock(lpDDOverlayBack, ddsd.lpSurface);
	}

	return (TRUE);
}

BOOL InitDD(HWND hWnd)
{
	HRESULT ddrval;
	DDCAPS DriverCaps;
	DDSURFACEDESC ddsd;

	if (DirectDrawCreate(NULL, &lpDD, NULL) != DD_OK)
	{
		MessageBox(NULL, "DirectDrawCreate failed", "dTV", MB_ICONSTOP | MB_OK);
		return (FALSE);
	}

	// can we use Overlay ??
	memset(&DriverCaps, 0x00, sizeof(DriverCaps));
	DriverCaps.dwSize = sizeof(DriverCaps);
	ddrval = IDirectDraw_GetCaps(lpDD, &DriverCaps, NULL);

	if (ddrval == DD_OK)
	{
		if (DriverCaps.dwCaps & DDCAPS_OVERLAY)
		{
			if (!(DriverCaps.dwCaps & DDCAPS_OVERLAYSTRETCH))
			{
				MessageBox(NULL, "Can't Strech Overlay", "dTV", MB_ICONSTOP | MB_OK);
				return FALSE;
			}
			
			if (!(DriverCaps.dwCKeyCaps & DDCKEYCAPS_DESTOVERLAY))
			{
				MessageBox(NULL, "Can't ColorKey Overlay", "dTV", MB_ICONSTOP | MB_OK);
				return FALSE;
			}
		}
		else
		{
			MessageBox(NULL, "Can't Use Overlay", "dTV", MB_ICONSTOP | MB_OK);
			return (FALSE);
		}
	}

	ddrval = IDirectDraw_SetCooperativeLevel(lpDD, hWnd, DDSCL_NORMAL);

	if (ddrval != DD_OK)
	{
		MessageBox(NULL, "SetCooperativeLevel failed", "dTV", MB_ICONSTOP | MB_OK);
		return (FALSE);
	}

	memset(&ddsd, 0x00, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	if (IDirectDraw_CreateSurface(lpDD, &ddsd, &lpDDSurface, NULL) != DD_OK)
	{
		MessageBox(NULL, "Error Creating Primary surface", "dTV", MB_ICONSTOP | MB_OK);
		return (FALSE);
	}

	ddrval = IDirectDrawSurface_Lock(lpDDSurface, NULL, &ddsd, 0, NULL);
	ddrval = IDirectDrawSurface_Unlock(lpDDSurface, ddsd.lpSurface);

	return TRUE;
}

BOOL Init_Tuner(int TunerNr)
{
	unsigned char j;

	if (USETUNER == FALSE)
		return (TRUE);

	InitializeCriticalSection(&m_cCrit);

	j = 0xc0;
	TunerDeviceRead = Tuners[TunerNr].I2C = j;
	TunerDeviceWrite = Tuners[TunerNr].I2C = j;

	while ((j <= 0xce) && (I2CBus_AddDevice((BYTE) j) == FALSE))
	{
		j++;
		TunerDeviceRead = Tuners[TunerNr].I2C = j;
		TunerDeviceWrite = Tuners[TunerNr].I2C = j;
	}

	if (j > 0xce)
	{
		return (FALSE);
	}
	sprintf(TunerStatus, "Tuner I2C-Bus I/O 0x%02x", j);
	return (TRUE);
}

void I2C_SetLine(BOOL bCtrl, BOOL bData)
{
	BT8X8_WriteDword(BT8X8_AD_BAR0, BT848_I2C, (bCtrl << 1) | bData);
	I2CBus_wait(I2C_DELAY);
}

BOOL I2C_GetLine()
{
	return BT8X8_ReadDword(BT8X8_AD_BAR0, BT848_I2C) & 1;
}

BYTE I2C_Read(BYTE nAddr)
{
	DWORD i;
	volatile DWORD stat;

	BT8X8_WriteDword(BT8X8_AD_BAR0, BT848_INT_STAT, BT848_INT_I2CDONE);
	BT8X8_WriteDword(BT8X8_AD_BAR0, BT848_I2C, (nAddr << 24) | I2C_COMMAND);

	for (i = 0x7fffffff; i; i--)
	{
		stat = BT8X8_ReadDword(BT8X8_AD_BAR0, BT848_INT_STAT);
		if (stat & BT848_INT_I2CDONE)
			break;
	}

	if (!i)
		return (BYTE) - 1;
	if (!(stat & BT848_INT_RACK))
		return (BYTE) - 2;

	return (BYTE) ((BT8X8_ReadDword(BT8X8_AD_BAR0, BT848_I2C) >> 8) & 0xFF);
}

BOOL I2C_Write(BYTE nAddr, BYTE nData1, BYTE nData2, BOOL bSendBoth)
{
	DWORD i;
	DWORD data;
	DWORD stat;

	BT8X8_WriteDword(BT8X8_AD_BAR0, BT848_INT_STAT, BT848_INT_I2CDONE);

	data = (nAddr << 24) | (nData1 << 16) | I2C_COMMAND;
	if (bSendBoth)
		data |= (nData2 << 8) | BT848_I2C_W3B;
	BT8X8_WriteDword(BT8X8_AD_BAR0, BT848_I2C, data);

	for (i = 0x7fffffff; i; i--)
	{
		stat = BT8X8_ReadDword(BT8X8_AD_BAR0, BT848_INT_STAT);
		if (stat & BT848_INT_I2CDONE)
			break;
	}

	if (!i)
		return FALSE;
	if (!(stat & BT848_INT_RACK))
		return FALSE;

	return TRUE;
}

//----------------------------------------------------------------

BOOL I2CBus_AddDevice(BYTE I2C_Port)
{
	BOOL bAck;

	// Test whether device exists
	I2CBus_Lock();
	I2CBus_Start();
	bAck = I2CBus_SendByte(I2C_Port, 0);
	I2CBus_Stop();
	I2CBus_Unlock();
	if (bAck)
		return TRUE;
	else
		return FALSE;
}

BOOL I2CBus_Lock()
{
	EnterCriticalSection(&m_cCrit);
	return TRUE;
}

BOOL I2CBus_Unlock()
{
	LeaveCriticalSection(&m_cCrit);
	return TRUE;
}

void I2CBus_Start()
{
	I2C_SetLine(0, 1);
	I2C_SetLine(1, 1);
	I2C_SetLine(1, 0);
	I2C_SetLine(0, 0);
}

void I2CBus_Stop()
{
	I2C_SetLine(0, 0);
	I2C_SetLine(1, 0);
	I2C_SetLine(1, 1);
}

void I2CBus_One()
{
	I2C_SetLine(0, 1);
	I2C_SetLine(1, 1);
	I2C_SetLine(0, 1);
}

void I2CBus_Zero()
{
	I2C_SetLine(0, 0);
	I2C_SetLine(1, 0);
	I2C_SetLine(0, 0);
}

BOOL I2CBus_Ack()
{
	BOOL bAck;

	I2C_SetLine(0, 1);
	I2C_SetLine(1, 1);
	bAck = !I2C_GetLine();
	I2C_SetLine(0, 1);
	return bAck;
}

BOOL I2CBus_SendByte(BYTE nData, int nWaitForAck)
{
	I2C_SetLine(0, 0);
	nData & 0x80 ? I2CBus_One() : I2CBus_Zero();
	nData & 0x40 ? I2CBus_One() : I2CBus_Zero();
	nData & 0x20 ? I2CBus_One() : I2CBus_Zero();
	nData & 0x10 ? I2CBus_One() : I2CBus_Zero();
	nData & 0x08 ? I2CBus_One() : I2CBus_Zero();
	nData & 0x04 ? I2CBus_One() : I2CBus_Zero();
	nData & 0x02 ? I2CBus_One() : I2CBus_Zero();
	nData & 0x01 ? I2CBus_One() : I2CBus_Zero();
	if (nWaitForAck)
		I2CBus_wait(nWaitForAck);
	return I2CBus_Ack();
}

BYTE I2CBus_ReadByte(BOOL bLast)
{
	int i;
	BYTE bData = 0;

	I2C_SetLine(0, 1);
	for (i = 7; i >= 0; i--)
	{
		I2C_SetLine(1, 1);
		if (I2C_GetLine())
			bData |= (1 << i);
		I2C_SetLine(0, 1);
	}

	bLast ? I2CBus_One() : I2CBus_Zero();
	return bData;
}

BYTE I2CBus_Read(BYTE nAddr)
{
	BYTE bData;

	I2CBus_Start();
	I2CBus_SendByte(nAddr, 0);
	bData = I2CBus_ReadByte(TRUE);
	I2CBus_Stop();
	return bData;
}

BOOL I2CBus_Write(BYTE nAddr, BYTE nData1, BYTE nData2, BOOL bSendBoth)
{
	BOOL bAck;

	I2CBus_Start();
	I2CBus_SendByte(nAddr, 0);
	bAck = I2CBus_SendByte(nData1, 0);
	if (bSendBoth)
		bAck = I2CBus_SendByte(nData2, 0);
	I2CBus_Stop();
	return bAck;
}

void I2CBus_wait(int us)
{
	if (us > 0)
	{
		Sleep(us);
		return;
	}
	Sleep(0);
	Sleep(0);
	Sleep(0);
	Sleep(0);
	Sleep(0);
}

/*
 *	Set TSA5522 synthesizer frequency 
 */
BOOL Tuner_SetFrequency(int TunerTyp, int wFrequency)
{
	BYTE config;
	WORD div;
	BOOL bAck;

	if (USETUNER == FALSE)
		return (TRUE);

	if (wFrequency < Tuners[TunerTyp].thresh1)
		config = Tuners[TunerTyp].VHF_L;
	else if (wFrequency < Tuners[TunerTyp].thresh2)
		config = Tuners[TunerTyp].VHF_H;
	else
		config = Tuners[TunerTyp].UHF;

	div = wFrequency + Tuners[TunerTyp].IFPCoff;

	div &= 0x7fff;
	I2CBus_Lock();				// Lock/wait
	if (!I2CBus_Write((BYTE) TunerDeviceWrite, (BYTE) ((div >> 8) & 0x7f), (BYTE) (div & 0xff), TRUE))
	{
		Sleep(1);
		if (!I2CBus_Write((BYTE) TunerDeviceWrite, (BYTE) ((div >> 8) & 0x7f), (BYTE) (div & 0xff), TRUE))
		{
			Sleep(1);
			if (!I2CBus_Write((BYTE) TunerDeviceWrite, (BYTE) ((div >> 8) & 0x7f), (BYTE) (div & 0xff), TRUE))
			{
				MessageBox(hWnd, "Tuner Device : Error Writing (1)", "dTV", MB_ICONSTOP | MB_OK);
				I2CBus_Unlock();	// Unlock
				return (FALSE);
			}
		}
	}
	if (!(bAck = I2CBus_Write(TunerDeviceWrite, Tuners[TunerTyp].config, config, TRUE)))
	{
		Sleep(1);
		if (!(bAck = I2CBus_Write(TunerDeviceWrite, Tuners[TunerTyp].config, config, TRUE)))
		{
			Sleep(1);
			if (!(bAck = I2CBus_Write(TunerDeviceWrite, Tuners[TunerTyp].config, config, TRUE)))
			{
				MessageBox(hWnd, "Tuner Device : Error Writing (2)", "dTV", MB_ICONSTOP | MB_OK);
			}
		}
	}
	I2CBus_Unlock();			// Unlock
	if (!bAck)
		return FALSE;
	return TRUE;
}

void Load_Country_Settings()
{
	FILE *iniFile;
	char zeile[128];
	char *Pos;
	char *Pos1;
	char *SemmelPos;
	unsigned int i;

	for (i = 0; i < 32; i++)
	{
		Countries[i].Name[0] = 0x00;
	}

	if ((iniFile = fopen("Channel.lst", "r")) == NULL)
	{
		MessageBox(hWnd, "File Channel.lst not Found", "dTV", MB_ICONSTOP | MB_OK);
		return;
	}
	i = 0;

	while (fgets(zeile, sizeof(zeile), iniFile) != NULL)
	{
		if (i >= 35)
		{
			MessageBox(hWnd, "File Channel.lst has more than 35 settings!\nThe extra ones are ingnored.", "dTV", MB_ICONSTOP | MB_OK);
			fclose(iniFile);
			return;
		}
		SemmelPos = strstr(zeile, ";");
		if (SemmelPos == NULL)
			SemmelPos = strstr(zeile, "\n");

		if (((Pos = strstr(zeile, "[")) != 0) && (SemmelPos > Pos) && ((Pos1 = strstr(zeile, "]")) != 0))
		{

			Pos++;
			Pos1 = &Countries[i].Name[0];
			i++;
			while (*Pos != ']')
			{
				*(Pos1++) = *(Pos++);
				*Pos1 = 0x00;
			}
		}
	}

	fclose(iniFile);

}

void Load_Country_Specific_Settings(int LPos)
{
	unsigned short i, j, k;
	FILE *iniFile;
	char zeile[128];
	char txt[128];
	char *Pos;
	char *Pos1;
	char *SemmelPos;

	if ((iniFile = fopen("Channel.lst", "r")) == NULL)
	{
		MessageBox(hWnd, "File Channel.lst not found", "dTV", MB_ICONSTOP | MB_OK);
		return;
	}
	i = 0;
	k = 0;

	while (fgets(zeile, sizeof(zeile), iniFile) != NULL)
	{

		SemmelPos = strstr(zeile, ";");
		if (SemmelPos == NULL)
			SemmelPos = strstr(zeile, "\n");

		sprintf(txt, "[%s]", Countries[LPos].Name);

		if (strstr(zeile, txt) != 0)
		{
			strcpy(Channels.Name, Countries[LPos].Name);
			while (fgets(zeile, sizeof(zeile), iniFile) != NULL)
			{
				SemmelPos = strstr(zeile, ";");
				if (SemmelPos == NULL)
					SemmelPos = strstr(zeile, "\n");

				if (((Pos = strstr(zeile, "[")) != 0) && (SemmelPos > Pos) && ((Pos1 = strstr(zeile, "]")) != 0))
				{
					fclose(iniFile);
					return;
				}

				if (((Pos = strstr(zeile, "KanalLow=")) != 0) && (SemmelPos > Pos))
				{
					Pos = Pos + 9;
					j = 0;
					txt[j] = 0x00;
					while (Pos < SemmelPos)
					{
						if (*Pos != 0x20)
						{
							txt[j] = *Pos;
							j++;
							txt[j] = 0x00;
						}
						Pos++;
					}
					Channels.MinChannel = atoi(txt);
				}
				else if (((Pos = strstr(zeile, "KanalHigh=")) != 0) && (SemmelPos > Pos))
				{
					Pos = Pos + 10;
					j = 0;
					txt[j] = 0x00;
					while (Pos < SemmelPos)
					{
						if (*Pos != 0x20)
						{
							txt[j] = *Pos;
							j++;
							txt[j] = 0x00;
						}
						Pos++;
					}
					Channels.MaxChannel = atoi(txt);
				}
				else
				{
					Pos = &zeile[0];
					j = 0;
					txt[j] = 0x00;
					while (Pos < SemmelPos)
					{
						if ((*Pos >= '0') && (*Pos <= '9'))
						{
							txt[j] = *Pos;
							j++;
							txt[j] = 0x00;
						}
						Pos++;
					}
					if (txt[0] != 0x00)
					{
						Channels.freq[k] = atol(txt);
						Channels.freq[k] = Channels.freq[k] / 1000;
						k++;
					}
				}
			}
		}
	}
	fclose(iniFile);
	return;
}

// Audio

BOOL Init_Audio(BYTE DWrite, BYTE DRead)
{
	int i;

	AudioDeviceWrite = DWrite;
	AudioDeviceRead = DRead;

	InitializeCriticalSection(&m_cCrit);

	Has_MSP = TRUE;

	if (!I2CBus_AddDevice(DRead))
	{
		Has_MSP = FALSE;
		return (FALSE);
	}

	if (!I2CBus_AddDevice(DWrite))
	{
		Has_MSP = FALSE;
		return (FALSE);
	}

	sprintf(MSPStatus, "MSP-Device I2C-Bus I/O 0x80/0x81");

	MSP_Reset();
	Sleep(4);
	MSP_Version();
	Sleep(4);
	MSP_SetMode(MSPMode);
	Sleep(4);
	MSP_Set_MajorMinor_Mode(MSPMajorMode, MSPMinorMode);
	Sleep(4);
	MSP_SetStereo(MSPMajorMode, MSPMinorMode, MSPStereo);

	MSPToneControl = TRUE;
	Audio_SetVolume(InitialVolume);
	Audio_SetBalance(InitialBalance);
	Audio_SetSuperBass(InitialSuperBass);
	Audio_SetBass(InitialBass);
	Audio_SetTreble(InitialTreble);
	Audio_SetLoudness(InitialLoudness);
	Audio_SetSpatial(InitialSpatial);
	for(i = 0; i < 5; i++)
	{
		Audio_SetEqualizer(i, InitialEqualizer[i]);
	}
	return (TRUE);
}

BOOL Audio_WriteMSP(BYTE bSubAddr, int wAddr, int wData)
{
	I2CBus_Lock();
	I2CBus_Start();
	I2CBus_SendByte(AudioDeviceWrite, 0);
	I2CBus_SendByte(bSubAddr, 0);
	if (bSubAddr != MSP_CONTROL && bSubAddr != MSP_TEST)
	{
		I2CBus_SendByte((BYTE) (wAddr >> 8), 0);
		I2CBus_SendByte((BYTE) (wAddr & 0xFF), 0);
	}
	I2CBus_SendByte((BYTE) (wData >> 8), 0);
	I2CBus_SendByte((BYTE) (wData & 0xFF), 0);
	I2CBus_Stop();
	I2CBus_Unlock();
	return TRUE;
}

WORD Audio_ReadMSP(BYTE bSubAddr, WORD wAddr)
{
	WORD wResult;
	BYTE B0, B1;

	B0 = (BYTE) (wAddr >> 8);
	B1 = (BYTE) (wAddr & 0xFF);
	I2CBus_Lock();
	I2CBus_Start();
	I2CBus_SendByte(AudioDeviceWrite, 2);
	I2CBus_SendByte(bSubAddr, 0);
	I2CBus_SendByte(B0, 0);
	I2CBus_SendByte(B1, 0);
	I2CBus_Start();
	if (I2CBus_SendByte(AudioDeviceRead, 2))
	{
		B0 = I2CBus_ReadByte(0);
		B1 = I2CBus_ReadByte(1);
		wResult = B0 << 8 | B1;
	}
	else
	{
		wResult = -1;
	}
	I2CBus_Stop();
	I2CBus_Unlock();
	return wResult;
}

BOOL Audio_SetVolume(int nVolume)
{
	Audio_SetToneControl(TRUE);
	if (nVolume < 0 || nVolume > 1000)
		return FALSE;
	InitialVolume = nVolume;
	if (nVolume > 0)
		nVolume = MulDiv(nVolume + 400, 0x7f0, 1400);
	WriteDSP(0, nVolume << 4);
	WriteDSP(6, nVolume << 4);

	return TRUE;
}

BOOL Audio_SetBalance(char nBalance)
{
	Audio_SetToneControl(TRUE);
	InitialBalance = nBalance;
	WriteDSP(1, nBalance << 8);
	WriteDSP(0x30, nBalance << 8);
	return TRUE;
}

BOOL Audio_SetBass(char nBass)
{
	Audio_SetToneControl(TRUE);
	if (nBass < -96)
		return FALSE;
	InitialBass = nBass;
	WriteDSP(2, nBass << 8);
	WriteDSP(0x31, nBass << 8);
	return TRUE;
}

BOOL Audio_SetTreble(char nTreble)
{
	Audio_SetToneControl(TRUE);
	if (nTreble < -96)
		return FALSE;
	InitialTreble = nTreble;
	WriteDSP(3, nTreble << 8);
	WriteDSP(0x32, nTreble << 8);
	return TRUE;
}

BOOL Audio_SetLoudness(BYTE nLoudness)
{
	Audio_SetToneControl(TRUE);
	if (nLoudness > 68)
		return FALSE;
	InitialLoudness = nLoudness;
	WriteDSP(4, (nLoudness << 8) + (InitialSuperBass ? 0x4 : 0));
	WriteDSP(0x33, (nLoudness << 8) + (InitialSuperBass ? 0x4 : 0));
	return TRUE;
}

BOOL Audio_SetSuperBass(BOOL bSuperBass)
{
	Audio_SetToneControl(TRUE);
	InitialSuperBass = bSuperBass;
	WriteDSP(4, (InitialLoudness << 8) + (bSuperBass ? 0x4 : 0));
	WriteDSP(0x33, (InitialLoudness << 8) + (bSuperBass ? 0x4 : 0));
	return TRUE;
}

BOOL Audio_SetSpatial(char nSpatial)
{
	Audio_SetToneControl(TRUE);
	InitialSpatial = nSpatial;
	WriteDSP(0x5, (nSpatial << 8) + 0x8);	// Mode A, Automatic high pass gain
	return TRUE;
}

BOOL Audio_SetEqualizer(int nIndex, char nLevel)
{
	Audio_SetToneControl(FALSE);
	if (nLevel < -96 || nLevel > 96)
		return FALSE;
	InitialEqualizer[nIndex] = nLevel;
	WriteDSP(0x21 + nIndex, nLevel << 8);
	return TRUE;
}

void Audio_SetToneControl(BOOL nMode)
{
	int i;

	if (nMode == MSPToneControl)
		return;
	MSPToneControl = nMode;
	if (MSPToneControl == TRUE)
	{
		WriteDSP(2, InitialBass << 8);	// Bass
		WriteDSP(0x31, InitialBass << 8);
		WriteDSP(3, InitialTreble << 8);	// Treble
		WriteDSP(0x32, InitialTreble << 8);
		for(i = 0; i < 5; i++)
		{
			WriteDSP(0x21 + i, InitialEqualizer[i]);	// Eq
		}
		WriteDSP(0x20, 0);		// Mode control here (need eq=0)
	}
	else
	{
		WriteDSP(0x20, 0xFF << 8);	// Mode control here (need eq=0)
		WriteDSP(2, 0);			// Bass
		WriteDSP(0x31, 0);
		WriteDSP(3, 0);			// Treble
		WriteDSP(0x32, 0);
		for(i = 0; i < 5; i++)
		{
			WriteDSP(0x21 + i, InitialEqualizer[i]);	// Eq
		}
	}
}

/****************************************************************************
*
*    FUNCTION: LoadDeviceDriver( const TCHAR, const TCHAR, HANDLE *)
*
*    PURPOSE: Registers a driver with the system configuration manager 
*	 and then loads it.
*
****************************************************************************/
BOOL LoadDeviceDriver(const TCHAR * Name, const TCHAR * Path, HANDLE * lphDevice, BOOL Install)
{
	SC_HANDLE schSCManager;
	BOOL okay;

	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	// Ignore success of installation: it may already be installed.
	if (Install == TRUE)
		InstallDriver(schSCManager, Name, Path);

	// Ignore success of start: it may already be started.
	okay = StartDriver(schSCManager, Name);

	CloseServiceHandle(schSCManager);

	return okay;
}

/****************************************************************************
*
*    FUNCTION: InstallDriver( IN SC_HANDLE, IN LPCTSTR, IN LPCTSTR)
*
*    PURPOSE: Creates a driver service.
*
****************************************************************************/
BOOL InstallDriver(IN SC_HANDLE SchSCManager, IN LPCTSTR DriverName, IN LPCTSTR ServiceExe)
{
	SC_HANDLE schService;

	//
	// NOTE: This creates an entry for a standalone driver. If this
	//       is modified for use with a driver that requires a Tag,
	//       Group, and/or Dependencies, it may be necessary to
	//       query the registry for existing driver information
	//       (in order to determine a unique Tag, etc.).
	//

	schService = CreateService(SchSCManager,	// SCManager database
							   DriverName,	// name of service
							   DriverName,	// name to display
							   SERVICE_ALL_ACCESS,	// desired access
							   SERVICE_KERNEL_DRIVER,	// service type
							   SERVICE_DEMAND_START,	// start type
							   SERVICE_ERROR_NORMAL,	// error control type
							   ServiceExe,	// service's binary
							   NULL,	// no load ordering group
							   NULL,	// no tag identifier
							   NULL,	// no dependencies
							   NULL,	// LocalSystem account
							   NULL	// no password
		);

	if (schService == NULL)
	{
		DWORD x = GetLastError();

		if (x == ERROR_SERVICE_EXISTS)
		{
			SC_HANDLE schService;
			BOOL ret;

			schService = OpenService(SchSCManager, DriverName, SERVICE_CHANGE_CONFIG);
			ret = ChangeServiceConfig(schService, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, ServiceExe, NULL, NULL, NULL, DriverName, NULL, DriverName);
			if (ret)
				return TRUE;
		}
		return FALSE;
	}

	CloseServiceHandle(schService);

	return TRUE;
}

/****************************************************************************
*
*    FUNCTION: StartDriver( IN SC_HANDLE, IN LPCTSTR)
*
*    PURPOSE: Starts the driver service.
*
****************************************************************************/
BOOL StartDriver(IN SC_HANDLE SchSCManager, IN LPCTSTR DriverName)
{
	SC_HANDLE schService;
	BOOL ret;
	SERVICE_STATUS ServiceStatus;

	schService = OpenService(SchSCManager, "WinDriver", SERVICE_ALL_ACCESS);
	if (schService != NULL)
	{
		// WinDriver Installation gefunden 
		(void) QueryServiceStatus(schService, &ServiceStatus);
		CloseServiceHandle(schService);
		if (ServiceStatus.dwCurrentState == SERVICE_RUNNING)
			return (TRUE);
	}

	schService = OpenService(SchSCManager, DriverName, SERVICE_ALL_ACCESS);
	if (schService == NULL)
		return FALSE;

	if (QueryServiceStatus(schService, &ServiceStatus) == FALSE)
		return (FALSE);

	if (ServiceStatus.dwCurrentState == SERVICE_RUNNING)
		return (TRUE);

	ret = StartService(schService, 0, NULL);

	if (ret == FALSE)
	{
		DWORD error = GetLastError();

		if (error == ERROR_SERVICE_ALREADY_RUNNING)
		{
			ret = TRUE;
		}
		else
		{
			RemoveDriver(SchSCManager, DriverName);
		}
	}

	CloseServiceHandle(schService);

	return ret;
}

/****************************************************************************
*
*    FUNCTION: OpenDevice( IN LPCTSTR, HANDLE *)
*
*    PURPOSE: Opens the device and returns a handle if desired.
*
****************************************************************************/
BOOL OpenDevice(IN LPCTSTR DriverName, HANDLE * lphDevice)
{
	TCHAR completeDeviceName[64];
	HANDLE hDevice;

	//
	// Create a \\.\XXX device name that CreateFile can use
	//
	// NOTE: We're making an assumption here that the driver
	//       has created a symbolic link using it's own name
	//       (i.e. if the driver has the name "XXX" we assume
	//       that it used IoCreateSymbolicLink to create a
	//       symbolic link "\DosDevices\XXX". Usually, there
	//       is this understanding between related apps/drivers.
	//
	//       An application might also peruse the DEVICEMAP
	//       section of the registry, or use the QueryDosDevice
	//       API to enumerate the existing symbolic links in the
	//       system.
	//

	wsprintf(completeDeviceName, TEXT("\\\\.\\%s"), DriverName);

	hDevice = CreateFile(completeDeviceName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDevice == ((HANDLE) - 1))
		return FALSE;

	// If user wants handle, give it to them.  Otherwise, just close it.
	if (lphDevice)
		*lphDevice = hDevice;
	else
		CloseHandle(hDevice);

	return TRUE;
}

/****************************************************************************
*
*    FUNCTION: UnloadDeviceDriver( const TCHAR *)
*
*    PURPOSE: Stops the driver and has the configuration manager unload it.
*
****************************************************************************/
BOOL UnloadDeviceDriver(const TCHAR * Name, BOOL DRemove)
{
	SC_HANDLE schSCManager;

	schSCManager = OpenSCManager(NULL,	// machine (NULL == local)
								 NULL,	// database (NULL == default)
								 SC_MANAGER_ALL_ACCESS	// access required
		);

	StopDriver(schSCManager, Name);
	if (DRemove == TRUE)
		RemoveDriver(schSCManager, Name);

	CloseServiceHandle(schSCManager);

	return TRUE;
}

/****************************************************************************
*
*    FUNCTION: StopDriver( IN SC_HANDLE, IN LPCTSTR)
*
*    PURPOSE: Has the configuration manager stop the driver (unload it)
*
****************************************************************************/
BOOL StopDriver(IN SC_HANDLE SchSCManager, IN LPCTSTR DriverName)
{
	SC_HANDLE schService;
	BOOL ret;
	SERVICE_STATUS serviceStatus;

	schService = OpenService(SchSCManager, DriverName, SERVICE_ALL_ACCESS);
	if (schService == NULL)
		return FALSE;

	ret = ControlService(schService, SERVICE_CONTROL_STOP, &serviceStatus);

	CloseServiceHandle(schService);

	return ret;
}

/****************************************************************************
*
*    FUNCTION: RemoveDriver( IN SC_HANDLE, IN LPCTSTR)
*
*    PURPOSE: Deletes the driver service.
*
****************************************************************************/
BOOL RemoveDriver(IN SC_HANDLE SchSCManager, IN LPCTSTR DriverName)
{
	SC_HANDLE schService;
	BOOL ret;

	schService = OpenService(SchSCManager, DriverName, SERVICE_ALL_ACCESS);

	if (schService == NULL)
		return FALSE;

	ret = DeleteService(schService);

	CloseServiceHandle(schService);

	return ret;
}

/****************************************************************************
*
*    FUNCTION: VBI_Driver_Stuff
*
****************************************************************************/


BOOL APIENTRY ChipSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_TEXT6, BTVendorID);
		SetDlgItemText(hDlg, IDC_TEXT7, BTDeviceID);

		SetDlgItemText(hDlg, IDC_TEXT1, BTTyp);
		SetDlgItemText(hDlg, IDC_TEXT13, TunerStatus);
		SetDlgItemText(hDlg, IDC_TEXT14, MSPStatus);
		SetDlgItemText(hDlg, IDC_TEXT16, MSPVersion);

		SetDlgItemText(hDlg, IDC_TEXT18, "YUV2");

		break;

	case WM_COMMAND:

		if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
		{
			EndDialog(hDlg, TRUE);
		}

		break;
	}

	return (FALSE);
}

BOOL APIENTRY IFormSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	int i;
	BYTE IFORM, IFORM1;

	char Text[40];

	switch (message)
	{
	case WM_INITDIALOG:

		CheckDlgButton(hDlg, IDC_RADIO1, FALSE);
		CheckDlgButton(hDlg, IDC_RADIO2, FALSE);
		CheckDlgButton(hDlg, IDC_RADIO3, FALSE);
		CheckDlgButton(hDlg, IDC_RADIO4, FALSE);
		CheckDlgButton(hDlg, IDC_RADIO5, FALSE);
		CheckDlgButton(hDlg, IDC_RADIO6, FALSE);
		CheckDlgButton(hDlg, IDC_RADIO7, FALSE);
		CheckDlgButton(hDlg, IDC_RADIO8, FALSE);
		CheckDlgButton(hDlg, IDC_RADIO9, FALSE);
		CheckDlgButton(hDlg, IDC_RADIO10, FALSE);
		CheckDlgButton(hDlg, IDC_RADIO11, FALSE);
		CheckDlgButton(hDlg, IDC_RADIO12, FALSE);
		CheckDlgButton(hDlg, IDC_RADIO13, FALSE);
		CheckDlgButton(hDlg, IDC_RADIO14, FALSE);
		CheckDlgButton(hDlg, IDC_RADIO15, FALSE);
		CheckDlgButton(hDlg, IDC_RADIO16, FALSE);
		CheckDlgButton(hDlg, IDC_RADIO17, FALSE);
		CheckDlgButton(hDlg, IDC_RADIO18, FALSE);

		if (InitialIFORM != 0)
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON2), TRUE);

		IFORM = BT8X8_ReadByte(BT8X8_AD_BAR0, BT848_IFORM);
		SaveIFORM = IFORM;

		if (InitialIFORM != 0)
		{
			SetDlgItemText(hDlg, IDC_TEXT1, "Fix vergeben");
			sprintf(Text, "0x%02x", InitialIFORM);
			SetDlgItemText(hDlg, IDC_TEXT2, Text);

		}
		else
		{

			SetDlgItemText(hDlg, IDC_TEXT1, "Automatisch");
			sprintf(Text, "0x%02x", IFORM);
			SetDlgItemText(hDlg, IDC_TEXT2, Text);

		}

		IFORM1 = IFORM;
		i = (IFORM1 >> 7);
		if (i == 0)
			CheckDlgButton(hDlg, IDC_RADIO1, TRUE);
		else
			CheckDlgButton(hDlg, IDC_RADIO2, TRUE);

		IFORM1 = (IFORM << 1);
		i = (IFORM1 >> 6);

		if (i == 0)
			CheckDlgButton(hDlg, IDC_RADIO3, TRUE);
		else if (i == 1)
			CheckDlgButton(hDlg, IDC_RADIO4, TRUE);
		else if (i == 2)
			CheckDlgButton(hDlg, IDC_RADIO5, TRUE);
		else if (i == 3)
			CheckDlgButton(hDlg, IDC_RADIO6, TRUE);

		IFORM1 = (IFORM << 3);
		i = (IFORM1 >> 6);

		if (i == 0)
			CheckDlgButton(hDlg, IDC_RADIO7, TRUE);
		else if (i == 1)
			CheckDlgButton(hDlg, IDC_RADIO8, TRUE);
		else if (i == 2)
			CheckDlgButton(hDlg, IDC_RADIO9, TRUE);
		else if (i == 3)
			CheckDlgButton(hDlg, IDC_RADIO10, TRUE);

		IFORM1 = (IFORM << 5);
		i = (IFORM1 >> 5);

		if (i == 0)
			CheckDlgButton(hDlg, IDC_RADIO11, TRUE);
		else if (i == 1)
			CheckDlgButton(hDlg, IDC_RADIO12, TRUE);
		else if (i == 2)
			CheckDlgButton(hDlg, IDC_RADIO13, TRUE);
		else if (i == 3)
			CheckDlgButton(hDlg, IDC_RADIO14, TRUE);

		break;

	case WM_COMMAND:

		IFORM = 0x00;
		if (IsDlgButtonChecked(hDlg, IDC_RADIO2))
			IFORM = IFORM + 128;

		if (IsDlgButtonChecked(hDlg, IDC_RADIO4))
			IFORM = IFORM + 32;
		else if (IsDlgButtonChecked(hDlg, IDC_RADIO5))
			IFORM = IFORM + 64;
		else if (IsDlgButtonChecked(hDlg, IDC_RADIO6))
			IFORM = IFORM + 96;

		if (IsDlgButtonChecked(hDlg, IDC_RADIO8))
			IFORM = IFORM + 8;
		else if (IsDlgButtonChecked(hDlg, IDC_RADIO9))
			IFORM = IFORM + 16;
		else if (IsDlgButtonChecked(hDlg, IDC_RADIO10))
			IFORM = IFORM + 24;

		if (IsDlgButtonChecked(hDlg, IDC_RADIO12))
			IFORM = IFORM + 1;
		else if (IsDlgButtonChecked(hDlg, IDC_RADIO13))
			IFORM = IFORM + 2;
		else if (IsDlgButtonChecked(hDlg, IDC_RADIO14))
			IFORM = IFORM + 3;
		else if (IsDlgButtonChecked(hDlg, IDC_RADIO15))
			IFORM = IFORM + 4;
		else if (IsDlgButtonChecked(hDlg, IDC_RADIO16))
			IFORM = IFORM + 5;
		else if (IsDlgButtonChecked(hDlg, IDC_RADIO17))
			IFORM = IFORM + 6;
		else if (IsDlgButtonChecked(hDlg, IDC_RADIO18))
			IFORM = IFORM + 7;

		sprintf(Text, "0x%02x", IFORM);
		SetDlgItemText(hDlg, IDC_TEXT2, Text);

		BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_IFORM, IFORM);

		if (LOWORD(wParam) == IDCANCEL)
		{
			BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_IFORM, SaveIFORM);
			EndDialog(hDlg, TRUE);
		}

		if (LOWORD(wParam) == IDOK)
		{
			EndDialog(hDlg, TRUE);
		}

		if (LOWORD(wParam) == IDC_BUTTON1)
		{

			if (MessageBox(hDlg, "Note: The assignment of a fixed IForm parameter has the following side effects!\n"
								"1.) SOURCE switching from the menu video input does not work (only from IFORM menu)\n"
								"2.) the video format - > type adjustment does not work. (only over IFORM menu)\n"
								"the deletion of this fixed adjustment can take place later in this dialog", "dTV", MB_ICONSTOP | MB_OKCANCEL) == IDOK)
			{
				InitialIFORM = IFORM;
				EndDialog(hDlg, TRUE);
			}
		}

		if (LOWORD(wParam) == IDC_BUTTON2)
		{
			InitialIFORM = 0;
			EndDialog(hDlg, TRUE);
		}

		break;
	}

	return (FALSE);
}

BOOL APIENTRY PLLSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{

	switch (message)
	{
	case WM_INITDIALOG:

		if (INIT_PLL == 1)
			CheckDlgButton(hDlg, IDC_CHECK1, TRUE);
		else if (INIT_PLL == 2)
			CheckDlgButton(hDlg, IDC_CHECK2, TRUE);
		break;

	case WM_COMMAND:

		if (LOWORD(wParam) == IDC_CHECK1)
		{
			if (IsDlgButtonChecked(hDlg, IDC_CHECK1) == TRUE)
			{
				CheckDlgButton(hDlg, IDC_CHECK2, FALSE);
			}
		}
		else if (LOWORD(wParam) == IDC_CHECK2)
		{
			if (IsDlgButtonChecked(hDlg, IDC_CHECK2) == TRUE)
			{
				CheckDlgButton(hDlg, IDC_CHECK1, FALSE);
			}
		}

		if (LOWORD(wParam) == IDOK)
		{
			INIT_PLL = 0;
			if (IsDlgButtonChecked(hDlg, IDC_CHECK1) == TRUE)
				INIT_PLL = 1;
			else if (IsDlgButtonChecked(hDlg, IDC_CHECK2) == TRUE)
				INIT_PLL = 2;
			Set_PLL(INIT_PLL);
			EndDialog(hDlg, TRUE);
			break;
		}

		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, TRUE);
			break;
		}
	}

	return (FALSE);
}

BOOL APIENTRY TunerSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	int i;

	char Text[128];
	float ffreq;
	unsigned int freq;

	switch (message)
	{
	case WM_INITDIALOG:
		SaveTuner = TunerType;
		SaveVideoSource = VideoSource;
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_RESETCONTENT, 0, 0);
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_ADDSTRING, 0, (LONG) (LPSTR) "Temic Pal");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_ADDSTRING, 0, (LONG) (LPSTR) "Philips Pal I");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_ADDSTRING, 0, (LONG) (LPSTR) "Philips NTSC");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_ADDSTRING, 0, (LONG) (LPSTR) "Philips SECAM");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_ADDSTRING, 0, (LONG) (LPSTR) "Kein Tuner");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_ADDSTRING, 0, (LONG) (LPSTR) "Philips Pal");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_ADDSTRING, 0, (LONG) (LPSTR) "Temic NTSC");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_ADDSTRING, 0, (LONG) (LPSTR) "Temic Pal I");
		SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_SETCURSEL, TunerType, 0);
		sprintf(Text, "%10.3f", (float) Programm[InitialProg].freq / 1000);
		SetDlgItemText(hDlg, IDC_FREQ, Text);
		PostMessage(hDlg, WM_USER, 1, 0);
		break;
	case WM_USER:

		if (wParam == 1)
		{
			i = SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0);
			if ((i >= 0) && (i < 8))
			{
				SetDlgItemInt(hDlg, IDT_THRESH1, Tuners[i].thresh1, FALSE);
				SetDlgItemInt(hDlg, IDT_THRESH2, Tuners[i].thresh2, FALSE);
				SetDlgItemInt(hDlg, IDT_VHF_L, Tuners[i].VHF_L, FALSE);
				SetDlgItemInt(hDlg, IDT_VHF_H, Tuners[i].VHF_H, FALSE);
				SetDlgItemInt(hDlg, IDT_UHF, Tuners[i].UHF, FALSE);
				SetDlgItemInt(hDlg, IDT_CONFIG, Tuners[i].config, FALSE);
				SetDlgItemInt(hDlg, IDT_I2C, Tuners[i].I2C, FALSE);
				SetDlgItemInt(hDlg, IDT_IFPCoff, Tuners[i].IFPCoff, FALSE);
			}
			SetDlgItemInt(hDlg, IDM_THRESH1, Tuners[8].thresh1, FALSE);
			SetDlgItemInt(hDlg, IDM_THRESH2, Tuners[8].thresh2, FALSE);
			SetDlgItemInt(hDlg, IDM_VHF_L, Tuners[8].VHF_L, FALSE);
			SetDlgItemInt(hDlg, IDM_VHF_H, Tuners[8].VHF_H, FALSE);
			SetDlgItemInt(hDlg, IDM_UHF, Tuners[8].UHF, FALSE);
			SetDlgItemInt(hDlg, IDM_CONFIG, Tuners[8].config, FALSE);
			SetDlgItemInt(hDlg, IDT_I2C2, Tuners[8].I2C, FALSE);
			SetDlgItemInt(hDlg, IDM_IFPCoff, Tuners[8].IFPCoff, FALSE);
		}

		if (wParam == 2)
		{
			if (TunerType != 8)
			{
				TunerType = 8;
				Init_Tuner(TunerType);
			}
			if (VideoSource != 0)
			{
				VideoSource = 0;
				SetVideoSource(VideoSource);
			}

			Tuners[8].thresh1 = GetDlgItemInt(hDlg, IDM_THRESH1, NULL, FALSE);
			Tuners[8].thresh2 = GetDlgItemInt(hDlg, IDM_THRESH2, NULL, FALSE);
			Tuners[8].VHF_L = GetDlgItemInt(hDlg, IDM_VHF_L, NULL, FALSE);
			Tuners[8].VHF_H = GetDlgItemInt(hDlg, IDM_VHF_H, NULL, FALSE);
			Tuners[8].UHF = GetDlgItemInt(hDlg, IDM_UHF, NULL, FALSE);
			Tuners[8].config = GetDlgItemInt(hDlg, IDM_CONFIG, NULL, FALSE);
			Tuners[8].IFPCoff = GetDlgItemInt(hDlg, IDM_IFPCoff, NULL, FALSE);
			GetDlgItemText(hDlg, IDC_FREQ, Text, sizeof(Text));
			ffreq = (float) atof(Text);
			freq = (unsigned long) (ffreq * 1000000);
			Tuner_SetFrequency(TunerType, MulDiv(freq, 16, 1000000));
			SendMessage(hDlg, WM_USER, 1, 0);
		}

		break;
	case WM_COMMAND:

		if (LOWORD(wParam) == IDC_COMBO1)
		{
			if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				SendMessage(hDlg, WM_USER, 1, 0);
			}
		}

		if (LOWORD(wParam) == IDSET)
		{
			SendMessage(hDlg, WM_USER, 2, 0);
			SendMessage(hDlg, WM_USER, 1, 0);
		}

		if (LOWORD(wParam) == IDOK)
		{
			CheckMenuItem(GetMenu(hWnd), TunerType + 1100, MF_CHECKED);
			CheckMenuItem(GetMenu(hWnd), SaveTuner + 1100, MF_UNCHECKED);
			CheckMenuItem(GetMenu(hWnd), IDM_EXTERN1, MF_UNCHECKED);
			CheckMenuItem(GetMenu(hWnd), IDM_EXTERN2, MF_UNCHECKED);
			CheckMenuItem(GetMenu(hWnd), IDM_EXTERN3, MF_UNCHECKED);
			GetDlgItemText(hDlg, IDC_FREQ, Text, sizeof(Text));
			ffreq = (float) atof(Text);
			EndDialog(hDlg, TRUE);
		}

		if (LOWORD(wParam) == IDCANCEL)
		{
			VideoSource = SaveVideoSource;
			TunerType = SaveTuner;
			Init_Tuner(TunerType);
			SetVideoSource(VideoSource);
			Tuner_SetFrequency(TunerType, MulDiv(Programm[InitialProg].freq * 1000, 16, 1000000));
			EndDialog(hDlg, TRUE);
		}
		break;
	}

	return (FALSE);
}

BOOL APIENTRY CardSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{

	switch (message)
	{
	case WM_INITDIALOG:
		SaveTuner = CardType;
		SaveVideoSource = AudioSource;
		PostMessage(hDlg, WM_USER, 1, 0);
		break;

	case WM_USER:

		if (wParam == 1)
		{
			SetDlgItemInt(hDlg, IDC_EDIT1, ManuellAudio[0], FALSE);
			SetDlgItemInt(hDlg, IDC_EDIT2, ManuellAudio[1], FALSE);
			SetDlgItemInt(hDlg, IDC_EDIT3, ManuellAudio[2], FALSE);
			SetDlgItemInt(hDlg, IDC_EDIT4, ManuellAudio[3], FALSE);
			SetDlgItemInt(hDlg, IDC_EDIT5, ManuellAudio[4], FALSE);
			SetDlgItemInt(hDlg, IDC_EDIT6, ManuellAudio[5], FALSE);
			SetDlgItemInt(hDlg, IDC_EDIT7, ManuellAudio[6], FALSE);
			SetDlgItemInt(hDlg, IDC_EDIT8, ManuellAudio[7], FALSE);

		}

		break;
	case WM_COMMAND:

		if (LOWORD(wParam) == IDSET)
		{
			CardType = 6;
			AudioSource = 0;
			if (IsDlgButtonChecked(hDlg, IDC_RADIO1))
				AudioSource = 0;
			if (IsDlgButtonChecked(hDlg, IDC_RADIO2))
				AudioSource = 1;
			if (IsDlgButtonChecked(hDlg, IDC_RADIO3))
				AudioSource = 2;
			if (IsDlgButtonChecked(hDlg, IDC_RADIO4))
				AudioSource = 3;
			if (IsDlgButtonChecked(hDlg, IDC_RADIO5))
				AudioSource = 4;
			if (IsDlgButtonChecked(hDlg, IDC_RADIO6))
				AudioSource = 5;
			ManuellAudio[0] = GetDlgItemInt(hDlg, IDC_EDIT1, NULL, FALSE);
			ManuellAudio[1] = GetDlgItemInt(hDlg, IDC_EDIT2, NULL, FALSE);
			ManuellAudio[2] = GetDlgItemInt(hDlg, IDC_EDIT3, NULL, FALSE);
			ManuellAudio[3] = GetDlgItemInt(hDlg, IDC_EDIT4, NULL, FALSE);
			ManuellAudio[4] = GetDlgItemInt(hDlg, IDC_EDIT5, NULL, FALSE);
			ManuellAudio[5] = GetDlgItemInt(hDlg, IDC_EDIT6, NULL, FALSE);
			ManuellAudio[6] = GetDlgItemInt(hDlg, IDC_EDIT7, NULL, FALSE);
			ManuellAudio[7] = GetDlgItemInt(hDlg, IDC_EDIT8, NULL, FALSE);
			SetAudioSource(AudioSource);
		}

		if (LOWORD(wParam) == IDOK)
		{
			ManuellAudio[0] = GetDlgItemInt(hDlg, IDC_EDIT1, NULL, FALSE);
			ManuellAudio[1] = GetDlgItemInt(hDlg, IDC_EDIT2, NULL, FALSE);
			ManuellAudio[2] = GetDlgItemInt(hDlg, IDC_EDIT3, NULL, FALSE);
			ManuellAudio[3] = GetDlgItemInt(hDlg, IDC_EDIT4, NULL, FALSE);
			ManuellAudio[4] = GetDlgItemInt(hDlg, IDC_EDIT5, NULL, FALSE);
			ManuellAudio[5] = GetDlgItemInt(hDlg, IDC_EDIT6, NULL, FALSE);
			ManuellAudio[6] = GetDlgItemInt(hDlg, IDC_EDIT7, NULL, FALSE);
			ManuellAudio[7] = GetDlgItemInt(hDlg, IDC_EDIT8, NULL, FALSE);

			CheckMenuItem(GetMenu(hWnd), SaveVideoSource + 1110, MF_UNCHECKED);
			CheckMenuItem(GetMenu(hWnd), AudioSource + 1110, MF_CHECKED);
			CheckMenuItem(GetMenu(hWnd), SaveTuner + 1080, MF_UNCHECKED);
			CheckMenuItem(GetMenu(hWnd), CardType + 1080, MF_CHECKED);
			EndDialog(hDlg, TRUE);
		}

		if (LOWORD(wParam) == IDCANCEL)
		{
			CardType = SaveTuner;
			AudioSource = SaveVideoSource;
			SetAudioSource(AudioSource);
			EndDialog(hDlg, TRUE);
		}
		break;
	}

	return (FALSE);
}

BOOL MSP_Reset()
{
	BOOL ret = TRUE;

	I2CBus_Lock();
	I2CBus_Start();
	I2CBus_SendByte(AudioDeviceWrite, 5);
	I2CBus_SendByte(0x00, 0);
	I2CBus_SendByte(0x80, 0);
	I2CBus_SendByte(0x00, 0);
	I2CBus_Stop();
	I2CBus_Start();
	if ((I2CBus_SendByte(AudioDeviceWrite, 5) == FALSE) || (I2CBus_SendByte(0x00, 0) == FALSE) || (I2CBus_SendByte(0x00, 0) == FALSE) || (I2CBus_SendByte(0x00, 0) == FALSE))
		ret = FALSE;

	I2CBus_Stop();
	I2CBus_Unlock();
	return ret;
}

void MSP_SetCarrier(int cdo1, int cdo2)
{
	WriteDem(0x93, cdo1 & 0xfff);
	WriteDem(0x9b, cdo1 >> 12);
	WriteDem(0xa3, cdo2 & 0xfff);
	WriteDem(0xab, cdo2 >> 12);
}

void MSP_SetMode(int type)
{
	int i;

	CheckMenuItem(GetMenu(hWnd), 1600 + MSPMode, MF_UNCHECKED);
	MSPMode = type;
	CheckMenuItem(GetMenu(hWnd), 1600 + MSPMode, MF_CHECKED);

	WriteDem(0xbb, msp_init_data[type].ad_cv);

	for (i = 5; i >= 0; i--)
		WriteDem(0x01, msp_init_data[type].fir1[i]);

	WriteDem(0x05, 0x0004);		/* fir 2 */
	WriteDem(0x05, 0x0040);
	WriteDem(0x05, 0x0000);

	for (i = 5; i >= 0; i--)
		WriteDem(0x05, msp_init_data[type].fir2[i]);

	WriteDem(0x83, msp_init_data[type].mode_reg);

	MSP_SetCarrier(msp_init_data[type].cdo1, msp_init_data[type].cdo2);

	WriteDem(0x60, 0);			/* LOAD_REG_1/2 */
	WriteDSP(0x08, msp_init_data[type].dfp_src);
	WriteDSP(0x09, msp_init_data[type].dfp_src);
	WriteDSP(0x0a, msp_init_data[type].dfp_src);
	WriteDSP(0x0e, msp_init_data[type].dfp_matrix);

// msp3410 needs some more initialization 
	if (MSPNicam)
		WriteDSP(0x10, 0x3000);

}

void MSP_SetStereo(int MajorMode, int MinorMode, int mode)
{
	int nicam = 0;

	CheckMenuItem(GetMenu(hWnd), 1630 + MSPStereo, MF_UNCHECKED);
	MSPStereo = mode;
	CheckMenuItem(GetMenu(hWnd), 1630 + MSPStereo, MF_CHECKED);

	// switch demodulator 
	switch (MSPMode)
	{
	case MSP_MODE_FM_TERRA:
		MSP_SetCarrier(carrier_detect[MinorMode], carrier_detect_main[MajorMode]);
		switch (MSPStereo)
		{
		case VIDEO_SOUND_STEREO:
			WriteDSP(0x0e, 0x3001);
			break;
		case VIDEO_SOUND_MONO:
		case VIDEO_SOUND_LANG1:
		case VIDEO_SOUND_LANG2:
			WriteDSP(0x0e, 0x3000);
			break;
		}
		break;
	case MSP_MODE_FM_SAT:
		switch (MSPStereo)
		{
		case VIDEO_SOUND_MONO:
			MSP_SetCarrier(MSP_CARRIER(6.5), MSP_CARRIER(6.5));
			break;
		case VIDEO_SOUND_STEREO:
			MSP_SetCarrier(MSP_CARRIER(7.2), MSP_CARRIER(7.02));
			break;
		case VIDEO_SOUND_LANG1:
			MSP_SetCarrier(MSP_CARRIER(7.38), MSP_CARRIER(7.02));
			break;
		case VIDEO_SOUND_LANG2:
			MSP_SetCarrier(MSP_CARRIER(7.38), MSP_CARRIER(7.02));
			break;
		}
		break;
	case MSP_MODE_FM_NICAM1:
	case MSP_MODE_FM_NICAM2:
		MSP_SetCarrier(carrier_detect[MinorMode], carrier_detect_main[MajorMode]);
		nicam = 0x0100;
		break;
	default:
		// can't do stereo - abort here 
		return;
	}

	// switch audio 
	switch (MSPStereo)
	{
	case VIDEO_SOUND_STEREO:
		WriteDSP(0x08, 0x0020 | nicam);
		WriteDSP(0x09, 0x0020 | nicam);
		WriteDSP(0x0a, 0x0020 | nicam);
		WriteDSP(0x05, 0x4000);
		break;
	case VIDEO_SOUND_MONO:
	case VIDEO_SOUND_LANG1:
		WriteDSP(0x08, 0x0000 | nicam);
		WriteDSP(0x09, 0x0000 | nicam);
		WriteDSP(0x0a, 0x0000 | nicam);
		break;
	case VIDEO_SOUND_LANG2:
		WriteDSP(0x08, 0x0010 | nicam);
		WriteDSP(0x09, 0x0010 | nicam);
		WriteDSP(0x0a, 0x0010 | nicam);
		break;
	}
}

BOOL MSP_Version()
{
	int rev1, rev2;

	MSPNicam = FALSE;

	rev1 = ReadDSP(0x1e);
	rev2 = ReadDSP(0x1f);
	if (0 == rev1 && 0 == rev2)
	{
		return (FALSE);
	}
	MSPAutoDetectValue = 3;

	sprintf(MSPVersion, "MSP34%02d%c-%c%d", (rev2 >> 8) & 0xff, (rev1 & 0xff) + '@', ((rev1 >> 8) & 0xff) + '@', rev2 & 0x1f);
	MSPNicam = (((rev2 >> 8) & 0xff) != 00) ? 1 : 0;
	if (MSPNicam == TRUE)
	{
		MSPAutoDetectValue = 5;

	}
	return (TRUE);
}

void MSP_Set_MajorMinor_Mode(int MajorMode, int MinorMode)
{
	CheckMenuItem(GetMenu(hWnd), 1610 + MSPMajorMode, MF_UNCHECKED);
	MSPMajorMode = MajorMode;
	CheckMenuItem(GetMenu(hWnd), 1610 + MSPMajorMode, MF_CHECKED);

	CheckMenuItem(GetMenu(hWnd), 1620 + MSPMinorMode, MF_UNCHECKED);
	MSPMinorMode = MinorMode;
	CheckMenuItem(GetMenu(hWnd), 1620 + MSPMinorMode, MF_CHECKED);

	switch (MajorMode)
	{
	case 1:					// 5.5 
		if (MinorMode == 0)
		{
			// B/G FM-stereo 
//              MSP_SetMode(MSP_MODE_FM_TERRA);
			MSP_SetStereo(MajorMode, MinorMode, VIDEO_SOUND_MONO);
		}
		else if (MinorMode == 1 && MSPNicam)
		{
			// B/G NICAM 
//              MSP_SetMode(MSP_MODE_FM_NICAM1);
			MSP_SetCarrier(carrier_detect[MinorMode], carrier_detect_main[MajorMode]);
		}
		else
		{
//          MSP_SetMode(MSP_MODE_FM_TERRA);
			MSP_SetCarrier(carrier_detect[MinorMode], carrier_detect_main[MajorMode]);
		}
		break;
	case 2:					// 6.0 
		// PAL I NICAM 
//          MSP_SetMode(MSP_MODE_FM_NICAM2);
		MSP_SetCarrier(MSP_CARRIER(6.552), carrier_detect_main[MajorMode]);
		break;
	case 3:					// 6.5 
		if (MinorMode == 1 || MinorMode == 2)
		{
			// D/K FM-stereo 
//              MSP_SetMode( MSP_MODE_FM_TERRA);
			MSP_SetStereo(MajorMode, MinorMode, VIDEO_SOUND_MONO);
		}
		else if (MinorMode == 0 && MSPNicam)
		{
			// D/K NICAM 
//              MSP_SetMode(MSP_MODE_FM_NICAM1);
			MSP_SetCarrier(carrier_detect[MinorMode], carrier_detect_main[MajorMode]);
		}
		else
		{
//          MSP_SetMode(MSP_MODE_FM_TERRA);
			MSP_SetCarrier(carrier_detect[MinorMode], carrier_detect_main[MajorMode]);
		}
		break;
	case 0:					// 4.5 
	default:
//          MSP_SetMode(MSP_MODE_FM_TERRA);
		MSP_SetCarrier(carrier_detect[MinorMode], carrier_detect_main[MajorMode]);
		break;
	}

}
void MSP_Print_Mode()
{
	char Text[128];

	if (Has_MSP == FALSE)
		strcpy(Text, "Kein MSP-Audio-Device");
	else
	{

		switch (MSPMode)
		{
		case 0:
			strcpy(Text, "AM (msp3400)+");
			break;
		case 1:
			strcpy(Text, "AM (msp3410)+");
			break;
		case 2:
			strcpy(Text, "FM Radio+");
			break;
		case 3:
			strcpy(Text, "TV Terrestial+");
			break;
		case 4:
			strcpy(Text, "TV Sat+");
			break;
		case 5:
			strcpy(Text, "NICAM B/G+");
			break;
		case 6:
			strcpy(Text, "NICAM I+");
			break;
		}

		switch (MSPMajorMode)
		{
		case 0:
			strcat(Text, "NTSC+");
			break;
		case 1:
			strcat(Text, "PAL B/G+");
			break;
		case 2:
			strcat(Text, "PAL I+");
			break;
		case 3:
			strcat(Text, "PAL D/K (Sat+Secam)+");
			break;
		}

		switch (MSPMinorMode)
		{
		case 0:
			strcat(Text, "FM-stereo ");
			break;
		case 1:
			strcat(Text, "NICAM ");
			break;
		case 2:
			strcat(Text, "NICAM ");
			break;
		case 3:
			strcat(Text, "D/K1 FM-Stereo ");
			break;
		case 4:
			strcat(Text, "D/K2 FM-stereo ");
			break;
		case 5:
			strcat(Text, "SAT FM-stereo s/b ");
			break;
		case 6:
			strcat(Text, "SAT FM-stereo s ");
			break;
		case 7:
			strcat(Text, "SAT FM-stereo b ");
			break;
		}

		switch (MSPStereo)
		{
		case 1:
			strcat(Text, "(Mono)");
			break;
		case 2:
			strcat(Text, "(Stereo)");
			break;
		case 3:
			strcat(Text, "(Kanal 1)");
			break;
		case 4:
			strcat(Text, "(Kanal 2)");
			break;
		}
	}
	SetWindowText(hwndAudioField, Text);
}

void MSPWatch_Mode()
{
	int val;
	int newstereo = MSPStereo;

	Sleep(2);
	switch (MSPMode)
	{
	case MSP_MODE_FM_TERRA:

		val = ReadDSP(0x18);
		if (val > 4096)
		{
			newstereo = VIDEO_SOUND_STEREO;
		}
		else if (val < -4096)
		{
			newstereo = VIDEO_SOUND_LANG1;
		}
		else
		{
			newstereo = VIDEO_SOUND_MONO;
		}
		break;
	case MSP_MODE_FM_NICAM1:
	case MSP_MODE_FM_NICAM2:
		val = ReadDSP(0x23);
		switch ((val & 0x1e) >> 1)
		{
		case 0:
		case 8:
			newstereo = VIDEO_SOUND_STEREO;
			break;
		default:
			newstereo = VIDEO_SOUND_MONO;
			break;
		}
		break;
	}

	if (MSPStereo != newstereo)
	{
		if (AutoStereoSelect == TRUE)
			MSP_SetStereo(MSPMajorMode, MSPMinorMode, newstereo);
		else
			MSPNewStereo = newstereo;

	}
}

