/////////////////////////////////////////////////////////////////////////////
// $Id: ProgramList.cpp,v 1.76 2002-09-30 16:25:18 adcockj Exp $
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
// 26 Dec 2000   Eric Schmidt          Made it possible to have whitespace in
//                                     your channel names in program.txt.
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.75  2002/09/28 13:31:41  kooiman
// Added sender object to events and added setting flag to treesettingsgeneric.
//
// Revision 1.74  2002/09/26 11:33:42  kooiman
// Use event collector
//
// Revision 1.73  2002/09/25 15:11:12  adcockj
// Preliminary code for format specific support for settings per channel
//
// Revision 1.72  2002/09/04 11:58:45  kooiman
// Added new tuners & fix for new Pinnacle cards with MT2032 tuner.
//
// Revision 1.71  2002/08/18 14:35:29  robmuller
// Changed default.
//
// Revision 1.70  2002/08/16 18:45:56  kooiman
// Added optional screen update delay during tuner frequency switch.
//
// Revision 1.69  2002/08/11 19:53:32  robmuller
// Increased default value of PostSwitchMuteDelay from 20 to 100.
//
// Revision 1.68  2002/08/06 18:35:43  kooiman
// Expandable and more independent channel change notification.
//
// Revision 1.67  2002/08/05 12:04:26  kooiman
// Added functions for channel change notification
//
// Revision 1.66  2002/08/04 12:28:32  kooiman
// Fixed previous channel feature.
//
// Revision 1.65  2002/08/02 21:59:03  laurentg
// Hide the menu "Channels" from the menu bar when the source has no tuner or when the tuner is not the selected input
//
// Revision 1.64  2002/08/02 20:33:52  laurentg
// Menu for channels without inactive channels and cut on several columns
//
// Revision 1.63  2002/08/02 19:33:24  robmuller
// Hide disabled channels from the menu.
//
// Revision 1.62  2002/08/02 18:37:35  robmuller
// Patch #588554 by Markus Debus. Change channel on remove added.
//
// Revision 1.61  2002/07/27 15:20:34  laurentg
// Channels menu updated
//
// Revision 1.60  2002/07/09 17:37:10  robmuller
// Retry on tuner write error.
//
// Revision 1.59  2002/06/18 19:46:06  adcockj
// Changed appliaction Messages to use WM_APP instead of WM_USER
//
// Revision 1.58  2002/06/13 14:00:41  adcockj
// Removed old Settings dialog header
//
// Revision 1.57  2002/06/13 12:10:22  adcockj
// Move to new Setings dialog for filers, video deint and advanced settings
//
// Revision 1.56  2002/06/13 10:40:37  robmuller
// Made anti plop mute delay configurable.
//
// Revision 1.55  2002/05/28 11:51:12  robmuller
// Prevent fine tuning to a negative frequency.
//
// Revision 1.54  2002/04/13 18:56:23  laurentg
// Checks added to manage case where the current source is not yet defined
//
// Revision 1.53  2002/03/13 15:32:45  robmuller
// Fixed problem when selecting None from the channel combo box.
//
// Revision 1.52  2002/03/11 21:38:24  robmuller
// Enabled auto scroll to the program list box.
// Insert icon from the program list is now visible before the channel has moved.
// Moving channel up/down does not scroll the list box anymore.
//
// Revision 1.51  2002/03/10 23:14:45  robmuller
// Added Clear List button.
// Scan no longer clears the program list.
// Position in program list is maintained when removing an item.
// Added support for the delete key in the program list.
// Fixed typos.
//
// Revision 1.50  2002/02/26 19:21:32  adcockj
// Add Format to Channel.txt file changes by Mike Temperton with some extra stuff by me
//
// Revision 1.49  2002/02/24 20:20:12  temperton
// Now we use currently selected video format instead of tuner default
//
// Revision 1.48  2002/02/11 21:23:54  laurentg
// Grayed certain items in the Channels menu when the current input is not the tuner
//
// Revision 1.47  2002/02/09 02:51:38  laurentg
// Grayed the channels when the source has no tuner
//
// Revision 1.46  2002/02/08 08:14:42  adcockj
// Select saved channel on startup if in tuner mode
//
// Revision 1.45  2002/01/26 17:55:13  robmuller
// Added ability to enter frequency directly.
// Fixed: When using the channel combo box the tuner was not set to the new frequency.
//
// Revision 1.44  2002/01/19 17:23:43  robmuller
// Added patch #504738 submitted by Keng Hoo Chuah (hoo)
// (fixed crash if channel.txt does not start with [country])
//
// Revision 1.43  2002/01/17 22:25:23  robmuller
// Channel searching is no longer dependant on the duration of Sleep(3).
// MT2032 channel searching speedup.
//
// Revision 1.42  2001/12/18 14:45:05  adcockj
// Moved to Common Controls status bar
//
// Revision 1.41  2001/12/05 21:45:11  ittarnavsky
// added changes for the AudioDecoder and AudioControls support
//
// Revision 1.40  2001/11/29 17:30:52  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.39  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.38  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.37  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.36  2001/11/01 12:05:21  laurentg
// Coorection of bug item #477091
//
// Revision 1.35  2001/10/17 11:46:11  adcockj
// Bug fixes
//
// Revision 1.34  2001/09/12 15:59:18  adcockj
// Added mute during scan code
//
// Revision 1.33  2001/08/23 18:54:21  adcockj
// Menu and Settings fixes
//
// Revision 1.32  2001/08/23 16:03:26  adcockj
// Improvements to dynamic menus to remove requirement that they are not empty
//
// Revision 1.31.2.7  2001/08/24 12:35:09  adcockj
// Menu handling changes
//
// Revision 1.31.2.6  2001/08/23 16:04:57  adcockj
// Improvements to dynamic menus to remove requirement that they are not empty
//
// Revision 1.31.2.5  2001/08/22 10:40:58  adcockj
// Added basic tuner support
// Fixed recusive bug
//
// Revision 1.31.2.4  2001/08/21 09:43:01  adcockj
// Brought branch up to date with latest code fixes
//
// Revision 1.31.2.3  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.31.2.2  2001/08/18 17:09:30  adcockj
// Got to compile, still lots to do...
//
// Revision 1.31.2.1  2001/08/14 16:41:37  adcockj
// Renamed driver
// Got to compile with new class based card
//
// Revision 1.31  2001/08/08 08:47:26  adcockj
// Stopped resetting program list when not in US mode
//
// Revision 1.30  2001/08/06 03:00:17  ericschmidt
// solidified auto-pixel-width detection
// preliminary pausing-of-live-tv work
//
// Revision 1.29  2001/08/05 16:31:34  adcockj
// Fixed crashing with PgUp
//
// Revision 1.28  2001/07/16 18:07:50  adcockj
// Added Optimisation parameter to ini file saving
//
// Revision 1.27  2001/07/13 18:13:24  adcockj
// Changed Mute to not be persisted and to work properly
//
// Revision 1.26  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.25  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "ProgramList.h"
#include "DScaler.h"
#include "VBI.h"
#include "Status.h"
#include "Audio.h"
#include "VBI_VideoText.h"
#include "MixerDev.h"
#include "OSD.h"
#include "Providers.h"

// From outthreads.cpp
extern BOOL bNoScreenUpdateDuringTuning;

int CurSel;
unsigned short SelectButton;
int EditProgram;
char KeyValue;
HWND ProgList;

typedef vector<CChannel*> CHANNELLIST;
typedef vector<CCountry*> COUNTRYLIST;

CHANNELLIST MyChannels;
COUNTRYLIST Countries;

int CountryCode = 1;

long CurrentProgram = 0;
long PreviousProgram = 0;
BOOL bCustomChannelOrder = FALSE;
BOOL InScan = FALSE;
BOOL InUpdate = FALSE;

int WM_DRAGLISTMESSAGE = 0;
long DragItemIndex = 0;

int PreSwitchMuteDelay = 0;
int PostSwitchMuteDelay = 0;

int TunerSwitchScreenUpdateDelay = 0;

static int PostSwitchMuteTimer = 0;
static int TunerSwitchScreenUpdateDelayTimer = 0;

static int InitialNbMenuItems = -1;


CChannel::CChannel(LPCSTR Name, DWORD Freq, int ChannelNumber, int Format, BOOL Active)
{
    m_Name = Name;
    m_Freq = Freq;
    m_Chan = ChannelNumber;
    m_Format = Format;
    m_Active = Active;
}

CChannel::CChannel(const CChannel& CopyFrom)
{
    m_Name = CopyFrom.m_Name;
    m_Freq = CopyFrom.m_Freq;
    m_Chan = CopyFrom.m_Chan;
    m_Format = CopyFrom.m_Format;
    m_Active = CopyFrom.m_Active;
}

CChannel::~CChannel()
{
}

LPCSTR CChannel::GetName() const
{
    static char sbuf[256];
    strncpy(sbuf, m_Name.c_str(), 255);
    sbuf[255] = '\0';
    return sbuf;
}

DWORD CChannel::GetFrequency() const
{
    return m_Freq;
}

int CChannel::GetChannelNumber() const
{
    return m_Chan;
}

int CChannel::GetFormat() const
{
    return m_Format;
}

BOOL CChannel::IsActive() const
{
    return m_Active;
}

void CChannel::SetActive(BOOL Active)
{
    m_Active = Active;
}

CCountry::CCountry()
{
    m_Name = "";
    m_MinChannel = 0;
    m_MaxChannel = 0;
    m_Frequencies.clear();
}

CCountry::~CCountry()
{
    m_Frequencies.clear();
}

void Channel_SetCurrent()
{
    Channel_Change(CurrentProgram);
}

const char* Channel_GetName()
{
    if(CurrentProgram < MyChannels.size())
    {
        return MyChannels[CurrentProgram]->GetName();
    }
    else
    {
        return "Unknown";
    }
}

void SelectChannel(HWND hDlg, long ChannelToSelect)
{
    ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_CHANNEL), 0);
    // then loop through looking for the correct channel
    for(int i(0); i < Countries[CountryCode]->m_Frequencies.size() + 1; ++i)
    {
        int Channel = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CHANNEL), i);
        if(Channel == ChannelToSelect)
        {
            ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_CHANNEL), i);
        }
    }

}

void UpdateDetails(HWND hDlg)
{
    InUpdate = TRUE;
    if(CurrentProgram < MyChannels.size())
    {
        char sbuf[256];

        // set the name     
        LPCSTR Name = MyChannels[CurrentProgram]->GetName();
        Edit_SetText(GetDlgItem(hDlg, IDC_NAME), Name);

        // set the frequency
        sprintf(sbuf, "%10.4f", (double)(MyChannels[CurrentProgram]->GetFrequency()) / 16.0);
        Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY),sbuf);

        // set the channel
        // select none to start off with
        SelectChannel(hDlg, (MyChannels[CurrentProgram]->GetChannelNumber()));
        
        // set format
        ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_FORMAT), (MyChannels[CurrentProgram]->GetFormat() + 1));

        // set active
        if(MyChannels[CurrentProgram]->IsActive())
        {
            Button_SetCheck(GetDlgItem(hDlg, IDC_ACTIVE), BST_CHECKED);
        }
        else
        {
            Button_SetCheck(GetDlgItem(hDlg, IDC_ACTIVE), BST_UNCHECKED);
        }
    }
    else
    {
        Edit_SetText(GetDlgItem(hDlg, IDC_NAME), "");
        Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY), "");
        ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_FORMAT), 0);
        ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_CHANNEL), 0);
        Button_SetCheck(GetDlgItem(hDlg, IDC_ACTIVE), BST_CHECKED);
    }
    InUpdate = FALSE;
}

void ResetProgramList(HWND hDlg)
{
    InUpdate = TRUE;
    CHANNELLIST::iterator it;
    ListBox_ResetContent(GetDlgItem(hDlg, IDC_PROGRAMLIST));
    CurrentProgram = 0;
    for(it = MyChannels.begin(); it != MyChannels.end(); ++it)
    {
        delete *it;
    }
    MyChannels.clear();
    if(bCustomChannelOrder)
    {
        for(int i(0); i < Countries[CountryCode]->m_Frequencies.size(); ++i)
        {
            if(Countries[CountryCode]->m_Frequencies[i].Freq != 0)
            {
                char sbuf[256];
                sprintf(sbuf, "%d", Countries[CountryCode]->m_MinChannel + i);
                MyChannels.push_back(new CChannel(sbuf, 
                                        Countries[CountryCode]->m_Frequencies[i].Freq,
                                        Countries[CountryCode]->m_MinChannel + i,
                                        Countries[CountryCode]->m_Frequencies[i].Format,
                                        TRUE));
                ListBox_AddString(GetDlgItem(hDlg, IDC_PROGRAMLIST), sbuf);
            }
        }
        ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), CurrentProgram);
        UpdateDetails(hDlg);
    }
    InUpdate = FALSE;
}

void RefreshProgramList(HWND hDlg, long ProgToSelect)
{
    InUpdate = TRUE;
    CHANNELLIST::iterator it;

    ListBox_ResetContent(GetDlgItem(hDlg, IDC_PROGRAMLIST));

    for(it = MyChannels.begin(); it != MyChannels.end(); ++it)
    {
        ListBox_AddString(GetDlgItem(hDlg, IDC_PROGRAMLIST), (*it)->GetName());
    }

    ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), ProgToSelect);
    InUpdate = FALSE;
}

void RefreshChannelList(HWND hDlg)
{
    InUpdate = TRUE;
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_CHANNEL));
    int Index = ComboBox_AddString(GetDlgItem(hDlg, IDC_CHANNEL), "None");
    SendMessage(GetDlgItem(hDlg, IDC_CHANNEL), CB_SETITEMDATA, Index, 0);
    for(int i(0); i < Countries[CountryCode]->m_Frequencies.size(); ++i)
    {
        if(Countries[CountryCode]->m_Frequencies[i].Freq != 0)
        {
            char sbuf[256];
            sprintf(sbuf, "%d", Countries[CountryCode]->m_MinChannel + i);
            Index = ComboBox_AddString(GetDlgItem(hDlg, IDC_CHANNEL), sbuf);
            ComboBox_SetItemData(GetDlgItem(hDlg, IDC_CHANNEL), Index, Countries[CountryCode]->m_MinChannel + i);
        }
    }
    InUpdate = FALSE;
}

//returns TRUE if a video signal is found
BOOL FindFrequency(DWORD Freq, int Format)
{
    char sbuf[256];

    if(Format == -1)
    {
        Format = VIDEOFORMAT_LASTONE;
    }

    if (!Providers_GetCurrentSource()->SetTunerFrequency(Freq, (eVideoFormat)Format))
    {
        sprintf(sbuf, "SetFrequency %10.2f Failed.", (float) Freq / 16.0);
        ErrorBox(sbuf);
        return false;
    }

    int       MaxTuneDelay = 0;

    switch(Providers_GetCurrentSource()->GetTunerId())
    {
        // The MT2032 is a silicon tuner and tunes real fast, no delay needed at this point.
        // Even channels with interference and snow are tuned and detected in about max 80ms,
        // so 120ms seems to be a safe value.
    case TUNER_MT2032:
    case TUNER_MT2032_PAL:
        MaxTuneDelay = 120;
        break;
    default:
        MaxTuneDelay = 225;
        Sleep(100);
        break;
    }

    int       StartTick = 0;
    int       ElapsedTicks = 0;

    StartTick = GetTickCount();
    while (Providers_GetCurrentSource()->IsVideoPresent() == FALSE)
    {
        MSG msg;
        if (PeekMessage(&msg, NULL, 0, 0xffffffff, PM_REMOVE) == TRUE)
        {
            SendMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
        }

        if(ElapsedTicks > MaxTuneDelay)
        {
            break;
        }
 
        ElapsedTicks = GetTickCount() - StartTick;
        Sleep(3);
    }
    return Providers_GetCurrentSource()->IsVideoPresent();
}

void ScanCustomChannel(HWND hDlg, int ChannelNum)
{
    BOOL result;
    InUpdate = TRUE;

    if(ChannelNum < 0 || ChannelNum >= MyChannels.size())
    {
        return;
    }

    MyChannels[ChannelNum]->SetActive(FALSE);

    CurrentProgram = ChannelNum;
    UpdateDetails(hDlg);
    ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), ChannelNum);

    result = FindFrequency(MyChannels[ChannelNum]->GetFrequency(), MyChannels[ChannelNum]->GetFormat());

    MyChannels[ChannelNum]->SetActive(result);
    InUpdate = FALSE;
}

void ScanFrequency(HWND hDlg, int FreqNum)
{
    InUpdate = TRUE;

    if(FreqNum < 0 || FreqNum >= Countries[CountryCode]->m_Frequencies.size())
    {
        return;
    }

    char sbuf[256];

    DWORD Freq = Countries[CountryCode]->m_Frequencies[FreqNum].Freq;
    
    if(Freq == 0)
    {
        return;
    }

    for(int i = 0; i < MyChannels.size(); i++)
    {
        if(MyChannels[i]->GetFrequency() == Freq)
        {
            return;
        }
    }    

    int Format = Countries[CountryCode]->m_Frequencies[FreqNum].Format;

    SelectChannel(hDlg, FreqNum + Countries[CountryCode]->m_MinChannel);
    ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_FORMAT), Format + 1);

    sprintf(sbuf, "%10.4f", (double)Freq / 16.0);
    Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf);

    if(FindFrequency(Freq, Format))
    {
        char sbuf[256];
        ++CurrentProgram;
        sprintf(sbuf, "Channel %d", CurrentProgram);
        MyChannels.push_back(new CChannel(
                                            sbuf, 
                                            Freq, 
                                            Countries[CountryCode]->m_MinChannel + FreqNum, 
                                            Format, 
                                            TRUE
                                         ));
        ListBox_AddString(GetDlgItem(hDlg, IDC_PROGRAMLIST), sbuf);
        ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), CurrentProgram - 1);
    }
    InUpdate = FALSE;
}

void RefreshControls(HWND hDlg)
{
    InUpdate = TRUE;
    Button_Enable(GetDlgItem(hDlg, IDC_ADD), (bCustomChannelOrder == FALSE));
    Button_Enable(GetDlgItem(hDlg, IDC_REMOVE), (bCustomChannelOrder == FALSE));
    Button_Enable(GetDlgItem(hDlg, IDC_UP), (bCustomChannelOrder == FALSE));
    Button_Enable(GetDlgItem(hDlg, IDC_DOWN), (bCustomChannelOrder == FALSE));
    ComboBox_Enable(GetDlgItem(hDlg, IDC_CHANNEL), (bCustomChannelOrder == FALSE));
    InUpdate = FALSE;
}

void ChangeChannelInfo(HWND hDlg)
{
    InUpdate = TRUE;
    char sbuf[265];

    if(CurrentProgram < MyChannels.size())
    {
        char* cLast;
        Edit_GetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf, 255);
        double dFreq = strtod(sbuf, &cLast);
        long Freq = (long)(dFreq * 16.0);
        int Channel = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_CHANNEL));
        Channel = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CHANNEL), Channel);
        int Format = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_FORMAT)) - 1;
        delete MyChannels[CurrentProgram];
        Edit_GetText(GetDlgItem(hDlg, IDC_NAME), sbuf , 255);
        BOOL Active = (Button_GetCheck(GetDlgItem(hDlg, IDC_ACTIVE)) == BST_CHECKED);
        MyChannels[CurrentProgram] = new CChannel(sbuf, Freq, Channel, Format, Active);
        ListBox_DeleteString(GetDlgItem(hDlg, IDC_PROGRAMLIST), CurrentProgram);
        ListBox_InsertString(GetDlgItem(hDlg, IDC_PROGRAMLIST), CurrentProgram, sbuf);
        ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), CurrentProgram);
    }
    InUpdate = FALSE;
}

eVideoFormat SelectedVideoFormat(HWND hDlg)
{
    int Format = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_FORMAT)) - 1;
    if(Format == -1)
    {
        Format = VIDEOFORMAT_LASTONE;
    }

    return (eVideoFormat)Format;
}

BOOL APIENTRY ProgramListProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    int i;
    char sbuf[256];
    static BOOL OldCustom;
    static int OldCountryCode;

    switch (message)
    {
    case WM_INITDIALOG:
        InScan = FALSE;
        InUpdate = FALSE;
        SetCapture(hDlg);
        RefreshControls(hDlg);
        ListBox_ResetContent(GetDlgItem(hDlg, IDC_PROGRAMLIST));
        RefreshProgramList(hDlg, CurrentProgram);


        WM_DRAGLISTMESSAGE = RegisterWindowMessage(DRAGLISTMSGSTRING);
        MakeDragList(GetDlgItem(hDlg, IDC_PROGRAMLIST));
        
        OldCustom = bCustomChannelOrder;
        OldCountryCode = CountryCode;
        Button_SetCheck(GetDlgItem(hDlg, IDC_CUSTOMCHANNELORDER), bCustomChannelOrder?BST_CHECKED:BST_UNCHECKED);

        SetFocus(GetDlgItem(hDlg, IDC_PROGRAMLIST)); 

        ScrollBar_SetRange(GetDlgItem(hDlg, IDC_FINETUNE), 0, 100, FALSE);
        ScrollBar_SetPos(GetDlgItem(hDlg, IDC_FINETUNE), 50, FALSE);

        // fill the formats box
        ComboBox_AddString(GetDlgItem(hDlg, IDC_FORMAT), "Same as Tuner");
        for(i = 0; i < VIDEOFORMAT_LASTONE; ++i)
        {
            ComboBox_AddString(GetDlgItem(hDlg, IDC_FORMAT), VideoFormatNames[i]);
        }

        // load up the country settings
        Load_Country_Settings();
        if(Countries.size() > 0)
        {
            ComboBox_ResetContent(GetDlgItem(hDlg, IDC_COUNTRY));
            i = 0;
            for(COUNTRYLIST::iterator it = Countries.begin(); 
                it != Countries.end(); 
                ++it)
            {
                ComboBox_AddString(GetDlgItem(hDlg, IDC_COUNTRY), ((*it)->m_Name.c_str()));
                i++;
            }
            ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_COUNTRY), CountryCode);
            
            RefreshChannelList(hDlg);

            // if we have any channels then also fill the details box with the current program
            UpdateDetails(hDlg);

        }
        else
        {
            ErrorBox("No countries Loaded, Channels.txt must be missing");
            EndDialog(hDlg, 0);
        }
        break;

    case WM_HSCROLL:
        if(InUpdate == FALSE)
        {
            char* cLast;
            if(LOWORD(wParam) == SB_LEFT ||
                LOWORD(wParam) == SB_PAGELEFT ||
                LOWORD(wParam) == SB_LINELEFT)
            {
                Edit_GetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf, 255);
                double dFreq = strtod(sbuf, &cLast);
                int Freq = (int)(dFreq * 16.0);
                --Freq;
                if(Freq < 0)
                {
                    Freq = 0;
                }
                sprintf(sbuf, "%10.4f", (double)Freq / 16.0);
                Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf);
                Providers_GetCurrentSource()->SetTunerFrequency(Freq, SelectedVideoFormat(hDlg));
                ChangeChannelInfo(hDlg);
            }
            else if(LOWORD(wParam) == SB_RIGHT ||
                LOWORD(wParam) == SB_PAGERIGHT ||
                LOWORD(wParam) == SB_LINERIGHT)
            {
                Edit_GetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf, 255);
                double dFreq = strtod(sbuf, &cLast);
                long Freq = (long)(dFreq * 16.0);
                ++Freq;
                sprintf(sbuf, "%10.4f", (double)Freq / 16.0);
                Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf);
                Providers_GetCurrentSource()->SetTunerFrequency(Freq, SelectedVideoFormat(hDlg));
                ChangeChannelInfo(hDlg);
            }
        }
        break;
    case WM_APP:
        if(lParam == 101)
        {
            if(InScan == TRUE)
            {
                if(bCustomChannelOrder)
                {
                    ScanCustomChannel(hDlg, wParam);
                    if(wParam < MyChannels.size())
                    {
                        PostMessage(hDlg, WM_APP, wParam + 1, 101);
                    }
                    else
                    {
                        InScan = FALSE;
                        PostMessage(hDlg, WM_APP, -1, 101);
                    }
                }
                else
                {
                    ScanFrequency(hDlg, wParam);
                    if(wParam < Countries[CountryCode]->m_Frequencies.size())
                    {
                        PostMessage(hDlg, WM_APP, wParam + 1, 101);
                    }
                    else
                    {
                        InScan = FALSE;
                        PostMessage(hDlg, WM_APP, -1, 101);
                    }
                }
            }
            else
            {
                Button_SetText(GetDlgItem(hDlg, IDC_SCAN), "Scan");
                if(MyChannels.size() > 0)
                {
                    CurrentProgram = CurrentProgram = MyChannels.size()-1;
                    ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), CurrentProgram);
                    Channel_Change(CurrentProgram);
                }
                UpdateDetails(hDlg);
                Audio_Unmute();
            }
        }
        break;
    case WM_VKEYTOITEM:
        if(LOWORD(wParam) == VK_DELETE && (HWND)lParam == GetDlgItem(hDlg, IDC_PROGRAMLIST))
        {
            SendMessage(hDlg, WM_COMMAND, IDC_REMOVE, 0);
        }
        // let the list box handle any key presses.
        return -1;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_PROGRAMLIST:
            if (InUpdate == FALSE && HIWORD(wParam) == LBN_SELCHANGE)
            {
                i = ListBox_GetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST));

                if ((i >= 0) && (i < MyChannels.size()))
                {
                    CurrentProgram = i;
                    Channel_Change(CurrentProgram);
                }
                else
                {
                    CurrentProgram = 0;
                }
                UpdateDetails(hDlg);
            }
            break;

        case IDC_COUNTRY:
            if(bCustomChannelOrder)
            {
                ResetProgramList(hDlg);
            }
            CountryCode = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_COUNTRY));
            RefreshChannelList(hDlg);
            break;

        case IDC_CUSTOMCHANNELORDER:
            bCustomChannelOrder = (Button_GetCheck(GetDlgItem(hDlg, IDC_CUSTOMCHANNELORDER)) == BST_CHECKED);
            RefreshControls(hDlg);
            ResetProgramList(hDlg);
            break;

        case IDC_CHANNEL:
            if(InUpdate == FALSE && HIWORD(wParam) == CBN_SELCHANGE)
            {
                char sbuf[256];
                // set the frequency
                int Channel = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_CHANNEL));
                long Freq = 0;
                int Format = -1;
                Channel = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CHANNEL), Channel);
                if(Channel != 0)
                {
                    Freq = Countries[CountryCode]->m_Frequencies[Channel - Countries[CountryCode]->m_MinChannel].Freq;
                    Format = Countries[CountryCode]->m_Frequencies[Channel - Countries[CountryCode]->m_MinChannel].Format;
                }
                sprintf(sbuf, "%10.4f", Freq / 16.0);
                Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY),sbuf);
                ScrollBar_SetPos(GetDlgItem(hDlg, IDC_FINETUNE), 50, FALSE);
                ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_FORMAT), Format + 1);
                Providers_GetCurrentSource()->SetTunerFrequency(Freq, SelectedVideoFormat(hDlg));
                ChangeChannelInfo(hDlg);
            }
            break;

        case IDC_SETFREQ:
            if(InUpdate == FALSE)
            {
                char* cLast;
                Edit_GetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf, 255);
                double dFreq = strtod(sbuf, &cLast);
                long Freq = (long)(dFreq * 16.0);
                sprintf(sbuf, "%10.4f", (double)Freq / 16.0);
                Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf);
                Providers_GetCurrentSource()->SetTunerFrequency(Freq, SelectedVideoFormat(hDlg));
                ChangeChannelInfo(hDlg);
            }
            break;

        case IDC_NAME:
            if(InUpdate == FALSE)
            {
                ChangeChannelInfo(hDlg);
            }
            break;

        case IDC_ACTIVE:
            if(InUpdate == FALSE)
            {
                ChangeChannelInfo(hDlg);
            }
            break;

        case IDC_FORMAT:
            if(InUpdate == FALSE && HIWORD(wParam) == CBN_SELCHANGE)
            {
                Edit_GetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf, 255);
                char* cLast;
                double dFreq = strtod(sbuf, &cLast);
                long Freq = (long)(dFreq * 16.0);
                Providers_GetCurrentSource()->SetTunerFrequency(Freq, SelectedVideoFormat(hDlg));
                ChangeChannelInfo(hDlg);
            }
            break;

        case IDC_ADD:
            {
                InUpdate = TRUE;
                char* cLast;
                Edit_GetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf, 255);
                double dFreq = strtod(sbuf, &cLast);
                long Freq = (long)(dFreq * 16.0);

                int Channel = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_CHANNEL));
                Channel = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CHANNEL), Channel);
                int Format = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_FORMAT)) - 1;
                Edit_GetText(GetDlgItem(hDlg, IDC_NAME), sbuf , 255);
                BOOL Active = (Button_GetCheck(GetDlgItem(hDlg, IDC_ACTIVE)) == BST_CHECKED);
                MyChannels.push_back(new CChannel(sbuf, Freq, Channel, Format, Active));
                CurrentProgram = ListBox_AddString(GetDlgItem(hDlg, IDC_PROGRAMLIST), sbuf);
                ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), CurrentProgram);
                InUpdate = FALSE;
            }
            break;
        case IDC_REMOVE:
            if(CurrentProgram >= 0 && CurrentProgram < MyChannels.size())
            {
                int TopIndex = 0;
                delete MyChannels[CurrentProgram];
                MyChannels.erase(&MyChannels[CurrentProgram]);
                if(CurrentProgram >= MyChannels.size())
                {
                    CurrentProgram = MyChannels.size() - 1;
                }
                Channel_Change(CurrentProgram);
                TopIndex = ListBox_GetTopIndex(GetDlgItem(hDlg, IDC_PROGRAMLIST));
                RefreshProgramList(hDlg, CurrentProgram);
                ListBox_SetTopIndex(GetDlgItem(hDlg, IDC_PROGRAMLIST), TopIndex);
                UpdateDetails(hDlg);
            }
            break;
        case IDC_UP:
            if(CurrentProgram > 0 && CurrentProgram < MyChannels.size())
            {
                int TopIndex = 0;
                CChannel* Temp = MyChannels[CurrentProgram];
                MyChannels[CurrentProgram] = MyChannels[CurrentProgram - 1];
                MyChannels[CurrentProgram - 1] = Temp;
                --CurrentProgram; 
                TopIndex = ListBox_GetTopIndex(GetDlgItem(hDlg, IDC_PROGRAMLIST));
                RefreshProgramList(hDlg, CurrentProgram);
                ListBox_SetTopIndex(GetDlgItem(hDlg, IDC_PROGRAMLIST), TopIndex);
            }
            break;
        case IDC_DOWN:
            if(CurrentProgram >= 0 && CurrentProgram < MyChannels.size() - 1)
            {
                int TopIndex = 0;
                CChannel* Temp = MyChannels[CurrentProgram];
                MyChannels[CurrentProgram] = MyChannels[CurrentProgram + 1];
                MyChannels[CurrentProgram + 1] = Temp;
                ++CurrentProgram; 
                TopIndex = ListBox_GetTopIndex(GetDlgItem(hDlg, IDC_PROGRAMLIST));
                RefreshProgramList(hDlg, CurrentProgram);
                ListBox_SetTopIndex(GetDlgItem(hDlg, IDC_PROGRAMLIST), TopIndex);
            }
            break;
        case IDC_CLEAR:
            if(!bCustomChannelOrder)
            {
                ResetProgramList(hDlg);
                Edit_SetText(GetDlgItem(hDlg, IDC_NAME), "");
                ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_FORMAT), 0);
            }
            break;
        case IDC_SCAN:
            if(InScan == TRUE)
            {
                InScan = FALSE;
                PostMessage(hDlg, WM_APP, -1, 101);
            }
            else
            {
                InScan = TRUE;
                Audio_Mute();
                Button_SetText(GetDlgItem(hDlg, IDC_SCAN), "Cancel");
                CurrentProgram = MyChannels.size();
                PostMessage(hDlg, WM_APP, 0, 101);
            }
            break;
        case IDOK:
            Write_Program_List_ASCII();
            Unload_Country_Settings();
			WriteSettingsToIni(TRUE);
            EndDialog(hDlg, TRUE);
            break;
        case IDCANCEL:
            bCustomChannelOrder = OldCustom;
            CountryCode = OldCountryCode;
            Load_Program_List_ASCII();
            Unload_Country_Settings();
            EndDialog(hDlg, TRUE);
            break;
        }
        break;
    }


    if(message == WM_DRAGLISTMESSAGE)
    {
        int Item = 0;
        LPDRAGLISTINFO pDragInfo = (LPDRAGLISTINFO) lParam; 
        switch(pDragInfo->uNotification)
        {
        case DL_BEGINDRAG:
            DragItemIndex = ListBox_GetCurSel(pDragInfo->hWnd);
            SetWindowLong(hDlg, DWL_MSGRESULT, TRUE);
            Item = LBItemFromPt(pDragInfo->hWnd, pDragInfo->ptCursor, FALSE);
            DrawInsert(hDlg, pDragInfo->hWnd, Item);
            break;
        case DL_DROPPED:
            DrawInsert(hDlg, pDragInfo->hWnd, -1);               
            Item = LBItemFromPt(pDragInfo->hWnd, pDragInfo->ptCursor, FALSE);
            if((Item >= 0) && (Item != DragItemIndex)) 
            {
                CChannel* Temp = MyChannels[DragItemIndex];
                CurrentProgram = DragItemIndex;
                if(Item < DragItemIndex)
                {
                    while(CurrentProgram > Item)
                    {
                        MyChannels[CurrentProgram] = MyChannels[CurrentProgram - 1];
                        --CurrentProgram;
                    }
                }
                else
                {
                    while(CurrentProgram < Item)
                    {
                        MyChannels[CurrentProgram] = MyChannels[CurrentProgram + 1];
                        ++CurrentProgram;
                    }
                }
                MyChannels[Item] = Temp;
                CurrentProgram = Item; 

                int TopIndex = 0;
                TopIndex = ListBox_GetTopIndex(GetDlgItem(hDlg, IDC_PROGRAMLIST));
                RefreshProgramList(hDlg, CurrentProgram);
                ListBox_SetTopIndex(GetDlgItem(hDlg, IDC_PROGRAMLIST), TopIndex);
            }
            break;
        case DL_CANCELDRAG:
            DrawInsert(hDlg, pDragInfo->hWnd, -1);
            break;
        case DL_DRAGGING:
            Item = LBItemFromPt(pDragInfo->hWnd, pDragInfo->ptCursor, TRUE);
            DrawInsert(hDlg, pDragInfo->hWnd, Item);
            SetWindowLong(hDlg, DWL_MSGRESULT, DL_MOVECURSOR);
            break;
        }
        return (TRUE);
    }

    return (FALSE);
}

// 
// Save ascii formatted program list
//
// 9 Novemeber 2000 - Michael Eskin, Conexant Systems
//
// List is a simple text file with the following format:
// Name <display_name>
// Freq <frequency_KHz>
// Name <display_name>
// Freq <frequency_KHz>
// ...
//
void Write_Program_List_ASCII()
{
    FILE* SettingFile;
    CHANNELLIST::iterator it;
    
    if ((SettingFile = fopen("program.txt", "w")) != NULL)
    {
        for(it = MyChannels.begin(); it != MyChannels.end(); ++it)
        {
            fprintf(SettingFile, "Name: %s\n", (*it)->GetName());
            fprintf(SettingFile, "Freq2: %ld\n", (*it)->GetFrequency());
            fprintf(SettingFile, "Chan: %d\n", (*it)->GetChannelNumber());
            fprintf(SettingFile, "Active: %d\n", (*it)->IsActive());
            if((*it)->GetFormat() != -1)
            {
                fprintf(SettingFile, "Form: %d\n", (*it)->GetFormat());
            }
        }
        fclose(SettingFile);
    }
}

void Channels_Exit()
{
    CHANNELLIST::iterator it;

    // Zero out the program list
    for(it = MyChannels.begin(); it != MyChannels.end(); ++it)
    {
        delete (*it);
    }
    MyChannels.clear();
}


// 
// Load ascii formatted program list
//
// 9 Novemeber 2000 - Michael Eskin, Conexant Systems
//
// List is a simple text file with the following format:
// Name <display_name>
// Freq <frequency_KHz>
// Name <display_name>
// Freq <frequency_KHz>
// ...
//

void Load_Program_List_ASCII()
{
    char sbuf[256];
    FILE* SettingFile;
    CHANNELLIST::iterator it;
    DWORD Frequency = -1;
    int Channel = 1;
    int Format = -1;
    BOOL Active = TRUE;
    string Name;

    // Zero out the program list
    for(it = MyChannels.begin(); it != MyChannels.end(); ++it)
    {
        delete (*it);
    }
    MyChannels.clear();

    SettingFile = fopen("program.txt", "r");
    if (SettingFile == NULL)
    {
        return;
    }
    while(!feof(SettingFile))
    {
        sbuf[0] = '\0';

        fgets(sbuf, 255, SettingFile);

        char* eol_ptr = strstr(sbuf, ";");
        if (eol_ptr == NULL)
        {
            eol_ptr = strstr(sbuf, "\n");
        }
        if (eol_ptr != NULL)
        {
            *eol_ptr = '\0';
        }


        if(strnicmp(sbuf, "Name:", 5) == 0)
        {
            if(Frequency != -1)
            {
                MyChannels.push_back(new CChannel(Name.c_str(), Frequency, Channel, Format, Active));
            }

            // skip "Name:"
            char* StartChar = sbuf + 5;

            // skip any spaces
            while(iswspace(*StartChar))
            {
                ++StartChar;
            }
            if(strlen(StartChar) > 0)
            {
                char* EndChar = StartChar + strlen(StartChar) - 1;
                while(EndChar > StartChar && iswspace(*EndChar))
                {
                    *EndChar = '\0';
                    --EndChar;
                }
                Name = StartChar;
            }
            else
            {
                Name = "Empty";
            }
            Frequency = -1;
            ++Channel;
            Format = -1;
            Active = TRUE;
        }
        // cope with old style frequencies
        else if(strnicmp(sbuf, "Freq:", 5) == 0)
        {
            Frequency = atol(sbuf + 5);
            Frequency = MulDiv(Frequency, 16, 1000);
        }
        else if(strnicmp(sbuf, "Freq2:", 6) == 0)
        {
            Frequency = atol(sbuf + 6);
        }
        else if(strnicmp(sbuf, "Chan:", 5) == 0)
        {
            Channel = atoi(sbuf + 5);
        }
        else if(strnicmp(sbuf, "Form:", 5) == 0)
        {
            Format = atoi(sbuf + 5);
        }
        else if(strnicmp(sbuf, "Active:", 7) == 0)
        {
            Active = (atoi(sbuf + 7) != 0);
        }
        else
        {
            ; //some other rubbish
        }
    }

    if(Frequency != -1)
    {
        MyChannels.push_back(new CChannel(Name.c_str(), Frequency, Channel, Format, Active));
    }

    fclose(SettingFile);
    return;
}


static VOID CALLBACK TunerSwitchScreenUpdateDelayTimerProc( 
    HWND hwnd,        // handle to window for timer messages 
    UINT message,     // WM_TIMER message 
    UINT idTimer,     // timer identifier 
    DWORD dwTime)
{
    TunerSwitchScreenUpdateDelayTimer = 0;
    KillTimer(hwnd, idTimer);    
    bNoScreenUpdateDuringTuning = FALSE;
}

static VOID CALLBACK PostSwitchMuteDelayTimerProc( 
    HWND hwnd,        // handle to window for timer messages 
    UINT message,     // WM_TIMER message 
    UINT idTimer,     // timer identifier 
    DWORD dwTime)
{
    PostSwitchMuteTimer = 0;
    KillTimer(hwnd, idTimer);    
    Audio_Unmute();
}


//---------------------------------------------------------------------------
void Channel_Change(int NewChannel, int DontStorePrevious)
{
    eVideoFormat VideoFormat;

    if (Providers_GetCurrentSource()->HasTuner() == TRUE)
    {
        if(NewChannel >= 0 && NewChannel < MyChannels.size())
        {
            if (MyChannels[NewChannel]->GetFrequency() != 0)
            {
				int OldChannel = CurrentProgram;
                Audio_Mute();
                Sleep(PreSwitchMuteDelay); // This helps reduce the static click noise.                
                if (EventCollector != NULL)
                {
                    EventCollector->RaiseEvent(Providers_GetCurrentSource(), EVENT_CHANNEL_PRECHANGE, OldChannel, NewChannel);
                }
                if (!DontStorePrevious)
                {
                    PreviousProgram = CurrentProgram;
                }
                CurrentProgram = NewChannel;
                if(MyChannels[CurrentProgram]->GetFormat() != -1)
                {
                    VideoFormat = (eVideoFormat)MyChannels[CurrentProgram]->GetFormat();
                }
                else
                {
                    VideoFormat = VIDEOFORMAT_LASTONE;
                }
                
                if (TunerSwitchScreenUpdateDelay > 0)
                {
                    if (TunerSwitchScreenUpdateDelayTimer > 0)
                    {
                        bNoScreenUpdateDuringTuning = FALSE;
                        KillTimer(NULL, TunerSwitchScreenUpdateDelayTimer);
                    }                    
                    TunerSwitchScreenUpdateDelayTimer = SetTimer(NULL, NULL, TunerSwitchScreenUpdateDelay, TunerSwitchScreenUpdateDelayTimerProc);
                    bNoScreenUpdateDuringTuning = TRUE;                
                }
                // try up to three times if something goes wrong.
                // \todo: fix tuner write errors
                for(int i = 0; i < 3; i++)
                {
                    if(Providers_GetCurrentSource()->SetTunerFrequency(
                                                     MyChannels[CurrentProgram]->GetFrequency(), 
                                                     VideoFormat))
                    {
                        break;
                    }
                }
                
                if (PostSwitchMuteDelay > 0)
                {
                    if (PostSwitchMuteTimer > 0)
                    {
                        KillTimer(NULL, PostSwitchMuteTimer);
                    }
                    PostSwitchMuteTimer = SetTimer(NULL, NULL, PostSwitchMuteDelay, PostSwitchMuteDelayTimerProc);
                }
                else
                {
                    Audio_Unmute(); 
                }

                if (EventCollector != NULL)
                {
                    EventCollector->RaiseEvent(Providers_GetCurrentSource(), EVENT_CHANNEL_CHANGE, OldChannel, NewChannel);
                }
                //Sleep(PostSwitchMuteDelay); //now timer controlled
                VT_ChannelChange();                                

                StatusBar_ShowText(STATUS_TEXT, MyChannels[CurrentProgram]->GetName());
                OSD_ShowText(hWnd,MyChannels[CurrentProgram]->GetName(), 0);
				
            }
        }
    }
}

void Channel_Reset()
{
    Channel_Change(CurrentProgram);
}

void Channel_Increment()
{
    int CurrentProg;

    if(MyChannels.size() > 0)
    {
        CurrentProg = CurrentProgram;
        PreviousProgram = CurrentProg;
        // look for next active channel
        ++CurrentProg;
        while(CurrentProg < MyChannels.size() && 
            !MyChannels[CurrentProg]->IsActive())
        {
            ++CurrentProg;
        }

        // see if we looped around
        if(CurrentProg == MyChannels.size())
        {
            CurrentProg = 0;
            while(CurrentProg < MyChannels.size() && 
                !MyChannels[CurrentProg]->IsActive())
            {
                ++CurrentProg;
            }

            // see if we looped around again
            if(CurrentProg == MyChannels.size())
            {
                CurrentProg = 0;
            }
        }
    
        Channel_Change(CurrentProg);

        StatusBar_ShowText(STATUS_TEXT, MyChannels[CurrentProgram]->GetName());
        OSD_ShowText(hWnd,MyChannels[CurrentProgram]->GetName(), 0);
    }
    else
    {
        StatusBar_ShowText(STATUS_TEXT, "No Channels");
        OSD_ShowText(hWnd, "No Channels", 0);
    }
}

void Channel_Decrement()
{
    int CurrentProg;

    if(MyChannels.size() > 0)
    {
        CurrentProg = CurrentProgram;
        PreviousProgram = CurrentProg;
        // look for next active channel
        --CurrentProg;
        while(CurrentProg > -1 && 
            !MyChannels[CurrentProg]->IsActive())
        {
            --CurrentProg;
        }

        // see if we looped around
        if(CurrentProg == -1)
        {
            CurrentProg = MyChannels.size() - 1;
            while(CurrentProg > -1  && 
                !MyChannels[CurrentProg]->IsActive())
            {
                --CurrentProg;
            }

            // see if we looped around again
            if(CurrentProg == -1)
            {
                CurrentProg = 0;
            }
        }
    
        Channel_Change(CurrentProg);

        StatusBar_ShowText(STATUS_TEXT, MyChannels[CurrentProgram]->GetName());
        OSD_ShowText(hWnd,MyChannels[CurrentProgram]->GetName(), 0);
    }
    else
    {
        StatusBar_ShowText(STATUS_TEXT, "No Channels");
        OSD_ShowText(hWnd, "No Channels", 0);
    }
}

void Channel_Previous()
{
    if(MyChannels.size() > 0)
    {
        if (MyChannels[PreviousProgram]->GetFrequency() != 0)
            Channel_Change(PreviousProgram);

        StatusBar_ShowText(STATUS_TEXT, MyChannels[CurrentProgram]->GetName());
        OSD_ShowText(hWnd,MyChannels[CurrentProgram]->GetName(), 0);
    }
    else
    {
        StatusBar_ShowText(STATUS_TEXT, "No Channels");
        OSD_ShowText(hWnd, "No Channels", 0);
    }

}

void Channel_ChangeToNumber(int ChannelNumber, int DontStorePrevious)
{
    BOOL found = FALSE;

    if (bCustomChannelOrder)
    {
        // Find the channel the user typed.
        for (int j = 0; j < MyChannels.size(); ++j)
        {
            if (MyChannels[j]->GetFrequency() != 0 && int(MyChannels[j]->GetChannelNumber()) == ChannelNumber)
            {
                found = TRUE;
                ChannelNumber = j;
                break;
            }
        }
    }
    else
    {
        found = TRUE;
        ChannelNumber = ChannelNumber - 1;
    }

    if (found)
    {
        Channel_Change(ChannelNumber, DontStorePrevious);
        found = CurrentProgram == ChannelNumber;
    }

    if (found)
    {
        StatusBar_ShowText(STATUS_TEXT, MyChannels[CurrentProgram]->GetName());
        OSD_ShowText(hWnd, MyChannels[CurrentProgram]->GetName(), 0);
    }
    else
    {
        StatusBar_ShowText(STATUS_TEXT, "Not Found");
        OSD_ShowText(hWnd, "Not Found", 0);
    }
}

void Unload_Country_Settings()
{
    COUNTRYLIST::iterator it;

    // Zero out the program list
    for(it = Countries.begin(); it != Countries.end(); ++it)
    {
        delete (*it);
    }
    Countries.clear();
}

int StrToVideoFormat(char* pszFormat)
{
    for(int a = 0; a < VIDEOFORMAT_LASTONE; ++a)
    {
        if(!stricmp(pszFormat, VideoFormatNames[a]))
        {
            return a;
        }  
    }
   
    return -1;
}

void Load_Country_Settings()
{
    FILE*     CountryFile;
    char      line[128];
    char*     Pos;
    char*     Pos1;
    char*     eol_ptr;
    string    Name;
    CCountry* NewCountry = NULL;
    int       Format = -1;

    if ((CountryFile = fopen("Channel.txt", "r")) == NULL)
    {
        ErrorBox("File Channel.txt not Found");
        return;
    }

    while (fgets(line, sizeof(line), CountryFile) != NULL)
    {
        eol_ptr = strstr(line, ";");
        if (eol_ptr == NULL)
        {
            eol_ptr = strstr(line, "\n");
        }
        if(eol_ptr != NULL)
        {
            *eol_ptr = '\0';
        }
        if(eol_ptr == line)
        {
            continue;
        }
        if(((Pos = strstr(line, "[")) != 0) && ((Pos1 = strstr(line, "]")) != 0) && Pos1 > Pos)
        {
            if(NewCountry != NULL)
            {
                Countries.push_back(NewCountry);
            }
            Pos++;
            NewCountry = new CCountry();
            NewCountry->m_Name = Pos;
            NewCountry->m_Name[Pos1-Pos] = '\0';
            Format = -1;
        }
        else
        {
            if (NewCountry == NULL)
            {
                fclose(CountryFile);
                ErrorBox("Invalid Channel.txt");
                return;
            }
            
            if ((Pos = strstr(line, "ChannelLow=")) != 0)
            {
                NewCountry->m_MinChannel = atoi(Pos + strlen("ChannelLow="));
            }
            else if ((Pos = strstr(line, "ChannelHigh=")) != 0)
            {
                NewCountry->m_MaxChannel = atoi(Pos + strlen("ChannelHigh="));
            }
            else if ((Pos = strstr(line, "Format=")) != 0)
            {
                Format = StrToVideoFormat(Pos + strlen("Format="));
            }
            else
            {
                Pos = line;
                while (*Pos != '\0')
                {
                    if ((*Pos >= '0') && (*Pos <= '9'))
                    {
                        // convert frequency in KHz to Units that the tuner wants
                        TCountryChannel Channel;
                        Channel.Freq = atol(Pos);
                        Channel.Freq = MulDiv(Channel.Freq, 16, 1000000);
                        Channel.Format = Format;
                        NewCountry->m_Frequencies.push_back(Channel);
                        break;
                    }
                    Pos++;
                }
            }
        }
    }
    if(NewCountry != NULL)
    {
        Countries.push_back(NewCountry);
    }

    fclose(CountryFile);
}

void Channels_UpdateMenu(HMENU hMenu)
{
    HMENU           hMenuChannels;
    int             j;
    CHANNELLIST::iterator it;
    hMenuChannels = GetChannelsSubmenu();
    if(hMenuChannels == NULL) return;

    if (InitialNbMenuItems == -1)
    {
        InitialNbMenuItems = GetMenuItemCount(hMenuChannels);
    }

    j = GetMenuItemCount(hMenuChannels);
    while (j > InitialNbMenuItems)
    {
        --j;
        RemoveMenu(hMenuChannels, j, MF_BYPOSITION);
    }
    
    j = 0;
    for (it = MyChannels.begin(); it != MyChannels.end() && (j < MAXPROGS); ++it)
    {
        if (((*it)->GetFrequency() != 0) && (*it)->IsActive() )
        {
            // Cut every 28 channels which is ok even when in 640x480
            // For the first column, take into account the first items (InitialNbMenuItems)
            // but reduce by 1 because of the two line separators
            if ((j+InitialNbMenuItems-1) % 28)
            {
                AppendMenu(hMenuChannels, MF_STRING | MF_ENABLED, IDM_CHANNEL_SELECT + j, (*it)->GetName());
            }
            else
            {
                AppendMenu(hMenuChannels, MF_STRING | MF_ENABLED | MF_MENUBARBREAK, IDM_CHANNEL_SELECT + j, (*it)->GetName());
            }
            j++;
        }
    }
}

void Channels_SetMenu(HMENU hMenu)
{
    int NDisabledChannels = 0;
    CHANNELLIST::iterator it;
    HMENU hMenuChannels(GetChannelsSubmenu());
    int i, j;
    if(hMenuChannels == NULL) return;

    if (InitialNbMenuItems == -1)
    {
        InitialNbMenuItems = GetMenuItemCount(hMenuChannels);
    }

    BOOL bHasTuner = Providers_GetCurrentSource() ? Providers_GetCurrentSource()->HasTuner() : FALSE;
    BOOL bInTunerMode = Providers_GetCurrentSource() ? Providers_GetCurrentSource()->IsInTunerMode() : FALSE;

    EnableMenuItem(hMenuChannels, IDM_CHANNELPLUS, bHasTuner && bInTunerMode?MF_ENABLED:MF_GRAYED);
    EnableMenuItem(hMenuChannels, IDM_CHANNELMINUS, bHasTuner && bInTunerMode?MF_ENABLED:MF_GRAYED);
    EnableMenuItem(hMenuChannels, IDM_CHANNEL_PREVIOUS, bHasTuner && bInTunerMode?MF_ENABLED:MF_GRAYED);
    EnableMenuItem(hMenuChannels, IDM_CHANNEL_LIST, bHasTuner?MF_ENABLED:MF_GRAYED);

    i = j = 0;
    for (it = MyChannels.begin(); it != MyChannels.end() && (j < MAXPROGS); ++it)
    {
        if (((*it)->GetFrequency() != 0) && (*it)->IsActive() )
        {
            EnableMenuItem(hMenuChannels, IDM_CHANNEL_SELECT + j, bHasTuner ? MF_ENABLED : MF_GRAYED);
            CheckMenuItem(hMenuChannels, IDM_CHANNEL_SELECT + j, (CurrentProgram == i) ? MF_CHECKED : MF_UNCHECKED);
            j++;
        }
        i++;
    }

    // Hide the menu "Channels" from the menu bar
    // when the source has no tuner or when the tuner
    // is not the selected input
    HMENU hSubMenu = GetSubMenu(hMenu, 2);
    if (!bHasTuner || !bInTunerMode)
    {
        if (hSubMenu == hMenuChannels)
        {
            RemoveMenu(hMenu, 2, MF_BYPOSITION);
        }
    }
    else
    {
        if (hSubMenu != hMenuChannels)
        {
            InsertMenu(hMenu, 2, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT)hMenuChannels, "&Channels");
        }
    }
}

BOOL ProcessProgramSelection(HWND hWnd, WORD wMenuID)
{
    int i, j;
    CHANNELLIST::iterator it;

    if ( (wMenuID >= IDM_CHANNEL_SELECT) && (wMenuID < (IDM_CHANNEL_SELECT+MAXPROGS)) )
    {
        if (Providers_GetCurrentSource()->IsInTunerMode())
        {
            i = j = 0;
            for (it = MyChannels.begin(); it != MyChannels.end() && (j < MAXPROGS); ++it)
            {
                if (((*it)->GetFrequency() != 0) && (*it)->IsActive() )
                {
                    if ((wMenuID - IDM_CHANNEL_SELECT) == j)
                    {
                        Channel_Change(i);
                        break;
                    }
                    j++;
                }
                i++;
            }
        }
        else
        {
            SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT1, 0);
            SendMessage(hWnd, WM_COMMAND, wMenuID, 0);
        }
        return TRUE;
    }
    return FALSE;
}

BOOL CurrentProgram_OnChange(long NewValue)
{
    CurrentProgram = NewValue;
    Channel_Change(CurrentProgram);
    return FALSE;
}


////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING ChannelsSettings[CHANNELS_SETTING_LASTONE] =
{
    {
        "CountryCode", SLIDER, 0, (long*)&CountryCode,
        1, 0, 100, 1, 1,
        NULL,
        "Show", "CountryCode", NULL,
    },
    {
        "Current Program", SLIDER, 0, (long*)&CurrentProgram,
        0, 0, MAXPROGS, 1, 1,
        NULL,
        "Show", "LastProgram", CurrentProgram_OnChange,
    },
    {
        "Custom Channel Order", ONOFF, 0, (long*)&bCustomChannelOrder,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Show", "CustomChannelOrder", NULL,
    },    
};

SETTING* Channels_GetSetting(CHANNELS_SETTING Setting)
{
    if(Setting > -1 && Setting < CHANNELS_SETTING_LASTONE)
    {
        return &(ChannelsSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void Channels_ReadSettingsFromIni()
{
    int i;
    for(i = 0; i < CHANNELS_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(ChannelsSettings[i]));
    }
}

void Channels_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < CHANNELS_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(ChannelsSettings[i]), bOptimizeFileAccess);
    }
}

SETTING AntiPlopSettings[ANTIPLOP_SETTING_LASTONE] =
{
    {
        "Pre switch mute delay", SLIDER, 0, (long*)&PreSwitchMuteDelay,
        100, 0, 1000, 1, 1,
        NULL,
        "Audio", "PreSwitchMuteDelay", NULL,
    },
    {
        "Post switch mute delay", SLIDER, 0, (long*)&PostSwitchMuteDelay,
        150, 0, 1000, 1, 1,
        NULL,
        "Audio", "PostSwitchMuteDelay", NULL,
    },    
};

SETTING* AntiPlop_GetSetting(ANTIPLOP_SETTING Setting)
{
    if(Setting > -1 && Setting < ANTIPLOP_SETTING_LASTONE)
    {
        return &(AntiPlopSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void AntiPlop_ReadSettingsFromIni()
{
    int i;
    for(i = 0; i < ANTIPLOP_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(AntiPlopSettings[i]));
    }
}

void AntiPlop_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < ANTIPLOP_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(AntiPlopSettings[i]), bOptimizeFileAccess);
    }
}

CTreeSettingsGeneric* AntiPlop_GetTreeSettingsPage()
{
    return new CTreeSettingsGeneric("Anti Plop Settings", AntiPlopSettings, ANTIPLOP_SETTING_LASTONE);
}


