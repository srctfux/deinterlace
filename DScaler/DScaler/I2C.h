/////////////////////////////////////////////////////////////////////////////
// $Id: I2C.h,v 1.6 2003-10-27 10:39:51 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
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
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 11 Aug 2000   John Adcock           Moved i2c Functions in here
//
/////////////////////////////////////////////////////////////////////////////

/** 
 * @file i2c.h i2c Header file
 */
 
#ifndef __I2C_H___
#define __I2C_H___

class CI2C
{
public:
    BOOL I2C_AddDevice(BYTE I2C_Port);
    BOOL I2C_SendByte(BYTE nData, int nWaitForAck);
    BYTE I2C_ReadByte(BOOL bLast);
protected:
    CI2C();
    ~CI2C();
    virtual void I2C_SetLine(BOOL bCtrl, BOOL bData) = 0;
    virtual BOOL I2C_GetLine() = 0;
    virtual BYTE I2C_Read(BYTE nAddr) = 0;
    virtual BOOL I2C_Write(BYTE nAddr, BYTE nData1, BYTE nData2, BOOL bSendBoth) = 0;
    void I2C_Wait(int MicroSecs);
    void I2C_Lock();
    void I2C_Unlock();
    void I2C_Start();
    void I2C_Stop();
private:
    void I2C_One();
    void I2C_Zero();
    BOOL I2C_Ack();
    CRITICAL_SECTION m_hCritSect;
};

#define I2C_TSA5522        0xc2
#define I2C_TDA7432        0x8a
#define I2C_TDA8425        0x82
#define I2C_TDA9840        0x84
#define I2C_TDA9850        0xb6 // also used by 9855,9873 
#define I2C_TDA9875        0xb0
#define I2C_HAUPEE         0xa0
#define I2C_STBEE          0xae
#define I2C_VHX            0xc0
#define I2C_MSP3400        0x80
#define I2C_TEA6300        0x80
#define I2C_DPL3518        0x84


#endif
