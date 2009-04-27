/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file MixerDev.cpp Mixer Classes
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "MixerDev.h"
#include "DScaler.h"
#include "Audio.h"
#include "Providers.h"
#include "SettingsMaster.h"

using namespace std;

#define MAX_SUPPORTED_INPUTS  6                     // Not readily changable


//  MixerDev prototypes and globals

void MixerDev_UpdateSettings(CSource* pSource);

static BOOL g_bMixerDevInvalidSection = TRUE;

SmartPtr<CSettingsHolder> MixerSettings;

//  Mixer typedefs

typedef CMixer* (tSyncChangesCallback)(void* pContext);


//  Mixer prototypes

static void     Mixer_SetCurrentMixerFromName();
static void     Mixer_SetCurrentMixer(long nMixerIndex);

static CMixerLineDst*   Mixer_GetCurrentDestination();
static CMixerLineSrc*   Mixer_GetCurrentActiveSource();

static void     Mixer_EventOnChangeNotification(void*, CEventObject*, eEventType, long, long, eEventType*);

static void     Mixer_OnInputChange(long nVideoInput);
static void     Mixer_OnSourceChange(CSource *pSource);

static CMixer*  LoadSourceSettingsCallback(void* pContext);

static void     Mixer_SetupActiveInputs(CSource* pCurrentSource);

static void     Mixer_MuteInputs(long nDestinationIndex, long* pIndexes, long nCount, BOOL bMute);
static void     Mixer_StoreRestoreInputs(long nDestinationIndex, long* pIndexes, long nCount, BOOL bRestore);

static void     Mixer_DoSettingsTransition(tSyncChangesCallback* pSyncfunc, void* pContext);

static long     Mixer_NameToIndex(char* szName);
static CMixer*  Mixer_Allocate(long nMixerIndex);

BOOL APIENTRY MixerSetupProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);


//  Utility prototypes

static long     LongArrayUnique(const long*, long, long*);
static void     LongArraySubstract(long*, long*, const long*, long);
static void     LongArrayDivide(long*, long *, long*, long *);


// Global variables

static CMixer*  g_pCurrentMixer = NULL;
static long     g_nActiveInputsCount = 0;
static long     g_nActiveInput = -1;

static BOOL     g_bUseMixer = FALSE;                        // Saved setting variable
static BOOL     g_bResetOnExit = FALSE;                     // Saved setting variable
static BOOL     g_bNoHardwareMute = FALSE;                  // Saved setting variable

SettingStringValue g_pMixerName;                            // Saved setting variable
                                                            // - mem-managed by Setting_Funcs

static long     g_nDestinationIndex;                        // Saved setting variable
static long     g_nSourceIndexes[MAX_SUPPORTED_INPUTS];     // Saved setting variable


//----------------------------------------------------------------------
//  Public functions
//----------------------------------------------------------------------

void Mixer_Init()
{
    eEventType EventList[] = { EVENT_SOURCE_CHANGE,
                               EVENT_VIDEOINPUT_CHANGE,
                               EVENT_NONE };

    EventCollector->Register(Mixer_EventOnChangeNotification, NULL, EventList);

    // This call takes care of all necessary initializations
    Mixer_OnSourceChange(Providers_GetCurrentSource());
}


void Mixer_Exit()
{
    EventCollector->Unregister(Mixer_EventOnChangeNotification, NULL);

    // This call takes care of all necessary changes
    Mixer_OnSourceChange(NULL);
}


BOOL Mixer_IsEnabled()
{
    return g_bUseMixer;
}


BOOL Mixer_IsNoHardwareMute()
{
    return g_bUseMixer && g_bNoHardwareMute;
}


void Mixer_SetMute(BOOL bEnabled)
{
    CMixerLineSrc* pLineSrc = Mixer_GetCurrentActiveSource();

    if (pLineSrc != NULL)
    {
        pLineSrc->SetMute(bEnabled);
    }
}


BOOL Mixer_GetMute(void)
{
    CMixerLineSrc* pLineSrc = Mixer_GetCurrentActiveSource();

    if (pLineSrc != NULL)
    {
        return pLineSrc->GetMute() != FALSE;
    }

    return -1;
}


void Mixer_Mute()
{
    Mixer_SetMute(TRUE);
}


void Mixer_UnMute()
{
    Mixer_SetMute(FALSE);
}


void Mixer_SetVolume(long newVolume)
{
    CMixerLineSrc* pLineSrc = Mixer_GetCurrentActiveSource();

    if (pLineSrc != NULL)
    {
        long oldVolume = pLineSrc->GetVolume();

        if (newVolume < 0)
        {
            newVolume = 0;
        }
        else if (newVolume > 100)
        {
            newVolume = 100;
        }

        pLineSrc->SetVolume(newVolume);

        EventCollector->RaiseEvent(NULL, EVENT_MIXERVOLUME, oldVolume, newVolume);
    }
}


long Mixer_GetVolume()
{
    CMixerLineSrc* pLineSrc = Mixer_GetCurrentActiveSource();

    if (pLineSrc != NULL)
    {
        return pLineSrc->GetVolume();
    }

    return -1;
}


void Mixer_AdjustVolume(long delta)
{
    CMixerLineSrc* pLineSrc = Mixer_GetCurrentActiveSource();

    if (pLineSrc != NULL)
    {
        long oldVolume = pLineSrc->GetVolume();
        long newVolume = oldVolume + delta;

        if (newVolume < 0)
        {
            newVolume = 0;
        }
        else if (newVolume > 100)
        {
            newVolume = 100;
        }

        pLineSrc->SetVolume(newVolume);

        EventCollector->RaiseEvent(NULL, EVENT_MIXERVOLUME, oldVolume, newVolume);
    }
}


void Mixer_Volume_Up()
{
    Mixer_AdjustVolume(5);
}


void Mixer_Volume_Down()
{
    Mixer_AdjustVolume(-5);
}


void Mixer_SetupDlg(HWND hWndParent)
{
    CSource* source = Providers_GetCurrentSource();
    if ((source == NULL) || !source->IsAudioMixerAccessAllowed())
    {
        MessageBox(GetMainWnd(), "No audio mixer setup needed for the current source", "DScaler Warning", MB_OK);
    }
    else
    {
        CMixerFinder mixerFinder;

        if (mixerFinder.GetMixerCount() > 0)
        {
            BOOL bWasInvalidSection = g_bMixerDevInvalidSection;

            // This Mixer_SetupDlg(...) function can be called in the early stages of
            // configuration, before a "current source" is properly set from a raised
            // EVENT_SOURCE_CHANGE event.  As such, g_bMixerDevInvalidSection can be
            // TRUE to indicate that this file is working with a "null-source".  If
            // this is the case, temporary set the "current source" based on the return
            // value of 'source = Providers_GetCurrentSource()'.  (Because there is no
            // point for the user to configure the null-source.)
            if (bWasInvalidSection)
            {
                // Change the mixer settings to the given source.
                Mixer_OnSourceChange(source);
            }

            DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_MIXERSETUP), hWndParent, MixerSetupProc);

            if (source->GetVolume() != NULL)
            {
                EventCollector->RaiseEvent(source, EVENT_VOLUME, 0, source->GetVolume()->GetValue());
            }
            else
            {
                EventCollector->RaiseEvent(source, EVENT_NO_VOLUME, 0, 1);
            }

            if (bWasInvalidSection)
            {
                // Restore the mixer source to the null-source.
                Mixer_OnSourceChange(NULL);
            }
        }
        else
        {
            MessageBox(GetMainWnd(), "No mixer hardware found", "DScaler Error", MB_OK);
        }
    }
}


//----------------------------------------------------------------------
//  Internal functions
//----------------------------------------------------------------------

static void Mixer_SetCurrentMixerFromName()
{
    Mixer_SetCurrentMixer(Mixer_NameToIndex(g_pMixerName));
}


static void Mixer_SetCurrentMixer(long nMixerIndex)
{
    if (g_pCurrentMixer != NULL)
    {
        if (g_pCurrentMixer->GetIndex() == nMixerIndex)
        {
            return;
        }

        delete g_pCurrentMixer;
        g_pCurrentMixer = NULL;
    }

    g_pCurrentMixer = Mixer_Allocate(nMixerIndex);
}


static CMixerLineDst* Mixer_GetCurrentDestination()
{
    if (g_bUseMixer && g_pCurrentMixer != NULL && g_nDestinationIndex != -1)
    {
        return g_pCurrentMixer->GetDestinationLine(g_nDestinationIndex);
    }

    return NULL;
}


static CMixerLineSrc* Mixer_GetCurrentActiveSource()
{
    if (g_nActiveInput == -1 || g_nSourceIndexes[g_nActiveInput] == -1)
    {
        return NULL;
    }

    CMixerLineDst* pLineDst = Mixer_GetCurrentDestination();

    if (pLineDst != NULL)
    {
        return pLineDst->GetSourceLine(g_nSourceIndexes[g_nActiveInput]);
    }

    return NULL;
}


static void Mixer_EventOnChangeNotification(void*, CEventObject*, eEventType EventType,
                                            long OldValue, long NewValue, eEventType*)
{
    switch (EventType)
    {
    case EVENT_SOURCE_CHANGE:
        Mixer_OnSourceChange((CSource*)NewValue);
        break;

    case EVENT_VIDEOINPUT_CHANGE:
        Mixer_OnInputChange(NewValue);
        break;

    default:
        break;
    }
}


static void Mixer_OnInputChange(long nVideoInput)
{
    if (nVideoInput < 0 || nVideoInput >= g_nActiveInputsCount)
    {
        g_nActiveInput = -1;
    }
    else
    {
        g_nActiveInput = nVideoInput;
    }

    CMixerLineDst* pLineDst = Mixer_GetCurrentDestination();

    if (pLineDst == NULL)
    {
        return;
    }

    long nActiveSourceIndex = -1;

    if (g_nActiveInput != -1)
    {
        nActiveSourceIndex = g_nSourceIndexes[g_nActiveInput];
    }

    long nUniqueSources[MAX_SUPPORTED_INPUTS];
    long nUniqueSourcesCount;

    nUniqueSourcesCount = LongArrayUnique(g_nSourceIndexes, g_nActiveInputsCount, nUniqueSources);
    LongArraySubstract(nUniqueSources, &nUniqueSourcesCount, &nActiveSourceIndex, 1);

    // Mute mixer lines for inactive lines
    Mixer_MuteInputs(g_nDestinationIndex, nUniqueSources, nUniqueSourcesCount, TRUE);

    if (nActiveSourceIndex != -1)
    {
        CMixerLineSrc* pLineSrc = pLineDst->GetSourceLine(nActiveSourceIndex);

        if (pLineSrc != NULL)
        {
            pLineSrc->SetMute(Audio_IsMute());

            // We do not store volume values
            // pLineSrc->SetVolume(Volume);

            EventCollector->RaiseEvent(NULL, EVENT_MIXERVOLUME, -1, pLineSrc->GetVolume());
        }
    }
}


// The mixer code in this file generally works with a single set of
// mixer settings.  However, there're multiple CSources in DScaler,
// each which need their own audio mixer setting.  This function
// Mixer_OnSourceChange(..) is responsible for switching the settings
// around for different sources.  When the settings are is switched
// for a specific CSource, all calls in this file, including those
// such as SettingsMaster->SaveAllSettings and ReadSettingsFromIni, will work
// exclusively for the settings of that CSource only.
//
// The special call Mixer_OnSourceChange(NULL) is used to set the mode
// to a null-source.  Mixer settings for this null-source are neither
// read or written.  This null-source is used to close all audio lines.
static void Mixer_OnSourceChange(CSource *pSource)
{
    Mixer_DoSettingsTransition(LoadSourceSettingsCallback, pSource);
}

static CMixer* LoadSourceSettingsCallback(void* pContext)
{
    CSource* pSource = (CSource*)pContext;

    // DoSettingsTranstion wants everything changed to the new
    // settings, except g_pCurrentMixer.  Instead of changing
    // g_pCurrentMixer, the new mixer should be returned.

    // Load the new source's settings
    MixerDev_UpdateSettings(pSource);

    Mixer_SetupActiveInputs(pSource);

    if (g_bUseMixer)
    {
        long nMixerIndex = Mixer_NameToIndex(g_pMixerName);

        if (g_pCurrentMixer != NULL &&
            g_pCurrentMixer->GetIndex() == nMixerIndex)
        {
            return g_pCurrentMixer;
        }

        return Mixer_Allocate(nMixerIndex);
    }

    return NULL;
}


static void Mixer_SetupActiveInputs(CSource* pCurrentSource)
{
    if (pCurrentSource != NULL)
    {
        g_nActiveInputsCount = pCurrentSource->NumInputs(VIDEOINPUT);
        g_nActiveInput = pCurrentSource->GetInput(VIDEOINPUT);

        if (g_nActiveInputsCount > MAX_SUPPORTED_INPUTS)
        {
            g_nActiveInputsCount = MAX_SUPPORTED_INPUTS;
        }

        if (g_nActiveInput < -1 || g_nActiveInput >= g_nActiveInputsCount)
        {
            g_nActiveInput = -1;
        }
    }
    else
    {
        g_nActiveInputsCount = 0;
        g_nActiveInput = -1;
    }
}


static void Mixer_MuteInputs(long nDestinationIndex, long* pIndexes, long nCount, BOOL bMute)
{
    CMixerLineDst* pLineDst;
    CMixerLineSrc* pLineSrc;

    if (g_pCurrentMixer == NULL || nDestinationIndex == -1 || nCount == 0)
    {
        return;
    }

    if ((pLineDst = g_pCurrentMixer->GetDestinationLine(nDestinationIndex)) == NULL)
    {
        return;
    }

    for (int i = 0; i < nCount; i++)
    {
        if (pIndexes[i] == -1)
        {
            continue;
        }

        if ((pLineSrc = pLineDst->GetSourceLine(pIndexes[i])) != NULL)
        {
            pLineSrc->SetMute(bMute);
        }
    }
}


static void Mixer_StoreRestoreInputs(long nDestinationIndex, long* pIndexes, long nCount, BOOL bRestore)
{
    CMixerLineDst* pLineDst;
    CMixerLineSrc* pLineSrc;

    if (g_pCurrentMixer == NULL || nDestinationIndex == -1 || nCount == 0)
    {
        return;
    }

    if ((pLineDst = g_pCurrentMixer->GetDestinationLine(nDestinationIndex)) == NULL)
    {
        return;
    }

    for (int i = 0; i < nCount; i++)
    {
        if (pIndexes[i] == -1)
        {
            continue;
        }

        if ((pLineSrc = pLineDst->GetSourceLine(pIndexes[i])) != NULL)
        {
            if (bRestore)
            {
                pLineSrc->RestoreState();
            }
            else
            {
                pLineSrc->StoreState();
            }
        }
    }
}


// This function handles what happens when audio line selections change.
//
// Background: A "Source" in DScaler is CSource object.  One instance of CSource
// is used for each one of these:
//   - TV card interfaced by DScaler's BT8x8, SAA713x or CX233x bridges.
//   - TV card or other hardware interfaced using DirectShow (WDM)
//   - One of the miscellaneous sources such as Patterns of video playback.
//
// To avoid confusion, these type of "Sources" will be referred from here as "CSources".
//
// Example: If I have a "FlyVideo SAA7134" card and a "Medion SAA7134" card,
// I may have all up five CSources:
//   - An SAA7134 object for the "FlyVideo SAA7134" card.
//   - An SAA7134 object for the "Medion SAA7134" card.
//   - A DirectShow object for the "FlyVideo SAA7134" card.
//   - A DirectShow object for the "Medion SAA7134" card.
//   - A Patterns object for display pictures.
//
// Each one of the CSources has its own "Audio Mixer" setting.  An audio mixer
// setting consists of:
//   - Whether it is used or not.
//   - Which sound device to use for incoming audio.
//   - Which sound device "destination" to use for incoming audio.
//   - Which Audio Lines (aka Audio Source) to use for incoming audio.
//   - Whether or not to "disable the use of hardware mute".
//   - Whether to "reset mixer settings on exit".
//
// The "Audio Line" are lines such as "Line-In", "Microphone", "CD Audio",
// that are provided by the system for each sound device -> destination.
//
// Each sound device (a.k.a "mixer") its own set of "destinations" (incoming
// audio is also considered a "destination") and each "destination" has its
// own set of audio lines:
//
// Sound Device (CMixer)
//   -> Destination ("Playback" / "Recording")
//     -> Audio Lines ("Line In", "Microphone", etc).
//
// Description:
// The purpose of this function is so that when the "Audio Mixer" setting
// changes (either because CSource changed or because the user reconfigured),
// all newly required Audio Lines are prepared, and all no longer needed
// Audio Lines are closed.
//
// Example: If my FlyVideo SAA7134 card is configured to use "Line-In" of
// mixer_1 and my Medion SAA7134 card is configured to use "Auxiliary" of
// mixer_1, and the current CSource changes from the FlyVideo to Medion,
// this function needs to close the "Line-In" line and prepare the
// "Auxiliary" line.
//
// If on the other hand, the Medion card is configured to use the same
// "Line-In" line on mixer_1, no preparation or closing is done.
//
// An Audio Line is "prepared" by creating the necessary programming objects.
// It is "closed" by either muting the line, or, if "restore mixer settings
// on exit" is set, restoring the mute state to the previous state, then
// releasing the line's resources.
//
// This function does not perform the Audio Mixer changes itself.  It calls
// 'pSyncfunc' whose purpose is to make the necessary changes for the occasion.
// (i.e. CSource changing or the user reconfiguring)  The function saves all
// settings before calling 'pSyncfunc' then compares to determine which lines
// are new and which are no longer needed.
static void Mixer_DoSettingsTransition(tSyncChangesCallback* pSyncfunc, void* pContext)
{
    long nOldDestination = -1;
    long nOldSources[MAX_SUPPORTED_INPUTS];
    long nOldSourcesCount = 0;
    long nNewSources[MAX_SUPPORTED_INPUTS];
    long nNewSourcesCount = 0;

    // Hold onto the old mixer info
    if (g_bUseMixer && g_pCurrentMixer != NULL && g_nDestinationIndex != -1)
    {
        nOldDestination = g_nDestinationIndex;
        nOldSourcesCount = LongArrayUnique(g_nSourceIndexes, g_nActiveInputsCount, nOldSources);
    }

    // Synchronize all new settings except g_pCurrentMixer
    CMixer* pNewMixer = (pSyncfunc)(pContext);

    // Check the new mixer info
    if (g_bUseMixer && pNewMixer != NULL && g_nDestinationIndex != -1)
    {
        nNewSourcesCount = LongArrayUnique(g_nSourceIndexes, g_nActiveInputsCount, nNewSources);

        // Compare the old with the new mixer info
        if (pNewMixer == g_pCurrentMixer && g_nDestinationIndex == nOldDestination)
        {
            LongArrayDivide(nOldSources, &nOldSourcesCount, nNewSources, &nNewSourcesCount);
        }
    }

    // Reset the no longer used inputs
    if (g_bResetOnExit)
    {
        Mixer_StoreRestoreInputs(nOldDestination, nOldSources, nOldSourcesCount, TRUE);
    }
    else
    {
        Mixer_MuteInputs(nOldDestination, nOldSources, nOldSourcesCount, TRUE);
    }

    // Synchronize g_pCurrentMixer
    if (g_pCurrentMixer != pNewMixer)
    {
        if (g_pCurrentMixer != NULL)
        {
            delete g_pCurrentMixer;
        }

        g_pCurrentMixer = pNewMixer;
    }

    // Setup the new inputs
    if (g_bUseMixer)
    {
        Mixer_StoreRestoreInputs(g_nDestinationIndex, nNewSources, nNewSourcesCount, FALSE);
        Mixer_OnInputChange(g_nActiveInput);
    }
}


static long Mixer_NameToIndex(char* szName)
{
    long nMixerIndex = -1;

    if (szName != NULL)
    {
        CMixerFinder mixerFinder;
        nMixerIndex = mixerFinder.FindMixer(szName);
    }

    return nMixerIndex;
}


static CMixer* Mixer_Allocate(long nMixerIndex)
{
    CMixer* pMixer = NULL;

    if (nMixerIndex != -1)
    {
        try
        {
            pMixer = new CMixer(nMixerIndex);
        }
        catch (...)
        {
            // do nothing
        }
    }

    return pMixer;
}


//----------------------------------------------------------------------
//  APIENTRY MixerSetupProc
//----------------------------------------------------------------------

static void InitAndNameActiveInputs(HWND hDlg);

static void AdjustSameForAllInputsBox(HWND hDlg);
static void SwitchSameForAllInputsMode(HWND hDlg, int iOneSameZeroNot);
static void EnableDisableMixerControls(HWND hDlg, BOOL bEnable);

static void RefillMixerDeviceBox(HWND hDlg, long nSelectIndex);
static void RefillDestinationBox(HWND hDlg, long nSelectIndex);
static void RefillSourceBox(HWND hDlg, long nSourceControlId, long nSelectIndex);

static CMixer* SynchronizeDlgChangesCallback(void* pContext);
static int ComboBox_GetCurSelItemData(HWND hControl);

static CMixer*  g_pDlgActiveMixer = NULL;


BOOL APIENTRY MixerSetupProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    int i;

    switch (message)
    {
    case WM_INITDIALOG:
        InitAndNameActiveInputs(hDlg);

        // Adjust the check boxes
        Button_SetCheck(GetDlgItem(hDlg, IDC_USE_MIXER), g_bUseMixer);
        Button_SetCheck(GetDlgItem(hDlg, IDC_DISABLE_HW_MUTE), g_bNoHardwareMute);
        Button_SetCheck(GetDlgItem(hDlg, IDC_RESETONEXIT), g_bResetOnExit);

        if (!g_bUseMixer)
        {
            // Create a temporally mixer as per user settings
            Mixer_SetCurrentMixerFromName();
        }

        if (g_pCurrentMixer != NULL)
        {
            RefillMixerDeviceBox(hDlg, g_pCurrentMixer->GetIndex());
        }
        else
        {
            RefillMixerDeviceBox(hDlg, -1);
        }

        MixerSetupProc(hDlg, WM_COMMAND, MAKEWPARAM(IDC_MIXER, CBN_SELCHANGE), 0L);

        AdjustSameForAllInputsBox(hDlg);

        // Enable all the fields that should be enabled
        EnableDisableMixerControls(hDlg, g_bUseMixer);

        SetFocus(GetDlgItem(hDlg, IDC_USE_MIXER));

        return FALSE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_USE_MIXER:
            if (Button_GetCheck(GetDlgItem(hDlg, IDC_USE_MIXER)) == BST_CHECKED)
            {
                EnableDisableMixerControls(hDlg, TRUE);
            }
            else
            {
                EnableDisableMixerControls(hDlg, FALSE);
            }
            break;

        case IDC_MIXER:
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                int nIndex = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_MIXER));

                // Delete of the old mixer
                if (g_pDlgActiveMixer != NULL)
                {
                    if (g_pDlgActiveMixer->GetIndex() == nIndex)
                    {
                        // Nothing to do
                        break;
                    }

                    // Make sure it's our own
                    if (g_pDlgActiveMixer != g_pCurrentMixer)
                    {
                        delete g_pDlgActiveMixer;
                    }

                    g_pDlgActiveMixer = NULL;
                }

                // Create the new mixer
                if (nIndex != -1)
                {
                    if (g_pCurrentMixer != NULL && g_pCurrentMixer->GetIndex() == nIndex)
                    {
                        // Just reference the current mixer
                        g_pDlgActiveMixer = g_pCurrentMixer;
                    }
                    else
                    {
                        try
                        {
                            g_pDlgActiveMixer = new CMixer(nIndex);
                        }
                        catch (...)
                        {
                            nIndex = -1;
                        }
                    }
                }

                // Update the destination box
                if (nIndex != -1)
                {
                    if (g_pDlgActiveMixer == g_pCurrentMixer)
                    {
                        RefillDestinationBox(hDlg, g_nDestinationIndex);
                    }
                    else
                    {
                        RefillDestinationBox(hDlg, -1);
                    }
                }
                else
                {
                    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_DEST));
                }

                MixerSetupProc(hDlg, WM_COMMAND, MAKEWPARAM(IDC_DEST, CBN_SELCHANGE), 0);
            }
            break;

        case IDC_DEST:
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                int nIndex = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_DEST));

                // Update the source boxes
                if (nIndex != -1 && g_pDlgActiveMixer != NULL)
                {
                    if (g_pDlgActiveMixer == g_pCurrentMixer && nIndex == g_nDestinationIndex)
                    {
                        for (i = 0; i < g_nActiveInputsCount; i++)
                        {
                            RefillSourceBox(hDlg, IDC_MIXER_INPUT0+i, g_nSourceIndexes[i]);
                        }
                    }
                    else
                    {
                        for (i = 0; i < g_nActiveInputsCount; i++)
                        {
                            RefillSourceBox(hDlg, IDC_MIXER_INPUT0+i, -1);
                        }
                    }
                }
                else
                {
                    for (i = 0; i < g_nActiveInputsCount; i++)
                    {
                        ComboBox_ResetContent(GetDlgItem(hDlg, IDC_MIXER_INPUT0+i));
                    }
                }
            }
            break;

        case IDC_MIXER_INPUT_ALL:
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                int index = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_MIXER_INPUT_ALL));

                for (i = 0; i < g_nActiveInputsCount; i++)
                {
                    RefillSourceBox(hDlg, IDC_MIXER_INPUT0+i, index);
                }
            }
            break;

        case IDC_SEPARATE_INPUT:
            SwitchSameForAllInputsMode(hDlg,
                Button_GetCheck(GetDlgItem(hDlg, IDC_SEPARATE_INPUT)) != BST_CHECKED);
            break;

        case IDOK:
            // Use the new g_bResetOnExit setting in Mixer_DoSettingsTransition()
            g_bResetOnExit =
                Button_GetCheck(GetDlgItem(hDlg, IDC_RESETONEXIT)) == BST_CHECKED;

            Mixer_DoSettingsTransition(SynchronizeDlgChangesCallback, hDlg);
            SettingsMaster->SaveAllSettings(TRUE);

            // FALLTHROUGH

        case IDCANCEL:
            if (g_pDlgActiveMixer != NULL)
            {
                if (g_pDlgActiveMixer != g_pCurrentMixer)
                {
                    delete g_pDlgActiveMixer;
                }
                g_pDlgActiveMixer = NULL;
            }

            if (!g_bUseMixer)
            {
                Mixer_SetCurrentMixer(-1);
            }

            EndDialog(hDlg, 0);
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }

    return FALSE;
}


static void InitAndNameActiveInputs(HWND hDlg)
{
    int     nInputCount = 0;
    string  sInputName;
    CSource* pSource;
    int     i;

    pSource = Providers_GetCurrentSource();

    if (pSource != NULL)
    {
        // fudge so that this works before the source change stuff has been setup
        Mixer_SetupActiveInputs(pSource);
        nInputCount = g_nActiveInputsCount;
    }

    // Set the source line names
    for (i = 0; i < nInputCount; i++)
    {
        string pName = pSource->GetInputName(VIDEOINPUT, i);
        if (pName.empty())
        {
            sInputName = "Input ";
            sInputName += i;
        }
        else
        {
            sInputName = pName;
        }

        SetDlgItemText(hDlg, IDC_MIXER_INPUT0NAME+i, sInputName.c_str());

        ShowWindow(GetDlgItem(hDlg, IDC_MIXER_INPUT0NAME+i), SW_SHOW);
        ShowWindow(GetDlgItem(hDlg, IDC_MIXER_INPUT0+i), SW_SHOW);

        LONG lStyle = GetWindowLong(GetDlgItem(hDlg, IDC_MIXER_INPUT0NAME+i), GWL_STYLE);

        // There are better ways of testing if the
        // text wraps but this way is probably okay
        if (sInputName.length() > 16)
        {
            // Unset center vertically so the text
            // wraps properly
            lStyle &= ~SS_CENTERIMAGE;
        }
        else
        {
            lStyle |= SS_CENTERIMAGE;
        }

        SetWindowLong(GetDlgItem(hDlg, IDC_MIXER_INPUT0NAME+i), GWL_STYLE, lStyle);
    }

    // Hide the unused source lines
    for ( ; i < MAX_SUPPORTED_INPUTS; i++)
    {
        ShowWindow(GetDlgItem(hDlg, IDC_MIXER_INPUT0NAME+i), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_MIXER_INPUT0+i), SW_HIDE);
    }

    if (nInputCount == 0)
    {
        ShowWindow(GetDlgItem(hDlg, IDC_MIXER_INPUTNAME_ALL), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_MIXER_INPUT_ALL), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_SEPARATE_INPUT), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_SOURCE_NO_INPUT), SW_SHOW);
    }
    else
    {
        ShowWindow(GetDlgItem(hDlg, IDC_MIXER_INPUTNAME_ALL), SW_SHOW);
        ShowWindow(GetDlgItem(hDlg, IDC_MIXER_INPUT_ALL), SW_SHOW);
        ShowWindow(GetDlgItem(hDlg, IDC_SEPARATE_INPUT), SW_SHOW);
        ShowWindow(GetDlgItem(hDlg, IDC_SOURCE_NO_INPUT), SW_HIDE);
    }
}


static void AdjustSameForAllInputsBox(HWND hDlg)
{
    int nFirstIndex;
    int nIndex;
    int i;

    if (g_nActiveInputsCount == 0)
    {
        return;
    }

    nFirstIndex = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_MIXER_INPUT0));

    for (i = 1; i < g_nActiveInputsCount; i++)
    {
        nIndex = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_MIXER_INPUT0+i));

        if (nIndex != nFirstIndex)
        {
            break;
        }
    }

    if (i == g_nActiveInputsCount)
    {
        if (nFirstIndex == -1)
        {
            nFirstIndex = -1;
        }

        RefillSourceBox(hDlg, IDC_MIXER_INPUT_ALL, nFirstIndex);
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEPARATE_INPUT), FALSE);
    }
    else
    {
        RefillSourceBox(hDlg, IDC_MIXER_INPUT_ALL, -1);
        Button_SetCheck(GetDlgItem(hDlg, IDC_SEPARATE_INPUT), TRUE);
    }
}


static void SwitchSameForAllInputsMode(HWND hDlg, int iOneSameZeroNot)
{
    EnableWindow(GetDlgItem(hDlg, IDC_MIXER_INPUT_ALL), iOneSameZeroNot == 1);
    EnableWindow(GetDlgItem(hDlg, IDC_MIXER_INPUTNAME_ALL), iOneSameZeroNot == 1);

    for (int i = 0; i < g_nActiveInputsCount; i++)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_MIXER_INPUT0+i), iOneSameZeroNot == 0);
        EnableWindow(GetDlgItem(hDlg, IDC_MIXER_INPUT0NAME+i), iOneSameZeroNot == 0);
    }
}


static void EnableDisableMixerControls(HWND hDlg, BOOL bEnable)
{
    EnableWindow(GetDlgItem(hDlg, IDC_MIXER), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_DEST), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_DEST_STATIC), bEnable);

    EnableWindow(GetDlgItem(hDlg, IDC_SEPARATE_INPUT), bEnable);

    SwitchSameForAllInputsMode(hDlg, bEnable == FALSE ? -1 :
        Button_GetCheck(GetDlgItem(hDlg, IDC_SEPARATE_INPUT)) != BST_CHECKED);

    EnableWindow(GetDlgItem(hDlg, IDC_DISABLE_HW_MUTE), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_RESETONEXIT), bEnable);
}


static void RefillMixerDeviceBox(HWND hDlg, long nSelectIndex)
{
    CMixerFinder mixerFinder;
    char buffer[MAXPNAMELEN];

    HWND hMixerControl = GetDlgItem(hDlg, IDC_MIXER);

    ComboBox_ResetContent(hMixerControl);

    int mixerCount = mixerFinder.GetMixerCount();
    BOOL bSelected = FALSE;
    int index;

    for (int i = 0; i < mixerCount; i++)
    {
        mixerFinder.GetMixerName(i, buffer);

        index = ComboBox_AddString(hMixerControl, buffer);
        ComboBox_SetItemData(hMixerControl, index, i);

        if (i == nSelectIndex)
        {
            ComboBox_SetCurSel(hMixerControl, index);
            bSelected = TRUE;
        }
    }

    if (!bSelected)
    {
        ComboBox_SetCurSel(hMixerControl, 0);
    }
}


static void RefillDestinationBox(HWND hDlg, long nSelectIndex)
{
    HWND hDestinationControl = GetDlgItem(hDlg, IDC_DEST);

    ComboBox_ResetContent(hDestinationControl);

    if (g_pDlgActiveMixer == NULL)
    {
        return;
    }

    int destinationCount = g_pDlgActiveMixer->GetDestinationCount();
    int recordingLines = 0;

    const char* pValidName = "";
    int bSelected = FALSE;
    int index;

    for (int i = 0; i < destinationCount; i++)
    {
        CMixerLineDst* pLineDst = g_pDlgActiveMixer->GetDestinationLine(i);

        if (pLineDst->IsTypicalRecordingLine() &&
            destinationCount - recordingLines > 1)
        {
            recordingLines++;
        }
        else
        {
            pValidName = pLineDst->GetName();

            index = ComboBox_AddString(hDestinationControl, pValidName);
            ComboBox_SetItemData(hDestinationControl, index, i);

            if (i == nSelectIndex)
            {
                ComboBox_SetCurSel(hDestinationControl, index);
                bSelected = TRUE;
            }
        }
    }

    if (!bSelected)
    {
        ComboBox_SetCurSel(hDestinationControl, 0);
    }

    // Hide the destination field if there is only one destination
    if(destinationCount - recordingLines == 1)
    {
        SetDlgItemText(hDlg, IDC_DEST_STATIC, pValidName);
        ShowWindow(hDestinationControl, SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_DEST_STATIC), SW_SHOW);
    }
    else
    {
        ShowWindow(GetDlgItem(hDlg, IDC_DEST_STATIC), SW_HIDE);
        ShowWindow(hDestinationControl, SW_SHOW);
    }
}


static void RefillSourceBox(HWND hDlg, long nSourceControlId, long nSelectIndex)
{
    HWND hSourceControl = GetDlgItem(hDlg, nSourceControlId);

    ComboBox_ResetContent(hSourceControl);

    if (g_pDlgActiveMixer == NULL)
    {
        return;
    }

    int nDestinationIndex = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_DEST));

    if (nDestinationIndex != -1)
    {
        int sourceCount = 0;
        int index;

        index = ComboBox_AddString(hSourceControl, "<None>");
        ComboBox_SetItemData(hSourceControl, index, -1);

        ComboBox_SetCurSel(hSourceControl, index);

        CMixerLineDst* pLineDst = g_pDlgActiveMixer->GetDestinationLine(nDestinationIndex);

        if (pLineDst != NULL)
        {
            sourceCount = pLineDst->GetSourceCount();
        }

        for (int i = 0; i < sourceCount; i++)
        {
            const char* pName = pLineDst->GetSourceLine(i)->GetName();

            if (pName == NULL || strcmp(pName, "Error") == 0)
            {
                continue;
            }

            index = ComboBox_AddString(hSourceControl, pName);
            ComboBox_SetItemData(hSourceControl, index, i);

            if (i == nSelectIndex)
            {
                ComboBox_SetCurSel(hSourceControl, index);
            }
        }
    }
}


static CMixer* SynchronizeDlgChangesCallback(void* pContext)
{
    HWND hDlg = (HWND)pContext;

    // DoSettingsTranstion wants everything changed to the new
    // settings, except g_pCurrentMixer.  Instead of changing
    // g_pCurrentMixer, the new mixer should be returned.

    if (g_pDlgActiveMixer != NULL)
    {
        Setting_SetValue(WM_MIXERDEV_GETVALUE, MIXERNAME, (long)g_pDlgActiveMixer->GetName());
        g_nDestinationIndex = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_DEST));
    }
    else
    {
        Setting_SetValue(WM_MIXERDEV_GETVALUE, MIXERNAME, (long)(const char*)"");
        g_nDestinationIndex = -1;
    }

    int i = 0;

    // Read the new settings
    if (g_nDestinationIndex != -1)
    {
        if (Button_GetCheck(GetDlgItem(hDlg, IDC_SEPARATE_INPUT)) != BST_CHECKED)
        {
            int nIndex = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_MIXER_INPUT_ALL));

            for ( ; i < g_nActiveInputsCount; i++)
            {
                g_nSourceIndexes[i] = nIndex;
            }
        }
        else
        {
            for ( ; i < g_nActiveInputsCount; i++)
            {
                g_nSourceIndexes[i] = ComboBox_GetCurSelItemData(GetDlgItem(hDlg, IDC_MIXER_INPUT0+i));
            }
        }
    }

    for ( ; i < MAX_SUPPORTED_INPUTS; i++)
    {
        g_nSourceIndexes[i] = -1;
    }

    g_bUseMixer = (Button_GetCheck(GetDlgItem(hDlg, IDC_USE_MIXER)) == BST_CHECKED);
    g_bNoHardwareMute = (Button_GetCheck(GetDlgItem(hDlg, IDC_DISABLE_HW_MUTE)) == BST_CHECKED);

    return g_pDlgActiveMixer;
}


static int ComboBox_GetCurSelItemData(HWND hControl)
{
    int nIndex = ComboBox_GetCurSel(hControl);

    if (nIndex != CB_ERR)
    {
        return ComboBox_GetItemData(hControl, nIndex);
    }

    return -1;
}


//----------------------------------------------------------------------
//  Long array utilities
//----------------------------------------------------------------------

// This function fills the 'dest' array with unique elements
// from the 'source' array.  Example, given the 'source' array
// { 3, 4, 5, 5, 4, 1 }, 'dest' will become { 3, 4, 5, 1 } and
// return value 4.  'size' is the number of elements in 'source'.
static long LongArrayUnique(const long* source, long size, long* dest)
{
    long resultCount;
    int i, j;

    if (size > 0)
    {
        // Copy the first element over.
        dest[0] = source[0];
        resultCount = 1;

        // Run through every value in source.
        for (i = 1; i < size; i++)
        {
            // See if this source[i] value is already in the destination.
            for (j = 0; j < resultCount; j++)
            {
                if (dest[j] == source[i])
                {
                    break;
                }
            }

            // If the value was not already in the destination, add it.
            if (j == resultCount)
            {
                dest[resultCount++] = source[i];
            }
        }
    }

    return resultCount;
}


// Given two arrays of longs: 'values' of size 'valuesSize', and 'subtract'
// of size 'subtractSize'.  All values in the array 'values' that are also in
// the array 'subtract' are removed.  Upon return, 'values' will not contain
// any values that're also in 'subtract', and 'valuesSize' will be updated to
// represent the new size of the array if necessary.
// Set definition: R = S - U
static void LongArraySubstract(long* values, long* valuesSize,
                               const long* subtract, long substractSize)
{
    int i, j;

    // Run through every value in the array 'subtract'.
    for (i = 0; i < substractSize; i++)
    {
        // See if this subtract[i] value is also in 'values'.
        for (j = 0; j < *valuesSize; j++)
        {
            if (values[j] == subtract[i])
            {
                break;
            }
        }

        // If the value WAS found (i.e. the above loop broke early).
        if (j != *valuesSize)
        {
            // Subtract one from the size of 'values' array.
            *valuesSize -= 1;

            // 'j' is the index of the value that is being removed.
            // *valueSize is now representing the new size of the array
            // where there is one less element.  This new size value is
            // also the same as the index of the last element from the
            // previous/ size (i.e. because: last index = size - 1).  If
            // 'j' was not the last index, put the value at the last index
            // where 'j' is.
            if (j != *valuesSize)
            {
                values[j] = values[*valuesSize];
            }
        }
    }
}


// Give two arrays: 'dividend' of size 'dividendSize' and 'divisor'
// of size 'divisorSize', two resulting arrays are created.
// One array holds values that were only in 'dividend' and is
// returned in a modified 'dividend'.  The holds values that were
// only in 'divisor' and is returned in a modified 'divisor'.
// e.g. dividend: { 2, 2, 4, 1, 3, 4 }, divisor { 1, 4, 5, 7, 7 }
//      result dividend: { 2, 2, 3 }, result divisor: { 5, 7, 7 }
// Set definition: R1 = S - (S intersect U), R2 = U - (S intersect U)
static void LongArrayDivide(long* dividend, long *dividendSize,
                            long* divisor, long *divisorSize)
{
    long resultCount = 0;
    int i, j;

    // Run through every element in the array 'dividend'.
    for (i = 0; i < *dividendSize; i++)
    {
        // See if this dividend[i] value is also in 'divisor'.
        for (j = 0; j < *divisorSize; j++)
        {
            if (divisor[j] == dividend[i])
            {
                break;
            }
        }

        // If the value was not also in 'divisor'.
        if (j == *divisorSize)
        {
            // Put the value in the result array.
            dividend[resultCount++] = dividend[i];
        }
        // If the value was also in 'divisor'.
        else
        {
            // Remove the value from 'divisor' in the same way
            // commented in LongArraySubstract().  (i.e. reduce the
            // size of the array and shuffle the last element into the
            // newly created gap.)
            if (--(*divisorSize) != j)
            {
                divisor[j] = divisor[*divisorSize];
            }
        }
    }

    // The resulting array was created "in place" over the 'dividend'
    // array.  Update the correct size of 'dividend' before returning.
    *dividendSize = resultCount;
}


//----------------------------------------------------------------------
//  CMixerLineSrc
//----------------------------------------------------------------------

CMixerLineSrc::CMixerLineSrc(HMIXER hMixer, DWORD nDstIndex, DWORD nSrcIndex)
{
    MMRESULT mmresult;

    m_hMixer = hMixer;
    m_VolumeControlID = 0xFFFFFFFF;
    m_MuteControlID = 0xFFFFFFFF;

    ZeroMemory(&m_mxl, sizeof(MIXERLINE));

    m_mxl.cbStruct      = sizeof(MIXERLINE);
    m_mxl.dwDestination = nDstIndex;
    m_mxl.dwSource      = nSrcIndex;

    mmresult = mixerGetLineInfo((HMIXEROBJ)hMixer, &m_mxl, MIXER_GETLINEINFOF_SOURCE);
    if (mmresult != MMSYSERR_NOERROR || m_mxl.cControls == 0)
    {
        strcpy(m_mxl.szName, "Error");
        return;
    }

    MIXERLINECONTROLS mxlc;
    LPMIXERCONTROL pmxctrl;

    pmxctrl = (MIXERCONTROL*)malloc(sizeof(MIXERCONTROL) * m_mxl.cControls);

    mxlc.cbStruct   = sizeof(MIXERLINECONTROLS);
    mxlc.dwLineID   = m_mxl.dwLineID;
    mxlc.cControls  = m_mxl.cControls;
    mxlc.cbmxctrl   = sizeof(MIXERCONTROL);
    mxlc.pamxctrl   = pmxctrl;

    mmresult = mixerGetLineControls((HMIXEROBJ)hMixer, &mxlc, MIXER_GETLINECONTROLSF_ALL);

    for (DWORD i = 0; i < m_mxl.cControls; i++)
    {
        if (pmxctrl[i].dwControlType == MIXERCONTROL_CONTROLTYPE_VOLUME)
        {
            m_VolumeControlID = pmxctrl[i].dwControlID;
            m_VolumeMinimum = pmxctrl[i].Bounds.dwMinimum;
            m_VolumeMaximum = pmxctrl[i].Bounds.dwMaximum;

        }
        else if (pmxctrl[i].dwControlType == MIXERCONTROL_CONTROLTYPE_MUTE)
        {
            m_MuteControlID = pmxctrl[i].dwControlID;
        }
    }

    free(pmxctrl);
}


CMixerLineSrc::~CMixerLineSrc()
{
}


const char* CMixerLineSrc::GetName()
{
    return m_mxl.szName;
}


void CMixerLineSrc::SetMute(BOOL bEnable)
{
    if (m_MuteControlID != 0xFFFFFFFF)
    {
        MixerControlDetailsSet(m_MuteControlID, bEnable);
    }
}


BOOL CMixerLineSrc::GetMute()
{
    DWORD dwEnabled = FALSE;

    if (m_MuteControlID != 0xFFFFFFFF)
    {
        MixerControlDetailsGet(m_MuteControlID, &dwEnabled);
    }

    return dwEnabled;
}


void CMixerLineSrc::SetVolume(int volumePercentage)
{
    if (m_VolumeControlID != 0xFFFFFFFF)
    {
        // sets all channels to the same volume
        MixerControlDetailsSet(m_VolumeControlID,
            MulDiv(volumePercentage, (m_VolumeMaximum - m_VolumeMinimum), 100));
    }
}


int CMixerLineSrc::GetVolume()
{
    DWORD nVolume = 0;

    if (m_VolumeControlID != 0xFFFFFFFF)
    {
        // get average volume, ignores balance
        if (MixerControlDetailsGet(m_VolumeControlID, &nVolume))
        {
            return MulDiv(nVolume, 100, m_VolumeMaximum - m_VolumeMinimum);
        }
    }

    return nVolume;
}


void CMixerLineSrc::StoreState()
{
    MixerControlDetailsGet(m_MuteControlID, &m_StoredMute);
    MixerControlDetailsGet(m_VolumeControlID, &m_StoredVolume);
}


void CMixerLineSrc::RestoreState()
{
    MixerControlDetailsSet(m_MuteControlID, m_StoredMute);
    MixerControlDetailsSet(m_VolumeControlID, m_StoredVolume);
}


BOOL CMixerLineSrc::MixerControlDetailsSet(DWORD dwControlID, DWORD dwValue)
{
    MIXERCONTROLDETAILS mxcd;

    mxcd.cbStruct       = sizeof(MIXERCONTROLDETAILS);
    mxcd.cChannels      = 1;
    mxcd.dwControlID    = dwControlID;
    mxcd.cMultipleItems = 0;
    mxcd.cbDetails      = sizeof(DWORD);
    mxcd.paDetails      = &dwValue;

    return mixerSetControlDetails((HMIXEROBJ)m_hMixer, &mxcd,
        MIXER_SETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_HMIXER) == MMSYSERR_NOERROR;
}


BOOL CMixerLineSrc::MixerControlDetailsGet(DWORD dwControlID, LPDWORD lpdwValue)
{
    MIXERCONTROLDETAILS mxcd;

    mxcd.cbStruct       = sizeof(MIXERCONTROLDETAILS);
    mxcd.cChannels      = 1;
    mxcd.dwControlID    = dwControlID;
    mxcd.cMultipleItems = 0;
    mxcd.cbDetails      = sizeof(DWORD);
    mxcd.paDetails      = lpdwValue;

    return mixerGetControlDetails((HMIXEROBJ)m_hMixer, &mxcd,
        MIXER_SETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_HMIXER) == MMSYSERR_NOERROR;
}


//----------------------------------------------------------------------
//  CMixerLineDst
//----------------------------------------------------------------------

CMixerLineDst::CMixerLineDst(HMIXER hMixer, DWORD nDstIndex)
{
    ZeroMemory(&m_mxl, sizeof(MIXERLINE));

    m_mxl.cbStruct = sizeof(MIXERLINE);
    m_mxl.dwDestination = nDstIndex;

    m_nSourceCount = 0;
    m_pSourceLines = NULL;

    MMRESULT mmresult = mixerGetLineInfo((HMIXEROBJ)hMixer, &m_mxl,
        MIXER_GETLINEINFOF_DESTINATION);

    if (mmresult != MMSYSERR_NOERROR || m_mxl.cConnections == 0)
    {
        strcpy(m_mxl.szName, "Error");
        return;
    }

    m_nSourceCount = m_mxl.cConnections;
    m_pSourceLines = new CMixerLineSrc*[m_nSourceCount];

    for (DWORD i = 0; i < m_nSourceCount; i++)
    {
        m_pSourceLines[i] = new CMixerLineSrc(hMixer, nDstIndex, i);
    }
}


CMixerLineDst::~CMixerLineDst()
{
    if (m_pSourceLines != NULL)
    {
        for (DWORD i = 0; i < m_nSourceCount; i++)
        {
            delete m_pSourceLines[i];
        }
        delete [] m_pSourceLines;
    }
}


const char* CMixerLineDst::GetName()
{
    return m_mxl.szName;
}


long CMixerLineDst::GetSourceCount()
{
    return m_nSourceCount;
}


CMixerLineSrc* CMixerLineDst::GetSourceLine(DWORD nIndex)
{
    if (nIndex >= m_nSourceCount)
    {
        return NULL;
    }

    return m_pSourceLines[nIndex];
}


void CMixerLineDst::StoreState()
{
    for (DWORD i = 0; i < m_nSourceCount; i++)
    {
        m_pSourceLines[i]->StoreState();
    }
}


void CMixerLineDst::RestoreState()
{
    for (DWORD i = 0; i < m_nSourceCount; i++)
    {
        m_pSourceLines[i]->RestoreState();
    }
}


BOOL CMixerLineDst::IsTypicalSpeakerLine()
{
    return (m_mxl.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_SPEAKERS);
}


BOOL CMixerLineDst::IsTypicalRecordingLine()
{
    return (m_mxl.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_WAVEIN);
}


//----------------------------------------------------------------------
//  CMixer
//----------------------------------------------------------------------

CMixer::CMixer(DWORD nMixerIndex)
{
    MMRESULT mmresult;

    m_hMixer = NULL;
    m_nDestinationCount = 0;
    m_pDestinationLines = NULL;

    m_nMixerIndex = nMixerIndex;

    mmresult = mixerGetDevCaps(m_nMixerIndex, &m_mxcaps, sizeof(MIXERCAPS));
    if (mmresult != MMSYSERR_NOERROR || m_mxcaps.cDestinations == 0)
    {
        delete this;
        throw NULL;
    }

    mmresult = mixerOpen(&m_hMixer, m_nMixerIndex, NULL, NULL, MIXER_OBJECTF_MIXER);
    if (mmresult != MMSYSERR_NOERROR)
    {
        delete this;
        throw NULL;
    }

    m_nDestinationCount = m_mxcaps.cDestinations;
    m_pDestinationLines = new CMixerLineDst*[m_nDestinationCount];

    for (DWORD i = 0; i < m_nDestinationCount; i++)
    {
        m_pDestinationLines[i] = new CMixerLineDst(m_hMixer, i);
    }
}


CMixer::~CMixer()
{
    if (m_pDestinationLines != NULL)
    {
        for (DWORD i = 0; i < m_nDestinationCount; i++)
        {
            delete m_pDestinationLines[i];
        }

        delete [] m_pDestinationLines;
    }

    if (m_hMixer != NULL)
    {
        mixerClose(m_hMixer);
    }
}


const char* CMixer::GetName()
{
    return m_mxcaps.szPname;
}


DWORD CMixer::GetIndex()
{
    return m_nMixerIndex;
}


long CMixer::GetDestinationCount()
{
    return m_nDestinationCount;
}


CMixerLineDst* CMixer::GetDestinationLine(DWORD nIndex)
{
    if (nIndex >= m_nDestinationCount)
    {
        return NULL;
    }

    return m_pDestinationLines[nIndex];
}


void CMixer::StoreState()
{
    for (DWORD i = 0; i < m_nDestinationCount; i++)
    {
        m_pDestinationLines[i]->StoreState();
    }
}


void CMixer::RestoreState()
{
    for (DWORD i = 0; i < m_nDestinationCount; i++)
    {
        m_pDestinationLines[i]->RestoreState();
    }
}


//----------------------------------------------------------------------
//  CMixerFinder
//----------------------------------------------------------------------

CMixerFinder::CMixerFinder()
{
    m_nMixerCount = mixerGetNumDevs();
}


CMixerFinder::~CMixerFinder()
{
}


long CMixerFinder::GetMixerCount()
{
    return m_nMixerCount;
}


BOOL CMixerFinder::GetMixerName(long nMixerIndex, char szName[MAXPNAMELEN])
{
    MIXERCAPS mxcaps;
    MMRESULT mmresult;

    _ASSERTE(nMixerIndex >= 0 && (UINT)nMixerIndex < m_nMixerCount);

    mmresult = mixerGetDevCaps(nMixerIndex, &mxcaps, sizeof(MIXERCAPS));
    if (mmresult == MMSYSERR_NOERROR)
    {
        strcpy(szName, mxcaps.szPname);
        return TRUE;
    }
    return FALSE;
}


long CMixerFinder::FindMixer(const char* szName)
{
    char buffer[MAXPNAMELEN];

    for (UINT i = 0; i < m_nMixerCount; i++)
    {
        if (GetMixerName(i, buffer) && lstrcmp(buffer, szName) == 0)
        {
            return i;
        }
    }
    return -1;
}


//----------------------------------------------------------------------
//  MixerDev stuff
//----------------------------------------------------------------------

SETTING MixerDevSettings[MIXERDEV_SETTING_LASTONE] =
{
    {
        "Use Mixer", ONOFF, 0, (long*)&g_bUseMixer,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Mixer", "UseMixer", NULL,
    },
    {
        "DestIndex", SLIDER, 0, (long*)&g_nDestinationIndex,
        0, 0, 255, 1, 1,
        NULL,
        "Mixer", "DestIndex", NULL,
    },
    {
        "Input 1 Index", SLIDER, 0, (long*)&g_nSourceIndexes[0],
        -1, -1, 255, 1, 1,
        NULL,
        "Mixer", "Input1Index", NULL,
    },
    {
        "Input 2 Index", SLIDER, 0, (long*)&g_nSourceIndexes[1],
        -1, -1, 255, 1, 1,
        NULL,
        "Mixer", "Input2Index", NULL,
    },
    {
        "Input 3 Index", SLIDER, 0, (long*)&g_nSourceIndexes[2],
        -1, -1, 255, 1, 1,
        NULL,
        "Mixer", "Input3Index", NULL,
    },
    {
        "Input 4 Index", SLIDER, 0, (long*)&g_nSourceIndexes[3],
        -1, -1, 255, 1, 1,
        NULL,
        "Mixer", "Input4Index", NULL,
    },
    {
        "Reset Mixer on Exit", ONOFF, 0, (long*)&g_bResetOnExit,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Mixer", "ResetOnExit", NULL,
    },
    {
        "Input 5 Index", SLIDER, 0, (long*)&g_nSourceIndexes[4],
        -1, -1, 255, 1, 1,
        NULL,
        "Mixer", "Input5Index", NULL,
    },
    {
        "Input 6 Index", SLIDER, 0, (long*)&g_nSourceIndexes[5],
        -1, -1, 255, 1, 1,
        NULL,
        "Mixer", "Input6Index", NULL,
    },
    {
        "Mixer Name", CHARSTRING, 0, g_pMixerName.GetPointer(),
        (long)"", 0, 0, 0, 0,
        NULL,
        "Mixer", "MixerName", NULL,
    },
    {
        "No Hardware Mute", ONOFF, 0, (long*)&g_bNoHardwareMute,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Mixer", "NoHardwareMute", NULL,
    },
};


void MixerDev_UpdateSettings(CSource* pSource)
{
    if(MixerSettings)
    {
        SettingsMaster->Unregister(MixerSettings);
        MixerSettings->WriteToIni(TRUE);
        MixerSettings = 0L;
    }

    if (pSource == NULL)
    {
        g_bMixerDevInvalidSection = TRUE;
    }
    else
    {
        MixerSettings = new CSettingsHolder(WM_MIXERDEV_GETVALUE);
        string MixerDev_Section(MakeString() << "MixerInput_" << pSource->IDString());
        g_bMixerDevInvalidSection = FALSE;

        for (int i = 0; i < MIXERDEV_SETTING_LASTONE; i++)
        {
            MixerDevSettings[i].szIniSection = (char*)MixerDev_Section.c_str();
        }

        MixerSettings->AddSettings(MixerDevSettings, MIXERDEV_SETTING_LASTONE);
        MixerSettings->ReadFromIni();
        SettingsMaster->Register(MixerSettings);
    }
}

void MixerDev_SetMenu(HMENU hMenu)
{

}

