/////////////////////////////////////////////////////////////////////////////
// $Id$
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
// This software was based on I2CBus.cpp.  Those portions are
// copyleft 2001 itt@myself.com.
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file saa7134i2cbus.h saa7134i2cbus Header file
 */

#ifdef WANT_SAA713X_SUPPORT

#ifndef __SAA7134I2CBUS_H__
#define __SAA7134I2CBUS_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "I2CBus.h"
#include "SAA7134I2CInterface.h"


class CSAA7134I2CBus : public CI2CBus
{
private:
    enum
    {
        MAX_BUSYWAIT_RETRIES    = 16,
    };

public:
    CSAA7134I2CBus(ISAA7134I2CInterface* pSAA7134I2C);

public:
    /**
    @return TRUE if sucessful
    */
    virtual BOOL Read(
                        const BYTE *writeBuffer,
                        size_t writeBufferSize,
                        BYTE *readBuffer,
                        size_t readBufferSize
                     );

    /**
    @return TRUE if sucessful
    */
    virtual BOOL Write(const BYTE *writeBuffer, size_t writeBufferSize);

protected:
    /// Sets the data for the next command
    virtual void SetData(BYTE Data);
    /// Reads the data from the last command
    virtual BYTE GetData();
    /// Wait until the last command is finished.
    virtual BOOL BusyWait();
    /// Is the bus ready?
    virtual BOOL IsBusReady();

    /// Is the status an error?
    virtual BOOL IsError(BYTE Status);

    /// Addresses the device
    virtual BOOL I2CStart();
    /// Stops the transfer
    virtual BOOL I2CStop();
    /// Continue transfering the next byte
    virtual BOOL I2CContinue();

    /// Lock device
    virtual void Lock();
    /// Unlock device
    virtual void Unlock();

    /// Generic delay function
    virtual void Sleep();

    /// These are not supported
    virtual void Start();
    virtual void Stop();
    virtual BOOL Write(BYTE byte);
    virtual BYTE Read(BOOL last=TRUE);
    virtual BOOL GetAcknowledge();
    virtual void SendACK();
    virtual void SendNAK();

private:
    void InitializeSleep();
    ULONG GetTickCount();

private:
    ISAA7134I2CInterface*   m_pSAA7134I2C;
    BOOL                    m_InitializedSleep;
    ULONG                   m_I2CSleepCycle;
};

#endif

#endif//xxx