//
// $Id$
//
/////////////////////////////////////////////////////////////////////////////
//
// copyleft 2001 itt@myself.com
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file I2CBus.cpp CI2CBus Implementation
 */

#include "stdafx.h"
#include "I2CBus.h"

#include "DebugLog.h"


CI2CBus::CI2CBus()
{
}

CI2CBus::~CI2CBus()
{
}

BOOL CI2CBus::Read(const BYTE *writeBuffer,
                  size_t writeBufferSize,
                  BYTE *readBuffer,
                  size_t readBufferSize)
{
    ASSERT(writeBuffer != 0);
    ASSERT(writeBufferSize >= 1);
    ASSERT(readBuffer != 0 || readBufferSize == 0);

    if (readBufferSize == 0)
        return TRUE;

    BYTE address = writeBuffer[0];

    Lock();

    if (writeBufferSize != 1)
    {
        ASSERT(writeBufferSize > 1);

        Start();

        // send the address
        if (!Write(address & ~1))
        {
            LOGD("I2CBus::write(0x%x) returned TRUE for write address in CI2CBus::read\n", address & ~1);
            Stop();
            Unlock();
            return FALSE;
        }

        for (size_t i = 1; i < (writeBufferSize - 1); i++)
        {
            if(!Write(writeBuffer[i]))
            {
                Stop();
                Unlock();
                return FALSE;
            }
        }

        // The last byte may also create a positive acknowledge, indicating, that
        // the device is "full", which is not an error.
        if (writeBufferSize >= 2)
            Write(writeBuffer[writeBufferSize - 1]);

    }

    Start();

    // The read address requires a negative ack
    if (!Write(address | 1))
    {
        LOGD("I2CBus::write(0x%x) returned FALSE for read address in CI2CBus::read\n", address | 1);
        Stop();
        Unlock();
        return FALSE;
    }

    size_t i;

    for (i = 0; i < (readBufferSize - 1); i++)
        readBuffer[i] = Read(FALSE);
    readBuffer[i] = Read(TRUE);
    Stop();
    Unlock();

    return TRUE;
}

BOOL CI2CBus::Write(const BYTE *writeBuffer, size_t writeBufferSize)
{
    ASSERT(writeBuffer != 0);
    ASSERT(writeBufferSize >= 1);
    ASSERT((writeBuffer[0] & 1) == 0);


    Lock();

    Start();

    for (size_t i = 0; i < writeBufferSize; i++)
    {
        if (!Write(writeBuffer[i]))
        {
            Stop();
            Unlock();
            return FALSE;
        }
    }

    Stop();
    Unlock();

    return TRUE;
}
