/////////////////////////////////////////////////////////////////////////////
// bt848.h
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

#ifndef __BT848_H___
#define __BT848_H___

#include "defines.h"
#include "structs.h"
#include "globals.h"


int BT8X8_Open (DWORD dwVendorID, DWORD dwDeviceID,  DWORD options,BOOL Lock);
void BT8X8_Close();
DWORD BT8X8_CountCards (DWORD dwVendorID, DWORD dwDeviceID);
BOOL BT8X8_IsAddrSpaceActive(BT8X8_ADDR addrSpace);

// General read/write function
void BT8X8_ReadWriteBlock(BT8X8_ADDR addrSpace, DWORD dwOffset, BOOL fRead, PVOID buf, DWORD dwBytes, BT8X8_MODE mode);
BYTE BT8X8_ReadByte (BT8X8_ADDR addrSpace, DWORD dwOffset);
WORD BT8X8_ReadWord (BT8X8_ADDR addrSpace, DWORD dwOffset);
DWORD BT8X8_ReadDword (BT8X8_ADDR addrSpace, DWORD dwOffset);
void BT8X8_WriteByte (BT8X8_ADDR addrSpace, DWORD dwOffset, BYTE data);
void BT8X8_WriteWord (BT8X8_ADDR addrSpace, DWORD dwOffset, WORD data);
void BT8X8_WriteDword (BT8X8_ADDR addrSpace, DWORD dwOffset, DWORD data);
// access to PCI configuration registers
BOOL BT8X8_DetectCardElements();

BOOL Alloc_DMA(DWORD dwSize, PMemStruct * dma, int Option);
BOOL Alloc_Display_DMA(DWORD dwSize, int NR);
void Free_DMA(PMemStruct * dma);
PHYS GetPhysicalAddress(PMemStruct dma, LPBYTE pLinear, DWORD dwSizeWanted, DWORD * pdwSizeAvailable);
void MaskDataByte(int Offset, BYTE d, BYTE m);
void MaskDataWord(int Offset, WORD d, WORD m);
void AndDataByte(int Offset, BYTE d);
void AndDataWord(int Offset, short d);
void OrDataByte(int Offset, BYTE d);
void OrDataWord(int Offset, unsigned short d);

#endif