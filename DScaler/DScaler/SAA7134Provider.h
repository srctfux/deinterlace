/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Provider.h,v 1.2 2002-09-09 14:27:58 atnak Exp $
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
// This software was based on BT848Provider.h.  Those portions are
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
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __SAA7134PROVIDER_H___
#define __SAA7134PROVIDER_H___

#include "SourceProvider.h"
#include "HardwareDriver.h"
#include "HardwareMemory.h"
#include "SAA7134Source.h"

/** The provider detects all the cards with saa7134 PCI bridges and creates the
    appropriate sources for them.
*/
class CSAA7134Provider : public ISourceProvider
{
public:
    CSAA7134Provider(CHardwareDriver* pHardwareDriver);
    virtual ~CSAA7134Provider();
    int GetNumberOfSources();
    CSource* GetSource(int SourceIndex);
private:
    ///  uses the subsystem id to determin the correct source to create
    CSAA7134Source* CreateCorrectSource(
                                        CHardwareDriver* pHardwareDriver, 
                                        LPCSTR szSection, 
                                        WORD VendorID, 
                                        WORD DeviceID, 
                                        int DeviceIndex, 
                                        DWORD SubSystemId,
                                        char* ChipName);
    /// creates the system accesable memory to be used by all cards
    BOOL MemoryInit(CHardwareDriver* pHardwareDriver);
    void MemoryFree();
    vector<CSAA7134Source*> m_SAA7134Sources;
    /// Memory used for DMA page table
    CContigMemory* m_PagelistDMAMem[4];
    /// Memory used for VBI
    CUserMemory* m_VBIDMAMem[2];
    /// Memory used for captured frames
    CUserMemory* m_DisplayDMAMem[2];
};

#endif