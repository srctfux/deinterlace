/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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

/**
 * @file dvbtsource.h dvbtsource Header file
 */

#ifndef __DVBTSOURCE_H___
#define __DVBTSOURCE_H___

#include "Source.h"
#include "DVBTCard.h"
#include "HardwareMemory.h"
#include "Setting.h"

class CDVBTSource : public CSource
{
public:
    ~CDVBTSource();
    CDVBTSource(CDVBTCard* pDVBTCard, LPCSTR IniSection);
    void CreateSettings(LPCSTR IniSection);
    void Start();
    void Stop();
    void Reset();
    void GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming);
    BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);
    LPCSTR GetStatus();
    ISetting* GetVolume();
    ISetting* GetBalance();
    void Mute();
    void UnMute();
    ISetting* GetBrightness();
    ISetting* GetContrast();
    ISetting* GetHue();
    ISetting* GetSaturation();
    ISetting* GetSaturationU();
    ISetting* GetSaturationV();
    ISetting* GetAnalogueBlanking() {return NULL;};
    ISetting* GetTopOverscan();
    ISetting* GetBottomOverscan();
    ISetting* GetLeftOverscan();
    ISetting* GetRightOverscan();
    ISetting* GetHDelay() {return NULL;};
    ISetting* GetVDelay() {return NULL;};
    eVideoFormat GetFormat();
    BOOL IsInTunerMode() {return TRUE;}
    int GetInitialWidth() {return GetWidth();};
    int GetInitialHeight() {return GetHeight() / 2;};
    int GetWidth() {return 720;};
    int GetHeight() {return 576;};
    void SetWidth() {return;};

    void UpdateMenu() {return;};
    void SetMenu(HMENU hMenu);
    void HandleTimerMessages(int TimerId);
    BOOL SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat);
    BOOL IsVideoPresent();
    void DecodeVBI(TDeinterlaceInfo* pInfo) {;};
    LPCSTR GetMenuLabel() {return NULL;};
    BOOL IsAccessAllowed() {return TRUE;};
    void SetAspectRatioData();
    BOOL HasSquarePixels() {return FALSE;};
    void ChangeSettingsBasedOnHW(int ProcessorSpeed, int TradeOff) {;};

    void Pause() {return;};
    void UnPause() {return;};

    BOOL HasMediaControl() {return FALSE;};
    BOOL IsAudioMixerAccessAllowed() {return TRUE;};

private:
    std::string  m_Section;
    CDVBTCard* m_pDVBTCard;
};


#endif