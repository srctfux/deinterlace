//
// $Id: I2CBusForLineInterface.cpp,v 1.1 2001-11-25 02:03:21 ittarnavsky Exp $
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
//
// $Log: not supported by cvs2svn $
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "I2CBusForLineInterface.h"

#include "DebugLog.h"

void CI2CBusForLineInterface::SetSDALo()
{
    m_LineInterface->SetSDA(false);
    Sleep();
}

void CI2CBusForLineInterface::SetSDAHi()
{
    m_LineInterface->SetSDA(true);
    Sleep();
}

void CI2CBusForLineInterface::SetSCLLo()
{
    m_LineInterface->SetSCL(false);
    Sleep();
}

void CI2CBusForLineInterface::SetSCLHi()
{
    m_LineInterface->SetSCL(true);
    Sleep();
    while (!m_LineInterface->GetSCL()) {    
        /* the hw knows how to read the clock line,
         * so we wait until it actually gets high.
         * This is safer as some chips may hold it low
         * while they are processing data internally. 
         */
        m_LineInterface->SetSCL(true);
        // FIXME yield here/timeout
    }
}


void CI2CBusForLineInterface::Start()
{
    ASSERT(m_LineInterface != 0);

    LOG(1, "S");
    // I2C start: SDA 1 -> 0 with SCL = 1
    // SDA   ^^^\___
    // SCL ___/^^^\_____       
    //        
    SetSCLLo();
    SetSDAHi();
    SetSCLHi();
    SetSDALo();
    SetSCLLo();
}

void CI2CBusForLineInterface::Stop()
{
    ASSERT(m_LineInterface != 0);

    LOG(1, "P");
    // I2C stop: SDA 0 -> 1 with SCL = 1
    // SDA    ___/^^^
    // SCL ____/^^^
    SetSCLLo();
    SetSDALo();
    SetSCLHi();
    SetSDAHi();
}


bool CI2CBusForLineInterface::GetAcknowledge()
{
    ASSERT(m_LineInterface != 0);
    
    SetSCLLo();
    SetSDAHi();
    // SDA = 0 means the slave ACK'd
    bool result = m_LineInterface->GetSDA();
    SetSCLHi();
    SetSCLLo();
    LOG(1, result ? ".N" : ".A");
    return !result;
}

bool CI2CBusForLineInterface::Write(BYTE byte)
{
    ASSERT(m_LineInterface != 0);

    LOG(1, "%02X", byte);
    for(BYTE mask = 0x80; mask > 0; mask /= 2) 
    {
        SetSCLLo();
        if ((byte & mask) != 0)
        {
            SetSDAHi();
        }
        else 
        {
            SetSDALo();
        }
        SetSCLHi();  
    }
    return GetAcknowledge();
}

unsigned char CI2CBusForLineInterface::Read(bool last)
{
    ASSERT(m_LineInterface != 0);

    BYTE result = 0;

    SetSDAHi();
    for (BYTE mask = 0x80; mask > 0; mask /= 2)
    {
        SetSCLLo();
        SetSCLHi();
        if (m_LineInterface->GetSDA())
            result |= mask;
    }
    LOG(1, ".%02X", result);
    if (last)
    {
        SendNAK();
    }
    else
    {
        SendACK();
    }
    return result;
}

void CI2CBusForLineInterface::SendNAK()
{
    ASSERT(m_LineInterface != 0);

    LOG(1, "N");
    SetSCLLo();
    SetSDAHi();
    SetSCLHi();
    SetSCLLo();
    SetSDALo();
}

void CI2CBusForLineInterface::SendACK()
{
    ASSERT(m_LineInterface != 0);

    LOG(1, "A");
    SetSCLLo();
    SetSDALo();
    SetSCLHi();
    SetSCLLo();
    SetSDAHi();
}