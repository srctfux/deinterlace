# Microsoft Developer Studio Project File - Name="DI_TomsMoComp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=DI_TomsMoComp - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DI_TomsMoComp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DI_TomsMoComp.mak" CFG="DI_TomsMoComp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DI_TomsMoComp - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DI_TomsMoComp - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DI_TomsMoComp - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DI_TOMSMOCOMP_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\Api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DI_TOMSMOCOMP_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"..\..\Release/DI_TomsMoComp.dll"

!ELSEIF  "$(CFG)" == "DI_TomsMoComp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DI_TOMSMOCOMP_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /Zi /Od /I "..\..\Api" /D "DI_TOMSMOCOMP_EXPORTS" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Fr /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:no /debug /machine:I386 /out:"..\..\Debug/DI_TomsMoComp.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "DI_TomsMoComp - Win32 Release"
# Name "DI_TomsMoComp - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\DI_TomsMoComp.c
# End Source File
# Begin Source File

SOURCE=.\DI_TomsMoComp3DNOW.c
# End Source File
# Begin Source File

SOURCE=.\DI_TomsMoCompMMX.c
# End Source File
# Begin Source File

SOURCE=.\DI_TomsMoCompSSE.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\SearchLoop0A.inc
# End Source File
# Begin Source File

SOURCE=.\SearchLoopBottom.inc
# End Source File
# Begin Source File

SOURCE=.\SearchLoopEdgeA.inc
# End Source File
# Begin Source File

SOURCE=.\SearchLoopEdgeA8.inc
# End Source File
# Begin Source File

SOURCE=.\SearchLoopEdgeAH.inc
# End Source File
# Begin Source File

SOURCE=.\SearchLoopEdgeAH8.inc
# End Source File
# Begin Source File

SOURCE=.\SearchLoopOddA.inc
# End Source File
# Begin Source File

SOURCE=.\SearchLoopOddA2.inc
# End Source File
# Begin Source File

SOURCE=.\SearchLoopOddA6.inc
# End Source File
# Begin Source File

SOURCE=.\SearchLoopOddAH.inc
# End Source File
# Begin Source File

SOURCE=.\SearchLoopOddAH2.inc
# End Source File
# Begin Source File

SOURCE=.\SearchLoopOddAH6.inc
# End Source File
# Begin Source File

SOURCE=.\SearchLoopOddAHH2.inc
# End Source File
# Begin Source File

SOURCE=.\SearchLoopTop.inc
# End Source File
# Begin Source File

SOURCE=.\SearchLoopVA.inc
# End Source File
# Begin Source File

SOURCE=.\SearchLoopVAH.inc
# End Source File
# Begin Source File

SOURCE=.\StrangeBob.inc
# End Source File
# Begin Source File

SOURCE=.\TomsMoComp.h
# End Source File
# Begin Source File

SOURCE=.\TomsMoCompAll.inc
# End Source File
# Begin Source File

SOURCE=.\TomsMoCompAll2.inc
# End Source File
# Begin Source File

SOURCE=.\WierdBob.inc
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\DI_TomsMoComp.rc
# End Source File
# End Group
# End Target
# End Project
