/////////////////////////////////////////////////////////////////////////////
// $Id: CT2388xProvider.h,v 1.1 2002-09-11 18:19:37 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 John Adcock.  All rights reserved.
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

#ifndef __CT2388xPROVIDER_H___
#define __CT2388xPROVIDER_H___

#include "SourceProvider.h"
#include "HardwareDriver.h"
#include "HardwareMemory.h"
#include "CT2388xSource.h"

class CCT2388xProvider : public ISourceProvider
{
public:
    CCT2388xProvider(CHardwareDriver* pHardwareDriver);
    virtual ~CCT2388xProvider();
    int GetNumberOfSources();
    CSource* GetSource(int SourceIndex);
private:
    ///  uses the subsystem id to determin the correct source to create
    CCT2388xSource* CreateCorrectSource(
                                        CHardwareDriver* pHardwareDriver, 
                                        LPCSTR szSection, 
                                        WORD VendorID, 
                                        WORD DeviceID, 
                                        int DeviceIndex, 
                                        DWORD SubSystemId);
    /// creates the system accesable memory to be used by all cards
    BOOL MemoryInit(CHardwareDriver* pHardwareDriver);
    void MemoryFree();
    vector<CCT2388xSource*> m_Sources;
    /// Memory used for the RISC code
    CContigMemory* m_RiscDMAMem;
    /// Memory used for captured frames
    CUserMemory* m_DisplayDMAMem[5];
};

#endif