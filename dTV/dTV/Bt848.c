/////////////////////////////////////////////////////////////////////////////
// bt848.c
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
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Original Release
//                                     Removed use of WinDrvr
//
//  3 Nov 2000   Michael Eskin         Added override of initial BDELAY setting
//               Conexant Systems      by adding non-zero InitialBDelay in .ini
//                                     File. Changed NTSC defaults to 0x5C
//
// 02 Jan 2001   John Adcock           Made RISC Code linear
//
// 08 Jan 2001   John Adcock           Added C++ like access for strings
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bt848.h"

typedef struct BT848_STRUCT
{
   DWORD dwPhysicalAddress;
   DWORD dwMemoryLength;
   DWORD dwMemoryBase;
   DWORD dwIrqNumber;
   DWORD dwSubSystemID;
} BT848_STRUCT;

BT848_STRUCT* hBT8X8 = NULL;

PMemStruct Risc_dma;
PMemStruct Vbi_dma[5];
PMemStruct Display_dma[5];

BYTE* pDisplay[5] = { NULL,NULL,NULL,NULL,NULL };
BYTE* pVBILines[5] = { NULL,NULL,NULL,NULL,NULL };

PHYS    RiscBasePhysical; 
DWORD  *RiscBaseLinear;
long BytesPerRISCField = 1;

char BTVendorID[10] = "";
char BTDeviceID[10] = "";
char BTChipType[10] = "";

BOOL bSaveSettings = FALSE;

// MAE 2 Nov 2000 - Start of change for Macrovision fix
// If non-zero in .ini file, will override TV table setting
int	InitialBDelay = 0x00;  // Original hardware default value was 0x5D
// MAE 2 Nov 2000 - End of change for Macrovision fix

// MAE, 3 Nov 2000
//    Changed all BDELAY values from 5D to 5C for Macrovision fix
//
// John Adcock, 19 Dec 2000
//    Fixed PAL-N to stop it from crashing, improved PAL-M values
//    These were the old PAL-N Values that crashed dTV
//    /* PAL-M */
//    { 754, 480,  910, 0x70, 0x5c, (BT848_IFORM_PAL_M|BT848_IFORM_XT0),
//        910, 754, 135, 754, 0x1a, 0, FALSE, 400},
//    /* PAL-N */
//    { 922, 576, 1135, 0x7f, 0x72, (BT848_IFORM_PAL_N|BT848_IFORM_XT1),
//	      1135, 922, 186, 922, 0x1c, 0, TRUE, 400},
//
struct TTVSetting TVSettings[] =
{
	/* PAL-BDGHI */
	{ "PAL DBGHI", 576, 1135, 0x7f, 0x72, (BT848_IFORM_PAL_BDGHI|BT848_IFORM_XT1),
	    186, 922, 0x20, 0, TRUE, 511, 19},
	/* NTSC */
	{ "NTSC", 480, 910, 0x68, 0x5c, (BT848_IFORM_NTSC|BT848_IFORM_XT0),
	    137, 754, 0x1a, 0, FALSE, 400, 13},
	/* SECAM */
	{ "SECAM", 576, 1135, 0x7f, 0xb0, (BT848_IFORM_SECAM|BT848_IFORM_XT1),
	    186, 922, 0x20, 0, TRUE, 511, 19},
	/* PAL-M */
	{ "PAL-M", 480,  910, 0x68, 0x5c, (BT848_IFORM_PAL_M|BT848_IFORM_XT0),
	    137, 754, 0x1a, 0, FALSE, 400, 13},
    /* PAL-N */
    { "PAL-M", 576, 1135, 0x7f, 0x72, (BT848_IFORM_PAL_N|BT848_IFORM_XT1),
        186, 922, 0x20, 0, TRUE, 511, 19},
	/* NTSC Japan*/
	{ "NTSC Japan", 480,  910, 0x70, 0x5c, (BT848_IFORM_NTSC_JAP|BT848_IFORM_XT0),
	    135, 754, 0x1a, 0, FALSE, 400, 13},
    /* PAL-60 */
	{ "PAL 60", 480, 1135, 0x7f, 0x72, (BT848_IFORM_PAL_BDGHI|BT848_IFORM_XT1),
	    186, 922, 0x1a, 0, TRUE, 400, 13},
};

int TVTYPE = -1;

int VideoSource = SOURCE_COMPOSITE;

// 10/19/2000 Mark Rejhon
// Better NTSC defaults
// These are the original defaults, likely optimized for PAL (could use refinement).
//int InitialHue        = 0x00;
//int InitialBrightness = 0x00;
//int InitialContrast    = 0xd8;
//int InitialSaturationU = 0xfe;
//int InitialSaturationV = 0xb4;
//int InitialOverscan    = 4;
int InitialHue         = 0;
int InitialBrightness  = 20;
int InitialContrast    = 207;
int InitialSaturationU = 254;
int InitialSaturationV = 219;
int InitialOverscan    = 4;

int CurrentX = 720;
int CurrentY;
int CurrentVBILines = 0;


const char* BT848_VendorID()
{
	return BTVendorID;
}

const char* BT848_DeviceID()
{
	return BTDeviceID;
}

const char* BT848_ChipType()
{
	return BTChipType;
}

BOOL BT848_FindTVCard(HWND hWnd)
{
	int ret;

	strcpy(BTVendorID, "0x109e");
	strcpy(BTDeviceID, "0x036e");

	ret = BT848_Open(0x109e, 0x36e, TRUE, FALSE);
	if (ret == 0)
	{
		strcpy(BTChipType, "BT878");
	}
	else if (ret == 3)
	{
		ErrorBox("PCI-Card with Bt878 Cannot be locked");
		return (FALSE);
	}
	else
	{
		strcpy(BTVendorID, "0x109e");
		strcpy(BTDeviceID, "0x0350");
		ret = BT848_Open(0x109e, 0x350, TRUE, FALSE);
		if (ret == 0)
		{
			strcpy(BTChipType, "BT848");
		}
		else if (ret == 3)
		{
			ErrorBox("PCI-Card with Bt848 Cannot be locked");
			return (FALSE);
		}
		else
		{
			strcpy(BTVendorID, "0x109e");
			strcpy(BTDeviceID, "0x0351");
			ret = BT848_Open(0x109e, 0x351, TRUE, FALSE);
			if (ret == 0)
			{
				strcpy(BTChipType, "BT849");
			}
			else if (ret == 3)
			{
				ErrorBox("PCI-Card with Bt849 Cannot be locked");
				return (FALSE);
			}
			else
			{
				strcpy(BTVendorID, "0x109e");
				strcpy(BTDeviceID, "0x036F");
				ret = BT848_Open(0x109e, 0x36F, TRUE, FALSE);
				if (ret == 0)
				{
					strcpy(BTChipType, "BT878a");
				}
				else if (ret == 3)
				{
					ErrorBox("PCI-Card with Bt878a Cannot be locked");
					return (FALSE);
				}
			}
		}
	}

	if (ret != 0)
	{
		return (FALSE);
	}

	if (bSaveSettings == TRUE)
	{
		BT848_SaveSettings("Setting.BT");
	}

	return (TRUE);
}


void BT848_SaveSettings(LPCSTR szFileName)
{
	FILE *SettingFile;
	unsigned short i;

	if ((SettingFile = fopen(szFileName, "w")) != NULL)
	{
		fprintf(SettingFile, "BT848_COLOR_CTL %02x\n", BT848_ReadByte(BT848_COLOR_CTL));
		fprintf(SettingFile, "BT848_CAP_CTL %02x\n", BT848_ReadByte(BT848_CAP_CTL));
		fprintf(SettingFile, "BT848_VBI_PACK_SIZE %02x\n", BT848_ReadByte(BT848_VBI_PACK_SIZE));
		fprintf(SettingFile, "BT848_VBI_PACK_DEL %02x\n", BT848_ReadByte(BT848_VBI_PACK_DEL));
		fprintf(SettingFile, "BT848_GPIO_DMA_CTL %02x\n", BT848_ReadByte(BT848_GPIO_DMA_CTL));
		fprintf(SettingFile, "BT848_IFORM %02x\n", BT848_ReadByte(BT848_IFORM));

		fprintf(SettingFile, "BT848_E_SCLOOP %02x\n", BT848_ReadByte(BT848_E_SCLOOP));
		fprintf(SettingFile, "BT848_O_SCLOOP %02x\n", BT848_ReadByte(BT848_O_SCLOOP));
		fprintf(SettingFile, "BT848_ADELAY %02x\n", BT848_ReadByte(BT848_ADELAY));
		fprintf(SettingFile, "BT848_BDELAY %02x\n", BT848_ReadByte(BT848_BDELAY));

		fprintf(SettingFile, "BT848_E_HSCALE_HI %02x\n", BT848_ReadByte(BT848_E_HSCALE_HI));
		fprintf(SettingFile, "BT848_E_HSCALE_LO %02x\n", BT848_ReadByte(BT848_E_HSCALE_LO));
		fprintf(SettingFile, "BT848_E_VSCALE_HI %02x\n", BT848_ReadByte(BT848_E_VSCALE_HI));
		fprintf(SettingFile, "BT848_E_VSCALE_LO %02x\n", BT848_ReadByte(BT848_E_VSCALE_LO));
		fprintf(SettingFile, "BT848_E_HACTIVE_LO %02x\n", BT848_ReadByte(BT848_E_HACTIVE_LO));
		fprintf(SettingFile, "BT848_E_HDELAY_LO %02x\n", BT848_ReadByte(BT848_E_HDELAY_LO));
		fprintf(SettingFile, "BT848_E_VACTIVE_LO %02x\n", BT848_ReadByte(BT848_E_VACTIVE_LO));
		fprintf(SettingFile, "BT848_E_VDELAY_LO %02x\n", BT848_ReadByte(BT848_E_VDELAY_LO));
		fprintf(SettingFile, "BT848_E_CROP %02x\n", BT848_ReadByte(BT848_E_CROP));

		fprintf(SettingFile, "BT848_O_HSCALE_HI %02x\n", BT848_ReadByte(BT848_O_HSCALE_HI));
		fprintf(SettingFile, "BT848_O_HSCALE_LO %02x\n", BT848_ReadByte(BT848_O_HSCALE_LO));
		fprintf(SettingFile, "BT848_O_VSCALE_HI %02x\n", BT848_ReadByte(BT848_O_VSCALE_HI));
		fprintf(SettingFile, "BT848_O_VSCALE_LO %02x\n", BT848_ReadByte(BT848_E_VSCALE_LO));
		fprintf(SettingFile, "BT848_O_HACTIVE_LO %02x\n", BT848_ReadByte(BT848_O_HACTIVE_LO));
		fprintf(SettingFile, "BT848_O_HDELAY_LO %02x\n", BT848_ReadByte(BT848_O_HDELAY_LO));
		fprintf(SettingFile, "BT848_O_VACTIVE_LO %02x\n", BT848_ReadByte(BT848_O_VACTIVE_LO));
		fprintf(SettingFile, "BT848_O_VDELAY_LO %02x\n", BT848_ReadByte(BT848_O_VDELAY_LO));
		fprintf(SettingFile, "BT848_O_CROP %02x\n", BT848_ReadByte(BT848_O_CROP));

		fprintf(SettingFile, "BT848_PLL_F_LO %02x\n", BT848_ReadByte(BT848_PLL_F_LO));
		fprintf(SettingFile, "BT848_PLL_F_HI %02x\n", BT848_ReadByte(BT848_PLL_F_HI));
		fprintf(SettingFile, "BT848_PLL_XCI %02x\n", BT848_ReadByte(BT848_PLL_XCI));

		fprintf(SettingFile, "BT848_BRIGHT %02x\n", BT848_ReadByte(BT848_BRIGHT));
		fprintf(SettingFile, "BT848_CONTRAST_LO %02x\n", BT848_ReadByte(BT848_CONTRAST_LO));
		fprintf(SettingFile, "BT848_SAT_V_LO %02x\n", BT848_ReadByte(BT848_SAT_V_LO));
		fprintf(SettingFile, "BT848_SAT_U_LO %02x\n", BT848_ReadByte(BT848_SAT_U_LO));
		fprintf(SettingFile, "BT848_GPIO_OUT_EN %04x\n", BT848_ReadWord(BT848_GPIO_OUT_EN));
		fprintf(SettingFile, "BT848_GPIO_OUT_EN_HIBYTE %02x\n", BT848_ReadByte(BT848_GPIO_OUT_EN_HIBYTE));

		fprintf(SettingFile, "BT848_GPIO_REG_INP %04x\n", BT848_ReadWord(BT848_GPIO_REG_INP));
		fprintf(SettingFile, "BT848_GPIO_REG_INP_HIBYTE %02x\n", BT848_ReadByte(BT848_GPIO_REG_INP_HIBYTE));

		fprintf(SettingFile, "BT848_GPIO_DATA %04x\n", BT848_ReadWord(BT848_GPIO_DATA));
		fprintf(SettingFile, "BT848_GPIO_DATA_HIBYTE %02x\n", BT848_ReadByte(BT848_GPIO_DATA_HIBYTE));
		i = ((BT848_ReadByte(BT848_GPIO_OUT_EN_HIBYTE)) << 16) + BT848_ReadWord(BT848_GPIO_OUT_EN);
		fprintf(SettingFile, "*********************************************\n");
		fprintf(SettingFile, "Ausgelesene Eintr�ge f�r Eigenen KartenTyp\n");
		fprintf(SettingFile, "Eintrag f�r BT848_GPIO_OUT_EN  %9d     ( Schaltwert )\n", i);
		i = ((BT848_ReadByte(BT848_GPIO_REG_INP_HIBYTE)) << 16) + BT848_ReadWord(BT848_GPIO_REG_INP);
		fprintf(SettingFile, "Eintrag f�r BT848_GPIO_REG_INP %9d     ( Input-Control )\n", i);
		i = ((BT848_ReadByte(BT848_GPIO_DATA_HIBYTE)) << 16) + BT848_ReadWord(BT848_GPIO_DATA);
		fprintf(SettingFile, "Eintrag f�r BT848_GPIO_DATA    %9d     ( Eingangswunsch) \n", i);
		fprintf(SettingFile, "*********************************************\n");
		fclose(SettingFile);
	}
}

DWORD BT848_GetSubSystemID()
{
	return hBT8X8->dwSubSystemID;
}

BOOL BT848_MemoryInit(void)
{
	int i;

	if (!Alloc_DMA(83968, &Risc_dma, ALLOC_MEMORY_CONTIG))
	{
		ErrorBox("Risc Memory (83 KB Contiguous) not Allocated");
		return (FALSE);
	}

	RiscBaseLinear = Risc_dma->dwUser;
	RiscBasePhysical = GetPhysicalAddress(Risc_dma, Risc_dma->dwUser, 83968, NULL);
	
	for (i = 0; i < 5; i++)
	{
		// JA 02/01/2001
		// Allocate some extra memory so that we can skip
		// start of buffer that is not page aligned
		if (!Alloc_DMA(2048 * 19 * 2 + 4095, &Vbi_dma[i], 0))
		{
			ErrorBox("VBI Memory for DMA not allocated");
			return (FALSE);
		}
		pVBILines[i] = GetFirstFullPage(Vbi_dma[i]);
		// JA 29/12/2000
		// Allocate some extra memory so that we can skip
		// start of buffer that is not page aligned
		if (!Alloc_DMA(1024 * 576 * 2 + 4095, &Display_dma[i], 0))
		{
			ErrorBox("Display Memory for DMA not allocated");
			return (FALSE);
		}
		pDisplay[i] = GetFirstFullPage(Display_dma[i]);
	}

	return (TRUE);
}

void BT848_MemoryFree()
{
	int i;
	Free_DMA(&Risc_dma);
	for(i = 0; i < 5; i++)
	{
		Free_DMA(&Vbi_dma[i]);
		Free_Display_DMA(i);
	}
}


void BT848_Restart_RISC_Code()
{
	BYTE CapCtl = BT848_ReadByte(BT848_CAP_CTL);
	BT848_MaskDataByte(BT848_CAP_CTL, 0, (BYTE) 0x0f);
	BT848_WriteDword(BT848_INT_STAT, (DWORD) 0x0fffffff);
	BT848_WriteDword(BT848_RISC_STRT_ADD, RiscBasePhysical);
	BT848_WriteByte(BT848_CAP_CTL, CapCtl);
}

BYTE	BtAgcDisable = 0;								// Luma AGC, 0 says AGC enabled
BYTE	BtCrush = BT848_ADC_CRUSH;				// Adaptive AGC, 0 says Crush disabled
BYTE	BtEvenChromaAGC = BT848_SCLOOP_CAGC;	// Even CAGC, 0 says CAGC disable
BYTE	BtOddChromaAGC = BT848_SCLOOP_CAGC;		// Odd chroma AGC
BYTE	BtEvenLumaPeak = 0;						// Even Peak, 0 says normal, not Luma peak
BYTE	BtOddLumaPeak = 0;							
BYTE	BtFullLumaRange = BT848_OFORM_RANGE;    // Full Range Luma, 0=normal,1=full
												// should be 1 for NTSC
BYTE	BtEvenLumaDec = 0;						// Even Luma decimation,  0 says disable
BYTE	BtOddLumaDec = 0;
BYTE	BtEvenComb = BT848_VSCALE_COMB;			// Even Comb, enabled
BYTE	BtOddComb = BT848_VSCALE_COMB;
BYTE	BtColorBars = 0;                        // Display Color Bars, 0 = no
BYTE	BtGammaCorrection = 0 ;					// Gamma Correction Removal, 0 = Enabled
BYTE    BtCoring = BT848_OFORM_CORE0;           // Coring function: (0,1,2,or 3) << 5
BYTE    BtHorFilter = 0 << 3;				    // Horizontal Filer: (0,1,2,3) << 3
												// maybe only 0,1 valid for full res?
BYTE    BtVertFilter = 0;						// Vert. Filter, only 0 and 4 valid here
BYTE    BtColorKill = BT848_SCLOOP_CKILL;		// Kill color if B/W: (0,1) << 5
BYTE    BtWhiteCrushUp = 0xcf;					// Crush up - entire register value
BYTE    BtWhiteCrushDown = 0x7f;				// Crush down - entire register value

void BT848_ResetHardware()
{
	BT848_SetDMA(FALSE);
	BT848_WriteByte(BT848_SRESET, 0);
	BT848_WriteDword(BT848_RISC_STRT_ADD, RiscBasePhysical);
	BT848_WriteByte(BT848_CAP_CTL, 0x00);
	BT848_WriteByte(BT848_VBI_PACK_SIZE, (VBI_SPL / 4) & 0xff);
	BT848_WriteByte(BT848_VBI_PACK_DEL, (VBI_SPL / 4) >> 8);
	BT848_WriteWord(BT848_GPIO_DMA_CTL, 0xfc);
	BT848_WriteByte(BT848_IFORM, BT848_IFORM_MUX1 | BT848_IFORM_XTAUTO | BT848_IFORM_PAL_BDGHI);

	BT848_WriteByte(BT848_E_CONTROL, BtEvenLumaDec);
	BT848_WriteByte(BT848_O_CONTROL, BtOddLumaDec);

	BT848_WriteByte(BT848_E_SCLOOP, 
		(BYTE) (BtEvenChromaAGC | BtEvenLumaPeak | BtColorKill | BtHorFilter));
	
	BT848_WriteByte(BT848_O_SCLOOP, 
		(BYTE) (BtOddChromaAGC | BtOddLumaPeak | BtColorKill | BtHorFilter));

	BT848_WriteByte(BT848_OFORM, (BYTE)(BtCoring | BtFullLumaRange));
	
	BT848_WriteByte(BT848_E_VSCALE_HI, BtEvenComb);
	BT848_WriteByte(BT848_O_VSCALE_HI, BtOddComb);
	
	BT848_WriteByte(BT848_ADC, (BYTE) (BT848_ADC_RESERVED | BtAgcDisable | BtCrush)); 

	BT848_WriteByte(BT848_COLOR_CTL, (BYTE) (BtColorBars | BtGammaCorrection));
	
	BT848_WriteByte(BT848_TDEC, 0x00);

// MAE 2 Nov 2000 - Start of change for Macrovision fix
	if (InitialBDelay != 0)
	{
		// BDELAY override from .ini file
		BT848_SetBDELAY((BYTE)InitialBDelay);
	}
// MAE 2 Nov 2000 - End of change for Macrovision fix


	BT848_WriteDword(BT848_INT_STAT, (DWORD) 0x0fffffff);
	BT848_WriteDword(BT848_INT_MASK, 0);
//						BT848_INT_SCERR|
//						BT848_INT_OCERR|
//						BT848_INT_FDSR|
//						BT848_INT_PPERR|
//						BT848_INT_FBUS|
//						BT848_INT_FTRGT);

	BT848_SetPLL(0);

	BT848_SetBrightness(InitialBrightness);
	BT848_SetContrast(InitialContrast);
	BT848_SetHue(InitialHue);
	BT848_SetSaturationU(InitialSaturationU);
	BT848_SetSaturationV(InitialSaturationV);
	BT848_SetVideoSource(VideoSource);
	BT848_SetGeoSize();
	BT848_WriteByte(BT848_WC_DOWN, BtWhiteCrushDown);			// TRB 12/00 allow parm
}

PHYS RiscLogToPhys(DWORD * pLog)
{
	return (RiscBasePhysical + (pLog - RiscBaseLinear) * 4);
}

void BT848_SetPLL(PLLFREQ PLL)
{
	int i;

	switch(PLL)
	{
	case PLL_NONE:
		BT848_WriteByte(BT848_TGCTRL, BT848_TGCTRL_TGCKI_NOPLL);
		BT848_WriteByte(BT848_PLL_XCI, 0x00);
		return;
		break;
	case PLL_28:
		BT848_WriteByte(BT848_PLL_F_LO, 0xf9);
		BT848_WriteByte(BT848_PLL_F_HI, 0xdc);
		BT848_WriteByte(BT848_PLL_XCI, 0x8E);
		break;
	case PLL_35:
		BT848_WriteByte(BT848_PLL_F_LO, 0x39);
		BT848_WriteByte(BT848_PLL_F_HI, 0xB0);
		BT848_WriteByte(BT848_PLL_XCI, 0x89);
		break;
	}

	for (i = 0; i < 100; i++)
	{
		if (BT848_ReadByte(BT848_DSTATUS) & BT848_DSTATUS_CSEL)
		{
			BT848_WriteByte(BT848_DSTATUS, 0x00);
		}
		else
		{
			BT848_WriteByte(BT848_TGCTRL, BT848_TGCTRL_TGCKI_PLL);
			break;
		}
		Sleep(10);
	}

	BT848_WriteByte(BT848_WC_UP, BtWhiteCrushUp);			// TRB 12/00 allow parm
	BT848_WriteByte(BT848_VTOTAL_LO, 0x00);
	BT848_WriteByte(BT848_VTOTAL_HI, 0x00);
	BT848_WriteByte(BT848_DVSIF, 0x00);
}

void BT848_SetDMA(BOOL bState)
{
	if (bState)
	{
		BT848_OrDataWord(BT848_GPIO_DMA_CTL, 3);
	}
	else
	{
		BT848_AndDataWord(BT848_GPIO_DMA_CTL, ~3);
	}
}

BOOL BT848_SetGeoSize()
{
	int vscale, hscale;
	DWORD sr;
	int hdelay, vdelay;
	int hactive, vactive;
	BYTE crop, vtc, ColourFormat;

	CurrentY = TVSettings[TVTYPE].wCropHeight;
	CurrentVBILines = TVSettings[TVTYPE].VBILines;

	// set the pll on the card if appropriate
	if(TVSettings[TVTYPE].Is25fps == TRUE && TVCards[CardType].pll != PLL_NONE)
	{
		BT848_SetPLL(TVCards[CardType].pll);
	}
	else
	{
		BT848_SetPLL(0);
	}

	BT848_WriteByte(BT848_ADELAY, TVSettings[TVTYPE].bDelayA);
	BT848_WriteByte(BT848_BDELAY, TVSettings[TVTYPE].bDelayB);

	BT848_WriteByte(BT848_VBI_PACK_SIZE, (BYTE)(TVSettings[TVTYPE].VBIPacketSize & 0xff));
	BT848_WriteByte(BT848_VBI_PACK_DEL, (BYTE)(TVSettings[TVTYPE].VBIPacketSize >> 8));

	BT848_MaskDataByte(BT848_IFORM, TVSettings[TVTYPE].bIForm, BT848_IFORM_NORM | BT848_IFORM_XTBOTH);

	ColourFormat = (BYTE)((BT848_COLOR_FMT_YUY2 << 4) | BT848_COLOR_FMT_YUY2);

	BT848_WriteByte(BT848_COLOR_FMT, ColourFormat);

// MAE 2 Nov 2000 - Start of change for Macrovision fix
	if (InitialBDelay != 0)
	{
		// BDELAY override from .ini file
		BT848_SetBDELAY((BYTE)InitialBDelay);
	}
// MAE 2 Nov 2000 - End of change for Macrovision fix

	hactive = CurrentX;
//	vtc = (hactive < 193) ?	2 : ((hactive < 385) ? 1 : 0);		// TRB 12/15/00  allow vertical filter from ini
	vtc = BtVertFilter;		
	hscale = ((TVSettings[TVTYPE].wHActivex1 - CurrentX) * 4096UL) / CurrentX;
	vdelay = TVSettings[TVTYPE].wVDelay;
	hdelay = ((CurrentX * TVSettings[TVTYPE].wHDelayx1) / TVSettings[TVTYPE].wHActivex1) & 0x3fe;

	sr = (TVSettings[TVTYPE].wCropHeight * 512) / CurrentY - 512;
	vscale = (WORD) (0x10000UL - sr) & 0x1fff;
	vactive = TVSettings[TVTYPE].wCropHeight;
	crop = ((hactive >> 8) & 0x03) | ((hdelay >> 6) & 0x0c) | ((vactive >> 4) & 0x30) | ((vdelay >> 2) & 0xc0);

	BT848_SetGeometryEvenOdd(FALSE, vtc, hscale, vscale, hactive, vactive, hdelay, vdelay, crop);
	BT848_SetGeometryEvenOdd(TRUE, vtc, hscale, vscale, hactive, vactive, hdelay, vdelay, crop);

	return TRUE;
}

BOOL BT848_SetBrightness(int wBrightness)
{
	BT848_WriteByte(BT848_BRIGHT, (BYTE) (wBrightness & 0xff));
	return TRUE;
}

BOOL BT848_SetHue(int wHue)
{
	BT848_WriteByte(BT848_HUE, (BYTE) (wHue & 0xff));
	return TRUE;
}

BOOL BT848_SetContrast(int wContrast)
{
	BYTE bContHi;

	bContHi = (BYTE) (wContrast >> 6) & 4;
	BT848_WriteByte(BT848_CONTRAST_LO, (BYTE) (wContrast & 0xff));
	BT848_MaskDataByte(BT848_E_CONTROL, bContHi, 4);
	BT848_MaskDataByte(BT848_O_CONTROL, bContHi, 4);
	return TRUE;
}

BOOL BT848_SetSaturationU(int wData)
{
	BYTE bDataHi;

	bDataHi = (BYTE) (wData >> 7) & 2;
	BT848_WriteByte(BT848_SAT_U_LO, (BYTE) (wData & 0xff));
	BT848_MaskDataByte(BT848_E_CONTROL, bDataHi, 2);
	BT848_MaskDataByte(BT848_O_CONTROL, bDataHi, 2);
	return TRUE;
}

BOOL BT848_SetSaturationV(int wData)
{
	BYTE bDataHi;

	bDataHi = (BYTE) (wData >> 8) & 1;
	BT848_WriteByte(BT848_SAT_V_LO, (BYTE) (wData & 0xff));
	BT848_MaskDataByte(BT848_E_CONTROL, bDataHi, 1);
	BT848_MaskDataByte(BT848_O_CONTROL, bDataHi, 1);
	return TRUE;
}

// MAE 3 Nov 2000 Start of Macrovision fix
BOOL BT848_SetBDELAY(BYTE bBDelay)
{
	BT848_WriteByte(BT848_BDELAY, bBDelay);
	return TRUE;
}
// MAE 3 Nov 2000 End of Macrovision fix

BOOL BT848_SetVideoSource(int nInput)
{
	DWORD MuxSel;
	// 0= Tuner,
	// 1= Composite,
	// 2= SVideo,
	// 3= Other 1
	// 4= Other 2
	// 5= Composite via SVideo

	BT848_AndOrDataDword(BT848_GPIO_OUT_EN, TVCards[CardType].GPIOMuxMask, ~TVCards[CardType].GPIOMuxMask);
	BT848_AndDataByte(BT848_IFORM, ~BT848_IFORM_MUXSEL);

	// set the comp bit for svideo
	switch (nInput)
	{
	case 0:
		BT848_AndDataByte(BT848_E_CONTROL, ~BT848_CONTROL_COMP);
		BT848_AndDataByte(BT848_O_CONTROL, ~BT848_CONTROL_COMP);
		MuxSel = TVCards[CardType].MuxSelect[TVCards[CardType].TunerInput & 7];
		break;
	case 2:
		BT848_OrDataByte(BT848_E_CONTROL, BT848_CONTROL_COMP);
		BT848_OrDataByte(BT848_O_CONTROL, BT848_CONTROL_COMP);
		MuxSel = TVCards[CardType].MuxSelect[TVCards[CardType].SVideoInput & 7];
		break;
	case 5:
		BT848_AndDataByte(BT848_E_CONTROL, ~BT848_CONTROL_COMP);
		BT848_AndDataByte(BT848_O_CONTROL, ~BT848_CONTROL_COMP);
		MuxSel = TVCards[CardType].MuxSelect[TVCards[CardType].SVideoInput & 7];
		break;
	case 1:
	case 3:
	case 4:
		BT848_AndDataByte(BT848_E_CONTROL, ~BT848_CONTROL_COMP);
		BT848_AndDataByte(BT848_O_CONTROL, ~BT848_CONTROL_COMP);
		MuxSel = TVCards[CardType].MuxSelect[nInput];
		break;
	}
	
	VideoSource = nInput;
	BT848_MaskDataByte(BT848_IFORM, (BYTE) (((MuxSel) & 3) << 5), BT848_IFORM_MUXSEL);
	BT848_AndOrDataDword(BT848_GPIO_DATA, MuxSel >> 4, ~TVCards[CardType].GPIOMuxMask);
	return TRUE;
}

void BT848_SetGeometryEvenOdd(BOOL bOdd, BYTE bVtc, int wHScale, int wVScale, int wHActive, int wVActive, int wHDelay, int wVDelay, BYTE bCrop)
{
	int nOff = bOdd ? 0x80 : 0x00;

	BT848_WriteByte(BT848_E_VTC + nOff, bVtc);
	BT848_WriteByte(BT848_E_HSCALE_HI + nOff, (BYTE) (wHScale >> 8));
	BT848_WriteByte(BT848_E_HSCALE_LO + nOff, (BYTE) (wHScale & 0xFF));
	BT848_MaskDataByte(BT848_E_VSCALE_HI + nOff, (BYTE) (wVScale >> 8), 0x1F);
	BT848_WriteByte(BT848_E_VSCALE_LO + nOff, (BYTE) (wVScale & 0xFF));
	BT848_WriteByte(BT848_E_HACTIVE_LO + nOff, (BYTE) (wHActive & 0xFF));
	BT848_WriteByte(BT848_E_HDELAY_LO + nOff, (BYTE) (wHDelay & 0xFF));
	BT848_WriteByte(BT848_E_VACTIVE_LO + nOff, (BYTE) (wVActive & 0xFF));
	BT848_WriteByte(BT848_E_VDELAY_LO + nOff, (BYTE) (wVDelay & 0xFF));
	BT848_WriteByte(BT848_E_CROP + nOff, bCrop);
}

BOOL BT848_IsVideoPresent()
{
	return ((BT848_ReadByte(BT848_DSTATUS) & (BT848_DSTATUS_PRES | BT848_DSTATUS_HLOC)) == (BT848_DSTATUS_PRES | BT848_DSTATUS_HLOC)) ? TRUE : FALSE;
}

// Creates the RISC code
// First syncs to field
// Then waits for data
// Then tells the bt848 where to put each line of data
void BT848_CreateRiscCode(int nFlags)
{
	DWORD *pRiscCode;
	int nField;
	int nLine;
	LPBYTE pUser;
	PHYS pPhysical;
	DWORD GotBytesPerLine;
	DWORD BytesPerLine = 0;

	pRiscCode = Risc_dma->dwUser;

	// we create the RISC code for 10 fields
	// the first one (0) is even
	// last one (9) is odd
	for(nField = 0; nField < 10; nField++)
	{
		// First we sync onto either the odd or even field
		if(nField & 1)
		{
			*(pRiscCode++) = (DWORD) (BT848_RISC_SYNC | BT848_RISC_RESYNC | BT848_FIFO_STATUS_VRO);
		}
		else
		{
			*(pRiscCode++) = (DWORD) (BT848_RISC_SYNC | BT848_RISC_RESYNC | BT848_FIFO_STATUS_VRE  | ((0xF1 + nField / 2) << 16));
		}
		*(pRiscCode++) = 0;

		// Create VBI code of required
		if (nField % 2 == 0 && nFlags & BT848_CAP_CTL_CAPTURE_VBI_EVEN ||
			nField % 2 == 1 && nFlags & BT848_CAP_CTL_CAPTURE_VBI_ODD)
		{
			*(pRiscCode++) = (DWORD) (BT848_RISC_SYNC | BT848_FIFO_STATUS_FM1);
			*(pRiscCode++) = 0;

			pUser = pVBILines[nField / 2];
			if(nField & 1)
			{
				pUser += CurrentVBILines * 2048;
			}
			for (nLine = 0; nLine < CurrentVBILines; nLine++)
			{
				pPhysical = GetPhysicalAddress(Vbi_dma[nField / 2], pUser, VBI_SPL, &GotBytesPerLine);
				if(pPhysical == 0 || VBI_SPL > GotBytesPerLine)
				{
					return;
				}
				*(pRiscCode++) = BT848_RISC_WRITE | BT848_RISC_SOL | BT848_RISC_EOL | BytesPerLine;
				*(pRiscCode++) = pPhysical;
				pUser += 2048;
			}
		}

		*(pRiscCode++) = (DWORD) (BT848_RISC_SYNC | BT848_FIFO_STATUS_FM1);
		*(pRiscCode++) = 0;


		// work out the position of the first line
		// first line is line zero an even line
		pUser = pDisplay[nField / 2];
		if(nField & 1)
		{
			pUser += 2048;
		}
		BytesPerLine = CurrentX * 2;
		for (nLine = 0; nLine < CurrentY / 2; nLine++)
		{

			pPhysical = GetPhysicalAddress(Display_dma[nField / 2], pUser, BytesPerLine, &GotBytesPerLine);
			if(pPhysical == 0 || BytesPerLine > GotBytesPerLine)
			{
				return;
			}
			*(pRiscCode++) = BT848_RISC_WRITE | BT848_RISC_SOL | BT848_RISC_EOL | BytesPerLine;
			*(pRiscCode++) = pPhysical;
			// since we are doing all the lines of the same
			// polarity at the same time we skip two lines
			pUser += 4096;
		}
	}

	BytesPerRISCField = ((long)pRiscCode - (long)Risc_dma->dwUser) / 10;
	*(pRiscCode++) = BT848_RISC_JUMP;
	*(pRiscCode++) = RiscBasePhysical;

	BT848_WriteDword(BT848_RISC_STRT_ADD, RiscBasePhysical);
}

// Works out a field number between 0-9 indicating which field we are currently
// sending to memory
int BT848_GetRISCPosAsInt()
{
	int CurrentPos = 10;
	while(CurrentPos > 9)
	{
		DWORD CurrentRiscPos = BT848_ReadDword(BT848_RISC_COUNT);
		CurrentPos = (CurrentRiscPos - RiscBasePhysical) / BytesPerRISCField;
	}

	return CurrentPos;
}

void BT848_Close()
{
	if(hBT8X8 != NULL)
	{
		BT848_WriteByte(BT848_SRESET, 0);
	
		memoryUnmap(hBT8X8->dwPhysicalAddress, hBT8X8->dwMemoryLength);

		free(hBT8X8);
		hBT8X8 = NULL;
	}
}

int BT848_Open(DWORD dwVendorID, DWORD dwDeviceID, DWORD options, BOOL Lock)
{
	int Ret = 0;

	if(hBT8X8 != NULL)
	{
		BT848_Close();
	}
	
	hBT8X8 = (BT848_STRUCT *) malloc(sizeof(BT848_STRUCT));

	memset(hBT8X8, 0, sizeof(BT848_STRUCT));

	Ret = pciGetHardwareResources( dwVendorID,  
									dwDeviceID,
									&hBT8X8->dwPhysicalAddress,
									&hBT8X8->dwMemoryLength,
									&hBT8X8->dwSubSystemID);


	// check if handle valid & version OK
	if (Ret != ERROR_SUCCESS)
	{
		Ret = 2;
		// error - Cannot find PCI card
		goto Exit;
	}

	hBT8X8->dwMemoryBase = memoryMap(hBT8X8->dwPhysicalAddress, hBT8X8->dwMemoryLength);
	if(hBT8X8->dwMemoryBase == 0)
	{
		Ret = 3;
		goto Exit;
	}

	// Open finished OK
	return 0;

  Exit:
	// Error during Open
	free(hBT8X8);
	hBT8X8 = NULL;
	return Ret;
}

BOOL Alloc_DMA(DWORD dwSize, PMemStruct * dma, int Option)
{
	*dma = NULL;

	memoryAlloc(dwSize, Option, dma);

	if (*dma == NULL)
	{
		return (FALSE);
	}
	return TRUE;
}

void Free_DMA(PMemStruct * dma)
{
	memoryFree(*dma);
}

void Free_Display_DMA(int NR)
{
	LPVOID *MemPtr = NULL;

	if (Display_dma[NR] == NULL)
	{
		return;
	}
	memoryFree(Display_dma[NR]);
	Display_dma[NR] = NULL;
}

PHYS GetPhysicalAddress(PMemStruct pMem, LPBYTE pLinear, DWORD dwSizeWanted, DWORD * pdwSizeAvailable)
{
	PPageStruct pPages = (PPageStruct)(pMem + 1);
	DWORD Offset;
    DWORD i; 
    DWORD sum;
	DWORD pRetVal = 0;

	Offset = (DWORD)pLinear - (DWORD)pMem->dwUser;
    sum = 0; 
	i = 0;
	while (i < pMem->dwPages)
	{
		if (sum + pPages[i].dwSize > (unsigned)Offset)
		{
            Offset -= sum;
		    pRetVal = pPages[i].dwPhysical + Offset;	
            if ( pdwSizeAvailable != NULL )
			{
				*pdwSizeAvailable = pPages[i].dwSize - Offset;
			}
			break;
		}
		sum += pPages[i].dwSize; 
		i++;
	}
	if(pRetVal == 0)
	{
		sum++;
	}
    if ( pdwSizeAvailable != NULL )
	{
		if (*pdwSizeAvailable < dwSizeWanted)
		{
			sum++;
		}
	}

	return pRetVal;	
}

// JA 29/12/2000
// This function returns the user space address of the first page aligned
// section of an allocated buffer.

void* GetFirstFullPage(PMemStruct pMem)
{
	PPageStruct pPages = (PPageStruct)(pMem + 1);
	DWORD pRetVal;

	pRetVal = (DWORD)pMem->dwUser;

	if(pPages[0].dwSize != 4096)
	{
		pRetVal += pPages[0].dwSize;
	}
	return (void*)pRetVal;	
}

void BT848_MaskDataByte(int Offset, BYTE d, BYTE m)
{
	BYTE a;

	a = BT848_ReadByte(Offset);
	a = (a & ~(m)) | ((d) & (m));
	BT848_WriteByte(Offset, a);
}

void BT848_MaskDataWord(int Offset, WORD d, WORD m)
{
	WORD a;

	a = BT848_ReadWord(Offset);
	a = (a & ~(m)) | ((d) & (m));
	BT848_WriteWord(Offset, a);
}
void BT848_AndOrDataDword(int Offset, DWORD d, DWORD m)
{
	DWORD a;

	a = BT848_ReadDword(Offset);
	a = (a & m) | d;
	BT848_WriteDword(Offset, a);
}

void BT848_AndDataByte(int Offset, BYTE d)
{
	BYTE a;

	a = BT848_ReadByte(Offset);
	a &= d;
	BT848_WriteByte(Offset, a);
}

void BT848_AndDataWord(int Offset, short d)
{
	WORD a;

	a = BT848_ReadWord(Offset);
	a &= d;
	BT848_WriteWord(Offset, a);
}

void BT848_OrDataByte(int Offset, BYTE d)
{
	BYTE a;

	a = BT848_ReadByte(Offset);
	a |= d;
	BT848_WriteByte(Offset, a);
}

void BT848_OrDataWord(int Offset, unsigned short d)
{
	WORD a;

	a = BT848_ReadWord(Offset);
	a |= d;
	BT848_WriteWord(Offset, a);
}

//===========================================================================
// CCIR656 Digital Input Support
//
// 13 Dec 2000 - Michael Eskin, Conexant Systems - Initial version
//
//===========================================================================
// Timing generator SRAM table values for CCIR601 720x480 NTSC
//===========================================================================
BYTE SRAMTable[ 60 ] =
{
      0x33, // size of table = 51
      0x0c, 0xc0, 0x00, 0x00, 0x90, 0xc2, 0x03, 0x10, 0x03, 0x06,
      0x10, 0x34, 0x12, 0x12, 0x65, 0x02, 0x13, 0x24, 0x19, 0x00,
      0x24, 0x39, 0x00, 0x96, 0x59, 0x08, 0x93, 0x83, 0x08, 0x97,
      0x03, 0x50, 0x30, 0xc0, 0x40, 0x30, 0x86, 0x01, 0x01, 0xa6,
      0x0d, 0x62, 0x03, 0x11, 0x61, 0x05, 0x37, 0x30, 0xac, 0x21, 0x50
};

//===========================================================================
// Enable CCIR656 Input mode
//===========================================================================
BOOL BT848_Enable656()
{
	int vscale, hscale;
	int hdelay, vdelay;
	int hactive, vactive;
	BYTE crop;
	int i;

	CurrentX = 720;
	CurrentY = 480;

	// Disable TG mode
	BT848_MaskDataByte(BT848_TGCTRL, 0, BT848_TGCTRL_TGMODE_ENABLE);
	
	// Reset the TG address
	BT848_MaskDataByte(BT848_TGCTRL, 0, BT848_TGCTRL_TGMODE_RESET);
	BT848_MaskDataByte(BT848_TGCTRL, BT848_TGCTRL_TGMODE_RESET, BT848_TGCTRL_TGMODE_RESET);
	BT848_MaskDataByte(BT848_TGCTRL, 0, BT848_TGCTRL_TGMODE_RESET);

	// Load up the TG table for CCIR656
	for (i=0;i<SRAMTable[0];++i)
	{
	  BT848_WriteByte(BT848_TBLG,SRAMTable[i+1]);
	}
	
	// Enable TG mode
	BT848_MaskDataByte(BT848_TGCTRL, BT848_TGCTRL_TGMODE_ENABLE, BT848_TGCTRL_TGMODE_ENABLE);

	// Enable the GPCLOCK
	BT848_MaskDataByte(BT848_TGCTRL, BT848_TGCTRL_TGCKI_GPCLK, BT848_TGCTRL_TGCKI_GPCLK);

	// Set the PLL mode
	BT848_WriteByte(BT848_PLL_XCI, 0x00);

	// Enable 656 Mode, bypass chroma filters
	BT848_WriteByte(BT848_DVSIF, BT848_DVSIF_VSIF_BCF | BT848_DVSIF_CCIR656);
	
	// Enable NTSC mode
	BT848_MaskDataByte(BT848_IFORM, (BT848_IFORM_NTSC | BT848_IFORM_XTBOTH), (BT848_IFORM_NORM | BT848_IFORM_XTBOTH));

	// Disable full range luma
	BT848_WriteByte(BT848_OFORM, 0);

	// Enable the SC loop luma peaking filters
	BT848_WriteByte(BT848_E_SCLOOP, BT848_SCLOOP_LUMA_PEAK);
	BT848_WriteByte(BT848_O_SCLOOP, BT848_SCLOOP_LUMA_PEAK);

	// Standard NTSC 525 line count
	BT848_WriteByte(BT848_VTOTAL_LO, 0x00);
	BT848_WriteByte(BT848_VTOTAL_HI, 0x00);

	// YUV 4:2:2 linear pixel format
	BT848_WriteByte(BT848_COLOR_FMT, (BYTE)((BT848_COLOR_FMT_YUY2 << 4) | BT848_COLOR_FMT_YUY2));

	// Setup parameters for overlay scale and crop calculation
	hactive = CurrentX;
	vactive = CurrentY;
	hscale = 0;
	vdelay = 16;
	hdelay = 0x80;
	vscale = 0;

	crop = ((hactive >> 8) & 0x03) | ((hdelay >> 6) & 0x0c) | ((vactive >> 4) & 0x30) | ((vdelay >> 2) & 0xc0);

	BT848_SetGeometryEvenOdd(FALSE, BT848_VTC_HSFMT_32, hscale, vscale, hactive, vactive, hdelay, vdelay, crop);
	BT848_SetGeometryEvenOdd(TRUE, BT848_VTC_HSFMT_32, hscale, vscale, hactive, vactive, hdelay, vdelay, crop);

	return TRUE;
}

