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
 * @file StillSource.cpp CStillSource Implementation
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "StillSource.h"
#include "StillProvider.h"
#include "DScaler.h"
#include "FieldTiming.h"
#include "DebugLog.h"
#include "Providers.h"
#include "TiffHelper.h"
#include "JpegHelper.h"
#include "Pattern.h"
#include "OutThreads.h"
#include "AspectRatio.h"
#include "ioutput.h"
#include "Filter.h"
#include "Dialogs.h"
#include "OSD.h"
#include "MultiFrames.h"


#define TIMER_SLIDESHOW 50

int __stdcall SimpleResize_InitTables(unsigned int* hControl, unsigned int* vOffsets, 
        unsigned int* vWeights, int OldWidth, int OldHeight, int NewWidth, int NewHeight);

static eStillFormat FormatSaving = STILL_TIFF_RGB;
static int SlideShowDelay = 5;
static int JpegQuality = 95;
static int PatternHeight = 576;
static int PatternWidth = 720;
static int DelayBetweenStills = 60;
static BOOL SaveInSameFile = FALSE;
static BOOL StillsInMemory = FALSE;
static BOOL KeepOriginalRatio = FALSE;
static int NbConsecutiveStills = 20;
static BOOL OSDForStills = TRUE;
static int PreviewNbCols = 4;
static int PreviewNbRows = 4;
static int MaxMemForStills = 64;

static const char *StillFormatNames[STILL_FORMAT_LASTONE] = 
{
    "TIFF RGB",
    "TIFF YCbCr",
    "JPEG",
};

static char ExePath[MAX_PATH] = {0};
static char* SavingPath = NULL;

extern long NumFilters;
extern FILTER_METHOD* Filters[];


void BuildDScalerContext(char* buf)
{
    int i;
    CSource* pSource = Providers_GetCurrentSource();

    sprintf(buf, "DScaler still\r\nTaken with %s", GetProductNameAndVersion());
    if (pSource != NULL)
    {
        strcat(buf, "\r\nInput was ");
        strcat(buf, pSource->GetStatus());
    }
    strcat(buf, "\r\nDeinterlace mode was ");
    strcat(buf, GetDeinterlaceModeName());
    for (i = 0 ; i < NumFilters ; i++)
    {
        if (Filters[i]->bActive)
        {
            strcat(buf, "\r\n");
            strcat(buf, Filters[i]->szName);
            strcat(buf, " was on");
        }
    }
}


CStillSourceHelper::CStillSourceHelper(CStillSource* pParent)
{
    m_pParent = pParent;
}


CPlayListItem::CPlayListItem(LPCSTR FileName) :
    m_FileName(FileName),
    m_FrameBuffer(NULL),
    m_FrameHeight(0),
    m_FrameWidth(0),
    m_LinePitch(0),
    m_SquarePixels(FALSE),
    m_Context(""),
    m_Supported(TRUE)
{
    m_TimeStamp = time(0);
}

CPlayListItem::CPlayListItem(BYTE* FrameBuffer, int FrameHeight, int FrameWidth, int LinePitch, BOOL SquarePixels, char* Context) :
    m_FileName("Still only in memory"),
    m_FrameBuffer(FrameBuffer),
    m_FrameHeight(FrameHeight),
    m_FrameWidth(FrameWidth),
    m_LinePitch(LinePitch),
    m_SquarePixels(SquarePixels),
    m_Context(Context),
    m_Supported(TRUE)
{
    m_TimeStamp = time(0);
}

LPCSTR CPlayListItem::GetFileName()
{
    return m_FileName.c_str();
}

void CPlayListItem::SetFileName(LPCSTR FileName)
{
    m_FileName = FileName;
    m_FrameBuffer = NULL;
}

BOOL CPlayListItem::GetMemoryInfo(BYTE** pFrameBuffer, int* pFrameHeight, int* pFrameWidth, int* pLinePitch, BOOL* pSquarePixels, const char** pContext)
{
    if (m_FrameBuffer != NULL)
    {
        *pFrameBuffer = m_FrameBuffer;
        *pFrameHeight = m_FrameHeight;
        *pFrameWidth = m_FrameWidth;
        *pLinePitch = m_LinePitch;
        *pSquarePixels = m_SquarePixels;
        *pContext = m_Context.c_str();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL CPlayListItem::IsInMemory()
{
    return (m_FrameBuffer == NULL) ? FALSE : TRUE;
}

time_t CPlayListItem::GetTimeStamp()
{
    return m_TimeStamp;
}

BOOL CPlayListItem::IsSupported()
{
    return m_Supported;
}

void CPlayListItem::SetSupported(BOOL Supported)
{
    m_Supported = Supported;
}

void CPlayListItem::FreeBuffer()
{
    if (m_FrameBuffer != NULL)
    {
        LOG(2, "FreeBuffer - start buf %d", m_FrameBuffer);
        free(m_FrameBuffer);
        m_FrameBuffer = NULL;
    }
}

CStillSource::CStillSource(LPCSTR IniSection) :
    CSource(0, IDC_STILL),
    m_Section(IniSection)
{
    m_IDString = "Still_" + std::string(IniSection);
    CreateSettings(IniSection);
    m_InitialWidth = 0;
    m_InitialHeight = 0;
    m_Width = 0;
    m_Height = 0;
    m_StillFrameBuffer = NULL;
    m_StillFrame.pData = NULL;
    m_StillFrame.Flags = PICTURE_PROGRESSIVE;
    m_StillFrame.IsFirstInSeries = FALSE;
    m_OriginalFrameBuffer = NULL;
    m_OriginalFrame.pData = NULL;
    m_OriginalFrame.Flags = PICTURE_PROGRESSIVE;
    m_OriginalFrame.IsFirstInSeries = FALSE;
    m_FieldFrequency = 50.0;    // 50 Hz
    m_FrameDuration = 20;        // 20 ms
    m_IsPictureRead = FALSE;
    m_Position = -1;
    m_SlideShowActive = FALSE;
    m_pMemcpy = NULL;
    m_SquarePixels = TRUE;
    m_NavigOnly = FALSE;
    m_LinePitch = 0;    
}

CStillSource::~CStillSource()
{
    if (m_StillFrameBuffer != NULL)
    {
        free(m_StillFrameBuffer);
    }
    FreeOriginalFrameBuffer();
    ClearPlayList();
    KillTimer(GetMainWnd(), TIMER_SLIDESHOW);
}

BOOL CStillSource::LoadPlayList(LPCSTR FileName)
{
    char BufferLine[512];
    char *Buffer;
    struct stat st;
    BOOL FirstItemAdded = FALSE;
    FILE* Playlist = fopen(FileName, "r");
    if(Playlist != NULL)
    {
        while(!feof(Playlist))
        {
            if(fgets(BufferLine, 512, Playlist))
            {
                BufferLine[511] = '\0';
                Buffer = BufferLine;
                while(strlen(Buffer) > 0 && *Buffer <= ' ')
                {
                    Buffer++;
                }
                if(strlen(Buffer) == 0 || *Buffer == '#' || *Buffer == ';')
                {
                    continue;
                }
                // take care of stuff that is at end of the line
                while(strlen(Buffer) > 0 && Buffer[strlen(Buffer) - 1] <= ' ')
                {
                    Buffer[strlen(Buffer) - 1] = '\0';
                }
                if (strlen(Buffer) == 0)
                {
                    continue;
                }
                if (!strncmp(&Buffer[1], ":\\", 2) || (Buffer[0] == '\\'))
                {
                    if (!stat(Buffer, &st))
                    {
                        CPlayListItem* Item = new CPlayListItem(Buffer);
                        m_PlayList.push_back(Item);
                        if (!FirstItemAdded)
                        {
                            m_Position = m_PlayList.size() - 1;
                            FirstItemAdded = TRUE;
                        }
                    }
                }
                else
                {
                    char FilePath[MAX_PATH];
                    const char* pStr = strrchr(FileName, '\\');
                    if (pStr == NULL)
                    {
                        strcpy(FilePath, FileName);
                    }
                    else
                    {
                        strncpy(FilePath, FileName, pStr - FileName + 1);
                        FilePath[pStr - FileName + 1] = '\0';
                        strcat(FilePath, Buffer);
                    }
                    if (!stat(FilePath, &st))
                    {
                        CPlayListItem* Item = new CPlayListItem(FilePath);
                        m_PlayList.push_back(Item);
                        if (!FirstItemAdded)
                        {
                            m_Position = m_PlayList.size() - 1;
                            FirstItemAdded = TRUE;
                        }
                    }
                }
            }
        }
        fclose(Playlist);
    }

    return FirstItemAdded;
}

BOOL CStillSource::OpenPictureFile(LPCSTR FileName)
{
    BOOL FileRead = FALSE;
    int h = m_Height;
    int w = m_Width;

    if(strlen(FileName) > 4 && _stricmp(FileName + strlen(FileName) - 4, ".tif") == 0 ||
        strlen(FileName) > 5 && _stricmp(FileName + strlen(FileName) - 5, ".tiff") == 0)
    {
        CTiffHelper TiffHelper(this, TIFF_CLASS_R);
        FileRead = TiffHelper.OpenMediaFile(FileName);
    }
    else if(strlen(FileName) > 4 && _stricmp(FileName + strlen(FileName) - 4, ".pat") == 0)
    {
        CPatternHelper PatternHelper(this);
        FileRead = PatternHelper.OpenMediaFile(FileName);
    }
    else if(strlen(FileName) > 4 && _stricmp(FileName + strlen(FileName) - 4, ".jpg") == 0 ||
            strlen(FileName) > 5 && _stricmp(FileName + strlen(FileName) - 5, ".jpeg") == 0)
    {
        CJpegHelper JpegHelper(this);
        FileRead = JpegHelper.OpenMediaFile(FileName);
    }

    if (FileRead && ((m_Width > DSCALER_MAX_WIDTH) || (m_Height > DSCALER_MAX_HEIGHT)) )
    {
        int NewWidth;
        int NewHeight;
        int NewPitch;
        BYTE* NewBuf;
        BYTE* NewStart;

        NewHeight = m_Height;
        NewWidth = m_Width;

        if (m_Width > DSCALER_MAX_WIDTH)
        {
            NewWidth = DSCALER_MAX_WIDTH;
            NewHeight = m_Height * DSCALER_MAX_WIDTH / m_Width;
        }

        if (NewHeight > DSCALER_MAX_HEIGHT)
        {
            NewWidth = NewWidth * DSCALER_MAX_HEIGHT / NewHeight;
            NewHeight = DSCALER_MAX_HEIGHT;
        }
        NewWidth = NewWidth & 0xfffffffe;       // even wid 

        // Allocate memory for the new YUYV buffer
        NewPitch = (NewWidth * 2 * sizeof(BYTE) + 15) & 0xfffffff0;
        NewBuf = (BYTE*)malloc(NewPitch * NewHeight + 16);
        if (NewBuf != NULL)
        {
            NewStart = START_ALIGNED16(NewBuf);
            if (!ResizeFrame(m_OriginalFrame.pData, m_LinePitch, m_Width, m_Height, NewStart, NewPitch, NewWidth, NewHeight))
            {
                free(NewBuf);
                NewBuf = NULL;
            }
            else
            {
                // Replace the old YUYV buffer by the new one
                free(m_OriginalFrameBuffer);
                m_OriginalFrameBuffer = NewBuf;
                m_OriginalFrame.pData = NewStart;
                m_LinePitch = NewPitch;
            }
        }
        m_Width = NewWidth;
        m_Height = NewHeight;
    }

    if (FileRead)
    {
        m_IsPictureRead = TRUE;

        //check if size has changed
        if(m_Height != h || m_Width != w)
        {
            NotifySizeChange();
        }

        NotifySquarePixelsCheck();
    }

    return FileRead;
}


BOOL CStillSource::OpenPictureMemory(BYTE* FrameBuffer, int FrameHeight, int FrameWidth, int LinePitch, BOOL SquarePixels, const char* Context)
{
    int h = m_Height;
    int w = m_Width;

    LOG(2, "OpenPictureMemory - start buf %d", FrameBuffer);
    FreeOriginalFrameBuffer();
    m_OriginalFrameBuffer = FrameBuffer;
    m_OriginalFrame.pData = START_ALIGNED16(m_OriginalFrameBuffer);
    m_LinePitch = LinePitch;
    m_InitialHeight = FrameHeight;
    m_InitialWidth = FrameWidth;
    m_Height = FrameHeight;
    m_Width = FrameWidth;
    m_SquarePixels = SquarePixels;
    m_Comments = Context;

    m_IsPictureRead = TRUE;

    //check if size has changed
    if(m_Height != h || m_Width != w)
    {
        NotifySizeChange();
    }

    NotifySquarePixelsCheck();

    return TRUE;
}


BOOL CStillSource::OpenMediaFile(LPCSTR FileName, BOOL NewPlayList)
{
    int i;

    if (NewPlayList)
    {
        ClearPlayList();
        m_Position = -1;
    }

    i = m_Position;

    // test for the correct extension and work out the 
    // correct helper for the file type
    if(strlen(FileName) > 4 && _stricmp(FileName + strlen(FileName) - 4, ".d3u") == 0)
    {
        if (LoadPlayList(FileName))
        {
            if (ShowNextInPlayList())
            {
                return TRUE;
            }
            else
            {
                m_Position = i;
                // All added files in the playlist are not readable
                // => suppress them from the playlist
                while ((m_Position + 1) < m_PlayList.size())
                {
                    m_PlayList.erase(m_PlayList.begin() + m_Position + 1);
                }
            }
        }
    }
    else if(OpenPictureFile(FileName))
    {
        CPlayListItem* Item = new CPlayListItem(FileName);
        m_PlayList.push_back(Item);
        m_Position = m_PlayList.size() - 1;
        return TRUE;
    }

    return FALSE;
}


BOOL CStillSource::ShowNextInPlayList()
{
    BYTE* FrameBuffer;
    int FrameHeight;
    int FrameWidth;
    int LinePitch;
    BOOL SquarePixels;
    const char* Context;
    int Pos = m_Position;

    while(Pos < m_PlayList.size())
    {
        if(!m_PlayList[Pos]->IsInMemory() && OpenPictureFile(m_PlayList[Pos]->GetFileName()))
        {
            m_Position = Pos;
            m_PlayList[Pos]->SetSupported(TRUE);
            return TRUE;
        }
        else if(m_PlayList[Pos]->GetMemoryInfo(&FrameBuffer, &FrameHeight, &FrameWidth, &LinePitch, &SquarePixels, &Context)
             && OpenPictureMemory(FrameBuffer, FrameHeight, FrameWidth, LinePitch, SquarePixels, Context))
        {
            m_Position = Pos;
            m_PlayList[Pos]->SetSupported(TRUE);
            return TRUE;
        }
        else
        {
            m_PlayList[Pos]->SetSupported(FALSE);
        }
        ++Pos;
    }
    return FALSE;
}

BOOL CStillSource::ShowPreviousInPlayList()
{
    BYTE* FrameBuffer;
    int FrameHeight;
    int FrameWidth;
    int LinePitch;
    BOOL SquarePixels;
    const char* Context;
    int Pos = m_Position;

    while(Pos >= 0)
    {
        if(!m_PlayList[Pos]->IsInMemory() && OpenPictureFile(m_PlayList[Pos]->GetFileName()))
        {
            m_Position = Pos;
            m_PlayList[Pos]->SetSupported(TRUE);
            return TRUE;
        }
        else if(m_PlayList[Pos]->GetMemoryInfo(&FrameBuffer, &FrameHeight, &FrameWidth, &LinePitch, &SquarePixels, &Context)
             && OpenPictureMemory(FrameBuffer, FrameHeight, FrameWidth, LinePitch, SquarePixels, Context))
        {
            m_Position = Pos;
            m_PlayList[Pos]->SetSupported(TRUE);
            return TRUE;
        }
        else
        {
            m_PlayList[Pos]->SetSupported(FALSE);
        }
        --Pos;
    }
    return FALSE;
}

int CStillSource::GetPlaylistPosition()
{
    return m_Position;
}

BOOL CStillSource::FindFileName(time_t TimeStamp, char* FileName)
{
    int n = 0;
    struct tm *ctm=localtime(&TimeStamp);
    char extension[4];
    struct stat st;

    switch ((eStillFormat)Setting_GetValue(Still_GetSetting(FORMATSAVING)))
    {
    case STILL_TIFF_RGB:
    case STILL_TIFF_YCbCr:
        strcpy(extension, "tif");
        break;
    case STILL_JPEG:
        strcpy(extension, "jpg");
        break;
    default:
        ErrorBox("Format of saving not supported.\nPlease change format in advanced settings");
        return FALSE;
        break;
    }

    while (n < 100)
    {
        sprintf(FileName,"%s\\TV%04d%02d%02d%02d%02d%02d%02d.%s",
                SavingPath,
                ctm->tm_year+1900,ctm->tm_mon+1,ctm->tm_mday,ctm->tm_hour,ctm->tm_min,ctm->tm_sec,n++, 
                // name ~ date & time & per-second-counter (for if anyone succeeds in multiple captures per second)
                // TVYYYYMMDDHHMMSSCC.ext ; eg .\TV2002123123595900.tif
                extension);

        if (stat(FileName, &st))
        {
            break;
        }
    }
    if(n == 100) // never reached in 1 second, so could be scrapped
    {
        ErrorBox("Could not create a file. You may have too many captures already.");
        return FALSE;
    }

    return TRUE;
}

void CStillSource::SaveSnapshotInFile(int FrameHeight, int FrameWidth, BYTE* pFrameBuffer, LONG LinePitch)
{
    char FilePath[MAX_PATH];
    char Context[1024];
    BYTE* pNewFrameBuffer = pFrameBuffer;
    int NewLinePitch = LinePitch;
    int NewFrameHeight = FrameHeight;
    int NewFrameWidth = FrameWidth;
    BYTE* NewBuf = NULL;

    if (Setting_GetValue(Still_GetSetting(SAVEINSAMEFILE)))
    {
        char extension[4];
        switch ((eStillFormat)Setting_GetValue(Still_GetSetting(FORMATSAVING)))
        {
        case STILL_TIFF_RGB:
        case STILL_TIFF_YCbCr:
            strcpy(extension, "tif");
            break;
        case STILL_JPEG:
            strcpy(extension, "jpg");
            break;
        default:
            ErrorBox("Format of saving not supported.\nPlease change format in advanced settings");
            return;
            break;
        }
        sprintf(FilePath,"%s\\TV.%s", SavingPath, extension);
    }
    else
    {
        if (!FindFileName(time(0), FilePath))
        {
            return;
        }
    }

    if (OSDForStills)
    {
        OSD_ShowText(strrchr(FilePath, '\\') + 1, 0);
    }

    m_SquarePixels = AspectSettings.SquarePixels;

    if (!m_SquarePixels && Setting_GetValue(Still_GetSetting(KEEPORIGINALRATIO)))
    {
        double Width;

        // if source is 16/9 anamorphic
        if (AspectSettings.AspectMode == 2)
        {
            Width = NewFrameHeight * 1.7777;
        }
        // else source is 4/3 non anamorphic
        else
        {
            Width = NewFrameHeight * 1.3333;
        }
        NewFrameWidth = (int)floor(Width / 2.0 + 0.5) * 2;

        // Allocate memory for the new YUYV buffer
        NewLinePitch = (NewFrameWidth * 2 * sizeof(BYTE) + 15) & 0xfffffff0;
        NewBuf = (BYTE*)malloc(NewLinePitch * NewFrameHeight + 16);
        if (NewBuf != NULL)
        {
            pNewFrameBuffer = START_ALIGNED16(NewBuf);
        }
        if ((NewBuf == NULL) || !ResizeFrame(pFrameBuffer, LinePitch, FrameWidth, FrameHeight, pNewFrameBuffer, NewLinePitch, NewFrameWidth, NewFrameHeight))
        {
            // If resize fails, we use the non resized still
            pNewFrameBuffer = pFrameBuffer;
            NewLinePitch = LinePitch;
            NewFrameHeight = FrameHeight;
            NewFrameWidth = FrameWidth;
        }
    }

    BuildDScalerContext(Context);

    switch ((eStillFormat)Setting_GetValue(Still_GetSetting(FORMATSAVING)))
    {
    case STILL_TIFF_RGB:
        {
            if (m_SquarePixels)
            {
                strcat(Context, SQUARE_MARK);
            }
            CTiffHelper TiffHelper(this, TIFF_CLASS_R);
            TiffHelper.SaveSnapshot(FilePath, NewFrameHeight, NewFrameWidth, pNewFrameBuffer, NewLinePitch, Context);
            break;
        }
    case STILL_TIFF_YCbCr:
        {
            if (m_SquarePixels)
            {
                strcat(Context, SQUARE_MARK);
            }
            CTiffHelper TiffHelper(this, TIFF_CLASS_Y);
            TiffHelper.SaveSnapshot(FilePath, NewFrameHeight, NewFrameWidth, pNewFrameBuffer, NewLinePitch, Context);
            break;
        }
    case STILL_JPEG:
        {
            CJpegHelper JpegHelper(this);
            JpegHelper.SaveSnapshot(FilePath, NewFrameHeight, NewFrameWidth, pNewFrameBuffer, NewLinePitch, Context);
            break;
        }
    default:
        break;
    }

    if (!IsItemInList(FilePath))
    {
        CPlayListItem* Item = new CPlayListItem(FilePath);
        m_PlayList.push_back(Item);
        if (m_PlayList.size() == 1)
        {
            m_Position = 0;
        }
        UpdateMenu();
    }

    if (NewBuf != NULL)
    {
        free(NewBuf);
    }
}

void CStillSource::SaveSnapshotInMemory(int FrameHeight, int FrameWidth, BYTE* pAllocBuffer, LONG LinePitch)
{
    char Context[1024];

    if (pAllocBuffer != NULL)
    {
        LOG(2, "SaveSnapshotInMemory - start buf %d", pAllocBuffer);
        BuildDScalerContext(Context);
        CPlayListItem* Item = new CPlayListItem(pAllocBuffer, FrameHeight, FrameWidth, LinePitch, AspectSettings.SquarePixels, Context);
        m_PlayList.push_back(Item);
        if (m_PlayList.size() == 1)
        {
            m_Position = 0;
        }
        UpdateMenu();
    }
}

void CStillSource::SaveInFile(int pos)
{
    BYTE* pFrameBuffer;
    BYTE* pStartFrame;
    int FrameHeight;
    int FrameWidth;
    int LinePitch;
    BOOL SquarePixels;
    const char* Context2;
    char Context[1024];
    char FilePath[MAX_PATH];
    BYTE* pNewFrameBuffer;
    int NewLinePitch;
    int NewFrameHeight;
    int NewFrameWidth;
    BYTE* NewBuf;

    if ((pos < 0)
     || (pos >= m_PlayList.size())
     || !m_PlayList[pos]->GetMemoryInfo(&pFrameBuffer, &FrameHeight, &FrameWidth, &LinePitch, &SquarePixels, &Context2))
    {
        return;
    }

    pStartFrame = START_ALIGNED16(pFrameBuffer);

    if (!FindFileName(m_PlayList[pos]->GetTimeStamp(), FilePath))
    {
        return;
    }

    if (OSDForStills)
    {
        OSD_ShowText(strrchr(FilePath, '\\') + 1, 0);
    }

    pNewFrameBuffer = pStartFrame;
    NewLinePitch = LinePitch;
    NewFrameHeight = FrameHeight;
    NewFrameWidth = FrameWidth;
    NewBuf = NULL;

    if (!m_SquarePixels && Setting_GetValue(Still_GetSetting(KEEPORIGINALRATIO)))
    {
        double Width;

        // if source is 16/9 anamorphic
        if (AspectSettings.AspectMode == 2)
        {
            Width = NewFrameHeight * 1.7777;
        }
        // else source is 4/3 non anamorphic
        else
        {
            Width = NewFrameHeight * 1.3333;
        }
        NewFrameWidth = (int)floor(Width / 2.0 + 0.5) * 2;

        // Allocate memory for the new YUYV buffer
        NewLinePitch = (NewFrameWidth * 2 * sizeof(BYTE) + 15) & 0xfffffff0;
        NewBuf = (BYTE*)malloc(NewLinePitch * NewFrameHeight + 16);
        if (NewBuf != NULL)
        {
            pNewFrameBuffer = START_ALIGNED16(NewBuf);
        }
        if ((NewBuf == NULL) || !ResizeFrame(pStartFrame, LinePitch, FrameWidth, FrameHeight, pNewFrameBuffer, NewLinePitch, NewFrameWidth, NewFrameHeight))
        {
            // If resize fails, we use the non resized still
            pNewFrameBuffer = pStartFrame;
            NewLinePitch = LinePitch;
            NewFrameHeight = FrameHeight;
            NewFrameWidth = FrameWidth;
        }
    }

    strcpy(Context, Context2);

    switch ((eStillFormat)Setting_GetValue(Still_GetSetting(FORMATSAVING)))
    {
    case STILL_TIFF_RGB:
        {
            if (m_SquarePixels)
            {
                strcat(Context, SQUARE_MARK);
            }
            CTiffHelper TiffHelper(this, TIFF_CLASS_R);
            TiffHelper.SaveSnapshot(FilePath, NewFrameHeight, NewFrameWidth, pNewFrameBuffer, NewLinePitch, Context);
            break;
        }
    case STILL_TIFF_YCbCr:
        {
            if (m_SquarePixels)
            {
                strcat(Context, SQUARE_MARK);
            }
            CTiffHelper TiffHelper(this, TIFF_CLASS_Y);
            TiffHelper.SaveSnapshot(FilePath, NewFrameHeight, NewFrameWidth, pNewFrameBuffer, NewLinePitch, Context);
            break;
        }
    case STILL_JPEG:
        {
            CJpegHelper JpegHelper(this);
            JpegHelper.SaveSnapshot(FilePath, NewFrameHeight, NewFrameWidth, pNewFrameBuffer, NewLinePitch, Context);
            break;
        }
    default:
        break;
    }

    if (NewBuf != NULL)
    {
        free(NewBuf);
    }

    // Free the image frame buffer except if it is the current displayed
    if (pFrameBuffer != m_OriginalFrameBuffer)
    {
        m_PlayList[pos]->FreeBuffer();
    }

    // Call SetFileName after FreeBuffer because SetFileName reset memory pointers
    m_PlayList[pos]->SetFileName(FilePath);
}

void CStillSource::CreateSettings(LPCSTR IniSection)
{
}

void CStillSource::Start()
{
    if (!m_IsPictureRead)
    {
        // Try to load the file
        if (!ShowNextInPlayList())
        {
            ShowPreviousInPlayList();
        }

        // If no file of the playlist is readable
        if (!m_IsPictureRead)
        {
            PostMessageToMainWindow(WM_COMMAND, IDM_CLOSE_ALL, 0);
        }

        // The file is loaded and notification of size change is done
        // no need to call NotifySizeChange in this case
    }
    else
    {
        // Necessary if this still source was not the
        // current source when the file was loaded
        NotifySizeChange();
    }

    NotifySquarePixelsCheck();

    m_LastTickCount = 0;
    m_NewFileRequested = STILL_REQ_NONE;
}

void CStillSource::Stop()
{
    if (m_SlideShowActive)
    {
        m_SlideShowActive = FALSE;
        KillTimer(GetMainWnd(), TIMER_SLIDESHOW);
    }
    if (m_StillFrameBuffer != NULL)
    {
        free(m_StillFrameBuffer);
        m_StillFrameBuffer = NULL;
        m_StillFrame.pData = NULL;
    }
    FreeOriginalFrameBuffer();
    m_OriginalFrame.pData = NULL;
    m_IsPictureRead = FALSE;
    // Reset the sizes to 0 to be sure the next time
    // a file will be loaded, a new notification of
    // size change will be generated
    m_Width = 0;
    m_Height = 0;
}

void CStillSource::Reset()
{
}

void CStillSource::GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming)
{
    BOOL bSlept = FALSE;
    BOOL bLate = TRUE;
    DWORD CurrentTickCount;
    int FieldDistance;
    
    m_pMemcpy = pInfo->pMemcpy;

    if(!ReadNextFrameInFile())
    {
        pInfo->bRunningLate = TRUE;
        pInfo->bMissedFrame = TRUE;
        pInfo->FrameWidth = 720;
        pInfo->FrameHeight = 480;
        pInfo->PictureHistory[0] = NULL;
        return;
    }

    CurrentTickCount = GetTickCount();
    if (m_LastTickCount == 0)
    {
        m_LastTickCount = CurrentTickCount;
    }
    while((CurrentTickCount - m_LastTickCount) < m_FrameDuration)
    {
        if (!AccurateTiming)
        {
            Timing_SmartSleep(pInfo, FALSE, bSlept);
        }
        pInfo->bRunningLate = FALSE;            // if we waited then we are not late
        bLate = FALSE;                            // if we waited then we are not late
        CurrentTickCount = GetTickCount();
    }
    if (bLate)
    {
        Timing_IncrementNotWaitedFields();
    }
    FieldDistance = (CurrentTickCount - m_LastTickCount) / m_FrameDuration;
    if(FieldDistance == 1)
    {
        m_LastTickCount += m_FrameDuration;
        pInfo->bMissedFrame = FALSE;
        if (bLate)
        {
            LOG(2, " Running late but right field");
            if (pInfo->bRunningLate)
            {
                Timing_AddDroppedFields(1);
            }
        }
    }
    else if (FieldDistance <= (MaxFieldShift+1))
    {
        m_LastTickCount += m_FrameDuration;
        if (AccurateTiming)
        {
            Timing_SetFlipAdjustFlag(TRUE);
        }
        pInfo->bMissedFrame = FALSE;
        Timing_AddLateFields(FieldDistance - 1);
        LOG(2, " Running late by %d fields", FieldDistance - 1);
    }
    else
    {
        m_LastTickCount += m_FrameDuration * FieldDistance;
        // delete all history
        ClearPictureHistory(pInfo);
        pInfo->bMissedFrame = TRUE;
        Timing_AddDroppedFields(FieldDistance - 1);
        LOG(2, " Dropped %d Field(s)", FieldDistance - 1);
        if (AccurateTiming)
        {
            Timing_Reset();
        }
    }

    pInfo->LineLength = m_Width * 2;
    pInfo->FrameWidth = m_Width;
    pInfo->FrameHeight = m_Height;
    pInfo->FieldHeight = m_Height;
    pInfo->InputPitch = m_LinePitch;
    pInfo->PictureHistory[0] =  &m_StillFrame;

    if (AccurateTiming)
    {
        Timing_SmartSleep(pInfo, pInfo->bRunningLate, bSlept);
    }

    Timing_IncrementUsedFields();
}

BOOL CStillSource::HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
{
    CPlayListItem* Item;
    vector<CPlayListItem*>::iterator it;
    OPENFILENAME SaveFileInfo;
    char FilePath[MAX_PATH];
    char* FileFilters = "DScaler Playlists\0*.d3u\0";
    int pos;

    if ((m_Position == -1) || (m_PlayList.size() == 0) || (m_NewFileRequested != STILL_REQ_NONE))
        return FALSE;

    if (m_NavigOnly
     && (LOWORD(wParam) == IDM_PLAYLIST_UP
      || LOWORD(wParam) == IDM_PLAYLIST_DOWN
      || LOWORD(wParam) == IDM_CLOSE_FILE
      || LOWORD(wParam) == IDM_CLOSE_ALL
      || LOWORD(wParam) == IDM_PLAYLIST_SAVE
      || LOWORD(wParam) == IDM_PLAYLIST_PREVIEW
      || LOWORD(wParam) == IDM_SAVE_IN_FILE
      || LOWORD(wParam) == IDM_SAVE_ALL_IN_FILE))
        return FALSE;

    if ( (LOWORD(wParam) >= IDM_PLAYLIST_FILES) && (LOWORD(wParam) < (IDM_PLAYLIST_FILES+MAX_PLAYLIST_SIZE)) )
    {
        m_SlideShowActive = FALSE;
        m_NewFileRequested = STILL_REQ_THIS_ONE;
        m_NewFileReqPos = LOWORD(wParam) - IDM_PLAYLIST_FILES;
        return TRUE;
    }

    switch(LOWORD(wParam))
    {
    case IDM_PLAYLIST_CURRENT:
        m_SlideShowActive = FALSE;
        m_NewFileRequested = STILL_REQ_THIS_ONE;
        m_NewFileReqPos = m_Position;
        return TRUE;
        break;
    case IDM_PLAYLIST_INDEX:
        m_SlideShowActive = FALSE;
        m_NewFileRequested = STILL_REQ_THIS_ONE;
        m_NewFileReqPos = lParam;
        if (pMultiFrames && pMultiFrames->IsActive())
        {
            // We sleep to be sure that the still is correctly displayed
            // in the output thread before acknowledging the change of content
            Sleep(100);
            pMultiFrames->AckContentChange();
        }
        return TRUE;
        break;
    case IDM_PLAYLIST_PREVIOUS:
        if(m_Position > 0)
        {
            m_SlideShowActive = FALSE;
            m_NewFileRequested = STILL_REQ_PREVIOUS;
            m_NewFileReqPos = m_Position - 1;
        }
        return TRUE;
        break;
    case IDM_PLAYLIST_NEXT:
        if(m_Position < m_PlayList.size() - 1)
        {
            m_SlideShowActive = FALSE;
            m_NewFileRequested = STILL_REQ_NEXT;
            m_NewFileReqPos = m_Position + 1;
        }
        return TRUE;
        break;
    case IDM_PLAYLIST_PREVIOUS_CIRC:
        m_SlideShowActive = FALSE;
        m_NewFileRequested = STILL_REQ_PREVIOUS_CIRC;
        if(m_Position > 0)
        {
            m_NewFileReqPos = m_Position - 1;
        }
        else
        {
            m_NewFileReqPos = m_PlayList.size() - 1;
        }
        if (pMultiFrames && pMultiFrames->IsActive())
        {
            // We sleep to be sure that the still is correctly displayed
            // in the output thread before acknowledging the change of content
            Sleep(100);
            pMultiFrames->AckContentChange();
        }
        return TRUE;
        break;
    case IDM_PLAYLIST_NEXT_CIRC:
        m_SlideShowActive = FALSE;
        m_NewFileRequested = STILL_REQ_NEXT_CIRC;
        if(m_Position < m_PlayList.size() - 1)
        {
            m_NewFileReqPos = m_Position + 1;
        }
        else
        {
            m_NewFileReqPos = 0;
        }
        if (pMultiFrames && pMultiFrames->IsActive())
        {
            // We sleep to be sure that the still is correctly displayed
            // in the output thread before acknowledging the change of content
            Sleep(100);
            pMultiFrames->AckContentChange();
        }
        return TRUE;
        break;
    case IDM_PLAYLIST_FIRST:
        if(m_Position != 0)
        {
            m_SlideShowActive = FALSE;
            m_NewFileRequested = STILL_REQ_NEXT;
            m_NewFileReqPos = 0;
        }
        return TRUE;
        break;
    case IDM_PLAYLIST_LAST:
        if(m_Position != m_PlayList.size() - 1)
        {
            m_SlideShowActive = FALSE;
            m_NewFileRequested = STILL_REQ_PREVIOUS;
            m_NewFileReqPos = m_PlayList.size() - 1;
        }
        return TRUE;
        break;
    case IDM_PLAYLIST_UP:
        if(m_Position > 0)
        {
            m_SlideShowActive = FALSE;
            Item = m_PlayList[m_Position];
            it = m_PlayList.erase(m_PlayList.begin() + m_Position);
            --it;
            m_PlayList.insert(it, Item);
            --m_Position;
            UpdateMenu();
        }
        return TRUE;
        break;
    case IDM_PLAYLIST_DOWN:
        if(m_Position < m_PlayList.size() - 1)
        {
            m_SlideShowActive = FALSE;
            Item = m_PlayList[m_Position];
            it = m_PlayList.erase(m_PlayList.begin() + m_Position);
            ++it;
            if (it != m_PlayList.end())
            {
                m_PlayList.insert(it, Item);
            }
            else
            {
                m_PlayList.push_back(Item);
            }
            ++m_Position;
            UpdateMenu();
        }
        return TRUE;
        break;
    case IDM_PLAYLIST_SLIDESHOW:
        m_SlideShowActive = !m_SlideShowActive;
        if (m_SlideShowActive)
        {
            SetTimer(hWnd, TIMER_SLIDESHOW, Setting_GetValue(Still_GetSetting(SLIDESHOWDELAY)) * 1000, NULL);
        }
        else
        {
            KillTimer(hWnd, TIMER_SLIDESHOW);
        }
        return TRUE;
        break;
    case IDM_CLOSE_FILE:
        m_SlideShowActive = FALSE;
        m_PlayList.erase(m_PlayList.begin() + m_Position);
        if (m_Position >= m_PlayList.size())
        {
            m_Position = m_PlayList.size() - 1;
        }
        if (!IsAccessAllowed())
        {
            if (m_PlayList.size() > 0)
            {
                ClearPlayList();
                m_Position = -1;
            }
            UpdateMenu();
            PostMessageToMainWindow(WM_COMMAND, IDM_SWITCH_SOURCE, 0);
        }
        else
        {
            UpdateMenu();
            m_NewFileRequested = STILL_REQ_NEXT_CIRC;
            m_NewFileReqPos = m_Position;
        }
        return TRUE;
        break;
    case IDM_CLOSE_ALL:
        m_SlideShowActive = FALSE;
        ClearPlayList();
        m_Position = -1;
        UpdateMenu();
        PostMessageToMainWindow(WM_COMMAND, IDM_SWITCH_SOURCE, 0);
        return TRUE;
        break;
    case IDM_PLAYLIST_SAVE:
        SaveFileInfo.lStructSize = sizeof(SaveFileInfo);
        SaveFileInfo.hwndOwner = hWnd;
        SaveFileInfo.lpstrFilter = FileFilters;
        SaveFileInfo.lpstrCustomFilter = NULL;
        FilePath[0] = 0;
        SaveFileInfo.lpstrFile = FilePath;
        SaveFileInfo.nMaxFile = sizeof(FilePath);
        SaveFileInfo.lpstrFileTitle = NULL;
        SaveFileInfo.lpstrInitialDir = NULL;
        SaveFileInfo.lpstrTitle = "Save Playlist As";
        SaveFileInfo.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT;
        SaveFileInfo.lpstrDefExt = "d3u";
        if (GetSaveFileName(&SaveFileInfo))
        {
            if (!SavePlayList(FilePath))
            {
                MessageBox(hWnd, "Playlist not saved", "DScaler Warning", MB_OK);
            }
        }
        return TRUE;
        break;
    case IDM_SAVE_IN_FILE:
        SaveInFile(m_Position);
        UpdateMenu();
        return TRUE;
        break;
    case IDM_SAVE_ALL_IN_FILE:
        for (pos = 0 ; pos < m_PlayList.size() ; pos++)
        {
            SaveInFile(pos);
        }
        UpdateMenu();
        return TRUE;
        break;
    case IDM_PLAYLIST_PREVIEW:
        if (pMultiFrames == NULL)
        {
            pMultiFrames = new CMultiFrames(PREVIEW_STILLS, PreviewNbCols, PreviewNbRows, this);
        }
        pMultiFrames->RequestSwitch();
        return TRUE;
        break;
    default:
        break;
    }
    return FALSE;
}

BOOL CStillSource::ReadNextFrameInFile()
{
    BYTE* FrameBuffer;
    int FrameHeight;
    int FrameWidth;
    int LinePitch;
    BOOL SquarePixels;
    const char* Context;
    BOOL Realloc = FALSE;
    int CurrentPos = m_Position;

    if (m_NewFileRequested == STILL_REQ_THIS_ONE)
    {
        m_NewFileRequested = STILL_REQ_NONE;
        if (!m_PlayList[m_NewFileReqPos]->IsInMemory() && OpenPictureFile(m_PlayList[m_NewFileReqPos]->GetFileName()))
        {
            m_PlayList[m_NewFileReqPos]->SetSupported(TRUE);
            m_Position = m_NewFileReqPos;
            Realloc = TRUE;
        }
        else if(m_PlayList[m_NewFileReqPos]->GetMemoryInfo(&FrameBuffer, &FrameHeight, &FrameWidth, &LinePitch, &SquarePixels, &Context)
             && OpenPictureMemory(FrameBuffer, FrameHeight, FrameWidth, LinePitch, SquarePixels, Context))
        {
            m_PlayList[m_NewFileReqPos]->SetSupported(TRUE);
            m_Position = m_NewFileReqPos;
            Realloc = TRUE;
        }
        else
        {
            m_PlayList[m_NewFileReqPos]->SetSupported(FALSE);

            if (m_Position == m_NewFileReqPos)
            {
                // The user tried to open the current file
                // and this time the file is not readable
                m_NewFileRequested = STILL_REQ_NEXT_CIRC;
            }
        }
    }

    switch (m_NewFileRequested)
    {
    case STILL_REQ_NEXT:
        m_Position = m_NewFileReqPos;
        if (ShowNextInPlayList())
        {
            Realloc = TRUE;
        }
        else
        {
            m_Position = CurrentPos;
        }
        break;
    case STILL_REQ_NEXT_CIRC:
        m_Position = m_NewFileReqPos;
        if (ShowNextInPlayList())
        {
            Realloc = TRUE;
        }
        else
        {
            m_Position = 0;
            if (ShowNextInPlayList())
            {
                Realloc = TRUE;
            }
            else
            {
                m_Position = CurrentPos;
                PostMessageToMainWindow(WM_COMMAND, IDM_CLOSE_ALL, 0);
            }
        }
        break;
    case STILL_REQ_PREVIOUS:
        m_Position = m_NewFileReqPos;
        if (ShowPreviousInPlayList())
        {
            Realloc = TRUE;
        }
        else
        {
            m_Position = CurrentPos;
        }
        break;
    case STILL_REQ_PREVIOUS_CIRC:
        m_Position = m_NewFileReqPos;
        if (ShowPreviousInPlayList())
        {
            Realloc = TRUE;
        }
        else
        {
            m_Position = m_PlayList.size() - 1;
            if (ShowPreviousInPlayList())
            {
                Realloc = TRUE;
            }
            else
            {
                m_Position = CurrentPos;
                PostMessageToMainWindow(WM_COMMAND, IDM_CLOSE_ALL, 0);
            }
        }
        break;
    case STILL_REQ_NONE:
    default:
        break;
    }
    m_NewFileRequested = STILL_REQ_NONE;

    if (m_IsPictureRead)
    {
        if (Realloc && m_StillFrameBuffer != NULL)
        {
            free(m_StillFrameBuffer);
            m_StillFrameBuffer = NULL;
            m_StillFrame.pData = NULL;
        }
        if (m_StillFrameBuffer == NULL)
        {
            m_StillFrameBuffer = (BYTE*)malloc(m_LinePitch * m_Height + 16);
            if (m_StillFrameBuffer != NULL)
            {
                m_StillFrame.pData = START_ALIGNED16(m_StillFrameBuffer);
            }
        }
        if (m_StillFrame.pData != NULL && m_OriginalFrame.pData != NULL)
        {
            //
            // WARNING: optimized memcpy seems to be the source of problem with certain hardware configurations
            //
            // RM sept 24 2006: Added missing emms instruction, maybe this was causing these problems?
            //
            m_pMemcpy(m_StillFrame.pData, m_OriginalFrame.pData, m_LinePitch * m_Height);
            _asm
            {
                emms
            }
            return TRUE;
        }
        return FALSE;
    }
    else
    {
        return FALSE;
    }
}

LPCSTR CStillSource::GetStatus()
{
    if(m_Position != -1 && m_PlayList.size() > 0)
    {
        LPCSTR FileName;

        FileName = strrchr(m_PlayList[m_Position]->GetFileName(), '\\');
        if (FileName == NULL)
        {
            FileName = m_PlayList[m_Position]->GetFileName();
        }
        else
        {
            ++FileName;
        }
        return FileName;
    }
    else
    {
        return "No File";
    }
}

eVideoFormat CStillSource::GetFormat()
{
    return VIDEOFORMAT_PAL_B;
}

int CStillSource::GetInitialWidth()
{
    return m_InitialWidth;
}

int CStillSource::GetInitialHeight()
{
    return m_InitialHeight;
}

int CStillSource::GetWidth()
{
    return m_Width;
}

int CStillSource::GetHeight()
{
    return m_Height;
}

void CStillSource::FreeOriginalFrameBuffer()
{
    BYTE* FrameBuffer;
    int FrameHeight;
    int FrameWidth;
    int LinePitch;
    BOOL SquarePixels;
    const char* Context;
    BOOL OkToFree = TRUE;

    if (m_OriginalFrameBuffer == NULL)
    {
        return;
    }

    for(vector<CPlayListItem*>::iterator it = m_PlayList.begin(); 
        it != m_PlayList.end(); 
        ++it)
    {
        if((*it)->GetMemoryInfo(&FrameBuffer, &FrameHeight, &FrameWidth, &LinePitch, &SquarePixels, &Context)
        && (m_OriginalFrameBuffer == FrameBuffer))
        {
            OkToFree = FALSE;
            break;
        }
    }

    if (OkToFree)
    {
        LOG(2, "FreeOriginalFrameBuffer - m_OriginalFrameBuffer %d", m_OriginalFrameBuffer);
        free(m_OriginalFrameBuffer);
    }

    m_OriginalFrameBuffer = NULL;
}

void CStillSource::ClearPlayList()
{
    BYTE* FrameBuffer;
    int FrameHeight;
    int FrameWidth;
    int LinePitch;
    BOOL SquarePixels;
    const char* Context;

    for(vector<CPlayListItem*>::iterator it = m_PlayList.begin(); 
        it != m_PlayList.end(); 
        ++it)
    {
        if((*it)->GetMemoryInfo(&FrameBuffer, &FrameHeight, &FrameWidth, &LinePitch, &SquarePixels, &Context)
        && (FrameBuffer != m_OriginalFrameBuffer))
        {
            (*it)->FreeBuffer();
        }
        delete (*it);
    }
    m_PlayList.clear();
}

BOOL CStillSource::SavePlayList(LPCSTR FileName)
{
    FILE* Playlist = fopen(FileName, "w");
    if (Playlist == NULL)
        return FALSE;

    for(vector<CPlayListItem*>::iterator it = m_PlayList.begin(); 
        it != m_PlayList.end(); 
        ++it)
    {
        if (!(*it)->IsInMemory())
        {
            fprintf(Playlist, "%s\n", (*it)->GetFileName());
        }
    }

    fclose(Playlist);

    return TRUE;
}

int CStillSource::CountMemoryUsage()
{
    BYTE* FrameBuffer;
    int FrameHeight;
    int FrameWidth;
    int LinePitch;
    BOOL SquarePixels;
    const char* Context;
    int Count = 0;

    for(vector<CPlayListItem*>::iterator it = m_PlayList.begin(); 
        it != m_PlayList.end(); 
        ++it)
    {
        if((*it)->GetMemoryInfo(&FrameBuffer, &FrameHeight, &FrameWidth, &LinePitch, &SquarePixels, &Context))
        {
            Count += LinePitch * FrameHeight;
        }
    }

    return Count;
}

void CStillSource::UpdateMenu()
{
    HMENU           hSubMenu;
    HMENU           hMenuFiles;
    MENUITEMINFO    MenuItemInfo;
    int             j;

    hSubMenu = GetSubMenu(m_hMenu, 0);
    if(hSubMenu == NULL) return;
    hMenuFiles = GetSubMenu(hSubMenu, 7);
    if(hMenuFiles == NULL)
    {
        if (ModifyMenu(hSubMenu, 7, MF_STRING | MF_BYPOSITION | MF_POPUP, (UINT)CreatePopupMenu(), "F&iles"))
        {
            hMenuFiles = GetSubMenu(hSubMenu, 7);
        }
    }
    if(hMenuFiles == NULL) return;

    j = GetMenuItemCount(hMenuFiles);
    while (j)
    {
        --j;
        RemoveMenu(hMenuFiles, j, MF_BYPOSITION);
    }
    
    j = 0;
    for (vector<CPlayListItem*>::iterator it = m_PlayList.begin(); 
         (it != m_PlayList.end()) && (j < MAX_PLAYLIST_SIZE); 
         ++it, ++j)
    {
        LPCSTR FileName = strrchr((*it)->GetFileName(), '\\');
        if (FileName == NULL)
        {
            FileName = (*it)->GetFileName();
        }
        else
        {
            ++FileName;
        }
        MenuItemInfo.cbSize = sizeof (MenuItemInfo);
        MenuItemInfo.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
        MenuItemInfo.fType = MFT_STRING;
        MenuItemInfo.dwTypeData = (LPSTR) FileName;
        MenuItemInfo.cch = strlen (FileName);
        MenuItemInfo.fState = (m_Position == j) ? MFS_CHECKED : MFS_UNCHECKED;
        MenuItemInfo.wID = IDM_PLAYLIST_FILES + j;
        InsertMenuItem(hMenuFiles, j, TRUE, &MenuItemInfo);
    }
}

void CStillSource::SetMenu(HMENU hMenu)
{
    HMENU           hSubMenu;
    HMENU           hMenuFiles;
    BOOL            OneInMemory;

    CheckMenuItemBool(hMenu, IDM_PLAYLIST_PREVIEW, pMultiFrames && (pMultiFrames->GetMode() == PREVIEW_STILLS) && pMultiFrames->IsActive());
    EnableMenuItem(hMenu, IDM_PLAYLIST_PREVIEW, m_NavigOnly ? MF_GRAYED : MF_ENABLED);
    CheckMenuItemBool(hMenu, IDM_PLAYLIST_SLIDESHOW, m_SlideShowActive);
    if(m_PlayList.size() > 1)
    {
        if(m_Position > 0)
        {
            EnableMenuItem(hMenu, IDM_PLAYLIST_PREVIOUS, MF_ENABLED);
            EnableMenuItem(hMenu, IDM_PLAYLIST_FIRST, MF_ENABLED);
            EnableMenuItem(hMenu, IDM_PLAYLIST_UP, m_NavigOnly ? MF_GRAYED : MF_ENABLED);
        }
        else
        {
            EnableMenuItem(hMenu, IDM_PLAYLIST_PREVIOUS, MF_GRAYED);
            EnableMenuItem(hMenu, IDM_PLAYLIST_FIRST, MF_GRAYED);
            EnableMenuItem(hMenu, IDM_PLAYLIST_UP, MF_GRAYED);
        }
        if(m_Position < m_PlayList.size() - 1)
        {
            EnableMenuItem(hMenu, IDM_PLAYLIST_NEXT, MF_ENABLED);
            EnableMenuItem(hMenu, IDM_PLAYLIST_LAST, MF_ENABLED);
            EnableMenuItem(hMenu, IDM_PLAYLIST_DOWN, m_NavigOnly ? MF_GRAYED : MF_ENABLED);
        }
        else
        {
            EnableMenuItem(hMenu, IDM_PLAYLIST_NEXT, MF_GRAYED);
            EnableMenuItem(hMenu, IDM_PLAYLIST_LAST, MF_GRAYED);
            EnableMenuItem(hMenu, IDM_PLAYLIST_DOWN, MF_GRAYED);
        }
        EnableMenuItem(hMenu, IDM_PLAYLIST_SLIDESHOW, MF_ENABLED);
    }
    else
    {
        EnableMenuItem(hMenu, IDM_PLAYLIST_PREVIOUS, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_PLAYLIST_NEXT, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_PLAYLIST_FIRST, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_PLAYLIST_LAST, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_PLAYLIST_UP, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_PLAYLIST_DOWN, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_PLAYLIST_SLIDESHOW, MF_GRAYED);
    }

    EnableMenuItem(hMenu, IDM_CLOSE_FILE, m_NavigOnly ? MF_GRAYED : MF_ENABLED);
    EnableMenuItem(hMenu, IDM_CLOSE_ALL, m_NavigOnly ? MF_GRAYED : MF_ENABLED);
    EnableMenuItem(hMenu, IDM_PLAYLIST_SAVE, m_NavigOnly ? MF_GRAYED : MF_ENABLED);

    if (m_NavigOnly
     || (m_Position == -1)
     || (m_PlayList.size() == 0)
     || !m_PlayList[m_Position]->IsInMemory())
    {
        EnableMenuItem(hMenu, IDM_SAVE_IN_FILE, MF_GRAYED);
    }
    else
    {
        EnableMenuItem(hMenu, IDM_SAVE_IN_FILE, MF_ENABLED);
    }

    OneInMemory = IsOneItemInMemory();
    EnableMenuItem(hMenu, IDM_SAVE_ALL_IN_FILE, OneInMemory ? MF_ENABLED : MF_GRAYED);

    hSubMenu = GetSubMenu(m_hMenu, 0);
    if(hSubMenu == NULL) return;
    hMenuFiles = GetSubMenu(hSubMenu, 7);
    if(hMenuFiles == NULL) return;

    for (int i(0); (i < MAX_PLAYLIST_SIZE) && (i < GetMenuItemCount(hMenuFiles)); ++i)
    {
        if (m_Position == i)
        {
            CheckMenuItem(hMenuFiles, i, MF_BYPOSITION | MF_CHECKED);
        }
        else
        {
            CheckMenuItem(hMenuFiles, i, MF_BYPOSITION | MF_UNCHECKED);
        }
        EnableMenuItem(hMenuFiles, i, m_PlayList[i]->IsSupported() ? MF_BYPOSITION | MF_ENABLED : MF_BYPOSITION | MF_GRAYED);
    }
}

void CStillSource::HandleTimerMessages(int TimerId)
{
    if (m_NewFileRequested != STILL_REQ_NONE)
        return;

    if ( (TimerId == TIMER_SLIDESHOW) && m_SlideShowActive )
    {
        m_NewFileRequested = STILL_REQ_NEXT_CIRC;
        m_NewFileReqPos = m_Position + 1;
        m_SlideShowActive = TRUE;
        SetTimer(GetMainWnd(), TIMER_SLIDESHOW, Setting_GetValue(Still_GetSetting(SLIDESHOWDELAY)) * 1000, NULL);
    }
}

LPCSTR CStillSource::GetMenuLabel()
{
    return m_Section.c_str();
}

BOOL CStillSource::IsVideoPresent()
{
    return m_IsPictureRead;
}

BOOL CStillSource::IsAccessAllowed()
{
    if ((m_Position == -1) || (m_PlayList.size() == 0))
        return FALSE;

    for(vector<CPlayListItem*>::iterator it = m_PlayList.begin(); 
        it != m_PlayList.end(); 
        ++it)
    {
        if ((*it)->IsSupported())
            return TRUE;
    }

    return FALSE;
}

void CStillSource::SetNavigOnly(BOOL NavigOnly)
{
    m_NavigOnly = NavigOnly;
}

BOOL CStillSource::IsNavigOnly()
{
    return m_NavigOnly;
}

void CStillSource::SetAspectRatioData()
{
    AspectSettings.InitialTopOverscan = 0;
    AspectSettings.InitialBottomOverscan = 0;
    AspectSettings.InitialLeftOverscan = 0;
    AspectSettings.InitialRightOverscan = 0;
    AspectSettings.bAnalogueBlanking = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////

BOOL Pattern_Height_OnChange(long NewValue)
{
    PatternHeight = (int)NewValue;
    PostMessageToMainWindow(WM_COMMAND, IDM_PLAYLIST_CURRENT, 0);
    return FALSE;
}

BOOL Pattern_Width_OnChange(long NewValue)
{
    PatternWidth = (int)NewValue;
    PostMessageToMainWindow(WM_COMMAND, IDM_PLAYLIST_CURRENT, 0);
    return FALSE;
}

BOOL PreviewNbCols_OnChange(long NewValue)
{
    PreviewNbCols = (int)NewValue;
    if (pMultiFrames && (pMultiFrames->GetMode() == PREVIEW_STILLS) && pMultiFrames->IsActive())
    {
        pMultiFrames->RequestSwitch();
    }
    return FALSE;
}

BOOL PreviewNbRows_OnChange(long NewValue)
{
    PreviewNbRows = (int)NewValue;
    if (pMultiFrames && (pMultiFrames->GetMode() == PREVIEW_STILLS) && pMultiFrames->IsActive())
    {
        pMultiFrames->RequestSwitch();
    }
    return FALSE;
}

SETTING StillSettings[STILL_SETTING_LASTONE] =
{
    {
        "Format of saving", ITEMFROMLIST, 0, (long*)&FormatSaving,
         STILL_TIFF_RGB, STILL_TIFF_RGB, STILL_FORMAT_LASTONE - 1, 1, 1,
         StillFormatNames,
        "Still", "FormatSaving", NULL,
    },
    {
        "Slide Show Delay", SLIDER, 0, (long*)&SlideShowDelay,
         5, 1, 60, 1, 1,
         NULL,
        "Still", "SlideShowDelay", NULL,
    },
    {
        "JPEG quality compression", SLIDER, 0, (long*)&JpegQuality,
         95, 0, 100, 1, 1,
         NULL,
        "Still", "JPEGQuality", NULL,
    },
    {
        "Pattern height", SLIDER, 0, (long*)&PatternHeight,
         576, 480, DSCALER_MAX_HEIGHT, 1, 1,
         NULL,
        "Pattern", "PatternHeight", Pattern_Height_OnChange,
    },
    {
        "Pattern width", SLIDER, 0, (long*)&PatternWidth,
         720, 240, DSCALER_MAX_WIDTH, 1, 1,
         NULL,
        "Pattern", "PatternWidth", Pattern_Width_OnChange,
    },
    {
        "Delay between stills (1/10 seconds", SLIDER, 0, (long*)&DelayBetweenStills,
         600, 1, 36000, 1, 1,
         NULL,
        "Still", "DelayBetweenStills", NULL,
    },
    {
        "Save in the same file", ONOFF, 0, (long*)&SaveInSameFile,
         FALSE, 0, 1, 1, 1,
         NULL,
        "Still", "SaveInSameFile", NULL,
    },
    {
        "Stills in memory", ONOFF, 0, (long*)&StillsInMemory,
         FALSE, 0, 1, 1, 1,
         NULL,
        "Still", "StillsInMemory", NULL,
    },
    {
        "Number of consecutive stills", SLIDER, 0, (long*)&NbConsecutiveStills,
         20, 2, 60, 1, 1,
         NULL,
        "Still", "NbConsecutiveStills", NULL,
    },
    {
        "Keep original ratio", ONOFF, 0, (long*)&KeepOriginalRatio,
         FALSE, 0, 1, 1, 1,
         NULL,
        "Still", "KeepOriginalRatio", NULL,
    },
    {
        "OSD when taking a still", ONOFF, 0, (long*)&OSDForStills,
         TRUE, 0, 1, 1, 1,
         NULL,
        "Still", "OSDForStills", NULL,
    },
    {
        "Number of columns in preview mode", SLIDER, 0, (long*)&PreviewNbCols,
         4, 2, 10, 1, 1,
         NULL,
        "Still", "PreviewNbCols", PreviewNbCols_OnChange,
    },
    {
        "Number of rows in preview mode", SLIDER, 0, (long*)&PreviewNbRows,
         4, 2, 10, 1, 1,
         NULL,
        "Still", "PreviewNbRows", PreviewNbRows_OnChange,
    },
    {
        "Maximum memory usage for stills (Mo)", SLIDER, 0, (long*)&MaxMemForStills,
         64, 32, 256, 1, 1,
         NULL,
        "Still", "MaxMemForStills", NULL,
    },
    {
        "Saving path for stills", CHARSTRING, 0, (long*)&SavingPath,
         (long)ExePath, 0, 0, 0, 0,
         NULL,
        "Still", "SavingPath", NULL,
    },
};


SETTING* Still_GetSetting(STILL_SETTING Setting)
{
    if(Setting > -1 && Setting < STILL_SETTING_LASTONE)
    {
        return &(StillSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void Still_ReadSettingsFromIni()
{
    int i;
    struct stat st;

    GetModuleFileName (NULL, ExePath, sizeof(ExePath));
    *(strrchr(ExePath, '\\')) = '\0';

    for(i = 0; i < STILL_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(StillSettings[i]));
    }

    if ((SavingPath == NULL) || stat(SavingPath, &st))
    {
        LOG(1, "Incorrect path for snapshots; using %s", ExePath);
        Setting_SetValue(Still_GetSetting(SAVINGPATH), (long)ExePath);
    }
}

void Still_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < STILL_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(StillSettings[i]), bOptimizeFileAccess);
    }
}

CTreeSettingsGeneric* Still_GetTreeSettingsPage()
{
    return new CTreeSettingsGeneric("Still Settings",StillSettings, STILL_SETTING_LASTONE);
}

void Still_FreeSettings()
{
    int i;
    for(i = 0; i < STILL_SETTING_LASTONE; i++)
    {
        Setting_Free(&StillSettings[i]);
    }
}


BOOL ResizeFrame(BYTE* OldBuf, int OldPitch, int OldWidth, int OldHeight, BYTE* NewBuf, int NewPitch, int NewWidth, int NewHeight)
{
    unsigned int* hControl;        // weighting masks, alternating dwords for Y & UV
                                // 1 qword for mask, 1 dword for src offset, 1 unused dword
    unsigned int* vOffsets;        // Vertical offsets of the source lines we will use
    unsigned int* vWeights;        // weighting masks, alternating dwords for Y & UV
    unsigned int* vWorkY;        // weighting masks 0Y0Y 0Y0Y...
    unsigned int* vWorkUV;        // weighting masks UVUV UVUV...

    int vWeight1[4];
    int vWeight2[4];

    // how do I do a __m128 constant, init and aligned on 16 byte boundar?
    const __int64 YMask[2]    = {0x00ff00ff00ff00ff,0x00ff00ff00ff00ff}; // keeps only luma
    const __int64 FPround1[2] = {0x0080008000800080,0x0080008000800080}; // round words
    const __int64 FPround2[2] = {0x0000008000000080,0x0000008000000080};// round dwords

    unsigned char* dstp;
    unsigned char* srcp1;
    unsigned char* srcp2;

    LOG(3, "OldWidth %d, NewWidth %d, OldHeight %d, NewHeight %d", OldWidth, NewWidth, OldHeight, NewHeight);

    dstp = NewBuf;

    // SimpleResize Init code
    hControl = (unsigned int*) malloc(NewWidth*12+128);  
    vOffsets = (unsigned int*) malloc(NewHeight*4);  
    vWeights = (unsigned int*) malloc(NewHeight*4);  
    vWorkY =   (unsigned int*) malloc(2*OldWidth+128);   
    vWorkUV =  (unsigned int*) malloc(OldWidth+128);   

    SimpleResize_InitTables(hControl, vOffsets, vWeights,
        OldWidth, OldHeight, NewWidth, NewHeight);


    // SimpleResize resize code

    if (NewBuf == NULL || hControl == NULL || vOffsets == NULL || vWeights == NULL
        || vWorkY == NULL || vWorkUV == NULL)
    {
        return FALSE;
    }
    
    for (int y = 0; y < NewHeight; y++)
    {

        vWeight1[0] = vWeight1[1] = vWeight1[2] = vWeight1[3] = 
            (256-vWeights[y]) << 16 | (256-vWeights[y]);
        vWeight2[0] = vWeight2[1] = vWeight2[2] = vWeight2[3] = 
            vWeights[y] << 16 | vWeights[y];

        srcp1 = OldBuf + vOffsets[y] * OldPitch;
        
        srcp2 = (y < NewHeight-1)
            ? srcp1 + OldPitch
            : srcp1;

        _asm        
        {
            push    ecx                        // have to save this?
            mov        ecx, OldWidth
            shr        ecx, 2                    // 8 bytes a time
            inc     ecx                        // do extra 8 bytes to pick up odd widths
                                            // we have malloced enough to get away with it
            mov        esi, srcp1                // top of 2 src lines to get
            mov        edx, srcp2                // next "
            mov        edi, vWorkY                // luma work destination line
            mov        ebx, vWorkUV            // luma work destination line
            xor        eax, eax

            movq    mm7, YMask                // useful luma mask constant
            movq    mm5, vWeight1
            movq    mm6, vWeight2
            movq    mm0, FPround1            // useful rounding constant
            align    16
    vLoopMMX:    
            movq    mm1, qword ptr[esi+eax*2] // top of 2 lines to interpolate
            movq    mm2, qword ptr[edx+eax*2] // 2nd of 2 lines

            movq    mm3, mm1                // copy top bytes
            pand    mm1, mm7                // keep only luma    
            pxor    mm3, mm1                // keep only chroma
            psrlw    mm3, 8                    // right just chroma
            pmullw    mm1, mm5                // mult by weighting factor
            pmullw    mm3, mm5                // mult by weighting factor

            movq    mm4, mm2                // copy 2nd bytes
            pand    mm2, mm7                // keep only luma    
            pxor    mm4, mm2                // keep only chroma
            psrlw    mm4, 8                    // right just chroma
            pmullw    mm2, mm6                // mult by weighting factor
            pmullw    mm4, mm6                // mult by weighting factor

            paddw    mm1, mm2                // combine lumas
            paddusw    mm1, mm0                // round
            psrlw    mm1, 8                    // right adjust luma
            movq    qword ptr[edi+eax*2], mm1    // save lumas in our work area

            paddw    mm3, mm4                // combine chromas
            paddusw    mm3, mm0                // round
            psrlw    mm3, 8                    // right adjust chroma
            packuswb mm3,mm3                // pack UV's into low dword
            movd    dword ptr[ebx+eax], mm3    // save in our work area

            lea     eax, [eax+4]
            loop    vLoopMMX
    

// We've taken care of the vertical scaling, now do horizontal
            movq    mm7, YMask            // useful 0U0U..  mask constant
            movq    mm6, FPround2            // useful rounding constant, dwords
            mov        esi, hControl        // @ horiz control bytes            
            mov        ecx, NewWidth
            shr        ecx, 1                // 4 bytes a time, 2 pixels
            mov     edx, vWorkY            // our luma data, as 0Y0Y 0Y0Y..
            mov        edi, dstp            // the destination line
            mov        ebx, vWorkUV        // chroma data, as UVUV UVUV...
            align 16
    hLoopMMX:   
            mov        eax, [esi+16]        // get data offset in pixels, 1st pixel pair
            movd mm0, [edx+eax*2]        // copy luma pair
            shr        eax, 1                // div offset by 2
            movd    mm1, [ebx+eax*2]    // copy UV pair VUVU
            psllw   mm1, 8                // shift out V, keep 0000U0U0    

            mov        eax, [esi+20]        // get data offset in pixels, 2nd pixel pair
            punpckldq mm0, [edx+eax*2]        // copy luma pair
            shr        eax, 1                // div offset by 2
            punpckldq mm1, [ebx+eax*2]    // copy UV pair VUVU
            psrlw   mm1, 8                // shift out U0, keep 0V0V 0U0U    
        
            pmaddwd mm0, [esi]            // mult and sum lumas by ctl weights
            paddusw    mm0, mm6            // round
            psrlw    mm0, 8                // right just 2 luma pixel value 000Y,000Y

            pmaddwd mm1, [esi+8]        // mult and sum chromas by ctl weights
            paddusw    mm1, mm6            // round
            pslld    mm1, 8                // shift into low bytes of different words
            pand    mm1, mm7            // keep only 2 chroma values 0V00,0U00
            por        mm0, mm1            // combine luma and chroma, 0V0Y,0U0Y
            packuswb mm0,mm0            // pack all into low dword, xxxxVYUY
            movd    dword ptr[edi], mm0    // done with 2 pixels

            lea    esi, [esi+24]        // bump to next control bytest
            lea    edi, [edi+4]            // bump to next output pixel addr
            loop   hLoopMMX                // loop for more

            pop        ecx
            emms
        }                               // done with one line
        dstp += NewPitch;
    }


    // SimpleResize cleanup code
    free(hControl);
    free(vOffsets);
    free(vWeights);
    free(vWorkY);
    free(vWorkUV);

    return TRUE;
}


// This function accepts a position from 0 to 1 and warps it, to 0 through 1 based
// upon the wFact var. The warp equations are designed to:
// 
// * Always be rising but yield results from 0 to 1
//
// * Have a first derivative that doesn't go to 0 or infinity, at least close
//   to the center of the screen
//
// * Have a curvature (absolute val of 2nd derivative) that is small in the
//   center and smoothly rises towards the edges. We would like the curvature
//   to be everywhere = 0 when the warp factor = 1
//

// For each horizontal output pixel there are 2 4 byte masks, and a src offset. The first gives
// the weights of the 4 surrounding luma values in the loaded qword, the second gives
// the weights of the chroma pixels. At most 2 values will be non-zero in each mask.
// The hControl offsets in the table are to where to load the qword of pixel data. Usually the
// 2nd & 3rd pixel values in that qword are used, but boundary conditions may change
// that and you can't average 2 adjacent chroma values in YUY2 format because they will
// contain YU YV YU YV YU YV...
// Horizontal weights are scaled 0-128, Vertical weights are scaled 0-256.

int __stdcall SimpleResize_InitTables(unsigned int* hControl, unsigned int* vOffsets, 
        unsigned int* vWeights, int OldWidth, int OldHeight, int NewWidth, int NewHeight)
{
    int i;
    int j;
    int k;
    int wY1;
    int wY2;
    int wUV1;
    int wUV2;
    int OldWidthW = OldWidth & 0xfffffffe;        // odd width is invalid YUY2
    // First set up horizontal table
    for(i=0; i < NewWidth; i+=2)
    {
        j = i * 256 * (OldWidthW-1) / (NewWidth-1);

        k = j>>8;
        wY2 = j - (k << 8);                // luma weight of right pixel
        wY1 = 256 - wY2;                // luma weight of left pixel
        wUV2 = (k%2)
            ? 128 + (wY2 >> 1)
            : wY2 >> 1;
        wUV1 = 256 - wUV2;

// the right hand edge luma will be off by one pixel currently to handle edge conditions.
// I should figure a better way aound this without a performance hit. But I can't see 
// the difference so it is lower prority.
        if (k > OldWidthW - 3)
        {
            hControl[i*3+4] = OldWidthW - 3;     //    point to last byte
            hControl[i*3] =   0x01000000;    // use 100% of rightmost Y
            hControl[i*3+2] = 0x01000000;    // use 100% of rightmost U
        }
        else
        {
            hControl[i*3+4] = k;            // pixel offset
            hControl[i*3] = wY2 << 16 | wY1; // luma weights
            hControl[i*3+2] = wUV2 << 16 | wUV1; // chroma weights
        }

        j = (i+1) * 256 * (OldWidthW-1) / (NewWidth-1);

        k = j>>8;
        wY2 = j - (k << 8);                // luma weight of right pixel
        wY1 = 256 - wY2;                // luma weight of left pixel
        wUV2 = (k%2)
            ? 128 + (wY2 >> 1)
            : wY2 >> 1;
        wUV1 = 256 - wUV2;

        if (k > OldWidthW - 3)
        {
            hControl[i*3+5] = OldWidthW - 3;     //    point to last byte
            hControl[i*3+1] = 0x01000000;    // use 100% of rightmost Y
            hControl[i*3+3] = 0x01000000;    // use 100% of rightmost V
        }
        else
        {
            hControl[i*3+5] = k;            // pixel offset
            hControl[i*3+1] = wY2 << 16 | wY1; // luma weights
            hControl[i*3+3] = wUV2 << 16 | wUV1; // chroma weights
        }
    }

    hControl[NewWidth*3+4] =  2 * (OldWidthW-1);        // give it something to prefetch at end
    hControl[NewWidth*3+5] =  2 * (OldWidthW-1);        // "

    // Next set up vertical table. The offsets are measured in lines and will be mult
    // by the source pitch later 
    for(i=0; i< NewHeight; ++i)
    {
        j = i * 256 * (OldHeight-1) / (NewHeight-1);
        k = j >> 8;
        vOffsets[i] = k;
        wY2 = j - (k << 8); 
        vWeights[i] = wY2;                // weight to give to 2nd line
    }

    return 0;
}

BOOL CStillSource::IsItemInList(LPCSTR FileName)
{
    BOOL found = FALSE;

    for(vector<CPlayListItem*>::iterator it = m_PlayList.begin(); 
        it != m_PlayList.end(); 
        ++it)
    {
        if (! strcmp((*it)->GetFileName(), FileName))
        {
            found = TRUE;
            break;
        }
    }
    return found;
}

BOOL CStillSource::IsOneItemInMemory()
{
    BOOL OneInMemory = FALSE;

    if (!m_NavigOnly && (m_PlayList.size() > 0))
    {
        for(vector<CPlayListItem*>::iterator it = m_PlayList.begin(); 
            it != m_PlayList.end(); 
            ++it)
        {
            if ((*it)->IsInMemory())
            {
                OneInMemory = TRUE;
                break;
            }
        }
    }
    return OneInMemory;
}
