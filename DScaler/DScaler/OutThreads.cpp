/////////////////////////////////////////////////////////////////////////////
// $Id: OutThreads.cpp,v 1.22 2001-07-13 16:14:56 adcockj Exp $
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
// Refminements made by Mark Rejhon and Steve Grimm
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
// 09 Aug 2000   John Adcock           Changed WaitForNextFrame to use current RISC
//                                     pointer rather than the status flag
//                                     Also changed VBI processing 
//
// 17 Sep 2000   Mark Rejhon           Implemented Steve Grimm's changes
//                                     Some cleanup done.
//                                     Made refinements to Steve Grimm's changes
//
// 02 Jan 2001   John Adcock           Fixed bug at end of GetCombFactor assember
//                                     Made PAL pulldown detect remember last video
//                                     Mode
//                                     Removed bTV plug-in
//                                     Added Scaled BOB method
//
// 05 Jan 2001   John Adcock           First attempt at judder fix
//                                     Added loop to make sure that we are never
//                                     too early for a flip
//                                     Changed default for gPulldownMode to 2 frame
//
// 07 Jan 2001   John Adcock           Added Adaptive deinterlacing method
//                                     Split code that did adaptive method
//                                     out of UpdateNTSCPulldownMode
//                                     Added gNTSCFilmFallbackMode setting
//                                     Fixed PAL detection bug
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 09 Jan 2001   John Adcock           Split out into new file
//                                     Changed functions to use DEINTERLACE_INFO
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.21  2001/07/13 07:04:43  adcockj
// Attemp 1 at fixing MSP muting
//
// Revision 1.20  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "OutThreads.h"
#include "Other.h"
#include "BT848.h"
#include "VBI_VideoText.h"
#include "VBI.h"
#include "Deinterlace.h"
#include "AspectRatio.h"
#include "DScaler.h"
#include "DebugLog.h"
#include "VBI.h"
#include "Settings.h"
#include "Filter.h"
#include "Status.h"
#include "FD_60Hz.h"
#include "FD_50Hz.h"
#include "FD_Common.h"
#include "FD_CommonFunctions.h"
#include "CPU.h"
#include "FieldTiming.h"
#include "MixerDev.h"
#include "Audio.h"

// Thread related variables
BOOL                bStopThread = FALSE;
BOOL                bIsPaused = FALSE;
BOOL                bRequestStreamSnap = FALSE;
HANDLE              OutThread;

// Dynamically updated variables
BOOL                bAutoDetectMode = TRUE;


// TRB 10/28/00 changes, parms, and new fields for sync problem fixes
DDSURFACEDESC       SurfaceDesc;                       // also add a surface descriptor for Lock           
HRESULT             FlipResult = 0;             // Need to try again for flip?
BOOL                WaitForFlip = TRUE;       // User parm, default=TRUE
BOOL                DoAccurateFlips = TRUE;     // User parm, default=TRUE
BOOL                bHurryWhenLate = FALSE;    // " , default=FALSE, skip processing if behind
long                RefreshRate = 0;
BOOL bIsOddField = FALSE;
BOOL bWaitForVsync = FALSE;
BOOL bReversePolarity = FALSE;

// FIXME: should be able to get of this variable
long OverlayPitch = 0;

// Statistics
long nTotalDropFields = 0;
double nDropFieldsLastSec = 0;
long nTotalUsedFields = 0;
double nUsedFieldsLastSec = 0;
long nSecTicks = 0;
long nInitialTicks = -1;
long nLastTicks = 0;
long nTotalDeintModeChanges = 0;

// cope with older DX header files
#if !defined(DDFLIP_DONOTWAIT)
    #define DDFLIP_DONOTWAIT 0
#endif

///////////////////////////////////////////////////////////////////////////////
void Start_Thread()
{
    DWORD LinkThreadID;

    // make sure we start with a clean sheet of paper
    Overlay_Clean();

    bStopThread = FALSE;

    OutThread = CreateThread((LPSECURITY_ATTRIBUTES) NULL,  // No security.
                             (DWORD) 0,                     // Same stack size.
                             YUVOutThread,                  // Thread procedure.
                             NULL,                          // Parameter.
                             (DWORD) 0,                     // Start immediatly.
                             (LPDWORD) & LinkThreadID);     // Thread ID.
    if(!bSystemInMute)
    {
        Audio_Unmute();
    }
}

///////////////////////////////////////////////////////////////////////////////
void Stop_Thread()
{
    DWORD ExitCode;
    int i;
    BOOL Thread_Stopped = FALSE;

    if (OutThread != NULL)
    {
        i = 5;
        SetThreadPriority(OutThread, THREAD_PRIORITY_NORMAL);
        bStopThread = TRUE;
        while(i-- > 0 && !Thread_Stopped)
        {
            if (GetExitCodeThread(OutThread, &ExitCode) == TRUE)
            {
                if (ExitCode != STILL_ACTIVE)
                {
                    Thread_Stopped = TRUE;
                }
                else
                {
                    Sleep(100);
                }
            }
            else
            {
                Thread_Stopped = TRUE;
            }
        }

        if (Thread_Stopped == FALSE)
        {
            TerminateThread(OutThread, 0);
            Sleep(100);
        }
        CloseHandle(OutThread);
        OutThread = NULL;
    }
}

void Pause_Capture()
{
    bIsPaused = TRUE;
}

void UnPause_Capture()
{
    bIsPaused = FALSE;
}

void RequestStreamSnap()
{
   bRequestStreamSnap = TRUE;
}


// save the Info structure to a snapshot file
// these files will make it easier to test 
// deinterlacing techniques as we can start
// to exchange the actual data we are each looking
// at and have the ability to recreate results
void SaveStreamSnapshot(DEINTERLACE_INFO* pInfo)
{
    FILE* file;
    char name[13];
    int n = 0;
    int i = 0;
    int j;
    struct stat st;

    while (n < 100)
    {
        sprintf(name,"sn%06d.dtv",++n) ;
        if (stat(name, &st))
            break;
    }

    if(n == 100)
    {
        ErrorBox("Could not create a file.  You may have too many snapshots already.");
        return;
    }

    file = fopen(name,"wb");
    if (!file)
    {
        ErrorBox("Could not open file in SaveStreamSnapshot");
        return;
    }

    // just save the Info struct
    // most of the data is pointers which will be useless
    // to anyone else
    // but NULLs will be useful in determining how many
    // fields we have.
    // The rest will contain all the data we need to use
    // the data in a test program
    fwrite(pInfo, sizeof(DEINTERLACE_INFO), 1, file);

    // save all the Odd fields first
    i = 0;
    while(i < MAX_FIELD_HISTORY && pInfo->OddLines[i] != NULL)
    {
        for(j = 0; j < pInfo->FieldHeight; ++j)
        {
            fwrite(pInfo->OddLines[i][j], pInfo->LineLength, 1, file);
        }
        i++;      
    }

    // then all the even frames
    i = 0;
    while(i < MAX_FIELD_HISTORY && pInfo->EvenLines[i] != NULL)
    {
        for(j = 0; j < pInfo->FieldHeight; ++j)
        {
            fwrite(pInfo->EvenLines[i][j], pInfo->LineLength, 1, file);
        }
    i++;      
    }
    fclose(file);
}

void Pause_Toggle_Capture()
{
    if(bIsPaused)
    {
        UnPause_Capture();
    }
    else
    {
        Pause_Capture();
    }
}

///////////////////////////////////////////////////////////////////////////////
void Start_Capture()
{
    int nFlags = BT848_CAP_CTL_CAPTURE_EVEN | BT848_CAP_CTL_CAPTURE_ODD;
    if (bCaptureVBI == TRUE)
    {
        nFlags |= BT848_CAP_CTL_CAPTURE_VBI_EVEN | BT848_CAP_CTL_CAPTURE_VBI_ODD;
    }

    BT848_MaskDataByte(BT848_CAP_CTL, 0, 0x0f);

    BT848_CreateRiscCode(nFlags);
    BT848_MaskDataByte(BT848_CAP_CTL, (BYTE) nFlags, (BYTE) 0x0f);
    BT848_SetDMA(TRUE);

    // ame sure half height Modes are set correctly
    PrepareDeinterlaceMode();

    Start_Thread();
}

///////////////////////////////////////////////////////////////////////////////
void Stop_Capture()
{
    //  Stop The Output Thread
    Stop_Thread();

    // stop capture
    BT848_MaskDataByte(BT848_CAP_CTL, 0, 0x0f);
}

///////////////////////////////////////////////////////////////////////////////
void Reset_Capture()
{
    Stop_Capture();
    Overlay_Clean();
    BT848_ResetHardware();
    BT848_SetGeoSize();
    WorkoutOverlaySize();
    Start_Capture();
}

//
// Add a function to Lock the overlay surface and update some Info from it.
// We always lock and write to the back buffer.
// Flipping takes care of the proper buffer addresses.
// Some of this Info can change each time.  
// We also check to see if we still need to Flip because the
// non-waiting last flip failed.  If so, try it one more time,
// then give up.  Tom Barry 10/26/00
//
BYTE* LockOverlay()
{
    HRESULT ddrval;
    static DWORD dwFlags = DDLOCK_WAIT | DDLOCK_NOSYSLOCK;

    if (FAILED(FlipResult))             // prev flip was busy?
    {
        ddrval = lpDDOverlay->Flip(NULL, DDFLIP_DONOTWAIT);  
        if(ddrval == DDERR_SURFACELOST)
        {
            return NULL;
        }
        FlipResult = 0;                 // but no time to try any more
    }

    memset(&SurfaceDesc, 0x00, sizeof(SurfaceDesc));
    SurfaceDesc.dwSize = sizeof(SurfaceDesc);
    ddrval = lpDDOverlayBack->Lock(NULL, &SurfaceDesc, dwFlags, NULL);

    // fix suggested by christoph for NT 4.0 sp6
    if(ddrval == E_INVALIDARG && (dwFlags & DDLOCK_NOSYSLOCK))
    {
        //remove flag
        ddrval = lpDDOverlayBack->Lock(NULL, &SurfaceDesc, DDLOCK_WAIT, NULL);
        if( SUCCEEDED(ddrval) )
        {
            //remember for next time
            dwFlags = DDLOCK_WAIT;
        }
    }
    

    if(FAILED(ddrval))
    {
        return NULL;
    }

    OverlayPitch = SurfaceDesc.lPitch;         // Set new pitch, may change
    return (BYTE*)SurfaceDesc.lpSurface;
}

DWORD WINAPI YUVOutThread(LPVOID lpThreadParameter)
{
    char Text[128];
    int i, j;
    int nLineTarget;
    DWORD dwLastSecondTicks;
    short* ppEvenLines[5][DSCALER_MAX_HEIGHT / 2];
    short* ppOddLines[5][DSCALER_MAX_HEIGHT / 2];
    BYTE* pDest;
    BOOL bFlipNow = TRUE;
    HRESULT ddrval;
    DEINTERLACE_INFO Info;
    DWORD FlipFlag;
    DEINTERLACE_METHOD* PrevDeintMethod = NULL;
    DEINTERLACE_METHOD* CurrentMethod = NULL;
    DWORD CurrentTickCount;
    int nHistory = 0;
    BOOL bIsPAL = BT848_GetTVFormat()->Is25fps;
    long SourceAspectAdjust = 1000;

    Timing_Setup();

    // set up Deinterlace Info struct
    memset(&Info, 0, sizeof(Info));
    Info.CpuFeatureFlags = CpuFeatureFlags;
    if(CpuFeatureFlags & FEATURE_SSE)
    {
        Info.pMemcpy = memcpySSE;
    }
    else
    {
        Info.pMemcpy = memcpyMMX;
    }

    // catch anything fatal in this loop so we don't crash the machine
    __try
    {
        if (lpDDOverlay == NULL || lpDDOverlayBack == NULL)
        {
            LOG(" No Overlay surface Created");
            ExitThread(-1);
        }

        // Sets processor Affinity and Thread priority according to menu selection
        SetThreadProcessorAndPriority();

        // Set up 5 sets of pointers to the start of odd and even lines
        for (j = 0; j < 5; j++)
        {
            for (i = 0; i < CurrentY; i += 2)
            {
                ppOddLines[j][i / 2] = (short*) pDisplay[j] + (i + 1) * 1024;
                ppEvenLines[j][i / 2] = (short*) pDisplay[j] + i * 1024;
            }
        }
        PrevDeintMethod = GetCurrentDeintMethod();

        // reset the static variables in the detection code
        if (bIsPAL)
            UpdatePALPulldownMode(NULL);
        else
            UpdateNTSCPulldownMode(NULL);

        // start the capture off
        BT848_Restart_RISC_Code();

        dwLastSecondTicks = GetTickCount();
        while(!bStopThread)
        {
            // update with any changes
            CurrentMethod = GetCurrentDeintMethod();
            Info.bDoAccurateFlips = DoAccurateFlips;
            Info.bRunningLate = bHurryWhenLate;
            Info.bMissedFrame = FALSE;
            
            Timing_WaitForNextField(&Info);
            
            if(bIsPaused == FALSE)
            {
                Info.OverlayPitch = OverlayPitch;
                Info.LineLength = CurrentX * 2;
                Info.FrameWidth = CurrentX;
                Info.FrameHeight = CurrentY;
                Info.FieldHeight = CurrentY / 2;
                Info.CombFactor = -1;
                Info.FieldDiff = -1;
                bFlipNow = FALSE;
                GetDestRect(&Info.DestRect);

                if(Info.IsOdd)
                {
                    memmove(&Info.OddLines[1], &Info.OddLines[0], sizeof(Info.OddLines) - sizeof(Info.OddLines[0]));
                    if(bReversePolarity == FALSE)
                    {
                        Info.OddLines[0] = ppOddLines[Info.CurrentFrame];
                    }
                    else
                    {
                        Info.OddLines[0] = ppEvenLines[Info.CurrentFrame];
                    }
                }
                else
                {
                    memmove(&Info.EvenLines[1], &Info.EvenLines[0], sizeof(Info.EvenLines) - sizeof(Info.EvenLines[0]));
                    if(bReversePolarity == FALSE)
                    {
                        Info.EvenLines[0] = ppEvenLines[Info.CurrentFrame];
                    }
                    else
                    {
                        Info.EvenLines[0] = ppOddLines[(Info.CurrentFrame + 4) % 5];
                    }
                }

                // update the source area
                GetSourceRect(&Info.SourceRect);
                
                // do any filters that operarate on the input
                // only
                SourceAspectAdjust = Filter_DoInput(&Info, (Info.bRunningLate || Info.bMissedFrame));

                if(!Info.bMissedFrame)
                {
                    if(bAutoDetectMode == TRUE)
                    {
                        if(bIsPAL)
                        {
                            // we will need always need both comb and diff
                            // for film detect to work properly
                            PerformFilmDetectCalculations(&Info, TRUE, TRUE);
                            UpdatePALPulldownMode(&Info);
                        }
                        else
                        {
                            // we will need always need diff
                            // comb is needed in film Mode orwhen we ask for it
                            PerformFilmDetectCalculations(&Info, 
                                                        IsFilmMode() ||
                                                            CurrentMethod->bNeedCombFactor,
                                                        TRUE);
                            UpdateNTSCPulldownMode(&Info);
                        }
                        // get the current method again
                        // after the film Modes have been selected
                        CurrentMethod = GetCurrentDeintMethod();
                    }
                    else
                    {
                        PerformFilmDetectCalculations(&Info, 
                                                        CurrentMethod->bNeedCombFactor, 
                                                        CurrentMethod->bNeedFieldDiff);
                    }
                }

                if (bCaptureVBI == TRUE)
                {
                    BYTE* pVBI = (LPBYTE) pVBILines[(Info.CurrentFrame + 4) % 5];
                    if (Info.IsOdd)
                    {
                        pVBI += CurrentVBILines * 2048;
                    }
                    for (nLineTarget = 0; nLineTarget < CurrentVBILines ; nLineTarget++)
                    {
                        VBI_DecodeLine(pVBI + nLineTarget * 2048, nLineTarget, Info.IsOdd);
                    }
                }

                __try
                {
                    if (!Info.bRunningLate)
                    {
                        pDest = LockOverlay();  // Ready to access screen, Lock back buffer berfore accessing
                                                // can't do this until after Lock Call
                        if(pDest == NULL)
                        {
                            PostMessage(hWnd, WM_COMMAND, IDM_OVERLAY_STOP, 0);
                            PostMessage(hWnd, WM_COMMAND, IDM_OVERLAY_START, 0);
                            LOG(" Falling out after LockOverlay");
                            ExitThread(1);
                            return 0;
                        }
                        Info.Overlay = pDest;
                    }

                    if(Info.IsOdd)
                    {
                        if(Info.EvenLines[0] == NULL)
                        {
                            nHistory = 1;
                        }
                        else if(Info.OddLines[1] == NULL)
                        {
                            nHistory = 2;
                        }
                        else if(Info.EvenLines[1] == NULL)
                        {
                            nHistory = 3;
                        }
                        else
                        {
                            nHistory = 4;
                        }
                    }
                    else
                    {
                        if(Info.OddLines[0] == NULL)
                        {
                            nHistory = 1;
                        }
                        else if(Info.EvenLines[1] == NULL)
                        {
                            nHistory = 2;
                        }
                        else if(Info.OddLines[1] == NULL)
                        {
                            nHistory = 3;
                        }
                        else
                        {
                            nHistory = 4;
                        }
                    }

                    if (Info.bRunningLate)
                    {
                        ;     // do nothing
                    }
                    // if we have dropped a field then do BOB 
                    // or if we need to get more history
                    // if we are doing a half height Mode then just do that
                    // anyway as it will be just as fast
                    else if(!CurrentMethod->bIsHalfHeight && (Info.bMissedFrame || nHistory < CurrentMethod->nFieldsRequired))
                    {
                        bFlipNow = Bob(&Info);
                    }
                    else
                    {
                        bFlipNow = CurrentMethod->pfnAlgorithm(&Info);
                    }
                    
                    if (bFlipNow)
                    {
                        // Do any filters that run on the output
                        // need to do this while the surface is locked
                        Filter_DoOutput(&Info, (Info.bRunningLate || Info.bMissedFrame));
                    }

                    AdjustAspectRatio(SourceAspectAdjust, Info.EvenLines[0], Info.OddLines[0]);
                }                   
                // if there is any exception thrown in the above then just carry on
                __except (EXCEPTION_EXECUTE_HANDLER) 
                { 
                    LOG(" Crash in output code");
                }

                if (!Info.bRunningLate)
                {
                    // somewhere above we will have locked the buffer, unlock before flip
                    ddrval = lpDDOverlayBack->Unlock(NULL);
                    if(ddrval == DDERR_SURFACELOST)
                    {
                        PostMessage(hWnd, WM_COMMAND, IDM_OVERLAY_STOP, 0);
                        PostMessage(hWnd, WM_COMMAND, IDM_OVERLAY_START, 0);
                        LOG(" Falling out after Surface Unlock");
                        ExitThread(1);
                        return 0;
                    }

                    if (bFlipNow)
                    {
                        // setup flip flag
                        // the odd and even flags may help the scaled bob
                        // on some cards
                        FlipFlag = (WaitForFlip)?DDFLIP_WAIT:DDFLIP_DONOTWAIT;
                        if(CurrentMethod->nMethodIndex == INDEX_SCALER_BOB)
                        {
                            FlipFlag |= (Info.IsOdd)?DDFLIP_ODD:DDFLIP_EVEN;
                        }

                        // Need to wait for a good time to flip
                        // only if we have been in the same Mode for at least one flip
                        if(Info.bDoAccurateFlips && PrevDeintMethod == CurrentMethod)
                        {
                            Timing_WaitForTimeToFlip(&Info, CurrentMethod, &bStopThread);
                        }

                        FlipResult = lpDDOverlay->Flip(NULL, FlipFlag); 
                        if(FlipResult == DDERR_SURFACELOST)
                        {
                            PostMessage(hWnd, WM_COMMAND, IDM_OVERLAY_STOP, 0);
                            PostMessage(hWnd, WM_COMMAND, IDM_OVERLAY_START, 0);
                            LOG(" Falling out after flip");
                            ExitThread(1);
                            return 0;
                        }
                    }
                }
            }
            
            CurrentTickCount = GetTickCount();
            if (dwLastSecondTicks + 1000 <= CurrentTickCount)
            {
                nTotalDropFields += Timing_GetDroppedFields();
                nTotalUsedFields += Timing_GetUsedFields();
                nDropFieldsLastSec = (double)Timing_GetDroppedFields() * 1000.0 / (double)(CurrentTickCount - dwLastSecondTicks);
                nUsedFieldsLastSec = (double)Timing_GetUsedFields() * 1000.0 / (double)(CurrentTickCount - dwLastSecondTicks);
                Timing_ResetDroppedFields();
                Timing_ResetUsedFields();
                nSecTicks += CurrentTickCount - dwLastSecondTicks;
                dwLastSecondTicks = CurrentTickCount;
                CurrentMethod->ModeTicks += CurrentTickCount - nLastTicks;
                nLastTicks = CurrentTickCount;
                if (IsStatusBarVisible())
                {
                    sprintf(Text, "%d DF/S", (int)ceil(nDropFieldsLastSec - 0.5));
                    
                    //TJ 010508: this will cause YUVOutThread thread to stop
                    //responding if main thread is not processing messages
                    StatusBar_ShowText(STATUS_FPS, Text);
                }
            }

            // if asked save the current Info to a file
            if(bRequestStreamSnap == TRUE)
            {
                SaveStreamSnapshot(&Info);
                bRequestStreamSnap = FALSE;
            }

            // save the last pulldown Mode so that we know if its changed
            PrevDeintMethod = CurrentMethod;
        }

        BT848_SetDMA(FALSE);
    }
    // if there is any exception thrown then exit the thread
    __except (EXCEPTION_EXECUTE_HANDLER) 
    { 
        LOG(" Crash in OutThreads main loop");
        ExitThread(1);
        return 0;
    }
    // end of __try loop
    
    ExitThread(0);
    return 0;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING OutThreadsSettings[OUTTHREADS_SETTING_LASTONE] =
{
    {
        "Hurry When Late", ONOFF, 0, (long*)&bHurryWhenLate,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Threads", "bHurryWhenLate", NULL,
    },
    {
        "Wait For Flip", ONOFF, 0, (long*)&WaitForFlip,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Threads", "WaitForFlip", NULL,
    },
    {
        "JudderTerminator", ONOFF, 0, (long*)&DoAccurateFlips,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Threads", "DoAccurateFlips", NULL,
    },
    {
        "Autodetect Pulldown", ONOFF, 0, (long*)&bAutoDetectMode,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Pulldown", "bAutoDetectMode", NULL,
    },
    {
        "Refresh Rate", SLIDER, 0, (long*)&RefreshRate,
        0, 0, 120, 1, 1,
        NULL,
        "Pulldown", "RefreshRate", NULL,
    },
    {
        "Wait For VSync", ONOFF, 0, (long*)&bWaitForVsync,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Threads", "bWaitForVsync", NULL,
    },
    {
        "Reverse Polarity", ONOFF, 0, (long*)&bReversePolarity,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Threads", "bReversePolarity", NULL,
    },
};

SETTING* OutThreads_GetSetting(OUTTHREADS_SETTING Setting)
{
    if(Setting > -1 && Setting < OUTTHREADS_SETTING_LASTONE)
    {
        return &(OutThreadsSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void OutThreads_ReadSettingsFromIni()
{
    int i;
    for(i = 0; i < OUTTHREADS_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(OutThreadsSettings[i]));
    }
}

void OutThreads_WriteSettingsToIni()
{
    int i;
    for(i = 0; i < OUTTHREADS_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(OutThreadsSettings[i]));
    }
}

void OutThreads_SetMenu(HMENU hMenu)
{
    CheckMenuItemBool(hMenu, IDM_CAPTURE_PAUSE, bIsPaused);
    CheckMenuItemBool(hMenu, IDM_AUTODETECT, bAutoDetectMode);
    CheckMenuItemBool(hMenu, IDM_JUDDERTERMINATOR, DoAccurateFlips);
}
