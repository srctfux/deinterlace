# Microsoft Developer Studio Project File - Name="dTV" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=dTV - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "dTV.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dTV.mak" CFG="dTV - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dTV - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "dTV - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dTV - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "dTV___Win32_Debug0"
# PROP BASE Intermediate_Dir "dTV___Win32_Debug0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "."
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /W3 /Gm /GX /Zi /Od /I "..\driver\include\\" /D "_DEBUG" /D "_WINDOWS" /D "WIN32" /D "_MBCS" /FR /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /Z7 /Od /I "..\driver\include\\" /D "_DEBUG" /D "_WINDOWS" /D "WIN32" /D "_MBCS" /FR /YX"stdafx.h" /FD /c
# ADD BASE MTL /D "_DEBUG" /mktyplib203 /win32
# SUBTRACT BASE MTL /nologo
# ADD MTL /D "_DEBUG" /mktyplib203 /win32
# SUBTRACT MTL /nologo
# ADD BASE RSC /l 0x1 /d "_DEBUG"
# ADD RSC /l 0x1 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo
# SUBTRACT BSC32 /Iu
LINK32=link.exe
# ADD BASE LINK32 ddraw.lib dxguid.lib kernel32.lib user32.lib gdi32.lib advapi32.lib winmm.lib comdlg32.lib ..\driver\bin\dTVdrv.lib /nologo /subsystem:windows /profile /debug /machine:I386 /out:".\dTV.exe"
# SUBTRACT BASE LINK32 /map /nodefaultlib
# ADD LINK32 ddraw.lib dxguid.lib kernel32.lib user32.lib gdi32.lib advapi32.lib winmm.lib comdlg32.lib .\dTVdrv.lib /nologo /subsystem:windows /pdb:none /map:"dTV.map" /debug /machine:I386
# SUBTRACT LINK32 /profile

!ELSEIF  "$(CFG)" == "dTV - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "dTV___Win32_Release"
# PROP BASE Intermediate_Dir "dTV___Win32_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "."
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MT /W3 /Gi /Ot /I "..\driver\include\\" /D "NDEBUG" /D "_WINDOWS" /D "WIN32" /D "_MBCS" /YX /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /G6 /MT /W3 /Gi /Ot /I "..\driver\include\\" /D "NDEBUG" /D "_WINDOWS" /D "WIN32" /D "_MBCS" /Fr /YX"stdafx.h" /FD /c
# ADD BASE MTL /D "NDEBUG" /mktyplib203 /win32
# SUBTRACT BASE MTL /nologo
# ADD MTL /D "NDEBUG" /mktyplib203 /win32
# SUBTRACT MTL /nologo
# ADD BASE RSC /l 0x420 /d "NDEBUG"
# ADD RSC /l 0x420 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# SUBTRACT BASE BSC32 /Iu
# ADD BSC32 /nologo
# SUBTRACT BSC32 /Iu
LINK32=link.exe
# ADD BASE LINK32 ddraw.lib dxguid.lib kernel32.lib user32.lib gdi32.lib advapi32.lib winmm.lib comdlg32.lib ..\driver\bin\hwiodrv.lib /nologo /subsystem:windows /incremental:yes /machine:I386
# SUBTRACT BASE LINK32 /profile /map /debug /nodefaultlib
# ADD LINK32 ddraw.lib dxguid.lib kernel32.lib user32.lib gdi32.lib advapi32.lib winmm.lib comdlg32.lib .\dTVdrv.lib /nologo /subsystem:windows /incremental:yes /machine:I386
# SUBTRACT LINK32 /profile /map /debug /nodefaultlib

!ENDIF 

# Begin Target

# Name "dTV - Win32 Debug"
# Name "dTV - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\Audio.c
# End Source File
# Begin Source File

SOURCE=.\Bt848.c
# End Source File
# Begin Source File

SOURCE=.\bTVPlugin.c
# End Source File
# Begin Source File

SOURCE=.\ccdecode.c
# End Source File
# Begin Source File

SOURCE=.\DebugLog.c
# End Source File
# Begin Source File

SOURCE=.\Deinterlace.c
# End Source File
# Begin Source File

SOURCE=.\Dialogs.c
# End Source File
# Begin Source File

SOURCE=.\dTV.C
# End Source File
# Begin Source File

SOURCE=.\dTV.RC
# End Source File
# Begin Source File

SOURCE=.\ErrorBox.c
# End Source File
# Begin Source File

SOURCE=.\globals.c
# End Source File
# Begin Source File

SOURCE=.\i2c.c
# End Source File
# Begin Source File

SOURCE=.\MixerDev.c
# End Source File
# Begin Source File

SOURCE=.\Other.c
# End Source File
# Begin Source File

SOURCE=.\OutThreads.c
# End Source File
# Begin Source File

SOURCE=.\ProgramList.c
# End Source File
# Begin Source File

SOURCE=.\settings.c
# End Source File
# Begin Source File

SOURCE=.\STATUS.C
# End Source File
# Begin Source File

SOURCE=.\Tuner.c
# End Source File
# Begin Source File

SOURCE=.\TVCards.c
# End Source File
# Begin Source File

SOURCE=.\vbi.c
# End Source File
# Begin Source File

SOURCE=.\vt.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\Audio.h
# End Source File
# Begin Source File

SOURCE=.\bt848.h
# End Source File
# Begin Source File

SOURCE=.\bTVPlugin.h
# End Source File
# Begin Source File

SOURCE=.\ccdecode.h
# End Source File
# Begin Source File

SOURCE=.\DebugLog.h
# End Source File
# Begin Source File

SOURCE=.\defines.h
# End Source File
# Begin Source File

SOURCE=.\Deinterlace.h
# End Source File
# Begin Source File

SOURCE=.\Dialogs.h
# End Source File
# Begin Source File

SOURCE=.\dTV.H
# End Source File
# Begin Source File

SOURCE=.\ErrorBox.h
# End Source File
# Begin Source File

SOURCE=.\globals.h
# End Source File
# Begin Source File

SOURCE=.\i2c.h
# End Source File
# Begin Source File

SOURCE=.\MixerDev.h
# End Source File
# Begin Source File

SOURCE=.\Other.h
# End Source File
# Begin Source File

SOURCE=.\OutThreads.h
# End Source File
# Begin Source File

SOURCE=.\ProgramList.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\settings.h
# End Source File
# Begin Source File

SOURCE=.\status.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\structs.h
# End Source File
# Begin Source File

SOURCE=.\Tuner.h
# End Source File
# Begin Source File

SOURCE=.\TVCards.h
# End Source File
# Begin Source File

SOURCE=.\vbi.h
# End Source File
# Begin Source File

SOURCE=.\vt.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\RES\bgruen1.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\brot1.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\dTV.ico
# End Source File
# Begin Source File

SOURCE=.\RES\sizex.cur
# End Source File
# Begin Source File

SOURCE=.\RES\SLIDER.BMP
# End Source File
# Begin Source File

SOURCE=.\RES\SLIDER1.BMP
# End Source File
# Begin Source File

SOURCE=.\Res\Startup.bmp
# End Source File
# Begin Source File

SOURCE=.\res\VTBack.ico
# End Source File
# Begin Source File

SOURCE=.\res\VTHome.ico
# End Source File
# Begin Source File

SOURCE=.\res\VTNext.ico
# End Source File
# Begin Source File

SOURCE=.\RES\VTUpDown.ico
# End Source File
# Begin Source File

SOURCE=.\res\VTx10x12.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Vtx15x18.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\32Spec.htm
# End Source File
# Begin Source File

SOURCE=.\Authors.htm
# End Source File
# Begin Source File

SOURCE=.\Channel.lst
# End Source File
# Begin Source File

SOURCE=.\COPYING.html
# End Source File
# Begin Source File

SOURCE=.\dTV_Readme.htm
# End Source File
# Begin Source File

SOURCE=.\News.htm
# End Source File
# Begin Source File

SOURCE=.\Thanks.htm
# End Source File
# Begin Source File

SOURCE=.\Thanks_MultiDec.txt
# End Source File
# Begin Source File

SOURCE=.\Todo.htm
# End Source File
# End Target
# End Project
