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
 * @file i2cdevice.h i2cdevice Header
 */

#if !defined(__I2CDEVICE_H__)
#define __I2CDEVICE_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "I2CBus.h"


/** Base class for devices that support control via I2C
*/
class CI2CDevice
{
public:
    CI2CDevice();
    virtual ~CI2CDevice() {};

    // Set the bus and the address that should be used for
    // reading and writing to the device.
    void        SetI2CBus(CI2CBus* i2cBus, BYTE address = 0x00);
    CI2CBus*    GetI2CBus() const;
    BYTE        GetDeviceAddress() const;

    // Various functions for writing data to this device.
    BOOL WriteToSubAddress(BYTE subAddress, BYTE writeByte);
    BOOL WriteToSubAddress(BYTE subAddress, const BYTE* writeBuffer, size_t writeBufferSize);
    // Various functions for reading data from this device.
    BOOL ReadFromSubAddress(BYTE subAddress, BYTE* readBuffer, size_t readBufferSize);
    BOOL ReadFromSubAddress(BYTE subAddress, const BYTE* writeBuffer, size_t writeBufferSize, BYTE* readBuffer, size_t readBufferSize);
protected:
    // This needs to be overridden to provide the expected
    // I2C address of the device.
    virtual BYTE GetDefaultAddress() const = 0;

protected:
    CI2CBus *m_I2CBus;
    BYTE m_DeviceAddress;
};


/** Generic I2C device class
*/
class CGenericI2CDevice : CI2CDevice
{
public:
    CGenericI2CDevice();
    CGenericI2CDevice(CI2CBus* i2cBus, BYTE address);
    virtual ~CGenericI2CDevice();

    // Set the bus and the address that should be used for
    // reading and writing to the device.
    void SetI2CBus(CI2CBus* i2cBus, BYTE address);

protected:
    BYTE GetDefaultAddress() const { return 0x00; }
};


#endif // !defined(__I2CDEVICE_H__)
