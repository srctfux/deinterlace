////////////////////////////////////////////////////////////////////////////
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
 * @file DScaler.cpp WinMain and related UI code
 */

#include "stdafx.h"
#define ISDSCALERCPPFILE
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "IOutput.h"
#include "CPU.h"
#include "MixerDev.h"
#include "VBI_VideoText.h"
#include "AspectRatio.h"
#include "DScaler.h"
#include "Settings.h"
#include "ProgramList.h"
#include "Dialogs.h"
#include "OutThreads.h"
#include "OSD.h"
#include "Audio.h"
#include "Status.h"
#include "VBI.h"
#include "FD_60Hz.H"
#include "FD_50Hz.H"
#include "FD_Common.H"
#include "Filter.h"
#include "Splash.h"
#include "VideoSettings.h"
#include "VBI_CCdecode.h"
#include "VBI_VideoText.h"
#include "Deinterlace.h"
#include "FieldTiming.h"
#include "DebugLog.h"
#include "TimeShift.h"
#include "Crash.h"
#include "BuildNum.h"
#include "Calibration.h"
#include "Providers.h"
#include "OverlaySettings.h"
#include "Perf.h"
#include "hardwaredriver.h"
#include "StillSource.h"
#include "TreeSettingsDlg.h"
#include "SettingsPerChannel.h"
#include "HardwareSettings.h"
#include "Events.h"
#include "WindowBorder.h"
#include "ToolbarControl.h"
#include "SettingsMaster.h"
#include "Credits.h"
#include "SizeSettings.h"
#include "PaintingHDC.h"
#include "OutReso.h"
#include "MultiFrames.h"
#include "SAA7134Card.h"
#include "CX2388xCard.h"
#include "EPG.h"
#include "ScheduledRecording.h"
#include "OverlayOutput.h"
#include "D3D9Output.h"
#include "RemoteInput.h"
#include "PathHelpers.h"
#include "..\API\DScalerVersion.h"

#ifdef WANT_DSHOW_SUPPORT
#include "dshowsource/DSSourceBase.h"
#endif

using namespace std;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


HWND hWnd = NULL;
HINSTANCE hResourceInst = NULL;
HINSTANCE hDScalerInst = NULL;
LPSTR lPStripTimingString = NULL;

HWND hPSWnd = NULL;

// Used to call MainWndOnInitBT
#define INIT_BT 1800

BOOL bDoResize = FALSE;

HWND VThWnd;

long WStyle;

BOOL    bShowMenu=TRUE;
HMENU   hMenu;
HMENU   hSubMenuChannels = NULL;
HMENU    hMenuTray;
HACCEL  hAccel;

char ChannelString[10] = "";

int MainProcessor=0;
int DecodeProcessor=0;
int PriorClassId = 0;
int ThreadClassId = 1;

//Cursor defines and vars
#define CURSOR_DEFAULT 0x0000
#define CURSOR_HAND    0x0001
HCURSOR hCursorDefault = NULL;
HCURSOR hCursorHand = NULL;

BOOL bShowCursor = TRUE;
BOOL bAutoHideCursor = TRUE;

long MainWndWidth = 649;
long MainWndHeight = 547;
long MainWndLeft = 10;
long MainWndTop = 10;

BOOL bAlwaysOnTop = FALSE;
BOOL bAlwaysOnTopFull = TRUE;
BOOL bDisplayStatusBar = TRUE;
BOOL bDisplaySplashScreen = TRUE;
BOOL bVTSingleKeyToggle = TRUE;
BOOL bIsFullScreen = FALSE;
BOOL bForceFullScreen = FALSE;
BOOL bUseAutoSave = FALSE;
BOOL bScreensaverOff = FALSE;
BOOL bVTAutoCodePage = FALSE;
BOOL bVTAntiAlias = FALSE;
BOOL bMinimized = FALSE;
BOOL bReverseChannelScroll = FALSE;

BOOL bMinToTray = FALSE;
BOOL bIconOn = FALSE;
int MinimizeHandling = 0;
BOOL BypassChgResoInRestore = FALSE;

BOOL bKeyboardLock = FALSE;
BOOL bKeyboardLockMainWindowOnly = FALSE;
HHOOK hKeyboardHook = NULL;

HFONT hCurrentFont = NULL;

int ChannelEnterTime = 0;

int InitialChannel = -1;
int InitialTextPage = -1;

BOOL bInMenu = FALSE;
BOOL bShowCrashDialog = FALSE;
BOOL bIsRightButtonDown = FALSE;
BOOL bIgnoreNextRightButtonUpMsg = FALSE;

UINT MsgWheel;
UINT MsgOSDShow;
UINT MsgTaskbarRestart;

NOTIFYICONDATA nIcon;

DWORD hMainThread = NULL;

// Current sleepmode timers (minutes)
TSMState SMState;
#define SMPeriodCount       7
int SMPeriods[SMPeriodCount] =
{
    0,
    1,
    15,
    30,
    60,
    90,
    120
};

HRGN DScalerWindowRgn = NULL;

char* szSkinName = NULL;
char szSkinDirectory[MAX_PATH+1];
vector<string> vSkinNameList;

CSettingsMaster* SettingsMaster = NULL;
CEventCollector* EventCollector = NULL;
CWindowBorder* WindowBorder = NULL;
CToolbarControl* ToolbarControl = NULL;

CPaintingHDC OffscreenHDC;

BOOL IsFullScreen_OnChange(long NewValue);
BOOL DisplayStatusBar_OnChange(long NewValue);
BOOL ScreensaverOff_OnChange(long NewValue);
BOOL Cursor_IsOurs();
void Cursor_UpdateVisibility();
void Cursor_SetVisibility(BOOL bVisible);
int Cursor_SetType(int type);
void Cursor_VTUpdate(int x = -1, int y = -1);
void MainWndOnDestroy();
int ProcessCommandLine(char* commandLine, char* argv[], int sizeArgv);
void SetKeyboardLock(BOOL Enabled);
bool bScreensaverDisabled = false;
bool bPoweroffDisabled = false;
bool bLowpowerDisabled = false;
HMENU CreateDScalerPopupMenu();
BOOL IsStatusBarVisible();
BOOL IsToolBarVisible();
HRGN UpdateWindowRegion(HWND hWnd, BOOL bUpdateWindowState);
void SetWindowBorder(HWND hWnd, LPCSTR szSkinName, BOOL bShow);
void Skin_SetMenu(HMENU hMenu, BOOL bUpdateOnly);
LPCSTR GetSkinDirectory();
LONG OnChar(HWND hWnd, UINT message, UINT wParam, LONG lParam);
LONG OnSize(HWND hWnd, UINT wParam, LONG lParam);
LONG OnAppCommand(HWND hWnd, UINT wParam, LONG lParam);
LONG OnInput(HWND hWnd, UINT wParam, LONG lParam);
void SetTray(BOOL Way);
int On_IconHandler(WPARAM wParam, LPARAM lParam);

static BOOL g_bOverlayStopped = FALSE;

static const char* UIPriorityNames[3] = 
{
    "Normal",
    "High",
    "Real time",
};

static const char* DecodingPriorityNames[5] = 
{
    "Below normal",
    "Normal",
    "Above Normal",
    "High",
    "Time critical",
};

static const char* OutputMethodNames[2] = 
{
    "Overlay",
    "Direct3D",
};

static const char* MinimizeHandlingLabels[3] = 
{
    "User control / Continue capture",
    "User control / Stop capture",
    "Automatic / Detect video signal",
};


static BOOL bTakingCyclicStills = FALSE;
static BOOL bIgnoreDoubleClick = FALSE;

static int ProcessorSpeed = 1;
static int TradeOff = 1;
static int FullCpu = 1;
static int VideoCard = 0;
static int ShowHWSetupBox;
static long m_EventTimerID = 0;

static int ChannelPreviewNbCols = 4;
static int ChannelPreviewNbRows = 4;

static IOutput::OUTPUTTYPES OutputMethod = IOutput::OUT_OVERLAY;

static HANDLE hMainWindowEvent = NULL;

///**************************************************************************
//
// FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)
//
// PURPOSE: calls initialization function, processes message loop
//
///**************************************************************************

int APIENTRY WinMainOld(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc;
    MSG msg;
    HWND hPrevWindow;

    hDScalerInst = hInstance;
    hMainThread = GetCurrentThreadId();

    SetErrorMode(SEM_NOGPFAULTERRORBOX);

    SetCurrentDirectory(GetInstallationPath().c_str());

    CPU_SetupFeatureFlag();

    MsgOSDShow = RegisterWindowMessage("DScalerShowOSDMsgString");
    MsgWheel = RegisterWindowMessage("MSWHEEL_ROLLMSG");
    MsgTaskbarRestart = RegisterWindowMessage("TaskbarCreated");

    // make a copy of the command line since ProcessCommandLine() will replace the spaces with \0
    char OldCmdLine[1024];
    strncpy(OldCmdLine, lpCmdLine, sizeof(OldCmdLine));

    char* ArgValues[20];
    int ArgCount = ProcessCommandLine(lpCmdLine, ArgValues, sizeof(ArgValues) / sizeof(char*));

    // if we are already running then start up old version
    hPrevWindow = FindWindow((LPCTSTR) DSCALER_APPNAME, NULL);
    if (hPrevWindow != NULL)
    {
        if (IsIconic(hPrevWindow))
        {
            SendMessage(hPrevWindow, WM_SYSCOMMAND, SC_RESTORE, NULL);
        }
        SetFocus(hPrevWindow);
        SetActiveWindow(hPrevWindow);
        SetForegroundWindow(hPrevWindow);

        if(ArgCount > 1)
        {
            // Command line parameter to send messages to the OSD of a running copy of DScaler.
            //
            // Usage:
            // /m for a temporary message. /M for a persistent message.
            // /m or /M must be the first parameter.
            //
            // use \n to start a new line.
            // If DScaler is not running this copy exits silently.
            //
            // example:
            // dscaler /m This is a message.\nSecond line.
            //
            if((ArgValues[0][0] == '/' || ArgValues[0][0] == '-') && tolower(ArgValues[0][1]) == 'm')
            {
                HANDLE hMapFile = NULL;
                char* lpMsg = NULL;

                hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE,  // Use the system page file
                    NULL, PAGE_READWRITE, 0, 1024, "DScalerSendMessageFileMappingObject");                        
                
                if(hMapFile == NULL) 
                { 
                    // send error message to running copy of DScaler
                    SendMessage(hPrevWindow, MsgOSDShow, GetLastError(), 1);
                } 
                else
                {
                    lpMsg = (char*)MapViewOfFile(hMapFile, FILE_MAP_WRITE, 0, 0, 1024);              
                    
                    if (lpMsg == NULL) 
                    { 
                        // send error message to running copy of DScaler
                        SendMessage(hPrevWindow, MsgOSDShow, GetLastError(), 2);
                    }
                    else
                    {
                        // Find the command line after the /m or -m or /M or -M
                        // a simple strstr(OldCmdLine, ArgValues[1]) will not work with a leading quote
                        char* s;
                        s = strstr(OldCmdLine, ArgValues[0]);   // s points to first parameter
                        s = &s[strlen(ArgValues[0])];           // s points to char after first parm
                        if(s[0] == ' ')                         // point s to string after the space
                        {
                            s++;
                        }
                        strncpy(lpMsg, s, 1024);
                        SendMessage(hPrevWindow, MsgOSDShow, ArgValues[0][1] == 'm' ? 0 : 1, 0);
                        UnmapViewOfFile(lpMsg);
                    }
                    CloseHandle(hMapFile);
                }
            }
        }
        return FALSE;
    }

    // also used by the InnoSetup installer to prevent (un)installation when DScaler is running.
    if(CreateMutex(NULL, FALSE, "DScaler"))
    {
        // RM oct 1 2006
        // Do not start if the mutex already exists.
        // There is another similar check that tests for the dscaler window name. 
        // This did not always work because several things happen before the window is created,
        // sometimes there is a delay in starting up before the window is created. The user
        // then tries to start the program a second time. This might result in corrupting the 
        // ini file.
        // I am uncertain what is causing the occasional delay in startup. This delay happens
        // for example when Windows Explorer (the shell) is busy, as can be seen by a 
        // non-responding task bar.
        if(GetLastError() == ERROR_ALREADY_EXISTS)
        {
            return FALSE;
        }
    }

    // JA 07/01/2001
    // Required to use slider control
    InitCommonControls();

    // setup default ini file
    SetIniFileForSettings("");
    
    // Process the command line arguments.
    // The following arguments are supported
    // -i<inifile>      specification of the ini file.
    // -c<channel>      the starting channel (0-x).
    // -p<page>         the starting videotext page (100-y).
    // -driverinstall   (un)install the NT driver. These arguments are mainly intended to be
    // -driveruninstall used by the installation program. When the user has not enough rights to
    //                  complete (un)installation a messagebox is shown. With other errors there
    //                  is no feedback.
    // For backwards compatibility an argument not starting with a - or / is
    // processed as -i<parameter> (ini file specification).
    for (int i = 0; i < ArgCount; ++i)
    {
        if (ArgValues[i][0] != '-' && ArgValues[i][0] != '/')
        {
            SetIniFileForSettings(ArgValues[i]);
        }
        else if(tolower(ArgValues[i][1]) == 'm')
        {
            // DScaler was called to show a message in a running instance of DScaler.
            // At this time DScaler is not running so we can exit now.
            return 0;
        }
        else if(strlen(ArgValues[i]) > 2)
        {
            char* szParameter = &ArgValues[i][2];
            switch (tolower(ArgValues[i][1]))
            {
            case 'd':
                if(strcmp(szParameter, "riverinstall") == 0)
                {
                    DWORD result = 0;                  
                    CHardwareDriver* HardwareDriver = NULL;
                    
                    HardwareDriver = new CHardwareDriver();
                    if(!HardwareDriver->InstallNTDriver())
                    {
                        // if access denied
                        if(GetLastError() == 5)
                        {
                            RealErrorBox("You must have administrative rights to install the driver.");
                        }
                        result = 1;
                    }
                    
                    delete HardwareDriver;
                    
                    return result;
                }
                else if(strcmp(szParameter, "riveruninstall") == 0)
                {
                    DWORD result = 0;                  
                    CHardwareDriver* HardwareDriver = NULL;
                    
                    HardwareDriver = new CHardwareDriver();
                    if(!HardwareDriver->UnInstallNTDriver())
                    {
                        // if access denied
                        if(GetLastError() == 5)
                        {
                            RealErrorBox("You must have administrative rights to uninstall the driver.");
                        }
                        result = 1;
                    }
                    
                    delete HardwareDriver;
                    
                    return result;
                }
            case 'i':
                SetIniFileForSettings(szParameter);              
                break;
            case 'c':
                sscanf(szParameter, "%d", &InitialChannel);
                break;
            case 'p':
                sscanf(szParameter, "%x", &InitialTextPage);
                break;
            default:
                // Unknown
                break;
            }
        }
    }

    // Load up the list of SAA713x and CX2388x cards
#ifdef WANT_SAA713X_SUPPORT
    if (!CSAA7134Card::InitializeSAA713xCardList())
    {
        // Caution, this code exits DScaler abruptly based on the user input.
        // Although this is not done forcefully using a call like exit(), it
        // should be noted that DScaler can exit here.  Any code requiring
        // clean up should be careful about this.  Driver related and memory
        // mapping operations should not be moved before this. --atnak 04-11-21
        return 0;
    }
#endif

#ifdef WANT_CX2388X_SUPPORT
    if (!CCX2388xCard::InitializeCX2388xCardList())
    {
        // Caution, this code exits DScaler abruptly based on the user input.
        // Although this is not done forcefully using a call like exit(), it
        // should be noted that DScaler can exit here.  Any code requiring
        // clean up should be careful about this.  Driver related and memory
        // mapping operations should not be moved before this. --atnak 04-11-21
        return 0;
    }
#endif

    ShowHWSetupBox =    !Setting_ReadFromIni(DScaler_GetSetting(PROCESSORSPEED))
                     || !Setting_ReadFromIni(DScaler_GetSetting(TRADEOFF))
                     || !Setting_ReadFromIni(DScaler_GetSetting(FULLCPU))
                     || !Setting_ReadFromIni(DScaler_GetSetting(VIDEOCARD));

    // Event collector
    if (EventCollector == NULL)
    {
        EventCollector = new CEventCollector();        
    }
        
    // Master setting. Holds all settings in the future
    //  For now, only settings that are registered here respond to events 
    //  like source/input/format/channel changes
    if (SettingsMaster == NULL)
    {
        SettingsMaster = new CSettingsMaster();
    }
    SettingsMaster->IniFile(GetIniFileForSettings());

    // load up ini file settings after parsing parms as 
    // the ini file location may have changed
    LoadSettingsFromIni();
    
    SetActiveOutput(OutputMethod);

    // make sure dscaler.ini exists with many of the options in it.
    // even if dscaler crashes a new user is able to make changes to dscaler.ini.
    WriteSettingsToIni(TRUE);

    // Initialize the audio muting module
    Initialize_Mute();

    // load up the cursors we want to use
    // we load up arrow as the default and try and load up
    // the hand cursor if we are running NT 5
    hCursorDefault = LoadCursor(NULL, IDC_ARROW);

    OSVERSIONINFO version;
    version.dwOSVersionInfoSize = sizeof(version);
    GetVersionEx(&version);
    if ((version.dwPlatformId == VER_PLATFORM_WIN32_NT) && (version.dwMajorVersion >= 5))
    {
        hCursorHand = LoadCursor(NULL, IDC_HAND);
    }
    else
    {        
        hCursorHand = LoadCursor(hResourceInst, MAKEINTRESOURCE(IDC_CURSOR_HAND));
    }

    wc.style = CS_DBLCLKS;      // Allow double click
    wc.lpfnWndProc = (WNDPROC) MainWndProcSafe;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(LONG);
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DSCALER));
    wc.hCursor = hCursorDefault; 
    // Set to the default overlay colour to workaround a redraw bug that happens
    // over overlay surfaces on some systems.  atnak 3rd May 2004
    wc.hbrBackground = CreateSolidBrush(RGB(32, 16, 16));
    wc.lpszMenuName = NULL;
    wc.lpszClassName = DSCALER_APPNAME;

    if (!RegisterClass(&wc))
    {
        return FALSE;
    }
    hMenu = LoadMenu(hResourceInst, MAKEINTRESOURCE(IDC_DSCALERMENU));

    hSubMenuChannels = GetSubMenuWithName(hMenu, 2, "&Channels");


    // 2000-10-31 Added by Mark: Changed to WS_POPUP for more cosmetic direct-to-full-screen startup,
    // let UpdateWindowState() handle initialization of windowed dTV instead.

    hWnd = CreateWindow(DSCALER_APPNAME, DSCALER_APPNAME, WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL, NULL, hInstance, NULL);
    if (!hWnd) return FALSE;
    // Always position the window to the last saved position even when starting in full screen mode
    // to be sure to have the display on the correct screen
    // Display in full screen mode is then done later when calling UpdateWindowState
    SetWindowPos(hWnd, 0, MainWndLeft, MainWndTop, MainWndWidth, MainWndHeight, SWP_SHOWWINDOW);

    // Restore the default positions for the window if the window was previously placed on a screen
    // which is no more active
    // The default position is on the primary monitor
    RECT screenRect;
    GetActiveOutput()->GetMonitorRect(hWnd, &screenRect);
    if ( (MainWndLeft > screenRect.right)
      || ((MainWndLeft+MainWndWidth) < screenRect.left)
      || (MainWndTop > screenRect.bottom)
      || ((MainWndTop+MainWndHeight) < screenRect.top) )
    {
        Setting_SetDefault(DScaler_GetSetting(STARTLEFT));
        Setting_SetDefault(DScaler_GetSetting(STARTTOP));
        Setting_SetDefault(DScaler_GetSetting(STARTWIDTH));
        Setting_SetDefault(DScaler_GetSetting(STARTHEIGHT));
        SetWindowPos(hWnd, 0, MainWndLeft, MainWndTop, MainWndWidth, MainWndHeight, SWP_SHOWWINDOW);
    }

    // Show the splash screen after creating the main window
    // to be sure to display it on the right monitor
    // also don't show it on the first run as that will get in the way
    // of the dialogs
    if(bDisplaySplashScreen && ShowHWSetupBox == FALSE)
    {
        ShowSpashScreen();
    }

    if (!StatusBar_Init()) return FALSE;

    if (bDisplayStatusBar == FALSE)
    {
        StatusBar_ShowWindow(FALSE);
    }

    if (bMinToTray)
    {
        SetTray(TRUE);
    }

    // 2000-10-31 Added by Mark Rejhon
    // Now show the window, directly to maximized or windowed right away.
    // That way, if the end user has configured dTV to startup maximized,
    // it won't flash a window right before maximizing.
    UpdateWindowState();
    CScheduledRecording::initScheduledRecordingThreadProc();

    PostMessage(hWnd, WM_SIZE, SIZENORMAL, MAKELONG(MainWndWidth, MainWndHeight));
    if ((hAccel = LoadAccelerators(hResourceInst, MAKEINTRESOURCE(IDA_DSCALER))) == NULL)
    {
        ErrorBox("Accelerators not Loaded");
    }

    // Initialize sleepmode
    SMState.State = SM_WaitMode;
    SMState.iPeriod = 0;
    SMState.Period = 0;
    SMState.SleepAt = 0;

    // trigger any error messages if the menu is corrupt
#ifdef _DEBUG
    CreateDScalerPopupMenu();
#endif
    
    // catch any serious errors during message handling
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(hWnd, hAccel, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    LOG(1,"Program exit");

    return msg.wParam;
}

LONG APIENTRY MainWndProcSafe(HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
    try
    {
        LOG(3, "Got Windows Message %d Params %d %d", message, wParam, lParam);
        return MainWndProc(hWnd, message, wParam, lParam);
    }
    // if there is any exception thrown then exit the process
    catch(std::exception& e)
    { 
        LOG(1, "Crash in MainWndProc");
        LOG(1, e.what());
        // try as best we can to unload everything
        // mostly we want to make sure that the driver is stopped
        // cleanly so that the machine doesn't blue screen
        MainWndOnDestroy();
        ExitProcess(1);
        return 0;
    }
    catch(...)
    { 
        LOG(1, "Crash in MainWndProc");
        // try as best we can to unload everything
        // mostly we want to make sure that the driver is stopped
        // cleanly so that the machine doesn't blue screen
        MainWndOnDestroy();
        ExitProcess(1);
        return 0;
    }
}

HMENU CreateDScalerPopupMenu()
{
    HMENU hMenuPopup;
    hMenuPopup = LoadMenu(hResourceInst, MAKEINTRESOURCE(IDC_CONTEXTMENU));
    if (hMenuPopup != NULL)
    {
        hMenuPopup = GetSubMenu(hMenuPopup,0);
    }
    if (hMenuPopup != NULL && hMenu != NULL)
    {
        MENUITEMINFO MenuItemInfo;
        HMENU hSubMenu;
        char string[128];
        int reduc1;

        // update the name of the source
        if(GetMenuString(hMenu, 1, string, sizeof(string), MF_BYPOSITION) != 0)
        {
            ModifyMenu(hMenuPopup, 1, MF_BYPOSITION | MF_STRING, IDM_POPUP_SOURCES, string);
        }

        MenuItemInfo.cbSize = sizeof (MenuItemInfo);
        MenuItemInfo.fMask = MIIM_SUBMENU;

        hSubMenu = GetSubMenuWithName(hMenu, 0, "&Sources");
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,0,TRUE,&MenuItemInfo);
        }

        // The name of this menu item depends on the source so we don't check the name
        hSubMenu = GetSubMenu(hMenu, 1);
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,1,TRUE,&MenuItemInfo);
        }

        hSubMenu = GetChannelsSubmenu();
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,2, TRUE, &MenuItemInfo);
        }

        string[0] = '\0';
        GetMenuString(hMenu, 2, string, sizeof(string), MF_BYPOSITION);
        reduc1 = !strcmp(string, "&Channels") ? 0 : 1;

        hSubMenu = GetSubMenuWithName(hMenu, 3-reduc1, "&View");
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,3,TRUE,&MenuItemInfo);
        }

        hSubMenu = GetVideoDeinterlaceSubmenu();
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,4, TRUE, &MenuItemInfo);
        }

        hSubMenu = GetFiltersSubmenu();
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,5, TRUE, &MenuItemInfo);
        }

        hSubMenu = GetSubMenuWithName(hMenu, 6-reduc1, "&Aspect Ratio");
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,6, TRUE, &MenuItemInfo);
        }

        hSubMenu = GetSubMenuWithName(hMenu, 7-reduc1, "S&ettings");
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,7, TRUE, &MenuItemInfo);
        }

        hSubMenu = GetSubMenuWithName(hMenu, 8-reduc1, "Ac&tions");
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,8, TRUE, &MenuItemInfo);
        }

        hSubMenu = GetSubMenuWithName(hMenu, 9-reduc1, "&Datacasting");
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,9,TRUE,&MenuItemInfo);
        }

        hSubMenu = GetSubMenuWithName(hMenu, 10-reduc1, "&Help");
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,10,TRUE,&MenuItemInfo);
        }
    }
    return hMenuPopup;
}


BOOL WINAPI OnContextMenu(HWND hWnd, int x, int y)
{ 
    RECT rc;                    // client area of window
    POINT pt = {0,0};           // location of mouse click
    HMENU hMenuPopup = CreateDScalerPopupMenu();

    // Get the bounding rectangle of the client area.
    GetClientRect(hWnd, &rc);

    //if the context menu is opend with a keypress, x and y is -1
    if(x!=-1 && y!=-1)
    {
        pt.x=x;
        pt.y=y;

        // Convert the mouse position to client coordinates.
        ScreenToClient(hWnd, &pt);
    }

    // If the position is in the client area, display a
    // shortcut menu.
    if (PtInRect(&rc, pt))
    {
        ClientToScreen(hWnd, &pt);
        // Display the shortcut menu. Track the right mouse
        // button.
        return TrackPopupMenuEx(hMenuPopup, 
                                TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, 
                                pt.x, pt.y, hWnd, NULL); 
    }

    if (hMenuPopup != NULL)
    {
        DestroyMenu(hMenuPopup);
    }
 
    // Return FALSE if no menu is displayed.
    return FALSE; 
} 


BOOL ProcessVTMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    RECT Rect;
    BOOL bHandled = FALSE;
    BOOL bPageChanged = FALSE;

    HDC hWndDC = GetDC(hWnd);
    GetDestRect(&Rect);

    OffscreenHDC.UpdateGeometry(hWndDC, &Rect);
    HDC hDC = OffscreenHDC.GetBufferDC();

    VT_ResetPaintedRects();

    switch (uMsg)
    {
    case WM_COMMAND:
        if (VT_GetState() != VT_OFF)
        {
            switch (LOWORD(wParam))
            {
            case IDM_VT_PAGE_MINUS:
                VT_ClearInput();
                bPageChanged = VT_PageScroll(hDC, &Rect, FALSE);
                bHandled = TRUE;
                break;

            case IDM_VT_PAGE_PLUS:
                VT_ClearInput();
                bPageChanged = VT_PageScroll(hDC, &Rect, TRUE);
                bHandled = TRUE;
                break;

            case IDM_VT_PAGE_UP:
                VT_ClearInput();
                bPageChanged = VT_SubPageScroll(hDC, &Rect, FALSE);
                bHandled = TRUE;
                break;

            case IDM_VT_PAGE_DOWN:
                VT_ClearInput();
                bPageChanged = VT_SubPageScroll(hDC, &Rect, TRUE);
                bHandled = TRUE;
                break;

            case IDM_CHANNELPLUS:
            case IDM_CHANNELMINUS:
            case IDM_CHANNEL_PREVIOUS:
                VT_ClearInput();
                VT_ShowHeader(hDC, &Rect);
                bHandled = TRUE;
                break;

            case IDC_TOOLBAR_CHANNELS_LIST:
                bHandled = TRUE;
                break;

            case IDM_VT_SEARCH:
            case IDM_VT_SEARCHNEXT:
            case IDM_VT_SEARCHPREV:
                {
                    BOOL bInclusive = FALSE;
                    BOOL bReverse = LOWORD(wParam) == IDM_VT_SEARCHPREV;
                    bHandled = TRUE;

                    VT_ClearInput();

                    // Get search string with a dialog if it's not search-next
                    // or search-previous or if the search string doesn't exist.
                    if (LOWORD(wParam) == IDM_VT_SEARCH || !VT_IsSearchStringValid())
                    {
                        if (!DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_VTSEARCH),
                            hWnd, (DLGPROC)VTSearchProc))
                        {
                            break;
                        }

                        bInclusive = TRUE;
                    }

                    bPageChanged = VT_PerformSearch(hDC, &Rect, bInclusive, bReverse);
                }
                break;

            case IDM_VT_AUTOCODEPAGE:
                VT_SetAutoCodepage(hDC, &Rect, !VT_GetAutoCodepage());
                bHandled = TRUE;
                break;

            case IDM_VT_ANTIALIAS:
                VT_SetAntialias(hDC, &Rect, !VT_GetAntialias());
                bHandled = TRUE;
                break;

            case IDM_TELETEXT_KEY1:
            case IDM_TELETEXT_KEY2:
            case IDM_TELETEXT_KEY3:
            case IDM_TELETEXT_KEY4:
            case IDM_TELETEXT_KEY5:
                {
                    VT_ClearInput();
                    BYTE nFlofKey = LOWORD(wParam) - IDM_TELETEXT_KEY1;
                    bPageChanged = VT_PerformFlofKey(hDC, &Rect, nFlofKey);
                    bHandled = TRUE;
                }
                break;

            case IDM_TELETEXT_KEY6:
                VT_ClearInput();
                VT_SetShowHidden(hDC, &Rect, !VT_GetShowHidden());
                bHandled = TRUE;
                break;

            case IDM_CHARSET_TEST:
                VT_ClearInput();
                bPageChanged = VT_ShowTestPage(hDC, &Rect);
                bHandled = TRUE;
                break;
            }
        }
        break;

    case WM_LBUTTONDOWN:
        if (VT_GetState() != VT_OFF)
        {
            VT_ClearInput();
            bPageChanged = VT_ClickAtPosition(hDC, &Rect, LOWORD(lParam), HIWORD(lParam));
            if (bPageChanged != FALSE)
            {
                bHandled = TRUE;
            }
        }
        break;

    case WM_TIMER:
        {
            switch (LOWORD(wParam))
            {
            case TIMER_VTFLASHER:
                VT_RedrawFlash(hDC, &Rect);
                bHandled = TRUE;
                break;

            case TIMER_VTINPUT:
                VT_OnInputTimer(hDC, &Rect);
                bHandled = TRUE;
                break;
            }
        }
        break;

    case WM_CHAR:
        if (VT_GetState() != VT_OFF)
        {
            if (wParam >= '0' && wParam <= '9')
            {
                bPageChanged = VT_OnInput(hDC, &Rect, (char)wParam);
                bHandled = TRUE;
            }
        }
        break;

    case UWM_VIDEOTEXT:
        {
            switch(LOWORD(wParam))
            {
            case VTM_VTHEADERUPDATE:
                VT_ProcessHeaderUpdate(hDC, &Rect);
                bHandled = TRUE;
                break;

            case VTM_VTCOMMENTUPDATE:
                bPageChanged = VT_ProcessCommentUpdate(hDC, &Rect, lParam);
                bHandled = TRUE;
                break;

            case VTM_VTPAGEUPDATE:
                bPageChanged = VT_ProcessPageUpdate(hDC, &Rect, lParam);
                bHandled = TRUE;
                break;

            case VTM_VTPAGEREFRESH:
                bPageChanged = VT_ProcessPageRefresh(hDC, &Rect, lParam);
                bHandled = TRUE;
                break;
            }
        }
        break;
    }

    RECT PaintedRects[25];

    LONG nPaintedRects = VT_GetPaintedRects(PaintedRects, 25);

    if (nPaintedRects != 0)
    {
        RECT OSDDrawRect;
        GetDisplayAreaRect(hWnd, &OSDDrawRect);

        // Draw the OSD over the top
        OSD_Redraw(hDC, &OSDDrawRect);
        switch(GetActiveOutput()->Type()) 
        {
        case IOutput::OUT_OVERLAY:
            OffscreenHDC.BitBltRects(PaintedRects, nPaintedRects, hWndDC);
            break;
        case IOutput::OUT_D3D:        
            OffscreenHDC.BitBltRectsD3D(PaintedRects, nPaintedRects, ((CD3D9Output *)GetActiveOutput())->lpDDOSD);
            break;
        }
    }

    ReleaseDC(hWnd, hWndDC);

    if (bPageChanged != FALSE)
    {
        Cursor_VTUpdate();
    }

    return bHandled;
}


BOOL ProcessOSDMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    RECT Rect;
    BOOL bHandled = FALSE;
    HDC hDC;

    HDC hWndDC = GetDC(hWnd);
    GetDisplayAreaRect(hWnd, &Rect);

    if (OffscreenHDC.UpdateGeometry(hWndDC, &Rect))
    {
        // We should not use the offscreen buffer
        // until it is filled in WM_PAINT.
        hDC = hWndDC;
    }
    else
    {
        hDC = OffscreenHDC.GetBufferDC();
    }

    OSD_ResetPaintedRects();

    switch (uMsg)
    {
    case WM_COMMAND:
        if (LOWORD(wParam) == IDM_CLEAROSD)
        {
            OSD_Clear(hDC, &Rect);
            bHandled = TRUE;
        }
        break;

    case WM_TIMER:
        switch (LOWORD(wParam))
        {
        case OSD_TIMER_ID:
            OSD_Clear(hDC, &Rect);
            bHandled = TRUE;
            break;

        case OSD_TIMER_REFRESH_ID:
            OSD_RefreshInfosScreen(hDC, &Rect, 0);
            bHandled = TRUE;
            break;
        }
        break;

    case UWM_OSD:
        if (LOWORD(wParam) == OSDM_DISPLAYUPDATE)
        {
            OSD_ProcessDisplayUpdate(hDC, &Rect);
            bHandled = TRUE;
        }
        break;
    }

    if (hDC != hWndDC)
    {
        RECT PaintedRects[OSD_MAX_TEXT];

        LONG nPaintedRects = OSD_GetPaintedRects(PaintedRects, OSD_MAX_TEXT);

        if (nPaintedRects != 0)
        {            
            switch(GetActiveOutput()->Type()) 
            {
            case IOutput::OUT_OVERLAY:
                OffscreenHDC.BitBltRects(PaintedRects, nPaintedRects, hWndDC);
                break;
            case IOutput::OUT_D3D:
                OffscreenHDC.BitBltRectsD3D(PaintedRects, nPaintedRects, ((CD3D9Output *)GetActiveOutput())->lpDDOSD);
                break;
            }            
        }
    }

    ReleaseDC(hWnd, hWndDC);

    return bHandled;            
}


LRESULT CALLBACK KeyboardHookProc(int code, UINT wParam, UINT lParam)
{
    if(code >= 0 && bKeyboardLock)
    {
        if(!(bKeyboardLockMainWindowOnly && hWnd != GetFocus()))
        {
            // if it is not Ctrl+Shift+L do not pass the message to the rest of the hook chain 
            // or the target window procedure
            if(!((char)wParam == 'L' && GetKeyState(VK_SHIFT) < 0 && GetKeyState(VK_CONTROL) < 0))
            {
                return 1;
            }
        }
    }
       return CallNextHookEx(hKeyboardHook, code, wParam, lParam);
}


void SetKeyboardLock(BOOL Enabled)
{
    if(Enabled)
    {
        bKeyboardLock = TRUE;
        if(hKeyboardHook == NULL)
        {
            hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)KeyboardHookProc, NULL, 
                                             GetCurrentThreadId());
        }
    }
    else
    {
        bKeyboardLock = FALSE;
        if(hKeyboardHook != NULL)
        {
            UnhookWindowsHookEx(hKeyboardHook);
            hKeyboardHook = NULL;
        }
    }
}


void SetScreensaverMode(BOOL bScreensaverOff)
{
    BOOL bScreensaverMode = false;
    BOOL bLowpowerMode = false;
    BOOL bPoweroffMode = false;

    if( bScreensaverOff )
    {
        // Disable screensaver if enabled
        SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &bScreensaverMode, 0);
        if( bScreensaverMode )
        {
            // Disable
            SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, 0 , NULL, 0);
            bScreensaverDisabled = true;
        }

        // Disable monitor sleeping
        SystemParametersInfo(SPI_GETLOWPOWERACTIVE, 0, &bLowpowerMode, 0);
        if( bLowpowerMode )
        {
            // BOTH low-power and power-off needs to be disabled
            // to stop monitor energy saver from kicking in.
            SystemParametersInfo(SPI_SETLOWPOWERACTIVE, 0 , NULL, 0);
            bLowpowerDisabled = true;
        }

        // Disable monitor sleeping
        SystemParametersInfo(SPI_GETPOWEROFFACTIVE, 0, &bPoweroffMode, 0);
        if( bPoweroffMode )
        {
            SystemParametersInfo(SPI_SETPOWEROFFACTIVE, 0 , NULL, 0);
            bPoweroffDisabled = true;
        }
    }
    else
    {
        // Enable if disabled by us
        if( bScreensaverDisabled )
        {
            SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, 1, NULL, 0);
            bScreensaverDisabled = false;    
        }

        if( bLowpowerDisabled )
        {
            SystemParametersInfo(SPI_SETLOWPOWERACTIVE, 1, NULL, 0);
            bLowpowerDisabled = false;
        }

        if( bPoweroffDisabled )
        {
            SystemParametersInfo(SPI_SETPOWEROFFACTIVE, 1, NULL, 0);
            bPoweroffDisabled = false;
        }
    }
}

string UpdateSleepMode(TSMState* SMState)
{
    time_t curr = 0;
    struct tm* SMTime = NULL;
    UINT uiPeriod;
    string RetVal;

    switch( SMState->State )
    {
    case SM_WaitMode:
        // Called by press on delete key. 
        // Initial mode. (ie 1st press)
        
        // Show current sleep setting
        if( SMState->SleepAt != 0 ) 
        {
            KillTimer(hWnd, TIMER_SLEEPMODE);
            SMTime = localtime(&SMState->SleepAt);
            RetVal = MakeString() << "Sleep  " 
                                 << SMState->Period
                                 << " (" 
                                 << setw(2) << setfill('0') << SMTime->tm_hour
                                 << ":"
                                 << setw(2) << setfill('0') << SMTime->tm_min
                                 << ")";
        }
        else
        {
            RetVal = "Sleep OFF";
        }
        // Set update window
        SetTimer(hWnd, TIMER_SLEEPMODE, TIMER_SLEEPMODE_MS, NULL);

        // Goto change-mode (interprete next push on key as change-request)
        SMState->State = SM_ChangeMode;
        break;

    case SM_ChangeMode:
        // Called by press on delete key.
        // Subsequent presses within TIMER_SLEEPMODE_MS of previous press 
        
        // Next higher period (cycle)
        SMState->iPeriod = ++SMState->iPeriod % SMPeriodCount;
        SMState->Period = SMPeriods[SMState->iPeriod];

        // Set & show activation time
        if( SMState->Period != 0 ) 
        {
            curr = time(0);
            SMState->SleepAt = curr + SMState->Period * 60;
            SMTime = localtime(&SMState->SleepAt);
            RetVal = MakeString() << "New sleep  " 
                                 << SMState->Period
                                 << " (" 
                                 << setw(2) << setfill('0') << SMTime->tm_hour
                                 << ":"
                                 << setw(2) << setfill('0') << SMTime->tm_min
                                 << ")";
        }
        else
        {   
            SMState->SleepAt = 0;
            RetVal = "New sleep OFF";
        }
        
        // Restart change-mode timing
        KillTimer(hWnd, TIMER_SLEEPMODE);
        SetTimer(hWnd, TIMER_SLEEPMODE, TIMER_SLEEPMODE_MS, NULL);
        break;
        
    case SM_UpdateMode:
        // Mode set by WM_TIMER upon passing of TIMER_SLEEPMODE_MS
        curr = time(0);
        
        // Set timer to remainder of period if applicable
        if( SMState->SleepAt > curr )
        {
            uiPeriod = (UINT)(( SMState->SleepAt - curr ) * 1000);
            SetTimer(hWnd, TIMER_SLEEPMODE, uiPeriod, NULL);
        }
        else
        {
            // Passed sleepat-time in SM_Show or ChangeMode, stop asap
            if( SMState->SleepAt != 0 )
            {
                uiPeriod = 10;
                SetTimer(hWnd, TIMER_SLEEPMODE, uiPeriod, NULL);
            }
            else
            {
                // Sleep OFF
                ; 
            }
        }

        // Return to wait-mode.
        SMState->State = SM_WaitMode;
        break;
        
    default:
        ; //NEVER_GET_HERE
    }
    return RetVal;
}

void UpdatePriorityClass()
{
    if (PriorClassId == 2)
    {
        SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    }
    else if (PriorClassId == 1)
    {
        SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    }
    else
    {
        SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);        
    }
}

BOOL GetDisplayAreaRect(HWND hWnd, LPRECT lpRect, BOOL WithToolbar)
{
    BOOL result = GetClientRect(hWnd, lpRect);

    if(bIsFullScreen == TRUE) 
    {
        if (WithToolbar == FALSE && ToolbarControl != NULL)
        {
            ToolbarControl->AdjustArea(lpRect, 1);        
        }

        return result;
    }

    if (result)
    {
        if (IsStatusBarVisible())
        {
            lpRect->bottom -= StatusBar_Height();
        }

        if ((WindowBorder!=NULL) && WindowBorder->Visible())
        {
            WindowBorder->AdjustArea(lpRect,1);
        }

        if (ToolbarControl!=NULL)
        {
            ToolbarControl->AdjustArea(lpRect, 1);        
        }
    }
    return result;
}

void AddDisplayAreaRect(HWND hWnd, LPRECT lpRect)
{
    if (ToolbarControl!=NULL)
    {
        ToolbarControl->AdjustArea(lpRect, 0);    
    }

    if ((WindowBorder!=NULL) && WindowBorder->Visible())
    { 
       WindowBorder->AdjustArea(lpRect,0);
    }    
    
    if (IsStatusBarVisible())
    {
        lpRect->bottom += StatusBar_Height();
    }   
}

void InvalidateDisplayAreaRect(HWND hWnd, LPRECT lpRect, BOOL bErase)
{
    if (lpRect == NULL)
    {
        RECT rc;
        GetDisplayAreaRect(hWnd, &rc);
        InvalidateRect(hWnd,&rc,bErase);
    }
    else
    {
        InvalidateRect(hWnd,lpRect,bErase);
    }
}

BOOL BorderGetClientRect(HWND hWnd, LPRECT lpRect)
{
    BOOL result = GetClientRect(hWnd, lpRect);
    if (IsStatusBarVisible())
    {
       lpRect->bottom -= StatusBar_Height();
    }
    return result;
}

LRESULT BorderButtonProc(string sID, void* pThis, HWND hWndParent, UINT MouseFlags, HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (sID=="BUTTON_CLOSE")
    {
        switch(message)
        {
          case WM_LBUTTONUP:
              {
                  SendMessage(hWndParent,WM_CLOSE,0,0);                  
              }
              return TRUE;              
        }
    } 
    else if (sID=="BUTTON_MINIMIZE")
    {
        switch(message)
        {
          case WM_LBUTTONUP:
              {
                  ShowWindow(hWndParent,SW_MINIMIZE);
              }
              return TRUE;              
        }
    } 
    else if (sID=="BUTTON_MAXIMIZE")
    {
        switch(message)
        {
          case WM_LBUTTONUP:
              {
                  SendMessage(hWnd, WM_COMMAND, IDM_FULL_SCREEN, 0);                  
              }
              return TRUE;              
        }
    } 
    else if (sID=="BUTTON_SIZE")
    {
        switch(message)
        {
          case WM_NCHITTEST:
              {
                  if (MouseFlags & MK_LBUTTON)
                  {
                      return ::DefWindowProc(hWndParent, WM_NCLBUTTONDOWN, HTBOTTOMRIGHT, lParam);                                            
                  }
              }
              break;
          case WM_SETCURSOR:
            static HCURSOR hResizeCursor = NULL;
            if (hResizeCursor == NULL)
            {
                hResizeCursor = LoadCursor(NULL,IDC_SIZENWSE);
            }
            if (hResizeCursor != NULL)
            {
                SetCursor(hResizeCursor);
            }
            return TRUE;
        }
    }
    else if (sID=="BUTTON_SIDEBAR")
    {
        switch(message)
        {
          case WM_LBUTTONUP:
              {                  
              }
              return TRUE;              
        }
    }
    return FALSE;
}

LPCSTR GetSkinDirectory()
{
    if (szSkinDirectory[0] != 0)
    {
        return szSkinDirectory;
    }

    char szPath[MAX_PATH+1];
    char* s = NULL;
    int len = GetFullPathName(GetIniFileForSettings(), MAX_PATH, szPath, &s);
    if ((len > 0) && (s!=NULL))
    {
        *s = 0;            
    }
    else
    {
        GetCurrentDirectory(MAX_PATH, szPath);
        strcat(szPath,"\\");
    }
    strcpy(szSkinDirectory,szPath);        
    strcat(szSkinDirectory,"Skins\\");    

    return szSkinDirectory;
}

void SetWindowBorder(HWND hWnd, LPCSTR szSkinName, BOOL bShow)
{
    if (WindowBorder==NULL) 
    {                
        if ((szSkinName == NULL) || (szSkinName[0] == 0))
        {
            //Don't make the windowborder unless it is necessary
            return;
        }
        WindowBorder = new CWindowBorder(hWnd, hDScalerInst, BorderGetClientRect);

        //Test border (white)
        //WindowBorder->SolidBorder(10,10,10,10, 0xFFFFFF);        
    }

    if ((szSkinName != NULL) && (szSkinName[0] == 0))
    {
        WindowBorder->ClearSkin();
    }

    if ((szSkinName != NULL) && (szSkinName[0] != 0))
    {
        char szSkinIniFile[MAX_PATH*2];
        strcpy(szSkinIniFile,GetSkinDirectory());
        strcat(szSkinIniFile,szSkinName);
        strcat(szSkinIniFile,"\\skin.ini");
        ///\todo check if the ini file exists

        //Add border buttons
        WindowBorder->RegisterButton("BUTTON_CLOSE",BITMAPASBUTTON_PUSH,"ButtonClose","ButtonCloseMouseOver","ButtonCloseClick", BorderButtonProc);
        WindowBorder->RegisterButton("BUTTON_SIZE",BITMAPASBUTTON_PUSH,"ButtonSize","ButtonSizeMouseOver","ButtonSizeClick", BorderButtonProc);
        WindowBorder->RegisterButton("BUTTON_MINIMIZE",BITMAPASBUTTON_PUSH,"ButtonMinimize","ButtonMinimizeMouseOver","ButtonMinimizeClick", BorderButtonProc);
        WindowBorder->RegisterButton("BUTTON_MAXIMIZE",BITMAPASBUTTON_PUSH,"ButtonMaximize","ButtonMaximizeMouseOver","ButtonMaximizeClick", BorderButtonProc);
        //WindowBorder->RegisterButton("BUTTON_SIDEBAR",BITMAPASBUTTON_PUSH,"ButtonSideBar","ButtonSideBarMouseOver","ButtonSideBarClick", BorderButtonProc);        
        
        vector<int>Results;        
        WindowBorder->LoadSkin(szSkinIniFile,"Border",&Results);

        ///\todo Process errors    
    }
    
    if (bShow && !bIsFullScreen && ((szSkinName == NULL) || (szSkinName[0]!=0)))
    {
        WindowBorder->Show();                        
    }
    else
    {
        WindowBorder->Hide();
    }
}

void Skin_SetMenu(HMENU hMenu, BOOL bUpdateOnly)
{
    if (!bUpdateOnly)
    {                
        vSkinNameList.clear();

        //Find sub directories with skin.ini files
        WIN32_FIND_DATA FindFileData;
        HANDLE hFind;                
        char szSearch[MAX_PATH+10];

        strcpy(szSearch,GetSkinDirectory());        
        strcat(szSearch,"*.*");
        hFind = FindFirstFile(szSearch, &FindFileData);
        if (hFind != INVALID_HANDLE_VALUE) 
        {
            do           
            {
                if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    && strcmp(FindFileData.cFileName,".") && strcmp(FindFileData.cFileName,"..") )
                {
                    char szSearchFile[MAX_PATH];
                    WIN32_FIND_DATA FindFileData2;
                    HANDLE hFind2;

                    strcpy(szSearchFile,GetSkinDirectory());
                    strcat(szSearchFile,FindFileData.cFileName);
                    strcat(szSearchFile,"\\skin.ini");
                    if ((hFind2=FindFirstFile(szSearchFile, &FindFileData2)) != INVALID_HANDLE_VALUE)
                    {
                        FindClose(hFind2);
                        vSkinNameList.push_back(FindFileData.cFileName);
                    }
                }
            } while (FindNextFile(hFind, &FindFileData));
            FindClose(hFind);       
        }        

        //Make menu

        // Find submenu
        char string[256];
        string[0] = '\0';
        GetMenuString(hMenu, 2, string, sizeof(string), MF_BYPOSITION);
        int reduc1 = !strcmp(string, "&Channels") ? 0 : 1;        
        HMENU hViewMenu = GetSubMenuWithName(hMenu, 3-reduc1, "&View");        
        HMENU hSkinMenu = NULL;
        for (int i = 0; i < GetMenuItemCount(hViewMenu); i++)
        {
            if (GetMenuItemID(GetSubMenu(hViewMenu, i), 0) == IDM_SKIN_NONE)
            {
                hSkinMenu = GetSubMenu(hViewMenu, i);
                break;
            }
        }

        if (hSkinMenu != NULL)
        {
            int num = GetMenuItemCount(hSkinMenu);
            size_t i;
            for (i = 2; static_cast<int>(i) < num; i++)
            {
                DeleteMenu(hSkinMenu, 2, MF_BYPOSITION);
            }
            
            MENUITEMINFO MenuItemInfo;
            memset(&MenuItemInfo, 0, sizeof(MenuItemInfo));            
            for (i = 0; i < vSkinNameList.size(); i++)
            {
                MenuItemInfo.cbSize = sizeof (MenuItemInfo);
                MenuItemInfo.fMask = MIIM_TYPE | MIIM_ID;
                MenuItemInfo.fType = MFT_STRING;
                MenuItemInfo.cch = sizeof(vSkinNameList[i].c_str());
                MenuItemInfo.dwTypeData = (LPSTR)vSkinNameList[i].c_str();
                MenuItemInfo.wID = IDM_SKIN_FIRST + i;
                InsertMenuItem(hSkinMenu, i+2, TRUE, &MenuItemInfo);
            }
        }
    }

    CheckMenuItemBool(hMenu, IDM_SKIN_NONE, ((vSkinNameList.size()==0) || (szSkinName[0] == 0)));
        
    int Found = 0;
    for (size_t i = 0; i < vSkinNameList.size(); i++)
    {
        if ((Found == 0) && (vSkinNameList[i] == szSkinName))
        {
            Found = 1;
        }
        CheckMenuItemBool(hMenu, IDM_SKIN_FIRST+i, (Found==1));
        if (Found==1) 
        {
            Found=2;
        }
    }    
}

///**************************************************************************
//
//    FUNCTION: MainWndProc(HWND, unsigned, WORD, LONG)
//
//    PURPOSE:  Processes messages
//
//    MESSAGES:
//
//        WM_COMMAND    - application menu (About dialog box)
//        WM_CREATE     - create window and objects
//        WM_PAINT      - update window, draw objects
//        WM_DESTROY    - destroy window
//
//    COMMENTS:
//
//        Handles to the objects you will use are obtained when the WM_CREATE
//        message is received, and deleted when the WM_DESTROY message is
//        received.  The actual drawing is done whenever a WM_PAINT message is
//        received.
//
//
///**************************************************************************
LONG APIENTRY MainWndProc(HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
    int i;
    BOOL bDone = FALSE;
    ISetting* pSetting = NULL;
    ISetting* pSetting2 = NULL;
    ISetting* pSetting3 = NULL;
    ISetting* pSetting4 = NULL;
    TGUIRequest req;

    if (message == MsgWheel)
    {
        // crack the mouse wheel delta
        // +ve is forward (away from user)
        // -ve is backward (towards user)
        if((short)wParam > 0)
        {
            if(GetKeyState(VK_SHIFT) < 0)
            {
                PostMessage(hWnd, WM_COMMAND, IDM_VOLUMEPLUS, 0);
            }
            else if(GetKeyState(VK_CONTROL) < 0)
            {
                PostMessage(hWnd, WM_COMMAND, IDM_PLAYLIST_PREVIOUS, 0);
            }
            else
            {
                PostMessage(hWnd, WM_COMMAND, IDM_CHANNELPLUS, 0);
            }
        }
        else
        {
            if(GetKeyState(VK_SHIFT) < 0)
            {
                PostMessage(hWnd, WM_COMMAND, IDM_VOLUMEMINUS, 0);
            }
            else if(GetKeyState(VK_CONTROL) < 0)
            {
                PostMessage(hWnd, WM_COMMAND, IDM_PLAYLIST_NEXT, 0);
            }
            else
            {
                PostMessage(hWnd, WM_COMMAND, IDM_CHANNELMINUS, 0);
            }
        }
    }
    else if(message == MsgOSDShow)
    {
        HANDLE hMapFile = NULL;
        char* lpMsg = NULL;
        char msg[1024];
        
        // show error message if an error occurred with the other instance of DScaler
        if(lParam != 0)
        {
            sprintf(msg, "Error processing incoming message. (#%i 0x%x)", lParam, wParam);
            LOG(0, msg);
            OSD_ShowTextPersistent(msg, 5);
        }
        else
        {
            hMapFile = OpenFileMapping(FILE_MAP_READ, FALSE, "DScalerSendMessageFileMappingObject");
            
            if(hMapFile == NULL) 
            { 
                sprintf(msg, "Error processing incoming message. (#10 0x%x)", GetLastError());
                LOG(0, msg); 
            } 
            else
            {
                lpMsg = (char*)MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 1024);              
                
                if (lpMsg == NULL) 
                { 
                    sprintf(msg, "Error processing incoming message. (#11 0x%x)", GetLastError());
                    LOG(0, msg); 
                }
                else
                {
                    strncpy(msg, lpMsg, sizeof(msg));
                }
            }
            // convert "\n" to newline characters
            char* s;
            while((s = strstr(msg,"\\n")) != NULL)
            {
                s[0] = '\n';
                strncpy(&s[1], &s[2], strlen(&s[1]));
            }
            
            if(wParam == 0 && hMapFile != NULL && lpMsg != NULL)
            {
                OSD_ShowText(msg, 5);
            }
            else
            {
                OSD_ShowTextPersistent(msg, 5);
            }
            
            if(lpMsg != NULL)
            {
                UnmapViewOfFile(lpMsg);
            }
            if(hMapFile != NULL)
            {
                CloseHandle(hMapFile);
            }
        }
        return 0;
    }
    else if(message == MsgTaskbarRestart)
    // the task bar has been restarted so the systray icon needs to be added again.
    {
        bIconOn = FALSE;
        if (bMinToTray)
        {
            SetTray(TRUE);
        }
    }
    
    if (message == IDI_TRAYICON)
    {
        return On_IconHandler(wParam, lParam);
    }

    switch (message)
    {

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_MUTE:
            Audio_SetUserMute(!Audio_GetUserMute());
            ShowText(hWnd, Audio_GetUserMute() ? "MUTE" : "UNMUTE");
            break;

        case IDC_TOOLBAR_VOLUME_MUTE:
            Audio_SetUserMute(lParam);
            ShowText(hWnd, lParam ? "MUTE" : "UNMUTE");
            break;
            
        case IDM_VOLUMEPLUS:
            if (Audio_GetUserMute() == TRUE)
            {
                Audio_SetUserMute(FALSE);
                ShowText(hWnd, "UNMUTE");
            }
            else
            {
                if (!Mixer_IsEnabled())
                {
                    ISetting* pSetting = Providers_GetCurrentSource()->GetVolume();
                    if(pSetting != NULL)
                    {
                        pSetting->ChangeValue(ADJUSTUP);                        
                    }
                    else
                    {
                        ShowText(hWnd, "Volume not supported");
                    }
                }
                else
                {
                    Mixer_Volume_Up();
                    ostringstream oss;
                    oss << "Mixer-Volume " << Mixer_GetVolume();
                    ShowText(hWnd, oss.str());
                }
            }
            break;

        case IDM_VOLUMEMINUS:
            if (Audio_GetUserMute() == TRUE)
            {
                Audio_SetUserMute(FALSE);
                ShowText(hWnd, "UNMUTE");
            }
            else
            {
                if (!Mixer_IsEnabled())
                {
                    ISetting* pSetting = Providers_GetCurrentSource()->GetVolume();
                    if(pSetting != NULL)
                    {
                        pSetting->ChangeValue(ADJUSTDOWN);
                    }
                    else
                    {
                        ShowText(hWnd, "Volume not supported");
                    }
                }
                else
                {
                    Mixer_Volume_Down();
                    ostringstream oss;
                    oss << "Mixer-Volume " << Mixer_GetVolume();
                    ShowText(hWnd, oss.str());
                }
            }
            break;

        case IDC_TOOLBAR_VOLUME_SLIDER:
            if (Audio_GetUserMute() == TRUE)
            {
                Audio_SetUserMute(FALSE);
            }
            if (!Mixer_IsEnabled())
            {
                ISetting* pSetting = Providers_GetCurrentSource()->GetVolume();
                if(pSetting != NULL)
                {
                    pSetting->SetValue(lParam);
                }
                else
                {
                    ShowText(hWnd, "Volume not supported");
                }
            }
            else
            {
                extern void Mixer_SetVolume(long volume);
            
                Mixer_SetVolume(lParam);
                ostringstream oss;
                oss << "Mixer-Volume " << Mixer_GetVolume();
                ShowText(hWnd, oss.str());
            }
            break;
    
        case IDM_AUTO_FORMAT:
            if(Setting_GetValue(Timing_GetSetting(AUTOFORMATDETECT)))
            {
                ShowText(hWnd, "Auto Format Detection OFF");
                Setting_SetValue(Timing_GetSetting(AUTOFORMATDETECT), FALSE);
            }
            else
            {
                ShowText(hWnd, "Auto Format Detection ON");
                Setting_SetValue(Timing_GetSetting(AUTOFORMATDETECT), TRUE);
            }
            break;

        case IDM_VT_SEARCH:
        case IDM_VT_SEARCHNEXT:
        case IDM_VT_SEARCHPREV:
            ProcessVTMessage(hWnd, message, wParam, lParam);
            break;

        case IDM_VT_PAGE_MINUS:
            if (pMultiFrames)
            {
                pMultiFrames->HandleWindowsCommands(hWnd, wParam, lParam);
            }
            else if (!ProcessVTMessage(hWnd, message, wParam, lParam))
            {
                ProcessAspectRatioSelection(hWnd, LOWORD(wParam));
            }
            break;

        case IDM_VT_PAGE_PLUS:
            if (pMultiFrames)
            {
                pMultiFrames->HandleWindowsCommands(hWnd, wParam, lParam);
            }
            else if (!ProcessVTMessage(hWnd, message, wParam, lParam))
            {
                ProcessAspectRatioSelection(hWnd, LOWORD(wParam));
            }
            break;

        case IDM_VT_PAGE_UP:
            if (pMultiFrames)
            {
                pMultiFrames->HandleWindowsCommands(hWnd, wParam, lParam);
            }
            else if(!ProcessVTMessage(hWnd, message, wParam, lParam))
            {
                ProcessAspectRatioSelection(hWnd, LOWORD(wParam));
            }
            break;

        case IDM_VT_PAGE_DOWN:
            if (pMultiFrames)
            {
                pMultiFrames->HandleWindowsCommands(hWnd, wParam, lParam);
            }
            else if(!ProcessVTMessage(hWnd, message, wParam, lParam))
            {
                ProcessAspectRatioSelection(hWnd, LOWORD(wParam));
            }
            break;

        case IDM_CHANNEL_LIST:
            if(Providers_GetCurrentSource() && Providers_GetCurrentSource()->HasTuner())
            {
                if(!Providers_GetCurrentSource()->IsInTunerMode())
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT1, 0);
                }
                if (pMultiFrames && pMultiFrames->IsActive())
                {
                    pMultiFrames->RequestSwitch();
                }
                DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_CHANNELLIST), hWnd, (DLGPROC) ProgramListProc);
                Channels_UpdateMenu(hMenu);
                EventCollector->RaiseEvent(NULL, EVENT_CHANNELLIST_CHANGE, 0, 1);
            }
            break;

        case IDM_CHANNELPLUS:
            if (!ProcessVTMessage(hWnd, message, wParam, lParam))
            {
                if (Providers_GetCurrentSource()->IsInTunerMode())
                {
                    // IDM_CHANNELPLUS and IDM_CHANNELMINUS are used
                    // as the hub of all user interface channel up/down
                    // commands.  Put bReverseChannelScroll check here
                    // so mousewheel/page keys/toolbar arrows are taken
                    // care of.
                    if (!bReverseChannelScroll)
                    {
                        Channel_Increment();
                    }
                    else
                    {
                        Channel_Decrement();
                    }
                    if (pMultiFrames && pMultiFrames->IsActive())
                    {
                        // We sleep to be sure that the channel is correctly displayed
                        // in the output thread before acknowledging the change of content
                        Sleep(250);
                        pMultiFrames->AckContentChange();
                    }
                }
            }
            // return instead of break. SetMenuAnalog() is called otherwise. This adds a delay with some
            // sources since the audio signal menu entries are updated and the audio signal status is 
            // unknown since we have just switched channels.
            return 0;

        case IDM_CHANNELMINUS:
            if (!ProcessVTMessage(hWnd, message, wParam, lParam))
            {
                if (Providers_GetCurrentSource()->IsInTunerMode())
                {
                    if (!bReverseChannelScroll)
                    {
                        Channel_Decrement();
                    }
                    else
                    {
                        Channel_Increment();
                    }
                    if (pMultiFrames && pMultiFrames->IsActive())
                    {
                        // We sleep to be sure that the channel is correctly displayed
                        // in the output thread before acknowledging the change of content
                        Sleep(250);
                        pMultiFrames->AckContentChange();
                    }
                }
            }
            // return instead of break. SetMenuAnalog() is called otherwise. This adds a delay with some
            // sources since the audio signal menu entries are updated and the audio signal status is 
            // unknown since we have just switched channels.
            return 0;

        case IDM_CHANNEL_PREVIOUS:
            if (!ProcessVTMessage(hWnd, message, wParam, lParam))
            {
                if (Providers_GetCurrentSource()->IsInTunerMode())
                {
                    Channel_Previous();
                }
            }
            // return instead of break. SetMenuAnalog() is called otherwise. This adds a delay with some
            // sources since the audio signal menu entries are updated and the audio signal status is 
            // unknown since we have just switched channels.
            return 0;

        case IDM_CHANNEL_PREVIEW:
            if (pMultiFrames)
            {
                pMultiFrames->RequestSwitch();
            }
            else if (Providers_GetCurrentSource() && Providers_GetCurrentSource()->IsInTunerMode())
            {
                pMultiFrames = new CMultiFrames(PREVIEW_CHANNELS, ChannelPreviewNbCols, ChannelPreviewNbRows, Providers_GetCurrentSource());
                pMultiFrames->RequestSwitch();
            }
            else
            {
                SendMessage(hWnd, WM_COMMAND, IDM_PLAYLIST_PREVIEW, 0);
            }
            break;

        case IDM_CHANNEL_INDEX:
            if (Providers_GetCurrentSource()->IsInTunerMode() && pMultiFrames && pMultiFrames->IsActive())
            {
                Channel_Change(lParam, 1);
                if (pMultiFrames && pMultiFrames->IsActive())
                {
                    // We sleep to be sure that the channel is correctly displayed
                    // in the output thread before acknowledging the change of content
                    Sleep(250);
                    pMultiFrames->AckContentChange();
                }
            }
            // return instead of break. SetMenuAnalog() is called otherwise. This adds a delay with some
            // sources since the audio signal menu entries are updated and the audio signal status is 
            // unknown since we have just switched channels.
            return 0;

        case IDC_TOOLBAR_CHANNELS_LIST:
            if (!ProcessVTMessage(hWnd, message, wParam, lParam))
            {
                if (Providers_GetCurrentSource()->IsInTunerMode())
                {
                    Channel_Change(lParam);
                }
            }
            // return instead of break. SetMenuAnalog() is called otherwise. This adds a delay with some
            // sources since the audio signal menu entries are updated and the audio signal status is 
            // unknown since we have just switched channels.
            return 0;

            
        case IDM_PATTERN_SELECT:
            pCalibration->SelectTestPattern(lParam);
            break;

        case IDM_START_AUTO_CALIBRATION:
            pCalibration->Start(CAL_AUTO_FULL);
            break;

        case IDM_START_AUTO_CALIBRATION2:
            pCalibration->Start(CAL_AUTO_BRIGHT_CONTRAST);
            break;

        case IDM_START_AUTO_CALIBRATION3:
            pCalibration->Start(CAL_AUTO_COLOR);
            break;

        case IDM_START_MANUAL_CALIBRATION:
            pCalibration->Start(CAL_MANUAL);
            break;

        case IDM_START_YUV_RANGE:
            pCalibration->Start(CAL_CHECK_YUV_RANGE);
            break;

        case IDM_STOP_CALIBRATION:
            pCalibration->Stop();
            break;

        case IDM_RESET:
            Reset_Capture();
            Sleep(100);
            Providers_GetCurrentSource()->UnMute();
            break;

        case IDM_TOGGLE_MENU:
            bShowMenu = !bShowMenu;
            UpdateWindowState();
            WorkoutOverlaySize(TRUE);
            break;

        case IDM_SLEEPMODE:
            ShowText(hWnd, UpdateSleepMode(&SMState));
            break;
        
        case IDM_AUTODETECT:
            KillTimer(hWnd, TIMER_FINDPULL);
            if(Setting_GetValue(OutThreads_GetSetting(AUTODETECT)))
            {
                ShowText(hWnd, "Auto Pulldown Detection OFF");
                Setting_SetValue(OutThreads_GetSetting(AUTODETECT), FALSE);
            }
            else
            {
                ShowText(hWnd, "Auto Pulldown Detection ON");
                Setting_SetValue(OutThreads_GetSetting(AUTODETECT), TRUE);
            }
            // Set Deinterlace Mode to film fallback in
            // either case
            if(GetTVFormat(Providers_GetCurrentSource()->GetFormat())->Is25fps)
            {
                SetVideoDeinterlaceIndex(Setting_GetValue(FD50_GetSetting(PALFILMFALLBACKMODE)));
            }
            else
            {
                SetVideoDeinterlaceIndex(Setting_GetValue(FD60_GetSetting(NTSCFILMFALLBACKMODE)));
            }
            break;

        case IDM_FINDLOCK_PULL:
            if(!Setting_GetValue(OutThreads_GetSetting(AUTODETECT)))
            {
                Setting_SetValue(OutThreads_GetSetting(AUTODETECT), TRUE);
            }
            // Set Deinterlace Mode to film fallback in
            // either case
            if(GetTVFormat(Providers_GetCurrentSource()->GetFormat())->Is25fps)
            {
                SetVideoDeinterlaceIndex(Setting_GetValue(FD50_GetSetting(PALFILMFALLBACKMODE)));
            }
            else
            {
                SetVideoDeinterlaceIndex(Setting_GetValue(FD60_GetSetting(NTSCFILMFALLBACKMODE)));
            }
            SetTimer(hWnd, TIMER_FINDPULL, TIMER_FINDPULL_MS, NULL);
//            ShowText(hWnd, "Searching Film mode ...");
            break;

        case IDM_FALLBACK:
            if(Setting_GetValue(FD60_GetSetting(FALLBACKTOVIDEO)))
            {
                ShowText(hWnd, "Fallback on Bad Pulldown OFF");
                Setting_SetValue(FD60_GetSetting(FALLBACKTOVIDEO), FALSE);
            }
            else
            {
                ShowText(hWnd, "Fallback on Bad Pulldown ON");
                Setting_SetValue(FD60_GetSetting(FALLBACKTOVIDEO), TRUE);
            }
            break;

        case IDM_22PULLODD:
        case IDM_22PULLEVEN:
        case IDM_32PULL1:
        case IDM_32PULL2:
        case IDM_32PULL3:
        case IDM_32PULL4:
        case IDM_32PULL5:
            KillTimer(hWnd, TIMER_FINDPULL);
            Setting_SetValue(OutThreads_GetSetting(AUTODETECT), FALSE);
            SetFilmDeinterlaceMode((eFilmPulldownMode)(LOWORD(wParam) - IDM_22PULLODD));
            ShowText(hWnd, GetDeinterlaceModeName());
            break;

        case IDM_ABOUT:
            DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutProc);
            break;

        case IDM_BRIGHTNESS_PLUS:
            if((pSetting = Providers_GetCurrentSource()->GetBrightness()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTUP_SILENT);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_BRIGHTNESS_CURRENT, 0);
            break;

        case IDM_BRIGHTNESS_MINUS:
            if((pSetting = Providers_GetCurrentSource()->GetBrightness()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTDOWN_SILENT);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_BRIGHTNESS_CURRENT, 0);
            break;

        case IDM_BRIGHTNESS_CURRENT:
            if((pSetting = Providers_GetCurrentSource()->GetBrightness()) != NULL)
            {
                pSetting->OSDShow();
            }
            else
            {
                ShowText(hWnd, "No Brightness Control");
            }
            break;

        case IDM_KONTRAST_PLUS:
            if((pSetting = Providers_GetCurrentSource()->GetContrast()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTUP_SILENT);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_KONTRAST_CURRENT, 0);
            break;

        case IDM_KONTRAST_MINUS:
            if((pSetting = Providers_GetCurrentSource()->GetContrast()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTDOWN_SILENT);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_KONTRAST_CURRENT, 0);
            break;

        case IDM_KONTRAST_CURRENT:
            if((pSetting = Providers_GetCurrentSource()->GetContrast()) != NULL)
            {
                pSetting->OSDShow();
            }
            else
            {
                ShowText(hWnd, "No Contrast Control");
            }
            break;

        case IDM_USATURATION_PLUS:
            if((pSetting = Providers_GetCurrentSource()->GetSaturationU()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTUP_SILENT);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_USATURATION_CURRENT, 0);
            break;
        
        case IDM_USATURATION_MINUS:
            if((pSetting = Providers_GetCurrentSource()->GetSaturationU()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTDOWN_SILENT);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_USATURATION_CURRENT, 0);
            break;

        case IDM_USATURATION_CURRENT:
            if((pSetting = Providers_GetCurrentSource()->GetSaturationU()) != NULL)
            {
                pSetting->OSDShow();
            }
            else
            {
                ShowText(hWnd, "No Saturation U Control");
            }
            break;
        
        case IDM_VSATURATION_PLUS:
            if((pSetting = Providers_GetCurrentSource()->GetSaturationV()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTUP_SILENT);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_VSATURATION_CURRENT, 0);
            break;

        case IDM_VSATURATION_MINUS:
            if((pSetting = Providers_GetCurrentSource()->GetSaturationV()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTDOWN_SILENT);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_VSATURATION_CURRENT, 0);
            break;

        case IDM_VSATURATION_CURRENT:
            if((pSetting = Providers_GetCurrentSource()->GetSaturationV()) != NULL)
            {
                pSetting->OSDShow();
            }
            else
            {
                ShowText(hWnd, "No Saturation V Control");
            }
            break;

        case IDM_COLOR_PLUS:
            if((pSetting = Providers_GetCurrentSource()->GetSaturation()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTUP_SILENT);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_COLOR_CURRENT, 0);
            break;

        case IDM_COLOR_MINUS:
            if((pSetting = Providers_GetCurrentSource()->GetSaturation()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTDOWN_SILENT);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_COLOR_CURRENT, 0);
            break;

        case IDM_COLOR_CURRENT:
            if((pSetting = Providers_GetCurrentSource()->GetSaturation()) != NULL)
            {
                pSetting->OSDShow();
            }
            else
            {
                ShowText(hWnd, "No Saturation Control");
            }
            break;

        case IDM_HUE_PLUS:
            if((pSetting = Providers_GetCurrentSource()->GetHue()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTUP_SILENT);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_HUE_CURRENT, 0);
            break;

        case IDM_HUE_MINUS:
            if((pSetting = Providers_GetCurrentSource()->GetHue()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTDOWN_SILENT);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_HUE_CURRENT, 0);
            break;

        case IDM_HUE_CURRENT:
            if((pSetting = Providers_GetCurrentSource()->GetHue()) != NULL)
            {
                pSetting->OSDShow();
            }
            else
            {
                ShowText(hWnd, "No Hue Control");
            }
            break;

        case IDM_OVERSCAN_PLUS:
            pSetting = Providers_GetCurrentSource()->GetTopOverscan();
            pSetting2 = Providers_GetCurrentSource()->GetBottomOverscan();
            pSetting3 = Providers_GetCurrentSource()->GetLeftOverscan();
            pSetting4 = Providers_GetCurrentSource()->GetRightOverscan();
            if ( ( (pSetting == NULL) || (pSetting->GetValue() != pSetting->GetMax()) )
              && ( (pSetting2 == NULL) || (pSetting2->GetValue() != pSetting2->GetMax()) )
              && ( (pSetting3 == NULL) || (pSetting3->GetValue() != pSetting3->GetMax()) )
              && ( (pSetting4 == NULL) || (pSetting4->GetValue() != pSetting4->GetMax()) ) )
            {
                if(pSetting != NULL)
                {
                    pSetting->ChangeValue(ADJUSTUP_SILENT);
                }
                if(pSetting2 != NULL)
                {
                    pSetting2->ChangeValue(ADJUSTUP_SILENT);
                }
                if(pSetting3 != NULL)
                {
                    pSetting3->ChangeValue(ADJUSTUP_SILENT);
                }
                if(pSetting4 != NULL)
                {
                    pSetting4->ChangeValue(ADJUSTUP_SILENT);
                }
            }
            SendMessage(hWnd, WM_COMMAND, IDM_OVERSCAN_CURRENT, 0);
            break;

        case IDM_OVERSCAN_MINUS:
            pSetting = Providers_GetCurrentSource()->GetTopOverscan();
            pSetting2 = Providers_GetCurrentSource()->GetBottomOverscan();
            pSetting3 = Providers_GetCurrentSource()->GetLeftOverscan();
            pSetting4 = Providers_GetCurrentSource()->GetRightOverscan();
            if ( ( (pSetting == NULL) || (pSetting->GetValue() != pSetting->GetMin()) )
              && ( (pSetting2 == NULL) || (pSetting2->GetValue() != pSetting2->GetMin()) )
              && ( (pSetting3 == NULL) || (pSetting3->GetValue() != pSetting3->GetMin()) )
              && ( (pSetting4 == NULL) || (pSetting4->GetValue() != pSetting4->GetMin()) ) )
            {
                if(pSetting != NULL)
                {
                    pSetting->ChangeValue(ADJUSTDOWN_SILENT);
                }
                if(pSetting2 != NULL)
                {
                    pSetting2->ChangeValue(ADJUSTDOWN_SILENT);
                }
                if(pSetting3 != NULL)
                {
                    pSetting3->ChangeValue(ADJUSTDOWN_SILENT);
                }
                if(pSetting4 != NULL)
                {
                    pSetting4->ChangeValue(ADJUSTDOWN_SILENT);
                }
            }
            SendMessage(hWnd, WM_COMMAND, IDM_OVERSCAN_CURRENT, 0);
            break;

        case IDM_OVERSCAN_CURRENT:
            if(Providers_GetCurrentSource()->GetTopOverscan() == NULL
            && Providers_GetCurrentSource()->GetBottomOverscan() == NULL
            && Providers_GetCurrentSource()->GetLeftOverscan() == NULL
            && Providers_GetCurrentSource()->GetRightOverscan() == NULL)
            {
                ShowText(hWnd, "No Overscan Control");
            }
            else
            {
                string Text("Overscan");
                if((pSetting = Providers_GetCurrentSource()->GetTopOverscan()) != NULL)
                {
                    Text +=  "\nTop ";
                    Text += ToString(pSetting->GetValue());
                }
                if((pSetting = Providers_GetCurrentSource()->GetBottomOverscan()) != NULL)
                {
                    Text +=  "\nBottom ";
                    Text += ToString(pSetting->GetValue());
                }
                if((pSetting = Providers_GetCurrentSource()->GetLeftOverscan()) != NULL)
                {
                    Text +=  "\nLeft ";
                    Text += ToString(pSetting->GetValue());
                }
                if((pSetting = Providers_GetCurrentSource()->GetRightOverscan()) != NULL)
                {
                    Text +=  "\nRight ";
                    Text += ToString(pSetting->GetValue());
                }
                OSD_ShowText(Text, 0);
            }
            break;

        case IDM_TOGGLECURSOR:
            if(!bAutoHideCursor && bIsFullScreen == FALSE)
            {
                bShowCursor = !bShowCursor;
                Cursor_UpdateVisibility();
            }
            break;

        case IDM_END:
            if (((CStillSource*)Providers_GetSnapshotsSource())->IsOneItemInMemory())
            {
                if (MessageBox(hWnd,
                               "At least one of your snapshots is not yet saved in a file.\n"
                               "Do you confirm that you want to exit without saving it?", 
                               "DScaler - Unsaved Snapshots", 
                               MB_YESNO | MB_ICONQUESTION | MB_APPLMODAL) == IDNO)
                {
                    break;
                } 
            }
            ShowWindow(hWnd, SW_HIDE);
            PostMessage(hWnd, WM_DESTROY, wParam, lParam);
            break;

        case IDM_TRAYEND:
            SendMessage(hWnd, WM_COMMAND, IDM_END, NULL);
            break;

        case IDM_VBI_VT:
            if (VT_GetState() != VT_OFF)
            {
                VT_SetState(NULL, NULL, VT_OFF);
                Cursor_VTUpdate();
                InvalidateDisplayAreaRect(hWnd, NULL, FALSE);
            }
            Setting_SetValue(VBI_GetSetting(DOTELETEXT),
                !Setting_GetValue(VBI_GetSetting(DOTELETEXT)));
            break;

        case IDM_CCOFF:
        case IDM_CC1:
        case IDM_CC2:
        case IDM_CC3:
        case IDM_CC4:
        case IDM_TEXT1:
        case IDM_TEXT2:
        case IDM_TEXT3:
        case IDM_TEXT4:
            Setting_SetValue(VBI_GetSetting(CLOSEDCAPTIONMODE), 
                LOWORD(wParam) - IDM_CCOFF);
            break;

        case IDM_VBI_VPS:
            Setting_SetValue(VBI_GetSetting(DOVPS), 
                !Setting_GetValue(VBI_GetSetting(DOVPS)));
            break;

        case IDM_VBI_WSS:
            Setting_SetValue(VBI_GetSetting(DOWSS), 
                !Setting_GetValue(VBI_GetSetting(DOWSS)));
            break;

        case IDM_CALL_VIDEOTEXT:
            {
                eVTState NewState;

                switch (VT_GetState())
                {
                case VT_OFF:
                    NewState = VT_BLACK;
                    break;
                case VT_BLACK:
                    NewState = (bVTSingleKeyToggle ? VT_MIXED : VT_OFF);
                    break;
                case VT_MIXED:
                default:
                    NewState = VT_OFF;
                    break;
                }
            
                if (NewState != VT_OFF)
                {
                    if (!Setting_GetValue(VBI_GetSetting(CAPTURE_VBI)))
                    {
                        SendMessage(hWnd, WM_COMMAND, IDM_VBI, 0);
                    }
                    if (!Setting_GetValue(VBI_GetSetting(DOTELETEXT)))
                    {
                        SendMessage(hWnd, WM_COMMAND, IDM_VBI_VT, 0);
                    }
                }

                VT_SetState(NULL, NULL, NewState);
                Cursor_VTUpdate();
                WorkoutOverlaySize(TRUE);
                InvalidateDisplayAreaRect(hWnd, NULL, FALSE);

                if (NewState == VT_OFF)
                {
                    OSD_ShowText("Teletext OFF", 0);
                }
                else
                {
                    OSD_Clear();
                }
            }
            *ChannelString = '\0';
            break;

        case IDM_VT_MIXEDMODE:
            {
                eVTState NewState;

                if (VT_GetState() == VT_MIXED)
                {
                    NewState = VT_BLACK;
                }
                else
                {
                    NewState = VT_MIXED;
                }

                if (!Setting_GetValue(VBI_GetSetting(CAPTURE_VBI)))
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_VBI, 0);
                }
                if (!Setting_GetValue(VBI_GetSetting(DOTELETEXT)))
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_VBI_VT, 0);
                }

                VT_SetState(NULL, NULL, NewState);
                Cursor_VTUpdate();
                WorkoutOverlaySize(TRUE);
                InvalidateDisplayAreaRect(hWnd, NULL, FALSE);
            }
            *ChannelString = '\0';
            break;

        case IDM_VT_RESET:
            VT_ChannelChange();

            if (VT_GetState() != VT_OFF)
            {
                InvalidateDisplayAreaRect(hWnd, NULL, FALSE);
            }
            break;

        case IDM_VIDEOSETTINGS:
            DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_VIDEOSETTINGS), hWnd, VideoSettingProc);
            break;

        case IDM_SIZESETTINGS:
            DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_SIZESETTINGS), hWnd, SizeSettingProc);
            break;

        case IDM_VPS_OUT:
            DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_VPSSTATUS), hWnd, VPSInfoProc);
            break;

        case IDM_VT_OUT:
            DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_VTSTATUS), hWnd, VTInfoProc);
            break;

        case IDM_VT_GOTO:
            DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_VTGOTO), hWnd, VTGotoProc);
            break;

        case IDM_VBI:
            if (VT_GetState() != VT_OFF)
            {
                VT_SetState(NULL, NULL, VT_OFF);
                Cursor_VTUpdate();
                InvalidateDisplayAreaRect(hWnd, NULL, FALSE);
            }
            Stop_Capture();
            Setting_SetValue(VBI_GetSetting(CAPTURE_VBI),
                !Setting_GetValue(VBI_GetSetting(CAPTURE_VBI)));
            Start_Capture();
            break;

        case IDM_VT_SEARCHHIGHLIGHT:
            Setting_SetValue(VBI_GetSetting(SEARCHHIGHLIGHT), 
                !Setting_GetValue(VBI_GetSetting(SEARCHHIGHLIGHT)));
            InvalidateDisplayAreaRect(hWnd, NULL, FALSE);
            break;

        case IDM_CAPTURE_PAUSE:
            Pause_Toggle_Capture();
            break;

        case IDM_AUDIO_MIXER:
            Mixer_SetupDlg(hWnd);
            break;

        case IDM_STATUSBAR:
            DisplayStatusBar_OnChange(!bDisplayStatusBar);
            break;

        case IDM_ON_TOP:
            bAlwaysOnTop = !bAlwaysOnTop;
            WorkoutOverlaySize(FALSE);
            break;

        case IDM_ALWAYONTOPFULLSCREEN:
            bAlwaysOnTopFull = !bAlwaysOnTopFull;
            WorkoutOverlaySize(FALSE);
            break;

        case IDM_VT_AUTOCODEPAGE:
            if (!ProcessVTMessage(hWnd, message, wParam, lParam))
            {
                // IDM_VT_AUTOCODEPAGE won't be handled in
                // ProcessVTMessage if VTState is off
                VT_SetAutoCodepage(NULL, NULL, !VT_GetAutoCodepage());
            }
            bVTAutoCodePage = VT_GetAutoCodepage();
            break;
        
        case IDM_VT_ANTIALIAS:
            if (!ProcessVTMessage(hWnd, message, wParam, lParam))
            {
                // IDM_VT_ANTIALIAS won't be handled in
                // ProcessVTMessage if VTState is off
                VT_SetAntialias(NULL, NULL, !VT_GetAntialias());
            }
            bVTAntiAlias = VT_GetAntialias();
            break;

        case IDM_KEYBOARDLOCK:
            bKeyboardLock = !bKeyboardLock;
            SetKeyboardLock(bKeyboardLock);
            if(bKeyboardLock)
            {
                OSD_ShowText("Keyboard lock on", 0);
            }
            else
            {
                OSD_ShowText("Keyboard lock off", 0);
            }
            
            break;

        case IDM_JUDDERTERMINATOR:
            Stop_Capture();
            Setting_SetValue(OutThreads_GetSetting(DOACCURATEFLIPS), 
                !Setting_GetValue(OutThreads_GetSetting(DOACCURATEFLIPS)));
            Start_Capture();
            break;

        case IDM_USECHROMA:
            Stop_Capture();
            Setting_SetValue(FD_Common_GetSetting(USECHROMA), 
                !Setting_GetValue(FD_Common_GetSetting(USECHROMA)));
            Start_Capture();
            break;

        case IDM_SPACEBAR:
            if(!Setting_GetValue(OutThreads_GetSetting(AUTODETECT)))
            {
                IncrementDeinterlaceMode();
                ShowText(hWnd, GetDeinterlaceModeName());
            }
            break;

        case IDM_SHIFT_SPACEBAR:
            if (!Setting_GetValue(OutThreads_GetSetting(AUTODETECT)))
            {
                DecrementDeinterlaceMode();
                ShowText(hWnd, GetDeinterlaceModeName());
            }
            break;

        case IDM_FULL_SCREEN:
            IsFullScreen_OnChange(!bIsFullScreen);
            WorkoutOverlaySize(TRUE);
            break;
        
        case IDM_RETURN_TO_WINDOW:
            if(bIsFullScreen)
            {
                IsFullScreen_OnChange(FALSE);
            }
            else
            {
                ShowWindow(hWnd, SW_MINIMIZE);
            }
            break;

        case IDM_MINTOTRAY:
            bMinToTray = !bMinToTray;
            SetTray(bMinToTray);
            break;

        case IDM_TRAYSHOW:
            SendMessage(hWnd, IDI_TRAYICON, 0, WM_LBUTTONDBLCLK);
            break;

        case IDM_TAKESTREAMSNAP:
            req.type = REQ_SNAPSHOT;
            PutRequest(&req);
            break;

        case IDM_TAKESTILL:
            req.type = REQ_STILL;
            req.param1 = 1;
            PutRequest(&req);
            break;

        case IDM_TAKECONSECUTIVESTILL:
            // Take cpnsecutive stills
            req.type = REQ_STILL;
            req.param1 = Setting_GetValue(Still_GetSetting(NBCONSECUTIVESTILLS));
            PutRequest(&req);
            break;

        case IDM_TAKECYCLICSTILL:
            bTakingCyclicStills = !bTakingCyclicStills;
            KillTimer(hWnd, TIMER_TAKESTILL);
            if (bTakingCyclicStills)
            {
                req.type = REQ_STILL;
                req.param1 = 1;
                PutRequest(&req);
                // The setting value is changed from a number of seconds to a number of 1/10 of seconds
                SetTimer(hWnd, TIMER_TAKESTILL, Setting_GetValue(Still_GetSetting(DELAYBETWEENSTILLS)) * 100, NULL);
            }
            break;

        case IDM_RESET_STATS:
            ResetDeinterlaceStats();
            ResetARStats();
            pPerf->Reset();
            // don't show the message since it will hide the statistics.
            //ShowText(hWnd, "Statistics reset");
            break;

        case IDM_TSOPTIONS:
            TimeShiftOnOptions();
            break;

        case IDM_SCHEDULE: 
            ShowSchedRecDlg();
            break;
                        
        case IDM_TSRECORD:
            if (TimeShiftRecord())
            {
                ShowText(hWnd, "Recording");
                TimeShiftOnSetMenu(hMenu);
            }
            break;

        case IDM_TSSTOP:
            if (TimeShiftStop())
            {
                ShowText(hWnd, "Stopped");
                TimeShiftOnSetMenu(hMenu);
            }
            break;

        case IDM_TSPLAY:
//            if (CTimeShift::OnPlay())
//            {
//                ShowText(hWnd, "Playing");
//                CTimeShift::OnSetMenu(hMenu);
//            }
            break;

        case IDM_TSPAUSE:
//            if (CTimeShift::OnPause())
//            {
//                ShowText(hWnd, "Paused");
//                CTimeShift::OnSetMenu(hMenu);
//            }
            break;

        case IDM_TSFFWD:
//            if (CTimeShift::OnFastForward())
//            {
//                ShowText(hWnd, "Scanning >>>");
//                CTimeShift::OnSetMenu(hMenu);
//            }
            break;

        case IDM_TSRWND:
//            if (CTimeShift::OnFastBackward())
//            {
//                ShowText(hWnd, "Scanning <<<");
//                CTimeShift::OnSetMenu(hMenu);
//            }
            break;

        case IDM_TSNEXT:
//            if (CTimeShift::OnGoNext())
//            {
//                ShowText(hWnd, "Next Clip");
//                CTimeShift::OnSetMenu(hMenu);
//            }
            break;

        case IDM_TSPREV:
//            if (CTimeShift::OnGoPrev())
//            {
//                ShowText(hWnd, "Previous Clip");
//                CTimeShift::OnSetMenu(hMenu);
//            }
            break;

        case IDM_SHOW_INFOS:
            OSD_ShowSourceComments();
            break;

        case IDM_SET_OSD_TEXT:
            // Useful for external programs for custom control of dTV's OSD display
            // Such as macros in software such as Girder, etc.
            if (lParam)
            {
                vector<char> Buffer(512);
                GlobalGetAtomName((ATOM) lParam, &Buffer[0], 512);
                OSD_ShowTextOverride(&Buffer[0], 0);
                GlobalDeleteAtom((ATOM) lParam);
            }
            else
            {
                OSD_ShowTextOverride("", 0);
            }
            break;

        case IDM_SAVE_SETTINGS_NOW:
            WriteSettingsToIni(FALSE);
            break;

        case IDM_OSD_CC_TEXT:
            {
                RECT winRect;
                RECT DestRect;
                PAINTSTRUCT sPaint;
                GetDisplayAreaRect(hWnd, &winRect);                
                InvalidateRect(hWnd, &winRect, FALSE);
                BeginPaint(hWnd, &sPaint);
                PaintColorkey(hWnd, TRUE, sPaint.hdc, &winRect);
                GetDestRect(&DestRect);
                CC_PaintScreen(hWnd, (TCCScreen*)lParam, sPaint.hdc, &DestRect);
                EndPaint(hWnd, &sPaint);
                ValidateRect(hWnd, &winRect);
            }
            break;

        case IDM_OVERLAY_STOP:
            Overlay_Stop(hWnd);
            break;

        case IDM_OVERLAY_START:
            Overlay_Start(hWnd);
            break;

        case IDM_OVERLAYSETTINGS:
            if(!GetActiveOutput()->CanDoOverlayColorControl())
            {
                MessageBox(hWnd, "Overlay color control is not supported by your video card.",
                           "DScaler", MB_OK);
            }
            else
            {
                DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_OVERLAYSETTINGS), hWnd, OverlaySettingProc);
            }
            break;

        case IDM_USE_DSCALER_OVERLAY:
            Setting_SetValue(Overlay_GetSetting(USEOVERLAYCONTROLS), !Setting_GetValue(Overlay_GetSetting(USEOVERLAYCONTROLS)));
            break;

        case IDM_FAST_REPAINT:
            {
                RECT Rect;

                GetDisplayAreaRect(hWnd, &Rect);

                HDC hWndDC = GetDC(hWnd);
                HDC hDC = OffscreenHDC.BeginPaint(hWndDC, &Rect);

                if (VT_GetState() != VT_OFF)
                {
                    RECT VTDrawRect;
                    GetDestRect(&VTDrawRect);

                    PaintColorkey(hWnd, TRUE, hDC, &Rect, TRUE);

                    VT_Redraw(hDC, &VTDrawRect);
                }
                else
                {
                    PaintColorkey(hWnd, TRUE, hDC, &Rect);
                }

                OSD_Redraw(hDC, &Rect);

                if (!bIsFullScreen && (WindowBorder!=NULL) && WindowBorder->Visible())
                {
                    WindowBorder->Paint(hWnd, hDC, &Rect);
                }

                OffscreenHDC.EndPaint();
                ReleaseDC(hWnd, hWndDC);

                ValidateRect(hWnd, NULL);
            }
            break;

        case IDM_HELP_HOMEPAGE:
            ShellExecute(hWnd, "open", "http://www.dscaler.org/", NULL, NULL, SW_SHOWNORMAL);
            break;

        case IDM_HELP_FAQ:
            HtmlHelp(hWnd, "DScaler.chm::/FAQ.htm", HH_DISPLAY_TOPIC, 0);
            break;

        case IDM_HELP_SUPPORT:
            HtmlHelp(hWnd, "DScaler.chm::/user_support.htm", HH_DISPLAY_TOPIC, 0);
            break;

        case IDM_HELP_KEYBOARD:
            HtmlHelp(hWnd, "DScaler.chm::/keyboard.htm", HH_DISPLAY_TOPIC, 0);
            break;

        case IDM_HELP_GPL:
            HtmlHelp(hWnd, "DScaler.chm::/COPYING.html", HH_DISPLAY_TOPIC, 0);
            break;

        case IDM_HELP_README:
            HtmlHelp(hWnd, "DScaler.chm::/Help.htm", HH_DISPLAY_TOPIC, 0);
            break;

        case IDM_CREDITS:
            {
                CCredits CreditsDlg;
                CreditsDlg.DoModal();
            }
            break;

        case IDM_TELETEXT_KEY1:
        case IDM_TELETEXT_KEY2:
        case IDM_TELETEXT_KEY3:
        case IDM_TELETEXT_KEY4:
        case IDM_TELETEXT_KEY5:
        case IDM_TELETEXT_KEY6:
            ProcessVTMessage(hWnd, message, wParam, lParam);
            break;

        case IDM_LEFT_CROP_PLUS:
            if ( pCalibration->IsRunning()
              && (pCalibration->GetType() == CAL_MANUAL) )
            {
                Setting_Up(Calibr_GetSetting(LEFT_SOURCE_CROPPING));
                SendMessage(hWnd, WM_COMMAND, IDM_LEFT_CROP_CURRENT, 0);
            }
            break;

        case IDM_LEFT_CROP_MINUS:
            if ( pCalibration->IsRunning()
              && (pCalibration->GetType() == CAL_MANUAL) )
            {
                Setting_Down(Calibr_GetSetting(LEFT_SOURCE_CROPPING));
                SendMessage(hWnd, WM_COMMAND, IDM_LEFT_CROP_CURRENT, 0);
            }
            break;

        case IDM_LEFT_CROP_CURRENT:
            if ( pCalibration->IsRunning()
              && (pCalibration->GetType() == CAL_MANUAL) )
            {
                Setting_OSDShow(Calibr_GetSetting(LEFT_SOURCE_CROPPING), hWnd);
            }
            break;

        case IDM_RIGHT_CROP_PLUS:
            if ( pCalibration->IsRunning()
              && (pCalibration->GetType() == CAL_MANUAL) )
            {
                Setting_Up(Calibr_GetSetting(RIGHT_SOURCE_CROPPING));
                SendMessage(hWnd, WM_COMMAND, IDM_RIGHT_CROP_CURRENT, 0);
            }
            break;

        case IDM_RIGHT_CROP_MINUS:
            if ( pCalibration->IsRunning()
              && (pCalibration->GetType() == CAL_MANUAL) )
            {
                Setting_Down(Calibr_GetSetting(RIGHT_SOURCE_CROPPING));
                SendMessage(hWnd, WM_COMMAND, IDM_RIGHT_CROP_CURRENT, 0);
            }
            break;

        case IDM_RIGHT_CROP_CURRENT:
            if ( pCalibration->IsRunning()
              && (pCalibration->GetType() == CAL_MANUAL) )
            {
                Setting_OSDShow(Calibr_GetSetting(RIGHT_SOURCE_CROPPING), hWnd);
            }
            break;

        case IDM_CHARSET_TEST:
            ProcessVTMessage(hWnd, message, wParam, lParam);
            break;
        
        case IDM_SETTINGS_CHANGESETTINGS:
            CTreeSettingsDlg::ShowTreeSettingsDlg(ADVANCED_SETTINGS_MASK);
            break;

        case IDM_SETTINGS_FILTERSETTINGS:
            CTreeSettingsDlg::ShowTreeSettingsDlg(FILTER_SETTINGS_MASK);
            break;

        case IDM_SETTINGS_DEINTERLACESETTINGS:
            CTreeSettingsDlg::ShowTreeSettingsDlg(DEINTERLACE_SETTINGS_MASK);
            break;

        case ID_SETTINGS_SAVESETTINGSPERCHANNEL:
            Setting_SetValue(SettingsPerChannel_GetSetting(SETTINGSPERCHANNEL_BYCHANNEL), !Setting_GetValue(SettingsPerChannel_GetSetting(SETTINGSPERCHANNEL_BYCHANNEL)));
            break;

        case ID_SETTINGS_SAVESETTINGSPERINPUT:
            Setting_SetValue(SettingsPerChannel_GetSetting(SETTINGSPERCHANNEL_BYINPUT), !Setting_GetValue(SettingsPerChannel_GetSetting(SETTINGSPERCHANNEL_BYINPUT)));
            break;

        case ID_SETTINGS_SAVESETTINGSPERFORMAT:
            Setting_SetValue(SettingsPerChannel_GetSetting(SETTINGSPERCHANNEL_BYFORMAT), !Setting_GetValue(SettingsPerChannel_GetSetting(SETTINGSPERCHANNEL_BYFORMAT)));
            break;

        case IDM_DEINTERLACE_SHOWVIDEOMETHODUI:
            ShowVideoModeUI();
            break;
        
        case IDM_CLEAROSD:
            ProcessOSDMessage(hWnd, message, wParam, lParam);
            break;

        case IDM_SETUPHARDWARE:
            // Stop and start capture because of possible pixel width chaange
            Stop_Capture();
            DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_HWSETUP), hWnd, (DLGPROC) HardwareSettingProc, (LPARAM)1);
            Start_Capture();
            break;

        case IDM_SKIN_NONE:
            szSkinName[0] = 0;
            Skin_SetMenu(hMenu, TRUE);
            SetWindowBorder(hWnd, szSkinName, FALSE);
            if (ToolbarControl!=NULL)
            {
                ToolbarControl->Set(hWnd, szSkinName, bIsFullScreen?1:0);
            }
            UpdateWindowState();
            WorkoutOverlaySize(FALSE);
            InvalidateRect(hWnd, NULL, FALSE);
            break;
        case IDM_OUTPUTTYPE_DIRECT3D:
            if(CheckMenuItem(hMenu, IDM_OUTPUTTYPE_DIRECT3D, MF_CHECKED)!=MF_CHECKED) 
            {
                Overlay_Stop(hWnd);
                GetActiveOutput()->ExitDD();
                SetActiveOutput(IOutput::OUT_D3D);
                CheckMenuItemBool(hMenu, IDM_OUTPUTTYPE_OVERLAY, MF_UNCHECKED);                
                CheckMenuItemBool(hMenu, IDM_OUTPUTTYPE_DIRECT3D, MF_CHECKED);
                OutputMethod = IOutput::OUT_D3D;
                if(GetActiveOutput()->InitDD(hWnd)==TRUE)
                {
                    Overlay_Start(hWnd);
                }
            }
            break;
        case IDM_OUTPUTTYPE_OVERLAY:            
            if(CheckMenuItem(hMenu, IDM_OUTPUTTYPE_OVERLAY, MF_CHECKED)!=MF_CHECKED) 
            {
                Overlay_Stop(hWnd);
                GetActiveOutput()->ExitDD();
                SetActiveOutput(IOutput::OUT_OVERLAY);
                CheckMenuItemBool(hMenu, IDM_OUTPUTTYPE_DIRECT3D, MF_UNCHECKED);
                CheckMenuItemBool(hMenu, IDM_OUTPUTTYPE_OVERLAY, MF_CHECKED);               
                OutputMethod = IOutput::OUT_OVERLAY;
                if(GetActiveOutput()->InitDD(hWnd)==TRUE)
                {
                    Overlay_Start(hWnd);
                }
                
            }
            break;
        
        default:
            bDone = FALSE;

            if ((LOWORD(wParam)>=IDM_SKIN_FIRST) && (LOWORD(wParam)<=IDM_SKIN_LAST))
            {
                size_t n = LOWORD(wParam)-IDM_SKIN_FIRST;
                if (n < vSkinNameList.size())
                {
                    strcpy(szSkinName, vSkinNameList[n].c_str());
                }
                Skin_SetMenu(hMenu, TRUE);
                SetWindowBorder(hWnd, szSkinName, TRUE);  
                if (ToolbarControl!=NULL)
                {
                    ToolbarControl->Set(hWnd, szSkinName, bIsFullScreen?1:0);
                }
                UpdateWindowState();            
                WorkoutOverlaySize(FALSE);
                InvalidateRect(hWnd, NULL, FALSE);
                bDone = TRUE;
            }

            if (pMultiFrames && !bDone)
            {
                bDone = pMultiFrames->HandleWindowsCommands(hWnd, wParam, lParam);
            }
            // Check whether menu ID is an aspect ratio related item
            if (!bDone)
            {
                bDone = ProcessAspectRatioSelection(hWnd, LOWORD(wParam));
            }
            if(!bDone)
            {
                bDone = ProcessDeinterlaceSelection(hWnd, LOWORD(wParam));
            }
            if(!bDone)
            {
                bDone = ProcessFilterSelection(hWnd, LOWORD(wParam));
            }
            if(!bDone)
            {
                bDone = ProcessProgramSelection(hWnd, LOWORD(wParam));
                if(bDone)
                {
                    // return here. SetMenuAnalog() is called otherwise. This adds a delay with some
                    // sources since the audio signal menu entries are updated and the audio signal status is 
                    // unknown since we have just switched channels.
                    return 0;
                }
            }
            if(!bDone)
            {
                bDone = ProcessOSDSelection(hWnd, LOWORD(wParam));
            }
            if(!bDone)
            {
                bDone = ProcessVTCodepageSelection(hWnd, LOWORD(wParam));
            }
            if(!bDone)
            {
                bDone = ProcessOutResoSelection(hWnd, LOWORD(wParam));
            }
            if(!bDone && pCalibration != NULL)
            {
                bDone = pCalibration->ProcessSelection(hWnd, LOWORD(wParam));
            }
            if(!bDone && ToolbarControl != NULL)
            {
                bDone = ToolbarControl->ProcessToolbar1Selection(hWnd, LOWORD(wParam));
                if (bDone)
                {
                    if (IsToolBarVisible())
                    {
                        SetTimer(hWnd, TIMER_TOOLBAR, TIMER_TOOLBAR_MS, NULL);
                    }
                    else
                    {
                        KillTimer(hWnd, TIMER_TOOLBAR);
                    }
                }
            }
            if(!bDone)
            {
                bDone = Providers_HandleWindowsCommands(hWnd, wParam, lParam);
            }
            if(!bDone)
            {
                bDone = MyEPG.HandleWindowsCommands(hWnd, wParam, lParam);
            }
            break;
        }

        //-------------------------------------------------------
        // The following code executes on all WM_COMMAND calls

        // Updates the menu checkbox settings
        SetMenuAnalog();
        if (ToolbarControl!=NULL)
        {
            ToolbarControl->UpdateMenu(hMenu);
        }

        if(bUseAutoSave)
        {
            // Set the configuration file autosave timer.
            // We use an autosave timer so that when the user has finished
            // making adjustments and at least a small delay has occured,
            // that the DTV.INI file is properly up to date, even if 
            // the system crashes or system is turned off abruptly.
            KillTimer(hWnd, TIMER_AUTOSAVE);
            SetTimer(hWnd, TIMER_AUTOSAVE, TIMER_AUTOSAVE_MS, NULL);
        }
        return 0;
        break;

    case WM_CREATE:
        MainWndOnCreate(hWnd);
        return 0;
        break;

    case INIT_BT:
        MainWndOnInitBT(hWnd);
        break;

// 2000-10-31 Added by Mark Rejhon
// This was an attempt to allow dTV to run properly through
// computer resolution changes.  Alas, dTV still crashes and burns
// in a fiery dive if you try to change resolution while dTV is
// running.  We need to somehow capture a message that comes right
// before a resolution change, so we can destroy the video overlay
// on time beforehand.   This message seems to happen right after
// a resolution change.
//
//  case WM_DISPLAYCHANGE:
//      // Windows resolution changed while software running
//      Stop_Capture();
//      Overlay_Destroy();
//      Sleep(100);
//      Overlay_Create();
//      BT848_ResetHardware();
//      BT848_SetGeoSize();
//      WorkoutOverlaySize();
//      Start_Capture();
//      Sleep(100);
//      BT848_SetAudioSource(AudioSource);
//      break;

    case WM_POWERBROADCAST:
        // Handing to keep dTV running during computer suspend/resume
        switch ((DWORD) wParam)
        {
        case PBT_APMSUSPEND:
            // Stops video overlay upon suspend operation.
            Overlay_Stop(hWnd);
            break;
        case PBT_APMRESUMESUSPEND:
            // Restarts video overlay upon resume operation.
            // The following crashes unless we do either a HWND_BROADCAST
            // or a Sleep() operation.  To be on the safe side, I do both
            // here.  Perhaps the video overlay drivers needed to reinitialize.
            SendMessage(HWND_BROADCAST, WM_PAINT, 0, 0);
            Sleep(500);
            Reset_Capture();
            if(Providers_GetCurrentSource()->IsInTunerMode())
            {
                Channel_Reset();
            }
            Overlay_Start(hWnd);
            break;
        }
        break;

    case WM_LBUTTONDBLCLK:
        if (bIgnoreDoubleClick == FALSE)
        {
            SendMessage(hWnd, WM_COMMAND, IDM_FULL_SCREEN, 0);
        }
        return 0;
        break;

//  case WM_RBUTTONUP:
//      if(!bAutoHideCursor && bIsFullScreen == FALSE)
//      {
//          bShowCursor = !bShowCursor;
//          Cursor_UpdateVisibility();
//      }
//      break;

    case WM_LBUTTONDOWN:
        if (ProcessVTMessage(hWnd, message, wParam, lParam))
        {
            bIgnoreDoubleClick = TRUE;
        }
        else
        {
            bIgnoreDoubleClick = FALSE;
        }

        if((bShowMenu == FALSE || (GetKeyState(VK_CONTROL) < 0)) && bIsFullScreen == FALSE)
        {
            // pretend we are hitting the caption bar
            // this will allow the user to move the window
            // when the menu and title bar are hidden
            return DefWindowProc(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, lParam);
        }
        Cursor_UpdateVisibility();
        return 0;
        break;

    case WM_LBUTTONUP:
        Cursor_UpdateVisibility();
        return 0;
    case WM_RBUTTONDOWN:
        Cursor_UpdateVisibility();
        bIsRightButtonDown = TRUE;
        return 0;
    case WM_RBUTTONUP:
        Cursor_UpdateVisibility();
        bIsRightButtonDown = FALSE;
        break;

    case WM_MOUSEMOVE:
        {
            static int x = -1;
            static int y = -1;
            int newx = GET_X_LPARAM(lParam);
            int newy = GET_Y_LPARAM(lParam);
            
            if (x != newx || y != newy)
            {
                x = newx;
                y = newy;
                Cursor_UpdateVisibility();
                if (VT_GetState() != VT_OFF)
                {
                    Cursor_VTUpdate(newx, newy);
                }
                if (bIsFullScreen && ToolbarControl != NULL)
                {
                    POINT Point;
                    Point.x = x;
                    Point.y = y;
                    if (ToolbarControl->AutomaticDisplay(Point))
                    {
                        if (IsToolBarVisible())
                        {
                            SetTimer(hWnd, TIMER_TOOLBAR, TIMER_TOOLBAR_MS, NULL);
                        }
                        else
                        {
                            KillTimer(hWnd, TIMER_TOOLBAR);
                        }
                    }
                }
            }
        }
        return 0;
        break;

    case WM_NCMOUSEMOVE:
        Cursor_UpdateVisibility();
        return 0;
        break;

    case WM_ENTERMENULOOP:
        bInMenu = TRUE;
        Cursor_UpdateVisibility();
        return 0;
        break;

    case WM_EXITMENULOOP:
        bInMenu = FALSE;
        Cursor_UpdateVisibility();
        return 0;
        break;

    case WM_INITMENU: 
        SetMenuAnalog();
        return 0;
        break;

    case WM_CONTEXTMENU:
        if(bIgnoreNextRightButtonUpMsg)
        {
            bIgnoreNextRightButtonUpMsg = FALSE;
            return 0;
        }
        if (!OnContextMenu(hWnd, GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)))
            return DefWindowProc(hWnd, message, wParam, lParam);
        break;

    case WM_KILLFOCUS:
        Cursor_UpdateVisibility();
        return 0;
        break;

    case WM_SETFOCUS:
        Cursor_UpdateVisibility();
        return 0;
        break;

    case WM_TIMER:
        pPerf->Suspend();

        switch (LOWORD(wParam))
        {
        //-------------------------------
        case TIMER_STATUS:
            if (IsStatusBarVisible() && (Providers_GetCurrentSource() != NULL))
            {
                if (Setting_GetValue(Audio_GetSetting(SYSTEMINMUTE)) == TRUE)
                {
                    StatusBar_ShowText(STATUS_TEXT, "Volume Mute");
                }
                else if (!Providers_GetCurrentSource()->IsVideoPresent())
                {
                    StatusBar_ShowText(STATUS_TEXT, "No Video Signal Found");
                }
                else
                {
                    string Text(Providers_GetCurrentSource()->GetStatus());
                    if(Text.empty())
                    {
                        if(Providers_GetCurrentSource()->IsInTunerMode())
                        {
                            Text = Channel_GetName();
                        }
                    }
                    StatusBar_ShowText(STATUS_TEXT, Text);
                }

                ostringstream strstream;
                strstream << pPerf->GetDroppedFieldsLastSecond();
                strstream << " DF/S";
                StatusBar_ShowText(STATUS_FPS, strstream.str());
            }
            break;
        //-------------------------------
        case TIMER_TOOLBAR:
            if (IsToolBarVisible() && (Providers_GetCurrentSource() != NULL))
            {
#ifdef WANT_DSHOW_SUPPORT
                if (Providers_GetCurrentSource()->HasMediaControl())
                {
                    EventCollector->RaiseEvent(NULL, EVENT_DURATION, -1, ((CDSSourceBase*)Providers_GetCurrentSource())->GetDuration());
                    EventCollector->RaiseEvent(NULL, EVENT_CURRENT_POSITION, -1, ((CDSSourceBase*)Providers_GetCurrentSource())->GetCurrentPos());
                }
#endif
                if (Mixer_IsEnabled())
                {
                    long val = Mixer_GetVolume();
                    if (val != -1)
                    {
                        EventCollector->RaiseEvent(NULL, EVENT_MIXERVOLUME, -1, val);
                    }
                    val = Mixer_GetMute();
                    if (val != -1)
                    {
                        EventCollector->RaiseEvent(NULL, EVENT_MUTE, -1, val || Audio_IsMute());
                    }
                }
            }
            break;
        //-------------------------------
        case TIMER_KEYNUMBER:
            KillTimer(hWnd, TIMER_KEYNUMBER);
            i = atoi(ChannelString);
            // if only zero's are entered video input is switched.
            if(i == 0)
            {
                if(strcmp(ChannelString, "0") == 0)
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT1, 0);
                }
                else if(strcmp(ChannelString, "00") == 0)
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT2, 0);
                }
                else if(strcmp(ChannelString, "000") == 0)
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT3, 0);
                }
                else if(strcmp(ChannelString, "0000") == 0)
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT4, 0);
                }
                else if(strcmp(ChannelString, "00000") == 0)
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT5, 0);
                }
                else if(strcmp(ChannelString, "000000") == 0)
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT6, 0);
                }
                else if(strcmp(ChannelString, "0000000") == 0)
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT7, 0);
                }
                else if(strcmp(ChannelString, "00000000") == 0)
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT8, 0);
                }
                else if(strcmp(ChannelString, "000000000") == 0)
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT9, 0);
                }
                else if(strcmp(ChannelString, "0000000000") == 0)
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT10, 0);
                }
                else if(strcmp(ChannelString, "00000000000") == 0)
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT11, 0);
                }
                else if(strcmp(ChannelString, "000000000000") == 0)
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT12, 0);
                }
            }
            ChannelString[0] = '\0';
            break;
        //-------------------------------
        case TIMER_AUTOSAVE:
            // JA 21/12/00 Added KillTimer so that settings are not
            // written repeatedly
            KillTimer(hWnd, TIMER_AUTOSAVE);
            WriteSettingsToIni(TRUE);
            break;
        //-------------------------------
        case OSD_TIMER_ID:
            ProcessOSDMessage(hWnd, message, wParam, lParam);
            break;
        //-------------------------------
        case OSD_TIMER_REFRESH_ID:
            ProcessOSDMessage(hWnd, message, wParam, lParam);
            break;
        //-------------------------------
        case TIMER_BOUNCE:
        case TIMER_ORBIT:
            // MRS 2-20-01 - Resetup the display for bounce and orbiting
            WorkoutOverlaySize(FALSE); // Takes care of everything...
            break;
        //-------------------------------
        case TIMER_HIDECURSOR:
            if (Cursor_IsOurs() != FALSE)
            {
                KillTimer(hWnd, TIMER_HIDECURSOR);
                if (ToolbarControl != NULL) 
                {
                    POINT Point;        
                    GetCursorPos(&Point);
                    
                    if (ToolbarControl->PtInToolbar(Point))
                    {
                        Cursor_SetVisibility(TRUE);
                        break;
                    }
                }                
                Cursor_SetVisibility(FALSE);
            }
            break;
        //-------------------------------
        case TIMER_VTFLASHER:
            ProcessVTMessage(hWnd, message, wParam, lParam);
            break;
        //---------------------------------
        case TIMER_VTINPUT:
            ProcessVTMessage(hWnd, message, wParam, lParam);
            break;
        //---------------------------------
        case TIMER_FINDPULL:
            {
                KillTimer(hWnd, TIMER_FINDPULL);
                Setting_SetValue(OutThreads_GetSetting(AUTODETECT), FALSE);
                ShowText(hWnd, GetDeinterlaceModeName());
            }
            break;
        //---------------------------------
        case TIMER_SLEEPMODE:
            {
                KillTimer(hWnd, TIMER_SLEEPMODE);
                switch( SMState.State )
                {
                case SM_WaitMode:
                    // End of main timing
                    ShowWindow(hWnd, SW_HIDE);
                    PostMessage(hWnd, WM_DESTROY, wParam, lParam);
                    break;
                case SM_ChangeMode:
                    // Update & Restart main timing
                    SMState.State = SM_UpdateMode;
                    UpdateSleepMode(&SMState);
                    break;
                default:
                    ; //NEVER_GET_HERE;
                }
            }
            break;
        //---------------------------------
        case TIMER_TAKESTILL:
            req.type = REQ_STILL;
            req.param1 = 1;
            PutRequest(&req);
            break;
        //---------------------------------
        default:
            Provider_HandleTimerMessages(LOWORD(wParam));
            break;
        }
        pPerf->Resume();
        return 0;
        break;
    
    // support for mouse wheel
    // the WM_MOUSEWHEEL message is not defined but this is it's Value
    case WM_MOUSEWHEEL:
        // if shift or right mouse button down change volume
        if ((wParam & MK_SHIFT) != 0 || bIsRightButtonDown)
        {
            // crack the mouse wheel delta
            // +ve is forward (away from user)
            // -ve is backward (towards user)
            if((short)HIWORD(wParam) > 0)
            {
                PostMessage(hWnd, WM_COMMAND, IDM_VOLUMEPLUS, 0);
            }
            else
            {
                PostMessage(hWnd, WM_COMMAND, IDM_VOLUMEMINUS, 0);
            }
            // make sure the context menu is not shown when the right mouse button is released
            if(bIsRightButtonDown)
            {
                bIgnoreNextRightButtonUpMsg = TRUE;
            }
        }
        // if ctrl key down change playlist file
        else if ((wParam & MK_CONTROL) != 0)
        {
            // crack the mouse wheel delta
            // +ve is forward (away from user)
            // -ve is backward (towards user)
            if((short)HIWORD(wParam) > 0)
            {
                PostMessage(hWnd, WM_COMMAND, IDM_PLAYLIST_PREVIOUS, 0);
            }
            else
            {
                PostMessage(hWnd, WM_COMMAND, IDM_PLAYLIST_NEXT, 0);
            }
            // make sure the context menu is not shown when the right mouse button is released
            if(bIsRightButtonDown)
            {
                bIgnoreNextRightButtonUpMsg = TRUE;
            }
        }
        // else change the channel
        else if ((wParam & MK_CONTROL) == 0)
        {
            // crack the mouse wheel delta
            // +ve is forward (away from user)
            // -ve is backward (towards user)
            if((short)HIWORD(wParam) > 0)
            {
                PostMessage(hWnd, WM_COMMAND, IDM_CHANNELPLUS, 0);
            }
            else
            {
                PostMessage(hWnd, WM_COMMAND, IDM_CHANNELMINUS, 0);
            }
        }
        return 0;
        break;

    case WM_SYSCOMMAND:
        switch (wParam & 0xFFF0)
        {
        case SC_SCREENSAVE:
        case SC_MONITORPOWER:
            return FALSE;
            break;
        }
        break;

    case WM_SIZE:        
        return OnSize(hWnd, wParam, lParam);
        break;

    case WM_MOVE:
        StatusBar_Adjust(hWnd);
        if (bDoResize == TRUE && !IsIconic(hWnd) && !IsZoomed(hWnd))
        {
            WorkoutOverlaySize(FALSE);
        }
        return 0;
        break;

    case WM_CHAR:
        if (ProcessVTMessage(hWnd, message, wParam, lParam))
        {
            return 0;
        }
        else
        {
            return OnChar(hWnd, message, wParam, lParam);
        }
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT sPaint;
            RECT Rect;

            BeginPaint(hWnd, &sPaint);

            GetDisplayAreaRect(hWnd, &Rect);

            HDC hDC = OffscreenHDC.BeginPaint(sPaint.hdc, &Rect);

            if (VT_GetState() != VT_OFF)
            {
                RECT VTDrawRect;
                GetDestRect(&VTDrawRect);

                PaintColorkey(hWnd, TRUE, hDC, &sPaint.rcPaint, TRUE);

                // Paint the Teletext stuff
                VT_Redraw(hDC, &VTDrawRect);
            }
            else
            {
                PaintColorkey(hWnd, TRUE, hDC, &sPaint.rcPaint);
            }

            // Paint the OSD stuff
            OSD_Redraw(hDC, &Rect);

            OffscreenHDC.EndPaint();

            if (!bIsFullScreen && (WindowBorder!=NULL) && WindowBorder->Visible())
            {
                WindowBorder->Paint(hWnd, sPaint.hdc, &sPaint.rcPaint);
            }

            EndPaint(hWnd, &sPaint);
        }
        return 0;
        break;

    case UWM_VIDEOTEXT:
        ProcessVTMessage(hWnd, message, wParam, lParam);
        return FALSE;
        break;

    case UWM_OSD:
        ProcessOSDMessage(hWnd, message, wParam, lParam);
        return FALSE;
        break;

    case UWM_INPUTSIZE_CHANGE:
        //the input source has changed its size, update overlay.
        WorkoutOverlaySize(FALSE);
        return FALSE;
        break;
    
    case UWM_SQUAREPIXELS_CHECK:
        if (Providers_GetCurrentSource())
        {
            if (UpdateSquarePixelsMode(Providers_GetCurrentSource()->HasSquarePixels()))
            {
                WorkoutOverlaySize(TRUE);
            }
        }
        return FALSE;
        break;

    case UWM_DEINTERLACE_SETSTATUS:
        if(wParam!=NULL)
        {
            StatusBar_ShowText(STATUS_MODE,(LPCSTR)wParam);
            free((void*)wParam);
        }
        return 0;
        break;

    case UWM_EVENTADDEDTOQUEUE:
        if (EventCollector != NULL)
        {
            EventCollector->ProcessEvents();
        }
        return FALSE;
        break;

    case UWM_SWITCH_WINDOW:
        if (bMinimized == FALSE)
        {
            ShowWindow(hWnd, SW_MINIMIZE);
        }
        else
        {
            ShowWindow(hWnd, SW_SHOW);
            ShowWindow(hWnd, SW_RESTORE);
            SetForegroundWindow(hWnd);
        }
        return FALSE;
        break;

    //TJ 010506 make sure we dont erase the background
    //if we do, it will cause flickering when resizing the window
    //in future we migth need to adjust this to only erase parts not covered by overlay
    case WM_ERASEBKGND:
        return TRUE;
    
    case WM_QUERYENDSESSION:
        return TRUE;
        break;

    case WM_ENDSESSION:
    case WM_DESTROY:
        //Reset screensaver-mode if necessary
        SetScreensaverMode(false);

        MainWndOnDestroy();
        PostQuitMessage(0);
        return 0;
        break;

    case UWM_OVERLAY_UPDATE:
        WorkoutOverlaySize((BOOL)wParam, (BOOL)lParam);
        SetEvent(hMainWindowEvent);
        return 0;
        break;

    default:
        {
            LONG RetVal = Remote_HandleMsgs(hWnd, message, wParam, lParam, &bDone);
            if(!bDone)
            {
                RetVal = Settings_HandleSettingMsgs(hWnd, message, wParam, lParam, &bDone);
            }
            if(!bDone)
            {
                RetVal = Deinterlace_HandleSettingsMsg(hWnd, message, wParam, lParam, &bDone);
            }
            if(!bDone)
            {
                RetVal = Filter_HandleSettingsMsg(hWnd, message, wParam, lParam, &bDone);
            }
            if(!bDone)
            {
                if(Providers_GetCurrentSource() != NULL)
                {
                    RetVal = Providers_GetCurrentSource()->HandleSettingsMessage(hWnd, message, wParam, lParam, &bDone);
                }
            }

            if(bDone)
            {
                // Updates the menu checkbox settings
                SetMenuAnalog();

                if(bUseAutoSave)
                {
                    // Set the configuration file autosave timer.
                    // We use an autosave timer so that when the user has finished
                    // making adjustments and at least a small delay has occured,
                    // that the DTV.INI file is properly up to date, even if 
                    // the system crashes or system is turned off abruptly.
                    KillTimer(hWnd, TIMER_AUTOSAVE);
                    SetTimer(hWnd, TIMER_AUTOSAVE, TIMER_AUTOSAVE_MS, NULL);
                }
                return RetVal;
            }
            else
            {
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        return 0;
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

//---------------------------------------------------------------------------
void SaveWindowPos(HWND hWnd)
{
    WINDOWPLACEMENT WndPlace;
    if(hWnd != NULL)
    {
        // MRS 2-20-01 - length must be set in WindowPlacement structure
        memset(&WndPlace,0,sizeof(WndPlace));
        WndPlace.length = sizeof(WndPlace);
        // End 2-20-01
        GetWindowPlacement(hWnd, &WndPlace);

        MainWndTop = WndPlace.rcNormalPosition.top;
        MainWndHeight = WndPlace.rcNormalPosition.bottom - WndPlace.rcNormalPosition.top;
        MainWndLeft = WndPlace.rcNormalPosition.left;
        MainWndWidth = WndPlace.rcNormalPosition.right - WndPlace.rcNormalPosition.left;

        // We need to adust these numbers by the workspace
        // offset so that we can later use them to set the
        // windows position
        RECT Workspace = {0,0,0,0};
        SystemParametersInfo(SPI_GETWORKAREA, 0, &Workspace, 0);
        MainWndTop += Workspace.top;
        MainWndLeft += Workspace.left;
    }
}


//---------------------------------------------------------------------------
void SaveActualPStripTiming(HWND hPSWnd)
{
    ATOM pStripTimingAtom = static_cast<ATOM>(SendMessage(hPSWnd, UM_GETPSTRIPTIMING, 0, 0));
    if(lPStripTimingString == NULL)
    {
        lPStripTimingString = new char[PSTRIP_TIMING_STRING_SIZE];    
    }
    GlobalGetAtomName(pStripTimingAtom, lPStripTimingString, PSTRIP_TIMING_STRING_SIZE);
    GlobalDeleteAtom(pStripTimingAtom);
}

//---------------------------------------------------------------------------
void MainWndOnInitBT(HWND hWnd)
{
    int i;
    BOOL bInitOK = FALSE;

    // Initialise the PowerStrip window handler
    hPSWnd = FindWindow("TPShidden", NULL);

    AddSplashTextLine("Hardware Init");

    if (ShowHWSetupBox)
    {
        DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_HWSETUP), hWnd, (DLGPROC) HardwareSettingProc, (LPARAM)0);
    }

    if (Providers_Load(hMenu) > 0)
    {
        if (ShowHWSetupBox)
        {
            Providers_ChangeSettingsBasedOnHW(Setting_GetValue(DScaler_GetSetting(PROCESSORSPEED)), Setting_GetValue(DScaler_GetSetting(TRADEOFF)));
        }

        if(GetActiveOutput()->InitDD(hWnd) == TRUE)
        {
            if(GetActiveOutput()->Overlay_Create() == TRUE)
            {
                bInitOK = TRUE;
            }
        }
        else 
        {
            // no supported d3d hardware?
            if(GetActiveOutput()->Type() == IOutput::OUT_D3D)
            {
                // try fallback to overlay
                SetActiveOutput(IOutput::OUT_OVERLAY);
                if(GetActiveOutput()->InitDD(hWnd) == TRUE)
                {
                    if(GetActiveOutput()->Overlay_Create() == TRUE)
                    {
                        // clear the error that appeared when we failed to start D3D
                        OSD_Clear();
                        OutputMethod = IOutput::OUT_OVERLAY;
                        bInitOK = TRUE;
                    }
                }
            }
            else
            {
                if (MessageBox(hWnd,
                               "The overlay couldn't be created.\n"
                               "Do you want to try DirectX?", 
                               "DScaler - Overlay create Failed", 
                               MB_YESNO | MB_ICONQUESTION | MB_APPLMODAL) == IDYES)
                {
                    SetActiveOutput(IOutput::OUT_D3D);
                    if(GetActiveOutput()->InitDD(hWnd) == TRUE)
                    {
                        if(GetActiveOutput()->Overlay_Create() == TRUE)
                        {
                            // clear the error that appeared when we failed to start the overlay
                            OSD_Clear();
                            OutputMethod = IOutput::OUT_D3D;
                            bInitOK = TRUE;
                        }
                    }
                }
            }
        }
    }
    else
    {
        AddSplashTextLine("");
        AddSplashTextLine("No");
        AddSplashTextLine("Suitable");
        AddSplashTextLine("Hardware");
    }

    if (bInitOK)
    {
        AddSplashTextLine("Load Plugins");
        if(!LoadDeinterlacePlugins())
        {
            AddSplashTextLine("");
            AddSplashTextLine("No");
            AddSplashTextLine("Plug-ins");
            AddSplashTextLine("Found");
            bInitOK = FALSE;
        }
        else
        {
            LoadFilterPlugins();
        }
    }
    
    if (bInitOK)
    {
        AddSplashTextLine("Position Window");
        WStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
        if (bAlwaysOnTop == FALSE)
        {
            WStyle = WStyle ^ 8;
            i = SetWindowLong(hWnd, GWL_EXSTYLE, WStyle);
            SetWindowPos(hWnd, HWND_NOTOPMOST, 10, 10, 20, 20, SWP_NOMOVE | SWP_NOCOPYBITS | SWP_NOSIZE | SWP_SHOWWINDOW);
        }
        else
        {
            WStyle = WStyle | 8;
            i = SetWindowLong(hWnd, GWL_EXSTYLE, WStyle);
            SetWindowPos(hWnd, HWND_TOPMOST, 10, 10, 20, 20, SWP_NOMOVE | SWP_NOCOPYBITS | SWP_NOSIZE | SWP_SHOWWINDOW);
        }

        if (bShowMenu == FALSE)
        {
            bShowMenu = TRUE;
            SendMessage(hWnd, WM_COMMAND, IDM_TOGGLE_MENU, 0);
        }

        AddSplashTextLine("Load Toolbars");
        if (ToolbarControl == NULL)
        {
            ToolbarControl = new CToolbarControl(WM_TOOLBARS_GETVALUE);
            ToolbarControl->Set(hWnd, NULL);
        }

        if (szSkinName[0] != 0)
        {
            AddSplashTextLine("Load Skin");
            
            SetWindowBorder(hWnd, szSkinName, (szSkinName[0]!=0));  
            if (ToolbarControl!=NULL)
            {
                ToolbarControl->Set(hWnd, szSkinName);  
            }
        }

        // We must do two calls, the first one displaying the toolbar
        // in order to have the correct toolbar rectangle initialized,
        // and the second to hide the toolbar if in full scrren mode
        if (ToolbarControl == NULL)
        {
            ToolbarControl->Set(hWnd, NULL, bIsFullScreen?1:0);
        }

        AddSplashTextLine("Setup Mixer");

        AddSplashTextLine("Start Timers");
        if(bIsFullScreen == FALSE && bDisplayStatusBar == TRUE)
        {
            SetTimer(hWnd, TIMER_STATUS, TIMER_STATUS_MS, NULL);
        }

        if (IsToolBarVisible())
        {
            SetTimer(hWnd, TIMER_TOOLBAR, TIMER_TOOLBAR_MS, NULL);
        }

        // do final setup routines for any files
        // basically where we need the hWnd to be set
        AddSplashTextLine("Setup Aspect Ratio");
        Aspect_FinalSetup();

        // OK we're ready to go
        WorkoutOverlaySize(FALSE);
        
        AddSplashTextLine("Update Menu");
        OSD_UpdateMenu(hMenu);
        pCalibration->UpdateMenu(hMenu);
        Channels_UpdateMenu(hMenu);
        VT_UpdateMenu(hMenu);
        OutReso_UpdateMenu(hMenu);
        SetMenuAnalog();
        if (ToolbarControl!=NULL)
        {
            ToolbarControl->UpdateMenu(hMenu);
        }
        Skin_SetMenu(hMenu, FALSE);

        if (bIsFullScreen)
        {
            if(hPSWnd)
            {
                // Save the actual PowerStrip timing string in lPStripTimingString
                SaveActualPStripTiming(hPSWnd);
            }
            OutReso_Change(hWnd, hPSWnd, FALSE, FALSE, NULL, FALSE);
            BypassChgResoInRestore = TRUE;
        }

        bDoResize = TRUE;

        if (Providers_GetCurrentSource())
        {
            // if we are in tuner mode
            // either set channel up as requested on the command line
            // or reset it to what is was last time
            if (Providers_GetCurrentSource()->IsInTunerMode())
            {
                if(InitialChannel >= 0)
                {
                    Channel_Change(InitialChannel);
                }
                else
                {
                    Channel_Reset();
                }
            }
        }
        
        if (InitialTextPage >= 0x100)
        {
            Setting_SetValue(VBI_GetSetting(CAPTURE_VBI), TRUE);
            Setting_SetValue(VBI_GetSetting(DOTELETEXT), TRUE);

            VT_SetState(NULL, NULL, VT_BLACK);
            VT_SetPage(NULL, NULL, InitialTextPage);
        }

        RemoteRegister();

        AddSplashTextLine("Start Video");
        Start_Capture();
        GetActiveOutput()->SetCurrentMonitor(hWnd);
        
    }
    else
    {
        Sleep(2000);
        PostQuitMessage(0);
    }
}

//---------------------------------------------------------------------------
void MainWndOnCreate(HWND hWnd)
{
    char Text[128];
    int i;
    int ProcessorMask;
    SYSTEM_INFO SysInfo;

    LOG(1 , "DScaler Version %s Compiled %s %s Build %s", VERSTRING, __DATE__, __TIME__, GetSVNBuildString());

    OSVERSIONINFO ovi;
    ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if(GetVersionEx(&ovi))
    {
        LOG(1, "Windows %d.%d (Win%s build %d) [%s]"
            ,ovi.dwMajorVersion
            ,ovi.dwMinorVersion
            ,ovi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS
            ? (ovi.dwMinorVersion>0 ? (ovi.dwMinorVersion>10 ? "Me" : "98") : "95")
                : ovi.dwPlatformId == VER_PLATFORM_WIN32_NT
                    ? (ovi.dwMajorVersion==5 &&ovi.dwMinorVersion==1 ? "XP" :
                    (ovi.dwMajorVersion >= 5 ? "2000" : "NT"))
                    : "?"
            ,ovi.dwBuildNumber & 0xffff
            ,ovi.szCSDVersion);
    }

    pCalibration = new CCalibration();

    pPerf = new CPerf();

    GetSystemInfo(&SysInfo);
    AddSplashTextLine("Table Build");
    AddSplashTextLine("Teletext");

    VBI_Init(); 
    OSD_Init();
    Mixer_Init();

    Load_Program_List_ASCII();

    AddSplashTextLine("System Analysis");

    sprintf(Text, "Processor %d ", SysInfo.dwProcessorType);
    AddSplashTextLine(Text);
    sprintf(Text, "Number %d ", SysInfo.dwNumberOfProcessors);
    AddSplashTextLine(Text);

    if (SysInfo.dwNumberOfProcessors > 1)
    {
        if (DecodeProcessor == 0)
        {
            if (SysInfo.dwNumberOfProcessors == 2)
            {
                MainProcessor = 0;
                DecodeProcessor = 1;
            }
            if (SysInfo.dwNumberOfProcessors == 3)
            {
                MainProcessor = 0;
                DecodeProcessor = 2;
            }
            if (SysInfo.dwNumberOfProcessors > 3)
            {
                DecodeProcessor = 3;
            }

        }

        AddSplashTextLine("Multi-Processor");
        sprintf(Text, "Main-CPU %d ", MainProcessor);
        AddSplashTextLine(Text);
        sprintf(Text, "DECODE-CPU %d ", DecodeProcessor);
        AddSplashTextLine(Text);
    }

    ProcessorMask = 1 << (MainProcessor);
    i = SetThreadAffinityMask(GetCurrentThread(), ProcessorMask);

    UpdatePriorityClass();

    Cursor_UpdateVisibility();

    TimeShiftInit(hWnd);

    PostMessage(hWnd, INIT_BT, 0, 0);
}

void KillTimers()
{
    KillTimer(hWnd, TIMER_BOUNCE);
    KillTimer(hWnd, TIMER_ORBIT);
    KillTimer(hWnd, TIMER_AUTOSAVE);
    KillTimer(hWnd, TIMER_KEYNUMBER);
    KillTimer(hWnd, TIMER_STATUS);
    KillTimer(hWnd, OSD_TIMER_ID);
    KillTimer(hWnd, OSD_TIMER_REFRESH_ID);
    KillTimer(hWnd, TIMER_HIDECURSOR);
    KillTimer(hWnd, TIMER_VTFLASHER);
    KillTimer(hWnd, TIMER_VTINPUT);
    KillTimer(hWnd, TIMER_FINDPULL);
    KillTimer(hWnd, TIMER_SLEEPMODE);
    KillTimer(hWnd, TIMER_TAKESTILL);
    KillTimer(hWnd, TIMER_TOOLBAR);
}


// basically we want do make sure everything that needs to be done on exit gets 
// done even if one of the functions crashes we should just carry on with the rest
// of the functions
void MainWndOnDestroy()
{
    try
    {
        KillTimers();
    }
    catch(...) {LOG(1, "Kill Timers");}

    // stop capture before stopping timneshift to avoid crash
    try
    {
        LOG(1, "Try Stop_Capture");
        Stop_Capture();
    }
    catch(...) {LOG(1, "Error Stop_Capture");}
      
    try
    {
        LOG(1, "Try CTimeShift::OnStop");

        TimeShiftStop();
    }
    catch(...) {LOG(1, "Error TimeShiftStop");}

    
    // Kill timeshift before muting since it always exits unmuted on cleanup.
    try
    {
        LOG(1, "Try TimeShiftShutdown");

        TimeShiftShutdown();
    }
    catch(...) {LOG(1, "Error TimeShiftShutdown");}

    try
    {
        LOG(1, "Try CleanUpMemory");
        CleanUpMemory();
    }
    catch(...) {LOG(1, "Error CleanUpMemory");}

    try
    {
        if(bIsFullScreen == FALSE)
        {
            LOG(1, "Try SaveWindowPos");
            if(hPSWnd)
            {            
                // Save the actual PowerStrip timing string in lPStripTimingString
                SaveActualPStripTiming(hPSWnd);
            }
            SaveWindowPos(hWnd);
        }
    }
    catch(...) {LOG(1, "Error SaveWindowPos");}
    
    try
    {
        LOG(1, "Try free skinned border");
        if (WindowBorder != NULL)
        {
            delete WindowBorder;
            WindowBorder = NULL;
        }            
    }
    catch(...) {LOG(1, "Error free skinned border");}

    try
    {
        LOG(1, "Try free toolbars");
        if (ToolbarControl!=NULL)
        {
            delete ToolbarControl;
            ToolbarControl = NULL;
        }
    }
    catch(...) {LOG(1, "Error free toolbars");}    

    try
    {
        // save settings per cahnnel/input ets
        // must be done before providers are unloaded
        LOG(1, "SettingsMaster->SaveSettings");
        SettingsMaster->SaveSettings();
    }
    catch(...) {LOG(1, "Error SettingsMaster->SaveSettings");}
    
    try
    {
        // write out setting with optimize on 
        // to avoid delay on flushing file
        // all the setting should be filled out anyway
        LOG(1, "WriteSettingsToIni");
        WriteSettingsToIni(TRUE);
    }
    catch(...) {LOG(1, "Error WriteSettingsToIni");}

    try
    {
        LOG(1, "Try Providers_Unload");
        Providers_Unload();
    }
    catch(...) {LOG(1, "Error Providers_Unload");}

    
    try
    {
        LOG(1, "Try free settings");
        if (SettingsMaster != NULL)
        {
            delete SettingsMaster;
            SettingsMaster = NULL;
        }                
        FreeSettings();
        // Free of filters settings is done later when calling UnloadFilterPlugins
        // Free of sources dependent settings is already done when calling Providers_Unload
    }
    catch(...) {LOG(1, "Error free settings");}
    

    try
    {
        LOG(1, "Try StatusBar_Destroy");
        StatusBar_Destroy();
    }
    catch(...) {LOG(1, "Error StatusBar_Destroy");}

    try
    {
        LOG(1, "Try SetTray(FALSE)");
        if (bIconOn)
            SetTray(FALSE);
    }
    catch(...) {LOG(1, "Error SetTray(FALSE)");}

    try
    {
        LOG(1, "Try free EventCollector");
        if (EventCollector != NULL)
        {
            delete EventCollector;
            EventCollector = NULL;
        }
    }
    catch(...) {LOG(1, "Error free EventCollector");}
    
    try
    {
        LOG(1, "Try ExitDD");
        GetActiveOutput()->ExitDD();
    }
    catch(...) {LOG(1, "Error ExitDD");}

    try
    {
        if(bIsFullScreen == TRUE)
        {
            // Do this here after the ExitDD to be sure that the overlay is destroyed
            LOG(1, "Try restore display resolution");
            BypassChgResoInRestore = TRUE;
            OutReso_Change(hWnd, hPSWnd, TRUE, FALSE, lPStripTimingString, TRUE);
        }
    }
    catch(...) {LOG(1, "Error restore display resolution");}
 
    try
    {
        // unload plug-ins
        UnloadDeinterlacePlugins();
        UnloadFilterPlugins();
    }
    catch(...) {LOG(1, "Error Unload plug-ins");}

    try
    {
        SetKeyboardLock(FALSE);
    }
    catch(...) {LOG(1, "Error SetKeyboardLock(FALSE)");}

    try
    {
        Timing_CleanUp();
    }
    catch(...) {LOG(1, "Error Timing_CleanUp()");}

}

LONG OnChar(HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
    char Text[128];
    int i;

    if (((char) wParam >= '0') && ((char) wParam <= '9'))
    {
        sprintf(Text, "%c", (char)wParam);
        // if something gets broken in the future
        if(strlen(ChannelString) >= sizeof(ChannelString)/sizeof(char) - 1)
        {
#ifdef _DEBUG
            // if this is a debug build show an error message
            MessageBox(hWnd, "dscaler.cpp: ChannelString out of bounds.", "Error", MB_ICONERROR);
#endif
            ChannelString[0] = '\0';
            return 0;
        }
        strcat(ChannelString, Text);

        // if the user is only typing zero's we are going to switch inputs
        if(atoi(ChannelString) == 0 && (char) wParam == '0')
        {
            int VideoInput = strlen(ChannelString) -1;

            if(Providers_GetCurrentSource() != NULL)
            {
                if(VideoInput < Providers_GetCurrentSource()->NumInputs(VIDEOINPUT))
                {
                    OSD_ShowText(Providers_GetCurrentSource()->GetInputName(VIDEOINPUT, VideoInput), 0);
                }
                else
                {
                    OSD_ShowText(ChannelString, 0);
                }

                if (strlen(ChannelString) >= 7)
                {
                    SetTimer(hWnd, TIMER_KEYNUMBER, 1, NULL);
                }
                else
                {
                    SetTimer(hWnd, TIMER_KEYNUMBER, ChannelEnterTime, NULL);
                }
            }
            else
            {
                OSD_ShowText("No Source", 0);
            }
        }
        // if in tuner mode or videotext mode
        else if (Providers_GetCurrentSource()->IsInTunerMode())
        {
            OSD_ShowText(ChannelString, 0);

            if(strlen(ChannelString) >= 3)
            {
                SetTimer(hWnd, TIMER_KEYNUMBER, 1, NULL);
            }
            else
            {
                SetTimer(hWnd, TIMER_KEYNUMBER, ChannelEnterTime, NULL);
            }

            i = atoi(ChannelString);
            if(i != 0)
            {
                Channel_ChangeToNumber(i,(strlen(ChannelString)>1)?1:0);
            }
        }
        // we don't know what to do with this keypress so we reset ChannelString
        else
        {
            ChannelString[0] = '\0';
        }
    }
    return 0;
}

LONG OnSize(HWND hWnd, UINT wParam, LONG lParam)
{
    StatusBar_Adjust(hWnd);
    if (ToolbarControl!=NULL)
    {
        ToolbarControl->Adjust(hWnd, FALSE, TRUE);
    }
    UpdateWindowRegion(hWnd, FALSE);
    if (bDoResize == TRUE)
    {
        switch(wParam)
        {
        case SIZE_MAXIMIZED:
            if(bIsFullScreen == FALSE || bMinimized == TRUE)
            {
                bCheckSignalPresent = FALSE;
                bCheckSignalMissing = (MinimizeHandling == 2);
                IsFullScreen_OnChange(TRUE);
                if ((MinimizeHandling == 1) && bMinimized && !GetActiveOutput()->OverlayActive())
                {
                    Overlay_Start(hWnd);
                }
                bMinimized = FALSE;
            }
            break;
        case SIZE_MINIMIZED:
            bMinimized = TRUE;
            if (GetActiveOutput()->OverlayActive() && (MinimizeHandling == 1))
            {
                Overlay_Stop(hWnd);
            }
            if(bIsFullScreen)
            {
                OutReso_Change(hWnd, hPSWnd, TRUE, TRUE, lPStripTimingString, TRUE);
            }
            if (GetActiveOutput()->OverlayActive())
            {
                GetActiveOutput()->Overlay_Update(NULL, NULL, DDOVER_HIDE);
            }
            if (bMinToTray)
            {
                ShowWindow(hWnd, SW_HIDE);
            }
            bCheckSignalPresent = (MinimizeHandling == 2);
            bCheckSignalMissing = FALSE;
            break;
        case SIZE_RESTORED:
            bMinimized = FALSE;
            bCheckSignalPresent = FALSE;
            bCheckSignalMissing = (MinimizeHandling == 2);
            InvalidateRect(hWnd, NULL, FALSE);
            if(bIsFullScreen)
            {
                if (BypassChgResoInRestore)
                {
                    BypassChgResoInRestore = FALSE;
                }
                else
                {
                    OutReso_Change(hWnd, hPSWnd, FALSE, TRUE, NULL, FALSE);
                }
            }
            if ((MinimizeHandling == 1) && !GetActiveOutput()->OverlayActive())
            {
                Overlay_Start(hWnd);
            }
            if (GetActiveOutput()->OverlayActive())
            {
                WorkoutOverlaySize(FALSE);
            }
            SetMenuAnalog();
            break;
        default:
            break;
        }
    }
    return 0;
}



//---------------------------------------------------------------------------
void SetMenuAnalog()
{
    CheckMenuItemBool(hMenu, IDM_TOGGLECURSOR, bShowCursor);
    EnableMenuItem(hMenu,IDM_TOGGLECURSOR, bAutoHideCursor?MF_GRAYED:MF_ENABLED);
    CheckMenuItemBool(hMenu, IDM_STATUSBAR, bDisplayStatusBar);
    CheckMenuItemBool(hMenu, IDM_TOGGLE_MENU, bShowMenu);
    CheckMenuItemBool(hMenu, IDM_ON_TOP, bAlwaysOnTop);
    CheckMenuItemBool(hMenu, IDM_ALWAYONTOPFULLSCREEN, bAlwaysOnTopFull);
    CheckMenuItemBool(hMenu, IDM_FULL_SCREEN, bIsFullScreen);
    CheckMenuItemBool(hMenu, IDM_MINTOTRAY, bMinToTray);
    CheckMenuItemBool(hMenu, IDM_VT_AUTOCODEPAGE, bVTAutoCodePage);
    CheckMenuItemBool(hMenu, IDM_VT_ANTIALIAS, bVTAntiAlias);

    CheckMenuItemBool(hMenu, IDM_USE_DSCALER_OVERLAY, Setting_GetValue(Overlay_GetSetting(USEOVERLAYCONTROLS)));
    EnableMenuItem(hMenu,IDM_OVERLAYSETTINGS, (GetActiveOutput()->CanDoOverlayColorControl())?MF_ENABLED:MF_GRAYED);

    EnableMenuItem(hMenu, IDM_OVERLAY_START, bMinimized ? MF_GRAYED : MF_ENABLED);
    EnableMenuItem(hMenu, IDM_OVERLAY_STOP, bMinimized ? MF_GRAYED : MF_ENABLED);
    CheckMenuItemBool(hMenu, IDM_TAKECYCLICSTILL, bTakingCyclicStills);

    EnableMenuItem(hMenu, IDM_AUDIO_MIXER, ((Providers_GetCurrentSource() == NULL) || !Providers_GetCurrentSource()->IsAudioMixerAccessAllowed()) ? MF_GRAYED : MF_ENABLED);

    SetMixedModeMenu(hMenu, !bVTSingleKeyToggle);

    AspectRatio_SetMenu(hMenu);
    FD60_SetMenu(hMenu);
    OutThreads_SetMenu(hMenu);
    Deinterlace_SetMenu(hMenu);
    Filter_SetMenu(hMenu);
    VBI_SetMenu(hMenu);
    Channels_SetMenu(hMenu);
    OSD_SetMenu(hMenu);
    FD_Common_SetMenu(hMenu);
    Timing_SetMenu(hMenu);
    MixerDev_SetMenu(hMenu);
    Audio_SetMenu(hMenu);
    VT_SetMenu(hMenu);
    OutReso_SetMenu(hMenu);
    Providers_SetMenu(hMenu);

    TimeShiftOnSetMenu(hMenu);
    if(pCalibration)
    {
        pCalibration->SetMenu(hMenu);
    }

    MyEPG.SetMenu(hMenu);

    CheckMenuItemBool(hMenu, ID_SETTINGS_SAVESETTINGSPERCHANNEL, SettingsPerChannel_IsPerChannel());
    CheckMenuItemBool(hMenu, ID_SETTINGS_SAVESETTINGSPERINPUT, SettingsPerChannel_IsPerInput());
    CheckMenuItemBool(hMenu, ID_SETTINGS_SAVESETTINGSPERFORMAT, SettingsPerChannel_IsPerFormat());
    
    CheckMenuItemBool(hMenu, IDM_OUTPUTTYPE_OVERLAY, GetActiveOutput()->Type() == IOutput::OUT_OVERLAY);
    CheckMenuItemBool(hMenu, IDM_OUTPUTTYPE_DIRECT3D, GetActiveOutput()->Type() == IOutput::OUT_D3D);
}

// This function checks the name of the menu item. A message box is shown with a debug build
// if the name is not correct.
HMENU GetSubMenuWithName(HMENU hMenu, int nPos, LPCSTR szMenuText)
{
#ifdef _DEBUG
    char name[128] = "\0";
    char msg[128] = "\0";

    GetMenuString(hMenu, nPos, name, sizeof(name), MF_BYPOSITION);

    if(strcmp(name, szMenuText) != 0)
    {
        sprintf(msg, "GetSubMenuWithName() error: \'%s\' != \'%s\'", name, szMenuText);
        MessageBox(hWnd, msg, "Error", MB_ICONERROR);
    }
#endif
    return GetSubMenu(hMenu, nPos);
}

HMENU GetOrCreateSubSubMenu(int SubId, int SubSubId, LPCSTR szMenuText)
{
    if(hMenu != NULL)
    {
        HMENU hSubMenu = GetSubMenu(hMenu, SubId);
        if(hSubMenu != NULL)
        {
            HMENU hSubSubMenu = GetSubMenu(hSubMenu, SubSubId);
            if(hSubSubMenu != NULL)
            {
                return hSubSubMenu;
            }
            else
            {
                if(ModifyMenu(hSubMenu, SubSubId, MF_STRING | MF_BYPOSITION | MF_POPUP, (UINT)CreatePopupMenu(), szMenuText))
                {
                    return GetSubMenu(hSubMenu, SubSubId);
                }
                else
                {
                    return NULL;
                }
            }
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
}

HMENU GetOrCreateSubSubSubMenu(int SubId, int SubSubId, int SubSubSubId, LPCSTR szMenuText)
{
    if(hMenu != NULL)
    {
        HMENU hSubMenu = GetSubMenu(hMenu, SubId);
        if(hSubMenu != NULL)
        {
            HMENU hSubSubMenu = GetSubMenu(hSubMenu, SubSubId);
            if(hSubSubMenu != NULL)
            {
                HMENU hSubSubSubMenu = GetSubMenu(hSubSubMenu, SubSubSubId);
                if(hSubSubSubMenu != NULL)
                {
                    return hSubSubSubMenu;
                }
                else
                {
                    if(ModifyMenu(hSubSubMenu, SubSubSubId, MF_STRING | MF_BYPOSITION | MF_POPUP, (UINT)CreatePopupMenu(), szMenuText))
                    {
                        return GetSubMenu(hSubSubMenu, SubSubSubId);
                    }
                    else
                    {
                        return NULL;
                    }
                }
            }
            else
            {
                return NULL;
            }
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
}

HMENU GetFiltersSubmenu()
{
    char string[128] = "\0";
    int reduc;

    GetMenuString(hMenu, 2, string, sizeof(string), MF_BYPOSITION);
    reduc = !strcmp(string, "&Channels") ? 0 : 1;

    HMENU hmenu = GetSubMenuWithName(hMenu, 5-reduc, "&Filters");
    ASSERT(hmenu != NULL);

    return hmenu;
}


HMENU GetVideoDeinterlaceSubmenu()
{
    char string[128] = "\0";
    int reduc;

    GetMenuString(hMenu, 2, string, sizeof(string), MF_BYPOSITION);
    reduc = !strcmp(string, "&Channels") ? 0 : 1;

    HMENU hmenu = GetSubMenuWithName(hMenu, 4-reduc, "Deinter&lace");
    ASSERT(hmenu != NULL);

    return hmenu;
}

HMENU GetChannelsSubmenu()
{
    ASSERT(hSubMenuChannels != NULL);

    return hSubMenuChannels;
}

HMENU GetOSDSubmenu()
{
    char string[128] = "\0";
    int reduc;

    GetMenuString(hMenu, 2, string, sizeof(string), MF_BYPOSITION);
    reduc = !strcmp(string, "&Channels") ? 0 : 1;

    HMENU hmenu = GetSubMenuWithName(hMenu, 3-reduc, "&View");
    ASSERT(hmenu != NULL);

    return hmenu;
}

HMENU GetPatternsSubmenu()
{
    char string[128] = "\0";
    int reduc;

    GetMenuString(hMenu, 2, string, sizeof(string), MF_BYPOSITION);
    reduc = !strcmp(string, "&Channels") ? 0 : 1;

    HMENU hmenu = GetOrCreateSubSubSubMenu(7-reduc, 2, 0, "Test &Patterns");
    ASSERT(hmenu != NULL);

    return hmenu;
}

HMENU GetVTCodepageSubmenu()
{
    char string[128] = "\0";
    int reduc;

    GetMenuString(hMenu, 2, string, sizeof(string), MF_BYPOSITION);
    reduc = !strcmp(string, "&Channels") ? 0 : 1;

    HMENU hmenu = GetSubMenuWithName(hMenu, 9-reduc, "&Datacasting");
    ASSERT(hmenu != NULL);

    GetMenuString(hmenu, 8, string, sizeof(string), MF_BYPOSITION);
    reduc = !strcmp(string, "Toggle &Mixed Mode\tShift-T") ? 0 : 1;

    hmenu = GetSubMenuWithName(hmenu, 9-reduc, "Teletext Code Page");
    ASSERT(hmenu != NULL);

    return hmenu;
}

HMENU GetOutResoSubmenu()
{
    char string[128] = "\0";
    int reduc;

    GetMenuString(hMenu, 2, string, sizeof(string), MF_BYPOSITION);
    reduc = !strcmp(string, "&Channels") ? 0 : 1;

    HMENU hmenu = GetOrCreateSubSubMenu(3-reduc, 10, "Switch Resolution in F&ull Screen");
    ASSERT(hmenu != NULL);

    return hmenu;
}

HMENU GetEPGDaySubmenu()
{
    char string[128] = "\0";
    int reduc;

    GetMenuString(hMenu, 2, string, sizeof(string), MF_BYPOSITION);
    reduc = !strcmp(string, "&Channels") ? 0 : 1;
    HMENU hmenu = GetOrCreateSubSubSubMenu(8-reduc, 12, 15, "Day");
    ASSERT(hmenu != NULL);

    return hmenu;
}

void SetMixedModeMenu(HMENU hMenu, BOOL bShow)
{
    static BOOL     bShown = TRUE;
    static char     szMenuName[64] = "";
    MENUITEMINFO    MenuItem;
    char            Buffer[64];

    if (bShow == bShown)
    {
        return;
    }

    MenuItem.cbSize     = sizeof(MENUITEMINFO);
    MenuItem.fMask      = MIIM_TYPE;
    MenuItem.dwTypeData = Buffer;
    MenuItem.cch        = sizeof(Buffer);

    // Get the Mixed Mode menu item name
    if (GetMenuItemInfo(hMenu, IDM_VT_MIXEDMODE, FALSE, &MenuItem))
    {
        // The menu item exists, delete it if necessary
        if (!bShow)
        {
            if (MenuItem.fType == MFT_STRING)
            {
                // Save the name so we can be put back
                strcpy(szMenuName, Buffer);
            }

            DeleteMenu(hMenu, IDM_VT_MIXEDMODE, MF_BYCOMMAND);
            bShown = FALSE;
        }
    }
    else
    {
        // The menu does not exists, add it if necessary
        if (bShow)
        {
            // To insert after, I insert before and delete the
            // previous then insert the previous before again
            GetMenuItemInfo(hMenu, IDM_CALL_VIDEOTEXT, FALSE, &MenuItem);

            MenuItem.fMask      = MIIM_TYPE | MIIM_ID;
            MenuItem.fType      = MFT_STRING;
            MenuItem.wID        = IDM_VT_MIXEDMODE;
            MenuItem.dwTypeData = szMenuName;
            MenuItem.cch        = strlen(szMenuName);

            // Put the mixed mode menu item back
            InsertMenuItem(hMenu, IDM_CALL_VIDEOTEXT, FALSE, &MenuItem);

            DeleteMenu(hMenu, IDM_CALL_VIDEOTEXT, MF_BYCOMMAND);

            MenuItem.wID        = IDM_CALL_VIDEOTEXT;
            MenuItem.dwTypeData = Buffer;
            MenuItem.cch        = strlen(Buffer);

            InsertMenuItem(hMenu, IDM_VT_MIXEDMODE, FALSE, &MenuItem);
            bShown = TRUE;
        }
    }
}

void RedrawMenuBar(HMENU)
{
    // We could use the 1st arg to make
    // sure the changed menu is on our
    // window, but it's not necessary.
    DrawMenuBar(hWnd);
}

//---------------------------------------------------------------------------
void CleanUpMemory()
{
    CScheduleDlg::OnDscalerExit();
    Mixer_Exit();
    VBI_Exit();
    OSD_Exit();
    if ((hMenu != NULL) && (GetMenu(hWnd) == NULL))
    {
        DestroyMenu(hMenu);
    }
    Channels_Exit();
    delete pCalibration;
    pCalibration = NULL;
    delete pPerf;
    pPerf = NULL;
    if(hMainWindowEvent != NULL)
    {
        ResetEvent(hMainWindowEvent);
    }

}

//---------------------------------------------------------------------------
// Stops video overlay - 2000-10-31 Added by Mark Rejhon
// This ends the video overlay from operating, so that end users can
// write scripts that sends this special message to safely stop the video
// before switching computer resolutions or timings.
// This is also called during a Suspend operation
void Overlay_Stop(HWND hWnd)
{
    RECT winRect;
    HDC hDC;

    if (g_bOverlayStopped == FALSE)
    {
        GetClientRect(hWnd, &winRect);
        hDC = GetDC(hWnd);
        PaintColorkey(hWnd, FALSE, hDC, &winRect);
        ReleaseDC(hWnd,hDC);
        Stop_Capture();
        GetActiveOutput()->Overlay_Destroy();
        InvalidateRect(hWnd, NULL, FALSE);
        g_bOverlayStopped = TRUE;
    }
}

//---------------------------------------------------------------------------
// Restarts video overlay - 2000-10-31 Added by Mark Rejhon
// This reinitializes the video overlay to continue operation,
// so that end users can write scripts that sends this special message
// to safely restart the video after a resolution or timings change.
// This is also called during a Resume operation
void Overlay_Start(HWND hWnd)
{
    if (g_bOverlayStopped != FALSE)
    {
        InvalidateRect(hWnd, NULL, FALSE);
        GetActiveOutput()->Overlay_Create();
        Start_Capture();
        g_bOverlayStopped = FALSE;
    }
}

//---------------------------------------------------------------------------
// Show text on both OSD and statusbar
void ShowText(HWND hWnd, const string& Text)
{
    StatusBar_ShowText(STATUS_TEXT, Text);
    OSD_ShowText(Text, 0);
}

//----------------------------------------------------------------------------
// Updates the window position/window state and enable/disable titlebar 
// as necessary.  This function should be globally used for everytime 
// you want to update the window everytime you have enabled/disabled the 
// statusbar, menus, full screen state, etc.
//
// This allows for more cosmetic handling - including the ability to 
// startup directly to maximized without any intermediate cosmetic
// glitches during startup.
//
void UpdateWindowState()
{
    if(bIsFullScreen == TRUE)
    {
        RECT ScreenRect;
        UpdateWindowRegion(hWnd, FALSE);
        SetWindowLong(hWnd, GWL_STYLE, WS_VISIBLE | (IsWindowEnabled(hWnd) ? 0 : WS_DISABLED));
        SetMenu(hWnd, NULL);
        StatusBar_ShowWindow(FALSE);
        GetActiveOutput()->GetMonitorRect(hWnd, &ScreenRect);
        SetWindowPos(hWnd,
                    bAlwaysOnTopFull?HWND_TOPMOST:HWND_NOTOPMOST,
                    ScreenRect.left,
                    ScreenRect.top,
                    ScreenRect.right  - ScreenRect.left,
                    ScreenRect.bottom - ScreenRect.top,
                    SWP_SHOWWINDOW | SWP_NOACTIVATE);
    }
    else
    {
        if(bShowMenu == TRUE)
        {
            SetWindowLong(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE | (IsWindowEnabled(hWnd) ? 0 : WS_DISABLED));
            SetMenu(hWnd, hMenu);
        }
        else
        {
            if ((WindowBorder!=NULL) && WindowBorder->Visible())
            {
                SetWindowLong(hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE | (IsWindowEnabled(hWnd) ? 0 : WS_DISABLED));                
            }
            else
            {
                SetWindowLong(hWnd, GWL_STYLE, WS_THICKFRAME | WS_POPUP | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | (IsWindowEnabled(hWnd) ? 0 : WS_DISABLED));
            }
            SetMenu(hWnd, NULL);            
        }
        StatusBar_ShowWindow(bDisplayStatusBar);
        if (ToolbarControl!=NULL)
        {
            ToolbarControl->Adjust(hWnd, FALSE, FALSE);
        }
        if (UpdateWindowRegion(hWnd, FALSE) == NULL)
        {                
           if (!bShowMenu)
           {
               SetWindowLong(hWnd, GWL_STYLE, WS_THICKFRAME | WS_POPUP | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | (IsWindowEnabled(hWnd) ? 0 : WS_DISABLED));
           }
        }
        SetWindowPos(hWnd,bAlwaysOnTop?HWND_TOPMOST:HWND_NOTOPMOST,
                    0,0,0,0,
                    SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW | SWP_NOACTIVATE);            
    }    
}

HRGN UpdateWindowRegion(HWND hWnd, BOOL bUpdateWindowState)
{
    if (!bIsFullScreen && (WindowBorder!=NULL) && WindowBorder->Visible() && !bShowMenu)
    {   
        RECT rcExtra;
        if (IsStatusBarVisible())
        {
            ::SetRect(&rcExtra,0,0,0, StatusBar_Height());
        }
        else
        {
            ::SetRect(&rcExtra,0,0,0,0);
        }   
        HRGN hRgn = WindowBorder->MakeRegion(&rcExtra);
        if (hRgn != NULL)
        {
            if (bUpdateWindowState)
            {
                SetWindowLong(hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE | (IsWindowEnabled(hWnd) ? 0 : WS_DISABLED));
            }
            LOG(2,"DScaler: Set window region (0x%08x)",hRgn);
            if (hRgn != DScalerWindowRgn)
            {
                SetWindowRgn(hWnd,hRgn,TRUE);            
            }
            DScalerWindowRgn = hRgn;
        }
    }
    else
    {
        if (DScalerWindowRgn != NULL)
        {
            LOG(2,"DScaler: Set window region (0x%08x)",NULL);
            if (DScalerWindowRgn != NULL)
            {
                SetWindowRgn(hWnd,NULL,TRUE);
            }
            DScalerWindowRgn = NULL;
        }
    }
    return DScalerWindowRgn;
}

BOOL IsStatusBarVisible()
{
    return (bDisplayStatusBar == TRUE && bIsFullScreen == FALSE);
}

BOOL IsToolBarVisible()
{
    return (ToolbarControl != NULL && ToolbarControl->Visible());
}

///////////////////////////////////////////////////////////////////////////////
void Cursor_SetVisibility(BOOL bVisible)
{
    static int nCursorIndex = 1;
    if(bVisible)
    {
        while(nCursorIndex < 1)
        {
            ShowCursor(TRUE);
            nCursorIndex++;
        }
    }
    else
    {
        while(nCursorIndex > 0)
        {
            ShowCursor(FALSE);
            nCursorIndex--;
        }
    }
}

void Cursor_UpdateVisibility()
{
    KillTimer(hWnd, TIMER_HIDECURSOR);    

    if (Cursor_IsOurs() == FALSE)
    {
        Cursor_SetVisibility(TRUE);
        return;
    }

    if (!bAutoHideCursor)
    {
        if (bIsFullScreen)
        {
            Cursor_SetVisibility(FALSE);
        }
        else
        {
            Cursor_SetVisibility(bShowCursor);
        }
    }
    else
    {
        Cursor_SetVisibility(TRUE);
        SetTimer(hWnd, TIMER_HIDECURSOR, TIMER_HIDECURSOR_MS, NULL);
    }
}

int Cursor_SetType(int type)
{
    HCURSOR hCur;
    switch (type)
    {
    case CURSOR_HAND:
        hCur = hCursorHand;
        break;
    default:
        hCur = hCursorDefault;
        break;
    }

    // SetCursor changes and redraws the cursor.
    SetCursor(hCur);

    // SetClassLong makes the change permanent.
    SetClassLong(hWnd, GCL_HCURSOR, (LONG)hCur);

    return true;
}

BOOL Cursor_IsOurs()
{
    /*
     *  The cursor is ours (ie. We can hide it, change it and
     *  stuff like that) if all of these points are true:
     *
     *  1. Our menubar is not active.
     *
     *  2. Our window is enabled. (eg. no modal dialogs open)
     *
     *  3. No other window in the same thread has captured the
     *     mouse.
     *
     *
     *  And if any one of these points are true:
     *
     *  4. Our window is the foreground window and the cursor
     *     is within the bounds of the our window (or if the
     *     mouse button is down --but we don't want this.)
     *
     *  5. Our window is not in the foreground but the cursor
     *     is in the visible portion of our window and the
     *     mouse button is not down.
     *
     *  6. Our window has SetCapture() and the cursor is within
     *     the bounds of any of the windows owned by the same
     *     thread.
     *
     *
     *  How to check:
     *
     *  1: Track WM_ENTERMENULOOP and WM_EXITMENULOOP messages.
     *
     *  2: IsWindowEnabled() on our window will return false.
     *
     *  3: Check GetCapture() returns NULL or equal to our window.
     *
     *  4/5: Use WindowFromPoint() too check if the cursor is in
     *       the visible portion of the window.
     *       Use GetForegroundWindow() to determine if we are the
     *       foreground window. GetAsyncKeyState(VK_xBUTTON)
     *
     *  6: GetCapture(), WindowFromPoint(), GetWindowThreadProcessId()
     *
     */

    if (bInMenu != FALSE)
    {
        return FALSE;
    }

    if (IsWindowEnabled(hWnd) == FALSE)
    {
        return FALSE;
    }

    POINT Point;
    GetCursorPos(&Point);

    // Get the mouse over window
    HWND hPointWnd = WindowFromPoint(Point);

    // Check if our window is in the foreground
    if (GetForegroundWindow() == hWnd)
    {
        // Check if the cursor is over our bounds
        if (hPointWnd == hWnd)
        {
            return TRUE;
        }

        if (GetCapture() == hWnd)
        {
            // Check if the cursor is over a captured area
            if (GetWindowThreadProcessId(hPointWnd, NULL) ==
                GetWindowThreadProcessId(hWnd, NULL))
            {
                return TRUE;
            }
        }

        return FALSE;
    }

    // See if the cursor is not in our bounds
    if (hPointWnd != hWnd)
    {
        return FALSE;
    }

    // Get the mouse button state
    WORD wMouseButtonState = 0;
    wMouseButtonState |= GetAsyncKeyState(VK_LBUTTON);
    wMouseButtonState |= GetAsyncKeyState(VK_RBUTTON);
    wMouseButtonState |= GetAsyncKeyState(VK_MBUTTON);

    // See if the mouse button is down
    if (wMouseButtonState & 0x8000)
    {
        return FALSE;
    }

    // Get the front window in the same thread
    HWND hActiveWnd = GetActiveWindow();

    // Check if the front window has capture
    if (hActiveWnd != NULL && GetCapture() == hActiveWnd)
    {
        return FALSE;
    }

    return TRUE;
}

void Cursor_VTUpdate(int x, int y)
{
    if (Cursor_IsOurs() == FALSE)
    {
        return;
    }

    if (VT_GetState() == VT_OFF)
    {
        Cursor_SetType(CURSOR_DEFAULT);
    }
    else
    {
        RECT Rect;
        POINT Point;

        if (x == -1 || y == -1) 
        {
            GetCursorPos(&Point);
            ScreenToClient(hWnd, &Point);
        }
        else
        {
            Point.x = x;
            Point.y = y;
        }

        GetDestRect(&Rect);

        if (VT_IsPageNumberAtPosition(&Rect, &Point))
        {
            Cursor_SetType(CURSOR_HAND);
        }
        else
        {
            Cursor_SetType(CURSOR_DEFAULT);
        }
    }
}

/** Process command line parameters and return them

    Routine process the command line and returns them in argv/argc format.
    Modifies the incoming string
*/
int ProcessCommandLine(char* CommandLine, char* ArgValues[], int SizeArgv)
{
   int ArgCount = 0;
   char* pCurrentChar = CommandLine;

   while (*pCurrentChar && ArgCount < SizeArgv)
   {
      // Skip any preceeding spaces.
      while (*pCurrentChar && isspace(*pCurrentChar))
      {
         pCurrentChar++;
      }
      // If the parameter starts with a double quote, copy until
      // the end quote.
      if (*pCurrentChar == '"')
      {
         pCurrentChar++;  // Skip the quote
         ArgValues[ArgCount++] = pCurrentChar;  // Save the start in the argument list.
         while (*pCurrentChar && *pCurrentChar != '"')
         {
            pCurrentChar++;
         }
         if (*pCurrentChar)
         {
             *pCurrentChar++ = '\0';   // Replace the end quote
         }
      }
      else if (*pCurrentChar) // Normal parameter, continue until white found (or end)
      {
         ArgValues[ArgCount++] = pCurrentChar++;
         while (*pCurrentChar && !isspace(*pCurrentChar))
         {
             ++pCurrentChar;
         }
         if (*pCurrentChar)
         {
             *pCurrentChar++ = '\0';
         }
      }
   }
   return ArgCount;
}


void SetTray(BOOL Way)
{
    switch (Way)
    {
    case TRUE:
        if (bIconOn==FALSE)
        {
            nIcon.cbSize = sizeof(nIcon);
            nIcon.uID = 0;
            nIcon.hIcon = LoadIcon(hResourceInst, MAKEINTRESOURCE(IDI_TRAYICON));
            nIcon.hWnd = hWnd;
            nIcon.uCallbackMessage = IDI_TRAYICON;
            sprintf(nIcon.szTip, "DScaler");
            nIcon.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
            Shell_NotifyIcon(NIM_ADD, &nIcon);
            bIconOn = TRUE;
        }
        break;
    case FALSE:
        if (bIconOn==TRUE)
        {
            Shell_NotifyIcon(NIM_DELETE, &nIcon);
            bIconOn = FALSE;
        }
        break;
    }
}

void SetTrayTip(const string& ChannelName)
{
    if (bIconOn)
    {
        string Tip("DScaler - ");
        Tip += ChannelName;
        nIcon.uFlags = NIF_TIP;
        strncpy(nIcon.szTip, Tip.c_str(), 128);
        SetWindowText(nIcon.hWnd, nIcon.szTip);
        Shell_NotifyIcon(NIM_MODIFY, &nIcon);
    }
}

static void Init_IconMenu()
{
    hMenuTray = LoadMenu(hResourceInst, MAKEINTRESOURCE(IDC_TRAYMENU));
    if (hMenuTray!=NULL)
    {
        MENUITEMINFO mInfo;

        hMenuTray = GetSubMenu(hMenuTray, 0);

        mInfo.cbSize = sizeof(mInfo);
        mInfo.fMask = MIIM_SUBMENU;
        mInfo.hSubMenu = CreateDScalerPopupMenu();

        SetMenuItemInfo(hMenuTray, 2, TRUE, &mInfo);
        
        SetMenuDefaultItem(hMenuTray, 4, TRUE);
    }
}

int On_IconHandler(WPARAM wParam, LPARAM lParam)
{
    UINT uID;
    UINT uMouseMsg;
    POINT mPoint;

    uID = (UINT) wParam;
    uMouseMsg = (UINT) lParam;

    switch (uID)
    {
    case 0:
        switch (uMouseMsg)
        {
        case WM_LBUTTONDBLCLK:
            if (bMinimized == FALSE)
            {
                ShowWindow(hWnd, SW_MINIMIZE);
            }
            else
            {
                ShowWindow(hWnd, SW_SHOW);
                ShowWindow(hWnd, SW_RESTORE);
                SetForegroundWindow(hWnd);
            }
            break;

        case WM_RBUTTONUP:
            if (hMenuTray==NULL)
            {
                Init_IconMenu();
            }
            GetCursorPos(&mPoint);
            SetForegroundWindow(hWnd); // To correct Windows errors. See KB Q135788
            TrackPopupMenuEx(hMenuTray, TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON, mPoint.x, mPoint.y, hWnd, NULL);
            // To correct Windows  errors. See KB Q135788. I am not really sure if this is necessary
            // with TrackPopupMenuEx() but it doesn't hurt either.
            PostMessage(hWnd, WM_NULL, 0, 0);
            break;

        }
        break;
    }
    
    return 0;
}



////////////////////////////////////////////////////////////////////////////
// On Change Functions for settings
/////////////////////////////////////////////////////////////////////////////

BOOL IsFullScreen_OnChange(long NewValue)
{
    bDoResize = FALSE;
    bIsFullScreen = (BOOL)NewValue;

    // make sure that the window has been created
    if(hWnd != NULL)
    {
        if(bIsFullScreen == FALSE)
        {
            if(lPStripTimingString != NULL)
            {
                OutReso_Change(hWnd, hPSWnd, TRUE, TRUE, lPStripTimingString, TRUE);
            }
            else
            {
                OutReso_Change(hWnd, hPSWnd, TRUE, TRUE, lPStripTimingString, FALSE);
            }
            SetWindowPos(hWnd, 0, MainWndLeft, MainWndTop, MainWndWidth, MainWndHeight, SWP_SHOWWINDOW);
            if (bDisplayStatusBar == TRUE)
            {
                SetTimer(hWnd, TIMER_STATUS, TIMER_STATUS_MS, NULL);
            }
        }
        else
        {
            if(hPSWnd)
            {
                // Save the actual PowerStrip timing string in lPStripTimingString
                SaveActualPStripTiming(hPSWnd);
            }
            SaveWindowPos(hWnd);                
            KillTimer(hWnd, TIMER_STATUS);
            OutReso_Change(hWnd, hPSWnd, FALSE, TRUE, NULL, FALSE);
        }
        if (WindowBorder!=NULL)
        {
            if (bIsFullScreen && WindowBorder->Visible())
            {                
                WindowBorder->Hide();
            }
            else if (!bIsFullScreen && szSkinName[0]!=0)
            {
                WindowBorder->Show();
            }
        }
        if (ToolbarControl!=NULL)
        {            
            ToolbarControl->Set(hWnd, NULL, 0, 1);
        }
        
        Cursor_UpdateVisibility();
        //InvalidateRect(hWnd, NULL, FALSE);
        UpdateWindowState();
        WorkoutOverlaySize(FALSE);

        // We must do two calls, the first one displaying the toolbar
        // in order to have the correct toolbar rectangle initialized,
        // and the second to hide the toolbar if in full scrren mode
        if (ToolbarControl!=NULL)
        {
            ToolbarControl->Set(hWnd, NULL, bIsFullScreen?1:0, 1);
        }
        if (IsToolBarVisible())
        {
            SetTimer(hWnd, TIMER_TOOLBAR, TIMER_TOOLBAR_MS, NULL);
        }
        else
        {
            KillTimer(hWnd, TIMER_TOOLBAR);
        }
    }
    bDoResize = TRUE;
    return FALSE;
}

BOOL AlwaysOnTop_OnChange(long NewValue)
{
    bAlwaysOnTop = (BOOL)NewValue;
    WorkoutOverlaySize(FALSE);
    return FALSE;
}

BOOL PriorClassId_OnChange(long NewValue)
{
    PriorClassId = (long)NewValue;
    UpdatePriorityClass();
    return FALSE;
}

BOOL ThreadClassId_OnChange(long NewValue)
{
    ThreadClassId = (long)NewValue;
    SetOutputThreadPriority();
    if (ThreadClassId == 0)
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
    else if (ThreadClassId == 1)
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
    else if (ThreadClassId == 2)
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
    else if (ThreadClassId == 3)
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    else if (ThreadClassId == 4)
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    return FALSE;
}

BOOL AlwaysOnTopFull_OnChange(long NewValue)
{
    bAlwaysOnTopFull = (BOOL)NewValue;
    WorkoutOverlaySize(FALSE);
    return FALSE;
}

BOOL ScreensaverOff_OnChange(long NewValue)
{
    bScreensaverOff = (BOOL)NewValue;
    SetScreensaverMode(bScreensaverOff);
    return FALSE;
}

BOOL DisplayStatusBar_OnChange(long NewValue)
{
    bDisplayStatusBar = (BOOL)NewValue;
    if(bIsFullScreen == FALSE)
    {
        if(bDisplayStatusBar == TRUE)
        {
            SetTimer(hWnd, TIMER_STATUS, TIMER_STATUS_MS, NULL);
        }
        else
        {
            KillTimer(hWnd, TIMER_STATUS);
        }
        if (ToolbarControl!=NULL)
        {
            ToolbarControl->Adjust(hWnd, TRUE, FALSE);
        }        
        UpdateWindowState();
        WorkoutOverlaySize(TRUE);
    }
    return FALSE;
}

BOOL ShowMenu_OnChange(long NewValue)
{
    bShowMenu = (BOOL)NewValue;
    if(bIsFullScreen == FALSE)
    {
        WorkoutOverlaySize(TRUE);
    }
    return FALSE;
}

BOOL KeyboardLock_OnChange(long NewValue)
{
    bKeyboardLock = (BOOL)NewValue;
    SetKeyboardLock(bKeyboardLock);
    return FALSE;
}


BOOL MinimizeHandling_OnChange(long NewValue)
{
    MinimizeHandling = (int)NewValue;
    if (bMinimized)
    {
        bCheckSignalPresent = (MinimizeHandling == 2);
    }
    else
    {
        bCheckSignalMissing = (MinimizeHandling == 2);
    }
    return FALSE;
}


BOOL ChannelPreviewNbCols_OnChange(long NewValue)
{
    ChannelPreviewNbCols = (int)NewValue;
    if (pMultiFrames && (pMultiFrames->GetMode() == PREVIEW_CHANNELS) && pMultiFrames->IsActive())
    {
        pMultiFrames->RequestSwitch();
    }
    return FALSE;
}

BOOL ChannelPreviewNbRows_OnChange(long NewValue)
{
    ChannelPreviewNbRows = (int)NewValue;
    if (pMultiFrames && (pMultiFrames->GetMode() == PREVIEW_CHANNELS) && pMultiFrames->IsActive())
    {
        pMultiFrames->RequestSwitch();
    }
    return FALSE;
}


////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING DScalerSettings[DSCALER_SETTING_LASTONE] =
{
    {
        "Window Left", SLIDER, 0, (long*)&MainWndLeft,
        10, -2048, 2048, 1, 1,
        NULL,
        "MainWindow", "StartLeft", NULL,
    },
    {
        "Window Top", SLIDER, 0, (long*)&MainWndTop,
        10, -2048, 2048, 1, 1,
        NULL,
        "MainWindow", "StartTop", NULL,
    },
    {
        "Window Width", SLIDER, 0, (long*)&MainWndWidth,
        649, 0, 2048, 1, 1,
        NULL,
        "MainWindow", "StartWidth", NULL,
    },
    {
        "Window Height", SLIDER, 0, (long*)&MainWndHeight,
        547, 0, 2048, 1, 1,
        NULL,
        "MainWindow", "StartHeight", NULL,
    },
    {
        "Always On Top (Window)", YESNO, 0, (long*)&bAlwaysOnTop,
        TRUE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "AlwaysOnTop", AlwaysOnTop_OnChange,
    },
    {
        "Full Screen", YESNO, 0, (long*)&bIsFullScreen,
        FALSE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "bIsFullScreen", IsFullScreen_OnChange,
    },
    {
        "Force Full Screen", ONOFF, 0, (long*)&bForceFullScreen,
        FALSE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "AlwaysForceFullScreen", NULL,
    },
    {
        "Status Bar", ONOFF, 0, (long*)&bDisplayStatusBar,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Show", "StatusBar", DisplayStatusBar_OnChange,
    },
    {
        "Menu", ONOFF, 0, (long*)&bShowMenu,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Show", "Menu", ShowMenu_OnChange,
    },
    {
        "Window Processor", SLIDER, 0, (long*)&MainProcessor,
        0, 0, 3, 1, 1,
        NULL,
        "Threads", "WindowProcessor", NULL,
    },
    {
        "Thread Processor", SLIDER, 0, (long*)&DecodeProcessor,
        0, 0, 3, 1, 1,
        NULL,
        "Threads", "DecodeProcessor", NULL,
    },
    {
        "Priority class", ITEMFROMLIST, 0, (long*)&PriorClassId,
        0, 0, 2, 1, 1,
        UIPriorityNames,
        "Threads", "WindowPriority", PriorClassId_OnChange,
    },
    {
        "Decoding / Output and UI Thread", ITEMFROMLIST, 0, (long*)&ThreadClassId,
        1, 0, 4, 1, 1,
        DecodingPriorityNames,
        "Threads", "ThreadPriority", ThreadClassId_OnChange,
    },
    {
        "Autosave settings", ONOFF, 0, (long*)&bUseAutoSave,
        FALSE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "UseAutoSave", NULL,
    },
    {
        "Always On Top (Full Screen)", YESNO, 0, (long*)&bAlwaysOnTopFull,
        TRUE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "AlwaysOnTopFull", AlwaysOnTopFull_OnChange,
    },
    {
        "Show Crash Dialog", ONOFF, 0, (long*)&bShowCrashDialog,
        FALSE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "ShowCrashDialog", NULL,
    },
    {
        "Splash Screen", ONOFF, 0, (long*)&bDisplaySplashScreen,
        TRUE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "DisplaySplashScreen", NULL,
    },
    {
        "Auto Hide Cursor", ONOFF, 0, (long*)&bAutoHideCursor,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Show", "AutoHideCursor", NULL,
    },
    {
        "Lock keyboard", ONOFF, 0, (long*)&bKeyboardLock,
        FALSE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "KeyboardLock", KeyboardLock_OnChange,
    },
    {
        "Keyboard lock affects main window only", ONOFF, 0, (long*)&bKeyboardLockMainWindowOnly,
        FALSE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "KeyboardLockMainWindowOnly", NULL,
    },
    {
        "Disable Screensaver", YESNO, 0, (long*)&bScreensaverOff,
        TRUE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "ScreensaverOff", ScreensaverOff_OnChange,
    },
    {
        "Auto CodePage", YESNO, 0, (long*)&bVTAutoCodePage,
        TRUE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "AutoCodePage", NULL,
    },
    {
        "VT Anti-alias", YESNO, 0, (long*)&bVTAntiAlias,
        TRUE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "VTAntiAlias", NULL,
    },
    {
        "Initial source", SLIDER, 0, (long*)&InitSourceIdx,
        -1, -1, 100, 1, 1,
        NULL,
        "Show", "InitialSource", NULL,
    },
    {
        "Channel enter time (ms)", SLIDER, 0, (long*)&ChannelEnterTime,
        TIMER_KEYNUMBER_MS, 0, 20000, 1, 1,
        NULL,
        "MainWindow", "ChannelEnterTime", NULL,
    },
    {
        "Processor Speed", SLIDER, 0, (long*)&ProcessorSpeed,
        3, 0, 3, 1, 1,
        NULL,
        "MainWindow", "ProcessorSpeed", NULL,
    },
    {
        "Quality Trade Off", SLIDER, 0, (long*)&TradeOff,
        1, 0, 1, 1, 1,
        NULL,
        "MainWindow", "TradeOff", NULL,
    },
    {
        "Use Full CPU", SLIDER, 0, (long*)&FullCpu,
        1, 0, 2, 1, 1,
        NULL,
        "MainWindow", "FullCpu", NULL,
    },
    {
        "Video Card", SLIDER, 0, (long*)&VideoCard,
        0, 0, 0, 1, 1,
        NULL,
        "MainWindow", "VideoCard", NULL,
    },
    {
        "Reverse channel scrolling", ONOFF, 0, (long*)&bReverseChannelScroll,
        FALSE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "ReverseChannelScroll", NULL,
    },
    {
        "Single key teletext toggle", ONOFF, 0, (long*)&bVTSingleKeyToggle,
        TRUE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "SingleKeyTeletextToggle", NULL,
    },
    {
        "Minimize to the Windows system tray", ONOFF, 0, (long*)&bMinToTray,
        FALSE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "MinToTray", NULL,
    },
    {
        "Minimize handling", ITEMFROMLIST, 0, (long*)&MinimizeHandling,
        0, 0, 2, 1, 1,
        MinimizeHandlingLabels,
        "MainWindow", "MinimizeHandling", MinimizeHandling_OnChange,
    },
    {
        "Display Resolution in Full Screen", SLIDER, 0, (long*)&OutputReso,
        0, 0, MAX_NUMBER_RESO, 1, 1,
        NULL,
        "MainWindow", "ResoFullScreen", NULL,
    },
    {
        "PowerStrip resolution for 576i sources", CHARSTRING, 0, (long*)&PStrip576i,
        (long)"", 0, 0, 0, 0,
        NULL,
        "PStripOutResolution", "576i", NULL,
    },
    {
        "PowerStrip resolution for 480i sources", CHARSTRING, 0, (long*)&PStrip480i,
        (long)"", 0, 0, 0, 0,
        NULL,
        "PStripOutResolution", "480i", NULL,
    },
    {
        "Skin name", CHARSTRING, 0, (long*)&szSkinName,
        (long)"", 0, 0, 0, 0,
        NULL,
        "MainWindow", "SkinName", NULL,
    },
    {
        "Number of columns in preview mode", SLIDER, 0, (long*)&ChannelPreviewNbCols,
         4, 2, 10, 1, 1,
         NULL,
        "Still", "ChannelPreviewNbCols", ChannelPreviewNbCols_OnChange,
    },
    {
        "Number of rows in preview mode", SLIDER, 0, (long*)&ChannelPreviewNbRows,
         4, 2, 10, 1, 1,
         NULL,
        "Still", "ChannelPreviewNbRows", ChannelPreviewNbRows_OnChange,
    },
    {
        "Output Method", ITEMFROMLIST, 0, (long*)&OutputMethod,
        0, 0, 1, 1, 1,
        OutputMethodNames,
        "MainWindow", "OutputMethod", NULL,
    },

};

SETTING* DScaler_GetSetting(DSCALER_SETTING Setting)
{
    if(Setting > -1 && Setting < DSCALER_SETTING_LASTONE)
    {
        return &(DScalerSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void DScaler_ReadSettingsFromIni()
{
    int i;
    for(i = 0; i < DSCALER_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(DScalerSettings[i]));
    }

    if(bForceFullScreen)
    {
        bIsFullScreen = TRUE;
    }
    if(bKeyboardLock)
    {
        SetKeyboardLock(TRUE);
    }
    ScreensaverOff_OnChange(bScreensaverOff);

    VT_SetAutoCodepage(NULL, NULL, bVTAutoCodePage);
    VT_SetAntialias(NULL, NULL, bVTAntiAlias);
}

void DScaler_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < DSCALER_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(DScalerSettings[i]), bOptimizeFileAccess);
    }
}

void DScaler_FreeSettings()
{
    int i;
    for(i = 0; i < DSCALER_SETTING_LASTONE; i++)
    {
        Setting_Free(&(DScalerSettings[i]));
    }
}

CTreeSettingsGeneric* DScaler_GetTreeSettingsPage()
{
    return new CTreeSettingsGeneric("Threads Priority Settings", &DScalerSettings[WINDOWPRIORITY], AUTOSAVESETTINGS - WINDOWPRIORITY);
}

CTreeSettingsGeneric* DScaler_GetTreeSettingsPage2()
{
    // Other Settings
    SETTING* OtherSettings[7] =
    {
        &DScalerSettings[DISPLAYSPLASHSCREEN    ],
        &DScalerSettings[AUTOHIDECURSOR         ],
        &DScalerSettings[LOCKKEYBOARD           ],
        &DScalerSettings[LOCKKEYBOARDMAINWINDOWONLY],
        &DScalerSettings[SCREENSAVEROFF         ],
        &DScalerSettings[SINGLEKEYTELETEXTTOGGLE],
        &DScalerSettings[MINIMIZEHANDLING       ],
    };
    return new CTreeSettingsGeneric("Other Settings", OtherSettings, sizeof(OtherSettings) / sizeof(OtherSettings[0]));
}

CTreeSettingsGeneric* DScaler_GetTreeSettingsPage3()
{
    // Channel Settings
    SETTING* OtherSettings[4] =
    {
        &DScalerSettings[REVERSECHANNELSCROLLING],
        &DScalerSettings[CHANNELPREVIEWWNBCOLS],
        &DScalerSettings[CHANNELPREVIEWNBROWS],
        &DScalerSettings[CHANNELENTERTIME],
    };
    return new CTreeSettingsGeneric("Channel Settings", OtherSettings, sizeof(OtherSettings) / sizeof(OtherSettings[0]));
}

CTreeSettingsGeneric* DScaler_GetTreeSettingsPage4()
{
    // PowerStrip Settings
    SETTING* OtherSettings[2] =
    {
        &DScalerSettings[PSTRIPRESO576I            ],
        &DScalerSettings[PSTRIPRESO480I            ],
    };
    return new CTreeSettingsGeneric("PowerStrip Settings", OtherSettings, sizeof(OtherSettings) / sizeof(OtherSettings[0]));
}

HWND GetMainWnd()
{
    if(!IsMainWindowThread())
    {
        LOG(1, "Window request from outside main thread");
    }
    return hWnd;
}

void PostMessageToMainWindow(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    PostMessage(hWnd, Msg, wParam, lParam);
}

BOOL IsMainWindowThread()
{
    return (GetCurrentThreadId() == hMainThread);
}

void WaitForMainWindowEvent()
{
    if(hMainWindowEvent != NULL)
    {
        if(WaitForSingleObject(hMainWindowEvent, 10) == WAIT_TIMEOUT)
        {
            LOG(1, "Timed out waiting for window message to be processed");
        }
    }
}

void ResetMainWindowEvent()
{
    if(hMainWindowEvent == NULL)
    {
        hMainWindowEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    }
    if(hMainWindowEvent != NULL)
    {
        ResetEvent(hMainWindowEvent);
    }
}

#if _MSC_VER > 1310
#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='X86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif