/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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

/**
 * @file HardwareMemory.cpp CHardwareMemory Implementation
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "HardwareMemory.h"
#include "DebugLog.h"

CHardwareMemory::CHardwareMemory(CHardwareDriver* pDriver) :
                    m_pDriver(pDriver)
{
    m_pMemStruct = NULL;
}

CHardwareMemory::~CHardwareMemory()
{

}

void* CHardwareMemory::GetUserPointer()
{
    if(m_pMemStruct != NULL)
    {
        return m_pMemStruct->dwUser;
    }
    else
    {
        return NULL;
    }
}

DWORD CHardwareMemory::TranslateToPhysical(void* pUser, DWORD dwSizeWanted, DWORD* pdwSizeAvailable)
{
    if(m_pMemStruct != NULL)
    {
        TPageStruct* pPages = (TPageStruct*)(m_pMemStruct + 1);
        DWORD Offset;
        DWORD i; 
        DWORD sum;
        DWORD pRetVal = 0;

        Offset = (DWORD)pUser - (DWORD)m_pMemStruct->dwUser;
        sum = 0; 
        i = 0;
        while (i < m_pMemStruct->dwPages)
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
    else
    {
        return 0;
    }
}

BOOL CHardwareMemory::IsValid()
{
    return (m_pMemStruct != NULL);
}

CUserMemory::CUserMemory(CHardwareDriver* pDriver, size_t Bytes) :
     CHardwareMemory(pDriver)
{
    TDSDrvParam paramIn;
    DWORD dwReturnedLength;
    DWORD status;
    DWORD nPages = 0;

    m_AllocatedBlock = malloc(Bytes + 0xFFF);
    if(m_AllocatedBlock == NULL)
    {
        throw std::runtime_error("Out of memory");
    }

    memset(m_AllocatedBlock, 0, Bytes + 0xFFF);

    nPages = Bytes / 0xFFF + 1;
    
    DWORD dwOutParamLength = sizeof(TMemStruct) + nPages * sizeof(TPageStruct);
    m_pMemStruct = (TMemStruct*) malloc(dwOutParamLength);
    if(m_pMemStruct == NULL)
    {
        free(m_AllocatedBlock);
        m_AllocatedBlock = NULL;
        throw std::runtime_error("Out of memory");
    }
    
    memset(m_pMemStruct, 0, dwOutParamLength);

    paramIn.dwValue = Bytes;
    paramIn.dwFlags = 0;

    // align memory to page boundary
    if(((DWORD)m_AllocatedBlock & 0xFFFFF000) < (DWORD)m_AllocatedBlock)
    {
        paramIn.dwAddress = (((DWORD)m_AllocatedBlock + 0xFFF) & 0xFFFFF000);
    }
    else
    {
        paramIn.dwAddress = (DWORD)m_AllocatedBlock;
    }

    status = m_pDriver->SendCommand(IOCTL_DSDRV_ALLOCMEMORY,
                            &paramIn,
                            sizeof(paramIn),
                            m_pMemStruct,
                            dwOutParamLength,
                            &dwReturnedLength);

    if(status != ERROR_SUCCESS || m_pMemStruct->dwUser == 0)
    {
        free(m_pMemStruct);
        free((void*)m_AllocatedBlock);
        m_AllocatedBlock = NULL;
        m_pMemStruct = NULL;
        throw std::runtime_error("Memory mapping failed");
    }
}

CUserMemory::~CUserMemory()
{
    DWORD status = ERROR_SUCCESS;
    if(m_pMemStruct != NULL)
    {
        DWORD dwInParamLength = sizeof(TMemStruct) + m_pMemStruct->dwPages * sizeof(TPageStruct);
        status = m_pDriver->SendCommand(IOCTL_DSDRV_FREEMEMORY, m_pMemStruct, dwInParamLength);
        free(m_pMemStruct);
    }
    if(m_AllocatedBlock != NULL)
    {
        free((void*)m_AllocatedBlock);
    }
}

CContigMemory::CContigMemory(CHardwareDriver* pDriver, size_t Bytes) :
                CHardwareMemory(pDriver)
{
    TDSDrvParam paramIn;
    DWORD dwReturnedLength;
    DWORD status;
    
    DWORD dwOutParamLength = sizeof(TMemStruct) + sizeof(TPageStruct);
    m_pMemStruct = (TMemStruct*) malloc(dwOutParamLength);
    if(m_pMemStruct == NULL)
    {
        throw std::runtime_error("Out of memory");
    }

    paramIn.dwValue = Bytes;
    paramIn.dwFlags = ALLOC_MEMORY_CONTIG;
    paramIn.dwAddress = 0;
    status = m_pDriver->SendCommand(IOCTL_DSDRV_ALLOCMEMORY,
                            &paramIn,
                            sizeof(paramIn),
                            m_pMemStruct,
                            dwOutParamLength,
                            &dwReturnedLength);

    if(status != ERROR_SUCCESS || m_pMemStruct->dwUser == 0)
    {
        free(m_pMemStruct);
        throw std::runtime_error("Memory mapping failed");
    }
}

CContigMemory::~CContigMemory()
{
    DWORD Status = ERROR_SUCCESS;
    if(m_pMemStruct != NULL)
    {
        DWORD dwInParamLength = sizeof(TMemStruct) + sizeof(TPageStruct);
        Status = m_pDriver->SendCommand(
                                          IOCTL_DSDRV_FREEMEMORY, 
                                          m_pMemStruct, 
                                          dwInParamLength                                       
                                       );
        free(m_pMemStruct);
    }
}
