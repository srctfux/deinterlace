/////////////////////////////////////////////////////////////////////////////
// $Id: StillSource.h,v 1.73 2005-03-04 20:23:34 laurentg Exp $
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
 * @file stillsource.h stillsource Header file
 */
 
#ifndef __STILLSOURCE_H___
#define __STILLSOURCE_H___

#include "Source.h"
#include "Bt848_Defines.h"
#include "TreeSettingsGeneric.h"

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
    virtual void SaveSnapshot(LPCSTR FilePath, int Height, int Width, BYTE* pOverlay, LONG OverlayPitch, char* Context) = 0;
protected:
    CStillSource* m_pParent;
};

/** An item in a CStillSource playlist
*/
class CPlayListItem
{
public:
    CPlayListItem(LPCSTR FileName);
    CPlayListItem(BYTE* FrameBuffer, int FrameHeight, int FrameWidth, int LinePitch, BOOL SquarePixels, char* Context);
    LPCSTR GetFileName();
    void SetFileName(LPCSTR FileName);
    BOOL GetMemoryInfo(BYTE** pFrameBuffer, int* pFrameHeight, int* pFrameWidth, int* pLinePitch, BOOL* pSquarePixels, const char** pContext);
    BOOL IsInMemory();
    time_t GetTimeStamp();
    BOOL IsSupported();
    void SetSupported(BOOL Supported);
    void FreeBuffer();
private:
    std::string m_FileName;
    BOOL m_Supported;
    BYTE* m_FrameBuffer;
	int m_FrameHeight;
	int m_FrameWidth;
	int m_LinePitch;
    BOOL m_SquarePixels;
	time_t m_TimeStamp;
	std::string m_Context;
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
    ISetting* GetAnalogueBlanking() {return NULL;};
    ISetting* GetTopOverscan() {return NULL;};
    ISetting* GetBottomOverscan() {return NULL;};
    ISetting* GetLeftOverscan() {return NULL;};
    ISetting* GetRightOverscan() {return NULL;};
	ISetting* GetHDelay() {return NULL;};
	ISetting* GetVDelay() {return NULL;};

    CTreeSettingsPage* GetTreeSettingsPage() {return NULL;};

    eVideoFormat GetFormat();
    BOOL IsInTunerMode() {return FALSE;};
    int GetInitialWidth();
    int GetInitialHeight();
    int GetWidth();
    int GetHeight();
	void SetWidth(int w) {return;};

    void UpdateMenu();
    void SetMenu(HMENU hMenu);
    void HandleTimerMessages(int TimerId);
    void SetFormat(eVideoFormat NewFormat) {};
    BOOL SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat) {return FALSE;};
    BOOL IsVideoPresent();
    void DecodeVBI(TDeinterlaceInfo* pInfo) {;};
    LPCSTR GetMenuLabel();
    BOOL ReadNextFrameInFile();
    BOOL LoadPlayList(LPCSTR FileName);
    void SaveSnapshotInFile(int FrameHeight, int FrameWidth, BYTE* pFrameBuffer, LONG LinePitch);
	void SaveSnapshotInMemory(int FrameHeight, int FrameWidth, BYTE* pAllocBuffer, LONG LinePitch);
    void SaveInFile(int pos);
    BOOL OpenMediaFile(LPCSTR FileName, BOOL NewPlayList);
    BOOL IsAccessAllowed();
    void SetAspectRatioData();
    BOOL HasSquarePixels() {return m_SquarePixels;};
    void ChangeSettingsBasedOnHW(int ProcessorSpeed, int TradeOff) {;};
    LPCSTR IDString() { return m_IDString.c_str(); }
    void SetNavigOnly(BOOL NavigOnly);
    BOOL IsNavigOnly();
    int  NumInputs(eSourceInputType InputType) { return 0; };
    BOOL SetInput(eSourceInputType InputType, int Nr) { return FALSE; };
    int GetInput(eSourceInputType InputType) { return -1; };
    const char* GetInputName(eSourceInputType InputType, int Nr) { return NULL; };
    BOOL InputHasTuner(eSourceInputType InputType, int Nr) { return FALSE; };

    ITuner* GetTuner() {return NULL;}

    void Pause() {return;};
    void UnPause() {return;};

	BOOL HasMediaControl() {return FALSE;};

    BOOL IsAudioMixerAccessAllowed() {return FALSE;};

	int	 GetPlaylistPosition();
	int CountMemoryUsage();

    BOOL IsOneItemInMemory();

    friend class CTiffHelper;
    friend class CJpegHelper;
    friend class CPatternHelper;

private:
	void FreeOriginalFrameBuffer();
    void ClearPlayList();
    BOOL ShowNextInPlayList();
    BOOL ShowPreviousInPlayList();
    BOOL OpenPictureFile(LPCSTR FileName);
	BOOL OpenPictureMemory(BYTE* FrameBuffer, int FrameHeight, int FrameWidth, int LinePitch, BOOL SquarePixels, const char* Context);
    BOOL SavePlayList(LPCSTR FileName);
    BOOL IsItemInList(LPCSTR FileName);
	BOOL FindFileName(time_t TimeStamp, char* FileName);

protected:
    int         m_InitialWidth;
    int         m_InitialHeight;
    int         m_Width;
    int         m_Height;
    BYTE*       m_StillFrameBuffer;
    TPicture    m_StillFrame;
    BYTE*       m_OriginalFrameBuffer;
    TPicture    m_OriginalFrame;
    vector<CPlayListItem*> m_PlayList;
    int         m_Position;
    BOOL        m_IsPictureRead;
    BOOL        m_SquarePixels;
    BOOL        m_NavigOnly;
    int         m_LinePitch;
    std::string m_IDString;

protected:
    void ChangeDefaultsForVideoFormat(BOOL bDontSetValue) {};
    void ChangeDefaultsForVideoInput(BOOL bDontSetValue) {};
    void ChangeDefaultsForAudioInput(BOOL bDontSetValue) {};

private:
    DWORD       m_LastTickCount;
    DWORD		m_FrameDuration;
    BOOL        m_SlideShowActive;
    
    eStillNewFileRequest    m_NewFileRequested;
    int                     m_NewFileReqPos;

    MEMCPY_FUNC*    m_pMemcpy;

    std::string m_Section;
};


//    	BYTE** y = (BYTE**) (x+16);
//    	y = (BYTE**) ((unsigned int) y & 0xfffffff0);
#define	START_ALIGNED16(buf)	((buf) + 16 - ((DWORD)(buf) % 16))

BOOL ResizeFrame(BYTE* OldBuf, int OldPitch, int OldWidth, int OldHeight, BYTE* NewBuf, int NewPitch, int NewWidth, int NewHeight);

void BuildDScalerContext(char* buf);

SETTING* Still_GetSetting(STILL_SETTING Setting);
void Still_ReadSettingsFromIni();
void Still_WriteSettingsToIni(BOOL bOptimizeFileAccess);
CTreeSettingsGeneric* Still_GetTreeSettingsPage();
void Still_FreeSettings();


#endif