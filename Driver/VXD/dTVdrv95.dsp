# Microsoft Developer Studio Project File - Name="dTVdrv95" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=dTVdrv95 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "dTVdrv95.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dTVdrv95.mak" CFG="dTVdrv95 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dTVdrv95 - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "dTVdrv95 - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "dTVdrv95 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Cmd_Line "NMAKE /f dTVdrv95.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "dTVdrv95.exe"
# PROP BASE Bsc_Name "dTVdrv95.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Cmd_Line "ddkbuild d:\98ddk release"
# PROP Rebuild_Opt "clean"
# PROP Target_File "..\..\dTV\dTVdrv95.vxd"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "dTVdrv95 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Cmd_Line "NMAKE /f dTVdrv95.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "dTVdrv95.exe"
# PROP BASE Bsc_Name "dTVdrv95.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Cmd_Line "ddkbuild d:\98ddk debug"
# PROP Rebuild_Opt "clean"
# PROP Target_File "..\..\dTV\dTVdrv95.vxd"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "dTVdrv95 - Win32 Release"
# Name "dTVdrv95 - Win32 Debug"

!IF  "$(CFG)" == "dTVdrv95 - Win32 Release"

!ELSEIF  "$(CFG)" == "dTVdrv95 - Win32 Debug"

!ENDIF 

# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\CRTL.CPP
# End Source File
# Begin Source File

SOURCE=..\COMMON\DEBUGOUT.CPP
# End Source File
# Begin Source File

SOURCE=.\dTVdrv95.CPP
# End Source File
# Begin Source File

SOURCE=.\dTVdrv95.def
# End Source File
# Begin Source File

SOURCE=..\COMMON\Ioclass.cpp
# End Source File
# Begin Source File

SOURCE=..\COMMON\PCIENUM.CPP
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\common\DEBUGOUT.H
# End Source File
# Begin Source File

SOURCE=..\common\IOCLASS.H
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\Makefile
# End Source File
# Begin Source File

SOURCE=.\VXDSTUB.ASM
# End Source File
# End Target
# End Project
