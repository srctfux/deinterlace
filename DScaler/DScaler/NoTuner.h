//
// $Id: NoTuner.h,v 1.2 2001-11-26 13:02:27 adcockj Exp $
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
// Revision 1.1  2001/11/25 02:03:21  ittarnavsky
// initial checkin of the new I2C code
//
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(__NOTUNER_H__)
#define __NOTUNER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ITuner.h"

class CNoTuner : public ITuner  
{
public:
    CNoTuner()
    {
    }
    eTunerId GetTunerId()
    {
        return TUNER_ABSENT;
    }
    eVideoFormat GetDefaultVideoFormat()
    {
        return FORMAT_NTSC;
    }
    bool HasRadio() const
    {
        return false;
    }
    bool SetRadioFrequency(long nFrequency)
    {
        return true;
    }
    bool SetTVFrequency(long nFrequency, eVideoFormat videoFormat)
    {
        return true;
    }

protected:
    virtual BYTE GetDefaultAddress()const
    {
        return 0;
    }

};

#endif // !defined(__NOTUNER_H__)
