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
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bt848.h"
#include "globals.h"

typedef struct BT848_STRUCT
{
   DWORD dwPhysicalAddress;
   DWORD dwMemoryLength;
   DWORD dwMemoryBase;
   DWORD dwIrqNumber;
   DWORD dwSubSystemID;
} BT848_STRUCT;


BT848_STRUCT* hBT8X8 = NULL;

PHYS    RiscBasePhysical; 
DWORD  *RiscBaseLinear;
DWORD *m_pRiscJump;
DWORD *m_pRiscVBIOdd[5];
DWORD *m_pRiscFrameEven[5];
DWORD *m_pRiscVBIEven[5];
DWORD *m_pRiscFrameOdd[5];


WORD m_wWindowWidth;
WORD m_wWindowHeight;

BOOL BT848_FindTVCard(HWND hWnd)
{
	FILE *SettingFile;

	char VersionString[255];
	int ret;
	unsigned short i;

	strcpy(BTVendorID, "0x109e");
	strcpy(BTDeviceID, "0x036e");

	ret = BT848_Open(0x109e, 0x36e, TRUE, FALSE);
	if (ret == 0)
	{
		strcpy(BTTyp, "BT878");
		strcpy(VersionString, "BT878 found");
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
			strcpy(BTTyp, "BT848");
			strcpy(VersionString, "BT848 found");
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
				strcpy(BTTyp, "BT849");
				strcpy(VersionString, "BT849 found");
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
					strcpy(BTTyp, "BT878");
					strcpy(VersionString, "Anubis BT878");
				}
				else if (ret == 3)
				{
					ErrorBox("PCI-Card with Anubis Bt849 Cannot be locked");
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
			fprintf(SettingFile, "Ausgelesene Einträge für Eigenen KartenTyp\n");
			fprintf(SettingFile, "Eintrag für BT848_GPIO_OUT_EN  %9d     ( Schaltwert )\n", i);
			i = ((BT848_ReadByte(BT848_GPIO_REG_INP_HIBYTE)) << 16) + BT848_ReadWord(BT848_GPIO_REG_INP);
			fprintf(SettingFile, "Eintrag für BT848_GPIO_REG_INP %9d     ( Input-Control )\n", i);
			i = ((BT848_ReadByte(BT848_GPIO_DATA_HIBYTE)) << 16) + BT848_ReadWord(BT848_GPIO_DATA);
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
		if (!Alloc_DMA(VBI_DATA_SIZE, &Vbi_dma[i], 0))
		{
			ErrorBox("VideoText Memory ( 77 KB ) for DMA not allocated");
			return (FALSE);
		}
		if (!Alloc_DMA(1024 * 576 * 2, &Display_dma[i], 0))
		{
			ErrorBox("Display Memory (1 MB) for DMA not allocated");
			return (FALSE);
		}
		if (!Alloc_DMA(256, &Burst_dma[i], 0))
		{
			ErrorBox("Burst Memory (256 Bytes) for DMA not allocated");
			return (FALSE);
		}

		pBurstLine[i] = Burst_dma[i]->dwUser;
	}

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

	BT848_ResetHardware();

	return (TRUE);
}

void BT848_Restart_RISC_Code()
{
	BYTE CapCtl = BT848_ReadByte(BT848_CAP_CTL);
	BT848_MaskDataByte(BT848_CAP_CTL, 0, (BYTE) 0x0f);
	BT848_WriteDword(BT848_INT_STAT, (DWORD) 0x0fffffff);
	BT848_WriteDword(BT848_RISC_STRT_ADD, RiscLogToPhys(m_pRiscJump + 2));
	BT848_WriteByte(BT848_CAP_CTL, CapCtl);
}

void BT848_ResetHardware()
{
	BT848_SetDMA(FALSE);
	BT848_WriteByte(BT848_SRESET, 0);
	BT848_WriteByte(BT848_CAP_CTL, 0x00);
	BT848_WriteDword(BT848_RISC_STRT_ADD, RiscLogToPhys(m_pRiscJump + 2));
	BT848_WriteByte(BT848_VBI_PACK_SIZE, (VBI_SPL / 4) & 0xff);
	BT848_WriteByte(BT848_VBI_PACK_DEL, (VBI_SPL / 4) >> 8);
	BT848_WriteWord(BT848_GPIO_DMA_CTL, 0xfc);
	BT848_WriteByte(BT848_IFORM, BT848_IFORM_MUX1 | BT848_IFORM_XTAUTO | BT848_IFORM_PAL_BDGHI);

	BT848_WriteByte(BT848_E_CONTROL, 0x00);
	BT848_WriteByte(BT848_O_CONTROL, 0x00);

	BT848_WriteByte(BT848_E_SCLOOP, BT848_SCLOOP_CAGC);
	BT848_WriteByte(BT848_O_SCLOOP, BT848_SCLOOP_CAGC);

	BT848_WriteByte(BT848_OFORM, BT848_OFORM_CORE0 | BT848_OFORM_RANGE);
	
	BT848_WriteByte(BT848_E_VSCALE_HI, BT848_VSCALE_COMB);
	BT848_WriteByte(BT848_O_VSCALE_HI, BT848_VSCALE_COMB);
	
	BT848_WriteByte(BT848_ADC, BT848_ADC_RESERVED | BT848_ADC_CRUSH);

	BT848_WriteByte(BT848_COLOR_CTL, 0x00);
	
	BT848_WriteByte(BT848_TDEC, 0x00);

	BT848_WriteDword(BT848_INT_STAT, (DWORD) 0x0fffffff);
	BT848_WriteDword(BT848_INT_MASK, 0x800800);

	BT848_SetPLL(0);

	BT848_SetBrightness(InitialBrightness);
	BT848_SetContrast(InitialContrast);
	BT848_SetHue(InitialHue);
	BT848_SetSaturationU(InitialSaturationU);
	BT848_SetSaturationV(InitialSaturationV);
	BT848_SetVideoSource(VideoSource);
}

PHYS RiscLogToPhys(DWORD * pLog)
{
	return (RiscBasePhysical + (pLog - RiscBaseLinear) * 4);
}

void BT848_MakeVBITable(int VBI_Lines)
{
	int nLine;
	int nIndex;
	PHYS pPhysVBI;

	int GotBytesPerLine;

	for(nIndex = 0; nIndex < 5; nIndex++)
	{
		DWORD *po = m_pRiscVBIOdd[nIndex];
		DWORD *pe = m_pRiscVBIEven[nIndex];
		LPBYTE pVBI = (LPBYTE) Vbi_dma[nIndex]->dwUser;

		*(pe++) = (BT848_RISC_SYNC | BT848_FIFO_STATUS_FM1);
		*(pe++) = 0;
		for (nLine = 0; nLine < VBI_Lines; nLine++)
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

		*(pe++) = BT848_RISC_JUMP;
		*(pe++) = RiscLogToPhys(m_pRiscJump + 4 + nIndex * 12);


		*(po++) = (BT848_RISC_SYNC | BT848_FIFO_STATUS_FM1);
		*(po++) = 0;

		for (nLine = VBI_Lines; nLine < VBI_Lines * 2; nLine++)
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

	BT848_WriteByte(BT848_WC_UP, 0xcf);
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

BOOL BT848_SetGeoSize(int width, int height)
{
	int vscale, hscale;
	DWORD sr;
	int hdelay, vdelay;
	int hactive, vactive;
	BYTE crop, vtc, ColourFormat;

	if (!width || !height)
	{
		return FALSE;
	}

	m_wWindowHeight = height;
	m_wWindowWidth = width;

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

	hactive = width;
	vtc = (hactive < 193) ? 2 : ((hactive < 385) ? 1 : 0);

	hscale = ((TVSettings[TVTYPE].wHActivex1 - width) * 4096UL) / width;
	vdelay = TVSettings[TVTYPE].wVDelay;
	hdelay = ((width * TVSettings[TVTYPE].wHDelayx1) / TVSettings[TVTYPE].wHActivex1) & 0x3fe;
	//hdelay -= 4;

	sr = (TVSettings[TVTYPE].wCropHeight * 512) / height - 512;
	vscale = (WORD) (0x10000UL - sr) & 0x1fff;
	vactive = TVSettings[TVTYPE].wCropHeight;
	crop = ((hactive >> 8) & 0x03) | ((hdelay >> 6) & 0x0c) | ((vactive >> 4) & 0x30) | ((vdelay >> 2) & 0xc0);

	BT848_SetGeometryEvenOdd(FALSE, vtc, hscale, vscale, hactive, vactive, hdelay, vdelay, crop);
	BT848_SetGeometryEvenOdd(TRUE, vtc, hscale, vscale, hactive, vactive, hdelay, vdelay, crop);

	MakeVideoTableForDisplay();
	return TRUE;
}

BOOL BT848_SetBrightness(char bBrightness)
{
	BT848_WriteByte(BT848_BRIGHT, bBrightness);
	return TRUE;
}

BOOL BT848_SetHue(char bHue)
{
	BT848_WriteByte(BT848_HUE, bHue);
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

BOOL BT848_SetVideoSource(int nInput)
{
	DWORD MuxSel;
	// 0= Tuner,
	// 1= Composite,
	// 2= SVideo,
	// 3= Other 1
	// 4= Other 2

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

void BT848_SetRiscJumpsDecode(int nFlags)
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

	BT848_WriteDword(BT848_RISC_STRT_ADD, RiscLogToPhys(m_pRiscJump + 2));
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
