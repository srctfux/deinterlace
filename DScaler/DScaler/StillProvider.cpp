/////////////////////////////////////////////////////////////////////////////
// $Id: StillProvider.cpp,v 1.17 2002-05-23 21:25:33 robmuller Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.16  2002/05/02 20:16:27  laurentg
// JPEG format added to take still
//
// Revision 1.15  2002/04/27 00:38:33  laurentg
// New default source (still) used at DScaler startup or when there is no more source accessible
//
// Revision 1.14  2002/03/30 13:18:31  laurentg
// New ini setting to choose the directory where to save snapshots
//
// Revision 1.13  2002/03/29 09:14:00  robmuller
// Fixed lockup when there are too many captures.
//
// Revision 1.12  2002/02/18 23:25:01  laurentg
// At startup, go to the first source having a content
// Order of still sources changed (Patterns before Snapshots)
//
// Revision 1.11  2002/02/11 21:33:13  laurentg
// Patterns as a new source from the Still provider
//
// Revision 1.10  2001/12/08 13:48:40  laurentg
// New StillSource for snapshots done during the DScaler session
//
// Revision 1.9  2001/12/08 12:04:07  laurentg
// New setting m_StillFormat
//
// Revision 1.8  2001/11/28 16:04:50  adcockj
// Major reorganization of STill support
//
// Revision 1.7  2001/11/26 13:02:27  adcockj
// Bug Fixes and standards changes
//
// Revision 1.6  2001/11/25 21:21:56  laurentg
// Destructor modified to delete sources
//
// Revision 1.5  2001/11/24 17:58:06  laurentg
// Still source
//
// Revision 1.4  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.3  2001/11/21 12:32:11  adcockj
// Renamed CInterlacedSource to CSource in preparation for changes to DEINTERLACE_INFO
//
// Revision 1.2  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.1.2.2  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.1.2.1  2001/08/15 14:44:05  adcockj
// Starting to put some flesh onto the new structure
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "StillProvider.h"
#include "Other.h"
#include "Providers.h"


#define MAX_SNAPSHOT_FILES 10000


CStillProvider::CStillProvider()
{
    char PlayList[MAX_PATH];
    CStillSource* pStillSource;

    pStillSource = new CStillSource("Still");
    m_StillSources.push_back(pStillSource);

    pStillSource = new CStillSource("Patterns");
    m_StillSources.push_back(pStillSource);
    GetModuleFileName (NULL, PlayList, sizeof(PlayList));
    strcpy(strrchr(PlayList, '\\'), "\\patterns\\pj_calibr.d3u");
    pStillSource->LoadPlayList(PlayList);

    pStillSource = new CStillSource("Snapshots");
    m_StillSources.push_back(pStillSource);

    pStillSource = new CStillSource("DScaler intro");
    m_StillSources.push_back(pStillSource);
    GetModuleFileName (NULL, PlayList, sizeof(PlayList));
    strcpy(strrchr(PlayList, '\\'), "\\DScaler.d3u");
    pStillSource->OpenMediaFile(PlayList, TRUE);
    pStillSource->SetNavigOnly(TRUE);
}

CStillProvider::~CStillProvider()
{
    for(vector<CStillSource*>::iterator it = m_StillSources.begin();
        it != m_StillSources.end();
        ++it)
    {
        delete *it;
    }
}

int CStillProvider::GetNumberOfSources()
{
    return m_StillSources.size();
}


CSource* CStillProvider::GetSource(int SourceIndex)
{
    if(SourceIndex >= 0 && SourceIndex < m_StillSources.size())
    {
        return m_StillSources[SourceIndex];
    }
    else
    {
        return NULL;
    }
}

void StillProvider_SaveSnapshot(TDeinterlaceInfo* pInfo)
{
    CStillSource* pStillSource = (CStillSource*) Providers_GetSnapshotsSource();

    if(pStillSource != NULL)
    {
        int n = 0;
        char name[MAX_PATH];
        time_t curr=time(0);
        struct tm *ctm=localtime(&curr);

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
            return;
            break;
        }

        while (n < 100)
        {
            sprintf(name,"%s\\TV%04d%02d%02d%02d%02d%02d%02d.%s",
				    SavingPath,
                    ctm->tm_year+1900,ctm->tm_mon+1,ctm->tm_mday,ctm->tm_hour,ctm->tm_min,ctm->tm_sec,++n, 
                    // name ~ date & time & per-second-counter (for if anyone succeeds in multiple captures per second)
                    // TMYYYYMMDDHHMMSSCC.ext ; eg .\TV2002123123595901.tif
                    extension);

            if (stat(name, &st))
            {
                break;
            }
        }
        if(n == 100) // never reached in 1 second, so could be scrapped
        {
            ErrorBox("Could not create a file. You may have too many captures already.");
            return;
        }

        if(Overlay_Lock(pInfo))
        {
            pStillSource->SaveSnapshot(name, pInfo->FrameHeight, pInfo->FrameWidth, pInfo->Overlay, pInfo->OverlayPitch);
            Overlay_Unlock();
        }
    }
}
