/////////////////////////////////////////////////////////////////////////////
// $Id: SettingKey.cpp,v 1.5 2005-03-05 12:15:20 atnak Exp $
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
//  This file is part of the SettingRepository module.  See
//  SettingRepository.h for more information.
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.4  2004/09/08 07:14:08  atnak
// Added type cast operators to simplify usage.
//
// Revision 1.3  2004/08/20 07:25:17  atnak
// Removed the title value.
//
// Revision 1.2  2004/08/14 13:45:23  adcockj
// Fixes to get new settings code working under VS6
//
// Revision 1.1  2004/08/06 17:12:10  atnak
// Setting repository initial upload.
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SettingKey.h"
#include "SettingGroup.h"
#include "SettingValue.h"
#include "SettingObject.h"
#include "SettingConfig.h"
#include <string>


//////////////////////////////////////////////////////////////////////////
// CSettingKey
//////////////////////////////////////////////////////////////////////////

CSettingKey::CSettingKey() :
	m_controller(NULL)
{
}


CSettingKey::~CSettingKey()
{
}


PSETTINGOBJECT CSettingKey::NewSetting(LPCSTR entry, BYTE type)
{
	PSETTINGOBJECT object = (PSETTINGOBJECT)new CSettingObjectContained(entry, type);
	if (object == NULL)
	{
		OUT_OF_MEMORY_ERROR;
	}
	return object;
}


void CSettingKey::LoadSetting()
{
	ASSERT(m_controller != NULL);
	m_controller->LoadSetting(m_identifier);
}


void CSettingKey::SaveSetting()
{
	ASSERT(m_controller != NULL);
	m_controller->SaveSetting(m_identifier);
}


void CSettingKey::SetValueSV(RCSETTINGVALUE value)
{
	ASSERT(m_controller != NULL);
	m_controller->SetValue(m_identifier, value);
}


CSettingValue CSettingKey::GetValueSV()
{
	ASSERT(m_controller != NULL);
	return m_controller->GetValue(m_identifier);
}


void CSettingKey::UseDefault()
{
	ASSERT(m_controller != NULL);
	m_controller->UseDefault(m_identifier);
}


void CSettingKey::SetDefaultSV(RCSETTINGVALUE value)
{
	ASSERT(m_controller != NULL);
	m_controller->SetDefault(m_identifier, value);
}


CSettingValue CSettingKey::GetDefaultSV()
{
	ASSERT(m_controller != NULL);
	return m_controller->GetDefault(m_identifier);
}


void CSettingKey::CheckLimiter()
{
	ASSERT(m_controller != NULL);
	m_controller->CheckLimiter(m_identifier);
}


void CSettingKey::SetLimiter(PSETTINGLIMITER limiter)
{
	ASSERT(m_controller != NULL);
	m_controller->SetLimiter(m_identifier, limiter);
}


PSETTINGLIMITER CSettingKey::GetLimiter()
{
	ASSERT(m_controller != NULL);
	return m_controller->GetLimiter(m_identifier);
}


BOOL CSettingKey::Notify(INT message, RCSETTINGVALUE newValue, RCSETTINGVALUE oldValue)
{
	return TRUE;
}


void CSettingKey::SetController(PSETTINGGROUP controller, HSETTING setting)
{
	m_controller = controller;
	m_identifier = setting;
}


PSETTINGGROUP CSettingKey::GetController() const
{
	return m_controller;
}


HSETTING CSettingKey::GetIdentifier() const
{
	ASSERT(m_controller != NULL);
	return m_identifier;
}


//////////////////////////////////////////////////////////////////////////
// CSettingKeyLong
//////////////////////////////////////////////////////////////////////////
CSettingKeyLong::CSettingKeyLong()
{
}


CSettingKeyLong::~CSettingKeyLong()
{
}


PSETTINGOBJECT CSettingKeyLong::NewSetting(LPCSTR entry, long initial)
{
	PSETTINGOBJECT object = CSettingKey::NewSetting(entry, SETTING_VALUE_INT);

	CSettingValue defaultValue;
	defaultValue.SetInt(initial);
	object->SetDefault(defaultValue, NULL, NULL);
	return object;
}


PSETTINGOBJECT CSettingKeyLong::NewSetting(LPCSTR entry, long initial, long minimum, long maximum)
{
	PSETTINGOBJECT object = CSettingKeyLong::NewSetting(entry, initial);

	PSETTINGLIMITER limiter = (PSETTINGLIMITER)new CSettingLimiterClampInt(minimum, maximum);
	object->SetLimiter(limiter, NULL, NULL);
	return object;
}


void CSettingKeyLong::SetValue(long value)
{
	CSettingValue newValue;
	newValue.SetInt(value);
	SetValueSV(newValue);
}


long CSettingKeyLong::GetValue()
{
	CSettingValue value = GetValueSV();
	ASSERT(value.GetType() == SETTING_VALUE_INT);
	return value.GetInt();
}


void CSettingKeyLong::SetDefault(long value)
{
	CSettingValue defaultValue;
	defaultValue.SetInt(value);
	SetDefaultSV(defaultValue);
}


long CSettingKeyLong::GetDefault()
{
	CSettingValue value = GetDefaultSV();
	ASSERT(value.GetType() == SETTING_VALUE_INT);
	return value.GetInt();
}


void CSettingKeyLong::SetRange(long minimum, long maximum)
{
	PSETTINGLIMITER limiter = (PSETTINGLIMITER)new CSettingLimiterClampInt(minimum, maximum);
	CSettingKey::SetLimiter(limiter);
}


BOOL CSettingKeyLong::Notify(INT message, RCSETTINGVALUE newValue, RCSETTINGVALUE oldValue)
{
	// Assert new value but don't assert old because old can be not set.
	ASSERT(newValue.GetType() == SETTING_VALUE_INT);

	long newInt = newValue.GetInt();
	long oldInt = oldValue.GetInt();

	return Notify(message, newInt, oldInt);
}


BOOL CSettingKeyLong::Notify(INT message, long newValue, long oldValue)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
// CSettingKeySlider
//////////////////////////////////////////////////////////////////////////

CSettingKeySlider::CSettingKeySlider() :
	m_initial(0),
	m_minimum(0),
	m_maximum(0),
	m_step(0)
{
}


CSettingKeySlider::~CSettingKeySlider()
{
}


void CSettingKeySlider::Setup(long initial, long minimum, long maximum, long step)
{
	m_initial = initial;
	m_minimum = minimum;
	m_maximum = maximum;
	m_step = step;
}


PSETTINGOBJECT CSettingKeySlider::CreateSetting(LPCSTR entry)
{
	return CSettingKeyLong::NewSetting(entry, m_initial, m_minimum, m_maximum);
}


PSETTINGCONFIG CSettingKeySlider::CreateConfig()
{
	return (PSETTINGCONFIG)new CSettingConfigSlider(m_controller, m_identifier,
		m_minimum, m_maximum, m_step);
}


void CSettingKeySlider::StepUp()
{
	SetValue(GetValue() + m_step);
}


void CSettingKeySlider::StepDown()
{
	SetValue(GetValue() - m_step);
}


void CSettingKeySlider::SetRange(long minimum, long maximum)
{
	m_minimum = minimum;
	m_maximum = maximum;
	CSettingKeyLong::SetRange(m_minimum, m_maximum);
}


void CSettingKeySlider::SetMin(long minimum)
{
	m_minimum = minimum;
	CSettingKeyLong::SetRange(m_minimum, m_maximum);
}


void CSettingKeySlider::SetMax(long maximum)
{
	m_maximum = maximum;
	CSettingKeyLong::SetRange(m_minimum, m_maximum);
}


long CSettingKeySlider::GetMin()
{
	return m_minimum;
}


long CSettingKeySlider::GetMax()
{
	return m_maximum;
}


void CSettingKeySlider::SetStep(long step)
{
	m_step = step;
}


//////////////////////////////////////////////////////////////////////////
// CSettingKeyDSSimple
//////////////////////////////////////////////////////////////////////////

CSettingKeyDSSimple::CSettingKeyDSSimple()
{
}

CSettingKeyDSSimple::~CSettingKeyDSSimple()
{
}

void CSettingKeyDSSimple::ChangeValue(eCHANGEVALUE method)
{
}

void CSettingKeyDSSimple::ChangeDefault(long newDefault, BOOL dontSetValue)
{
	SetDefault(newDefault);
}

void CSettingKeyDSSimple::SetDefault(long newDefault, BOOL dontSetValue)
{
	CSettingKeySlider::SetDefault(newDefault);
}

void CSettingKeyDSSimple::SetValue(long newValue, BOOL supressOnChange)
{
	CSettingKeySlider::SetValue(newValue);
}

void CSettingKeyDSSimple::OSDShow()
{
}

void CSettingKeyDSSimple::SetupControl(HWND hWnd)
{
}

void CSettingKeyDSSimple::SetControlValue(HWND hWnd)
{
}

void CSettingKeyDSSimple::SetFromControl(HWND hWnd)
{
}


//////////////////////////////////////////////////////////////////////////
// Setting Repository Utilities
//////////////////////////////////////////////////////////////////////////

static PSETTINGGROUP g_implicitGroup;
static PSETTINGGROUPEX g_ex_implicitGroup;
static DBIT g_ex_implicitDependentBits;
static DBIT g_ex_implicitAbsoluteBits;


void SRUtil_Set(PSETTINGGROUP group)
{
	g_implicitGroup = group;
}


void SRUtil_Ex_Set(PSETTINGGROUPEX groupEx, DBIT dependentBits, DBIT absoluteBits)
{
	g_ex_implicitGroup = groupEx;
	g_ex_implicitDependentBits = dependentBits;
	g_ex_implicitAbsoluteBits = absoluteBits;
}


HSETTING SRUtil_Add_Long(PSETTINGKEY key, LPCSTR displayName, LPCSTR entryName,
						 LONG defaultValue, LONG minValue, LONG maxValue)
{
	ASSERT(g_implicitGroup != NULL);
	return g_implicitGroup->AddSetting(displayName,
		CSettingKeyLong::NewSetting(entryName, defaultValue, minValue, maxValue), key);
}

HSETTING SRUtil_Add_Long(PSETTINGKEY key, LPCSTR displayName, LPCSTR entryName,
						  LONG defaultValue)
{
	ASSERT(g_implicitGroup != NULL);
	return g_implicitGroup->AddSetting(displayName,
		CSettingKeyLong::NewSetting(entryName, defaultValue), key);
}


HSETTING SRUtil_Ex_Add_Long(PSETTINGKEY key, LPCSTR displayName, LPCSTR entryName,
							LONG defaultValue)
{
	ASSERT(g_ex_implicitGroup != NULL);
	return g_ex_implicitGroup->AddSetting(displayName,
		CSettingKeyLong::NewSetting(entryName, defaultValue),
		key, g_ex_implicitDependentBits, g_ex_implicitAbsoluteBits);
}


HSETTING SRUtil_Ex_Add_Long(PSETTINGKEY key, LPCSTR displayName, LPCSTR entryName,
					 LONG defaultValue, LONG minValue, LONG maxValue)
{
	ASSERT(g_ex_implicitGroup != NULL);
	return g_ex_implicitGroup->AddSetting(displayName,
		CSettingKeyLong::NewSetting(entryName, defaultValue, minValue, maxValue),
		key, g_ex_implicitDependentBits, g_ex_implicitAbsoluteBits);
}


HSETTING SRUtil_Ex_AddMaster_Long(PSETTINGKEY key, DBIT masterBits, LPCSTR displayName, LPCSTR entryName,
						 LONG defaultValue, LONG minValue, LONG maxValue)
{
	ASSERT(g_ex_implicitGroup != NULL);
	return g_ex_implicitGroup->AddMaster(displayName,
		CSettingKeyLong::NewSetting(entryName, defaultValue, minValue, maxValue),
		key, masterBits, g_ex_implicitDependentBits, g_ex_implicitAbsoluteBits);
}


