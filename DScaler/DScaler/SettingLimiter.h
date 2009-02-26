/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004 Atsushi Nakagawa.  All rights reserved.
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

#ifndef __SETTINGLIMITER_H__
#define __SETTINGLIMITER_H__

#include "SettingValue.h"

typedef class CSettingLimiter *PSETTINGLIMITER;


//////////////////////////////////////////////////////////////////////////
// CSettingLimiter
//////////////////////////////////////////////////////////////////////////
class CSettingLimiter
{
public:
    CSettingLimiter() { };
    virtual ~CSettingLimiter() { };

    // Returns TRUE if the value is out of bounds of the
    // programmed limit
    virtual BOOL OutOfLimit(IN const CSettingValue& value) const =0;

    // Applys an arbitrary programmed limit on value and
    // returns TRUE if the value was changed.
    virtual BOOL ApplyLimit(IN OUT CSettingValue& value) const =0;
};


//////////////////////////////////////////////////////////////////////////
// CSettingLimiterClampInt
//////////////////////////////////////////////////////////////////////////
class CSettingLimiterClampInt : public CSettingLimiter
{
public:
    CSettingLimiterClampInt(IN INT minimum, IN INT maximum);
    virtual ~CSettingLimiterClampInt();

    virtual void SetMax(IN INT maximum);
    virtual void SetMin(IN INT minimum);
    virtual INT GetMax() const;
    virtual INT GetMin() const;

    virtual BOOL OutOfLimit(IN const CSettingValue& value) const;
    virtual BOOL ApplyLimit(IN OUT CSettingValue& value) const;

private:
    INT        m_minimum;
    INT        m_maximum;
};


//////////////////////////////////////////////////////////////////////////
// CSettingLimiterStringLength
//////////////////////////////////////////////////////////////////////////
class CSettingLimiterStringLength : public CSettingLimiter
{
public:
    CSettingLimiterStringLength(IN ULONG maxLength);
    virtual ~CSettingLimiterStringLength();

    virtual void SetMaxLength(IN ULONG);
    virtual ULONG SetMaxLength() const;

    virtual BOOL OutOfLimit(IN const CSettingValue& value) const;
    virtual BOOL ApplyLimit(IN OUT CSettingValue& value) const;

private:
    ULONG    m_maxLength;
};

#endif

