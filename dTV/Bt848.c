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

#include "bt848.h"

typedef struct BT8X8_STRUCT
{
   DWORD dwPhysicalAddress;
   DWORD dwMemoryLength;
   DWORD dwMemoryBase;
   DWORD dwIrqNumber;
} BT8X8_STRUCT;


// file global variables

BT8X8_STRUCT* hBT8X8 = NULL;


void BT8X8_Close()
{
	if(hBT8X8 != NULL)
	{
		BT8X8_WriteByte(BT8X8_AD_BAR0, BT848_SRESET, 0);
	
		memoryUnmap(hBT8X8->dwPhysicalAddress, hBT8X8->dwMemoryLength);

		free(hBT8X8);
		hBT8X8 = NULL;
	}
}

BOOL BT8X8_IsAddrSpaceActive(BT8X8_ADDR addrSpace)
{
	return TRUE;
}


BYTE BT8X8_ReadByte(BT8X8_ADDR addrSpace, DWORD dwOffset)
{
	return memoryReadBYTE(dwOffset);
}

WORD BT8X8_ReadWord(BT8X8_ADDR addrSpace, DWORD dwOffset)
{
	return memoryReadWORD(dwOffset);
}

DWORD BT8X8_ReadDword(BT8X8_ADDR addrSpace, DWORD dwOffset)
{
	return memoryReadDWORD(dwOffset);
}

void BT8X8_WriteByte(BT8X8_ADDR addrSpace, DWORD dwOffset, BYTE data)
{
	memoryWriteBYTE(dwOffset, data);
}

void BT8X8_WriteWord(BT8X8_ADDR addrSpace, DWORD dwOffset, WORD data)
{
	memoryWriteWORD(dwOffset, data);
}

void BT8X8_WriteDword(BT8X8_ADDR addrSpace, DWORD dwOffset, DWORD data)
{
	memoryWriteDWORD(dwOffset, data);
}

int BT8X8_Open(DWORD dwVendorID, DWORD dwDeviceID, DWORD options, BOOL Lock)
{
	int Ret = 0;

	if(hBT8X8 != NULL)
	{
		BT8X8_Close();
	}
	
	hBT8X8 = (BT8X8_STRUCT *) malloc(sizeof(BT8X8_STRUCT));

	memset(hBT8X8, 0, sizeof(BT8X8_STRUCT));

	Ret = pciGetHardwareResources( dwVendorID,  
									dwDeviceID,
									&hBT8X8->dwPhysicalAddress,
									&hBT8X8->dwMemoryLength,
									&hBT8X8->dwIrqNumber);


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

BOOL Alloc_Display_DMA(DWORD dwSize, int NR)
{
	memoryAlloc(dwSize, 0, &Display_dma[NR]);
	if(Display_dma[NR] == NULL)
	{
		return (FALSE);
	}
	pDisplay[NR] = Display_dma[NR]->dwUser;
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

void MaskDataByte(int Offset, BYTE d, BYTE m)
{
	BYTE a;
	BYTE b;

	a = BT8X8_ReadByte(BT8X8_AD_BAR0, Offset);
	b = (a & ~(m)) | ((d) & (m));
	BT8X8_WriteByte(BT8X8_AD_BAR0, Offset, b);
}

void MaskDataWord(int Offset, WORD d, WORD m)
{
	WORD a;
	WORD b;

	a = BT8X8_ReadWord(BT8X8_AD_BAR0, Offset);
	b = (a & ~(m)) | ((d) & (m));
	BT8X8_WriteWord(BT8X8_AD_BAR0, Offset, b);
}

void AndDataByte(int Offset, BYTE d)
{
	BYTE a;
	BYTE b;

	a = BT8X8_ReadByte(BT8X8_AD_BAR0, Offset);
	b = a & d;
	BT8X8_WriteByte(BT8X8_AD_BAR0, Offset, b);
}

void AndDataWord(int Offset, short d)
{
	WORD a;
	WORD b;

	a = BT8X8_ReadWord(BT8X8_AD_BAR0, Offset);
	b = a & d;
	BT8X8_WriteWord(BT8X8_AD_BAR0, Offset, b);
}

void OrDataByte(int Offset, BYTE d)
{
	BYTE a;
	BYTE b;

	a = BT8X8_ReadByte(BT8X8_AD_BAR0, Offset);
	b = a | d;
	BT8X8_WriteByte(BT8X8_AD_BAR0, Offset, b);
}

void OrDataWord(int Offset, unsigned short d)
{
	WORD a;
	WORD b;

	a = BT8X8_ReadWord(BT8X8_AD_BAR0, Offset);
	b = a | d;
	BT8X8_WriteWord(BT8X8_AD_BAR0, Offset, b);
}
