/////////////////////////////////////////////////////////////////////////////
// $Id: StillSource.h,v 1.30 2002-05-02 20:16:27 laurentg Exp $
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

#ifndef __STILLSOURCE_H___
#define __STILLSOURCE_H___

#include "Source.h"
#include "Bt848_Defines.h"

#define MAX_PLAYLIST_SIZE   50

enum eStillFormat
{
    STILL_TIFF_RGB = 0,
    STILL_TIFF_YCbCr,
    STILL_JPEG,
    STILL_FORMAT_LASTONE,
};

enum eStillNewFileRequest
{
    STILL_REQ_NONE = 0,
    STILL_REQ_THIS_ONE,
    STILL_REQ_NEXT,
    STILL_REQ_NEXT_CIRC,
    STILL_REQ_PREVIOUS,
    STILL_REQ_PREVIOUS_CIRC,
};

class CStillSource;
class CTiffHelper;
class CJpegHelper;
class CPatternHelper;

/** Base class for file type specific code that CStillSource uses to read
    Still files
*/
class CStillSourceHelper
{
public:
    CStillSourceHelper(CStillSource* pParent);
    virtual BOOL OpenMediaFile(LPCSTR FileName) = 0;
    virtual void SaveSnapshot(LPCSTR FilePath, int Height, int Width, BYTE* pOverlay, LONG OverlayPitch) = 0;
protected:
    CStillSource* m_pParent;
};

/** An item in a CStillSource playlist
*/
class CPlayListItem
{
public:
    CPlayListItem(LPCSTR FileName, int SecondsToDisplay);
    LPCSTR GetFileName();
    BOOL IsSupported();
    int GetSecondsToDisplay();
    void SetSupported(BOOL Supported);
private:
    std::string m_FileName;
    int m_SecondsToDisplay;
    BOOL m_Supported;
};

/** Source class that can read files and playlists and display
    then as faked video
*/
class CStillSource : public CSource
{
public:
    CStillSource(LPCSTR IniSection);
    ~CStillSource();
    void CreateSettings(LPCSTR IniSection);
    void Start();
    void Stop();
    void Reset();
    void GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming);
    BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);
    LPCSTR GetStatus();
    ISetting* GetVolume() {return NULL;};
    ISetting* GetBalance() {return NULL;};
    void Mute() {;};
    void UnMute() {;};
    ISetting* GetBrightness() {return NULL;};
    ISetting* GetContrast() {return NULL;};
    ISetting* GetHue() {return NULL;};
    ISetting* GetSaturation() {return NULL;};
    ISetting* GetSaturationU() {return NULL;};
    ISetting* GetSaturationV() {return NULL;};
    ISetting* GetOverscan() {return NULL;};
	eTunerId GetTunerId();
    eVideoFormat GetFormat();
    BOOL IsInTunerMode() {return FALSE;};
    int GetWidth();
    int GetHeight();
    BOOL HasTuner() {return FALSE;};
    void UpdateMenu();
    void SetMenu(HMENU hMenu);
    void HandleTimerMessages(int TimerId);
    BOOL SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat) {return FALSE;};
    BOOL IsVideoPresent();
    void DecodeVBI(TDeinterlaceInfo* pInfo) {;};
    LPCSTR GetMenuLabel();
    BOOL ReadNextFrameInFile();
    BOOL LoadPlayList(LPCSTR FileName);
    void SaveSnapshot(LPCSTR FilePath, int FrameHeight, int FrameWidth, BYTE* pOverlay, LONG OverlayPitch);
    BOOL OpenMediaFile(LPCSTR FileName, BOOL NewPlayList);
    BOOL IsAccessAllowed();
    void SetOverscan();
    BOOL HasSquarePixels() {return m_SquarePixels;};
    void SetNavigOnly(BOOL NavigOnly);
    BOOL IsNavigOnly();

    friend class CTiffHelper;
    friend class CJpegHelper;
    friend class CPatternHelper;

private:
    void ClearPlayList();
    BOOL ShowNextInPlayList();
    BOOL ShowPreviousInPlayList();
    BOOL OpenPictureFile(LPCSTR FileName);
    BOOL SavePlayList(LPCSTR FileName);
    BOOL ResizeOriginalFrame(int NewWidth, int NewHeight);

protected:
    int         m_Width;
    int         m_Height;
    TPicture    m_StillFrame;
    TPicture    m_OriginalFrame;
    vector<CPlayListItem*> m_PlayList;
    int         m_Position;
    BOOL        m_IsPictureRead;
    BOOL        m_SquarePixels;
    BOOL        m_NavigOnly;

private:
    DWORD       m_LastTickCount;
    double      m_FrameDuration;
    BOOL        m_SlideShowActive;
    
    eStillNewFileRequest    m_NewFileRequested;
    int                     m_NewFileReqPos;

    MEMCPY_FUNC*    m_pMemcpy;

    std::string m_Section;
};


SETTING* Still_GetSetting(STILL_SETTING Setting);
void Still_ReadSettingsFromIni();
void Still_WriteSettingsToIni(BOOL bOptimizeFileAccess);
void Still_ShowUI();

extern char SavingPath[];

#endif