/////////////////////////////////////////////////////////////////////////////
// $Id: ProgramList.h,v 1.12 2001-11-02 16:30:08 adcockj Exp $
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
// 24 Jul 2000   John Adcock           Original Release
//                                     Translated most code from German
//                                     Combined Header files
//                                     Cut out all decoding
//                                     Cut out digital hardware stuff
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 11 Mar 2001   Laurent Garnier       Previous Channel feature added
//
// 06 Apr 2001   Laurent Garnier       New menu to select channel
//
// 26 May 2001   Eric Schmidt          Added Custom Channel Order.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __PROGRAMLIST_H___
#define __PROGRAMLIST_H___


#include "settings.h"

// Get Hold of the OutThreads.c file settings
SETTING* Channels_GetSetting(CHANNELS_SETTING Setting);
void Channels_ReadSettingsFromIni();
void Channels_WriteSettingsToIni(BOOL bOptimizeFileAccess);
void Channels_UpdateMenu(HMENU hMenu);
void Channels_SetMenu(HMENU hMenu);
BOOL ProcessProgramSelection(HWND hWnd, WORD wMenuID);

#define MAXPROGS 255

class CChannel
{
public:
    CChannel(LPCSTR Name, DWORD Freq, int ChannelNumber, int Format, BOOL Active);
    CChannel(const CChannel& CopyFrom);
    ~CChannel();
    LPCSTR GetName() const;
    DWORD GetFrequency() const;
    int GetChannelNumber() const;
    int GetFormat() const;
    BOOL IsActive() const;
    void SetActive(BOOL Active);
private:
    string m_Name;
    DWORD m_Freq;
    int m_Chan;
    int m_Format;
    BOOL m_Active;
};

class CCountry
{
public:
    CCountry();
    ~CCountry();
    string m_Name;
    int m_MinChannel;
    int m_MaxChannel;
    vector<DWORD> m_Frequencies;
};

void Channels_Exit();
void Write_Program_List_ASCII();
void Load_Program_List_ASCII();
void Load_Country_Settings();
void Unload_Country_Settings();

BOOL APIENTRY ProgramListProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

void Channel_Change(int NewChannelIndex);
void Channel_ChangeToNumber(int NewChannelNumber);
void Channel_Increment();
void Channel_Decrement();
void Channel_SetCurrent();
void Channel_Previous();
const char* Channel_GetName();

#endif
